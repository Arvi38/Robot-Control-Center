/*      Errors.h     */
/*  Aravind Dhulipalla                SID: 01684798 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "assert.h"
#include "includes.h"
#include "error.h"
#include "SerIODriver.h"
#include "RobotMgr.h"
#include "PBuffer.h"
#include "Globals.h"
#include "Memmgr.h"

/*----- c o n s t a n t    d e f i n i t i o n s -----*/
#define BaudRate 9600 
#define	Payload_STK_SIZE     256 
#define PayloadPrio 4
#define SuspendTimeout 0	    // Timeout for semaphore wait
#define PAYLOAD_STK_SIZE 128
static OS_TCB PayloadTCB;
static CPU_STK	PayloadStk[PAYLOAD_STK_SIZE];
OS_Q RobotQueue;


#pragma Pack(1)
typedef struct /*payload structure datatype*/
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



/*--------------- S e r P r i n t f ( ) ---------------
PURPOSE:
Provides functionality of standard C printf( ) function printing
to the RS232 Tx rather than to the screen.
INPUT PARAMETERS:
format  - Standard printf format string
...     - Zero or more parameters giving values to print according
          to the "format" format string
*/
void SerPrintf(CPU_CHAR *format, ...)
{
#define PrintBfrSize 81
  
  CPU_CHAR buffer[PrintBfrSize];
  CPU_CHAR *ptr;
  va_list vArgs;
  va_start(vArgs, format);
  vsnprintf((char *)buffer,PrintBfrSize,(char const *)format,vArgs);
  va_end(vArgs);
  for (ptr=buffer; *ptr!='\0';)
    PutByte(*ptr++);
  PutByte(*ptr++);
}


void PayloadInit(BfrPair *pBfrPair)
{
  
}
//create payload task
void CreatePayloadTask(void)
{
  OS_ERR osErr;
  OSTaskCreate(&PayloadTCB,                             
               "PayloadTask",         
               PayloadTask,                 
               NULL,                    
               PayloadPrio,            
               &PayloadStk[0],         
               PAYLOAD_STK_SIZE / 10,  
               PAYLOAD_STK_SIZE,       
               0,                       
               0,                       
               NULL,                    
               (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
               &osErr);
  
  assert(osErr == OS_ERR_NONE);
  
}

/*---------PayloadTask()-------------
Purpose : Takes the payload and prints the appropiate message */
void PayloadTask(void *data)
{
  CPU_INT16S c;
  PBuffer		*iBfr = NULL;	/* -- Current input buffer */
  PBuffer		*oBfr = NULL;	/* -- Current output buffer */
  OS_MSG_SIZE msgSize;
  OS_ERR osErr;
  for(;;)
  {
    /* If input buffer is not assigned, get a filled buffer from the input queue. */
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
    /* If no output buffer is assigned, get an empty buffer from the buffer pool. */
    if (oBfr == NULL)
      oBfr = Allocate();
    
    /* Get the next character from the input buffer, and capitalize it. */
    c = RemoveByte(iBfr);
    
    /* If not a vowel, write the character to the output buffer. */
      AddByte(oBfr, c);
    
    /* If the input buffer is empty, return it to the buffer pool. */
    if (Empty(iBfr))
    {
      Free(iBfr);
      iBfr = NULL;
    }
    
    /* If the output buffer is full, send it to the output task. */			
    if (Full(oBfr))
    {
      OSQPost(&RobotQueue, oBfr, sizeof(Buffer), OS_OPT_POST_FIFO, &osErr);
      assert(osErr==OS_ERR_NONE);
      
      oBfr = NULL;
    }
    
    
    
  }
}
