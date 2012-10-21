/*
 * sched.c - initializes struct for task 0 and task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>



union task_union task[NR_TASKS]
  __attribute__((__section__(".data.task")));


#if 0
struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return list_entry( l, struct task_struct, list);
}
#endif 

int lastPID;

extern struct list_head blocked;

struct list_head freequeue;

struct list_head readyqueue;

struct task_struct * idle_task;

/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry * get_DIR (struct task_struct *t) 
{
	return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry * get_PT (struct task_struct *t) 
{
	return (page_table_entry *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr))<<12);
}

void cpu_idle(void)
{
	__asm__ __volatile__("sti": : :"memory");

	while(1)
	{
	;
	}
}

void init_freequeue (void) {
	INIT_LIST_HEAD(&freequeue);

	int i;
	for (i = 0; i < NR_TASKS; ++i) {
		list_add_tail(&task[i].task.list,&freequeue);
	}
}

void init_readyqueue (void) {
	INIT_LIST_HEAD(&readyqueue);
}

void init_idle (void) {
	struct list_head *idle_list_pointer = list_first(&freequeue);
	list_del(idle_list_pointer);
	idle_task = list_head_to_task_struct(idle_list_pointer);

	union task_union *idle_union_stack = (union task_union*)idle_task;

	idle_task->PID = 0;
	idle_task->quantum = RR_QUANTUM; // TODO Check
	idle_union_stack->stack[1023] = (unsigned long)&cpu_idle;
	idle_union_stack->stack[1022] = 0;
	idle_union_stack->task.kernel_esp = (unsigned long)&idle_union_stack->stack[1022];
}

void init_task1(void) {
	struct list_head *task1_list_pointer = list_first(&freequeue);
	list_del(task1_list_pointer);
	struct task_struct * task1_task_struct = list_head_to_task_struct(task1_list_pointer);
	page_table_entry * dir_task1 = get_DIR(task1_task_struct);

	task1_task_struct->PID = 1;
	task1_task_struct->quantum = RR_QUANTUM;
	set_user_pages(task1_task_struct);
	set_cr3(dir_task1);

}


void init_sched() {

}


void task_switch(union task_union *new) {
	struct task_struct * current_task_struct = current();
	page_table_entry * dir_new = get_DIR((struct task_struct *) new);

	tss.esp0 = (unsigned long)&new->stack[KERNEL_STACK_SIZE]; // o 1023?
	set_cr3(dir_new);

	unsigned long *kernel_esp = &current_task_struct->kernel_esp;
	unsigned long new_kernel_esp = new->task.kernel_esp;

  __asm__ __volatile__(
  		"mov %%ebp,(%0);" 	/*	Punt 3	*/
  		"mov %1,%%esp;" 		/*	Punt 4	*/
  		"pop %%ebp;"				/*	Punt 5	*/
  		"ret;"
  		: /* no output */
  		: "r" (kernel_esp), "r" (new_kernel_esp)
  );
}

/* Retorna el lastPID assignat +1.
 * Si s'arriba al overflow es pot controlar i mirar quin esta lliure*/
int getNewPID() {
	return ++lastPID;
}

struct task_struct *list_head_to_task_struct(struct list_head *l) {
		return list_entry(l,struct task_struct,list);
}

struct task_struct* current()
{
  int ret_value;
  
  __asm__ __volatile__(
  	"movl %%esp, %0"
	: "=g" (ret_value)
  );
  return (struct task_struct*)(ret_value&0xfffff000);
}


/* Update policy data (update tics in the case of a round robin, update priorities, etc.).
 * There can be a generic function that calls one or another function based on a global
 * variable or a constant that selects the policy. */
int sched_update_data() {

	if (SCHRED_POLICY == SCHRED_POLICY_RR) {
		struct task_struct * current_task = current();
		--current_task->quantum;
	}
	else {
		/* Other scheduler policies */
	}

	return 0;
}

/* Check the policy to see whether a process switch is required or not. */
int schred_change_needed() {

	if (SCHRED_POLICY == SCHRED_POLICY_RR) {
		struct task_struct * current_task = current();
		if (current_task->quantum == 0) return 1;
	}
	else {
		/* Other scheduler policies */
	}
	return 0;
}

/* Select the next process to run and perform a process switch. */
int schred_switch_process() {

	if (SCHRED_POLICY == SCHRED_POLICY_RR) {
		if (!list_empty(&readyqueue)) {
			struct list_head *new_list_task = list_first(&readyqueue);
			list_del(new_list_task);
			struct task_struct * new_task = list_head_to_task_struct(new_list_task);
			struct task_struct * current_task = current();

			list_add_tail(&current_task->list,&readyqueue);
			new_task->quantum = RR_QUANTUM;
			task_switch((union task_union*)new_task);
		}
		else {
			struct task_struct * current_task = current();
			current_task->quantum = RR_QUANTUM;
		}
	}
	else {
		/* Other scheduler policies */
	}
	return 0;
}

/* Update queues and state of processes.*/
int schred_update_queues_state() {

	if (SCHRED_POLICY == SCHRED_POLICY_RR) {

	}
	else {
		/* Other scheduler policies */
	}
	return 0;
}



