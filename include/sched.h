/*
 * sched.h - Estructures i macros pel tractament de processos
 */

#ifndef __SCHED_H__
#define __SCHED_H__

#include <list.h>
#include <types.h>
#include <mm_address.h>

#define NR_TASKS      10
#define KERNEL_STACK_SIZE	1024
#define RR_QUANTUM	500
#define SCHRED_POLICY_RR	1
#define SCHRED_POLICY SCHRED_POLICY_RR

enum state_t { ST_RUN, ST_READY, ST_BLOCKED, ST_ZOMBIE };

struct task_struct {
  int PID;			/* Process ID */
  page_table_entry * dir_pages_baseAddr;
  struct list_head list;
  unsigned long kernel_esp;
  unsigned int quantum;
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

struct task_struct *list_head_to_task_struct(struct list_head *l);

page_table_entry * get_PT (struct task_struct *t) ;

page_table_entry * get_DIR (struct task_struct *t) ;

int sched_update_data();

int schred_change_needed();

int schred_switch_process();

int schred_update_queues_state();

#endif  /* __SCHED_H__ */
