/*=============== RobotCtrl . c ===============*/
/*
BY:	Aravind Dhulipalla
		16.472 / 16.572 Embedded Real Time Systems
		Electrical and Computer Engineering Dept.
		UMASS Lowell
PURPOSE
Contains the Robot task function and makes the decision where to step 

*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "assert.h"
#include "includes.h"
#include "PktParser.h"
#include "RobotMgr.h"
#include "PBuffer.h"
#include "Memmgr.h"
#include "Framer.h"
#include "RobotCtrl.h"

/*----Constant Definitions------*/
#define North 1
#define NorthEast 2
#define NorthWest 8
#define West 7
#define East 3
#define SouthWest 6
#define South 5
#define SouthEast 4
#define NoStep 0
#define RobotPrio 1
#define ROBOT_STK_SIZE 256

/*-------Hereiam datatype----*/
typedef struct
{
  CPU_INT08U Addr;
  CPU_INT08U RAddr;
  CPU_INT08U x;
  CPU_INT08U y;
}Here; //for the Hereiam and stop message 

/*------Globals----*/
CPU_STK RobotStk[RobotMax][ROBOT_STK_SIZE];//RobotStack
OS_TCB RobotTCB[RobotMax];//Task Ctrl Block
OS_MUTEX Mutex; //Mutex
OS_Q MQueue[RobotMax];//Maibox Queue
CPU_INT08U direction; 
OS_ERR osErr;
RobotLocations RobotLocation[RobotMax];

/*------------Function Prototypes---*/
CPU_INT08U Step(CPU_INT08U cx,CPU_INT08U cy,CPU_INT08S X,CPU_INT08S Y);
void HereIAM(CPU_INT08U CurrentRobot);
void RobotTask(void *data);void StepRobot(CPU_INT08U Robot,CPU_INT08U x,CPU_INT08U y);


/*--------------- CreateRobot( ) ---------------*/

/*
PURPOSE
Creates and initializes the robot task

INPUT PARAMETERS
Robots  -- data of type RobotLocations
*/
void CreateRobot(RobotLocations Robots)
{
  OS_ERR osErr;
  //Create Robot Queues
  OSQCreate(&RobotQueue[Robots.RobotAddress], "Robot Queue",PoolSize, &osErr);
  assert(osErr == OS_ERR_NONE);
  //Create Maibox
  OSQCreate(&MQueue[Robots.RobotAddress],"MailBox Queue",1,&osErr);
  assert(osErr == OS_ERR_NONE);
  //Create Robot Ctrl Task
  OSTaskCreate(&RobotTCB[Robots.RobotAddress],                             
               "RobotTask",         
               RobotTask,                 
               &Robots,                    
               RobotPrio,            
               &RobotStk[Robots.RobotAddress][256],         
               ROBOT_STK_SIZE/ 10,  
               ROBOT_STK_SIZE,       
               0,                       
               0,                       
               0,                    
               (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
               &osErr);
  
  assert(osErr == OS_ERR_NONE);
  RobotLocation[Robots.RobotAddress].xLocation = Robots.xLocation;
  RobotLocation[Robots.RobotAddress].yLocation = Robots.yLocation;
}


/*--------------- RobotTask ( ) ---------------*/

/*
PURPOSE
The task function of Robots

INPUT PARAMETERS
data  -- paointer to the data that is passed while task Robot data is passed)
*/
void RobotTask(void *data)
{
  //converting optional task control block into type RobotLocations
  RobotLocations Robot = *(RobotLocations *)data;
  OS_MSG_SIZE msgSize;
  OS_ERR osErr;
  PBuffer *iBfr = NULL;
  for(;;)
  {
    if(iBfr==NULL)
    {
      iBfr =  OSQPend(&RobotQueue[Robot.RobotAddress],
                      0,
                      OS_OPT_PEND_BLOCKING,
                      &msgSize,
                      NULL,
                      &osErr);
      assert(osErr==OS_ERR_NONE);
    }
    payload *Robots = (payload *)iBfr->bfr;
    switch(Robots->msgType)
    {
    case move:   
      RobotLocation[Robots->cmdExt.msgcmd.RobotAddress].Nextx = Robots->cmdExt.msgcmd.Pos[0].X;
      RobotLocation[Robots->cmdExt.msgcmd.RobotAddress].Nexty = Robots->cmdExt.msgcmd.Pos[0].Y;
      StepRobot(Robots->cmdExt.msgcmd.RobotAddress,Robots->cmdExt.msgcmd.Pos[0].X,Robots->cmdExt.msgcmd.Pos[0].Y);
      break;
    case followpath:
      for(CPU_INT08U a=0;a<=(Robots->payloadLen-5)/2;a++)
      {
        StepRobot(Robots->cmdExt.msgcmd.RobotAddress,Robots->cmdExt.msgcmd.Pos[a].X,Robots->cmdExt.msgcmd.Pos[a].Y);
      }
      break;
    case loop:
      while(RobotLocation[Robots->cmdExt.msgcmd.RobotAddress].STOP==FALSE)
      {
        for(CPU_INT08U b=0;b<=(Robots->payloadLen-5)/2;b++)
        {
          StepRobot(Robots->cmdExt.msgcmd.RobotAddress,Robots->cmdExt.msgcmd.Pos[b].X,Robots->cmdExt.msgcmd.Pos[b].Y);
        }
        StepRobot(Robots->cmdExt.msgcmd.RobotAddress,Robots->cmdExt.msgcmd.Pos[0].X,Robots->cmdExt.msgcmd.Pos[0].Y);
      
      }
      RobotLocation[Robots->cmdExt.msgcmd.RobotAddress].STOP=FALSE;
      break;
    }
   
    Free(iBfr);
    iBfr=NULL;
  }
}

/*--------------- StepRobot( ) ---------------*/
/*
PURPOSE
Checks the Floor locations if they are empty or free also updtaes floor location if a robot is moved

INPUT PARAMETERS
Robot  -- Address of the Robot
x	   -- X location where the robot should be moved next]
y      -- Y location where the robot should be moved next
*/
void StepRobot(CPU_INT08U Robot,CPU_INT08U x,CPU_INT08U y)
{
  PBuffer *oBfr = NULL;
  while((RobotLocation[Robot].xLocation !=x || RobotLocation[Robot].yLocation!=y) && !RobotLocation[Robot].STOP)
  {
    CPU_INT08U CurrentX=RobotLocation[Robot].xLocation;
    CPU_INT08U CurrentY = RobotLocation[Robot].yLocation;
    CPU_INT08S dif1=x-CurrentX;
    CPU_INT08S dif2=y-CurrentY;
    if(oBfr==NULL)
      oBfr=Allocate(); 
    OSMutexPend(&Mutex,NULL,OS_OPT_PEND_BLOCKING,NULL,&osErr);
    CPU_INT08U dir = Step(CurrentX,CurrentY,dif1,dif2);
    AddByte(oBfr,dir);
    AddByte(oBfr,0x07);
    AddByte(oBfr,Robot);
    OS_ERR osErr;
    OSQPost(&FramerQueue,oBfr, sizeof(PBuffer), OS_OPT_POST_FIFO, &osErr);
    assert(osErr==OS_ERR_NONE);
    if(!dir==0)
     Floor[RobotLocation[Robot].xLocation][RobotLocation[Robot].yLocation]=0;
        else
      RobotLocation[Robot].Retry = RobotLocation[Robot].Retry+1;
    HereIAM(Robot);
    OSMutexPost(&Mutex,OS_OPT_POST_NONE,&osErr);
    if(RobotLocation[Robot].Retry>=10)
    {
      ErrorState(100+Robot);
    }
    oBfr=NULL;
  }
  if(RobotLocation[Robot].STOP)
  {
    PBuffer *ofBfr=NULL;
    if(ofBfr==NULL)
      ofBfr=Allocate(); 
    AddByte(ofBfr,0);
    AddByte(ofBfr,0x07);
    AddByte(ofBfr,Robot);
    OS_ERR osErr;
    OSQPost(&FramerQueue,ofBfr, sizeof(PBuffer), OS_OPT_POST_FIFO, &osErr);
    assert(osErr==OS_ERR_NONE);
    HereIAM(Robot);
  }
}

/*--------------- Step( ) ---------------*/
/*
PURPOSE
Makes a decision on which direction to step 

INPUT PARAMETERS
cx	   -- Current x location where the robot is
cy      -- Current y location where the robot is
X       -- Next x location where the robot should be moved
Y      --  Next Y location where the robot should be moved

ReturnValue
Direction in which the step should be made
*/

CPU_INT08U Step(CPU_INT08U cx,CPU_INT08U cy,CPU_INT08S X,CPU_INT08S Y)
{
  if(X==0 && Y>0)
  {
    if(Floor[cx][cy+1]==0)
      direction=1;
    else if(Floor[cx+1][cy+1]==0)
      direction=2;
    else if(Floor[cx-1][cy]==0)
      direction=8;
    else if(Floor[cx-1][cy]==0)
      direction=7;
    else if(Floor[cx+1][cy]==0)
      direction=3;
    else if(Floor[cx][cy-1]==0)
      direction=5;
    else if(Floor[cx-1][cy-1]==0)
      direction=6;
    else if(Floor[cx+1][cy-1]==0)
      direction=4;
    else
      direction=0;
  }
  else if(X==0 && Y<0)
  {
    if(Floor[cx][cy-1]==0)
      direction=5;
    else if(Floor[cx-1][cy-1]==0)
      direction=6;
    else if(Floor[cx+1][cy-1]==0)
      direction=4;
    else if(Floor[cx-1][cy]==0)
      direction=7;
    else if(Floor[cx+1][cy]==0)
      direction=3;
    else if(Floor[cx-1][cy+1]==0)
      direction=8;
    else if(Floor[cx][cy+1]==0)
      direction=1;
    else if(Floor[cx+1][cy+1]==0)
      direction=2;
    else
      direction=0;
  }
  else if(X>0 && Y==0)
  {
    if(Floor[cx+1][cy]==0)
      direction=3;
    else if(Floor[cx+1][cy+1]==0)
      direction=2;
    else if(Floor[cx+1][cy-1]==0)
      direction=4;
    else if(Floor[cx][cy+1]==0)
      direction=1;
    else if(Floor[cx][cy-1]==0)
      direction=5;
    else if(Floor[cx-1][cy+1]==0)
      direction=8; 
    else if(Floor[cx-1][cy]==0)
      direction=7;
    else if(Floor[cx-1][cy-1]==0)
      direction=6;
    else
      direction=0;
  }
  else if(X<0 && Y==0)
  {
    if(Floor[cx-1][cy]==0)
      direction=7;
    else if(Floor[cx-1][cy+1]==0)
      direction=8; 
    else if(Floor[cx-1][cy-1]==0)
      direction=6;
    else if(Floor[cx][cy+1]==0)
      direction=1;
    else if(Floor[cx][cy-1]==0)
      direction=5;
    else if(Floor[cx+1][cy+1]==0)
      direction=2;
    else if(Floor[cx+1][cy]==0)
      direction=3;
    else if(Floor[cx+1][cy-1]==0)
      direction=4;
    else
      direction=0;
  }
  else if(X>0 && Y>0)
  {
    if(Floor[cx+1][cy+1]==0)
        direction=2;
      else if(Floor[cx][cy+1]==0)
        direction=1;
      else if(Floor[cx-1][cy+1]==0)
        direction=8;
      else if(Floor[cx-1][cy]==0)
        direction=7;
      else if(Floor[cx+1][cy]==0)
        direction=3;
      else if(Floor[cx][cy-1]==0)
        direction=5;
      else if(Floor[cx-1][cy-1]==0)
        direction=6;
      else if(Floor[cx+1][cy-1]==0)
        direction=4;
      else
        direction=0;
  }
  else if(X<0 && Y>0)
  {
     if(Floor[cx-1][cy+1]==0)
        direction=8;
      else if(Floor[cx][cy+1]==0)
        direction=1;
      else if(Floor[cx-1][cy]==0)
        direction=7;
      else if(Floor[cx+1][cy+1]==0)
        direction=2;
      else if(Floor[cx-1][cy-1]==0)
        direction=6;
      else if(Floor[cx+1][cy]==0)
        direction=3;
      else if(Floor[cx][cy-1]==0)
        direction=5;
      else if(Floor[cx+1][cy-1]==0)
        direction=4;
      else
        direction=0;
  }
  else if(X>0 && Y<0)
  {
   if(Floor[cx+1][cy-1]==0)
        direction=4;
      else if(Floor[cx][cy-1]==0)
        direction=5;
      else if(Floor[cx+1][cy]==0)
        direction=3;
      else if(Floor[cx-1][cy-1]==0)
        direction=6;
      else if(Floor[cx-1][cy]==0)
        direction=7;
      else if(Floor[cx+1][cy+1]==0)
        direction=2;
      else if(Floor[cx][cy+1]==0)
        direction=1;
      else if(Floor[cx-1][cy+1]==0)
        direction=8;
      else
        direction=0;
  }
  else if(X<0 && Y<0)
  {
    if(Floor[cx-1][cy-1]==0)
        direction=6;
      else if(Floor[cx-1][cy]==0)
        direction=7;
      else if(Floor[cx][cy-1]==0)
        direction=5;
      else if(Floor[cx+1][cy-1]==0)
        direction=4;
      else if(Floor[cx+1][cy]==0)
        direction=3;
      else if(Floor[cx-1][cy+1]==0)
        direction=8;
      else if(Floor[cx][cy+1]==0)
        direction=1;
      else if(Floor[cx+1][cy+1]==0)
        direction=2;
      else
        direction=0;
  }
  return direction ;
}

/*--------------- HereIam()------------*/
/*
PURPOSE
To process the HereIam and Stop commands and update the floor locations and robot locations

INPUT PARAMETERS
CurrentRobot - Address of the robot
*/
void HereIAM(CPU_INT08U CurrentRobot)
{
  
  PBuffer *mBfr=NULL;
  OS_MSG_SIZE msgSize;
  OS_ERR osErr;
  
  if (mBfr == NULL)
  {
    mBfr = OSQPend(&MQueue[CurrentRobot],
                   NULL,
                   OS_OPT_PEND_BLOCKING,
                   &msgSize,
                   NULL,
                   &osErr);
    assert(osErr==OS_ERR_NONE);
  }
  Here *Robot = (Here *)mBfr->bfr;
  switch(Robot->Addr)
  {
  case 0x09: //HereIam message
    RobotLocation[CurrentRobot].xLocation= Robot->x;
    RobotLocation[CurrentRobot].yLocation = Robot->y;
    Floor[RobotLocation[Robot->RAddr].xLocation][RobotLocation[Robot->RAddr].yLocation]=1;
    break;
  case 0x05: //stop message
    RobotLocation[CurrentRobot].STOP=TRUE;
    break;
  }
  Free(mBfr);
}
