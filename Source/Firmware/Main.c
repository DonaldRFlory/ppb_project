/****************************************************************************
 *   $Id:: demo.c 7214 2011-04-27 00:50:19Z usb00423                        $
 *   Project: NXP LPC13xx USB HID ROM version example
 *
 *   Description:
 *     This file contains USB HID test modules, main entry, to test ROM based
 *     USB HID APIs.
 *
 ****************************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
****************************************************************************/
#include "lpc13xx.h"                        /* LPC13xx definitions */
#include "type.h"
#include "target.h"
#include "slink.h"
#include "rom_drivers.h"
#include "servo.h"
#include "hidadapt.h"
#include "seradapt.h"
#include "hwdefs.h"
#include "pwm.h"
#include "quadrature.h"
#include "math.h"
#include "msgbuff.h"
#include <stdio.h>
#include "msgbuff.h"
#include "seglist.h"
#include "i2c_1343.h"

#define TABLE_LENGTH 101
S32 CosineStepTable[TABLE_LENGTH];
void CosineTable(S32 *Table, double Radians, U32 RadiusSteps, U16 MaxIndex);

void DumpTable(void);
static void InitIntervalTimer(void);
static void InitIO(void);
static void LEDServe(void);
static unsigned char MsecTick;
void DumpTable(void);
unsigned char MsecDivide;
/* Main Program */
int main (void) 
{
#if (USING_HID_LINK)
	HIDInit();
#endif
	SystemCoreClockUpdate();
 	InitIO();                         		// LED Initialization (Happy Lights)                            	
	ResetPIDs();
	ResetServos();
	InitIntervalTimer(); 					//set up general purpose interval timer
	UARTInit();
	PWMInit();
	I2CInit();
	TelltaleInit();
	QuadratureInit();
	DirectionInit();
	InitSlaveLink();
	InitPointList();

	SetTT1();	//to keep them high when not in use
	SetTT2();	//They connect to LED's which are on 
	SetTT3();	//when output is low.
	SysTick_Config(SystemCoreClock/4000);  	// Generate interrupt each 250 USec  
	while (1)        // Loop forever
	{
		if(MsecTick)
		{
			MsecTick = 0;
			LEDServe();
			//DumpTable();
		}
		LinkServe();
		PListServe();
	}
}


unsigned char LEDIsOn = 0;
//----------------------------------------------------------------------------
//  Function that initializes LEDs
//  PIO1_8 & PIO1_9
//----------------------------------------------------------------------------
static void InitIO(void) 
{
  LPC_GPIO3->DIR |= 0x0000000F;       // Happy light - P3.2, Telltales P3.0, P3.1, P3.3     

  ClrHappyLED();
	
}


static void LEDServe(void)
{
	static int MsecCount;
	++MsecCount;
//	if(MsecCount > 500)
	if(MsecCount > 200)
	{
		MsecCount = 0;
		if(LEDIsOn)
		{
//			LPC_UART->U0THR = 0x55;//DFDEBUG
			LEDIsOn = 0;
			ClrHappyLED();
		}
		else
		{
//			LPC_UART->U0THR = 0x55;//DFDEBUG
			LEDIsOn = 1;
			SetHappyLED();
		}
	}
}

U8 HIDLinkTimer;
U8 SerLinkTimer;

static U32 Milliseconds;
void SysTick_Handler (void)           // SysTick Interrupt Handler (1ms)
{
	ClrTT1();
	//	StartIntervalTiming();
	++MsecDivide;
	if(MsecDivide > 3)
	{
		MsecDivide = 0;
		++Milliseconds;
		MsecTick = 1;
#if (USING_SER_LINK)
		if(SerLinkTimer)
		{
//DFDEBUG
			--SerLinkTimer;
		}
#endif
#if (USING_HID_LINK)
		if(HIDLinkTimer)
		{
//DFDEBUG
			--HIDLinkTimer;
		}
#endif
	}
	SegmentProcess0();
	AccVelProcess0(); 
	SegmentProcess1();
	AccVelProcess1(); 
	SegmentProcess2();
	AccVelProcess2(); 
	SegmentProcess3();
	AccVelProcess3(); 
	PIDServe();
	SetTT1();
}

U32 ReadMilliseconds(void)
{
	return Milliseconds;
}

extern U8 LineBuff[81];
int TableIndex = 0;
void DumpTable(void)
{
	if((TableIndex < TABLE_LENGTH) && (GetMessageBuffCount() <= MESSAGE_BUFF_LEN - 81))
	{
		if(TableIndex == 0)
		{
			sprintf((char*) LineBuff, "Cosine step table:\r\n");
			PutMessage(LineBuff);
		}
		sprintf((char*) LineBuff, "Index %d: %d\r\n", TableIndex, CosineStepTable[TableIndex]);
		PutMessage(LineBuff);
		++TableIndex;
   	}
}
//Setup CT32B0 to count SysClk as up counter.
static void InitIntervalTimer(void)
{
	// Enable the clock respectively for LPC_TMR32B0 and IO config block
	LPC_SYSCON->SYSAHBCLKCTRL |= ((1<<9) | (1<<16));
   	//Prescale by unity
  	LPC_TMR32B0->PR = 0;
	LPC_TMR32B0->MCR = 0; //Do nothing on matches
	LPC_TMR32B0->TCR = 0; //Not active
	LPC_TMR32B0->TC = 0; //Clear counter
}

//Unexpected condition in link logic or elsewher. Probably
//indicates a logical error in firmware design or implementation
void PostLogicError(U16 ErrorIdentifier)
{
}
	
