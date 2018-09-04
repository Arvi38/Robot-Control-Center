/*      Errors.h     */
/*  Aravind Dhulipalla                SID: 01684798 */
#ifndef error_H
#define error_H
#include "RobotMgr.h"
//-------Error Codes--------
#define P1Error -1  
#define P2Error -2  
#define P3Error -3
#define CheckSumError -4
#define PacketSizeError -5

//-------------Function Prototypes-----
void ErrorType(CPU_INT08S errorNumber);

#endif