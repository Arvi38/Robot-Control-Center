/*---------------BfrPair.c----------------------
BY:	Aravind Dhulipalla
		16.472 / 16.572 Embedded Real Time Systems
		Electrical and Computer Engineering Dept.
		UMASS Lowell

PURPOSE
Interface to BfrPair.c.

*/
#ifndef BfrPair_H
#define BfrPair_H

#include "Buffer.h"

/*----- c o n s t a n t    d e f i n i t i o n s -----*/

#define Numbfr 2 


/*----- t y p e    d e f i n i t i o n s -----*/

//the buffer pair type

typedef struct
{
  CPU_INT08U putBfrNum;  /* The index of the put buffer*/
  Buffer buffers[Numbfr]; /* The 2 buffers */
}BfrPair;


//---------Buffer Pair Function Prototypes----------
void BfrPairInit(BfrPair *bfrPair,CPU_INT08U *bfr0Space,CPU_INT08U *bfr1Space,CPU_INT16U size);
void putBfrReset(BfrPair *bfrPair);
void *PutBfrAddr(BfrPair *bfrPair);
void *GetBfrAddr(BfrPair *bfrPair);
CPU_BOOLEAN PutBfrClosed(BfrPair *bfrPair);
CPU_BOOLEAN GetBfrClosed(BfrPair *bfrPair);
void ClosePutBfr(BfrPair *bfrPair);
void OpenGetBfr(BfrPair *bfrPair);
CPU_INT16S PutBfrAddByte(BfrPair *bfrPair,CPU_INT16S byte);
CPU_INT16S GetBfrNextByte(BfrPair *bfrPair);
CPU_BOOLEAN BfrPairSwappable(BfrPair *bfrPair);
CPU_INT16S GetBfrRemByte(BfrPair *bfrPair);
void BfrPairSwap(BfrPair *bfrPair);
#endif
