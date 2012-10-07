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
#include <io.h> // TODO DEBUG

#define LECTURA 0
#define ESCRIPTURA 1

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -1; /*EBADF*/
  if (permissions!=ESCRIPTURA) return -2; /*EACCES*/
  return 0;
}

int sys_ni_syscall()
{
	return -3; /*ENOSYS*/
}

int sys_getpid()
{
	printint(current()->dir_pages_baseAddr->bits.pbase_addr);
	return current()->PID;
}

int sys_fork()
{
  int PID=-1;

  // creates the child process
  
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
  	if (buffer == NULL) return -4;
	if (size <= 0) return -5;
	if (access_ok(VERIFY_READ, buffer, size) == 0) return -6;

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





