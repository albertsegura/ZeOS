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

  if (list_empty(&freequeue)) return -1; // TODO Crear errno
  struct list_head *new_list_pointer = list_first(&freequeue);
	list_del(new_list_pointer);
	struct task_struct * new_pcb = list_head_to_task_struct(new_list_pointer);
	struct task_struct * current_pcb = current();

	union task_union *new_union_stack = (union task_union*)new_pcb;

	int frame = alloc_frame();
	if (frame == -1) return -1; // TODO Crear errno
	/* Copia del Stack */
	copy_data(current_pcb, new_pcb, 4096);

	/* punt d.i: copia de les page tables corresponents*/
	page_table_entry * pt_new = get_PT(new_pcb);
	page_table_entry * pt_current = get_PT(current_pcb);

	int pag;
	int new_ph_pag;
	 /* CODE */
	for (pag=0;pag<NUM_PAG_CODE;pag++){ /*Inecessari, la copia ja es igual*/
		pt_new[PAG_LOG_INIT_CODE_P0+pag].entry =
				pt_current[PAG_LOG_INIT_CODE_P0+pag].entry;
	}

	/* DATA */
	for (pag=0;pag<NUM_PAG_DATA;pag++){
		new_ph_pag=alloc_frame();
		pt_new[PAG_LOG_INIT_DATA_P0+pag].entry = 0;
		pt_new[PAG_LOG_INIT_DATA_P0+pag].bits.pbase_addr = new_ph_pag;
		pt_new[PAG_LOG_INIT_DATA_P0+pag].bits.user = 1;
		pt_new[PAG_LOG_INIT_DATA_P0+pag].bits.rw = 1;
		pt_new[PAG_LOG_INIT_DATA_P0+pag].bits.present = 1;
	}


	/* punt d.ii */
	int first_free_pag; // TODO Determinar
	// TODO WARRNING pbase_addr es el numero no la @, ARREGLAR
	for (pag=0;pag<NUM_PAG_DATA;pag++){
		set_ss_pag(pt_current, first_free_pag+pag,pt_new[PAG_LOG_INIT_DATA_P0+pag].bits.pbase_addr);
	}
	copy_data(pt_current[PAG_LOG_INIT_DATA_P0].bits.pbase_addr,
			pt_current[first_free_pag].bits.pbase_addr, 4*1024*NUM_PAG_DATA);
	for (pag=0;pag<NUM_PAG_DATA;pag++){
		del_ss_pag(pt_current, first_free_pag+pag);
	}
	set_cr3(pt_current);

	/* punt e */
	PID = getNewPID();
	new_pcb->PID = PID;

  
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





