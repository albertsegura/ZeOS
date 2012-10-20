#ifndef MM_ADDRESS_H
#define MM_ADDRESS_H

#define ENTRY_DIR_PAGES       0

#define TOTAL_PAGES 1024
#define NUM_PAG_KERNEL 256
#define PAG_LOG_INIT_CODE_P0 (L_USER_START>>12)
#define FRAME_INIT_CODE_P0 (PH_USER_START>>12)
#define NUM_PAG_CODE 8
#define PAG_LOG_INIT_DATA_P0 (PAG_LOG_INIT_CODE_P0+NUM_PAG_CODE)
#define NUM_PAG_DATA 20
#define PAGE_SIZE 0x1000

// Primera p�gina lliure en un proc�s acabat de crear
#define FIRST_FREE_PAG_P (NUM_PAG_KERNEL+NUM_PAG_CODE+NUM_PAG_DATA)

/* Memory distribution */
/***********************/


#define KERNEL_START     0x10000
#define L_USER_START        0x100000
#define PH_USER_START       0x100000
#define USER_ESP	L_USER_START+(NUM_PAG_CODE+NUM_PAG_DATA)*0x1000-16

#define USER_FIRST_PAGE	(L_USER_START>>12)

#define PH_PAGE(x) (x>>12)

#endif

