//---------------------------------------------------------------------------
//
//      File: MainLL.H
//
//      Purpose: This file is the master list of functions callable over a
//               link between Master and Slave processors
//               It is processed by the ldfutil program to
//               produce headers including function stubs and data tablese
//               used by link communication programs at each processor.
//
//
//      Note:    Use only C++ style comments in this file
//               due to simple minded ldfutil.cpp parser.
//
//
//---------------------------------------------------------------------------



//
// Legitimate functions based on slave function calling routine limitations:
// 1) Long, int, char, or void functions with zero to four parameters
//          of types any types which will pass as an integer, (char, short, int).
// 2) Any signed or unsigned variations on above.
//
// 3) Block tranfer. Each of the four parameters can and the host side consist of
//   two parameters in the host side stub, an UP_BLOCK_XX type or a DOWN_POINTER_XX type
//   followed by a count parameter. The XX represents U8, U16 or U32. Each pointer/count
//  pair counts as one parameter. These pairs may be mixed in master side functions
//	with standard arguments with the only restriction being that only four effective
//  arguments are allowed.
// There are no longer any built-in  built in link functions.
// linklist preprocessor program. Prototypes are in link.h
//

//Following three define turn on exporting of DLL functions in Modes 2 and 3 and 9 of LDFUtil. The idea
//is that only some of the functions in a linklist will be exported from the DLL and this allows
//turning exporting on and off during linklist processing. In this example we export everything
#define DRF_LDF_EXPORT 1	   //for benefit of LDFUTIL.EXE
//This define only applies to DLL exports
#define DRF_LDF_Prefix PPB_API

#include <basicll.h>

U8 SetServoUsec(U8 ServoIndex,U16 Usec);
U16 GetServoUsec(U8 ServoIndex);
U8 DataUpdate(DOWN_PTR_U16 UsecsPtr, U8 UsecCount, UP_PTR_U16 ADValsPtr, U8 ADValCount);
U8 SetSolenoid(U8 SolIndex, U8 Value);
void ServoSlewMove(U8 ServoIndex, U16 UsecTarget, float Velocity);
void SetStep(U8 StepperIndex, U16 PeriodCount, U16 Steps, U8 Dir);
void ProgramEEPROM(U16 Address, U8 Value);
U8 ReadEEPROM(U16 Address);
U8 ReadTraceBuffer(UP_PTR_U8 TraceValsPtr, U8 TraceValCount);
U8 SetPWMVal(U8 ServoIndex,U16 Count); //for PCA9685
U16 SetPWMFreq(U8 ChipIndex, U16 FreqX10);
//Top 4 bits of USec values below will be the index within the group
//of the Value in the lower 12 bits. This allows packet to only
//send values which have changed for link efficiency.
U8 ServoUpdate(DOWN_PTR_U16 UsecsPtr, U8 UsecCount, U8 GroupIndex);
void LoadServoUsec(U8 ServoIndex, U16 Usec);
