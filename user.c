#include <libc.h>

#include <perror.h>


char buff[24];

int pid;

void clearScreen() {
	int i, j;
	write(1,"\n",1);
	for (i=0;i<33;++i) {
		for (j=0;j<80;++j) {
			write(1," ",1);
		}
	}
}

void writeStats(int pid) {
	char statsc[11];
	struct stats st;

	if (-1 == get_stats(pid,&st)) perror("Error en la syscall get_stats");
	else {
		write(1,"\n--- ContextSwitch: ",20);
			itoa(st.cs,statsc);
			write(1,statsc,strlen(statsc));
		write(1,"\n--- RemainingQuantum: ",23);
			itoa(st.remaining_quantum,statsc);
			write(1,statsc,strlen(statsc));
		write(1,"\n--- ClockTics: ",16);
			itoa(st.tics,statsc);
			write(1,statsc,strlen(statsc));
			write(1,"\n",1);
	}
}


int __attribute__ ((__section__(".text.main")))
main(void) {
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
    /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

	char printed = -1;
	int time = 0;
	char timec[11];
	char pidc[11];
	char statsc[11];
	struct stats st;
	clearScreen();
	if (-1 == write(1,"\n*****  ZEOS SO task1: Hola mon!  *****\n",40)) perror("Error d'escriptura");
	if (-1 == write(1,"Execucio de Syscalls:\n",22)) perror("Error d'escriptura");
	if (-1 == write(1,"GetTime: ",9)) perror("Error d'escriptura");
	time = gettime();
	itoa(time,timec);
	if (-1 == write(1,timec,strlen(timec))) perror("Error d'escriptura");
	if (-1 == write(1,"\nGetPID: ",9)) perror("Error d'escriptura");
	pid = getpid();
	itoa(pid,pidc);
	if (-1 == write(1,pidc,strlen(pidc))) perror("Error d'escriptura");
	if (-1 == write(1,"\nGetStats: ",11)) perror("Error d'escriptura");
	writeStats(1);
	if (-1 == write(1,"\nFork: ",7)) perror("Error d'escriptura");
	
	pid = fork();
	if (pid != 0) { // Pare
		write(1,"\nSoc el pare",12);
		if (-1 == write(1,"\nGetPID: ",9)) perror("Error d'escriptura");
		pid = getpid();
		itoa(pid,pidc);
		if (-1 == write(1,pidc,strlen(pidc))) perror("Error d'escriptura");
		if (-1 == write(1,"\nGetStats: ",11)) perror("Error d'escriptura");
		writeStats(1);
	}
	else {
		write(1,"\nSoc el fill",12);
		if (-1 == write(1,"\nGetPID: ",9)) perror("Error d'escriptura");
		pid = getpid();
		itoa(pid,pidc);
		if (-1 == write(1,pidc,strlen(pidc))) perror("Error d'escriptura");
		if (-1 == write(1,"\nGetStats: ",11)) perror("Error d'escriptura");
		writeStats(1);
		//if (-1 == write(1,"\nExit fill: ",12)) perror("Error d'escriptura");
		//exit();
	}

	while (1) {
		pid = getpid();
		if (-1 == get_stats(pid,&st)) perror("Error en la syscall get_stats");
		if (st.remaining_quantum == 200 && st.cs < 10 && printed != st.cs) {
			printed = st.cs;
			itoa(pid,pidc);
			write(1,"\nEn execucio PID: ",18);
			write(1,pidc,strlen(pidc));
			if (pid == 2 && st.cs == 3) {
				if (-1 == write(1,"\nExit fill: ",12)) perror("Error d'escriptura");
				exit();
			}
		}
		//debug_task_switch();
	}



	while (1);
	return 0;
}
