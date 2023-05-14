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
//#include "hostopsys"
#include <limits.h>
#include "mlink.h"
#include  "slink.h"
//#include "type.h"
#define UINT_SIZE (sizeof U32))

//Link packets are always packed Big Endian.
//We are assuming little-endian ARM environment
#define REVERSE_BYTE_ORDER

//LDef is inited at start of a function processing to data for current function
//being called.
struct LinkDef LDef;
U8 LinkCallStatus;

static int LRet(U8 Count);
static int LRcv( U8 Count);

//Union below is used to efficiently pass arguments to/from link
// communication functions.
B64Union U;

//May be used by link functions to post an error status during execution
void PostLinkCallStatus(U8 ErrCode)
{
    LinkCallStatus = ErrCode;
}

//Define below needs to be appropriate to processors natural integer size
//#define INT_IS_32BIT
#undef INT_IS_32BIT
#ifdef INT_IS_32BIT
    #define MAX_ARG_BYTES 4
    #define UINT_T U32
    #define UINT_ELEM X32  //component of B64Union corresponding to processor integer size
#else
    #define MAX_ARG_BYTES 2
    #define UINT_T U16
    #define UINT_ELEM X16  //component of B64Union corresponding to processor integer size
#endif
bool LDefValid()
{
    if( (LDef.Arg1Size > MAX_ARG_BYTES) ||
        (LDef.Arg2Size > MAX_ARG_BYTES) ||
        (LDef.Arg3Size > MAX_ARG_BYTES) ||
        (LDef.Arg4Size > MAX_ARG_BYTES)
    ) return false;
    return true;
}

//So if all goes well in SlaveLinkProc, the return value has been setup in the return packet buffer.
//SlaveLink will return a status byte indicating any errors. Certain errors may indicate the function was not called.
//LE_BAD_PARAM, LE_BAD_TABLE in fact any bad return probably means function was not called.
U8 SlaveLinkProc(void)
{
	static UINT_T A1, A2, A3, A4;
	int ArgCount = 0;
	U16 Code = RcvChar();

	if(Code > 0XFF)
	{
		return LE_BAD_FUNCODE_S;
	}
   	if(!GetFDef((U8)Code, &LDef))//returns TRUE if valid command code
	{
		return LE_BAD_FUNCODE_S;
	}

	if(!LDefValid())
	{
		return LE_BAD_TABLE;//mainly see if arg sizes fit in current processors integer size
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
		A1 = U.UINT_ELEM[0];
		++ArgCount;
		if(!LDef.Arg2Size)
		{
			break;
		}
		if(!LRcv(LDef.Arg2Size))
		{
			return LE_BAD_FORMAT;
		}
		A2 = U.UINT_ELEM[0];
		++ArgCount;
		if(!LDef.Arg3Size)
		{
			break;
		}
		if(!LRcv(LDef.Arg3Size))
		{
			return LE_BAD_FORMAT;
		}
		A3 = U.UINT_ELEM[0];
		++ArgCount;
		if(!LDef.Arg4Size)
		{
			break;
		}
		if(!LRcv(LDef.Arg4Size))
		{
			return LE_BAD_FORMAT;
		}
		A4 = U.UINT_ELEM[0];
		++ArgCount;
		break;
	}
	//so if we are here, we have correct ArgCount and we have retrieved all argument bytes
	//Sets up pointers so that they are ready to supply any DownBlock bytes to RcvChar(void) calls.
	SetExPhase(EX_CALL, LDef.FunSize);
	LinkCallStatus = LE_NO_ERROR;//init status for link call to OK
	U.X64 = 0;

	switch(ArgCount)
	{
		case 0:
			switch ( LDef.FunSize )
			{
				case 0: /*void function*/
				//we can handle void function as if it is char fun since
				//garbage value will not be returned due to zero FunSIze.
				case sizeof (U8):
					U.X8[0] = ( (U8 (*)(void))(LDef.FunP) )();
					break;

				case sizeof (U16):
					U.X16[0] = ( (U16 (*)(void))(LDef.FunP) )();
					break;

				case sizeof (U32):
					U.X32[0] = ( (U32 (*)(void))(LDef.FunP) )();
					break;

				case sizeof (U64):
					U.X64 = ( (U64 (*)(void))(LDef.FunP) )();
					break;

				default:
					return LE_BAD_TABLE;
			}
			break;

		case 1:
			switch ( LDef.FunSize )
			{
				case 0: /*void function*/
					//we can handle void function as if it is char fun since
					//garbage value will not be returned due to zero FunSIze.
				case sizeof (U8):
					U.X8[0] = ( (U8 (*)(UINT_T a1))(LDef.FunP) )(A1);
					break;

				case sizeof (U16):
					U.X16[0] = ( (U16 (*)(UINT_T a1))(LDef.FunP) )(A1);
					break;

				case sizeof (U32):
					U.X32[0] = ( (U32 (*)(UINT_T a1))(LDef.FunP) )(A1);
					break;

				case sizeof (U64):
					U.X64 = ( (U64 (*)(UINT_T a1))(LDef.FunP) )(A1);
					break;

				default:
					return LE_BAD_TABLE;
			}
			break;

		case 2:
			switch ( LDef.FunSize )
			{
				case 0: /*void function*/
					//we can handle void function as if it is char fun since
					//garbage value will not be returned due to zero FunSize.

				case sizeof (U8):
					U.X8[0] = ( (U8 (*)(UINT_T a1, UINT_T a2))(LDef.FunP) )(A1,A2);
					break;

				case sizeof (U16):
					U.X16[0] = ( (U16 (*)(UINT_T a1, UINT_T a2))(LDef.FunP) )(A1,A2);
					break;

				case sizeof (U32):
					U.X32[0] = ( (U32 (*)(UINT_T a1, UINT_T a2))(LDef.FunP) )(A1,A2);
					break;

				case sizeof (U64):
					U.X64 = ( (U64 (*)(UINT_T a1, UINT_T a2))(LDef.FunP) )(A1,A2);
					break;

				default:
					return LE_BAD_TABLE;
			}
			break;

		case 3:
			switch ( LDef.FunSize )
			{
				case 0: /*void function*/
					//we can handle void function as if it is char fun since
					//garbage value will not be returned due to zero FunSize.

				case sizeof (U8):
					U.X8[0] = ( (U8 (*)(UINT_T a1, UINT_T a2, UINT_T a3))(LDef.FunP) )(A1,A2,A3);
					break;

				case sizeof (U16):
					U.X16[0] = ( (U16 (*)(UINT_T a1, UINT_T a2, UINT_T a3))(LDef.FunP) )(A1,A2,A3);
					break;

				case sizeof (U32):
					U.X32[0] = ( (U32 (*)(UINT_T a1, UINT_T a2, UINT_T a3))(LDef.FunP) )(A1,A2,A3);
					break;

				case sizeof (U64):
					U.X64 = ( (U64 (*)(UINT_T a1, UINT_T a2, UINT_T a3))(LDef.FunP) )(A1,A2,A3);
					break;

				default:
					return LE_BAD_TABLE;
			}
			break;

		case 4:
			switch ( LDef.FunSize )
			{
				case 0: /*void function*/
					//we can handle void function as if it is char fun since
					//garbage value will not be returned due to zero FunSize.

				case sizeof (U8):
					U.X8[0] = ( (U8 (*)(UINT_T a1, UINT_T a2, UINT_T a3, UINT_T a4))(LDef.FunP) )(A1,A2,A3,A4);
					break;

				case sizeof (U16):
					U.X16[0] = ( (U16 (*)(UINT_T a1, UINT_T a2, UINT_T a3, UINT_T a4))(LDef.FunP) )(A1,A2,A3,A4);
					break;

				case sizeof (U32):
					U.X32[0] = ( (U32 (*)(UINT_T a1, UINT_T a2, UINT_T a3, UINT_T a4))(LDef.FunP) )(A1,A2,A3,A4);
					break;

				case sizeof (U64):
					U.X64 = ( (U64 (*)(UINT_T a1, UINT_T a2, UINT_T a3, UINT_T a4))(LDef.FunP) )(A1,A2,A3,A4);
					break;

				default:
					return LE_BAD_TABLE;
			}
			break;
		}

		SetExPhase(EX_RETURN, 0);
		LRet(LDef.FunSize);//pass back return value in union to packet handling
		return LinkCallStatus;
}

int RcvFloat(float *Value)
{
	if(LRcv(4))
	{
		*Value = U.F[0];
		return TRUE;
	}
	return FALSE;
}

int RcvU32(U32 *Value)
{
	if(LRcv(4))
	{
		*Value = U.X32[0];
		return TRUE;
	}
	return FALSE;
}

int RcvU16(U16 *Value)
{
	if(LRcv(2))
	{
		*Value = U.X16[0];
		return TRUE;
	}
	return FALSE;
}

int RcvU8(U8 *Value)
{
	if(LRcv(1))
	{
		*Value = U.X8[0];
		return TRUE;
	}
	return FALSE;
}

int SendFloat(float Value)
{
	U.F[0] = Value;
	return LRet(4);
}

int SendU32(U32 Value)
{
	U.X32[0] = Value;
	return LRet(4);
}

int SendU16(U16 Value)
{
	U.X16[0] = Value;
	return LRet(2);
}

int SendU8(U8 Value)
{
	U.X8[0] = Value;
	return LRet(1);
}

#ifdef REVERSE_BYTE_ORDER
static int LRcv(U8 Count)
{
	int i;
	int ICnt;
	U.X64 = 0;
	for (ICnt = ((int)Count)-1; ICnt >= 0; --ICnt)
	{
		if((i = RcvChar()) == NO_CHAR)
		{
				return FALSE;
		}
		U.X8[ICnt] = (unsigned char) i;
	}
	return TRUE;
}

static int LRet(U8 Count)
{
	int ICnt;

	//Sets up pointers and count so SendChar calls put the return value in the response packet
	for (ICnt=((int)Count)-1; ICnt >= 0; --ICnt )
	{
		if(!SendChar(U.X8[ICnt]))
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
	U.X64 = 0;
	for (uChar = 0; uChar < Count; ++uChar)
	{
		if((i = RcvChar()) == NO_CHAR)
		{
				return FALSE;
		}
		U.X8[uChar] = (unsigned char) i;
	}
	//Since we always pass byte arguments to functions as if they were int's,
	//a single byte must be explicitly converted to an int to make sure the
	//byte is in the correct location. (big-endian/little-endian)
	if(Count == 1)
		U.I[0] = (unsigned int) U.X8[0];
	return TRUE;
}

static int LRet(U8 Count)
{
	//Sets up pointers in response packet to receive Return value bytes
	//via SendChar().
	for (uChar=0; uChar < Count; ++uChar )
	{
		if(!SendChar(U.X8[uChar]))
		{
		   return FALSE;
		}
	}
	return TRUE;
}
#endif
