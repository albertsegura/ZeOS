/*
 * sched.c - initializes struct for task 0 anda task 1
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
	// Versió funció
	//INIT_LIST_HEAD(&freequeue);
	freequeue.next = &freequeue;
	freequeue.prev = &freequeue;
	int i;
	for(i=NR_TASKS-1;i!=0;--i) {
		list_add(&task[i].task.list, &freequeue);
	}

	// Versió treballada
	/*
	int i;
	for(i=0;i<NR_TASKS;++i) {
		if  (i == 0) {
			freequeue.next = &task[i].task.list;
			task[i].task.list.prev = &freequeue;
		}
		if (i != NR_TASKS-1) {
			task[i].task.list.next = &task[i+1].task.list;
			task[i+1].task.list.prev = &task[i].task.list;
		}
		else {
			task[i].task.list.next = &freequeue;
			freequeue.prev = &task[i].task.list;
		}
	}
	*/
}

void init_readyqueue (void) {
	// Versió funció
	//INIT_LIST_HEAD(&readyqueue);

	// Versió treballada
	readyqueue.next = &readyqueue;
	readyqueue.prev = &readyqueue;
}

/* No comprova que la llista no es buida perque, al inici es compleix */
void init_idle (void) {
	struct list_head *idle_list_pointer = list_first(&freequeue);
	list_del(idle_list_pointer);
	idle_task = list_head_to_task_struct(idle_list_pointer);

	union task_union *idle_union_stack = (union task_union*)idle_task;

	// TODO Preguntar errors compilació
	idle_task->PID = 0;
	idle_union_stack->stack[1023] = &cpu_idle;
	idle_union_stack->stack[1022] = 0;
	idle_union_stack->task.dir_pages_baseAddr->kernel_esp = &idle_union_stack->stack[1022];
}

void init_task1(void) {
	struct list_head *task1_list_pointer = list_first(&freequeue);
	list_del(task1_list_pointer);
	struct task_struct * task1_task_struct = list_head_to_task_struct(task1_list_pointer);

	task1_task_struct->PID = 1;
	set_user_pages(task1_task_struct);
	set_cr3(task1_task_struct->dir_pages_baseAddr);
}


void init_sched() {

}


void task_switch(union task_union *new) {

	tss.
	/*
		1) Update the TSS to make it point to the new_task system stack.
		2) Change the user address space by updating the current page directory: use the set_cr3
			funtion to set the cr3 register to point to the page directory of the new_task.
		3) Store, in the PCB, the current value of the EBP register (corresponding to the position in the
			current system stack where this routine begins).
		4) Change the current system stack by setting ESP register to point to the stored value in the
			new PCB.
		5) Restore the EBP register from the stack.
		6) RET.

	 */
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

