/*---------------Buffer.h----------------------

BY:	Aravind Dhulipalla
		16.472 / 16.572 Embedded Real Time Systems
		Electrical and Computer Engineering Dept.
		UMASS Lowell
		
Purpose
Interface to Buffer.c

*/
#include <stdio.h>
#include "includes.h"
#ifndef Buffer_H
#define Buffer_H

/*----- t y p e    d e f i n i t i o n s -----*/
// the buffer type

typedef struct
{
  volatile CPU_BOOLEAN closed; /*TRUE if buffer has data ready to process,Ready to be
  emptied or being emptied,False if buffer is not ready to
  Process,ready to fill or being filled.*/
  CPU_INT16U size;      /*The size of the buffer*/
  CPU_INT16U putIndex;  /*The Position where the next byte is added*/
  CPU_INT16U getIndex;  /*The Position of the next byte to remove*/
  CPU_INT08U *buffer;   /*The address of the buffer space*/
}Buffer;

//-----Function Prototypes-------

void BfrInit(Buffer *bfr,CPU_INT08U *bfrSpace,CPU_INT16U size);
void BfrReset(Buffer *bfr);
CPU_BOOLEAN BfrClosed(Buffer *bfr);
void BfrClose(Buffer *bfr);
CPU_BOOLEAN BfrFull(Buffer *bfr);
CPU_BOOLEAN BfrEmpty(Buffer *bfr);
CPU_INT16S BfrAddByte(Buffer *bfr,CPU_INT16S theByte);
CPU_INT16S BfrNextByte(Buffer *bfr);
CPU_INT16S BfrRemoveByte(Buffer *bfr);
void BfrOpen(Buffer *bfr);

#endif
