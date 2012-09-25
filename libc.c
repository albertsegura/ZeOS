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

int write (int fd, char *buffer, int size) {
	/* Wrapper de la Syscall Write */
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
	return ret;
}

int gettime(int *time) {
	/* Wrapper de la Syscall Gettime */
		int ret;
	__asm__ volatile( 
					"int $0x80"
				:"=a" (ret), 		// resultat de %eax a ret
				"+b" (time)
				:"a"  (10)
				);
	if (ret < 0) {
		errno = -ret;
		ret = -1;
	}
	return ret;
	
	

}


