/*=============== RobotCtrl . h ===============*/
/*
BY:	Aravind Dhulipalla
		16.472 / 16.572 Embedded Real Time Systems
		Electrical and Computer Engineering Dept.
		UMASS Lowell
PURPOSE
Interface to RobotCtrl.c
*/
#ifndef RobotCtrl_H
#define RobotCtrl_H

/*----- c o n s t a n t    d e f i n i t i o n s -----*/
#define RobotMax 17 //Maximum robot address


/*----- t y p e    d e f i n i t i o n s -----*/

// Robot Locations type contains the robot data
typedef struct
{
  CPU_INT08U RobotAddress;
  CPU_INT08U xLocation;
  CPU_INT08U yLocation;
  CPU_INT08U Nextx;
  CPU_INT08U Nexty;
  CPU_BOOLEAN STOP;
  CPU_INT08U Retry;
}RobotLocations; 

/*-----Global Variables-----*/
extern OS_MUTEX Mutex;
extern OS_Q     MQueue[RobotMax];
extern OS_Q     RobotQueue[RobotMax];

/*------Function Prototypes----*/
void CreateRobot(RobotLocations Robots);
#endif