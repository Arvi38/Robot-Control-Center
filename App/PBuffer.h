/*=============== PB u f f e r . h ===============*/
/*
BY:	    Aravind Dhulipalla
		16.472 / 16.572 Embedded Real Time Systems
		Electrical and Computer Engineering Dept.
		UMASS Lowell
*/

/*
PURPOSE
Provide a set of operations on buffers (not circular)
*/

#ifndef PBUFFER_H
#define PBUFFER_H

/*----- c o n s t a n t    d e f i n i t i o n s -----*/

// Define the buffer capacity

#ifndef PBfrSize
#define PBfrSize 35
#endif

/*----- t y p e    d e f i n i t i o n s -----*/

// Define the buffer type

typedef struct
{
	CPU_INT08U  *in;             // Points to space to add next byte
	CPU_INT08U  *out;            // Points to next byte to remove
	CPU_INT08U  bfr[PBfrSize];    // The buffer storage area
}PBuffer;


/*----- f u n c t i o n    p r o t o t y p e s -----*/

CPU_BOOLEAN Empty(PBuffer *theBfr);
CPU_BOOLEAN Full(PBuffer *theBfr);
CPU_INT16S AddByte(PBuffer *theBfr, CPU_INT16S theByte);
CPU_INT16S RemoveByte(PBuffer *theBfr);
void InitBfr(PBuffer *bfr);
#endif
