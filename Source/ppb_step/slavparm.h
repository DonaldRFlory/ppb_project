//--------------------------------------------------------------------------
//                Copyright (C) Bio-Rad Laboratories 2015
//
//  MODULE :     slavparm.h
//  Purpose:     This file defines basic parameter IDs for GetSlaveParameter and SetSlaveParameter
//				 calls to be supported on all interboard channels using link call mechanism
//               This file is used in both host test programs and embedded firmware.
//
//--------------------------------------------------------------------------
#ifndef SLAVPARM_H
#define SLAVPARM_H
#define RESTART_MAGIC_ONE 0xA5A5
#define RESTART_MAGIC_TWO 0x1234

//SPAR_CS_CALC index values:
#define BYTEWISE_CHECKSUM	0
#define LONGWISE_CHECKSUM	1
#define ANSI_CRC16			2

//Parameter codes for GetSlaveParameter and SetSlaveParameter
//Basic set to be supported by all boards implementing link
#define SPAR_PROGVERSION        0  //Get only
#define SPAR_CS_START           1  //Get/Set
#define SPAR_CS_END             2  //Get/Set
#define SPAR_CS_CALC            3  //Get only
#define SPAR_LINK_BUFF_ADDRESS  4  //Get only
#define SPAR_LINK_BUFF_LEN      5  //Get only
#define SPAR_MSECS              6  //Get only
#define SPAR_MISC_COUNTS        7  //Get only
#define SPAR_RESET_CPU          8  // Set only
#define SPAR_VERBLOCK_ADDRESS   9  // Get only
#define SPAR_VERBLOCK_LENGTH    10  // Get only
#define SPAR_SPARVERSION        11 // Get only version of basic link functions
#define SPAR_BUILD_TYPE	        12 // Get only
#define SPAR_STEPPERS_INTERLOCKED	      20 // Get/Set
#define SPAR_STEPPERS_LOCAL	      21 // Get/Set
#define SPAR_PARS1				  22
#define SPAR_PARS2				  23
#define SPAR_PARS3				  24
#define SPAR_PARS4				  25
#define SPAR_ROT_RADIUS 		  26
#define SPAR_ROT_ACTIVE			  28
#define SPAR_ROT_SERVE			  29
#define SPAR_ROT_ISR			  30
#define SPAR_SEG_STEP             31
#define SPAR_TRACE_TEST           32
#define SPAR_ROT_RPM_X1000        33

//unsigned long GetMiscCounts(unsigned short Index);
//void SetMiscCounts(unsigned short ParOne, unsigned short ParTwo);
U32 GetSlaveParameter(U32 ParCode, U32 Index);
U8 SetSlaveParameter(U32 ParCode, U32 ParOne, U32 ParTwo);
U16 ComputeCRC(U8 * Buff, U32 CharCount);
#endif
