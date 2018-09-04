/*---------------BfrPair.c----------------------
BY:	Aravind Dhulipalla
		16.472 / 16.572 Embedded Real Time Systems
		Electrical and Computer Engineering Dept.
		UMASS Lowell

PURPOSE
To provide basic required functions for buffer management.

*/

#include "includes.h"
#include <stdio.h>
#include "Buffer.h"

/*--------BfrInit()--------
Initialises the buffer and sets putindex,getindex to zero,marks the buffer open 
and records the size*/
void BfrInit(Buffer *bfr,CPU_INT08U *bfrSpace,CPU_INT16U size)
{
  bfr->putIndex =0;
  bfr->getIndex =0;
  bfr->closed = FALSE;
  bfr->size =size;
  bfr->buffer=bfrSpace;
}

/*------BfrReset()-----
Resets the buffer,sets putindex and getindex to zero and marks the buffer open*/
void BfrReset(Buffer *bfr)
{
    bfr->putIndex =0;
    bfr->getIndex =0;
    bfr->closed = FALSE;
}

/*------BfrClosed()--------
Tests whether a buffer is closed */
CPU_BOOLEAN BfrClosed(Buffer *bfr)
{
  return(bfr->closed);
}

/*-------BfrClose()-------
Marks the buffer as clode */
void BfrClose(Buffer *bfr)
{
  bfr->closed = TRUE;
}

/*-------BfrOpen()-----
Marks the buffer as open */
void BfrOpen(Buffer *bfr)
{
  bfr->closed = FALSE;
}

/*-----BfrFull()------
Tests whether a buffer full or not */
CPU_BOOLEAN BfrFull(Buffer *bfr)
{
 return (bfr->putIndex >= bfr->size);
}

/*------BfrEmpty()-----
Tests whether a buffer is empty */
CPU_BOOLEAN BfrEmpty(Buffer *bfr)
{
 
  return (bfr->getIndex >= bfr->size);
}

/*--------BfrAddByte()------
Adds a byte to a buffer at position putindex and increment putindex by 1.If the 
putindex is full marks it as closed */
CPU_INT16S BfrAddByte(Buffer *bfr,CPU_INT16S theByte)
{
  if(BfrFull(bfr))
  {
    return -1;
  }
     bfr->buffer[bfr->putIndex++]=theByte;
     if(BfrFull(bfr))
     {
       BfrClose(bfr);
     }
        return (theByte);
}

/*-----BfrNextByte()------
Retuns the byte from the position getindex,returns -1 if the buffer is empty*/
CPU_INT16S BfrNextByte(Buffer *bfr)
{
  if(BfrEmpty(bfr))
  {    
    return -1;
  }
  return (bfr->buffer[bfr->getIndex]);
     
}

/*-------BfrRemoveByte()------
Returns the byte from position getindex and increments getindex by 1.marks the
buffer as open if the buffer is empty */
CPU_INT16S BfrRemoveByte(Buffer *bfr)
{
  CPU_INT08U theByte;
  if(BfrEmpty(bfr))
     {
        return -1;
     }
        theByte = bfr->buffer[bfr->getIndex++];
        if(BfrEmpty(bfr))
       {
         BfrOpen(bfr);
       }
  
        return (theByte);

}
