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

	struct task_union *idle_union_stack = idle_task;

	idle_task->PID = 0;
	idle_union_stack->stack[1023] = &cpu_idle; // ?
	// TODO push de @ret de cpu_idle i fer el dinamic link
	// Punt 3 (pag 32): Initialize an execution context for the procees to restore it when it gets assigned the cpu
	//				(see section 4.5) and executes cpu_idle.



	/*1) Store in the stack of the idle process the address of the code that it will execute (address
			of the cpu_idle function).
		2) Store in the stack the value that we want to assign to register ebp when undoing the
			dynamic link (it can be 0),
		3) Finally, we need to keep for this process (in a field of its task_struct) the value to load
			in the esp register when undoing the dynamic link (in this case this value will point to
			the position of the stack where we have stored the initial value for the ebp register).
	 */


}

void init_task1(void) {

	/*1) Assign PID 1 to the process
		2) Complete the initialization of its address space, by using the function set_user_pages (see file
			mm.c). This function allocates physical pages to hold the user address space (both code and
			data pages) and adds to the page table the logical-to-physical translation for these pages.
			Remind that the region that supports the kernel address space is already configure for all
			the possible processes by the function init_mm.
		3) Set its page directory as the current page directory in the system, by using the set_cr3
			function (see file mm.c).
	 */
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

