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
#include    <stdarg.h>
#include    <stddef.h>
#include    "link.h"
#include    "mlink.h"
#include    "apilinkadapt.h"
#include <limits.h>
//#include    <mlinkcom.h>


short LastLinkFun = -1; /*last function code for error funct.*/
int LinkIsLittleEndian = FALSE;

static void InitLCtrl(LINK_CTRL &LCtrl);
static void LinkSendArg(LINK_CTRL &LCtrl, U32 I,  unsigned short ByteCount);
static bool LinkSendBlock(LINK_CTRL &LCtrl, U8 Flags, void *Ptr, U16 ByteCount);
static bool LinkGetBlock(LINK_CTRL &LCtrl, U8 Flags, void *Ptr, U16 ByteCount);

static int LinkGetReturn(LINK_CTRL &LCtrl, ISCUnion &u, U8 ReturnSize);

//for now, we will pack the link call macro status in low byte.
//We may pack more info in higher three bytes.
static bool GoodLinkStatus(LINK_STAT LStat)
{
	return (LStat.Stat & 0xFF) == LE_NO_ERROR;
}

static bool ValidFlag(U8 Flag)
{
	switch(Flag)
	{
		case F_DOWN_PTR_U8:
		case F_DOWN_PTR_U16:
		case F_DOWN_PTR_U32:
		case F_UP_PTR_U8:
		case F_UP_PTR_U16:	
		case F_UP_PTR_U32:	
			return true;

		default:
			return false;
	}
}

//unsigned long Link(unsigned char Channel, MLINK_DEF *MDef, UINT FunCode, ...)
// (One of two overloaded versions)
unsigned long Link(LINK_SEL LSel, MLINK_DEF *MDef,
				LINK_STAT *LinkStat, U8 FunCode, ...)
{
	float FVal;
	U32 * U32Ptr;
	bool FloatFlag;
	U32 BlockUpCount;
	UINT Arg[MAX_ARGS];
	void *UDPtrs[MAX_ARGS];
	U8 Flags[MAX_ARGS];
	UCHAR FIdx;
	va_list ap;
	U8 ArgSize;
	unsigned int i, ArgCnt;
	LINK_CTRL LCtrl;
	ISCUnion u;

	va_start( ap, FunCode );

	if((FunCode >= MDef->FunctionCount) )
	{
		LinkStat->Stat = LE_BAD_FUNCODE;
		return 0;
	}
	LinkStat->FIdx = FunCode;
	
	if(LSel.LHand == NULL)
	{
		LinkStat->Stat = LE_BAD_HANDLE;
		return 0;
	}
	if(!ValidateLinkSel(LSel))
	{
		LinkStat->Stat = LE_BAD_FORMAT;
		return 0;
	}

	FIdx = (UCHAR)(FunCode & 0xFF);//get 8 bit function code
	LinkStat->FIdx = FIdx;
	InitLCtrl(LCtrl);
  	LCtrl.Buff[LCtrl.NextIndex++] = LCtrl.FIdx = FIdx;//put function code in buffer
	LCtrl.LSel = LSel;
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
	#error  Program is designed for environments where int size is at least bytes
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
				LinkStat->Stat = LE_BLOCK_SIZE;
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
					LinkStat->Stat = LE_BLOCK_SIZE;
					return 0;
				}
				break;
				
			case F_UP_PTR_U8:
				BlockUpCount += Arg[i] * sizeof(U8);
				break;

			case F_UP_PTR_U16:	
				BlockUpCount	+= Arg[i] * sizeof(U16);
				break;

			case F_UP_PTR_U32:	
				BlockUpCount	+= Arg[i] * sizeof(U32);
				break;

			case 0:
				break;		//no flag
			
			default:
				LinkStat->Stat = LE_BAD_PARAM_FLAG;
				return 0;
		}
	}

	LCtrl.RtnSize += (U16)BlockUpCount;
	if((LCtrl.NextIndex - LCtrl.StartIndex) > GetMaxLinkSendSize(LCtrl.LSel))
	{
		LinkStat->Stat = LE_BLOCK_SIZE;
		return 0;
	}
	if(LCtrl.RtnSize > GetMaxLinkReturnSize(LCtrl.LSel))
	{
		LinkStat->Stat = LE_BLOCK_SIZE;
		return 0;
	}
	
	*LinkStat = LinkTransact(LCtrl);
	if(GoodLinkStatus(*LinkStat))
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
					LinkStat->Stat = LE_BLOCK_SIZE;
					return 0;
				}
				break;
			}
		}
	   	return(u.I);
	}

	return 0;
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
      LCtrl.Buff[LCtrl.NextIndex++] = u.C[i];
    }
  }
  else
  {
    for (i=ByteCount-1; i >= 0; --i)
    {
      LCtrl.Buff[LCtrl.NextIndex++] = u.C[i];
    }
  }
}


static bool LinkSendU8(LINK_CTRL &LCtrl, U8 Value) 
{
	if(LCtrl.NextIndex >= (LCB_BUFFSIZE + LCB_PRESPACE))
	{
		return false;
	}
	LCtrl.Buff[LCtrl.NextIndex++] = Value;
	return true;
}

static bool LinkSendU16(LINK_CTRL &LCtrl, U16 Value) 
{
	if((LCtrl.NextIndex + 1) >= (LCB_BUFFSIZE + LCB_PRESPACE))
	{
		return false;
	}
	LCtrl.Buff[LCtrl.NextIndex++] = (U8)(Value >> 8);
	LCtrl.Buff[LCtrl.NextIndex++] = (U8)Value;
	return true;
}

static bool LinkSendU32(LINK_CTRL &LCtrl, U32 Value) 
{
	if((LCtrl.NextIndex + 3) >= (LCB_BUFFSIZE + LCB_PRESPACE))
	{
		return false;
	}
	LCtrl.Buff[LCtrl.NextIndex++] = (U8)(Value >> 24);
	LCtrl.Buff[LCtrl.NextIndex++] = (U8)(Value >> 16);
	LCtrl.Buff[LCtrl.NextIndex++] = (U8)(Value >> 8);
	LCtrl.Buff[LCtrl.NextIndex++] = (U8)Value;
	return true;
}


//Range of access validated before call
static bool LinkGetU8(LINK_CTRL &LCtrl, U8 &Value) 
{
	Value = LCtrl.Buff[LCtrl.StartIndex++];
	return true;
}

//Range of access validated before call
static bool LinkGetU16(LINK_CTRL &LCtrl, U16 &Value) 
{
	Value = LCtrl.Buff[LCtrl.StartIndex++];
	Value <<= 8;
	Value |=  LCtrl.Buff[LCtrl.StartIndex++];
	return true;
}

//Range of access validated before call
static bool LinkGetU32(LINK_CTRL &LCtrl, U32 &Value) 
{
	Value = LCtrl.Buff[LCtrl.StartIndex++];
	Value <<= 8;
	Value |=  LCtrl.Buff[LCtrl.StartIndex++];
	Value <<= 8;
	Value |=  LCtrl.Buff[LCtrl.StartIndex++];
	Value <<= 8;
	Value |=  LCtrl.Buff[LCtrl.StartIndex++];
	return true;
}


//This function only limits sends to size of LCtrl Buffer. This may be too long
//for the particular link target.
static bool LinkSendBlock(LINK_CTRL &LCtrl, U8 Flags, void *Ptr, U16 Count)
{
	int i;
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

static bool LinkGetBlock(LINK_CTRL &LCtrl, U8 Flags, void *Ptr, U16 Count)
{
	int i;
	switch(Flags)
	{
		case F_UP_PTR_U8:
			if((LCtrl.StartIndex + (Count * sizeof(U8))) > LCtrl.NextIndex)
			{
				return false;
			}
			for(i = 0; i < Count; ++i)
			{
				if(!LinkGetU8(LCtrl, ((U8 *)Ptr)[i]))
				{
					return false;
				}
			}
			break;
	
		case F_UP_PTR_U16:	
			if((LCtrl.StartIndex + (Count * sizeof(U16))) > LCtrl.NextIndex)
			{
				return false;
			}
			for(i = 0; i < Count; ++i)
			{
				if(!LinkGetU16(LCtrl, ((U16 *)Ptr)[i]))
				{
					return false;
				}
			}
			break;
	
		case F_UP_PTR_U32:	
			if((LCtrl.StartIndex + (Count * sizeof(U32))) > LCtrl.NextIndex)
			{
				return false;
			}
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


static int LinkGetReturn(LINK_CTRL &LCtrl, ISCUnion &u, U8 ReturnSize)
{
	int i, j;
	u.I =0; /*clear union where return value is assembled*/

	if((LCtrl.StartIndex + ReturnSize) > LCtrl.NextIndex)
	{
		return false;
	}
	if(LinkIsLittleEndian)
	{
		for (i = 0 ; i < ReturnSize; ++i )
		{
			u.C[i] = LCtrl.Buff[LCtrl.StartIndex++];
		}
	}
	else
	{ //normal big-endian link
		for (i = 0, j = ReturnSize-1; i < ReturnSize; ++i, --j )
		{
			u.C[j] = LCtrl.Buff[LCtrl.StartIndex++];
		}
	}
	return (TRUE);
}


static void InitLCtrl(LINK_CTRL &LCtrl)
{
	for(int i = 0; i < LCB_TOTSIZE; ++i)
	{
		LCtrl.Buff[i] = 0;//clear buffer to aid debugging
	}
	LCtrl.StartIndex = LCtrl.NextIndex = LCB_PRESPACE;
	LCtrl.FIdx = 255;  //Illegal value
	LCtrl.RtnSize = 0;
}



#ifndef DLL_EXPORTS
//The standard block xfer functions must be in your linklist or
//the following two helper functions must be 'ifdef'ed out
U8 MasterBlockDown(DOWN_PTR_U8 SrcPtr, U16 Count, U32 DestPtr);
U8 MasterBlockUp(U32 SrcAddress, UP_PTR_U8 DestPtr, U16 Count);

//These are standard helper functions to extend the basicll.h functions:
//	U8 MasterBlockDown(DOWN_PTR_U8 SrcPtr, U16 Count, U32 DestPtr);
//	U8 MasterBlockUp(U32 SrcAddress, UP_PTR_U8 DestPtr, U16 Count);
// to larger transfer sizes than can be handled in a single link call.
bool LinkBlockDown(U8 *Source, U32 Dest, U32 Count)
{
	long Chunk;
	long Counter;
	U16 MaxLinkPayloadSize = GetMaxLinkSendSize(0);
	
	//Maximum chunk we can send in one call is packet size less the
	//address and count arguments		Dest		  Count
	Chunk = MaxLinkPayloadSize - sizeof(U32) - sizeof(U16) - 1;
	for (Counter = Count; Counter > 0; Counter -= Chunk)
	{
		if(Counter <= Chunk)
		{ //handle possible shorter final packet
			Chunk = Counter;
		}
		if(MasterBlockDown(Source, Dest, Chunk) != LE_NO_ERROR)
		{
			return false;
		}
		Source += Chunk;
		Dest += Chunk;
	}
	return true;
}


bool LinkBlockUp(U32 Src, U8 * Dest, U32 Count)
{
	long Chunk;
	long Counter;
	U16 MaxLinkPayloadSize = GetMaxLinkReturnSize(0);
	
	Chunk = MaxLinkPayloadSize-1;//account for U8 MasterBlockUp() return value
	for (Counter = Count; Counter > 0; Counter -= Chunk)
	{
		if(Counter <= Chunk)
		{ //handle possible shorter final packet
			Chunk = Counter;
		}
		if(!MasterBlockUp(Src, Dest, (USHORT)Chunk))
		{
			return false;
		}
		Src += Chunk;
		Dest += Chunk;
	}
	return true;
}

#endif //of #ifndef DLL_EXPORTS
