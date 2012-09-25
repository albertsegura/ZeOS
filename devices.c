#include <io.h>
#include <utils.h>
#include <list.h>

// Blocked queue for this device
LIST_HEAD(blocked);

int sys_write_console(char *buffer,int size)
{
  int i;
  
  for (i=0; i<size; i++)
    printc(buffer[i]);
  
  return size;
}
