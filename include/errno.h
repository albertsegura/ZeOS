#ifndef __ERRNO__
#define __ERRNO__

extern int errno;

#define EBADF 1 /* Bad file number */
#define EACCES 2 /* Permission denied */
#define ENOSYS 3 /* Function not implemented */
#define EPNULL 4 /* Pointer is null */
#define ESIZEB 5 /* Size is not positive */
#define ENACCB 6 /* Can't access pointer */
#define ENTASK 7 /* There are no more free tasks */
#define ENMPHP 8 /* There are no more free physical pages */
#define ENSPID 9 /* There is no task with the specified pid */
#define ENEPTE 10 /* There are no enough page table entries for the process */
#define EINVSN 11 /* Invalid semaphore number */
#define ESINIT 12 /* Semaphore already initialised */
#define ENINIT 13 /* Semaphore not initialised */
#define ESDEST 14 /* Blocked in a destroyed semaphore */
#define ESNOWN 15 /* Not the owner of the semaphore */
#define ENOMEM 16 /* Not enough free memory in the heap */
#define EHLIMI 17 /* Heap limit reached */

#endif

