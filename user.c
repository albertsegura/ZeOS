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
		write(1," --- RemainingQuantum: ",23);
			itoa(st.remaining_quantum,statsc);
			write(1,statsc,strlen(statsc));
		write(1," --- ClockTics: ",16);
			itoa(st.tics,statsc);
			write(1,statsc,strlen(statsc));
			write(1,"\n",1);
	}
}

void fork_test1() {
	char printed = -1;
	struct stats st;
	char pidc[11];
	pid = fork();
	if (pid > 0) { // Pare
		write(1,"\nSoc el pare",12);
		if (-1 == write(1,"\nGetPID: ",9)) perror("Error d'escriptura");
		pid = getpid();
		itoa(pid,pidc);
		if (-1 == write(1,pidc,strlen(pidc))) perror("Error d'escriptura");
		if (-1 == write(1,"\nGetStats: ",11)) perror("Error d'escriptura");
		writeStats(pid);
		exit();
	}
	else if (pid == 0) { // Fill
		write(1,"\nSoc el fill",12);
		if (-1 == write(1,"\nGetPID: ",9)) perror("Error d'escriptura");
		pid = getpid();
		itoa(pid,pidc);
		if (-1 == write(1,pidc,strlen(pidc))) perror("Error d'escriptura");
		if (-1 == write(1,"\nGetStats: ",11)) perror("Error d'escriptura");
		writeStats(pid);
		//if (-1 == write(1,"\nExit fill: ",12)) perror("Error d'escriptura");
		//exit();
	}
	else perror("Error en el fork");

	while (1) {
		pid = getpid();
		if (-1 == get_stats(pid,&st)) perror("Error en la syscall get_stats");
		if (st.remaining_quantum == 200 && st.cs < 10 && printed != st.cs) {
			printed = st.cs;
			itoa(pid,pidc);
			write(1,"\nEn execucio PID: ",18);
			write(1,pidc,strlen(pidc));
			writeStats(pid);
			if (pid == 2 && st.cs == 5) {
				if (-1 == write(1,"\nExit fill: ",12)) perror("Error d'escriptura");
				exit();
			}
		}
	}
}

/* Cronograma del test;
 * f = fork, /X = surt del bucle, (X) = pid del proces
 * it0				it1					it2
 * f -> (1) /X
 *   -> (2) -> f -> (2) /X
 * 							 -> (3) -> f -> (3) /X
 * 							 						 -> (4) /X
 *																	.... (9)
 *
 */
void fork_test2() {
	int i = 0;
	pid = 0;
	char pidc[11];
	int pidf = 0;
	for (i = 0; i<10 && pidf == 0; ++i) { // 10 aposta, per provocar perror. Aconsellabe reduir quantum
		pidf = fork();
		if (pidf > 0) { // pare
			write(1,"\nSoc el pare",12);
			if (-1 == write(1,"\nGetPID: ",9)) perror("Error d'escriptura");
			pid = getpid();
			itoa(pid,pidc);
			if (-1 == write(1,pidc,strlen(pidc))) perror("Error d'escriptura");
			if (-1 == write(1,"\nGetStats: ",11)) perror("Error d'escriptura");
			writeStats(pid);
		}
		else if (pidf == 0) {
			write(1,"\nSoc el fill",12);
			if (-1 == write(1,"\nGetPID: ",9)) perror("Error d'escriptura");
			pid = getpid();
			itoa(pid,pidc);
			if (-1 == write(1,pidc,strlen(pidc))) perror("Error d'escriptura");
			if (-1 == write(1,"\nGetStats: ",11)) perror("Error d'escriptura");
			writeStats(pid);
		}
		else perror("Error en el fork");
	}
}

int __attribute__ ((__section__(".text.main")))
main(void) {
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
    /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

	int time = 0;
	char timec[11];
	char pidc[11];
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

	//fork_test1();
	fork_test2();


	while (1);
	return 0;
}
