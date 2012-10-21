/*
 * libc.h - macros per fer els traps amb diferents arguments
 *          definició de les crides a sistema
 */
 
#ifndef __LIBC_H__
#define __LIBC_H__

#include <stats.h>

int write(int fd, char *buffer, int size);

int gettime();

void itoa(int a, char *b);

int strlen(char *a);

int getpid();

int fork();

int debug_task_switch();

void exit();

#endif  /* __LIBC_H__ */
