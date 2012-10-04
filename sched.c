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
}

void init_readyqueue (void) {
	readyqueue.next = &readyqueue;
	readyqueue.prev = &readyqueue;
}

void init_idle (void) {
// Pagina 32
/*
	1) Get an available task_union from the freequeue to contain the characteristics of this process
	2) Assign PID 0 to the process.
	3) Initialize an execution context for the procees to restore it when it gets assigned the cpu
	(see section 4.5) and executes cpu_idle.
	4) Define a global variable idle_task
	struct task_struct * idle_task;
	5) Initialize the global variable idle_task, which will help to get easily the task_struct of the
	idle process.
*/



}

void init_task1(void) {

}


void init_sched() {

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

