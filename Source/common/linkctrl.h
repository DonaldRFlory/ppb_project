//---------------------------------------------------------------------------
//
//                 Copyright (C) Bio-Rad Laboratories 2018
//
// Include file: MLINK.H
//
// Created:   Don Flory 2018
//
//---------------------------------------------------------------------------
#ifndef LINKCTRL_H
#define LINKCTRL_H

#include "type.h"

typedef U8 L_STAT;

typedef struct
{
	U8		Stat;
	U8		FIdx;
	U8		Channel;
	U8		CommType;
} LINK_STAT;

#define HT_COMM 0   //Handle is to a comm/tty port
#define HT_FTDI 1   //Handle is FTDI D2XX handle

typedef	void* LINK_HANDLE;
//After link transact, CommType is set to indicate whether
//transaction was Local or which instrument channel it was
#define CT_LOCAL 0
#define CT_INST_1	1
#define CT_INST_2	2
#define CT_INST_3	3
#define CT_INST_4	4
#define CT_INST_UNKNOWN 255

#define INVALID_COMM_TYPE 255
#define  LINK_SERIAL_COMM 		0
#define  LINK_HID_STAND_COMM	1
#define  LINK_HID_STREAM_COMM	2

typedef struct
{
	LINK_HANDLE		LHand;
	U8			HandleType;//HT_COMM or HT_FTDI
	U32			ReadSize;	//based on CommType
	U32			WriteSize;	//based on CommType
	U8			ChannelIndex; //used only in multichannel link implementations.
	U8			SubID; 	//Intended for satellite, may never be used.
	U8			CommType;
	U8			LocalFlag;//If non-zero, we ar in local mode
}LINK_SEL;


#define LCB_BUFFSIZE 4096 //Biggest buffer contemplated as of 2016
#define LCB_PRESPACE 20
#define LCB_POSTSPACE 10
#define LCB_TOTSIZE (LCB_PRESPACE + LCB_BUFFSIZE + LCB_POSTSPACE)
typedef struct
{
	LINK_SEL LSel;
	U8 FIdx;
	U32 RtnSize;
	U8 * Buffer;
	U8 LCBuff[LCB_TOTSIZE];
	U32 BuffSize;
	U32 StartIndex;
	U32 NextIndex;
}LINK_CTRL;

#endif // of ifndef LINKCTRL_H
//////////
