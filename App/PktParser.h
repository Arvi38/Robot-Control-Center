/*=============== P k t P a r s e r . h ===============
BY:	Aravind Dhulipalla
		16.472 / 16.572 Embedded Real Time Systems
		Electrical and Computer Engineering Dept.
		UMASS Lowell

PURPOSE
Inteface to PktParser.c

*/

#ifndef PKTPARSER_H
#define PKTPARSER_H

#include "includes.h"
#include "BfrPair.h"
#include "RobotMgr.h"
#include "SerIODriver.h"
#include "assert.h"

/*------ c o n s t a n t    d e f i n i t i o n s -----*/
#define DestinationAddress 1

/*----- t y p e    d e f i n i t i o n s -----*/

// Define the PktBfr type
typedef struct
{
  CPU_INT08S  payloadLen; //length of the payload
  CPU_INT08U  msgdata[1]; //Remaing data from packet
}PktBfr;

//----- f u n c t i o n    p r o t o t y p e s -----
CPU_VOID CreateParserTask(CPU_VOID);
void ErrorState(CPU_INT08U ErrorNumber);
#endif







