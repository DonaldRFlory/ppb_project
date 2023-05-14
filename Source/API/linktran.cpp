//--------------------------------------------------------------------------
//                  Copyright (C)
//
//  MODULE :        LINKTRAN.C
//  Purpose:        Master link communication LinkTransact and
//					related functions. Adapter between packet
//					forming logic in MASTER.C and communication
//					channel functions (physical layer).
//--------------------------------------------------------------------------
#include <Windows.h>
#include "mlink.h"
#include "apilinkadapt.h"
#include "mastadapt.h"

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

static U8 FlushBuff[2000];
//This version is for communication with Arduino. It has 63 byte receive buffer so
//packets will be short. Not using CRC in this implementation.
//On entry, LCtrl contains the raw packet, starting with FIdx.
LINK_STAT SerialTransact(LINK_CTRL &LCtrl)
{
	ULONG BytesWritten;
	//U16 CRC;
	LINK_STAT Stat;

	U16 PayloadLength = (U16)(LCtrl.NextIndex - LCtrl.StartIndex);

	Stat.FIdx = LCtrl.FIdx;
	if(LCtrl.FIdx > MAX_FIDX)
	{
		Stat.Stat = LE_BAD_LT_CALL_D;
		return Stat;
	}

	//Add the board address to FIdx in the raw packet
	LCtrl.Buffer[LCtrl.StartIndex] |= (GetBoardAddress() << BOARD_ADDRESS_LEFT_SHIFT);

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
	U8 Status;
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
