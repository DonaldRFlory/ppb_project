//--------------------------------------------------------------------------
//	MiscHW.C 
//	Miscellaneous hardware init and  control functions.
//	Not fitting any other functional category
//
//
//--------------------------------------------------------------------------
#include "target.h"
#include "type.h"
#include "HWDefs.h"


unsigned short ReadIOPort(unsigned char PortIndex)
{
	switch(PortIndex)
	{
		default:
		case 0:
			return (unsigned short)LPC_GPIO0->DATA;

		case 1:
			return (unsigned short)LPC_GPIO1->DATA;

		case 2:
			return (unsigned short)LPC_GPIO2->DATA;

		case 3:
			return (unsigned short)LPC_GPIO3->DATA;
	}
}

//Setup CT32B0 to count SysClk as up counter.
void InitIntervalTimer(void)
{
	// Enable the clock respectively for LPC_TMR32B0 and IO config block
	LPC_SYSCON->SYSAHBCLKCTRL |= ((1<<9) | (1<<16));
   	//Prescale by unity
  	LPC_TMR32B0->PR = 0;
	LPC_TMR32B0->MCR = 0; //Do nothing on matches
	LPC_TMR32B0->TCR = 0; //Not active
	LPC_TMR32B0->TC = 0; //Clear counter
}

	
//--------------------------------------------------------------------------
//	         END FILE:   MiscHW.c
//--------------------------------------------------------------------------
