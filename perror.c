#include <perror.h>

#include <types.h>


char *sys_errlist[] = {

/*	EZERO 0			*/ "No error condition exists",
/*	EBADF 1			*/ "Bad file number",
/*	EACCES 2		*/ "Permission denied",
/*	ENOSYS 3		*/ "Function not implemented",
/*	EPNULL 4		*/ "Pointer is null",
/*	ESIZEB 5		*/ "Size is not positive",
/*	ENACCB 6		*/ "Can't access pointer"

};

int sys_nerr = 6; // Max number

void perror(char *s) {
	char *cp = sys_errlist[errno];
	if (errno < 0 || errno > sys_nerr) cp = "Unknown error";
	if (s != NULL) {
		write(1,s,strlen(s));
		write(1,": ",2);
	}
	write(1,cp,strlen(cp));
	
}
