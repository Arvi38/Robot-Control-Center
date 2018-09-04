/*=============== SerIODriver . h ===============*/
/*  BY:	Aravind Dhulipalla
		16.472 / 16.572 Embedded Real Time Systems
		Electrical and Computer Engineering Dept.
		UMASS Lowell
PURPOSE
Interface to the SerIODriver.c. */

#include <stdio.h>
#include "includes.h"
#include "assert.h"
#include "BfrPair.h"
#ifndef SerIODriver_H
#define SerIODriver_H

//------------Function Prototypes-------------
void InitSerIO(void);
CPU_INT16S PutByte(CPU_INT16S c);
CPU_INT16S GetByte(void);
void ISR(void);
void TxFlush(void);

#endif
