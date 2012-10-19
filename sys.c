/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

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



int sys_fork()
{
  int PID=-1;

  int current_ebp = 0;
  int new_ebp = 0;
  unsigned int pos_ebp = 0; // posició del ebp en la stack: new/current_stack->stack[pos_ebp]

  /* Pag 36 Punt a: Obtenció d'una task_struct nova de la freequeue */
  if (list_empty(&freequeue)) return -1; // TODO Crear errno

  struct list_head *new_list_pointer = list_first(&freequeue);
	list_del(new_list_pointer);
	struct task_struct * new_pcb = list_head_to_task_struct(new_list_pointer);
	struct task_struct * current_pcb = current();
	union task_union *new_stack = (union task_union*)new_pcb;
	union task_union *current_stack = (union task_union*)current_pcb;

	/* Càlcul de pos_ebp */
	__asm__ __volatile__(
	  		"mov %%ebp,%0;"
	  		: "=r" (current_ebp)
	  		:
	  );
	pos_ebp = ((unsigned int)current_ebp-(unsigned int)current_pcb)/4;

	/*// Prints per veure les diferencies entre el ebp i la base de la pila en els processos.
	printint((unsigned int)current_ebp); printc('-'); printint((unsigned int)current_pcb); printc('=');
	printint((unsigned int)current_ebp-(unsigned int)current_pcb);
	printc('\n');
	new_ebp = ((unsigned int)current_ebp-(unsigned int)current_pcb)+(unsigned int)new_pcb;
	printint(new_ebp); printc('-');	printint((unsigned int)new_pcb); printc('=');
		printint((unsigned int)new_ebp-(unsigned int)new_pcb);
	*/

	/* Pag 36 Punt b: Obtenció de pàgines físiques */
	int frame = alloc_frame();
	if (frame == -1) return -1; // TODO Crear errno

	/* Pag 36 Punt c: Copia del Stack */
	copy_data(current_pcb, new_pcb, 4096);

	/* Pag 36 Punt d.i: Copia de les page tables corresponents*/
	page_table_entry * pt_new = get_PT(new_pcb);
	page_table_entry * pt_current = get_PT(current_pcb);
	page_table_entry * dir_current = get_DIR(current_pcb);

	int pag;
	int new_ph_pag;

	/* CODE */
	/* Inecessari, la copia ja es igual
	for (pag=0;pag<NUM_PAG_CODE;pag++){
		pt_new[PAG_LOG_INIT_CODE_P0+pag].entry =
				pt_current[PAG_LOG_INIT_CODE_P0+pag].entry;
	}
	*/

	/* DATA */
	for (pag=0;pag<NUM_PAG_DATA;pag++){
		new_ph_pag=alloc_frame();
		set_ss_pag(pt_new,PAG_LOG_INIT_DATA_P0+pag,new_ph_pag);
	}


	/* Pag 36 Punt d.ii */
		/* d.ii.A: Assignació de noves pàgines logiques al procés actual,
		 * 					de les pàgines físiques obtingudes per al procés nou	*/
	int first_free_pag = NUM_PAG_KERNEL + NUM_PAG_CODE + NUM_PAG_DATA;
	for (pag=0;pag<NUM_PAG_DATA;pag++){
		set_ss_pag(pt_current, first_free_pag+pag,pt_new[PAG_LOG_INIT_DATA_P0+pag].bits.pbase_addr);
	}

		/* d.ii.B: Copia de l'espai d'usuari del proces actual al nou */
	copy_data((void *)(PAG_LOG_INIT_DATA_P0<<12),	(void *)(first_free_pag<<12), 4096*NUM_PAG_DATA);

		/* d.ii.C: Desassignació de les pagines en el procés actual, i flush */
	for (pag=0;pag<NUM_PAG_DATA;pag++){
		del_ss_pag(pt_current, first_free_pag+pag);
	}
	set_cr3(dir_current);

	/* Pag 36 Punt e */
	PID = getNewPID();
	new_pcb->PID = PID;

	/* Pag 36 Punt f */
	new_stack->stack[pos_ebp+7] = 0; //eax del Save_all
	new_stack->task.kernel_esp = &new_stack->stack[pos_ebp+2]; // Apunta al ultim element guardat pel save_all
	new_stack->stack[pos_ebp+1] = &ret_from_fork;
 /* __asm__ __volatile__(
  		"mov %%ebp,(%0);"
  		"mov %1,%%esp;"
  		"pop %%ebp;"
  		"ret;"
  		: /* no output */
  	/*	: "r" (kernel_esp), "r" (new_kernel_esp)
  );
	// TODO eax pid, @ret ebp

	/* Pag 36 Punt g */
  

	/* Pag 36 Punt h */
	list_add_tail(&new_pcb->list,&readyqueue);

  return PID;
}

void sys_exit()
{

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





