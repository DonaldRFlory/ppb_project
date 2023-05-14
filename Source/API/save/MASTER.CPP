 //--------------------------------------------------------------------------*
//
//                  Copyright (C) Bio-Rad Laboratories 1997
//
//  MODULE :        MASTER.C
//  Purpose:        Master link communication functions for packet based
//                  link.
//                  Legitimate functions based on slave function
//                  calling routine limitations:
//                      1)  Long, int, char, or void functions with
//                          zero parameters or one parameter of type long,
//                          int, or char.
//
//                      2)  Long, int, char, or void functions with
//                          two or three parameters of types char or int.
//
//                      3)  Any signed or unsigned variations on above.
//
//
//  Functions:      Name
//                  ---------------
//                  Link
//
//   Created by:   Don Flory  Derived from earlier serial link based
//                            MASTER.C
//
//   Date:         4-19-97
//   Modified:
//    12-14-98 Modified several UINTS to USHORT for WIN32 DLL
//
//--------------------------------------------------------------------------*/
#include "hostopsys.h"
#include "mastadapt.h"
#include    <stdarg.h>
#include    <stddef.h>
#include    <stdio.h>
#include    "link.h"
#include    "mlink.h"
#include	"type.h"
#include <limits.h>


static int LinkIsLittleEndian = FALSE;

static void LinkSendArg(LINK_CTRL &LCtrl, U32 I,  unsigned short ByteCount);
static bool LinkSendBlock(LINK_CTRL &LCtrl, U8 Flags, void *Ptr, U32 ByteCount);
static bool LinkGetBlock(LINK_CTRL &LCtrl, U8 Flags, void *Ptr, U32 ByteCount);
static U64 LinkGetReturn(LINK_CTRL &LCtrl, B64Union &u, U8 ReturnSize);

U64 Link(LINK_SEL LSel, MLINK_DEF *MDef, U8 FunCode, ...)

{
	float FVal;
	U32 * U32Ptr;
	bool FloatFlag;
	U32 BlockUpCount;
	U32 Arg[MAX_ARGS];
	void *UDPtrs[MAX_ARGS];
	U8 Flags[MAX_ARGS];
	UCHAR FIdx;
	va_list ap;
	U8 ArgSize;
	unsigned int i, ArgCnt;
	LINK_CTRL LCtrl;
	LINK_STAT LStat;
	B64Union u;

	va_start( ap, FunCode );

	FIdx = (UCHAR)(FunCode & 0xFF);//get 8 bit function code
	LStat.FIdx = FIdx;
	LStat.Channel = LSel.ChannelIndex;
	LStat.CommType = LSel.CommType;
	LStat.Stat = LE_NO_ERROR;

	if((FunCode >= MDef->FunctionCount) )
	{
		LStat.Stat = LE_BAD_FUNCODE_M;
		LINK_STATUS(LStat);
		return 0;
	}

	if(!VALIDATE_LINK_SEL(LSel))
	{
	    LStat.Stat = LE_BAD_FORMAT_M;
	    LINK_STATUS(LStat);
		return 0;
	}

	LCtrl.LSel = LSel;
	LStat.Stat = InitLCtrl(LCtrl);
	if(!GOOD_LINK_STATUS(LStat))
	{
	    LINK_STATUS(LStat);
		return 0;//bad handle or problem with buffer allocation
	}
  	LCtrl.Buffer[LCtrl.NextIndex++] = FIdx;
  	LCtrl.FIdx = FIdx;
	LCtrl.RtnSize = MDef->FDef[FIdx].fsz;

	for(i = 0; i < MAX_ARGS; ++i)
	{
		if(MDef->FDef[FIdx].asz[i] == 0)
		{
			break;
		}
	}
	ArgCnt = i;

	//We are now assuming longs and ints are same size
	#if(UINT_MAX < 0XFFFFFFFF)
	#error  Program is designed for environments where int size is at four least bytes
	#endif

	for ( i = 0; i < ArgCnt; ++i)
	{
		ArgSize = MDef->FDef[FIdx].asz[i] & (ARG_SIZE_MASK);
		FloatFlag =  (MDef->FDef[FIdx].asz[i] & F_FLOAT) != 0;
		Flags[i] = MDef->FDef[FIdx].asz[i] & PTR_TYPE_MASK;
		if(Flags[i])
		{  //if there is a pointer, get it into pointers array
			UDPtrs[i] = va_arg( ap, void * );
		}
		if(FloatFlag)
		{
			FVal = (float)va_arg(ap, double);
			U32Ptr = (U32*) &FVal;//type punning to copy it without conversion
			Arg[i] = *U32Ptr;
		}
		else
		{
			Arg[i] = va_arg( ap, unsigned int );//and get the parameter (or count for pointer)
		}
		if(Flags[i])
		{
			if(Arg[i] > MAX_LINK_BLOCK_SIZE)
			{
				LStat.Stat = LE_BLOCK_SIZE;
				LINK_STATUS(LStat);
				return 0;
			}
		}
		LinkSendArg(LCtrl, Arg[i], ArgSize);//and put it in call packet
	}
	//We have all the actual parameters in packet. Now if there
	//are any down pointers, we need to pack the data into packet
	//At this point we should also check that any block xfers fit in
	//the packet.
	BlockUpCount = 0;
	for ( i = 0; i < ArgCnt; ++i)
	{
		switch(Flags[i])
		{
			case F_DOWN_PTR_U8:
			case F_DOWN_PTR_U16:
			case F_DOWN_PTR_U32:
				//LinkSendBlock will return false if we try to send too much for
				//LCtrlBuf to hold but this may be more than we can actually send.
				if(!LinkSendBlock(LCtrl, Flags[i], UDPtrs[i], Arg[i]))
				{
					LStat.Stat = LE_BLOCK_SIZE;
					LINK_STATUS(LStat);
					return 0;
				}
				break;

			case F_UP_PTR_U8:
				BlockUpCount += BlockUpCount + (Arg[i] * sizeof(U8));
				break;

			case F_UP_PTR_U16:
				BlockUpCount += BlockUpCount + (Arg[i] * sizeof(U16));
				break;

			case F_UP_PTR_U32:
				BlockUpCount += BlockUpCount + (Arg[i] * sizeof(U32));
				break;

			case 0:
				break;		//no flag

			default:
				LStat.Stat = LE_BAD_FLAGS;
				LINK_STATUS(LStat);
				return 0;
		}
	}

	LCtrl.RtnSize = LCtrl.RtnSize + BlockUpCount;
	if((LCtrl.NextIndex - LCtrl.StartIndex) > MAX_SEND_SIZE(LCtrl.LSel))
	{
	    LStat.Stat = LE_BLOCK_SIZE;
		LINK_STATUS(LStat);
		return 0;
	}
	if(LCtrl.RtnSize > MAX_RETURN_SIZE(LCtrl.LSel))
	{
		LStat.Stat = LE_BLOCK_SIZE;
		LINK_STATUS(LStat);
		return 0;
	}

	if(DummyTransaction(LCtrl))
	{
	    LINK_STATUS(LStat);//so GetLastLinkError is reset to no LE_NO_ERROR
	    return 0;
    }

	LStat.Stat = LINK_TRANSACT(LCtrl);

//We want to check size of return here
//-------------------------------------
	if(LStat.Stat == LE_NO_ERROR)
	{
	   	LinkGetReturn(LCtrl, u, MDef->FDef[FIdx].fsz);//get return value out of response packet
		for ( i = 0; i < ArgCnt; ++i)
		{ //process any block Up data
			switch(Flags[i])
			{
				case F_UP_PTR_U8:
				case F_UP_PTR_U16:
				case F_UP_PTR_U32:
				if(!LinkGetBlock(LCtrl, Flags[i], UDPtrs[i], Arg[i]))
				{
					LStat.Stat = LE_BLOCK_UP_SHORT;
					LINK_STATUS(LStat);
					return(u.X64);
				}
				break;
			}
		}
		LINK_STATUS(LStat);
	   	return(u.X64);
	}
	else
	{
	    LINK_STATUS(LStat);
	    return 0;
	}
}


/*--------------------------------------------------------------------------*
//  Function:       LinkSendArg
//  Description:   Copy argument stored in the argument union into the
//                  packet buffer. Number of bytes to send (size of argument)
//                  is passed in ByteCount.
//--------------------------------------------------------------------------*/
static void LinkSendArg(LINK_CTRL &LCtrl, U32 I,  unsigned short ByteCount)
{
  int i;
  ISCUnion u;
  u.I = I;

  if(LinkIsLittleEndian)
  {
    for (i=0; i < ByteCount; ++i)
    {
      LCtrl.Buffer[LCtrl.NextIndex++] = u.C[i];
    }
  }
  else
  {
    for (i=ByteCount-1; i >= 0; --i)
    {
      LCtrl.Buffer[LCtrl.NextIndex++] = u.C[i];
    }
  }
}


static bool LinkSendU8(LINK_CTRL &LCtrl, U8 Value)
{
	if(LCtrl.NextIndex >= LCtrl.BuffSize)
	{
		return false;
	}
	LCtrl.Buffer[LCtrl.NextIndex++] = Value;
	return true;
}

static bool LinkSendU16(LINK_CTRL &LCtrl, U16 Value)
{
	if((LCtrl.NextIndex + 1) >= LCtrl.BuffSize)
	{
		return false;
	}
	LCtrl.Buffer[LCtrl.NextIndex++] = (U8)(Value >> 8);
	LCtrl.Buffer[LCtrl.NextIndex++] = (U8)Value;
	return true;
}

static bool LinkSendU32(LINK_CTRL &LCtrl, U32 Value)
{
	if((LCtrl.NextIndex + 3) >= LCtrl.BuffSize)
	{
		return false;
	}
	LCtrl.Buffer[LCtrl.NextIndex++] = (U8)(Value >> 24);
	LCtrl.Buffer[LCtrl.NextIndex++] = (U8)(Value >> 16);
	LCtrl.Buffer[LCtrl.NextIndex++] = (U8)(Value >> 8);
	LCtrl.Buffer[LCtrl.NextIndex++] = (U8)Value;
	return true;
}


static bool LinkGetU8(LINK_CTRL &LCtrl, U8 &Value)
{
	if((LCtrl.StartIndex + sizeof(U8)) > LCtrl.NextIndex)
	{
		return false;
	}
	Value = LCtrl.Buffer[LCtrl.StartIndex++];
	return true;
}

static bool LinkGetU16(LINK_CTRL &LCtrl, U16 &Value)
{
	if((LCtrl.StartIndex + sizeof(U16)) > LCtrl.NextIndex)
	{
		return false;
	}
	int Temp;	//eliminate potential truncation warning
	Temp = LCtrl.Buffer[LCtrl.StartIndex++];
	Temp <<= 8;
	Temp |= LCtrl.Buffer[LCtrl.StartIndex++];
	Value = (U16)Temp;
	//Value |=  LCtrl.Buffer[LCtrl.StartIndex++];
	return true;
}

//Range of access validated before call
static bool LinkGetU32(LINK_CTRL &LCtrl, U32 &Value)
{
	if((LCtrl.StartIndex + sizeof(U32)) > LCtrl.NextIndex)
	{
		return false;
	}
	Value = LCtrl.Buffer[LCtrl.StartIndex++];
	Value <<= 8;
	Value |=  LCtrl.Buffer[LCtrl.StartIndex++];
	Value <<= 8;
	Value |=  LCtrl.Buffer[LCtrl.StartIndex++];
	Value <<= 8;
	Value |=  LCtrl.Buffer[LCtrl.StartIndex++];
	return true;
}


//This function only limits sends to size of LCtrl Buffer. This may be too long
//for the particular link target.
static bool LinkSendBlock(LINK_CTRL &LCtrl, U8 Flags, void *Ptr, U32 Count)
{
	U32 i;
	switch(Flags)
	{
		case F_DOWN_PTR_U8:
			for(i = 0; i < Count; ++i)
			{
				if(!LinkSendU8(LCtrl, ((U8 *)Ptr)[i]))
				{
					return false;
				}
			}
			break;

		case F_DOWN_PTR_U16:
			for(i = 0; i < Count; ++i)
			{
				if(!LinkSendU16(LCtrl, ((U16 *)Ptr)[i]))
				{
					return false;
				}
			}
			break;

		case F_DOWN_PTR_U32:
			for(i = 0; i < Count; ++i)
			{
				if(!LinkSendU32(LCtrl, ((U32 *)Ptr)[i]))
				{
					return false;
				}
			}
			break;

		default:
			return false;
	}
	return true;
}

static bool LinkGetBlock(LINK_CTRL &LCtrl, U8 Flags, void *Ptr, U32 Count)
{
	U32 i;
	switch(Flags)
	{
		case F_UP_PTR_U8:
			for(i = 0; i < Count; ++i)
			{
				if(!LinkGetU8(LCtrl, ((U8 *)Ptr)[i]))
				{
					return false;
				}
			}
			break;

		case F_UP_PTR_U16:
			for(i = 0; i < Count; ++i)
			{
				if(!LinkGetU16(LCtrl, ((U16 *)Ptr)[i]))
				{
					return false;
				}
			}
			break;

		case F_UP_PTR_U32:
			for(i = 0; i < Count; ++i)
			{
				if(!LinkGetU32(LCtrl, ((U32 *)Ptr)[i]))
				{
					return false;
				}
			}
			break;

		default:
			return false;
	}
	return true;
}


static U64 LinkGetReturn(LINK_CTRL &LCtrl, B64Union &u, U8 ReturnSize)
{
	int i, j;
	u.X64 = 0; /*clear union where return value is assembled*/

	if((LCtrl.StartIndex + ReturnSize) > LCtrl.NextIndex)
	{
		return false;
	}
	if(LinkIsLittleEndian)
	{
		for (i = 0 ; i < ReturnSize; ++i )
		{
			u.X8[i] = LCtrl.Buffer[LCtrl.StartIndex++];
		}
	}
	else
	{ //normal big-endian link
		for (i = 0, j = ReturnSize-1; i < ReturnSize; ++i, --j )
		{
			u.X8[j] = LCtrl.Buffer[LCtrl.StartIndex++];
		}
	}
	return (TRUE);
}
