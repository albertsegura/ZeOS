#ifndef _SEMAPHORE
#define _SEMAPHORE

#include <list.h>

typedef struct {
		int id;
		unsigned int value;
		int pid_owner;
		struct list_head semqueue;
}Sem;

#endif /* _SEMAPHORE */
