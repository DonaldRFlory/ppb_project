// ..\..\output\API\dlllink.cpp**************************************
// This file is machine generated from the link definition file
// ..\Common\mainll.h by LDFUTIL V2_015
// Processed in Mode: 0 - Standard stubs Mode
// It should not be directly edited.
#include "mlink.h"


static MLFUN_DEF FDef[] =
{
		{1, {33, 2, 0, 0} },
		{1, {2, 161, 0, 0} },
		{4, {1, 1, 0, 0} },
		{4, {1, 2, 2, 0} },
		{1, {1, 2, 0, 0} },
		{2, {1, 0, 0, 0} },
		{1, {65, 193, 0, 0} }
};

MLINK_DEF MDef  =
{
	sizeof(FDef)/sizeof(MLFUN_DEF),
	FDef,
};

U8 MasterBlockDown(LINK_SEL LSel, LINK_STAT *LStat, DOWN_PTR_U8 SrcPtr, U8 Count, U16 DestAddress)
{
	return (U8) Link(LSel, &MDef, LStat, 0, SrcPtr, Count, DestAddress);
}

U8 MasterBlockUp(LINK_SEL LSel, LINK_STAT *LStat, U16 SrcAddress, UP_PTR_U8 DestPtr, U8 Count)
{
	return (U8) Link(LSel, &MDef, LStat, 1, SrcAddress, DestPtr, Count);
}

U32 GetSlaveParameter(LINK_SEL LSel, LINK_STAT *LStat, U8 ParCode, U8 Index)
{
	return (U32) Link(LSel, &MDef, LStat, 2, ParCode, Index);
}

U32 SetSlaveParameter(LINK_SEL LSel, LINK_STAT *LStat, U8 ParCode, U16 Param1, U16 Param2)
{
	return (U32) Link(LSel, &MDef, LStat, 3, ParCode, Param1, Param2);
}

U8 SetServoUsec(LINK_SEL LSel, LINK_STAT *LStat, U8 ServoIndex, U16 USec)
{
	return (U8) Link(LSel, &MDef, LStat, 4, ServoIndex, USec);
}

U16 GetServoUsec(LINK_SEL LSel, LINK_STAT *LStat, U8 ServoIndex)
{
	return (U16) Link(LSel, &MDef, LStat, 5, ServoIndex);
}

U8 DataUpdate(LINK_SEL LSel, LINK_STAT *LStat, DOWN_PTR_U16 USecsPtr, U8 USecCount, UP_PTR_U16 ADValsPtr, U8 ADValCount)
{
	return (U8) Link(LSel, &MDef, LStat, 6, USecsPtr, USecCount, ADValsPtr, ADValCount);
}

