/*=============== RobotMgr . h ===============*/
/*  BY:	Aravind Dhulipalla
		16.472 / 16.572 Embedded Real Time Systems
		Electrical and Computer Engineering Dept.
		UMASS Lowell
PURPOSE
Interface to RobotMgr.c
*/

#ifndef RobotMgr_H
#define RobotMgr_H
#include "includes.h"
#include "BfrPair.h"
#include "PBuffer.h"

/*----- c o n s t a n t    d e f i n i t i o n s -----*/
#define move 0x02
#define followpath 0x03
#define loop 0x04

/*----- t y p e    d e f i n i t i o n s -----*/
/*payload structure datatype*/
typedef struct 
{
  CPU_INT08U  payloadLen;
  CPU_INT08U  dstAddr;
  CPU_INT08U  srcAddr;
  CPU_INT08U  msgType;
  
  union
  {
    struct
    {
      CPU_INT08U RobotAddress;
      struct
      {
        CPU_INT08U X;
        CPU_INT08U Y;
      }Pos[10];
    }msgcmd;
  }cmdExt;
}payload;


//------Global Variables--------
extern CPU_INT08U Floor[39][18];
extern CPU_BOOLEAN stopRobot[13];
extern OS_Q	ParserQueue;
extern OS_Q     FramerQueue;

//---------Function Prototypes------
void CreateRobotManagerTask(void);

#endif