/*
 * sched.h - Estructures i macros pel tractament de processos
 */

#ifndef __SCHED_H__
#define __SCHED_H__

#include <list.h>
#include <types.h>
#include <mm_address.h>
#include <stats.h>

#define NR_TASKS      10
#define KERNEL_STACK_SIZE	1024
#define DEFAULT_RR_QUANTUM	100

enum state_t { ST_RUN, ST_READY, ST_BLOCKED, ST_ZOMBIE };

struct keyboard_info {
		char * keybuffer;
		int keystoread;
		int keysread;
};

struct task_struct {
  int PID;			/* Process ID */
  page_table_entry * dir_pages_baseAddr;
  struct list_head list;
  unsigned long kernel_esp;
  struct stats statistics;
  enum state_t process_state;

  /* Utilitzat per implementar els threads*/
  Byte *dir_count; /* Punter al contador de referencies al directori propi. */

  /* Utilitzat per a la implementaci� del read*/
  struct keyboard_info kbinfo;

  /* Utilitzat pel control de la zona HEAP */
  unsigned int *program_break;
  Byte *pb_count;
};

union task_union {
  struct task_struct task;
  unsigned long stack[KERNEL_STACK_SIZE];    /* pila de sistema, per proces */
};

extern union task_union task[NR_TASKS]; /* Vector de tasques */
extern struct task_struct *idle_task;
extern struct list_head freequeue;
extern struct list_head readyqueue;
extern struct list_head keyboardqueue;
extern struct task_struct * idle_task;
extern unsigned int rr_quantum;
extern int lastPID;

#define KERNEL_ESP       (DWord) &task[1].stack[KERNEL_STACK_SIZE]

void init_task1(void);

void init_freequeue (void);

void init_readyqueue (void);

void init_keyboardqueue (void);

void init_idle(void);

void init_sched(void);

void init_semarray(void);

struct task_struct * current();

void task_switch(union task_union *new);

int getNewPID();

int getStructPID(int PID, struct list_head * queue, struct task_struct ** pointer_to_desired);

struct task_struct *list_head_to_task_struct(struct list_head *l);

page_table_entry * get_PT (struct task_struct *t) ;

page_table_entry * get_DIR (struct task_struct *t) ;



/* SCHEDULER */


/* Update policy data (update tics in the case of a round robin, update priorities, etc.).
 * There can be a generic function that calls one or another function based on a global
 * variable or a constant that selects the policy. */
void (* sched_update_data)();

/* Check the policy to see whether a process switch is required or not. */
int (* sched_change_needed)();

/* Select the next process to run and perform a process switch. */
void (* sched_switch_process)();

/* Update queues and state of processes.*/
void (* sched_update_queues_state)(struct list_head* ls, struct task_struct * task);

/* Inicialitzaci� de la politica de scheduler Round Robin */
void init_Sched_RR();

void sched_update_data_RR();

int sched_change_needed_RR();

void sched_switch_process_RR();

void sched_update_queues_state_RR(struct list_head* ls, struct task_struct * task);



#endif  /* __SCHED_H__ */
