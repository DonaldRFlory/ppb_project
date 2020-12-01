//--------------------------------------------------------------------------
//                  Copyright (C)
//
//  MODULE :        LINKTRAN.C
//  Purpose:        Master link communication LinkTransact and
//					related functions. Adapter between packet
//					forming logic in MASTER.C and communication
//					channel functions (physical layer).
//--------------------------------------------------------------------------
#include "stdafx.h"
#include <Windows.h>
#include "mlink.h"
#include "apilinkadapt.h"
//#include "masteror.h"
#include "findhid.h"
//#include "hiddefs.h"
#include "hidadapt.h"

#include <limits.h>
#define INIT_CRC  		0  /* CCITT: 0xFFFF */
#define CRCPOLY  		0xA001  // ANSI CRC-16  CCITT: 0x8408

static U8 GetSerialResponse(LINK_CTRL & LCtrl);
static U32 SlowTransactMillisec = 0;

static LINK_STAT LinkSend(LINK_CTRL &LCtrl);
static LINK_STAT LinkGetResponse(LINK_CTRL &LCtrl);

//------------------------------------------------------------------------------
//  FUNCTION:     MakeCRCTable
//
//  DESCRIPTION:  This Function is used to make CRC Table
//
//  PARAMETERS: None
//
//  RETURNS:    None
//
//  Notes:
//------------------------------------------------------------------------------
static U16 CRCTable[UCHAR_MAX + 1];    //Holds Value of CRCTable
static void MakeCRCTable(void)
{
  	static short CRCTabInited = FALSE;
	unsigned short i, j, r;

  	if(CRCTabInited)
    	return;
  	CRCTabInited = TRUE;

  	for (i = 0; i <= UCHAR_MAX; i++)
  	{
		r = i;
		for (j = 0; j < CHAR_BIT; j++)
		{
      		if (r & 1)
      		{
        		r = (r >> 1) ^ CRCPOLY;
      		}
      		else
      		{
        		r >>= 1;
      		}
    	}
    	CRCTable[i] = r;
	}
}



//Compute complete CRC on a buffer in one call.
U16 ComputeCRC(U8 * Buff, U32 CharCount)
{
	U32 u;
	U16 CRC;
	MakeCRCTable();
	CRC = INIT_CRC;
	for(u=0; u < CharCount; ++u)
	{
		CRC = CRCTable[(CRC ^ (Buff[u] & 0xFF)) & 0xFF] ^ (CRC >> CHAR_BIT);
	}
	return CRC;
}


void SetSlowTransact(U32 Milliseconds)
{
	SlowTransactMillisec = Milliseconds;
}

//true return means that control byte indicates successful function
//call at slave end
static bool GoodSlaveStatus(UCHAR CtrlByte)
{
	//TODO: flesh this out according to final slave design
	return true;
}


bool FlushResponses(LINK_SEL LSel)
{
	unsigned char Buffer[0x41];
	unsigned char Flag = 0XFF;
	int i = 0, j = 0;
	while(QuickGetResponse(LSel, Buffer))
	{
		Flag = Buffer[1];
		++j;
		if(j > 20)
		{  //should not be possible, but..
			return false;
		}
	}
	if(Flag == 0)
	{ //we are good, last packet was a null packet
		return true;
	}

	for(i = 0; i < 66; ++i)
	{
		if(GetUSBReport(LSel, Buffer, 20))
		{
			if(Buffer[1] == 0)
			{
				return true;
			}
		}
		else
		{  //we timed out on report read. Should not happen
			return false;
		}
	}
	return false;//read 66 reports and none were null
}


typedef enum
{
	RTN_0 = 0,
	RTN_1,
	RTN_2,
	RTN_3,
	RTN_4,
	RTN_5,
	RTN_6,
	RTN_7,
	RTN_8,
	RTN_9
} RETURN_ENUM;

int GetHIDStreamResponse(LINK_SEL LSel, unsigned char *Buffer, int Length)
{
	bool End = false, Fail = false;
	unsigned char Flag;
	int ByteIndex = 0, ByteCount = 0;
	unsigned char PacketCount, PacketIndex = 0;
	unsigned char PackBuffer[65];
	int Timeouts = 0;
	unsigned char Count, i;

	//So we should pull out any null packets. We can allow up to 100 null packets as they come in
	//at one msec intervals.
	if(Length > 4095)
	{
		Length = 4095;
	}
	for(i = 0; i < 100; ++i)
	{
		if(GetUSBReport(LSel, PackBuffer, 10))
		{
			if((Flag = PackBuffer[1]) != 0)
			{
				break;
			}
		}
		else
		{  //we timed out on report read. Should not happen
			++Timeouts;
			if(Timeouts > 10)
			{
				return RTN_1;
			}
		}
	}
	if(i >= 100)
	{
		return RTN_2;
	}
	//When we get here we have the first non-null packet
	switch(Flag & HID_FLAG_MASK)
	{
		case START_FLAG:
			PacketCount = Flag & (~HID_FLAG_MASK);
			Count = 63;
			break;

		case START_END_FLAG:
			Count = Flag & (~HID_FLAG_MASK);
			End = true;
			break;

		default:
			return RTN_3;
	}
	while(1)
	{
		if((Count + ByteCount) > Length)
		{
			Fail = true;
			Count = Length - ByteCount;
		}
		for(i = 0; i < Count; ++i)
		{
			Buffer[ByteCount++] = PackBuffer[i+2];
		}
		if(Fail)
		{
			return RTN_4;
		}
		if(End)
		{
			if(ByteCount == Length)
			{
				return RTN_0;
			}
			else
			{
				return RTN_5;
			}
		}
		++PacketIndex;
		if((PacketIndex) >= PacketCount)
		{
			return RTN_6;
		}
		if(GetUSBReport(LSel, PackBuffer, 20))
		{
			Flag = PackBuffer[1];
			switch(Flag & HID_FLAG_MASK)
			{
				case MIDDLE_FLAG:
					if((Flag & (~HID_FLAG_MASK)) != PacketIndex)
					{
						return RTN_7; //probably missed a packet
					}
					Count = 63;
					break;

				case END_FLAG:
						Count = Flag & (~HID_FLAG_MASK);
					End = true;
					break;

				default:
					return RTN_8;
			}
		}
		else
		{  //we timed out on report read. Should not happen
			return RTN_9;
		}
	}
}

//We will fail if we cannot send it within 100 msec.
bool SendHIDStreamCommand(LINK_SEL LSel, unsigned char *Buffer, int Length)
{
	unsigned char Timeouts = 0;
	unsigned char Count, i;
	int ByteIndex = 0;
	unsigned char PackBuff[65];
	unsigned char PacketCount, PacketIndex = 0;

	if(Length > 4095)
	{
		Length = 4095;
	}
	PackBuff[0] = 0;//the report ID which is always zero
	PacketCount = Length/63;	//max possible is 65 packets
	if(Length % 63)
	{
		++PacketCount;
	}
	while(PacketIndex < PacketCount)
	{
		if(PacketIndex == 0)
		{
			if(PacketCount >= 2)
			{
				PackBuff[1]	= (PacketCount & (~HID_FLAG_MASK)) | START_FLAG;
				Count = 63;
			}
			else
			{
				Count = (unsigned char)Length;
				PackBuff[1]	= ((Count) & (~HID_FLAG_MASK)) | START_END_FLAG;
			}
		}
		else if(PacketIndex >= (PacketCount - 1))
		{  //this is the final packet of two or more
			Count = (unsigned char)(Length - ByteIndex);
			PackBuff[1]	= (Count & (~HID_FLAG_MASK)) | END_FLAG;
		}
		else
		{  //This is a middle packet of two or more
			Count = 63;
			PackBuff[1] = (PacketIndex & (~HID_FLAG_MASK)) | MIDDLE_FLAG;
		}
		Count += 2; //cause our index starts at 2
		for(i = 2; i < Count; ++i)
		{  //copy data into current packet
			PackBuff[i] = Buffer[ByteIndex++];
		}
		while(!SendUSBReport(LSel, PackBuff, 20, false))
		{
			++Timeouts;
			if(Timeouts >= 6)
			{
				return false;
			}
		}
		++PacketIndex;
	}
	return true;
}

LINK_STAT StreamHIDTransact(LINK_CTRL &LCtrl)
{
	U8 FIdx;
	LINK_STAT LStat;
	bool SendResult;
	int ResponseResult;
	U32 PayloadLength = LCtrl.NextIndex - LCtrl.StartIndex;

	LStat.FIdx = LCtrl.FIdx;
	if((PayloadLength  < 1) || (PayloadLength > GetMaxLinkSendSize(LCtrl.LSel)))
	{
		LStat.Stat = LE_BAD_LT_CALL_D;
		return LStat;
	}

	FlushResponses(LCtrl.LSel);
	SendResult = SendHIDStreamCommand(LCtrl.LSel, &LCtrl.Buffer[LCtrl.StartIndex], PayloadLength);
	if(!SendResult)
	{
		LStat.Stat = LE_BAD_SEND_D;
		return LStat;
	}
	//RtnSize does not account for response status byte and function index at beginning of packet
	ResponseResult = GetHIDStreamResponse(LCtrl.LSel, LCtrl.Buffer, LCtrl.RtnSize + 2);
	if(ResponseResult != RTN_0)
	{
		LStat.Stat = LE_RESP_TIMEOUT_D;
		return LStat;
	}
	LCtrl.NextIndex = LCtrl.RtnSize + 2;
	LCtrl.StartIndex = 2;
	LStat.Stat = LCtrl.Buffer[0];
	FIdx = LCtrl.Buffer[1];
	if(LStat.Stat == LE_NO_ERROR)
	{
		if(FIdx != LStat.FIdx)
		{
			LStat.Stat = LE_BAD_CHECK_D;
		}
	}
	return LStat;
 }


static U8 FlushBuff[2000];
LINK_STAT SerialTransact(LINK_CTRL &LCtrl)
{
	ULONG BytesWritten;
	U16 CRC;
	LINK_STAT Stat;

	U16 PayloadLength = (U16)(LCtrl.NextIndex - LCtrl.StartIndex);

	Stat.FIdx = LCtrl.FIdx;
	if((PayloadLength  < 1) || (PayloadLength > GetMaxLinkSendSize(LCtrl.LSel)))
	{
		Stat.Stat = LE_BAD_LT_CALL_D;
		return Stat;
	}

	if(PayloadLength > 127)
	{  //two length bytes
		LCtrl.Buffer[--LCtrl.StartIndex] = (U8)PayloadLength;
		LCtrl.Buffer[--LCtrl.StartIndex] = (U8) (PayloadLength >> 8 | 0X80);
		PayloadLength += 2;
	}
	else
	{
		LCtrl.Buffer[--LCtrl.StartIndex] = (U8)PayloadLength++;
	}
	//CRC = ComputeCRC(&LCtrl.Buffer[LCtrl.StartIndex], PayloadLength);
	//LCtrl.Buffer[LCtrl.NextIndex++] = (U8)(CRC >> 8);
	//LCtrl.Buffer[LCtrl.NextIndex++] = (U8)CRC;
	//PayloadLength += 2;
	FlushRead(LCtrl.LSel.LHand, FlushBuff, 2000);

	WriteFile(LCtrl.LSel.LHand, &LCtrl.Buffer[LCtrl.StartIndex], PayloadLength, &BytesWritten, NULL);
	if(BytesWritten  != PayloadLength)
	{
		Stat.Stat = LE_BAD_SEND_D;
		return Stat;
	}

	Sleep(10);

	Stat.Stat = GetSerialResponse(LCtrl);
	return Stat;
}


//So we are going to get response into LCtrl buffer at beginning
//We will then check length byte or bytes and set StartIndex and NextIndex
//to bracket the raw packet. We will also validate the returned status and
//the returned FIdx as well as length byte(s) and CRC.
static U8 GetSerialResponse(LINK_CTRL & LCtrl)
{
	U8 Status, FIdx;
	U16 LenVal, ReadLength, PayloadLength;
	ULONG  BytesRead;

	PayloadLength = (U16)LCtrl.RtnSize + 1;//status byte, raw return value
	ReadLength= PayloadLength + 1; //len-byte

	ReadFile(LCtrl.LSel.LHand, LCtrl.Buffer, ReadLength, &BytesRead, NULL);
	if(BytesRead != ReadLength)
	{
		return LE_SHORT_PACKET_RD;
	}
	LCtrl.StartIndex = 0; //we are putting things right at beginning of LCtrl Buffer
	LCtrl.NextIndex = ReadLength;
	LenVal = LCtrl.Buffer[0];
	if(PayloadLength != LenVal)
	{
		return LE_BAD_RETURN;
	}
	LCtrl.StartIndex = 1;//they are not expecting length byte
	Status = LCtrl.Buffer[LCtrl.StartIndex++];//nor status byte
	//FIdx = LCtrl.Buffer[LCtrl.StartIndex++];
	//if(FIdx != LCtrl.FIdx)
	//{
	//	return LE_BAD_CHECK_D;
	//}
	return Status;
}

