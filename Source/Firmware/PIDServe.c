//*****************************************************************************
//
//	PIDServe.c
//  Author   DF
//  COMPILER    -      Keil uVision Version 5.15
//  TARGET      -      Cortex M3
//
//           
//*****************************************************************************
#include "target.h"
//#include <stdlib.h>
#include "core_cm3.h"
#include "pwm.h"
#include "HWDefs.h"
#include "servo.h"
#include "stdio.h"
#include "i2c_1343.h"

extern S32 QCount[4];
extern LLUnion ServCmdPos[4];

U8 PIDEnable[4];//enables PID control
S32 LastError[4];
S32 IAccum[4] = {0, 0,0,0};
U32 PTerm[4] = {50000, 50000,50000,50000};
U32 ITerm[4] = {200, 200,200,200};
U32 DTerm[4] = {40, 40, 40, 40};
S32 PIDDrive[4];

U8 TuningChannel = 5;
U8 TuningToggle;
U16 TuningOffset;
U16 TuningCounter;
U16 TuningDelay;

void SetTuning(U8 ChanIndex, U16 Offset, U16 Delay)
{
	TuningToggle = 0;
	TuningChannel = ChanIndex;
	TuningOffset = Offset;
	TuningCounter = 0;
	TuningDelay = Delay;
}

void ResetPIDs(void)
{
	int i;
	for(i = 0; i < 4; ++i)
	{
		SetPIDEnable(i, 0);
	}
}

void SetPIDEnable(U8 Channel, U8 OnFlag)
{
	if(Channel <= 3)
	{
		if(OnFlag)
		{
			PIDEnable[Channel] = 1;
		}
		else
		{
			PIDEnable[Channel] = 0;
			SetPWM(Channel, 0);
			PIDDrive[Channel] = 0;
			IAccum[Channel] = 0;
		}
	}
}

U32 Counter;
static S32 DelError, Error, CurDrive;
void PIDServe(void)
{
	#define SET_DIR() SetServ0Dir()
	#define CLR_DIR() ClrServ0Dir()
	#define SIDX  0
	if(PIDEnable[SIDX])
	{
		if(TuningChannel == SIDX)
		{
			if(++TuningCounter >= TuningDelay)
			{
				TuningCounter = 0;
				if(TuningToggle)
				{
					TuningToggle = 0;
					ServCmdPos[SIDX].l[1] -= TuningOffset;
				}
				else
				{
					TuningToggle = 1;
					ServCmdPos[SIDX].l[1] += TuningOffset;
				}
			}
			WriteDAC(0, (U16)QCount[SIDX]);
		}
		Error = ServCmdPos[SIDX].l[1] - QCount[SIDX];
		DelError = Error - LastError[SIDX];
		LastError[SIDX] = Error;
		if(Error > MAX_ERROR)
		{
			Error = MAX_ERROR;
		}
		else if(Error < -MAX_ERROR)
		{
			Error = -MAX_ERROR;
		}
	
		if(DelError > MAX_DEL_ERROR)
		{
			DelError = MAX_DEL_ERROR;
		}
		else if(DelError < -MAX_DEL_ERROR)
		{
			DelError = -MAX_DEL_ERROR;
		}
		IAccum[SIDX] += Error;
		if(IAccum[SIDX] > ITERM_MAX_WINDUP)
		{
			IAccum[SIDX] = ITERM_MAX_WINDUP;
		}
		else if(IAccum[SIDX] < -ITERM_MAX_WINDUP)
		{
			IAccum[SIDX] = -ITERM_MAX_WINDUP;
		}
		CurDrive = (PTerm[SIDX] * Error) + (ITerm[SIDX] * IAccum[SIDX]) + (DTerm[SIDX] * DelError);
		PIDDrive[SIDX] = CurDrive;
		if(CurDrive < 0)
		{
			CLR_DIR();
			CurDrive = -CurDrive;
		}
		else
		{
			SET_DIR();
		}

		SetPWM(SIDX, CurDrive >> 16);//SetPWM function takes care of limiting the drive
	}
	#undef SET_DIR
	#undef CLR_DIR
	#undef SIDX
	//End PID 0 ----------------------------------------------------------------------------------------

	#define SET_DIR() SetServ1Dir()
	#define CLR_DIR() ClrServ1Dir()
	#define SIDX  1
	if(PIDEnable[SIDX])
	{
		if(TuningChannel == SIDX)
		{
			if(++TuningCounter >= TuningDelay)
			{
				TuningCounter = 0;
				if(TuningToggle)
				{
					TuningToggle = 0;
					ServCmdPos[SIDX].l[1] -= TuningOffset;;
				}
				else
				{
					TuningToggle = 1;
					ServCmdPos[SIDX].l[1] += TuningOffset;;
				}
			}
			WriteDAC(0, (U16)QCount[SIDX]);
		}
		Error = ServCmdPos[SIDX].l[1] - QCount[SIDX];
		DelError = Error - LastError[SIDX];
		LastError[SIDX] = Error;
		if(Error > MAX_ERROR)
		{
			Error = MAX_ERROR;
		}
		else if(Error < -MAX_ERROR)
		{
			Error = -MAX_ERROR;
		}
	
		if(DelError > MAX_DEL_ERROR)
		{
			DelError = MAX_DEL_ERROR;
		}
		else if(DelError < -MAX_DEL_ERROR)
		{
			DelError = -MAX_DEL_ERROR;
		}
		IAccum[SIDX] += Error;
		if(IAccum[SIDX] > ITERM_MAX_WINDUP)
		{
			IAccum[SIDX] = ITERM_MAX_WINDUP;
		}
		else if(IAccum[SIDX] < -ITERM_MAX_WINDUP)
		{
			IAccum[SIDX] = -ITERM_MAX_WINDUP;
		}
		CurDrive = (PTerm[SIDX] * Error) + (ITerm[SIDX] * IAccum[SIDX]) + (DTerm[SIDX] * DelError);
		PIDDrive[SIDX] = CurDrive;
		if(CurDrive < 0)
		{
			CLR_DIR();
			CurDrive = -CurDrive;
		}
		else
		{
			SET_DIR();
		}

		SetPWM(SIDX, CurDrive >> 16);//SetPWM function takes care of limiting the drive
	}
	#undef SET_DIR
	#undef CLR_DIR
	#undef SIDX
	//End PID 1 ----------------------------------------------------------------------------------------

	#define SET_DIR() SetServ2Dir()
	#define CLR_DIR() ClrServ2Dir()
	#define SIDX  2
	if(PIDEnable[SIDX])
	{
		if(TuningChannel == SIDX)
		{
			if(++TuningCounter >= TuningDelay)
			{
				TuningCounter = 0;
				if(TuningToggle)
				{
					TuningToggle = 0;
					ServCmdPos[SIDX].l[1] -= TuningOffset;;
				}
				else
				{
					TuningToggle = 1;
					ServCmdPos[SIDX].l[1] += TuningOffset;;
				}
			}
			WriteDAC(0, (U16)QCount[SIDX]);
		}
		Error = ServCmdPos[SIDX].l[1] - QCount[SIDX];
		DelError = Error - LastError[SIDX];
		LastError[SIDX] = Error;
		if(Error > MAX_ERROR)
		{
			Error = MAX_ERROR;
		}
		else if(Error < -MAX_ERROR)
		{
			Error = -MAX_ERROR;
		}
	
		if(DelError > MAX_DEL_ERROR)
		{
			DelError = MAX_DEL_ERROR;
		}
		else if(DelError < -MAX_DEL_ERROR)
		{
			DelError = -MAX_DEL_ERROR;
		}
		IAccum[SIDX] += Error;
		if(IAccum[SIDX] > ITERM_MAX_WINDUP)
		{
			IAccum[SIDX] = ITERM_MAX_WINDUP;
		}
		else if(IAccum[SIDX] < -ITERM_MAX_WINDUP)
		{
			IAccum[SIDX] = -ITERM_MAX_WINDUP;
		}
		CurDrive = (PTerm[SIDX] * Error) + (ITerm[SIDX] * IAccum[SIDX]) + (DTerm[SIDX] * DelError);
		PIDDrive[SIDX] = CurDrive;
		if(CurDrive < 0)
		{
			CLR_DIR();
			CurDrive = -CurDrive;
		}
		else
		{
			SET_DIR();
		}

		SetPWM(SIDX, CurDrive >> 16);//SetPWM function takes care of limiting the drive
	}
	#undef SET_DIR
	#undef CLR_DIR
	#undef SIDX
	//End PID 1 ----------------------------------------------------------------------------------------
	#define SET_DIR() SetServ3Dir()
	#define CLR_DIR() ClrServ3Dir()
	#define SIDX  3
	if(PIDEnable[SIDX])
	{
		if(TuningChannel == SIDX)
		{
			if(++TuningCounter >= TuningDelay)
			{
				TuningCounter = 0;
				if(TuningToggle)
				{
					TuningToggle = 0;
					ServCmdPos[SIDX].l[1] -= TuningOffset;;
				}
				else
				{
					TuningToggle = 1;
					ServCmdPos[SIDX].l[1] += TuningOffset;;
				}
			}
			WriteDAC(0, (U16)QCount[SIDX]);
		}
		Error = ServCmdPos[SIDX].l[1] - QCount[SIDX];
		DelError = Error - LastError[SIDX];
		LastError[SIDX] = Error;
		if(Error > MAX_ERROR)
		{
			Error = MAX_ERROR;
		}
		else if(Error < -MAX_ERROR)
		{
			Error = -MAX_ERROR;
		}
	
		if(DelError > MAX_DEL_ERROR)
		{
			DelError = MAX_DEL_ERROR;
		}
		else if(DelError < -MAX_DEL_ERROR)
		{
			DelError = -MAX_DEL_ERROR;
		}
		IAccum[SIDX] += Error;
		if(IAccum[SIDX] > ITERM_MAX_WINDUP)
		{
			IAccum[SIDX] = ITERM_MAX_WINDUP;
		}
		else if(IAccum[SIDX] < -ITERM_MAX_WINDUP)
		{
			IAccum[SIDX] = -ITERM_MAX_WINDUP;
		}
		CurDrive = (PTerm[SIDX] * Error) + (ITerm[SIDX] * IAccum[SIDX]) + (DTerm[SIDX] * DelError);
		PIDDrive[SIDX] = CurDrive;
		if(CurDrive < 0)
		{
			CLR_DIR();
			CurDrive = -CurDrive;
		}
		else
		{
			SET_DIR();
		}

		SetPWM(SIDX, CurDrive >> 16);//SetPWM function takes care of limiting the drive
	}
	#undef SET_DIR
	#undef CLR_DIR
	#undef SIDX
}//end PIDServe()
