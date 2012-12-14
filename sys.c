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
#include <cbuffer.h>

#define LECTURA 0
#define ESCRIPTURA 1

int check_fd(int fd, int permissions)
{
  if (fd != 1 && fd != 0) return -EBADF;
  if (fd == 1 && permissions!=ESCRIPTURA) return -EACCES;
  if (fd == 0 && permissions!=LECTURA) return -EACCES;
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
	/*struct list_head *new_list_task = list_first(&readyqueue);
	list_del(new_list_task);
	struct task_struct * new_task = list_head_to_task_struct(new_list_task);
	struct task_struct * current_task = current();

	list_add_tail(&current_task->list,&readyqueue);

	task_switch((union task_union*)new_task);*/

	sched_update_queues_state(&readyqueue,current());
	sched_switch_process();

	return 0;
}


int sys_clone (void (*function)(void), void *stack) {
	int PID;
	int current_ebp = 0;
	unsigned int pos_ebp = 0; /* posició del ebp en la stack: new/current_stack->stack[pos_ebp] */

	/* Obtenció d'una task_struct nova de la freequeue */
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


	/* Copia del Stack, actualització del contador de punters del directori*/
	copy_data(current_pcb, new_pcb, 4096);
	*(new_pcb->dir_count) += 1;
	*(new_pcb->pb_count) += 1;

	PID = getNewPID();
	new_pcb->PID = PID;

	/* Construint l'enllaç dinàmic fent que el esp apunti al ebp guardat */
	new_stack->task.kernel_esp = (unsigned int)&new_stack->stack[pos_ebp];
	/* @ retorn estàndard: Restore ALL + iret */
	new_stack->stack[pos_ebp+1] = (unsigned int)&ret_from_fork;
	/* Modificació del ebp amb la @ de la stack */
	new_stack->stack[pos_ebp+7] = (unsigned int)stack;

	// |	eip	|
	// |	es	|
	// |eflags|
	// |	esp	|
	// |	ss	|

	/* Modificació del registre eip que restaurarà el iret */
	new_stack->stack[pos_ebp+13] = (unsigned int)function;
	/* Modificació del registre esp per fer a la @stack: push   %ebp 	*/
	new_stack->stack[pos_ebp+16] = (unsigned int)stack;

	/* Inicialització estadistica */
	new_stack->task.process_state = ST_READY;
	new_stack->task.statistics.tics = 0;
	new_stack->task.statistics.cs = 0;

	/* El posem en la pila de ready per a la seva execució en un futur */
	sched_update_queues_state(&readyqueue,new_pcb);

	return PID;
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
	//page_table_entry * copy_dir_pages_baseAddr = get_DIR(new_pcb);
	copy_data(current_pcb, new_pcb, 4096);
	allocate_page_dir(new_pcb);
	//new_pcb->dir_pages_baseAddr = copy_dir_pages_baseAddr;

	/* Punt d.i: Copia de les page tables de codi, i assignació de
	 * 						frames per a les dades	*/
	page_table_entry * pt_new = get_PT(new_pcb);
	page_table_entry * pt_current = get_PT(current_pcb);
	page_table_entry * dir_current = get_DIR(current_pcb);

	/* CODE */
	for (pag=0;pag<NUM_PAG_CODE;pag++){
		pt_new[PAG_LOG_INIT_CODE_P0+pag].entry = pt_current[PAG_LOG_INIT_CODE_P0+pag].entry;
	}

	/* DATA */
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


	/* TRACTAMENT DEL HEAP */

	get_newpb(new_pcb);
	*(new_pcb->program_break) = *(current_pcb->program_break);

	/* Copia de la zona HEAP, similar a la copia de pagines de dades */
	free_pag = (*(current_pcb->program_break)>>12)+1;
	for (pag=HEAPSTART; pag < (*(current_pcb->program_break)>>12) ||
					(pag == (*(current_pcb->program_break)>>12) && 0 != *(current_pcb->program_break)%PAGE_SIZE);pag++) {
			while(pt_current[free_pag].entry != 0 && free_pag<TOTAL_PAGES) free_pag++;

			if (free_pag == TOTAL_PAGES) {
				if (pag != 0) {
					free_pag = (*(current_pcb->program_break)>>12)+1;
					--pag;
					set_cr3(dir_current);
				}
				else return -ENEPTE;
			}
			else {
				/* Obtenció del frame pel heap del fill */
				new_ph_pag=alloc_frame();
				if (new_ph_pag == -1)	{
					pag--;
					while(pag >= HEAPSTART) free_frame(pt_new[pag--].bits.pbase_addr);
					return -ENMPHP;
				}

				/* Assignació del frame nou, al procés fill */
				set_ss_pag(pt_new,pag,new_ph_pag);

				/* Copia del Heap: es necessari posar el frame en el pt del pare per a fer-ho */
				set_ss_pag(pt_current,free_pag,pt_new[pag].bits.pbase_addr);
				copy_data((void *)((pag)<<12),	(void *)(free_pag<<12), PAGE_SIZE);
				del_ss_pag(pt_current, free_pag);

				free_pag++;
			}
	}
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
	sched_update_queues_state(&readyqueue,new_pcb);

	return PID;
}

void sys_exit() {
	/* Punt a */
	int pag;
	struct task_struct * current_pcb = current();
	page_table_entry * pt_current = get_PT(current_pcb);

	/* Allibera les 20 de Data i la resta (HEAP), a partir de l'entrada 256+8 */
	if (*(current_pcb->dir_count) == 1) {
		for (pag=PAG_LOG_INIT_DATA_P0;pag<TOTAL_PAGES ;pag++){
			free_frame(pt_current[pag].bits.pbase_addr);
		}
	}
	*(current_pcb->dir_count) -= 1;
	*(current_pcb->pb_count) -= 1;

	/* Punt b */
	sched_update_queues_state(&freequeue,current());
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
		ret += sys_write_console(buff,4);
		buffer += 4;
		size -= 4;
	}
	copy_from_user(buffer, buff, size);
	ret += sys_write_console(buff,size);

	return ret;
}

int sys_read(int fd, char * buffer, int size) {
	int ret = 0;

	ret = check_fd(fd,LECTURA);
	if (ret != 0) return ret;
  if (buffer == NULL) return -EPNULL;
	if (size <= 0) return -ESIZEB;
	if (access_ok(VERIFY_WRITE, buffer, size) == 0) return -ENACCB;

	ret = sys_read_keyboard(buffer,size);

	return ret;
}


int sys_gettime() {
	return zeos_ticks;
}

int sys_get_stats(int pid, struct stats *st) {
	struct task_struct * desired;
	int found;
	if (access_ok(VERIFY_WRITE,st,12) == 0) return -ENACCB;

	found = getStructPID(pid, &readyqueue, &desired);
	if (!found) found = getStructPID(pid, &keyboardqueue, &desired);
	if (found) copy_to_user(&desired->statistics,st,12);

	else return -ENSPID;
	return 0;
}


/* SEMAPHORES */
int sys_sem_init(int n_sem, unsigned int value) {
	int ret = 0;

	if (n_sem < 0 || n_sem >= SEM_SIZE) return -EINVSN;
	if (sem_array[n_sem].pid_owner != -1) return -ESINIT;
	sem_array[n_sem].pid_owner = current()->PID;
	sem_array[n_sem].value = value;
	INIT_LIST_HEAD(&sem_array[n_sem].semqueue);
	return ret;
}

int sys_sem_wait(int n_sem) {
	int ret = 0;
	if (n_sem < 0 || n_sem >= SEM_SIZE) return -EINVSN;
	if (sem_array[n_sem].pid_owner == -1) return -ENINIT;
	if (sem_array[n_sem].value <= 0) {
		sched_update_queues_state(&sem_array[n_sem].semqueue,current());
		sched_switch_process();
	}
	else sem_array[n_sem].value--;

	/* Hem de mirar si després de l'espera en la cua
	 * s'ha destruit el semafor o no, per indicar-ho al usuari*/
	if (sem_array[n_sem].pid_owner == -1) ret = -ESDEST;

	return ret;
}

int sys_sem_signal(int n_sem) {
	int ret = 0;

	if (n_sem < 0 || n_sem >= SEM_SIZE) return -EINVSN;
	if (sem_array[n_sem].pid_owner == -1) return -ENINIT;
	if(list_empty(&sem_array[n_sem].semqueue)) sem_array[n_sem].value++;
	else {
		struct list_head *task_list = list_first(&sem_array[n_sem].semqueue);
		list_del(task_list);
		struct task_struct * semtask = list_head_to_task_struct(task_list);
		sched_update_queues_state(&readyqueue,semtask);
	}

	return ret;
}

int sys_sem_destroy(int n_sem) {
	int ret = 0;

	if (n_sem < 0 || n_sem >= SEM_SIZE) return -EINVSN;
	if (sem_array[n_sem].pid_owner == -1) return -ENINIT;
	if (current()->PID == sem_array[n_sem].pid_owner) {
		sem_array[n_sem].pid_owner = -1;
		while (!list_empty(&sem_array[n_sem].semqueue)) {
			struct list_head *task_list = list_first(&sem_array[n_sem].semqueue);
			list_del(task_list);
			struct task_struct * semtask = list_head_to_task_struct(task_list);
			sched_update_queues_state(&readyqueue,semtask);
		}
	}
	else ret = -ESNOWN;

	return ret;
}

void *sys_sbrk(int increment) {
	int i;
	struct task_struct * current_pcb = current();
	void * ret  = (void *)*(current_pcb->program_break);

	page_table_entry * pt_current = get_PT(current_pcb);

	if (increment > 0) {
		int end = (*(current_pcb->program_break)+increment)>>12;
		if (end < TOTAL_PAGES) { /* Limit inferior del HEAP */
			for(i = (*(current_pcb->program_break)>>12);
						i < end || (i==end && 0!=(*(current_pcb->program_break)+increment)%PAGE_SIZE); ++i) {
				if (pt_current[i].entry == 0) {
					int new_ph_pag=alloc_frame();
					if (new_ph_pag == -1) {
						free_frame(new_ph_pag);
						return (void *)-ENMPHP;
					}
					set_ss_pag(pt_current,i,new_ph_pag);
				}
			}
		}
		else 	return (void *)-ENOMEM;
	}
	else if (increment < 0) {
		page_table_entry * dir_current = get_DIR(current_pcb);
		int new_pb = (*(current_pcb->program_break)+increment)>>12;

		if (new_pb >= HEAPSTART) { /* Limit superior del HEAP */
			for(i = (*(current_pcb->program_break)>>12);
						i > new_pb || (i==new_pb && 0==(*(current_pcb->program_break)+increment)%PAGE_SIZE); --i) {
				free_frame(pt_current[i].bits.pbase_addr);
				del_ss_pag(pt_current, i);
			}
			set_cr3(dir_current);
		}
		else return (void *)-EHLIMI;
	}
	*(current_pcb->program_break) += increment;

	return ret;
}


