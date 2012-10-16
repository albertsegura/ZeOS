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
	for(i=NR_TASKS-1; i!=-1 ;--i) {
		list_add(&task[i].task.list, &freequeue);
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
	// TODO Falta revisar si esta b�, i comprovar el funcionament
	struct task_struct * current_task_struct = current();
	tss.esp0 = (unsigned long)&new->stack[1023]; // o 1024?
	set_cr3(new->task.dir_pages_baseAddr);
	// TODO Camp kernel_ebp s'ha de fusionar en el kernel_esp, fan el mateix proposit.

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

