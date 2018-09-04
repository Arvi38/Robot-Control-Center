/*            Errors.c             */
/*  Aravind Dhulipalla                SID: 01684798 */
#include "includes.h"
#include "error.h"

void ErrorType(CPU_INT08S errorNumber)
{ 
  switch(errorNumber)
  {
  case P1Error:
      SerPrintf("\a*** ERROR: Bad Preamble Byte 1 \n");
      break;
      
  case P2Error:
    
      SerPrintf("\a***ERROR: Bad Preamble Byte 2 \n");
      break;
    
  case P3Error:
    
      SerPrintf("\a***ERROR: Bad Preamble Byte 3\n");
      break;
    
  case PacketSizeError:
    
      SerPrintf("\a***ERROR: Bad Packet Size \n");
      break;
        
  case CheckSumError:
    
       SerPrintf("\n\a***ERROR: Checksum Error\n");
  }
}