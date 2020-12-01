// ..\..\output\API\ExpLink.cpp**************************************
// This file is machine generated from the link definition file
// ..\Common\mainll.h by LDFUTIL V2_015
// Processed in Mode: 2 - DLL export stubs Mode
// It should not be directly edited.
#include "ELHdr.h"
#include "mlink.h"

extern "C" {
PPB_API API_STAT CALL_CONV MasterBlockDown(API_DEVICE_HANDLE Handle, U8 &RtnVal, DOWN_PTR_U8 SrcPtr, U8 Count, U16 DestAddress)
{
	LINK_STAT LStat;
	LINK_SEL LSel;

	LSel.ChannelIndex = 0;
	LSel.CommType = INVALID_COMM_TYPE;
	LSel.LHand = Handle;
	RtnVal = MasterBlockDown(LSel, &LStat, SrcPtr, Count, DestAddress);
	return LinkStatToAPIStat(LStat);
}

PPB_API API_STAT CALL_CONV MasterBlockUp(API_DEVICE_HANDLE Handle, U8 &RtnVal, U16 SrcAddress, UP_PTR_U8 DestPtr, U8 Count)
{
	LINK_STAT LStat;
	LINK_SEL LSel;

	LSel.ChannelIndex = 0;
	LSel.CommType = INVALID_COMM_TYPE;
	LSel.LHand = Handle;
	RtnVal = MasterBlockUp(LSel, &LStat, SrcAddress, DestPtr, Count);
	return LinkStatToAPIStat(LStat);
}

PPB_API API_STAT CALL_CONV GetSlaveParameter(API_DEVICE_HANDLE Handle, U32 &RtnVal, U8 ParCode, U8 Index)
{
	LINK_STAT LStat;
	LINK_SEL LSel;

	LSel.ChannelIndex = 0;
	LSel.CommType = INVALID_COMM_TYPE;
	LSel.LHand = Handle;
	RtnVal = GetSlaveParameter(LSel, &LStat, ParCode, Index);
	return LinkStatToAPIStat(LStat);
}

PPB_API API_STAT CALL_CONV SetSlaveParameter(API_DEVICE_HANDLE Handle, U32 &RtnVal, U8 ParCode, U16 Param1, U16 Param2)
{
	LINK_STAT LStat;
	LINK_SEL LSel;

	LSel.ChannelIndex = 0;
	LSel.CommType = INVALID_COMM_TYPE;
	LSel.LHand = Handle;
	RtnVal = SetSlaveParameter(LSel, &LStat, ParCode, Param1, Param2);
	return LinkStatToAPIStat(LStat);
}

PPB_API API_STAT CALL_CONV SetServoUsec(API_DEVICE_HANDLE Handle, U8 &RtnVal, U8 ServoIndex, U16 USec)
{
	LINK_STAT LStat;
	LINK_SEL LSel;

	LSel.ChannelIndex = 0;
	LSel.CommType = INVALID_COMM_TYPE;
	LSel.LHand = Handle;
	RtnVal = SetServoUsec(LSel, &LStat, ServoIndex, USec);
	return LinkStatToAPIStat(LStat);
}

PPB_API API_STAT CALL_CONV GetServoUsec(API_DEVICE_HANDLE Handle, U16 &RtnVal, U8 ServoIndex)
{
	LINK_STAT LStat;
	LINK_SEL LSel;

	LSel.ChannelIndex = 0;
	LSel.CommType = INVALID_COMM_TYPE;
	LSel.LHand = Handle;
	RtnVal = GetServoUsec(LSel, &LStat, ServoIndex);
	return LinkStatToAPIStat(LStat);
}

PPB_API API_STAT CALL_CONV DataUpdate(API_DEVICE_HANDLE Handle, U8 &RtnVal, DOWN_PTR_U16 USecsPtr, U8 USecCount, UP_PTR_U16 ADValsPtr, U8 ADValCount)
{
	LINK_STAT LStat;
	LINK_SEL LSel;

	LSel.ChannelIndex = 0;
	LSel.CommType = INVALID_COMM_TYPE;
	LSel.LHand = Handle;
	RtnVal = DataUpdate(LSel, &LStat, USecsPtr, USecCount, ADValsPtr, ADValCount);
	return LinkStatToAPIStat(LStat);
}

} // End of Extern "C"
