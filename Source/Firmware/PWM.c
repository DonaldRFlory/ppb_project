//--------------------------------------------------------------------------
//	PWM.C 
//	PWM control functions.
//	TMR32B1	used for PWM0 and PWM1 setting drive for servo channels 0 and 1
//	TMR16B0	used for PWM2 and PWM3 setting drive for servo channels 2 and 3
//	TMR16B1 used for tuning position PWM out. Filtered to get DC voltage.
//
//--------------------------------------------------------------------------

#include "target.h"
#include "type.h"
#include "pwm.h"
#include "HWDefs.h"



void SetDir(U8 ServoIndex, U8 Direction)
{
	if(Direction)
	{
		switch(ServoIndex)
		{
			case 0:
				SetServ0Dir();
				break;

			case 1:
				SetServ1Dir();
				break;
		
			case 2:
				SetServ2Dir();
				break;
		
			case 3:
				SetServ3Dir();
				break;
		}
	}
	else
	{
		switch(ServoIndex)
		{
			case 0:
				ClrServ0Dir();
				break;

			case 1:
				ClrServ1Dir();
				break;
		
			case 2:
				ClrServ2Dir();
				break;
		
			case 3:
				ClrServ3Dir();
				break;
		}
	}
}

//Accept 0 to 2000 for 0 to 100%
void SetPWM(U8 PWMIndex, U16 PWMVal)//PWMVal is Pct x 20
{
	if(PWMVal > PWM_PERIOD_COUNT + 1)
	{
		PWMVal = PWM_PERIOD_COUNT + 1;
	}
	PWMVal = (PWM_PERIOD_COUNT + 1) - PWMVal;
	switch(PWMIndex)
	{
		case 0:
			LPC_TMR16B1->MR0 = PWMVal;   	//PWM4  //swapped PWM4 to use for channel 0
			break;
					
		case 1:
			LPC_TMR32B0->MR3 = PWMVal;	//PWM1
			break;
		
		case 2:
			LPC_TMR16B0->MR0 = PWMVal;	//PWM2
			break;
		
		case 3:
			LPC_TMR16B0->MR1 = PWMVal;   	//PWM3
			break;

		case 4:
			LPC_TMR32B0->MR2 = PWMVal;	//PWM0
			break;
	}
}
		

//--------------------------------------------------------------------------
//	PWMInit
//
//	Set up two timers for PWM.
//	CT32B1 controls	PWM0 and PWM1, MAT1 is PWO0, MAT3 is PWM1.
//	CT16B0 controls PWM2 and PWM3, MAT0 is PWM2, MAT1 is PWM3.
//	CT16B1 controls PWM4  (MAT0).
//   Selection was based on pinout availability/pinshare issues
//   and fact that max PWMs on any one timer is 3.
//
//Later on we will set TMR32B1 match register 1 to interrupt on match to
//time the change of Dir0-Dir3
//--------------------------------------------------------------------------
void PWMInit(void)
{	
	// Enable the clock respectively for LPC_TMR32B0, LPC_TMR16B0, and IO config block
	LPC_SYSCON->SYSAHBCLKCTRL |= ((1<<9) | (1<<7) | (1<<8) | (1<<16));

	//Need to set PWM  pin modes for match output
  	LPC_IOCON->PIO0_1 = 2;	//PIO0.1 CT32B0_Mat2 (PWM0)
  	LPC_IOCON->R_PIO0_11 = 3;		//PIO0.11 CT32B0_Mat3 (PWM1)

  	LPC_IOCON->PIO0_8 = 2;	// PIO0.8,  CT16B0_Mat0 (PWM2)
  	LPC_IOCON->PIO0_9 = 2;	// PIO0.9, CT16B0_Mat1	(PWM3)

  	LPC_IOCON->PIO1_9 = 1;	//PIO1.9 CT16B1_Mat0 (PWM4)

   	//Prescale by unity
  	LPC_TMR16B0->PR = 0;
  	LPC_TMR16B1->PR = 0;
  	LPC_TMR32B0->PR = 0;

	// Set Initial Duty Cycles 
	LPC_TMR32B0->MR2 = PWM_PERIOD_COUNT + 1;   	// PWM0 0%
	LPC_TMR32B0->MR3 = PWM_PERIOD_COUNT + 1;   	// PWM1 0%
	LPC_TMR16B0->MR0 = PWM_PERIOD_COUNT + 1;   	// PWM2 0%
	LPC_TMR16B0->MR1 = PWM_PERIOD_COUNT + 1;   	// PWM3 0%

	LPC_TMR16B1->MR0 = PWM_PERIOD_COUNT + 1;   	// PWM4 0%

	// Set Periods 
	LPC_TMR16B0->MR2 = PWM_PERIOD_COUNT;
	LPC_TMR16B1->MR2 = PWM_PERIOD_COUNT;
	LPC_TMR32B0->MR0 = PWM_PERIOD_COUNT;

  	// Configure match control registers 
  	LPC_TMR16B0->MCR = 1<<7; //Reset on MR2
	LPC_TMR16B1->MCR = 1<<7; //Reset on MR2
	LPC_TMR32B0->MCR = 2; //Reset on MR0

  	// Enable PWMS 
  	LPC_TMR16B0->PWMC = (1<<0) | (1<<1); //MAT0 and MAT1 enabled as PWM
  	LPC_TMR16B1->PWMC = (1<<0); //MAT0 as PWM
	LPC_TMR32B0->PWMC = (1<<2) | (1<<3);//MAT2 and MAT3 enabled as PWM;

   	// Enable the two timers 
  	LPC_TMR16B0->TCR = 1;
  	LPC_TMR16B1->TCR = 1;
  	LPC_TMR32B0->TCR = 1;

	//DFDEBUG
	return;
}
		

//--------------------------------------------------------------------------
//	DirectionInit
//
//	 Set up direction control pins for the four potential servo controls.
//   Selection was based on pinout availability/pinshare issues
//   PIO2.4 is Dir0, PIO2.5 is Dir1, PIO2.6 is Dir2, and PIO2.7 is DIR3.
//
//--------------------------------------------------------------------------
void DirectionInit(void)
{
   //Servo Direction pins
	LPC_GPIO0->DIR |= (1<<2) | (1<<7); //set Port 0 DirN pins to output
	LPC_GPIO1->DIR |= (1<<5) | (1<<8); //set Port 1 DirN pins to output
	LPC_IOCON->PIO0_2 = 0xD0; //GPIO, pull-down when set for input (Dir0)
	LPC_IOCON->PIO0_7 = 0xD0; //GPIO, pull-down when set for input (Dir2)
	LPC_IOCON->PIO1_5 = 0xD0; //GPIO, pull-down when set for input (Dir3)
	LPC_IOCON->PIO1_8 = 0xD0; //GPIO, pull-down when set for input (Dir4)

}



//***************************************************************************
//	         END FILE:   PWM.c
//***************************************************************************
