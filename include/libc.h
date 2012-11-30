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

int get_stats(int pid, struct stats *st);

int clone (void (*function)(void), void *stack);

int read (int fd, char *buffer, int size);

int sem_init (int n_sem, unsigned int value);

int sem_wait (int n_sem);

int sem_signal (int n_sem);

int sem_destroy (int n_sem);

void *sbrk (int increment);

#endif  /* __LIBC_H__ */
