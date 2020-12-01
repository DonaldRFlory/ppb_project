//--------------------------------------------------------------------------
//
//          Copyright (C) Bio-Rad Laboratories 1997-2005, 2015
//
//  Module:      SLAVE.C
//
//  Purpose:     Implement slave side of master/slave function calling
//               interface over serial comm. This version removes hooks
//               for 8051, Z80, X86 family processors. Now targetted
//               for ARM processors.
//
//
//   Created by:     Don Flory
//   Date:
//
//   Modified:    10-17-2016 Reworked extensively. Eliminated BlockDown and BlockUp
//				  functions as new block xfer functions may be defined in linkist.
//				   Now there are no Built-in link functions. MasterBlockDown() and
//				   MasterBlockUp() are by convention the first two functions in the
//				   basicll.h but appear explicitly in the linklist.
//
// $Revision: $
// $Author: $
// $Date: $
//
//---------------------------------------------------------------------------
#include <limits.h>
#include "link.h"
#include  "slink.h"
//#include "type.h"
#define UINT_SIZE (sizeof U32))

//Link packets are always packed Big Endian.
//We are assuming little-endian ARM environment
#define REVERSE_BYTE_ORDER

//LDef is inited at start of a function processing to data for current function
//being called.
struct LinkDef LDef;

static int LRet(U8 Count);
static int LRcv( U8 Count);

//Union below is used to efficiently pass arguments to/from link
// communication functions.

union
{
    float F;
    U32 I;
    U16 S[sizeof(U32)/sizeof(U16)];
    unsigned char C[sizeof(U32)];
}U;

//Before this is called, higher level LinkServe() has selected the link (if more than one)
//and verified that it is in EXECUTE state and ready for function code and argument. Command
//packet has been verified as valid as far as CRC or Checksum if implemented
//
//So if all goes well in SlaveLinkProc, the return value has been setup in the return packet buffer.
//SlaveLink will return a status byte indicating any errors. Certain errors may indicate the function was not called.
//LE_BAD_PARAM, LE_BAD_TABLE in fact any bad return probably means function was not called.
U8 SlaveLinkProc(void)
{
	#if( 0XFFFFFFFF > UINT_MAX)
	#error Error: Code will not work on targets with less than 32 bit word size
	#endif
	static U32 A1, A2, A3, A4;
	int ArgCount = 0;
	U16 Code = RcvChar();

	if(Code > 0XFF)
	{
		return LE_BAD_FUNCODE;
	}
   	if(!GetFDef((U8)Code, &LDef))//returns TRUE if valid command code
	{
		return LE_BAD_FUNCODE;
	}

	while(1)
    {
		if(!LDef.Arg1Size)
		{
			break;
		}
		if(!LRcv(LDef.Arg1Size))
		{
			return LE_BAD_FORMAT;
		}
		A1 = U.I;
		++ArgCount;
		if(!LDef.Arg2Size)
		{
			break;
		}
		if(!LRcv(LDef.Arg2Size))
		{
			return LE_BAD_FORMAT;
		}
		A2 = U.I;
		++ArgCount;
		if(!LDef.Arg3Size)
		{
			break;
		}
		if(!LRcv(LDef.Arg3Size))
		{
			return LE_BAD_FORMAT;
		}
		A3 = U.I;
		++ArgCount;
		if(!LDef.Arg4Size)
		{
			break;
		}
		if(!LRcv(LDef.Arg4Size))
		{
			return LE_BAD_FORMAT;
		}
		A4 = U.I;
		++ArgCount;
		break;
	}
	//so if we are here, we have correct ArgCount and we have retrieved all argument bytes
	//Sets up pointers so that they are ready to supply any DownBlock bytes to RcvChar(void) calls.
	SetExPhase(EX_CALL, LDef.FunSize);

	U.I = 0;
	switch(ArgCount)
	{
		case 0:
			switch ( LDef.FunSize )
			{
				case 0: /*void function*/
				//we can handle void function as if it is char fun since
				//garbage value will not be returned due to zero FunSIze.
				case sizeof (U8):
					U.C[0] = ( (U8 (*)(void))(LDef.FunP) )();
					break;
				case sizeof (U16):
					U.S[0] = ( (U32 (*)(void))(LDef.FunP) )();
					break;
				case sizeof (U32):
					U.I = ( (U32 (*)(void))(LDef.FunP) )();
					break;
				default:
					return LE_BAD_TABLE;
			}
			break;

		case 1:
			switch ( LDef.FunSize )
			{
				case	0: /*void function*/
					//we can handle void function as if it is char fun since
					//garbage value will not be returned due to zero FunSIze.
				case    sizeof (char):
					U.C[0] = ( (U8 (*)(U32 a1))(LDef.FunP) )(A1);
					break;
				case    sizeof (short):
					U.S[0] = ( (U16 (*)(U32 a1))(LDef.FunP) )(A1);
					break;
				case sizeof (U32):
					U.I = ( (U32 (*)(U32 a1))(LDef.FunP) )(A1);
					break;
				default:
					return LE_BAD_TABLE;
			}
			break;

		case 2:
			switch ( LDef.FunSize )
			{
				case    0: /*void function*/
					//we can handle void function as if it is char fun since
					//garbage value will not be returned due to zero FunSize.
				case    sizeof (U8):
					U.C[0] = ( (U8 (*)(U32 a1, U32 a2))(LDef.FunP) )(A1,A2);
					break;
				case    sizeof (U16):
					U.S[0] = ( (U16 (*)(U32 a1, U32 a2))(LDef.FunP) )(A1,A2);
					break;
				case    sizeof (U32):
					U.I = ( (U32 (*)(U32 a1, U32 a2))(LDef.FunP) )(A1,A2);
					break;
				default:
					return LE_BAD_TABLE;
			}
			break;

		case 3:
			switch ( LDef.FunSize )
			{
				case    0: /*void function*/
					//we can handle void function as if it is char fun since
					//garbage value will not be returned due to zero FunSize.
				case    sizeof (U8):
					U.C[0] = ( (U8 (*)(U32 a1, U32 a2, U32 a3))(LDef.FunP) )(A1,A2,A3);
					break;
				case    sizeof (U16):
					U.S[0] = ( (U16 (*)(U32 a1, U32 a2, U32 a3))(LDef.FunP) )(A1,A2,A3);
					break;
				case    sizeof (U32):
					U.I = ( (U32 (*)(U32 a1, U32 a2, U32 a3))(LDef.FunP) )(A1,A2,A3);
					break;
				default:
					return LE_BAD_TABLE;
			}
			break;

		case 4:
			switch ( LDef.FunSize )
			{
				case    0: /*void function*/
					//we can handle void function as if it is char fun since
					//garbage value will not be returned due to zero FunSize.
				case    sizeof (U8):
					U.C[0] = ( (U8 (*)(U32 a1, U32 a2, U32 a3, U32 a4))(LDef.FunP) )(A1,A2,A3,A4);
					break;
				case    sizeof (U16):
					U.S[0] = ( (U16 (*)(U32 a1, U32 a2, U32 a3, U32 a4))(LDef.FunP) )(A1,A2,A3,A4);
					break;
				case    sizeof (U32):
					U.I = ( (U32 (*)(U32 a1, U32 a2, U32 a3, U32 a4))(LDef.FunP) )(A1,A2,A3,A4);
					break;
				default:
					return LE_BAD_TABLE;
			}
			break;
		}

		SetExPhase(EX_RETURN, 0);
		LRet(LDef.FunSize);//pass back return value in union to packet handling
		return LE_NO_ERROR;
}


int RcvFloat(float *Value)
{
	if(LRcv(4))
	{
		*Value = U.F;
		return TRUE;
	}
	return FALSE;
}

int RcvU32(U32 *Value)
{
	if(LRcv(4))
	{
		*Value = U.I;
		return TRUE;
	}
	return FALSE;
}

int RcvU16(U16 *Value)
{
	if(LRcv(2))
	{
		*Value = U.S[0];
		return TRUE;
	}
	return FALSE;
}

int RcvU8(U8 *Value)
{
	if(LRcv(1))
	{
		*Value = U.C[0];
		return TRUE;
	}
	return FALSE;
}

int SendFloat(float Value)
{
	U.F = Value;
	return LRet(4);
}

int SendU32(U32 Value)
{
	U.I = Value;
	return LRet(4);
}

int SendU16(U16 Value)
{
	U.S[0] = Value;
	return LRet(2);
}

int SendU8(U8 Value)
{
	U.C[0] = Value;
	return LRet(1);
}

#ifdef REVERSE_BYTE_ORDER
static int LRcv(U8 Count)
{
	short i;
	int ICnt;
	U.I = 0;
	for (ICnt = ((int)Count)-1; ICnt >= 0; --ICnt)
	{
		if((i = RcvChar()) == NO_CHAR)
		{
				return FALSE;
		}
		U.C[ICnt] = (unsigned char) i;
	}
	return TRUE;
}

static int LRet(U8 Count)
{
	int ICnt;

	//Sets up pointers and count so SendChar calls put the return value in the response packet
	for (ICnt=((int)Count)-1; ICnt >= 0; --ICnt )
	{
		if(!SendChar(U.C[ICnt]))
		{
			return FALSE;
		}
	}
	return TRUE;
}

#else
//leave byte order alone
static int LRcv( U8 Count)
{
	U.I = 0;
	for (uChar = 0; uChar < Count; ++uChar)
	{
		if((i = RcvChar()) == NO_CHAR)
		{
				return FALSE;
		}
		U.C[uChar] = (unsigned char) i;
	}
	//Since we always pass byte arguments to functions as if they were int's,
	//a single byte must be explicitly converted to an int to make sure the
	//byte is in the correct location. (big-endian/little-endian)
	if(Count == 1)
		U.I[0] = (unsigned int) U.C[0];
	return TRUE;
}

static int LRet(U8 Count)
{
	//Sets up pointers in response packet to receive Return value bytes
	//via SendChar().
	for (uChar=0; uChar < Count; ++uChar )
	{
		if(!SendChar(U.C[uChar]))
		{
		   return FALSE;
		}
	}
	return TRUE;
}
#endif
