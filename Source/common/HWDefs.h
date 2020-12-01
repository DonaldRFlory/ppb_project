//--------------------------------------------------------------------------
//	HWDefs.h 
//	Definitions for Servo controller hardware I/O pins and such.
//
//--------------------------------------------------------------------------

//	CT32B1 controls	PWM0 and PWM1, MAT1 is PWO0, MAT3 is PWM1.
//	CT16B0 controls PWM2 and PWM3, MAT0 is PWM2, MAT1 is PWM3.
//Motor PWM pins:
//PWM0 is PIO0_1/CT32B0_MAT2
//PWM1 is PIO0_11/CT32B0_MAT3
//PWM2 is PIO0_8/CT16B0_MAT0
//PWM3 is PIO0_9/CT16B0_MAT1

//Dir0 = P0.2
#define DIR0_BIT 0X004
//Dir1 = P0.7
#define DIR1_BIT 0X080
//Dir2 = P1.5
#define DIR2_BIT 0X020
//Dir3 = P1.8
#define DIR3_BIT 0X100

	
#define StartIntervalTiming() (LPC_TMR32B0->TC = 0); (LPC_TMR32B0->TCR = 1)
#define EndIntervalTiming() (LPC_TMR32B0->TCR = 1)
#define ReadIntervalTime() (LPC_TMR32B0->TC)


#define SetServ0Dir() (LPC_GPIO0->MASKED_ACCESS[DIR0_BIT] = 0XFFF)
#define ClrServ0Dir() (LPC_GPIO0->MASKED_ACCESS[DIR0_BIT] = 0X000)
#define SetServ1Dir() (LPC_GPIO0->MASKED_ACCESS[DIR1_BIT] = 0XFFF)
#define ClrServ1Dir() (LPC_GPIO0->MASKED_ACCESS[DIR1_BIT] = 0X000)
#define SetServ2Dir() (LPC_GPIO1->MASKED_ACCESS[DIR2_BIT] = 0XFFF)
#define ClrServ2Dir() (LPC_GPIO1->MASKED_ACCESS[DIR2_BIT] = 0X000)
#define SetServ3Dir() (LPC_GPIO1->MASKED_ACCESS[DIR3_BIT] = 0XFFF)
#define ClrServ3Dir() (LPC_GPIO1->MASKED_ACCESS[DIR3_BIT] = 0X000)

//HappyLED is PIO3.2
#define SetHappyLED() (LPC_GPIO3->MASKED_ACCESS[0X004] = 0XFFF)
#define ClrHappyLED() (LPC_GPIO3->MASKED_ACCESS[0X004] = 0X000)

//TT1 is PIO3.0
//TT2 is PIO3.1
//TT3 is PIO0.3
#define SetTT1() (LPC_GPIO3->MASKED_ACCESS[0X001] = 0XFFF)
#define ClrTT1() (LPC_GPIO3->MASKED_ACCESS[0X001] = 0X000)
#define SetTT2() (LPC_GPIO3->MASKED_ACCESS[0X002] = 0XFFF)
#define ClrTT2() (LPC_GPIO3->MASKED_ACCESS[0X002] = 0X000)
#define SetTT3() (LPC_GPIO3->MASKED_ACCESS[0X008] = 0XFFF)
#define ClrTT3() (LPC_GPIO3->MASKED_ACCESS[0X008] = 0X000)

unsigned short ReadIOPort(unsigned char PortIndex);
void InitIntervalTimer(void);

//--------------------------------------------------------------------------
//	End of file HWDefs.h
//--------------------------------------------------------------------------
