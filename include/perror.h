#ifndef __PERROR_H__
#define __PERROR_H__

#include <errno.h>
#include <libc.h>

extern char *sys_errlist[];

extern int sys_nerr;

void perror(char *s);

#endif
