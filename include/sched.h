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
#define DEFAULT_RR_QUANTUM	500

enum state_t { ST_RUN, ST_READY, ST_BLOCKED, ST_ZOMBIE };

struct task_struct {
  int PID;			/* Process ID */
  page_table_entry * dir_pages_baseAddr;
  struct list_head list;
  unsigned long kernel_esp;
  struct stats statistics;
};

union task_union {
  struct task_struct task;
  unsigned long stack[KERNEL_STACK_SIZE];    /* pila de sistema, per proces */
};

extern union task_union task[NR_TASKS]; /* Vector de tasques */
extern struct task_struct *idle_task;
extern struct list_head freequeue;
extern struct list_head readyqueue;
extern struct task_struct * idle_task;
extern unsigned int rr_quantum;
extern int lastPID;

#define KERNEL_ESP       (DWord) &task[1].stack[KERNEL_STACK_SIZE]

/* Inicialitza les dades del proces inicial */
void init_task1(void);

void init_freequeue (void);

void init_readyqueue (void);

void init_idle(void);

void init_sched(void);

struct task_struct * current();

void task_switch(union task_union *new);

int getNewPID();

int getStructPID(int PID, struct task_struct * desired);

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
void (* sched_update_queues_state)(struct list_head* ls);

/* Inicialització de la politica de scheduler Round Robin */
void init_Sched_RR();

void sched_update_data_RR();

int sched_change_needed_RR();

void sched_switch_process_RR();

void sched_update_queues_state_RR(struct list_head* ls);

#endif  /* __SCHED_H__ */
