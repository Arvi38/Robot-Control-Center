/*=============== Framer . c ===============*/
/*
BY:	Aravind Dhulipalla
		16.472 / 16.572 Embedded Real Time Systems
		Electrical and Computer Engineering Dept.
		UMASS Lowell
		
Purpose
To construct a packet and send it to the SerIo driver.
*/

#include "PktParser.h"
#include "includes.h"
#include "PBuffer.h"
#include "MemMgr.h"
#include "Framer.h"
#include "SerIODriver.h"

/*------Constant Definitions----- */
#define SuspendTimeout 0
#define FramerPrio 2 // Framer task priority
/* Size of the Process task stack */
#define	FRAMER_STK_SIZE     256
#define error 0x0B
#define Ack 0x0A
#define step 0x07  
#define p1 0x03  //Preamble byte 1
#define p2 0xaf  //Premmble byte 2
#define p3 0xef  //Preamble byte 3
#define PL 0x09  //Packet Length
#define SA 0x02 //Source Address

/*----- g l o b a l s -----*/
// Task Control Block
 OS_TCB FramerTCB;
/* Stack space for task stack */
 CPU_STK FramerStk[FRAMER_STK_SIZE];

 /*----framer datatype---*/
typedef struct
{
  CPU_INT08U Msg; //can be error number or can be direction of the robot
  CPU_INT08U Pkt_type; //packet type that is to be sent
  CPU_INT08U CurrentRobot; //robot address
}Framer;
 
/*--------------- CreateFramerTask ( ) ---------------*/
/*
PURPOSE
Create and Initialize the Framer task.
*/
CPU_VOID CreateFramerTask(CPU_VOID)
{
  OS_ERR		osErr;/* -- OS Error code */
  
  /* Create Framer task. */	
  OSTaskCreate(&FramerTCB,
               "Framer Task",
               FramerTask, 
               NULL,
               FramerPrio,
               &FramerStk[0],
               FRAMER_STK_SIZE / 10,
               FRAMER_STK_SIZE,
               0,
               0,
               0,
               (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
               &osErr);
  assert(osErr == OS_ERR_NONE);
}

/*--------------- Framertask( ) ---------------*/
/*
PURPOSE
Task function for Framer task.

INPUT PARAMETERS
data		-- pointer to task data (not used)
*/
CPU_VOID FramerTask(CPU_VOID *data)
{
  PBuffer *oBfr = NULL;
  OS_MSG_SIZE msgSize;
  OS_ERR osErr;
  for(;;)
  {
    if (oBfr == NULL)
    {
      //Wait until the buffer is arrived
      oBfr =  OSQPend(&FramerQueue,
                      0,
                      OS_OPT_PEND_BLOCKING,
                      &msgSize,
                      NULL,
                      &osErr);
      assert(osErr==OS_ERR_NONE);
    }
   Framer *framer = (Framer *)oBfr->bfr;
    switch(framer->Pkt_type)
    {
    case error:
    Framedata(0x01,0x0B,framer->Msg);
    break;
    case Ack:
     Framedata(0x01,0x0A,framer->Msg);
      break;
    case step:
    Framedata(framer->CurrentRobot,0x07,framer->Msg);
      break;
    }
    Free(oBfr);
    oBfr = NULL;
}
}

/*--------------- Framedata( ) ---------------*/

/*
PURPOSE
Creates the packet as defined in the output packet structure and stores it in a buffer.

INPUT PARAMETERS
Robot		-- Address of the Robot
Pkt_type 		-- Type of message that is to be sent
Msg_code        -- Can be dirction of robot step or command type acknowledged or Error code
*/

void Framedata(CPU_INT08U Robot,CPU_INT08U Pkt_type,CPU_INT08U Msg_code)
{
      PutByte(p1);
      PutByte(p2);
      PutByte(p3);
      PutByte(PL);
      PutByte(Robot);
      PutByte(SA);
      PutByte(Pkt_type);
      PutByte(Msg_code);
      PutByte(0x03^0xaf^0xef^0x09^Robot^0x02^Pkt_type^Msg_code);
      TxFlush();
}