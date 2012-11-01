/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>
#include <stats.h>
#include <sched.h>
#include <errno.h>
#include <system.h>
#include <entry.h>

#define LECTURA 0
#define ESCRIPTURA 1

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -EBADF; /*EBADF*/
  if (permissions!=ESCRIPTURA) return -EACCES; /*EACCES*/
  return 0;
}

int sys_ni_syscall()
{
	return -ENOSYS; /*ENOSYS*/
}

int sys_getpid()
{
	return current()->PID;
}

/* Funció de debug per al task_switch */
int sys_DEBUG_tswitch() {
	struct list_head *new_list_task = list_first(&readyqueue);
	list_del(new_list_task);
	struct task_struct * new_task = list_head_to_task_struct(new_list_task);
	struct task_struct * current_task = current();

	list_add_tail(&current_task->list,&readyqueue);

	task_switch((union task_union*)new_task);

	return 0;
}

int sys_fork()
{
	int PID;
	int current_ebp = 0;
	unsigned int pos_ebp = 0; // posició del ebp en la stack: new/current_stack->stack[pos_ebp]
	int pag;
	int new_ph_pag;
	int frames[NUM_PAG_DATA];

	/* Punt a: Obtenció d'una task_struct nova de la freequeue */
	if (list_empty(&freequeue)) return -ENTASK;
	struct list_head *new_list_pointer = list_first(&freequeue);
	list_del(new_list_pointer);
	struct task_struct * new_pcb = list_head_to_task_struct(new_list_pointer);
	struct task_struct * current_pcb = current();
	union task_union *new_stack = (union task_union*)new_pcb;

	/* Càlcul de pos_ebp */
	__asm__ __volatile__(
	  		"mov %%ebp,%0;"
	  		: "=r" (current_ebp)
	  );
	pos_ebp = ((unsigned int)current_ebp-(unsigned int)current_pcb)/4;

	/* Punt b: Obtenció dels frames per al nou procés */
	for (pag=0;pag<NUM_PAG_DATA;pag++){
		new_ph_pag=alloc_frame();
		if (new_ph_pag == -1) {
			while(pag != 0) free_frame(frames[--pag]);
			return -ENMPHP;
		}
		else frames[pag] = new_ph_pag;
	}

	/* Punt c: Copia del Stack, i restauració del directori del fill*/
	page_table_entry * copy_dir_pages_baseAddr = get_DIR(new_pcb);
	copy_data(current_pcb, new_pcb, 4096);
	new_pcb->dir_pages_baseAddr = copy_dir_pages_baseAddr;

	/* Punt d.i: Copia de les page tables de codi, i assignació de
	 * 						frames per a les dades	*/
	page_table_entry * pt_new = get_PT(new_pcb);
	page_table_entry * pt_current = get_PT(current_pcb);
	page_table_entry * dir_current = get_DIR(current_pcb);

	/* CODE */
	for (pag=0;pag<NUM_PAG_CODE;pag++){
		pt_new[PAG_LOG_INIT_CODE_P0+pag].entry = pt_current[PAG_LOG_INIT_CODE_P0+pag].entry;
	}

	/* DATA + Punt b: Obtenció de pàgines físiques */
	for (pag=0;pag<NUM_PAG_DATA;pag++){
		set_ss_pag(pt_new,PAG_LOG_INIT_DATA_P0+pag,frames[pag]);
	}

	/* Punt d.ii */
	/* Gestió extra per evitar problemes amb Memoria dinàmica:
	 * 	- Busquem entrades a la page_table lliures, en comptes de 20 directes
	 * 	- Si arribem al limit sense haver-ne trobat ni una retornem error, doncs
	 * 		no es pot fer el fork per falta de page tables entry al pare.
	 * 	- Si arribem al limit però hem pogut fer almenys una copia:
	 * 		flush a la TLB i tornem a buscar desde el principi.
	 * */
	int free_pag = FIRST_FREE_PAG_P;
	for (pag=0;pag<NUM_PAG_DATA;pag++){
			while(pt_current[free_pag].entry != 0 && free_pag<TOTAL_PAGES) free_pag++;

			if (free_pag == TOTAL_PAGES) {
				if (pag != 0) {
					free_pag = FIRST_FREE_PAG_P;
					--pag;
					set_cr3(dir_current);
				}
				else return -ENEPTE;
			}
			else {
				/* d.ii.A: Assignació de noves pàgines logiques al procés actual, corresponents
				 * 					a les pàgines físiques obtingudes per al procés nou	*/
				set_ss_pag(pt_current,free_pag,pt_new[PAG_LOG_INIT_DATA_P0+pag].bits.pbase_addr);

				/* d.ii.B: Copia de l'espai d'usuari del proces actual al nou */
				copy_data((void *)((PAG_LOG_INIT_DATA_P0+pag)<<12),	(void *)(free_pag<<12), PAGE_SIZE);

				/* d.ii.C: Desassignació de les pagines en el procés actual */
				del_ss_pag(pt_current, free_pag);

				free_pag++;
			}
	}

	/* Flush de la TLB */
	set_cr3(dir_current);

	/* Punt e */
	PID = getNewPID();
	new_pcb->PID = PID;

	/* Punt f i g */
	// eax del Save_all (Serà el pid de retorn del fill)
	new_stack->stack[pos_ebp+8] = 0;
	// Construint l'enllaç dinamic fent que el esp apunti al ebp guardat
	new_stack->task.kernel_esp = (unsigned int)&new_stack->stack[pos_ebp];
	// Modificant la funció a on retornarà
	new_stack->stack[pos_ebp+1] = (unsigned int)&ret_from_fork;

	/* Inicialització estadistica */
	new_stack->task.process_state = ST_READY;
	new_stack->task.statistics.tics = 0;
	new_stack->task.statistics.cs = 0;

	/* Punt h */
	list_add_tail(&new_pcb->list,&readyqueue);

	return PID;
}

void sys_exit() {
	/* Punt a */
	int pag;
	struct task_struct * current_pcb = current();
	page_table_entry * pt_current = get_PT(current_pcb);

	// Allibera només 20 page de Data del procés, canviar en Mem.dinamica
	for (pag=0;pag<NUM_PAG_DATA;pag++){
		free_frame(pt_current[PAG_LOG_INIT_DATA_P0+pag].bits.pbase_addr);
	}

	/* Punt b */
	sched_update_queues_state(&freequeue);
	sched_switch_process();
}

int sys_write(int fd, char * buffer, int size) {
	char buff[4];
	int ret = 0;
	
	ret = check_fd(fd,ESCRIPTURA);
	if (ret != 0) return ret;
  if (buffer == NULL) return -EPNULL;
	if (size <= 0) return -ESIZEB;
	if (access_ok(VERIFY_READ, buffer, size) == 0) return -ENACCB;

	while (size > 4) {
		copy_from_user(buffer, buff, 4);
		sys_write_console(buff,4);
		buffer += 4;
		size -= 4;
	}
	copy_from_user(buffer, buff, size);
	sys_write_console(buff,size);

	return 0;
}

int sys_gettime() {
	return zeos_ticks;
}

int sys_get_stats(int pid, struct stats *st) {
	struct task_struct * desired;
	int found = getStructPID(pid, &desired);

	if (found == 1) {
		if (access_ok(VERIFY_WRITE,st,12) == 0) return -ENACCB;
		copy_to_user(&desired->statistics,st,12);
	}
	else return -ENSPID;
	return 0;
}




