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

void cloneHello() {
	char pidc[11];
	pid = getpid();
	write(1,"Surto del clone!\n",17);
	int ret = read(0,pidc,5);
	write(1,"Lectura: ",9);
	write(1,pidc,5);
	itoa(ret,pidc);
	write(1," Size: ",7);
	write(1,pidc,strlen(pidc));
	write(1,"\n",1);
	while(1);
}


void exempleClone() {
	char stack[4][1024];
	clone(cloneHello, &stack[0][1024]);
	clone(cloneHello, &stack[1][1024]);
	clone(cloneHello, &stack[2][1024]);
	clone(cloneHello, &stack[3][1024]);
	write(1,"Pare del clone!\n",16);
	while(1);
}

void exempleFork() {
	char pidc[11];
	pid = fork();
	if (pid > 0) { // Pare
		write(1,"Soc el pare\n",12);
	}
	else if(pid == 0) {
			write(1,"Soc el fill\n",12);
			read(0,pidc,5);
			write(1,pidc,5);
	}

	while(1);
}

void exempleFC() {
	char stack[1024];
	char pidc[11];
	pid = fork();
	if (pid > 0) { // Pare
		write(1,"Soc el pare\n",12);
		clone(cloneHello, &stack[1024]);
		read(0,pidc,5);
		write(1,pidc,5);
	}
	else if(pid == 0) {
			write(1,"Soc el fill\n",12);
			read(0,pidc,5);
			write(1,pidc,5);
	}

	while(1);
}

/* Test de HEAP en unic procés */
void dinam_test() {
	char cbuff[11];
	int *pointer = 0;
	int c = 0;
	/*while ((int)pointer != -1) {
		pointer =sbrk(4096);
		++c;
	}*/
	pointer=sbrk(4097);
	//pointer=sbrk(-4097);
	/*if (-1 == (int)pointer) perror("Memoria");
	itoa(c,cbuff);
	write(1,cbuff,strlen(cbuff));*/
	pointer[0] = 1;
	pointer[1] = 2;
	pointer[2] = 3;
	pointer[3] = 4;
	pointer[4] = 5;
	while(1);
}

void semaphores_test1sub() {
	int err;
	write(1,"Clone bloquejant-me pel semaphore 0\n",36);
	err = sem_wait(0);
	if (err == -1 )perror("Sem");
	write(1,"Clone desbloquejat\n",19);
	exit();
}

void semaphores_test1() {
	char stack[4][1024];
	char pidc[11];
	sem_init(0,0);
	//sem_signal(0);
	clone(semaphores_test1sub, &stack[0][1024]);
	clone(semaphores_test1sub, &stack[1][1024]);
	write(1,"Allibero els clones si em parles: \n",35);
	read(0,pidc,5);
	write(1,pidc,5);
	sem_signal(0);
	sem_signal(0);
	write(1,"\nAlliberats!\n",13);
	while(1);
}

/* Test de HEAP & FORK */
void dinam_test2() {
	int *pointer = 0;

	pointer=sbrk(16);
	pointer[0] = 1;
	pointer[1] = 2;
	pointer[2] = 3;
	pointer[3] = 4;

	pid = fork();
	if (pid > 0) { // Pare
		pointer=sbrk(4097);
		pointer[2] = 1;
		pointer[3] = 1;
	}
	else if(pid == 0) {
		pointer=sbrk(0);
		pointer[2] = 2;
		pointer[3] = 2;
		pointer=sbrk(9000);
		pointer[2] = 3;
		pointer[3] = 4;
	}

	while(1);
}


/* Funció del thread */
void subdinam_test3() {
	int *pointer = 0;
	int pid = getpid();
	pointer=sbrk(0);
	pointer[2] = pid;
	pointer[3] = pid;

	pointer=sbrk(4097);

	while(1);
}

/* Test de HEAP & CLONE */
void dinam_test3() {
	char stack[4][1024];
	int *pointer = 0;

	pointer=sbrk(16);
	pointer[0] = 1;
	pointer[1] = 2;
	pointer[2] = 3;
	pointer[3] = 4;

	clone(subdinam_test3, &stack[0][1024]);
	clone(subdinam_test3, &stack[1][1024]);

	pointer=sbrk(4097);
	pointer[2] = 1;
	pointer[3] = 1;

	while(1);
}


void subdinam_test4() {
	sem_signal(0);
	exit();

	while(1);
}

void dinam_test4() {
	char stack[4][1024];
	int *pointer = 0;

	sem_init(0,0);
	/*
	pointer=sbrk(16);
	pointer[0] = 1;
	pointer[1] = 2;
	pointer[2] = 3;
	pointer[3] = 4;
*/

	pid = fork();
	if (pid > 0) { // Pare
		sem_wait(0);
	}
	else if(pid == 0) {
		pointer=sbrk(4096);
		clone(subdinam_test4, &stack[0][1024]);
		exit();
	}
	pointer=sbrk(4096);
	exit();

	while(1);
}

int __attribute__ ((__section__(".text.main")))
main(void) {
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
    /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

	/*int time = 0;
	char timec[11];
	char pidc[11];*/

	clearScreen();

	//semaphores_test1();
	//dinam_test();
	//dinam_test2();
	//dinam_test3();
	dinam_test4();

	//exempleClone();
	//exempleFC();

	/*read(0,pidc,5);
	write(1,pidc,5);
*/




	//clone(cloneHello, &stack[1024]);
	//pid = fork();
	//if (pid > 0) { // Pare
			//write(1,"Soc el pare\n",12);
		//	write(1,"Hola jo no vinc del clone\n",26);
	//}
	/*else if(pid == 0) {
		write(1,"Soc el fill\n",12);
		write(1,"Hola jo no vinc del clone\n",26);
	}*/
	/*
	while(1) {
		if (pid > 0) write(1,"Father\n",7);
		//else write(1,"Child\n",6);
	}
	*/

	/*
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
	*/

	while (1);
	return 0;
}
