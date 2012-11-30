/*
 * system.h - Capçalera del modul principal del sistema operatiu
 */

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include <types.h>
#include <cbuffer.h>
#include <sem.h>
#include <mm_address.h>


extern TSS         tss;
extern Descriptor* gdt;
extern int 	zeos_ticks;
extern Circular_Buffer cbuffer;
extern Sem sem_array[SEM_SIZE];

#endif  /* __SYSTEM_H__ */
