//*****************************************************************************
//
//	ServoMain.c
//  Author   DF
//  COMPILER    -      Keil uVision Version 5.15
//  TARGET      -      Cortex M3
//
//           
//*****************************************************************************
#include "target.h"
#include <stdlib.h>
#include "core_cm3.h"
#include "servo.h"
#include "seglist.h"
#include "stdio.h"
#include "msgbuff.h"

extern U8 SegCount[4]; //number of segments in fifo segment buffer

static U8 Dir[4];//direction servo was moving when STOP  state was entered
static LLUnion LLParam2[4];  //LLParam2 of current segment
static long SegTick[4];  //counts ticks while executing a motion segment
static U8 Flags[4];
U8 ServCmd[4];//various state transition commands
long SegEndTick[4];

extern SEG_DATA SegList[4][SEG_LIST_LEN];//four channels of trajectory segment control data,
extern U32 SegListOutIdx[4];//where we pull segments out for processing owned by ISR
extern U32 SegListInIdx[4];

//applied when Stop command is recognized by ISR or when we hit end of seg list while still moving
long long StopAccel[4] =   
{
	(long long) (ACC_REG_SCALE * 200000.0),
	(long long) (ACC_REG_SCALE * 200000.0),
	(long long) (ACC_REG_SCALE * 200000.0),
	(long long) (ACC_REG_SCALE * 200000.0),
};

LLUnion ServCmdPos[4];
LLUnion CurAcc[4];
LLUnion CurVel[4];
U8 SState[4];//Operational states of the servos



void ServoStop(U8 Channel)
{
	if(Channel > 3)
		return;
	
	ServCmd[Channel] = SCMD_STOP;
}

void ServoCommand(unsigned char Channel, unsigned char Command)
{
	if(Channel > 3)
		return;
	
	switch(Command)
	{
		case SCMD_RUN:
			if(SState[Channel] != SS_IDLE)
			{
				return;
			}
			ServCmd[Channel] = Command;
			break;

		case SCMD_STOP:
			ServCmd[Channel] = Command;
			break;


		case SCMD_STOP_IMMEDIATE:
			SState[Channel] = SS_IDLE;
			CurAcc[Channel].ll = 0;
			CurVel[Channel].ll = 0;
			break;

		default:
			break;
	}
}

static	unsigned char Index;
#define SIDX  0
void SegmentProcess0(void)
{
	//First process any servo commands
	if(ServCmd[SIDX] == SCMD_STOP)
	{
		if(CurVel[SIDX].ll > 0)//See what current direction of motion is
		{
			Dir[SIDX] =  1;
			CurAcc[SIDX].ll = -StopAccel[SIDX];//accel in negative direction
			SState[SIDX] = SS_STOP;
		}
		else if(CurVel[SIDX].ll < 0)
		{
			Dir[SIDX] =  0;
			CurAcc[SIDX].ll = StopAccel[SIDX];
			SState[SIDX] = SS_STOP;
		}
		else
		{
			SState[SIDX] = SS_IDLE;
		}
		ServCmd[SIDX] = SCMD_NONE;
	}
	else if(ServCmd[SIDX] == SCMD_RUN)
	{
		SegTick[SIDX] = 0;
		SegListOutIdx[SIDX] = 0;//always start at beginning of list
		Flags[SIDX] = SegList[SIDX][0].Flags;
		if(Flags[SIDX] & SEG_FLAG_CV)
		{
			CurAcc[SIDX].ll = 0;//set starting accel for new segment
			CurVel[SIDX].ll = SegList[SIDX][0].LLParam1.ll;
		
		}
		else
		{
			CurAcc[SIDX].ll = SegList[SIDX][0].LLParam1.ll;//set starting accel for new segment
		}
		LLParam2[SIDX] = SegList[SIDX][0].LLParam2; //and LLParam2 to working set
		SegEndTick[SIDX] = SegList[SIDX][0].EndTick;//copy endtick to working set
		SState[SIDX] = SS_MOVE;
		ServCmd[SIDX] = SCMD_NONE;
		--SegCount[SIDX];
		SegListOutIdx[SIDX] = 1;//and increment the out index
	}
	if(SState[SIDX] == SS_MOVE)
	{//we are in a trajectory
		++SegTick[SIDX];
		if(SegTick[SIDX] >= SegEndTick[SIDX])
		{ //done with segment
			if(Flags[SIDX] & SEG_FLAG_JAM)
			{
				if(Flags[SIDX] & SEG_FLAG_VEL)
				{
					CurVel[SIDX].ll = LLParam2[SIDX].ll;
				}
				else
				{
					ServCmdPos[SIDX].ll = LLParam2[SIDX].ll;
				}
			}
			if(SegCount[SIDX] == 0)//no more segments available
			{
				if(Flags[SIDX] & SEG_FLAG_FINAL)
				{
					CurVel[SIDX].ll = 0;
					CurAcc[SIDX].ll = 0;
					SState[SIDX] = SS_IDLE;
				}
				else
				{ //If not intended end of motion, deal with it in controlled fashion
					if(CurVel[SIDX].ll > 0)//See what current direction of motion is
					{
						Dir[SIDX] =  1;
						CurAcc[SIDX].ll = -StopAccel[SIDX];//accel in negative direction
						SState[SIDX] = SS_STOP;
					}
					else if(CurVel[SIDX].ll < 0)
					{
						Dir[SIDX] =  0;
						CurAcc[SIDX].ll = StopAccel[SIDX];//accel in positive direction
						SState[SIDX] = SS_STOP;
					}
					else
					{
						CurVel[SIDX].ll = 0;
						SState[SIDX] = SS_IDLE;
						CurAcc[SIDX].ll = 0;
					}
				}
			}
			else
			{ //start up next segment
				Index = SegListOutIdx[SIDX];
				Flags[SIDX] = SegList[SIDX][Index].Flags;
				SegTick[SIDX] = 0;
				SegEndTick[SIDX] = SegList[SIDX][Index].EndTick;//copy endtick to working set
				LLParam2[SIDX] = SegList[SIDX][Index].LLParam2; //and LLParam2 to working set
				if(Flags[SIDX] & SEG_FLAG_CV)
				{
					CurAcc[SIDX].ll = 0;
					CurVel[SIDX].ll = SegList[SIDX][Index].LLParam1.ll;
				}
				else
				{
					CurAcc[SIDX].ll = SegList[SIDX][Index].LLParam1.ll;
				}
				++Index;
				SegListOutIdx[SIDX] = (Index >= SEG_LIST_LEN) ? 0 : Index ;
				--SegCount[SIDX];
			}
		}										
	}
}

void	AccVelProcess0(void) 
{
	if(SState[SIDX] == SS_MOVE)
	{//just apply acceleration to velocity, and velocity to position
		CurVel[SIDX].ll += CurAcc[SIDX].ll;
		ServCmdPos[SIDX].ll += CurVel[SIDX].ll >> VEL_SHIFT;
	}
	else if(SState[SIDX] == SS_STOP)
	{
		CurVel[SIDX].ll += CurAcc[SIDX].ll;
		ServCmdPos[SIDX].ll += CurVel[SIDX].ll >> VEL_SHIFT;
		if(Dir[SIDX])//direction was positive
		{
			if(CurVel[SIDX].ll <  0)
			{
				SState[SIDX] = SS_IDLE;
				CurAcc[SIDX].ll = 0;
				CurVel[SIDX].ll = 0;
			}
		}
		else
		{
			if(CurVel[SIDX].ll >  0)
			{
				SState[SIDX] = SS_IDLE;
				CurAcc[SIDX].ll = 0;
				CurVel[SIDX].ll = 0;
			}
		}
	}///end of SS_STOP processing
} // End AccVelProcess0() 
#undef SIDX

#define SIDX  1
void SegmentProcess1(void)
{
	//First process any servo commands
	if(ServCmd[SIDX] == SCMD_STOP)
	{
		if(CurVel[SIDX].ll > 0)//See what current direction of motion is
		{
			Dir[SIDX] =  1;
			CurAcc[SIDX].ll = -StopAccel[SIDX];//accel in negative direction
			SState[SIDX] = SS_STOP;
		}
		else if(CurVel[SIDX].ll < 0)
		{
			Dir[SIDX] =  0;
			CurAcc[SIDX].ll = StopAccel[SIDX];
			SState[SIDX] = SS_STOP;
		}
		else
		{
			SState[SIDX] = SS_IDLE;
		}
		ServCmd[SIDX] = SCMD_NONE;
	}
	else if(ServCmd[SIDX] == SCMD_RUN)
	{
		SegTick[SIDX] = 0;
		SegListOutIdx[SIDX] = 0;//always start at beginning of list
		Flags[SIDX] = SegList[SIDX][0].Flags;
		if(Flags[SIDX] & SEG_FLAG_CV)
		{
			CurAcc[SIDX].ll = 0;//set starting accel for new segment
			CurVel[SIDX].ll = SegList[SIDX][0].LLParam1.ll;
		
		}
		else
		{
			CurAcc[SIDX].ll = SegList[SIDX][0].LLParam1.ll;//set starting accel for new segment
		}
		LLParam2[SIDX] = SegList[SIDX][0].LLParam2; //and LLParam2 to working set
		SegEndTick[SIDX] = SegList[SIDX][0].EndTick;//copy endtick to working set
		SState[SIDX] = SS_MOVE;
		ServCmd[SIDX] = SCMD_NONE;
		--SegCount[SIDX];
		SegListOutIdx[SIDX] = 1;//and increment the out index
	}
	if(SState[SIDX] == SS_MOVE)
	{//we are in a trajectory
		++SegTick[SIDX];
		if(SegTick[SIDX] >= SegEndTick[SIDX])
		{ //done with segment
			if(Flags[SIDX] & SEG_FLAG_JAM)
			{
				if(Flags[SIDX] & SEG_FLAG_VEL)
				{
					CurVel[SIDX].ll = LLParam2[SIDX].ll;
				}
				else
				{
					ServCmdPos[SIDX].ll = LLParam2[SIDX].ll;
				}
			}
			if(SegCount[SIDX] == 0)//no more segments available
			{
				if(Flags[SIDX] & SEG_FLAG_FINAL)
				{
					CurVel[SIDX].ll = 0;
					CurAcc[SIDX].ll = 0;
					SState[SIDX] = SS_IDLE;
				}
				else
				{ //If not intended end of motion, deal with it in controlled fashion
					if(CurVel[SIDX].ll > 0)//See what current direction of motion is
					{
						Dir[SIDX] =  1;
						CurAcc[SIDX].ll = -StopAccel[SIDX];//accel in negative direction
						SState[SIDX] = SS_STOP;
					}
					else if(CurVel[SIDX].ll < 0)
					{
						Dir[SIDX] =  0;
						CurAcc[SIDX].ll = StopAccel[SIDX];//accel in positive direction
						SState[SIDX] = SS_STOP;
					}
					else
					{
						CurVel[SIDX].ll = 0;
						SState[SIDX] = SS_IDLE;
						CurAcc[SIDX].ll = 0;
					}
				}
			}
			else
			{ //start up next segment
				Index = SegListOutIdx[SIDX];
				Flags[SIDX] = SegList[SIDX][Index].Flags;
				SegTick[SIDX] = 0;
				SegEndTick[SIDX] = SegList[SIDX][Index].EndTick;//copy endtick to working set
				LLParam2[SIDX] = SegList[SIDX][Index].LLParam2; //and LLParam2 to working set
				if(Flags[SIDX] & SEG_FLAG_CV)
				{
					CurAcc[SIDX].ll = 0;
					CurVel[SIDX].ll = SegList[SIDX][Index].LLParam1.ll;
				}
				else
				{
					CurAcc[SIDX].ll = SegList[SIDX][Index].LLParam1.ll;
				}
				++Index;
				SegListOutIdx[SIDX] = (Index >= SEG_LIST_LEN) ? 0 : Index ;
				--SegCount[SIDX];
			}
		}										
	}
}

void	AccVelProcess1(void) 
{
	if(SState[SIDX] == SS_MOVE)
	{//just apply acceleration to velocity, and velocity to position
		CurVel[SIDX].ll += CurAcc[SIDX].ll;
		ServCmdPos[SIDX].ll += CurVel[SIDX].ll >> VEL_SHIFT;
	}
	else if(SState[SIDX] == SS_STOP)
	{
		CurVel[SIDX].ll += CurAcc[SIDX].ll;
		ServCmdPos[SIDX].ll += CurVel[SIDX].ll >> VEL_SHIFT;
		if(Dir[SIDX])//direction was positive
		{
			if(CurVel[SIDX].ll <  0)
			{
				SState[SIDX] = SS_IDLE;
				CurAcc[SIDX].ll = 0;
				CurVel[SIDX].ll = 0;
			}
		}
		else
		{
			if(CurVel[SIDX].ll >  0)
			{
				SState[SIDX] = SS_IDLE;
				CurAcc[SIDX].ll = 0;
				CurVel[SIDX].ll = 0;
			}
		}
	}///end of SS_STOP processing
} // End AccVelProcess0() 
#undef SIDX
#define SIDX  2
void SegmentProcess2(void)
{
	//First process any servo commands
	if(ServCmd[SIDX] == SCMD_STOP)
	{
		if(CurVel[SIDX].ll > 0)//See what current direction of motion is
		{
			Dir[SIDX] =  1;
			CurAcc[SIDX].ll = -StopAccel[SIDX];//accel in negative direction
			SState[SIDX] = SS_STOP;
		}
		else if(CurVel[SIDX].ll < 0)
		{
			Dir[SIDX] =  0;
			CurAcc[SIDX].ll = StopAccel[SIDX];
			SState[SIDX] = SS_STOP;
		}
		else
		{
			SState[SIDX] = SS_IDLE;
		}
		ServCmd[SIDX] = SCMD_NONE;
	}
	else if(ServCmd[SIDX] == SCMD_RUN)
	{
		SegTick[SIDX] = 0;
		SegListOutIdx[SIDX] = 0;//always start at beginning of list
		Flags[SIDX] = SegList[SIDX][0].Flags;
		if(Flags[SIDX] & SEG_FLAG_CV)
		{
			CurAcc[SIDX].ll = 0;//set starting accel for new segment
			CurVel[SIDX].ll = SegList[SIDX][0].LLParam1.ll;
		
		}
		else
		{
			CurAcc[SIDX].ll = SegList[SIDX][0].LLParam1.ll;//set starting accel for new segment
		}
		LLParam2[SIDX] = SegList[SIDX][0].LLParam2; //and LLParam2 to working set
		SegEndTick[SIDX] = SegList[SIDX][0].EndTick;//copy endtick to working set
		SState[SIDX] = SS_MOVE;
		ServCmd[SIDX] = SCMD_NONE;
		--SegCount[SIDX];
		SegListOutIdx[SIDX] = 1;//and increment the out index
	}
	if(SState[SIDX] == SS_MOVE)
	{//we are in a trajectory
		++SegTick[SIDX];
		if(SegTick[SIDX] >= SegEndTick[SIDX])
		{ //done with segment
			if(Flags[SIDX] & SEG_FLAG_JAM)
			{
				if(Flags[SIDX] & SEG_FLAG_VEL)
				{
					CurVel[SIDX].ll = LLParam2[SIDX].ll;
				}
				else
				{
					ServCmdPos[SIDX].ll = LLParam2[SIDX].ll;
				}
			}
			if(SegCount[SIDX] == 0)//no more segments available
			{
				if(Flags[SIDX] & SEG_FLAG_FINAL)
				{
					CurVel[SIDX].ll = 0;
					CurAcc[SIDX].ll = 0;
					SState[SIDX] = SS_IDLE;
				}
				else
				{ //If not intended end of motion, deal with it in controlled fashion
					if(CurVel[SIDX].ll > 0)//See what current direction of motion is
					{
						Dir[SIDX] =  1;
						CurAcc[SIDX].ll = -StopAccel[SIDX];//accel in negative direction
						SState[SIDX] = SS_STOP;
					}
					else if(CurVel[SIDX].ll < 0)
					{
						Dir[SIDX] =  0;
						CurAcc[SIDX].ll = StopAccel[SIDX];//accel in positive direction
						SState[SIDX] = SS_STOP;
					}
					else
					{
						CurVel[SIDX].ll = 0;
						SState[SIDX] = SS_IDLE;
						CurAcc[SIDX].ll = 0;
					}
				}
			}
			else
			{ //start up next segment
				Index = SegListOutIdx[SIDX];
				Flags[SIDX] = SegList[SIDX][Index].Flags;
				SegTick[SIDX] = 0;
				SegEndTick[SIDX] = SegList[SIDX][Index].EndTick;//copy endtick to working set
				LLParam2[SIDX] = SegList[SIDX][Index].LLParam2; //and LLParam2 to working set
				if(Flags[SIDX] & SEG_FLAG_CV)
				{
					CurAcc[SIDX].ll = 0;
					CurVel[SIDX].ll = SegList[SIDX][Index].LLParam1.ll;
				}
				else
				{
					CurAcc[SIDX].ll = SegList[SIDX][Index].LLParam1.ll;
				}
				++Index;
				SegListOutIdx[SIDX] = (Index >= SEG_LIST_LEN) ? 0 : Index ;
				--SegCount[SIDX];
			}
		}										
	}
}

void	AccVelProcess2(void) 
{
	if(SState[SIDX] == SS_MOVE)
	{//just apply acceleration to velocity, and velocity to position
		CurVel[SIDX].ll += CurAcc[SIDX].ll;
		ServCmdPos[SIDX].ll += CurVel[SIDX].ll >> VEL_SHIFT;
	}
	else if(SState[SIDX] == SS_STOP)
	{
		CurVel[SIDX].ll += CurAcc[SIDX].ll;
		ServCmdPos[SIDX].ll += CurVel[SIDX].ll >> VEL_SHIFT;
		if(Dir[SIDX])//direction was positive
		{
			if(CurVel[SIDX].ll <  0)
			{
				SState[SIDX] = SS_IDLE;
				CurAcc[SIDX].ll = 0;
				CurVel[SIDX].ll = 0;
			}
		}
		else
		{
			if(CurVel[SIDX].ll >  0)
			{
				SState[SIDX] = SS_IDLE;
				CurAcc[SIDX].ll = 0;
				CurVel[SIDX].ll = 0;
			}
		}
	}///end of SS_STOP processing
} // End AccVelProcess0() 
#undef SIDX
#define SIDX  3
void SegmentProcess3(void)
{
	//First process any servo commands
	if(ServCmd[SIDX] == SCMD_STOP)
	{
		if(CurVel[SIDX].ll > 0)//See what current direction of motion is
		{
			Dir[SIDX] =  1;
			CurAcc[SIDX].ll = -StopAccel[SIDX];//accel in negative direction
			SState[SIDX] = SS_STOP;
		}
		else if(CurVel[SIDX].ll < 0)
		{
			Dir[SIDX] =  0;
			CurAcc[SIDX].ll = StopAccel[SIDX];
			SState[SIDX] = SS_STOP;
		}
		else
		{
			SState[SIDX] = SS_IDLE;
		}
		ServCmd[SIDX] = SCMD_NONE;
	}
	else if(ServCmd[SIDX] == SCMD_RUN)
	{
		SegTick[SIDX] = 0;
		SegListOutIdx[SIDX] = 0;//always start at beginning of list
		Flags[SIDX] = SegList[SIDX][0].Flags;
		if(Flags[SIDX] & SEG_FLAG_CV)
		{
			CurAcc[SIDX].ll = 0;//set starting accel for new segment
			CurVel[SIDX].ll = SegList[SIDX][0].LLParam1.ll;
		
		}
		else
		{
			CurAcc[SIDX].ll = SegList[SIDX][0].LLParam1.ll;//set starting accel for new segment
		}
		LLParam2[SIDX] = SegList[SIDX][0].LLParam2; //and LLParam2 to working set
		SegEndTick[SIDX] = SegList[SIDX][0].EndTick;//copy endtick to working set
		SState[SIDX] = SS_MOVE;
		ServCmd[SIDX] = SCMD_NONE;
		--SegCount[SIDX];
		SegListOutIdx[SIDX] = 1;//and increment the out index
	}
	if(SState[SIDX] == SS_MOVE)
	{//we are in a trajectory
		++SegTick[SIDX];
		if(SegTick[SIDX] >= SegEndTick[SIDX])
		{ //done with segment
			if(Flags[SIDX] & SEG_FLAG_JAM)
			{
				if(Flags[SIDX] & SEG_FLAG_VEL)
				{
					CurVel[SIDX].ll = LLParam2[SIDX].ll;
				}
				else
				{
					ServCmdPos[SIDX].ll = LLParam2[SIDX].ll;
				}
			}
			if(SegCount[SIDX] == 0)//no more segments available
			{
				if(Flags[SIDX] & SEG_FLAG_FINAL)
				{
					CurVel[SIDX].ll = 0;
					CurAcc[SIDX].ll = 0;
					SState[SIDX] = SS_IDLE;
				}
				else
				{ //If not intended end of motion, deal with it in controlled fashion
					if(CurVel[SIDX].ll > 0)//See what current direction of motion is
					{
						Dir[SIDX] =  1;
						CurAcc[SIDX].ll = -StopAccel[SIDX];//accel in negative direction
						SState[SIDX] = SS_STOP;
					}
					else if(CurVel[SIDX].ll < 0)
					{
						Dir[SIDX] =  0;
						CurAcc[SIDX].ll = StopAccel[SIDX];//accel in positive direction
						SState[SIDX] = SS_STOP;
					}
					else
					{
						CurVel[SIDX].ll = 0;
						SState[SIDX] = SS_IDLE;
						CurAcc[SIDX].ll = 0;
					}
				}
			}
			else
			{ //start up next segment
				Index = SegListOutIdx[SIDX];
				Flags[SIDX] = SegList[SIDX][Index].Flags;
				SegTick[SIDX] = 0;
				SegEndTick[SIDX] = SegList[SIDX][Index].EndTick;//copy endtick to working set
				LLParam2[SIDX] = SegList[SIDX][Index].LLParam2; //and LLParam2 to working set
				if(Flags[SIDX] & SEG_FLAG_CV)
				{
					CurAcc[SIDX].ll = 0;
					CurVel[SIDX].ll = SegList[SIDX][Index].LLParam1.ll;
				}
				else
				{
					CurAcc[SIDX].ll = SegList[SIDX][Index].LLParam1.ll;
				}
				++Index;
				SegListOutIdx[SIDX] = (Index >= SEG_LIST_LEN) ? 0 : Index ;
				--SegCount[SIDX];
			}
		}										
	}
}
void AccVelProcess3(void) 
{
	if(SState[SIDX] == SS_MOVE)
	{//just apply acceleration to velocity, and velocity to position
		CurVel[SIDX].ll += CurAcc[SIDX].ll;
		ServCmdPos[SIDX].ll += CurVel[SIDX].ll >> VEL_SHIFT;
	}
	else if(SState[SIDX] == SS_STOP)
	{
		CurVel[SIDX].ll += CurAcc[SIDX].ll;
		ServCmdPos[SIDX].ll += CurVel[SIDX].ll >> VEL_SHIFT;
		if(Dir[SIDX])//direction was positive
		{
			if(CurVel[SIDX].ll <  0)
			{
				SState[SIDX] = SS_IDLE;
				CurAcc[SIDX].ll = 0;
				CurVel[SIDX].ll = 0;
			}
		}
		else
		{
			if(CurVel[SIDX].ll >  0)
			{
				SState[SIDX] = SS_IDLE;
				CurAcc[SIDX].ll = 0;
				CurVel[SIDX].ll = 0;
			}
		}
	}///end of SS_STOP processing
} // End AccVelProcess0() 
#undef SIDX

//*****************************************************************************
// End file Segmentprocess.c
//*****************************************************************************/
