/*
 * libc.c 
 */

#include <libc.h>

#include <types.h>
#include <errno.h>

void itoa(int a, char *b)
{
  int i, i1;
  char c;
  
  if (a==0) { b[0]='0'; b[1]=0; return ;}
  
  i=0;
  while (a>0)
  {
    b[i]=(a%10)+'0';
    a=a/10;
    i++;
  }
  
  for (i1=0; i1<i/2; i1++)
  {
    c=b[i1];
    b[i1]=b[i-i1-1];
    b[i-i1-1]=c;
  }
  b[i]=0;
}

int strlen(char *a)
{
  int i;
  
  i=0;
  
  while (a[i]!=0) i++;
  
  return i;
}

/* Wrapper de la Syscall Write */
int write (int fd, char *buffer, int size) {
	int ret;
	__asm__ volatile( 
				"int $0x80"
				:"=a" (ret), 		// resultat de %eax a ret
				"+b" (fd), 
				"+c" (buffer),
				"+d" (size)
				:"a"  (4)
	);
	if (ret < 0) {
		errno = -ret;
		ret = -1;
	}
	else ret = size;
	return ret;
}

/* Wrapper de la Syscall Gettime */
int gettime() {
	int ret;
	__asm__ volatile( 
				"int $0x80"
				:"=a" (ret) 		// resultat de %eax a ret
				:"a"  (10)
	);
	return ret;
}

/* Wrapper de la Syscall GetPid */
int getpid(void) {
	int ret;
	__asm__ volatile(
				"int $0x80"
				:"=a" (ret) 		// resultat de %eax a ret
				:"a"  (20)
	);
	return ret;
}

/* Wrapper de la Syscall Fork */
int fork() {
	int ret;
	__asm__ volatile(
				"int $0x80"
				:"=a" (ret)
				:"a"  (2)
	);
	if (ret < 0) {
		errno = -ret;
		ret = -1;
	}
	return ret;
}


/* Wrapper de la Syscall de Debug sys_DEBUG_tswitch */
int debug_task_switch() {
	int ret;
	__asm__ volatile(
				"int $0x80"
				:"=a" (ret)
				:"a"  (9)
	);
	return ret;
}

/* Wrapper de la Syscall Exit */
void exit() {
	__asm__ volatile(
				"int $0x80"
				:
				:"a"  (1)
	);
}

/* Wrapper de la Syscall get_stats */
int get_stats(int pid, struct stat *st) {
	int ret;
	__asm__ volatile(
				"int $0x80"
				:"=a" (ret),
				"+b" (pid),
				"+c" (st),
				:"a" (35)
	);
	return ret;
}
