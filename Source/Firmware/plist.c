//*****************************************************************************
//
//	SegList.c
//  Author   DF
//  COMPILER    -      Keil uVision Version 5.15
//  TARGET      -      Cortex M3
//
//           
//*****************************************************************************
#include "target.h"
#include "servo.h"
#include "seglist.h"
#include <stdio.h>
#include <math.h>
#include <msgbuff.h>
#define M_PI acos(-1.0)

extern U8 LineBuff[81];
extern U8 ServCmd[4];//various state transition commands
extern U8 SState[4];//Operational states of the servos
double PVel, PAcc;
static double GetTabValue(U16 Index);
static U8 AddSegments(void); //add dual segment to both segment lists
double AScale, BScale;
double PrevSegVelA;
double PrevSegVelB;
double PrevBPos;
double PrevAPos;
double AStartP, BStartP;
extern LLUnion ServCmdPos[4];

U8 PListActive;
U8 ServoA = 0;
U8 ServoB = 1;
U16 BOffset = NUM_LIST_POINTS;
U16 Index;
float CosTable[NUM_LIST_POINTS];
static void CosineTable(float *Table, float Radians, U16 MaxIndex)
{
	int i;
	double Increment;
	Increment = Radians/MaxIndex;
	for(i = 0; i <= MaxIndex; ++i)
	{
		Table[i] = (float)(cos(i*Increment));
	}
}


void InitPointList(void)
{
	CosineTable(CosTable, M_PI/2.0, NUM_LIST_POINTS - 1);
	PListActive = FALSE;
}
	
void PListServe(void)	
{
	//both seglists should be in sync and stay in sync
	//so we only check for space in one
	if(PListActive &&(SLFreeSpace(ServoA) >= 2))
	{
		AddSegments(); //add dual segment to both segment lists
	}
}

U8 StartPList(float Accel, float ScaleA, float ScaleB)	
{
	if(Accel < .00001)
	{
		PListActive = FALSE;
		ServCmd[ServoA] = SCMD_STOP;
		ServCmd[ServoB] = SCMD_STOP;
		ClearSegList(ServoA);
		ClearSegList(ServoB);
		return FALSE;
	}
	AStartP = ServCmdPos[ServoA].ll/POS_REG_SCALE; 
	BStartP = ServCmdPos[ServoB].ll/POS_REG_SCALE; 
	if((SState[ServoA] != SS_IDLE) || (SState[ServoB] != SS_IDLE))
	{
		return FALSE;
	}
	AScale = ScaleA;
	BScale = ScaleB;
	PrevSegVelA =  PrevSegVelB = 0.0;
    Index = 0;
	PrevAPos = GetTabValue(0) * AScale;
	PrevBPos = GetTabValue(BOffset) * BScale;
	PAcc = (Accel < MIN_PLIST_ACC) ? MIN_PLIST_ACC : Accel;
	PVel = MIN_PLIST_VEL;
	ClearSegList(ServoA);
	ClearSegList(ServoB); //just to be doubly sure
	while(SLFreeSpace(ServoA) >= 2)//both seglists should be in sync and stay in sync
	{
		if(!AddSegments()) //add dual segment to both segment lists
		{
			return FALSE;
		}
		//We can't risk interrupt coming in between start commands and getting the segments
		//out of sync.
		__disable_irq();
		ServCmd[ServoA] = SCMD_RUN;
		ServCmd[ServoB] = SCMD_RUN;
		__enable_irq();
	}
	PListActive = TRUE;
	return TRUE;
}

U16 Idx;
static double GetTabValue(U16 Index)
{
	Idx = Index % (NUM_LIST_POINTS-1);
	switch(Index/(NUM_LIST_POINTS-1))
	{
		case 0:
			return CosTable[Idx];

		case 1:
			Idx = NUM_LIST_POINTS - 1 - Idx;
			return -CosTable[Idx];
	//		break;

		case 2:
			return -CosTable[Idx];
//			break;

		default:
		case 3:
			Idx = NUM_LIST_POINTS - 1 - Idx;
			return CosTable[Idx];
//			break;
	}
}

static U8 AddSegments(void) //add dual segment to both segment lists
{
	U8 BIndex;
	double PTime, VEnd, SegVel;
	double CurPos, DelPos;
	int PTicks;
	PTime =  (-PVel + sqrt(PVel*PVel + 2*PAcc))/PAcc;
	PTicks = (int)(PTime * TICKS_PER_SECOND);
	PTicks =  (PTicks < MIN_PTICKS) ? MIN_PTICKS : PTicks > MAX_PTICKS ? MAX_PTICKS : PTicks;
	if(PTicks & 1)
	{
		++PTicks; //make sure it is even
	}
	PTime = (1.0*PTicks)/TICKS_PER_SECOND; //and get limited segment time
	PVel += PTime * PAcc;
	//so now we have done our acceleration and limiting,
	//and we have number of ticks for the segment(s) we are adding
	// to figure the segment end velocity, we need average vel of last segment
	 //and average vel of current segment.
	if(PVel > 0)
	{
	   Index = (Index < (NUM_PERIOD_SEGS - 1)) ? ++Index : 0;
	}
	else
	{
	   Index = Index > 0 ? --Index : (NUM_PERIOD_SEGS - 1);
	}
	CurPos = GetTabValue(Index) * AScale;
	DelPos = CurPos - PrevAPos;
	SegVel = DelPos/PTime;
    VEnd = ((3 * SegVel) - PrevSegVelA)/2;
    PrevSegVelA = SegVel;
	PrevAPos = CurPos;
//U8 SegCompute(U8 Channel, double DeltaPos, double VEnd, unsigned long DeltaTicks, long long EndPos, U8 Flags)
	if(!SegCompute(ServoA, DelPos, VEnd, PTicks, (long long)((CurPos + AStartP)*POS_REG_SCALE), SEG_FLAG_JAM))
	{
		return FALSE;
	}
	BIndex = Index + BOffset;
    BIndex = (BIndex >= NUM_PERIOD_SEGS) ?  BIndex - NUM_PERIOD_SEGS : BIndex;
	CurPos = GetTabValue(BIndex) * BScale;
//	sprintf((char*)LineBuff, "%d  %d  %d\r\n", Index, BIndex, Idx);
//	PutMessage(LineBuff);
	DelPos = CurPos - PrevBPos;
	SegVel = DelPos/PTime;
    VEnd = ((3 * SegVel) - PrevSegVelB)/2;
    PrevSegVelB = SegVel;
	PrevBPos = CurPos;
 	if(!SegCompute(ServoB, DelPos, VEnd, PTicks, (long long)((CurPos + BStartP)*POS_REG_SCALE), SEG_FLAG_JAM))
 	{
 		return FALSE;
 	}
	return TRUE;
}

//*****************************************************************************
// End file SegList.c
//*****************************************************************************/
