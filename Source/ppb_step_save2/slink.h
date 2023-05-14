//--------------------------------------------------------------------------
//                Copyright (C) Bio-Rad Laboratories 2008
//
//  MODULE :     slink.h
//  Purpose:
//
//
//
//---------------------------------------------------------------------------
#ifndef SLINK_H
#define SLINK_H
#include "type.h"
#define NO_CODE 255
#define NO_CHAR -1

//#ifdef __cplusplus
//extern "C" {
//#endif

//Link pointer types:
//typedef unsigned char * UP_PTR_U8;
//typedef unsigned short * UP_PTR_U16;
//typedef unsigned int * UP_PTR_U32;
//typedef unsigned char * DOWN_PTR_U8;
//typedef unsigned short * DOWN_PTR_U16;
//typedef unsigned int * DOWN_PTR_U32;

struct LinkDef
{
	void (*FunP)(void);
	unsigned char  FunSize, Arg1Size, Arg2Size, Arg3Size, Arg4Size;
};

//TYPEDEF for LinkDef structure above
typedef struct
{
	void (*FunP)(void);
	unsigned char  FunSize, Arg1Size, Arg2Size, Arg3Size, Arg4Size;
} FUN_DEF;

typedef struct
{
	U8 * CmdBuff;  //These two buffer pointers may point to the same buffer
	U8 * RspBuff;  //or to two distinct buffers. That is up to the transport layer.
	U32 ByteCount;
	U32 IndexLimit;
	U32 Index;
	U32 MaxReturnSize;
	U8 ExMode;
	U8 FunSize;
	U8 FIdx;
	U8 ReturnStatus;
	U8 BoardAddress;
} LINK_CONTROL;

//These may control different behavior of RcvChar() and SendChar()
typedef enum
{
	EX_IDLE = 0,
	EX_START,  	//Starting to execute, getting function index
	EX_PARM,  	//getting parameters
	EX_CALL,	//function is executing	(can get BlockDown data here)
	EX_UP,		//function has sent back BlockUp data (so no more data may be fetched)
	EX_RETURN,	//sending any return value
} EXEC_PHASE;


int GetNumLinks(void);
void SetLinkIndex(U8 Index);
int GetLinkCommandPacket(U8 **CmdPtrAddress, U8 **RspPtrAddress, U32 *MaxReturnSizeAddress);
void PostLinkResult(U32 ByteCount, U8 ReturnStatus, U8 FIdx);//they add wrapper
U32 MaxLinkSend(void);
U32 MaxLinkReturn(void);
void InitSlaveLink(void);
U8 SlaveLinkProc(void);
//Called just before executing slave target function
U8 SetExPhase(U8 Phase, U32 Parm);
//Used by SlaveLinkProc() to get command data and send response data
U8 SendChar(U8 C);
U16 RcvChar(void);
bool SendBlock(U8 * Buffer, U32 Count);
bool RcvBlock(U8 * Buffer, U32 Count);

//Used by slave.cpp to get function definition for current function on link currently being serviced
U32 GetCurFDef(struct LinkDef **FDefP);
U32 GetFDef(U8 FIdx, struct LinkDef *FDefP);
void PostLinkCallStatus(U8 ErrCode);

//Called from main background loop to service link(s)
void LinkServe(void);

void PostLogicError(U16 ErrorIdentifier);
#define BAD_EXMODE_LOGIC_ERROR 1
#define RCV_CHAR_LOGIC_ERROR	2
#define SEND_CHAR_LOGIC_ERROR	3

//Used by Block Up and Block Down functions to get/return array values
int RcvFloat(float *Value);
int RcvU32(U32 *Value);
int RcvU16(U16 *Value);
int RcvU8(U8 *Value);
int SendFloat(float Value);
int SendU32(U32 Value);
int SendU16(U16 Value);
int SendU8(U8 Value);

//#ifdef __cplusplus
//}
//#endif

#endif	//of ifndef SLINK_H











