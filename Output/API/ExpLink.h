// ..\..\output\API\ExpLink.h**************************************
// This file is machine generated from the link definition file
// ..\Common\mainll.h by LDFUTIL V2_015
// Processed in Mode: 2 - DLL export stubs Mode
// It should not be directly edited.

#include "mlink.h"

extern "C" {
PPB_API API_STAT CALL_CONV MasterBlockDown(API_DEVICE_HANDLE Handle, U8 &RtnVal, DOWN_PTR_U8 SrcPtr, U8 Count, U16 DestAddress);
PPB_API API_STAT CALL_CONV MasterBlockUp(API_DEVICE_HANDLE Handle, U8 &RtnVal, U16 SrcAddress, UP_PTR_U8 DestPtr, U8 Count);
PPB_API API_STAT CALL_CONV GetSlaveParameter(API_DEVICE_HANDLE Handle, U32 &RtnVal, U8 ParCode, U8 Index);
PPB_API API_STAT CALL_CONV SetSlaveParameter(API_DEVICE_HANDLE Handle, U32 &RtnVal, U8 ParCode, U16 Param1, U16 Param2);
PPB_API API_STAT CALL_CONV SetServoUsec(API_DEVICE_HANDLE Handle, U8 &RtnVal, U8 ServoIndex, U16 USec);
PPB_API API_STAT CALL_CONV GetServoUsec(API_DEVICE_HANDLE Handle, U16 &RtnVal, U8 ServoIndex);
PPB_API API_STAT CALL_CONV DataUpdate(API_DEVICE_HANDLE Handle, U8 &RtnVal, DOWN_PTR_U16 USecsPtr, U8 USecCount, UP_PTR_U16 ADValsPtr, U8 ADValCount);
} // End of Extern "C"
