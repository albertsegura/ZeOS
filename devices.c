#include <io.h>
#include <utils.h>
#include <list.h>
#include <mm_address.h>
#include <system.h>
#include <sched.h>
#include <cbuffer.h>

// Blocked queue for this device
LIST_HEAD(blocked);

int sys_write_console(char *buffer,int size)
{
  int i;
  
  for (i=0; i<size; i++)
    printc(buffer[i]);
  
  return size;
}

int sys_read_keyboard(char *buffer, int size) {
	struct task_struct * current_pcb = current();
	char read;

	/* Si hi han procesos bloquejats, es posa a la cua */
	if (!list_empty(&keyboardqueue)) {
		current_pcb->kbinfo.keystoread = size;
		current_pcb->kbinfo.keybuffer = buffer;
		current_pcb->kbinfo.keysread = 0;
		sched_update_queues_state(&keyboardqueue,current());
		sched_switch_process();
	}
	else {
		/* Si hi han dades suficients per proporcionar-li
		 * al procés, es copien les dades					*/
		if (size <= circularbNumElements(&cbuffer)) {
			current_pcb->kbinfo.keystoread = 0;
			current_pcb->kbinfo.keysread = size;
			while (size > 0) {
				circularbRead(&cbuffer,&read);
				copy_to_user(&read, buffer++, 1);
				--size;
			}
		}
		else { /* Si no hi han dades suficients */
			current_pcb->kbinfo.keystoread = size;
			current_pcb->kbinfo.keysread = 0;
			/* Si el buffer està ple es buida */
			if (circularbIsFull(&cbuffer)) {
				current_pcb->kbinfo.keystoread -= CBUFFER_SIZE;
				current_pcb->kbinfo.keysread = CBUFFER_SIZE;
				while (!circularbIsEmpty(&cbuffer)) {
					circularbRead(&cbuffer,&read);
					copy_to_user(&read, buffer++, 1);
				}
			}
			current_pcb->kbinfo.keybuffer = buffer;

			/* Es bloqueja i es canvia de procés*/

			sched_update_queues_state(&keyboardqueue,current_pcb);
			sched_switch_process();

		}
	}

	return current_pcb->kbinfo.keysread;
}
