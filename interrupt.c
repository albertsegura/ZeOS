/*
 * interrupt.c -
 */
#include <types.h>
#include <interrupt.h>
#include <segment.h>
#include <hardware.h>
#include <io.h>

#include <zeos_interrupt.h>
#include <system.h>
#include <sched.h>
#include <mm_address.h>
#include <utils.h>
#include <cbuffer.h>

Gate idt[IDT_ENTRIES];
Register    idtR;



char char_map[] =
{
  '\0','\0','1','2','3','4','5','6',
  '7','8','9','0','\'','¡','\0','\0',
  'q','w','e','r','t','y','u','i',
  'o','p','`','+','\0','\0','a','s',
  'd','f','g','h','j','k','l','ñ',
  '\0','º','\0','ç','z','x','c','v',
  'b','n','m',',','.','-','\0','*',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','7',
  '8','9','-','4','5','6','+','1',
  '2','3','0','\0','\0','\0','<','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0'
};

void setInterruptHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
  /***********************************************************************/
  /* THE INTERRUPTION GATE FLAGS:                          R1: pg. 5-11  */
  /* ***************************                                         */
  /* flags = x xx 0x110 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);
  flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}

void setTrapHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
  /***********************************************************************/
  /* THE TRAP GATE FLAGS:                                  R1: pg. 5-11  */
  /* ********************                                                */
  /* flags = x xx 0x111 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);

  //flags |= 0x8F00;    /* P = 1, D = 1, Type = 1111 (Trap Gate) */
  /* Changed to 0x8e00 to convert it to an 'interrupt gate' and so
     the system calls will be thread-safe. */
  flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}


void setIdt()
{
  /* Program interrupts/exception service routines */
  idtR.base  = (DWord)idt;
  idtR.limit = IDT_ENTRIES * sizeof(Gate) - 1;
  /* ADD INITIALIZATION CODE FOR INTERRUPT VECTOR (IDT)*/

  set_handlers();
  
  setInterruptHandler(32, clock_handler, 0);
  setInterruptHandler(33, keyboard_handler, 0);
  
  set_idt_reg(&idtR);
}

void clock_routine() {
	
	zeos_ticks++;
	zeos_show_clock();
	sched_update_data();
	if (sched_change_needed()) {
		sched_update_queues_state(&readyqueue, current());
		sched_switch_process();
	}
}

void keyboard_routine() {

	struct list_head *task_list;
	struct task_struct *taskbloqued;
	char bread;
	unsigned char read = inb(0x60); // Lectura del Port
	char keyPressed;
	
	/* Primer bit indica Make(0)/Break(1) => key (Pressed/Relased)*/
	if (read < 0x80){ // Make
		
		if (read < 98) {
			keyPressed = char_map[read]; // Conversió de la lectura a char
			if (keyPressed == '\0') keyPressed = 'C';
		}
		else keyPressed = 'C';

		/* Si el Buffer Circular no està ple */
		if (!circularbIsFull(&cbuffer)) {
			circularbWrite(&cbuffer,&keyPressed);

			/* Si hi ha algun element bloquejat actuem, sino només hem d'escriure */
			if (!list_empty(&keyboardqueue)) {

				/* Si tenim just el que es volia llegir:
				 * 	- Proporcionem totes les dades del Cbuffer
				 * 		al buffer del usuari
				 * 	- Desbloquejem el procés bloquejat -> readyqueue
				 * 	- Si hi han més procesos bloquejats:
				 * 		> Actualitzem el buffer i el keystoread a partir
				 * 			del primer element bloquejat									*/
				if (keystoread == circularbNumElements(&cbuffer)) {
					while (keystoread > 0) {
						circularbRead(&cbuffer,&bread);
						copy_to_user(&bread, keybuffer, 1);
						++keybuffer;
						--keystoread;
					}
					task_list = list_first(&keyboardqueue);
					list_del(task_list);
					taskbloqued = list_head_to_task_struct(task_list);
					sched_update_queues_state(&readyqueue, taskbloqued);

					if (!list_empty(&keyboardqueue)) {
						union task_union *unionbloqued;
						task_list = list_first(&keyboardqueue);
						taskbloqued = list_head_to_task_struct(task_list);
						unionbloqued = (union task_union*)taskbloqued;

						keybuffer = (unsigned int)unionbloqued->stack[taskbloqued->kernel_esp+3];
						keystoread = (char)(unionbloqued->stack[taskbloqued->kernel_esp+4]);
					}
				}

				/* Si el CircularBuffer està ple:
				 * 	- Proporcionem totes les dades del Cbuffer
				 * 		al buffer del usuari
				 * 	-	Actualitzem el keystoread amb el nou valor */
				else if (circularbIsFull(&cbuffer)) {
					while (!circularbIsEmpty(&cbuffer)) {
								circularbRead(&cbuffer,&bread);
								copy_to_user(&bread, keybuffer++, 1);
					}
					keystoread = keystoread - CBUFFER_SIZE;
				}
			}
		}
	}
	else { // Break

	}
}

