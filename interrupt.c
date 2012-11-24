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
#include <mm.h>

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

/*	Gestiona la copia del cbuffer i els problemes ocasionats al ser un
 * 	procés diferent del bloquejat el que ha d'accedir al espai d'@ del
 * 	procés bloquejat.
 *	Llegeix min(keystoread,size del cbuffer) elements.
 * 		-	Retorna 1:
 * 					Si ha pogut realitzar la copia.
 * 		-	Retorna 0:
 * 					Si no ha pogut realitzar la copia per falta de pagines
 * 					lliures en el procés per poder accedir al espai d'@ del
 * 					procés bloquejat.
 * 	*/
int keyboard_cbuffer_read() {
	struct list_head *list_blocked = list_first(&keyboardqueue);
	struct task_struct * blocked_pcb = list_head_to_task_struct(list_blocked);
	struct task_struct * current_pcb = current();
	page_table_entry * pt_blocked = get_PT(blocked_pcb);
	page_table_entry * pt_current = get_PT(current_pcb);
	page_table_entry * dir_blocked = get_DIR(blocked_pcb);
	page_table_entry * dir_current = get_DIR(current_pcb);
	char bread;

	if (dir_blocked != dir_current) { // Si són 2 procesos independents
		int id_pag_buffer = ((int)keybuffer&0x005ff000)>>12;
		int	addr_buffer = ((int)keybuffer&0x00000FFF);

		int free_pag = FIRST_FREE_PAG_P;
		while(pt_current[free_pag].entry != 0 && free_pag<TOTAL_PAGES) free_pag++;
		if (free_pag == TOTAL_PAGES) return 0;

		set_ss_pag(pt_current,free_pag, pt_blocked[id_pag_buffer].bits.pbase_addr);

		while (!circularbIsEmpty(&cbuffer) && keystoread > 0) {
			circularbRead(&cbuffer,&bread);
			copy_to_user(&bread, (void *)((free_pag<<12)+addr_buffer), 1);
			keystoread--;
			keybuffer++;
			addr_buffer++;
			if (addr_buffer == PAGE_SIZE) {
				id_pag_buffer++;
				set_ss_pag(pt_current,free_pag, pt_blocked[id_pag_buffer].bits.pbase_addr);
				set_cr3(dir_current);
			}
		}
		del_ss_pag(pt_current, free_pag);
	}
	else { // Si els 2 procesos són threads amb mem.compartida
		while (!circularbIsEmpty(&cbuffer) && keystoread > 0) {
			circularbRead(&cbuffer,&bread);
			copy_to_user(&bread, keybuffer++, 1);
			keystoread--;
		}
	}

	return 1;
}

void keyboard_routine() {
	unsigned char read = inb(0x60); // Lectura del Port
	char keyPressed;
	
	/* Primer bit indica Make(0)/Break(1) => key (Pressed/Relased)*/
	if (read < 0x80) { // Make
		if (read < 98) keyPressed = char_map[read] == '\0' ? 'C' : char_map[read];
		else keyPressed = 'C';

		/* Si el Buffer Circular no està ple */
		if (!circularbIsFull(&cbuffer)) {
			circularbWrite(&cbuffer,&keyPressed);
		}

		/* Com que el buffer es pot emplenar i no sempre podem garantir que es
		 * podrà buidar, sempre s'ha de comprovar si podem actuar.
		 * Rao: Abans si el buffer estava ple era perque no hi havia cap
		 * 			proces que volgues llegir, ara no es així.  */
		if (!list_empty(&keyboardqueue)) {
			int ret = 0;

			/*	Si es compleix la condició hem d'intentar llegir el buffer */
			if (keystoread <= circularbNumElements(&cbuffer) || circularbIsFull(&cbuffer)) {
				 ret = keyboard_cbuffer_read();
			}

			/* Si hem pogut llegir del buffer i no queda res més per llegir:
			 * 	-	Posem el procés en la readyqueue.
			 * 	-	Actualitzem el buffer i el keystoread a partir del primer
			 * 		element bloquejat, si n'hi han més.											*/
			if (keystoread == 0 && ret) {

				struct list_head * task_list = list_first(&keyboardqueue);
				list_del(task_list);
				struct task_struct * taskbloqued = list_head_to_task_struct(task_list);
				sched_update_queues_state(&readyqueue, taskbloqued);

				if (!list_empty(&keyboardqueue)) {
					union task_union *unionbloqued;
					task_list = list_first(&keyboardqueue);
					taskbloqued = list_head_to_task_struct(task_list);
					unionbloqued = (union task_union*)taskbloqued;

					/* Parametres d'on esta bloquejada la rutina : sys_read_keyboard */
					keybuffer = (void *)unionbloqued->stack[taskbloqued->kernel_esp+2];
					keystoread = (char)(unionbloqued->stack[taskbloqued->kernel_esp+3]);
				}
			}
		}
	}
	else { // Break

	}
}

