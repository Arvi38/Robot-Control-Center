/*=============== RobotMgr . c ===============*/
/*  BY:	Aravind Dhulipalla
		16.472 / 16.572 Embedded Real Time Systems
		Electrical and Computer Engineering Dept.
		UMASS Lowell
PURPOSE
Contains the Robot Manager task which creates and sends commands to the robots 
*/

#include "SerIODriver.h"
#include "RobotMgr.h"
#include "Memmgr.h"
#include "Framer.h"
#include "PktParser.h"
#include "RobotCtrl.h"

/*-----Function Prototypes-----*/
void RobotManagerTask(void *data);
void AckCommand(CPU_INT08U Type);
void AddRobot(CPU_INT08U Robots);
void makemsg(PBuffer *bfr,CPU_INT08U CurrentRobot);
void HereIam(CPU_INT08U Robot,CPU_INT08U x,CPU_INT08U y);
void stop(CPU_INT08U Robot);

/*----- c o n s t a n t    d e f i n i t i o n s -----*/
#define reset 0x00
#define add 0x01
#define st 0x05
#define here 0x09
#define LocationOccupied 13
#define AddBadLocation 12
#define RobotExists 14
#define AddBadRobotAddress 11
#define MoveBadRobotAddress 21
#define MoveNonexistentRobot 22
#define MoveBadLocation 23
#define FollowBadRobotAddress 31
#define FollowNonexistentRobot 32
#define FollowBadLocation 33
#define LoopBadRobotAddress 41
#define LoopNonexistentRobot 42
#define LoopBadLocation 43
#define StopBadRobotAddress 51
#define StopNonexistentRobot 52
#define BadMessageType 61
#define BaudRate 9600 
#define RobotManagerPrio 2
#define SuspendTimeout 0 // Timeout for semaphore wait
#define ROBOTMANAGER_STK_SIZE 128 

/*--------Global Variables---*/
static OS_TCB RobotManagerTCB;
static CPU_STK	RobotManagerStk[ROBOTMANAGER_STK_SIZE];
OS_Q RobotQueue[RobotMax];
CPU_INT08U ctr=0;
CPU_INT08U Floor[39][18];
CPU_INT08U Robot[16];
CPU_BOOLEAN stopRobot[13];
RobotLocations robot;

/*--------------- CreateRobotManagerTask ( ) ---------------*/
/*
PURPOSE
Create and Initialize the robotmanager task.
*/
void CreateRobotManagerTask(void)
{
  OS_ERR osErr;
  OSMutexCreate(&Mutex,"Floor Ctrl",&osErr);
  assert(osErr==OS_ERR_NONE);
  OSTaskCreate(&RobotManagerTCB,                             
               "Robot Manager Task",         
               RobotManagerTask,                 
               NULL,                    
               RobotManagerPrio,            
               &RobotManagerStk[0],         
               ROBOTMANAGER_STK_SIZE / 10,  
               ROBOTMANAGER_STK_SIZE,       
               0,                       
               0,                       
               NULL,                    
               (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
               &osErr);
  
  assert(osErr == OS_ERR_NONE);
}

/*--------------- RobotManagerTask ( ) ---------------*/
/*
PURPOSE
Receive the payload decode the message and send it to Robots.

INPUT PARAMETERS
data  -- pointer to data that can be passed while creating the task(Not Used)
*/
void RobotManagerTask(void *data)
{
    OS_ERR osErr;

  PBuffer	*iBfr = NULL;	/* -- Current input buffer */
  OS_MSG_SIZE msgSize;

  for(;;)
  {
    /* If input buffer is not assigned, get a filled buffer from the parser queue. */
    if (iBfr == NULL)
    {
      iBfr =  OSQPend(&ParserQueue,
                      0,
                      OS_OPT_PEND_BLOCKING,
                      &msgSize,
                      NULL,
                      &osErr);
      assert(osErr==OS_ERR_NONE);
    }
    payload *payloadBfr = (payload *)iBfr->bfr;
    switch(payloadBfr->msgType)
    {
    case reset:
      NVIC_GenerateCoreReset();  //Reset command
      break;
    case add:
      robot.RobotAddress=payloadBfr->cmdExt.msgcmd.RobotAddress;
      robot.xLocation=payloadBfr->cmdExt.msgcmd.Pos->X;
      robot.yLocation=payloadBfr->cmdExt.msgcmd.Pos->Y;
      //Check for the correct locations and add robot
      if(robot.RobotAddress<=16) 
      {
        if(Robot[robot.RobotAddress]!=1)
        {
          if(robot.xLocation<=39 && robot.xLocation<=18)
          {
            if(Floor[robot.xLocation][robot.yLocation]==0)
            {
              CreateRobot(robot);
              Robot[robot.RobotAddress]=1;
              Floor[robot.xLocation][robot.yLocation]=1;
              AckCommand(payloadBfr->msgType);
            }
            else
              ErrorState(LocationOccupied);
          }
          else
            ErrorState(AddBadLocation);
        }
        else
          ErrorState(RobotExists);
      }
      else
        ErrorState(AddBadRobotAddress);  
       Free(iBfr);
      break;
    case move:
      //Check for the correct locations and send the move command to robotctrl
      if(payloadBfr->cmdExt.msgcmd.RobotAddress<=16)
      {
        if(Robot[payloadBfr->cmdExt.msgcmd.RobotAddress])
        {
          if(payloadBfr->cmdExt.msgcmd.Pos->X<=39 && payloadBfr->cmdExt.msgcmd.Pos->Y<=18)
          {
            makemsg(iBfr,payloadBfr->cmdExt.msgcmd.RobotAddress);
            AckCommand(payloadBfr->msgType);
          }
          else
            ErrorState(MoveBadLocation);
        }
        else 
          ErrorState(MoveNonexistentRobot);
      }
      else
        ErrorState(MoveBadRobotAddress);
      break;
   case followpath:
     //Check for correct locations and send the followpath command to the robotcrtl
      if(payloadBfr->cmdExt.msgcmd.RobotAddress<=16)
      {
        if(Robot[payloadBfr->cmdExt.msgcmd.RobotAddress])
        {
          if(payloadBfr->cmdExt.msgcmd.Pos->X<=39 && payloadBfr->cmdExt.msgcmd.Pos->Y<=18)
          {
            makemsg(iBfr,payloadBfr->cmdExt.msgcmd.RobotAddress);
            AckCommand(payloadBfr->msgType);
          }
          else
            ErrorState(FollowBadLocation);
        }
        else 
          ErrorState(FollowNonexistentRobot);
      }
      else
        ErrorState(FollowBadRobotAddress);
      break;
    case loop:
      //Check for correct locations and send the loop command to the robot ctrl
      if(payloadBfr->cmdExt.msgcmd.RobotAddress<=16)
      {
        if(Robot[payloadBfr->cmdExt.msgcmd.RobotAddress])
        {
          if(payloadBfr->cmdExt.msgcmd.Pos->X<=39 && payloadBfr->cmdExt.msgcmd.Pos->Y<=18)
          {
            makemsg(iBfr,payloadBfr->cmdExt.msgcmd.RobotAddress);
            AckCommand(payloadBfr->msgType);
          }
          else
            ErrorState(LoopBadLocation);
        }
        else 
          ErrorState(LoopNonexistentRobot);
      }
      else
        ErrorState(LoopBadRobotAddress);
      break;
    case here:
      //Here i am command posted to the mailbox
      payloadBfr->cmdExt.msgcmd.Pos->Y=payloadBfr->cmdExt.msgcmd.Pos->X;
      payloadBfr->cmdExt.msgcmd.Pos->X=payloadBfr->cmdExt.msgcmd.RobotAddress;
      HereIam(payloadBfr->srcAddr,payloadBfr->cmdExt.msgcmd.Pos->X,payloadBfr->cmdExt.msgcmd.Pos->Y);
      Free(iBfr);
      break;
    case st:
      //Stop command posted to the mailbox
       AckCommand(0x05);
       if(payloadBfr->cmdExt.msgcmd.RobotAddress<=16)
      {
        if(Robot[payloadBfr->cmdExt.msgcmd.RobotAddress])
        {
          stop(payloadBfr->cmdExt.msgcmd.RobotAddress);
        }
        else
          ErrorState(StopNonexistentRobot);
      }
        else
            ErrorState(StopBadRobotAddress);
       Free(iBfr);
       break;
       
    default:
      ErrorState(6);
    }
    iBfr=NULL;
  }
}

/*--------------- AckCommand( ) ---------------*/
/*
PURPOSE
Creates a packet of type acknowledgement and keeps it in the buffer.

INPUT PARAMETERS
Type  -- Type of packet that is being acknowledged
*/
void AckCommand(CPU_INT08U Type)
{
  PBuffer *oBfr = NULL;
  if (oBfr == NULL)
    oBfr = Allocate();
  AddByte(oBfr,Type);
  AddByte(oBfr,0x0A);
  OS_ERR osErr;
  OSQPost(&FramerQueue,oBfr, sizeof(PBuffer), OS_OPT_POST_FIFO, &osErr);
  assert(osErr==OS_ERR_NONE);
}


/*--------------- makemsg( ) ---------------*/
/*
PURPOSE
Make a message to be sent to the robots.

INPUT PARAMETERS
bfr  -- the address of the buffer
CurrentRobot - Address of the Robot
*/
void makemsg(PBuffer *bfr,CPU_INT08U CurrentRobot)
{
  OS_ERR osErr;
  OSQPost(&RobotQueue[CurrentRobot],bfr,sizeof(PBuffer),OS_OPT_POST_FIFO, &osErr);
  assert(osErr==OS_ERR_NONE);
}

/*--------------- stop( ) ---------------*/
/*
PURPOSE
Creates a stop message and keeps it in the approriate robot mailbox.

INPUT PARAMETERS
Robot  -- Address of the Robot
*/

void stop(CPU_INT08U Robot)
{
  PBuffer *oBfr = NULL;
  if (oBfr == NULL)
    oBfr = Allocate();
  AddByte(oBfr,st);
  AddByte(oBfr,Robot);
  OS_ERR osErr;
  OSQPost(&MQueue[Robot],oBfr, sizeof(PBuffer), OS_OPT_POST_FIFO, &osErr);
  assert(osErr==OS_ERR_NONE); 
}

/*--------------- HereIam( ) ---------------*/
/*
PURPOSE
Creates a HereIam message and keeps it in the appropriate mailbox.

INPUT PARAMETERS
Robot -- the address of the robot
x     -- X location of the robot
y     -- Y location of the robot
*/
void HereIam(CPU_INT08U Robot,CPU_INT08U x,CPU_INT08U y)
{
  PBuffer *oBfr = NULL;
  if (oBfr == NULL)
    oBfr = Allocate();
  AddByte(oBfr,here);
  AddByte(oBfr,Robot);
  AddByte(oBfr,x);
  AddByte(oBfr,y);
  OS_ERR osErr;
  OSQPost(&MQueue[Robot],oBfr, sizeof(PBuffer), OS_OPT_POST_FIFO, &osErr);
  assert(osErr==OS_ERR_NONE); 
}

