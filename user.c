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
	int pid;
	char timec[11];
	char pidc[11];
	
	int ret = write(1,"Hola mon!",9);
	if (ret == -1) perror("Error d'escriptura");
	
	time = gettime();
	ret = write(1," Gettime: ",10);
	if (ret == -1) perror("Error d'escriptura");
	itoa(time,timec);
	ret = write(1,timec,strlen(timec));
	if (ret == -1) perror("Error d'escriptura");
	
	/*	long count, acum;
	count = 75;
	acum = 0;
	acum = outer(count);
	int resul = add(4,2);*/
	
	write(1," NUM: ",6);
	ret = getpid();
	itoa(ret,pidc);
	write(1," PID: ",6);
	write(1,pidc,strlen(pidc));


	write(1,"\n",1);
	pid = fork();
	if (pid != 0) { // Pare
		write(1,"Soc el pare\n",12);
		debug_task_switch();
	}
	else {
		write(1,"Soc el fill\n",12);
	}

	ret = getpid();
	itoa(ret,pidc);
	write(1," PID: ",6);
	write(1,pidc,strlen(pidc));

	while (1);
	return 0;
}
