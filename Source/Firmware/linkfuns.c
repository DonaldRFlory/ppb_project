//Standard link functions Universal PowerPac main board (MK22 uP). 
//Supported by both main and boot programs
//Don Flory 2-7-2017
//
#include "type.h"
#include "slink.h"
#include "mainll.h"  //This is the slave side version with no pointer variables
#include "target.h"
#include "verblock.h"
#include <limits.h>
#include "servo.h"
#include "seglist.h"
#include "slavparm.h"
#define INIT_CRC  		0  /* CCITT: 0xFFFF */
#define CRCPOLY  		0xA001  /* ANSI CRC-16 */
                         /* CCITT: 0x8408 */

extern double VelMax[4];
extern double Accel[4];

const U32 ProgVersion = 1001;

//We need to guarantee that we only try to access implemented portions of memory.
//For our purposes that will be FLASH from 0 - 0X7FFFF and RAM from 1FFF0000 - 2000FFFF
//To make the tests more efficient, we reject transfers with any portion outside allowed area.
U8 MasterBlockDown(U16 Count, U32 DestAddress)
{
	U32 u;
	for(u = 0; u < Count; ++u)
	{
		if(!RcvU8((U8 *)DestAddress++))
		{	
			return FALSE;
		}
	}
	return TRUE;
}

U8 MasterBlockUp(U32 SrcAddress, U16 Count)
{
	U32 u;
	for(u = 0; u < Count; ++u)
	{
		if(!SendU8(((U8*)SrcAddress)[u]))
		{
			return FALSE;
		}
	}
	return TRUE;
}

//------------------------------------------------------------------------------
//  FUNCTION:     MakeCRCTable
//                
//  DESCRIPTION:  This Function is used to make CRC Table
//                
//  PARAMETERS: None  
//                
//  RETURNS:    None 
//
//  Notes:        
//------------------------------------------------------------------------------
static U16 CRCTable[UCHAR_MAX + 1];    //Holds Value of CRCTable
static void MakeCRCTable(void)
{
  	static short CRCTabInited = FALSE;
	unsigned short i, j, r;

  	if(CRCTabInited)
    	return;
  	CRCTabInited = TRUE;

  	for (i = 0; i <= UCHAR_MAX; i++)
  	{
		r = i;
		for (j = 0; j < CHAR_BIT; j++)
		{
      		if (r & 1)
      		{
        		r = (r >> 1) ^ CRCPOLY;
      		}
      		else
      		{
        		r >>= 1;
      		}
    	}
    	CRCTable[i] = r;
	}
}

extern S32 QCount[4];
extern U32 QErrors[4];
extern LLUnion CurAcc[4];
extern LLUnion CurVel[4];
extern LLUnion ServCmdPos[4];
extern S32 IAccum[4];
extern U32 PTerm[4];
extern U32 ITerm[4];
extern U32 DTerm[4];
extern S32 PIDDrive[4];
extern U8 PIDEnable[4];//enables PID control
extern U8 SState[4];//Operational state of the servo

U32  PunFloatToLong(float Float)
{
	U32 *LP;
	float *FP;
	FP =  &Float;
	LP = (U32*)FP;
	return *LP;
}

float PunLongToFloat(U32 Long)
{
	U32 *LP;
	float *FP;
	LP = &Long;
	FP = (float *)LP;
	return *FP;
}


void PListControl(float Accel, float ScaleA, float ScaleB)
{
	StartPList(Accel, ScaleA, ScaleB);
}

void SetVelAcc(U8 Channel, float Vel, float Acceleration)
{
	if(Channel > 3)
	{
		return;
	}
	VelMax[Channel] = Vel;
	Accel[Channel] = Acceleration;
}

//Host side: void LSetFloats(U8 ID, U8 SubID, DOWN_PTR_FLOAT Accelerations, U8 Count);
float TestArray[8];
void LSetFloats(U8 ID, U8 SubID, U8 Count) //slave side
{
//	int Result;
	int i;

	if(Count > 8)
		return;
	for(i = 0; i < Count; ++i)
	{
//		Result = RcvFloat(&TestArray[i]);
			RcvFloat(&TestArray[i]);
	}
}

void LGetFloats(U8 ID, U8 SubID, U8 Count) //slave side
{
	//int Result;
	int i;

	if(Count > 8)
		return;
	for(i = 0; i < Count; ++i)
	{
		//wResult = SendFloat(TestArray[i]);
		SendFloat(TestArray[i]);
	}
}

void GetVelAcc(U8 Channel, U8 Count)
{
	U32 Val;
	if(Channel > 3)
	{
		SendU32(0);
		SendU32(0);
		return;
	}
	Val = PunFloatToLong((float)VelMax[Channel]);
	SendU32(Val);
	Val = PunFloatToLong((float)Accel[Channel]);
	SendU32(Val);
}


void SetCommandPosition(U8 Channel, U32 Position)
{
	if(Channel <= 3)
	{
		ServCmdPos[Channel].l[0] = 0;
		ServCmdPos[Channel].l[1] = Position;
	}
}

U8 GetPIDData(U8 Channel, U16 UpCount)
{
	int i;
	if(Channel > 3)
	{
		for(i = 0; i < UpCount; ++i)
		{
			SendU32(0);
		}
		return 0;
	}
	i = 0;
	if(i++ >= UpCount) { return 1; }
	SendU32(PTerm[Channel]);
	if(i++ >= UpCount) { return 1; }
	SendU32(ITerm[Channel]);
	if(i >= UpCount) { return 1; }
	SendU32(DTerm[Channel]);
	return 1;
}

U8 SetPIDData(U8 Channel, U16 DownCount)
{
	int i;
	U32 Dummy;
	if(Channel > 3)
	{
		for(i = 0; i < DownCount; ++i)
		{
			RcvU32(&Dummy);
		}
		return 0;
	}
	i = 0;
	if(i++ >= DownCount) { return 1; }
	RcvU32(&PTerm[Channel]);
	if(i++ >= DownCount) { return 1; }
	RcvU32(&ITerm[Channel]);
	if(i >= DownCount) { return 1; }
	RcvU32(&DTerm[Channel]);
	return 1;
}


//This will return for the selected channel,
//QCount, QErrors, ServCmdPosHigh, ServCmdPosLow, CurAccHigh, CurrAccLow, CurVelHigh,
// CurVelLow, PIDEnDrive(En in high word), PIDIAccum
U8 GetServoData(U8 Channel, U16 UpCount)
{
	int i;
	if(Channel > 3)
	{
		for(i = 0; i < UpCount; ++i)
		{
			SendU32(0);
		}
		return 0;
	}
	i = 0;
	if(i++ >= UpCount) { return 0; }
	SendU32(QCount[Channel]);  //QCount
	if(i++ >= UpCount) { return 0; }
	SendU32(QErrors[Channel]);	//QErrors
	if(i++ >= UpCount) { return 0; }
	SendU32(ServCmdPos[Channel].l[1]); //CmdPosHigh
	if(i++ >= UpCount) { return 0; } 
	SendU32(ServCmdPos[Channel].l[0]); //CmdPosLow
	if(i++ >= UpCount) { return 0; }
	SendU32(CurAcc[Channel].l[1]);  //Acc High
	if(i++ >= UpCount) { return 0; }
	SendU32(CurAcc[Channel].l[0]);	//Acc low
	if(i++ >= UpCount) { return 0; }
	SendU32(CurVel[Channel].l[1]);	//Vel high
	if(i++ >= UpCount) { return 1; }
	SendU32(CurVel[Channel].l[0]); //Vel low
	if(i++ >= UpCount) { return 0; }
	SendU32(PIDDrive[Channel]);   //PIDEn_Drv
	if(i++ >= UpCount) { return 0; }
	SendU32(IAccum[Channel]);   //IAccum
	if(i++ >= UpCount) { return 0; }
	SendU32(SState[Channel]);   //SState (servo state)
	return 1;
}


void ResetServos()
{
	int i;
	for(i = 0; i < 4; ++i)
	{
		ResetServo(i);
	}
}

//Clears QCount, QErrors, ServoCmdPosition, CurAcc, and CurVel for specified channel
void ResetServo(U8 Channel)
{
	if(Channel <= 3)
	{
		QCount[Channel] = 0;
		QErrors[Channel] = 0;
		CurAcc[Channel].ll = 0;
		CurVel[Channel].ll = 0;
		ServCmdPos[Channel].ll = 0;
		SState[Channel] = SS_IDLE;
	}
}

//Block up function for processor GUID
U8 GetProcessorID(U8 IDLength)
{
	int i;
	//Dummy function returning array of zeros for now
	for(i = 0; i < IDLength; ++i)
	{
		SendU8(0);
	}
	return IDLength;
}


//Compute complete CRC on a buffer in one call. 
U16 ComputeCRC(U8 * Buff, U32 CharCount)
{
	U32 u;
	U16 CRC;
	MakeCRCTable();
	CRC = INIT_CRC;
	for(u=0; u < CharCount; ++u)
	{
		CRC = CRCTable[(CRC ^ (Buff[u] & 0xFF)) & 0xFF] ^ (CRC >> CHAR_BIT);
	}
	return CRC;
}


static U16 ComputeRangeCRC(U32 Start, U32 End)
{
	if(Start <= End)
	{
		return ComputeCRC((U8*)Start, Start - End + 1);
	}
	return ComputeCRC((U8*)Start, 0);
}


static U32 ChecksumStart, ChecksumEnd;
static unsigned long ChecksumCalc(int Index)
{
	unsigned long i, Sum = 0;
	switch(Index)
	{
		case 0:
			for(i = ChecksumStart; i <= ChecksumEnd; ++i)
			{
				Sum += *((unsigned char *)i);
			}
			return Sum;

		case 1:
			ChecksumStart &= 0xFFFFFFFC;//force to even word
			ChecksumEnd &= 0xFFFFFFFC;//force to even word
			for(i = ChecksumStart; i <= ChecksumEnd; i += 4)
			{
				Sum += *((unsigned long *)i);
			}
			return Sum;

		case 2:
		default:
			return ComputeRangeCRC(ChecksumStart, ChecksumEnd);
	}
}



//U32 EraseStartAddress, EraseEndAddress, EraseLength;
U32 GetSlaveParameter(U32 ParCode, U32 Index)
{

	switch (ParCode)
	{
		//Note SSP_ params start at 1000 and have specified behavior required for Universal Upgrade Utility
		//--------------------------------------------------------------------------------------
		case SSP_SSPVERSION:
			return CUR_SSP_VERSION;

		case SPAR_PROGVERSION:
		case SSP_PROGVERSION:
			if((Index == 0) || (Index == 1))
			{
				return ProgVersion;
			}
			return 0;

		case SSP_CS_START:
			return ChecksumStart;

		case SSP_CS_END:
			return ChecksumEnd;

		case SSP_CS_CALC:
			//Computes 32 bit check sum of bytes for Index==0 (modulo 2^^32)
			//or for Index==1 checksum of 32 bit words (modulo 2^^32)
			//or for Index==2 computes ANSI CRC-16.
			return ChecksumCalc(Index);

		case SSP_MAX_SEND_SIZE:
			return MaxLinkSend();

		case SSP_MAX_RETURN_SIZE:
			return MaxLinkReturn();

//		case SSP_RESTART_MAGIC:
//			return ((((unsigned long)RESTART_MAGIC_ONE) << 16)| RESTART_MAGIC_TWO); 

//		case SSP_VERBLOCK_ADDRESS:
//			return (U32)&VersionBlock;


//		case SSP_ERASE_START: //starting byte address of region to be erased by SSP_UPGRADE_ERASE
//			return EraseStartAddress;

//		case SSP_ERASE_END: //ending byte address of region to be erased by SSP_UPGRADE_ERASE
//			return EraseEndAddress;
		//End of Universal Upgrade Utility	standard parameters ----------------------------
		//--------------------------------------------------------------------------------------

		case SPAR_MSECS:
			return ReadMilliseconds();

		default:
			return 0;
	}				 
} // end function GetSlaveParameter


U8 SetSlaveParameter(U32 ParCode, U32 ParOne, U32 ParTwo)
{
	switch (ParCode)
	{
		//Note SSP_ params start at 1000 and have specified behavior required for Universal Upgrade Utility
		//--------------------------------------------------------------------------------------
		case SSP_CS_START:
			ChecksumStart = ParOne;
			break;

		case SSP_CS_END:
			ChecksumEnd = ParOne;
			break;

//		case SSP_RESET_CPU:
//			if((ParOne == RESTART_MAGIC_ONE) && (ParTwo == RESTART_MAGIC_TWO ))
//			{
//				CPUResetFlag = TRUE; //will cause CPU restart when link response is complete
//			}
//			break;

//		case SSP_ERASE_MAIN_PROG_KEY:
//			if((ParOne == RESTART_MAGIC_ONE) && (ParTwo == RESTART_MAGIC_TWO ))
//			{
//				EraseMainProgramKey(); // 
//			}
//			break;

//		case SSP_SET_MAIN_PROG_KEY:
//			if((ParOne == RESTART_MAGIC_ONE) && (ParTwo == RESTART_MAGIC_TWO ))
//			{
//				SetMainProgramKey(); 
//			}
//			break;

//		case SSP_ERASE_START: //starting byte address of region to be erased by SSP_UPGRADE_ERASE
//			EraseStartAddress = ParOne;
//			break;
//
//		case SSP_ERASE_END: //ending byte address of region to be erased by SSP_UPGRADE_ERASE
//			EraseEndAddress = ParOne;
//			break;
//
//		case SSP_UPGRADE_ERASE:
//			if((ParOne == RESTART_MAGIC_ONE) && (ParTwo == RESTART_MAGIC_TWO ))
//			{  //Erase main program memory region to prepare for programming
//				//May conceivably be called repeatedly for multiple disjoint regions by upgrader.
//				EraseLength = EraseEndAddress - EraseStartAddress + 1;
//				return EraseFlashSector(EraseStartAddress, EraseLength);
//			}
//			break;
		//Note end of Universal Upgrade Utility	standard parameters ----------------------------
		//--------------------------------------------------------------------------------------
		default:
			return 0;
	}	   														 
	return 1;
}

// EOF
