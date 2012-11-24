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
	char read;

	/* Si hi han procesos bloquejats, es posa a la cua */
	if (!list_empty(&keyboardqueue)) {
		sched_update_queues_state(&keyboardqueue,current());
		sched_switch_process();
	}
	else {

		/* Si hi han dades suficients per proporcionar-li
		 * al procés, es copien les dades					*/
		if (size <= circularbNumElements(&cbuffer)) {
			while (size > 0) {
				circularbRead(&cbuffer,&read);
				copy_to_user(&read, buffer++, 1);
				--size;
			}
		}
		else { /* Si no hi han dades suficients */
			keystoread = size;

			/* Si el buffer està ple es buida */
			if (circularbIsFull(&cbuffer)) {
				keystoread -= CBUFFER_SIZE;
				while (!circularbIsEmpty(&cbuffer)) {
					circularbRead(&cbuffer,&read);
					copy_to_user(&read, buffer++, 1);
				}
			}
			keybuffer = buffer; // S'actualitza el buffer global

			/* Es bloqueja i es canvia de procés*/
			sched_update_queues_state(&keyboardqueue,current());
			sched_switch_process();
		}
	}

	return 0;
}
