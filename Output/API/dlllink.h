// ..\..\output\API\dlllink.h**************************************
// This file is machine generated from the link definition file
// ..\Common\mainll.h by LDFUTIL V2_015
// Processed in Mode: 0 - Standard stubs Mode
// It should not be directly edited.

#include "mlink.h"

U8 MasterBlockDown(LINK_SEL LSel, LINK_STAT *LStat, DOWN_PTR_U8 SrcPtr, U8 Count, U16 DestAddress);
U8 MasterBlockUp(LINK_SEL LSel, LINK_STAT *LStat, U16 SrcAddress, UP_PTR_U8 DestPtr, U8 Count);
U32 GetSlaveParameter(LINK_SEL LSel, LINK_STAT *LStat, U8 ParCode, U8 Index);
U32 SetSlaveParameter(LINK_SEL LSel, LINK_STAT *LStat, U8 ParCode, U16 Param1, U16 Param2);
U8 SetServoUsec(LINK_SEL LSel, LINK_STAT *LStat, U8 ServoIndex, U16 USec);
U16 GetServoUsec(LINK_SEL LSel, LINK_STAT *LStat, U8 ServoIndex);
U8 DataUpdate(LINK_SEL LSel, LINK_STAT *LStat, DOWN_PTR_U16 USecsPtr, U8 USecCount, UP_PTR_U16 ADValsPtr, U8 ADValCount);
