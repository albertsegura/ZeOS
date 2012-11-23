/*
 * system.h - Capçalera del mòdul principal del sistema operatiu
 */

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include <types.h>
#include <cbuffer.h>


extern TSS         tss;
extern Descriptor* gdt;
extern int 	zeos_ticks;
extern int keystoread;
extern char * keybuffer;
extern Circular_Buffer cbuffer;

#endif  /* __SYSTEM_H__ */
