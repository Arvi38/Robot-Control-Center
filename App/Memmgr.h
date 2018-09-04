/*=============== M e m M g r . h ===============*/

/*
BY:	Aravind Dhulipalla
		16.472 / 16.572 Embedded Real Time Systems
		Electrical and Computer Engineering Dept.
		UMASS Lowell

PURPOSE
Interface to MemMgr.c.

*/

#ifndef MEMMGR_H
#define MEMMGR_H
#include "PBuffer.h"

/*----- c o n s t a n t    d e f i n i t i o n s -----*/

/* Total number of buffers in buffer pool */
#define PoolSize 45

/*----- g l o b a l s -----*/

/* The buffer pool */
extern OS_MEM bfrPool;

/*----- f u n c t i o n    p r o t o t y p e s -----*/

void InitMemMgr(void);
PBuffer *Allocate(void);
void Free(PBuffer *bfr);

#endif