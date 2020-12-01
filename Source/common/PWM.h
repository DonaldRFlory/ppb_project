//--------------------------------------------------------------------------
//	PWM.H 
//	Definitions for PWM control functions.
//
//--------------------------------------------------------------------------

void SetPWM(U8 PWMIndex, U16 PWMVal);
void DirectionInit(void);
void PWMInit(void);
void SetServoDirection(U8 ServoIndex, U8 Direction);

//Fast macros for setting PWM match registers. Value should already be limited to 0-4000
#define SetPWM0(Value)(LPC_TMR32B0->MR2 = Value)
#define SetPWM1(Value)(LPC_TMR32B0->MR3 = Value)
#define SetPWM2(Value)(LPC_TMR16B0->MR0 = Value)
#define SetPWM3(Value)(LPC_TMR16B0->MR1 = Value)


#define SYS_CLK_FREQUENCY	48000000
#define PWM_FREQUENCY  24000
//Full on is 2000
#define PWM_FULL_ON  (SYS_CLK_FREQUENCY/PWM_FREQUENCY)
#define PWM_PERIOD_COUNT  (PWM_FULL_ON-1)

//--------------------------------------------------------------------------
//	End of file PWM.H
//--------------------------------------------------------------------------
