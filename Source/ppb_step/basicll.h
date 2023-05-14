//---------------------------------------------------------------------------
//
//
//      File: basicll.H
//
//      Purpose: This file is the master list of standard basic linklist
//				 functions callable over the link between Master and Slave processors
//               It is processed by the ldfutil program to
//               produce headers including function stubs and data tables
//               used by link communication programs at each processor.
//
//
//      Note:    Use only C++ style comments in this file
//               due to simple minded ldfgen.c parser.
//
//
//---------------------------------------------------------------------------
//
// Legitimate functions based on slave function calling routine limitations:
// 1) Long, int, char, or void functions with zero parameters or one
//          parameter of type long, int, or char.
// 2) Long, int, char, or void functions with two, three or four parameters
//          of types char or int.
// 3) Any signed or unsigned variations on above.
//
//This is just here to demonstrate inclusion of header file in another


U8 MasterBlockDown(DOWN_PTR_U8 SrcPtr, U8 Count, U16 DestAddress);
U8 MasterBlockUp(U16 SrcAddress, UP_PTR_U8 DestPtr, U8 Count);
U32 GetSlaveParameter(U8 ParCode, U8 Index);
U32 SetSlaveParameter(U8 ParCode, U16 Param1, U16 Param2);
