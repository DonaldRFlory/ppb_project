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

double ListVel, ListAcc;
double VelMax[4] = { MIN_VEL, MIN_VEL, MIN_VEL, MIN_VEL};
double Accel[4] = { MIN_ACCEL, MIN_ACCEL, MIN_ACCEL, MIN_ACCEL};

extern U8 LineBuff[81];

static unsigned char  MoveR_D(unsigned char Channel, double Steps);
//Compute a one or two part segment for a segment with specified steps over 
//specified time with specified end velocity. That means we enter the segment
//at some velocity from previous segment, and we are going to end the segment
//at a specified velocity, and we are going to cover the desired distance int
//the desired time. This may mean we have to speed up and slow down, to meet
//the constraints, or it may mean we are going to have to slow down and speed
//up. If the average of the start and end velocities of the segment over the
//specified ticks does not produce the specified movement (the area under velocity
//graph), we vary the area independently of the start and end velocities by
//making a hat or hole to produce the desired area.

//Compute a constant velocity segment (for trapezoidal moves)
static int SegCompute3(U8 Channel, double DeltaPos, double Vel, U8 Flags);

//Compute a constant acceleration segment (for trapezoidal moves)
static int SegCompute2(U8 Channel, double DeltaPos, double Acc, U8 Flags);

extern U8 SState[4];//Operational state of the servo
extern U8 ServCmd[4];//various state transition commands
extern const LLUnion ServCmdPos[4];

//We are designing for short segment FIFO, no more than 255 elements long
volatile U8 SegCount[4]; //number of segments in fifo segment buffer
U8 SegFault[4]; //may not need this at all
U32 SegListInIdx[4];//where next generated segment goes	owned by background.
U32 SegListOutIdx[4];//where we pull segments out for processing (owned by ISR)
SEG_DATA SegList[4][SEG_LIST_LEN];//four channels of trajectory segment control data,
							   //one list per channel. 
//Seglist general notes: 
//The segment list is a FIFO ring-buffer between background computation of
//motion segments and ISR driven consumption of motion segments for positioning.
//It is initialized and filled before movement is started, then may be 
//refilled as required by background for continuous motion or cyclical interpolated paths.

//VLast holds the final velocity of the previous segment added to seglist.
//ClearSegList initializes all to zero. 
double VLast[4];
double PLast[4];
																	 
//Clears seglist. Calculates the relative move required to get us to specified
//absolute position. Then calls MoveR(). Refers to current command positon of the
//specified Channel.
U8 ServoMove(U8 Channel, U32 Position)
{
	long Steps = ((S32)Position) - ServCmdPos[Channel].l[1];
	return MoveR(Channel, Steps);
}

	
//So... Rather than just implement trapezoidal moves we generalized the notion to a  list of
//move segments. Each segment lasts for a specific number of ticks.
// We came up with the idea of MoveType which
//encapsulates the acc and vel used. We have four velocities, and three accelerations permutated
//for a total of 12 move types. 

//The Move function (absolute move) simply computes the relative move required and calls MoveR.
//The MoveR function implements a simple trapezoidal move. We specify the acc and MaxVel based on
//MoveType, and the length of the move (relative to current postion). First we figure the direction,
//then we compute based on absolute distance of move. We compute the critical move steps which
//is distance we need to move to reach maximum velocity. A single step move is a kind of degenerate
//case. We compute it as if we are moving at the ISR rate since we need to generate a one step
//move in the minimum segment time of one tick. The general idea is to compute the move segments,
//in floating point in the bacground, then stuff the segments into a FIFO which executes the
//moves using fixed point calculations. We don't envision a bunch of one step moves, but rather
//a set of longer moves.
//

//This clears segment list and fills it with the several segments required for
//a trapezoidal move, relative to current position of Channel.
unsigned char  MoveR(unsigned char Channel, long Steps)
{
	return MoveR_D(Channel, (double)Steps);
}

double SCrit, VMax, Acc;
double Steps1, Steps2, Steps3;
static unsigned char  MoveR_D(unsigned char Channel, double Steps)
{
	int Direction;

	if(SState[Channel] != SS_IDLE)
	{	
		return FALSE;
	}
	ClearSegList(Channel);

	if(Steps >= 0)
	{
		Direction = 1;
	}
	else
	{
		Steps = -Steps;
		Direction = -1;
	}
	VMax = VelMax[Channel];
	Acc = Accel[Channel];
	if(VMax < MIN_VEL)
	{
		VMax = MIN_VEL;
	}
	if(Acc < MIN_ACCEL)
	{
		return FALSE;
	}
	if(Steps < MIN_MOVE)
	{
		return TRUE; //say we did approximately zero length move
	}

   	SCrit = (VMax*VMax)/(2*Acc);
	if( Steps/2 <= SCrit )
	{
		Steps1 = Steps3 = Steps/2;
	}
	else
	{
		Steps1 = Steps3 = SCrit;
	}
	Steps2 = Steps - Steps1 - Steps3;
	if(Steps2 < MIN_MOVE)
	{
		Steps1 = Steps3 = Steps/2;
		if(!SegCompute2(Channel, Direction * Steps1, Direction * Acc, 0))
		{
			return FALSE;
		}
		if(!SegCompute2(Channel, Direction * Steps3, -Direction * Acc, SEG_FLAG_JAM | SEG_FLAG_FINAL))
		{
			return FALSE;
		}
	}
	else
	{
		if(!SegCompute2(Channel, Direction * Steps1, Direction * Acc, SEG_FLAG_JAM | SEG_FLAG_VEL))
		{
			return FALSE;
		}
		if(!SegCompute3(Channel, Direction * Steps2, VLast[Channel], SEG_FLAG_CV))
		{
			return FALSE;
		}
		if(!SegCompute2(Channel, Direction * Steps3, -Direction * Acc, SEG_FLAG_JAM | SEG_FLAG_FINAL))
		{
			return FALSE;
		}
	}
	ServCmd[Channel] = SCMD_RUN;
	return TRUE;
}



U8 ClearSegList(U8 Channel)
{
	if(SState[Channel] != SS_IDLE)
	{
		return FALSE;
	}
		SegCount[Channel] = 0;
	SegListInIdx[Channel] = 0;
	SegListOutIdx[Channel] = 0;
	VLast[Channel] = 0.0;
	PLast[Channel] = ServCmdPos[Channel].ll / POS_REG_SCALE;
	return TRUE;
}

U32 SLFreeSpace(U8 Channel)
{
	return (SEG_LIST_LEN - SegCount[Channel]);
}

//So we want to be able to store pieces of trapezoidal move as simple constant ACC moves.
//This means end velocity is unconstrained. We just want a move from wherever the servo
//is at the beginning, from the starting velocity and with the specified acceleration.
//If move is flagged as SEG_FLAG_CV, we supply instead the velocity. At end of
//move we may force position and for CV moves position is the second LL Param.
//For non-CV moves, first LL param is acceleration, and second is final position or
//velocity based on flags. For profiled moves, the final segment will have SEG_FLAG_JAM
//and SEG_FLAG_POSITION. The LL param will be final position. The velociy can be left
//alone as velocity is meaningless after we go to IDLE at end of sequence. This leaves
//open using jam position during continuing moves. We always reset velocity before we
//start filling segment list.
//For first move of a ramp, the endpoint of most importance is velocity as that will influence
//the distance moved in the constant velocity segment.
//So when we add a segment, we are prescribing a move of specified (double) steps in specified time in integer ticks.
//For the most part, the profile generator is incremental and does not depend
//on absolute position. It is only for jamming position that it becomes absolute.
//That means that PLast here is only signifigant for that purpose.
//DeltaTicks must be non-zero.

//We are going to guarantee delta T is even.
//Compute a segment to move a specified distance in a specified number (integral) of ticks
//and ending at a specified velocity. Always produces two subsegments, even if parameters
//could accomplish requirements with one segment. DeltaTicks must be at least 2.
//First segment has LLParam1 = Acceleration, LLParam2 = dont care
//Second segment has7 LLParam1 = Acceleration, LLParam2 as final position or final velocity
//DeltaTicks must be even.
U8 SegCompute(U8 Channel, double DeltaPos, double VEnd, unsigned long DeltaTicks, long long EndPos, U8 Flags)
{
	double Sx, DeltaT, A0, Ax;
	
	if((SLFreeSpace(Channel) < 2) || (DeltaTicks < 2))
	{ //we dont even start unless there is room for the two segments we might produce
		return 0;
	}
 	DeltaT = ((double)DeltaTicks)/TICKS_PER_SECOND;
	A0 = (VEnd - VLast[Channel])/DeltaT; //average acceleration of dual segment
	if((A0 > MAX_ACCEL) || (A0 < -MAX_ACCEL))
	{
		return FALSE;
	}

	Sx = DeltaPos - (((VEnd + VLast[Channel])/2)*DeltaT);  //S-S1
	VLast[Channel] = VEnd;
    //always use two sub-segments
	Ax = (4*Sx)/(DeltaT * DeltaT);
	SegList[Channel][SegListInIdx[Channel]].LLParam1.ll = (long long) (ACC_REG_SCALE * (A0 + Ax));
	SegList[Channel][SegListInIdx[Channel]].EndTick = DeltaTicks/2;
	SegList[Channel][SegListInIdx[Channel]].Flags = 0;
	SegListInIdx[Channel]++;
	if(SegListInIdx[Channel] >= SEG_LIST_LEN)
	{
		SegListInIdx[Channel] = 0;//wrap to beginning
	}
	SegList[Channel][SegListInIdx[Channel]].LLParam1.ll = (long long) (ACC_REG_SCALE * (A0 - Ax));
	SegList[Channel][SegListInIdx[Channel]].EndTick = DeltaTicks/2;
	SegList[Channel][SegListInIdx[Channel]].LLParam2.ll = EndPos;
	SegList[Channel][SegListInIdx[Channel]].Flags = Flags;
	SegListInIdx[Channel]++;
	if(SegListInIdx[Channel] >= SEG_LIST_LEN)
	{
		SegListInIdx[Channel] = 0;//wrap to beginning
	}
	__disable_irq();
	SegCount[Channel] += 2;//and add two to segment count
	__enable_irq();

	return TRUE;
}			 


static double T, T2;
static unsigned long uT;
//Compute a constant acceleration segment (for trapezoidal moves)
// Since our movement routine works in discrete ticks, we need to modify acceleration based
//on actual integral tick time to get the expected change in velocity.
//Flag might be SEG_FLAG_END if this is final move of trapezoidal sequence
//With new seglist, we have LLParam1 as acceleration, LLParam2 as final position. 
//I think flags should control if we jam final position at end of segment
static int SegCompute2(U8 Channel, double DeltaPos, double Acc, U8 Flags)
{
	if((!SLFreeSpace(Channel)))
	{
		return FALSE;
	}

	if( (Acc < 0.001) && (Acc > -0.001))
	{
		return FALSE;
	}

	T = sqrt(fabs((2*DeltaPos)/Acc));
	uT = (unsigned long)( T * TICKS_PER_SECOND);
	if(uT == 0)
		uT = 1;

	T2 = ((double)uT)/TICKS_PER_SECOND;
	Acc *= (T/T2);
	VLast[Channel] = T2 * Acc;
	PLast[Channel] += DeltaPos;
	SegList[Channel][SegListInIdx[Channel]].LLParam1.ll = (long long) (ACC_REG_SCALE * Acc);
	SegList[Channel][SegListInIdx[Channel]].EndTick = uT;
	SegList[Channel][SegListInIdx[Channel]].Flags = Flags;
	if(Flags & SEG_FLAG_JAM)
	{
		if(Flags & SEG_FLAG_VEL)
		{
			SegList[Channel][SegListInIdx[Channel]].LLParam2.ll = (long long) (VEL_REG_SCALE * VLast[Channel]);
		}
		else
		{
			SegList[Channel][SegListInIdx[Channel]].LLParam2.ll = (long long) (POS_REG_SCALE * PLast[Channel]);
		}
	}
//	sprintf((char*)LineBuff, "SComp2:%d A = %f, DT = %ld\r\n", SegListInIdx[Channel], Acc, uT);
//	PutMessage(LineBuff);
	SegListInIdx[Channel]++;
	if(SegListInIdx[Channel] >= SEG_LIST_LEN)
	{
		SegListInIdx[Channel] = 0;//wrap to beginning
	}
	__disable_irq();
	++SegCount[Channel];
	__enable_irq();
	return TRUE;
}

//Compute a constant velocity segment (for trapezoidal moves)
//LLParam1 is velocity. LLParam2 is final position. 
//I think flags should control if we jam final position at end of segment
static int SegCompute3(U8 Channel, double DeltaPos, double Vel, U8 Flags)
{
	if((!SLFreeSpace(Channel)) || (fabs(Vel) < MIN_VEL))
	{
		return FALSE;
	}

	PLast[Channel] += DeltaPos;
	T =  DeltaPos/Vel;
	if(T < 0)
	{
		T = -T;
	}
	uT = (unsigned long)(T * TICKS_PER_SECOND);	
	if(uT == 0)
		uT = 1;
	T2 = ((double)uT)/TICKS_PER_SECOND;
	Vel *= (T/T2);
	SegList[Channel][SegListInIdx[Channel]].LLParam1.ll = (long long)(Vel * VEL_REG_SCALE);
	SegList[Channel][SegListInIdx[Channel]].EndTick = uT;
	SegList[Channel][SegListInIdx[Channel]].Flags = Flags | SEG_FLAG_CV;
//	sprintf((char*)LineBuff, "SComp3:%d DP = %d, Vel = %d Ticks = %lu\r\n", Channel, (int)DeltaPos, (int)Vel, uT);
//	PutMessage(LineBuff);
	SegListInIdx[Channel]++;
	if(SegListInIdx[Channel] >= SEG_LIST_LEN)
	{
		SegListInIdx[Channel] = 0;//wrap to beginning
	}
	__disable_irq();
	++SegCount[Channel];
	__enable_irq();
	return TRUE;
}

//*****************************************************************************
// End file SegList.c
//*****************************************************************************/
