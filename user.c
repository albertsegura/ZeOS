#include <libc.h>

#include <perror.h>


char buff[24];

int pid;




int __attribute__ ((__section__(".text.main")))
main(void) {
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
    /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

	int time = 0;
	char timec[11];
	char pidc[11];
	struct stats st;
	
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
		//debug_task_switch();
	}
	else {
		write(1,"Soc el fill\n",12);
		exit();
	}

	ret = get_stats(1,&st);
	//ret = getpid();
	itoa(ret,pidc);
	if (ret == -1) perror("Error de stats");
	itoa(st.cs,pidc);
	write(1,pidc,strlen(pidc));
	write(1,"\n",1);
	itoa(st.remaining_quantum,pidc);
	write(1,pidc,strlen(pidc));
	write(1,"\n",1);
	itoa(st.tics,pidc);
	write(1,pidc,strlen(pidc));
	write(1,"\n",1);

	//write(1," PID: ",6);
	//write(1,pidc,strlen(pidc));
	while (1) {
		ret = getpid();
		//itoa(ret,pidc);
		//write(1,"\nPID: ",6);
		//write(1,pidc,strlen(pidc));
		//debug_task_switch();
	}



	while (1);
	return 0;
}
