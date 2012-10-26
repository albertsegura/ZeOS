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

#endif

