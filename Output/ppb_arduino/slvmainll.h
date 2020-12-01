// ..\..\output\ppb_arduino\slvmainll.h**************************************
// This file is machine generated from the link definition file
// mainll.h by LDFUTIL V2_015
// Processed in Mode: 9 - DLL unified export Mode
// It should not be directly edited.
U8 MasterBlockDown(U8 Count, U16 DestAddress);
U8 MasterBlockUp(U16 SrcAddress, U8 Count);
U32 GetSlaveParameter(U8 ParCode, U8 Index);
U32 SetSlaveParameter(U8 ParCode, U16 Param1, U16 Param2);
U8 SetServoUsec(U8 ServoIndex, U16 USec);
U16 GetServoUsec(U8 ServoIndex);
U8 DataUpdate(U8 USecCount, U8 ADValCount);
