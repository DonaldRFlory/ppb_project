// ..\..\Output\ppb_arduino\slvmainll.h**************************************
// This file is machine generated from the link definition file
// mainll.h by LDFUTIL V2_022
// Processed in Mode: 9 - DLL unified export Mode
// It should not be directly edited.
U8 MasterBlockDown(U8 Count, U16 DestAddress);
U8 MasterBlockUp(U16 SrcAddress, U8 Count);
U32 GetSlaveParameter(U8 ParCode, U8 Index);
U32 SetSlaveParameter(U8 ParCode, U16 Param1, U16 Param2);
U8 SetServoUsec(U8 ServoIndex, U16 Usec);
U16 GetServoUsec(U8 ServoIndex);
U8 DataUpdate(U8 UsecCount, U8 ADValCount);
U8 SetSolenoid(U8 SolIndex, U8 Value);
void ServoSlewMove(U8 ServoIndex, U16 UsecTarget, float Velocity);
void SetStep(U8 StepperIndex, U16 PeriodCount, U16 Steps, U8 Dir);
void ProgramEEPROM(U16 Address, U8 Value);
U8 ReadEEPROM(U16 Address);
U8 ReadTraceBuffer(U8 TraceValCount);
U8 SetPWMVal(U8 ServoIndex, U16 Count);
U16 SetPWMFreq(U8 ChipIndex, U16 FreqX10);
U8 ServoUpdate(U8 UsecCount, U8 GroupIndex);
void LoadServoUsec(U8 ServoIndex, U16 Usec);
