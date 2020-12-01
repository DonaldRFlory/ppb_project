//--------------------------------------------------------------------------
//	Quadrature.c 
//	
//	Quadrature decoder and counter functions.
//
//--------------------------------------------------------------------------
#include "target.h"
#include "type.h"
#include "HWDefs.h"

S32 QCount[4];
U32 QErrors[4];
static U16 QLast;

//TT1 is PIO3.0
//TT2 is PIO3.1
//TT3 is PIO3.3
void TelltaleInit(void)
{
	LPC_IOCON->PIO3_0 = 0xD0; //GPIO, 	   TT1
	LPC_IOCON->PIO3_1 = 0xD0; //GPIO, 	   TT2
	LPC_IOCON->PIO3_3 = 0xD0; //GPIO, 	   TT1

	LPC_GPIO3->DIR |= ((1<<0) | (1<<1) | (1<<3)); //PIO0_6, and PIO0_7 output
}

//static  uint32_t Priority;
void QuadratureInit(void)
{
	NVIC_DisableIRQ(EINT0_IRQn);
	NVIC_DisableIRQ(EINT1_IRQn);
	NVIC_DisableIRQ(EINT2_IRQn);
	NVIC_DisableIRQ(EINT3_IRQn);

	//TODO NEED to set up INDEXn input pins and macros to read

	//All Quadrature GPIO IN with hysteresis
	LPC_IOCON->PIO2_0 = 0xE8; //GPIO, pull-down, hysteresis enabled QA0
	LPC_IOCON->PIO2_1 = 0xE8; //GPIO, pull-down, hysteresis enabled QB0

	LPC_IOCON->PIO2_2 = 0xE8; //GPIO, pull-down, hysteresis enabled QA1 Digital in mode
	LPC_IOCON->PIO2_3 = 0xE8; //GPIO, pull-down, hysteresis enabled QB1	Digital in mode

	LPC_IOCON->PIO2_4 = 0xE8; //GPIO, pull-down, hysteresis enabled QA1 Digital in mode
	LPC_IOCON->PIO2_5 = 0xE8; //GPIO, pull-down, hysteresis enabled QB1	Digital in mode

	LPC_IOCON->PIO2_6 = 0xE8; //GPIO, pull-down, hysteresis enabled QA1 Digital in mode
	LPC_IOCON->PIO2_7 = 0xE8; //GPIO, pull-down, hysteresis enabled QB1	Digital in mode

	LPC_GPIO2->DIR &= ~0XFF; //clear bits for PIO2_0 - PIO2_7 (set to input)

	//now set up for interrupts on both edges	
	LPC_GPIO2->IS = 0; //All pins are edge sensitive
	LPC_GPIO2->IBE = 0X0FF; //All pins sense both edges
	LPC_GPIO2->IE = 0X0FF; //Bits 0 through 7 changes produce interrupt
	LPC_GPIO2->IC = 0X0FF; 

	//Record current states of all quadrature inputs
	QLast = (LPC_GPIO2->MASKED_ACCESS[0XFF]);

	NVIC_ClearPendingIRQ(EINT2_IRQn);

	NVIC_SetPriority(EINT2_IRQn, 0);

	NVIC_EnableIRQ(EINT2_IRQn);
}

U8 TT3Set;
void PIOINT2_IRQHandler(void)
{ //Quad2
	U8 TLast, TCur;
	U8 Cur, Last;
	ClrTT2();
	TLast = QLast;
	LPC_GPIO2->IC = 0X0FF; //Clears edge detect interrupts on Bits 0 through 7
	TLast = (U8)QLast;
	QLast = (U16)LPC_GPIO2->MASKED_ACCESS[0XFFF];
	TCur = (U8)QLast;
	
	if((TCur & 0X03) != (TLast & 0X03))
	{//process Q0
		#define IDX 0
		Cur = TCur & 0X03;
		Last = TLast & 0X03;
		if(Cur < 2)
		{
			if(Cur < 1)
			{ //Cur is 0
				if(Last == 2)
				{
					++QCount[IDX];
				}
				else if(Last == 1)
				{
					--QCount[IDX];
				}
				else
				{
					if(TT3Set)
					{
						TT3Set = 0;
						ClrTT3();
					}
					else
					{
						TT3Set = 1;
						SetTT3();
					}
					++QErrors[IDX];
				}
			}
			else
			{ //Cur is 1
				if(Last == 0)
				{
					++QCount[IDX];
				}
				else if(Last == 3)
				{
					--QCount[IDX];
				}
				else
				{
					if(TT3Set)
					{
						TT3Set = 0;
						ClrTT3();
					}
					else
					{
						TT3Set = 1;
						SetTT3();
					}
					++QErrors[IDX];
				}
			}
	 	}
		else
		{
			if(Cur < 3)
			{ //Cur is 2
				if(Last == 3)
				{
					++QCount[IDX];
				}
				else if(Last == 0)
				{
					--QCount[IDX];
				}
				else
				{
					if(TT3Set)
					{
						TT3Set = 0;
						ClrTT3();
					}
					else
					{
						TT3Set = 1;
						SetTT3();
					}
					++QErrors[IDX];
				}
			}
			else
			{ //Cur is 3
				if(Last == 1)
				{
					++QCount[IDX];
				}
				else if(Last == 2)
				{
					--QCount[IDX];
				}
				else
				{
					if(TT3Set)
					{
						TT3Set = 0;
						ClrTT3();
					}
					else
					{
						TT3Set = 1;
						SetTT3();
					}
					++QErrors[IDX];
				}
			}
		}
		#undef IDX
	}

	if((TCur & 0X0C) != (TLast & 0X0C))
	{//process Q1
		#define IDX 1
		Cur = (TCur >> 2) & 0X03;
		Last = (TLast >> 2) & 0X03;
		if(Cur < 2)
		{
			if(Cur < 1)
			{ //Cur is 0
				if(Last == 2)
				{
					++QCount[IDX];
				}
				else if(Last == 1)
				{
					--QCount[IDX];
				}
				else
				{
					++QErrors[IDX];
				}
			}
			else
			{ //Cur is 1
				if(Last == 0)
				{
					++QCount[IDX];
				}
				else if(Last == 3)
				{
					--QCount[IDX];
				}
				else
				{
					++QErrors[IDX];
				}
			}
	 	}
		else
		{
			if(Cur < 3)
			{ //Cur is 2
				if(Last == 3)
				{
					++QCount[IDX];
				}
				else if(Last == 0)
				{
					--QCount[IDX];
				}
				else
				{
					++QErrors[IDX];
				}
			}
			else
			{ //Cur is 3
				if(Last == 1)
				{
					++QCount[IDX];
				}
				else if(Last == 2)
				{
					--QCount[IDX];
				}
				else
				{
					++QErrors[IDX];
				}
			}
		}
		#undef IDX
	}
	if((TCur & 0X30) != (TLast & 0X30))
	{//process Q2
		#define IDX 2
		Cur = (TCur >> 4) & 0X03;
		Last = (TLast >> 4) & 0X03;
		if(Cur < 2)
		{
			if(Cur < 1)
			{ //Cur is 0
				if(Last == 2)
				{
					++QCount[IDX];
				}
				else if(Last == 1)
				{
					--QCount[IDX];
				}
				else
				{
					++QErrors[IDX];
				}
			}
			else
			{ //Cur is 1
				if(Last == 0)
				{
					++QCount[IDX];
				}
				else if(Last == 3)
				{
					--QCount[IDX];
				}
				else
				{
					++QErrors[IDX];
				}
			}
		}
		else
		{
			if(Cur < 3)
			{ //Cur is 2
				if(Last == 3)
				{
					++QCount[IDX];
				}
				else if(Last == 0)
				{
					--QCount[IDX];
				}
				else
				{
					++QErrors[IDX];
				}
			}
			else
			{ //Cur is 3
				if(Last == 1)
				{
					++QCount[IDX];
				}
				else if(Last == 2)
				{
					--QCount[IDX];
				}
				else
				{
					++QErrors[IDX];
				}
			}
		}
		#undef IDX
	}
	if((TCur & 0XC0) != (TLast & 0X0C0))
	{//process Q3
		#define IDX 3
		Cur = (TCur >> 6) & 0X03;
		Last = (TLast >> 6) & 0X03;
		if(Cur < 2)
		{
			if(Cur < 1)
			{ //Cur is 0
				if(Last == 2)
				{
					++QCount[IDX];
				}
				else if(Last == 1)
				{
					--QCount[IDX];
				}
				else
				{
					++QErrors[IDX];
				}
			}
			else
			{ //Cur is 1
				if(Last == 0)
				{
					++QCount[IDX];
				}
				else if(Last == 3)
				{
					--QCount[IDX];
				}
				else
				{
					++QErrors[IDX];
				}
			}
	 	}
		else
		{
			if(Cur < 3)
			{ //Cur is 2
				if(Last == 3)
				{
					++QCount[IDX];
				}
				else if(Last == 0)
				{
					--QCount[IDX];
				}
				else
				{
					++QErrors[IDX];
				}
			}
			else
			{ //Cur is 3
				if(Last == 1)
				{
					++QCount[IDX];
				}
				else if(Last == 2)
				{
					--QCount[IDX];
				}
				else
				{
					++QErrors[IDX];
				}
			}
		}
		#undef IDX
	}
	SetTT2();
}

/*****************************************************************************
*	         END FILE:   Quadrature.c
*****************************************************************************/


