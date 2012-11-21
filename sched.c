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

unsigned int rr_quantum;

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

int getStructPID(int PID, struct task_struct ** pointer_to_desired) {
	int found = 0;

	if (current()->PID == PID) {
		*pointer_to_desired = current();
		found = 1;
	}

	if (!list_empty(&readyqueue) && (found == 0)) {
		struct list_head *first = list_first(&readyqueue);
		if (list_head_to_task_struct(first)->PID == PID) {
			found = 1;
			*pointer_to_desired = list_head_to_task_struct(first);
		}
		else {
			list_del(&readyqueue);
			list_add_tail(first, &readyqueue);
		}

		while(first != list_first(&readyqueue)) {
			struct list_head *act = list_first(&readyqueue);
			if (list_head_to_task_struct(act)->PID == PID) {
				found = 1;
				*pointer_to_desired = list_head_to_task_struct(act);
			}
			list_del(&readyqueue);
			list_add_tail(act, &readyqueue);
		}
	}

	return found;
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
	idle_union_stack->stack[1023] = (unsigned long)&cpu_idle;
	idle_union_stack->stack[1022] = 0;
	idle_union_stack->task.kernel_esp = (unsigned long)&idle_union_stack->stack[1022];

	// Inicialitzacio estadistica
	idle_task->statistics.cs = 0;
	idle_task->statistics.tics = 0;
	idle_task->statistics.remaining_quantum = 0;
	idle_task->process_state = ST_READY;
}

void init_task1(void) {
	struct list_head *task1_list_pointer = list_first(&freequeue);
	list_del(task1_list_pointer);
	struct task_struct * task1_task_struct = list_head_to_task_struct(task1_list_pointer);
	allocate_page_dir(task1_task_struct);
	page_table_entry * dir_task1 = get_DIR(task1_task_struct);

	task1_task_struct->PID = 1;
	set_user_pages(task1_task_struct);
	set_cr3(dir_task1);

	// Inicialitzacio estadistica
	task1_task_struct->statistics.cs = 0;
	task1_task_struct->statistics.tics = 0;
	task1_task_struct->statistics.remaining_quantum = DEFAULT_RR_QUANTUM;
	task1_task_struct->process_state = ST_READY;
}

void init_sched() {
	init_Sched_RR();
}

void task_switch(union task_union *new) {
	//TODO Comprovar funcionament optimització
	struct task_struct * current_task_struct = current();
	page_table_entry * dir_new = get_DIR((struct task_struct *) new);
	page_table_entry * dir_current = get_DIR(current_task_struct);

	tss.esp0 = (unsigned long)&new->stack[KERNEL_STACK_SIZE];
	if (dir_new != dir_current) set_cr3(dir_new);

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

/* SCHEDULER */
void init_Sched_RR() {
	sched_update_data = sched_update_data_RR;
	sched_change_needed = sched_change_needed_RR;
	sched_switch_process = sched_switch_process_RR;
	sched_update_queues_state = sched_update_queues_state_RR;
	rr_quantum = DEFAULT_RR_QUANTUM;

	//Inicialitzacio estadistica
	struct task_struct * current_task = current();
	current_task->statistics.remaining_quantum = DEFAULT_RR_QUANTUM;
	current_task->process_state = ST_READY;
}

void sched_update_data_RR() {
	--rr_quantum;

	// Actualitzacio estadistica
	struct task_struct * current_task = current();
	--current_task->statistics.remaining_quantum;
	++current_task->statistics.tics;
}

int sched_change_needed_RR() {
	return rr_quantum == 0;
}

void sched_switch_process_RR() {
	struct list_head *task_list;
	struct task_struct * task;

	if (!list_empty(&readyqueue)) {
		task_list = list_first(&readyqueue);
		list_del(task_list);
		task = list_head_to_task_struct(task_list);
	}
	else task = idle_task;

	task->statistics.remaining_quantum = DEFAULT_RR_QUANTUM;
	rr_quantum = DEFAULT_RR_QUANTUM;
	if (task != current()) {
		++task->statistics.cs;
		task->process_state = ST_RUN;
		task_switch((union task_union*)task);
	}
}

void sched_update_queues_state_RR(struct list_head* ls) {
	struct task_struct * current_task = current();
	if (ls == &freequeue) current_task->process_state = ST_ZOMBIE;
	else if (ls == &readyqueue) current_task->process_state = ST_READY;

	list_add_tail(&current_task->list,ls);
}
