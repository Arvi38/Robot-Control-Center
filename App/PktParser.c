/*=============== P k t P a r s e r .c ===============
BY:	Aravind Dhulipalla
		16.472 / 16.572 Embedded Real Time Systems
		Electrical and Computer Engineering Dept.
		UMASS Lowell
		
Purpose :
Takes the packet and gives the payload to the appropriate queue 

*/

#include "PktParser.h"
#include "includes.h"
#include "MemMgr.h"

//----- c o n s t a n t    d e f i n i t i o n s -----
#define p1Char 0x03
#define p2Char 0xaF
#define p3Char 0xeF
#define HeaderLength 5
#define MaxLength 8
#define checkSumbit 0
#define SuspendTimeout 0
#define ParserPrio 6 // Parser task priority

/* Size of the Process task stack */
#define	PARSER_STK_SIZE     256 

/*------Function Prototypes----*/
CPU_VOID ParsePkt(CPU_VOID *data);

/*----- g l o b a l s -----*/
// Process Task Control Block
static OS_TCB parserTCB;
OS_Q	ParserQueue;
OS_Q    FramerQueue;

/* Stack space for Process task stack */
static CPU_STK parserStk[PARSER_STK_SIZE];
typedef enum  {p1,p2,p3,L,D,C,Er} parserstate;//Packet Parser states

/*----- C r e a t e P a r s e r T a s k ( ) -----

PURPOSE
Create and initialize the Parser Task.
*/
CPU_VOID CreateParserTask(CPU_VOID)
{
  OS_ERR		osErr;/* -- OS Error code */
  
  /* Create Parser task. */	
  OSTaskCreate(&parserTCB,
               "Parser Task",
               ParsePkt, 
               NULL,
               ParserPrio,
               &parserStk[0],
               PARSER_STK_SIZE / 10,
               PARSER_STK_SIZE,
               0,
               100,
               0,
               (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
               &osErr);
   assert(osErr == OS_ERR_NONE);
   //Create Parser Queue
   OSQCreate(&ParserQueue, "Parser Queue",PoolSize, &osErr);
   assert(osErr == OS_ERR_NONE);
   //Create Framer Queue
   OSQCreate(&FramerQueue, "Framer Queue",PoolSize, &osErr);
   assert(osErr == OS_ERR_NONE);
}

/*-----ParsePkt()------
Purpose
Takes the packet and puts the payload in the appropriate buffer queue
*/
CPU_VOID ParsePkt(CPU_VOID *data)
{
  PBuffer *iBfr = NULL;
  static  parserstate  parsestate=p1;
  CPU_INT16S  c;
  CPU_INT08U i=0;
  CPU_INT08U checkSum;
  for(;;)
  {
    c = GetByte(); 
     if (iBfr == NULL)
     iBfr = Allocate();
    if(c>=0)
    {  
      switch(parsestate)
      {
      case p1:
        if(c == p1Char) 
        {
          parsestate = p2;
          checkSum = c;
        } 
        else
        {
           ErrorState(1); 
          parsestate=Er;
        }
        break;
      case p2:
        if(c == p2Char) 
        {
          parsestate=p3;
          checkSum=checkSum^c;
        }
        else
        {
            ErrorState(2); 
          parsestate = Er;
        }
        break;
      case p3:
        if(c == p3Char) 
        {
          parsestate=L ;
          checkSum=checkSum^c;
        }
        else
        {
           ErrorState(3); 
          parsestate = Er;
        }
        break;
      case L:
        if(c<MaxLength)
        {
           ErrorState(5); 
          parsestate=Er;
        }
        else
        {
          AddByte(iBfr, (c - HeaderLength));    
          checkSum=checkSum^c;
          parsestate = D;
          i=0;
        }
        break;
      case D:
        i++; 
        AddByte(iBfr,c);   
        checkSum=checkSum^c;
        if(i >= iBfr->bfr[0])
        {
          parsestate = C;
        }      
        break;
      case C:
        checkSum=checkSum^c;
        if(checkSum==checkSumbit)
        {
          parsestate=p1;
          OS_ERR osErr;
          OSQPost(&ParserQueue, iBfr, sizeof(PBuffer), OS_OPT_POST_FIFO, &osErr);
          assert(osErr==OS_ERR_NONE);
          // Indicate that a new input buffer is needed.
          iBfr = NULL; 
  
        }
        else
        {
           ErrorState(4); 
          parsestate=Er;
        }
        break;
      case Er:
        if(c==p1Char)        /*it checks for p1 and p2 sync bytes and if found goes to case p3 */
        {
          checkSum=c;
          c = GetByte();
          if(c>0)
          {
            if(c==p2Char)
            {
              parsestate=p3;
              checkSum=checkSum^c; 
            }
          }
        }
        break;
      }  
    }
  }
}

/*--------------- ErrorState( ) ---------------*/

/*
PURPOSE
Make a error packet and keep it in the framer queue.

INPUT PARAMETERS
ErrorNumber  -- the type of error that is detected
*/
void ErrorState(CPU_INT08U ErrorNumber)
{
    PBuffer *iBfr = NULL;
   if (iBfr == NULL)
     iBfr = Allocate();
   AddByte(iBfr,ErrorNumber);
   AddByte(iBfr,0x0B);
   OS_ERR osErr;
   OSQPost(&FramerQueue,iBfr, sizeof(PBuffer), OS_OPT_POST_FIFO, &osErr);
   assert(osErr==OS_ERR_NONE);
   iBfr=NULL;
}