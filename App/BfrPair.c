/*---------------BfrPair.c----------------------
BY:	Aravind Dhulipalla
		16.472 / 16.572 Embedded Real Time Systems
		Electrical and Computer Engineering Dept.
		UMASS Lowell

PURPOSE
To provide required functions for bufferpair management to be used in Serial IO driver.
Built on top of Buffer.c functions.
*/

#include "includes.h"
#include "BfrPair.h"

/*-------------BfrPairInit()------------
           To Initialise both buffers.*/
void BfrPairInit(BfrPair *bfrPair,CPU_INT08U *bfr0Space,CPU_INT08U *bfr1Space,CPU_INT16U size)
{
  bfrPair->putBfrNum=0;
  BfrInit(&bfrPair->buffers[0],bfr0Space,size);
  BfrInit(&bfrPair->buffers[1],bfr1Space,size);
}

/*---------putBfrReset()--------
        Resets the put buffer */
void putBfrReset(BfrPair *bfrPair)
{
 BfrReset(&bfrPair->buffers[bfrPair->putBfrNum]); 
}

/*--------*putBfrAddr()-------
to obtain the address of the put buffers buffer dataspace.Returns the address 
of the put buffers buffer data space*/
void *PutBfrAddr(BfrPair *bfrPair)
{
   
   return (bfrPair->buffers[bfrPair->putBfrNum].buffer);
}

/*----------*GetBfrAddr()-------
To obtain the address of the get buffers buffer data space.Returns the address 
of the get buffers buffer data space*/
void *GetBfrAddr(BfrPair *bfrPair)
{
  return (bfrPair->buffers[1-bfrPair->putBfrNum].buffer);
}

/*---------putBfrClosed()--------
To test whether put buffer is closed or not*/
CPU_BOOLEAN PutBfrClosed(BfrPair *bfrPair)
{
  return (BfrClosed(&bfrPair->buffers[bfrPair->putBfrNum]));
}

/*---------GetBfrClosed()--------
To test whether get buffer is closed or not */
CPU_BOOLEAN GetBfrClosed(BfrPair *bfrPair)
{
  return (BfrClosed(&bfrPair->buffers[1 - bfrPair->putBfrNum]));
}
/*------ClosePutBfr()------
To Mark the put buffer as closed*/
void ClosePutBfr(BfrPair *bfrPair)
{
  BfrClose(&bfrPair->buffers[bfrPair->putBfrNum]);
}

/*-------OpenGetBfr()-------
To mark the get buffer as open */
void OpenGetBfr(BfrPair *bfrPair)
{
  BfrOpen(&bfrPair->buffers[1 - bfrPair->putBfrNum]);
  
}

/*---------PutBfrAddByte()------
To Add a byte to the put buffer at position putindex and increment putindex by 1
If the buffer is full mark it as closed.Returns the byte added.if the buffer is 
full returns -1.*/
CPU_INT16S PutBfrAddByte(BfrPair *bfrPair,CPU_INT16S byte)
{
  return(BfrAddByte(&bfrPair->buffers[bfrPair->putBfrNum],byte));
}

/*-------GetBfrNextByte()-----
Returns the byte from the position get index of the get buffer or returns -1 if 
the buffer gets empty */
CPU_INT16S GetBfrNextByte(BfrPair *bfrPair)
{
  return(BfrNextByte(&bfrPair->buffers[1 - bfrPair->putBfrNum]));
}

/*-------GetBfrRemByte()-------
Returns the byte from the position getindex in the get buffer and increments the 
get buffer's get index by 1.If buffer is empty marks it as open*/
CPU_INT16S GetBfrRemByte(BfrPair *bfrPair)
{
  return(BfrRemoveByte(&bfrPair->buffers[1 - bfrPair->putBfrNum]));
}

/*-------BfrPairSwappable()-------
Tests whether or not a buffer pair is ready to be swapped i.e the put buffer is 
closed and the get buffer is open*/
CPU_BOOLEAN BfrPairSwappable(BfrPair *bfrPair)
{
    return (PutBfrClosed(bfrPair) && !GetBfrClosed(bfrPair));
  
  
}

/*-------BfrPairSwap()-----
swaps the put buffer and get buffer and resets the put buffer*/
void BfrPairSwap(BfrPair *bfrPair)
{
     bfrPair->putBfrNum = 1 - bfrPair->putBfrNum; 
     putBfrReset(bfrPair);
}
