#include <libc.h>

#include <perror.h>


char buff[24];

int pid;


long inner(long n) {
	int i;
	long suma;
	suma = 0;
	for (i=0; i<n; i++) suma = suma + i;
	return suma;
}

long outer(long n) {
	int i;
	long acum;
	acum = 0;
	for (i=0; i<n; i++) acum = acum + inner(i);
	return acum;
}

/*
int add(int par1,int par2) {
	__asm__ ( "movl 12(%ebp), %eax;"
              "addl 8(%ebp), %eax;"
    );
}
*/

int __attribute__ ((__section__(".text.main")))
main(void) {
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

	int time = 0;
	char timec[11];
	
	int ret = write(1,"Hola mon!",9);
	if (ret == -1) perror("Error d'escriptura");
	
	ret = gettime(&time);
	if (ret == -1) {
		perror("Error al obtenir el temps");
	}
	else {	
		write(1," Gettime: ",10);
		itoa(time,timec);
		write(1,timec,strlen(timec));
	}
	
	/*	long count, acum;
	count = 75;
	acum = 0;
	acum = outer(count);
	int resul = add(4,2);*/
	
	while (1);
	return 0;
}
