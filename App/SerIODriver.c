/*=============== S e r I O D r i v e r . c ===============*/
/*
BY:	Aravind Dhulipalla
		16.472 / 16.572 Embedded Real Time Systems
		Electrical and Computer Engineering Dept.
		UMASS Lowell
PURPOSE
The serial driver for UART communication.

*/
#include "SerIODriver.h"

//----- c o n s t a n t    d e f i n i t i o n s -----
#define USART_TXEIE 0x80        // To Unmask Tx Interrupts
#define USART_TXE   0x80        // Tx Empty Bit
#define USART_RXNE   0x20       //Rx Empty Bit
#define USART_RXNEIE 0x20       //To Unmask Rx Interrupts

// IRQ38 Definitions
#define SETENA1   (*((CPU_INT32U *) 0XE000E104))
#define USART2ENA 0x00000040   

#define SuspendTimeout 0	    // Timeout for semaphore wait

//----- g l o b a l    v a r i a b l e s -----
static OS_SEM	openOBfrs;
static OS_SEM   closedIBfrs;
static BfrPair iBfrPair;
static CPU_INT08U iBfr0Space[BfrSize];
static CPU_INT08U iBfr1Space[BfrSize];
static BfrPair oBfrPair;
static CPU_INT08U oBfr0Space[BfrSize];
static CPU_INT08U oBfr1Space[BfrSize];


/*--------------- I n i t S e r I O ( ) ---------------*/

/*
PURPOSE
Initialize the serial I/O driver module by ceating the txDone semaphore and
unmasking the Tx interrupt.
*/

void InitSerIO(void)
{
  OS_ERR  osErr;  // O/S Error Code
  
  BfrPairInit(&iBfrPair,iBfr0Space,iBfr1Space,BfrSize); /*Initialise the input buffer pair*/
  BfrPairInit(&oBfrPair,oBfr0Space,oBfr1Space,BfrSize); /*Initialise the output buffer pair */
  OSSemCreate(&openOBfrs, "open obfrs",1, &osErr);	/*Create openOBfrs Semaphone*/
  OSSemCreate(&closedIBfrs, "closed ibfrs",0, &osErr);  /*Create closedIBfrs Semaphore*/
  assert(osErr == OS_ERR_NONE);
  // Unmask Tx interrupts. 
  USART2->CR1 |= USART_TXEIE;
  //Unmask Rx interrupts
  USART2->CR1 |=USART_RXNEIE;
  
  // Enable IRQ38.
  SETENA1 = USART2ENA;
}

/*--------------- P u t B y t e ( ) ---------------*/

/*
PURPOSE
Output 1 character to USART2.

INPUT PARAMETERS
c-- the character to transmit
*/
CPU_INT16S PutByte(CPU_INT16S c)
{
  OS_ERR		osErr;		/* -- Semaphore error code */
  if(PutBfrClosed(&oBfrPair))
  {
    /* Wait for xmit done and buffer not empty. */
    OSSemPend(&openOBfrs, SuspendTimeout, OS_OPT_PEND_BLOCKING, NULL, &osErr);		
    assert(osErr==OS_ERR_NONE);
    BfrPairSwap(&oBfrPair);
  }
  CPU_INT16S Char = PutBfrAddByte(&oBfrPair,c);
  USART2->CR1 |= USART_TXEIE;   //Unmask Tx Interrupts
  return Char ;
}

/*--------------- ServiceTX ( ) --------*/
void ServiceTx(void)
{
  OS_ERR osErr;
  
  if (!(USART2->SR & USART_TXE))
  {
    return;
  }
  if (!GetBfrClosed(&oBfrPair))
  {
    USART2->CR1 &= ~USART_TXEIE;
    return;
  }
  USART2->DR = (CPU_INT08U)GetBfrRemByte(&oBfrPair);
  if(!GetBfrClosed(&oBfrPair))
  {
    OSSemPost(&openOBfrs, OS_OPT_POST_1, &osErr); //Posts 1 to the semaphore openOBfrs
    assert(osErr==OS_ERR_NONE);
  }
}

/*------------------GetByte()-----------*/
CPU_INT16S GetByte(void)
{
  OS_ERR osErr;		/* -- Semaphore error code */
  if(!GetBfrClosed(&iBfrPair))
  {
    /* Wait for xmit done and buffer not empty. */
    OSSemPend(&closedIBfrs, SuspendTimeout, OS_OPT_PEND_BLOCKING, NULL, &osErr);	
    assert(osErr==OS_ERR_NONE);
    BfrPairSwap(&iBfrPair);
  }		
  CPU_INT16S c =GetBfrRemByte(&iBfrPair);
  USART2->CR1 |= USART_RXNEIE;
  return c;
}
/*------------------ServiceRx()----------*/
void ServiceRx(void)
{
  OS_ERR os_Err;
  if (!(USART2->SR &USART_RXNE))
  {
    return;
  }
  if (PutBfrClosed(&iBfrPair))
  {
    USART2->CR1 &= ~USART_RXNEIE;
    return;
  }
  PutBfrAddByte(&iBfrPair, (CPU_INT16S) USART2->DR);
  if(PutBfrClosed(&iBfrPair))
  {
    OSSemPost(&closedIBfrs, OS_OPT_POST_1, &os_Err);
    assert(os_Err==OS_ERR_NONE);
  }
}

/*--------------- I S R ( ) ---------------*/

/*
PURPOSE
Interrupt handler. 
Signal on tx interrupt to indicate tx ready for data.
*/
void ISR(void)
{
  
  /* Disable interrupts. */
  CPU_SR_ALLOC();
  OS_CRITICAL_ENTER();  
  
  /* Tell kernel we're in an ISR. */
  OSIntEnter();
  
  /* Enable interrupts. */
  OS_CRITICAL_EXIT();
  
  ServiceRx();
  ServiceTx();
  /* Give the O/S a chance to swap tasks. */
  OSIntExit ();
  //return;
}

void TxFlush(void)
{
  OS_ERR      osErr; 

  ClosePutBfr(&oBfrPair);

  OSSemPend(&openOBfrs, 0, OS_OPT_PEND_BLOCKING, NULL, &osErr);
  assert(osErr==OS_ERR_NONE);

  BfrPairSwap(&oBfrPair);

  USART2->CR1 |= USART_TXEIE;
}
