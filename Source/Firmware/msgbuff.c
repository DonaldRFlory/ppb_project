//--------------------------------------------------------------------------
//	MsgBuff.C 
//	Provides a FIFO buffer for logging messages which can be uploaded by host function.
//
//--------------------------------------------------------------------------

#include "target.h"
#include "type.h"
#include "slink.h"
#include <stdio.h>


U8 LineBuff[81];
U8 MessageBuff[MESSAGE_BUFF_LEN];
U16 InIndex, OutIndex, MBCount;

void InitMessageBuff()
{
	int i;
	InIndex = OutIndex = MBCount = 0;
	for(i = 0; i < MESSAGE_BUFF_LEN; ++i)
	{
		MessageBuff[i] = 0;
	}
}

//Add a zero terminated string to message buff but stop if
//if message buff is full.
//Called only in background, never ISR context 
U8 PutMessage(U8 * String)
{
	while(MBCount < MESSAGE_BUFF_LEN)
	{
		if(*String == 0)
		{
			return 1; //success, we added the whole string
		}
		MessageBuff[InIndex++] = *String++;
		InIndex = InIndex >= MESSAGE_BUFF_LEN ? 0 : InIndex;
		++MBCount;
	}
	return (*String != 0); //fail, the buffer filled before we hit end of string 
}


//Link block transfer function 
//Called only in background, never ISR context 
//Always supplies amount requested to avoid link errors.
//Pads with zero bytes if we run out of data.
//Host side prototype: U8 ReadMessageBuff(UP_PTR_U8 Buff, U16 Count)
U8 ReadMessageBuff(U16 Count)
{
	int i;
	U8 RetVal = 1;
	for(i = 0; i < Count; ++i)
	{
		if(MBCount)
		{
			SendU8(MessageBuff[OutIndex++]);
			OutIndex = OutIndex >= MESSAGE_BUFF_LEN ? 0 : OutIndex;
			--MBCount;
		}
		else
		{
			RetVal = 0;
			SendU8(MessageBuff[0]);//pad with zeros
		}
	}
	return RetVal;
}
U16 GetMessageBuffCount()
{
	return MBCount;
}


//U32 ReadMilliseconds(void);
//U8 Message[100];
//int MessageCounter;
//void MsgServe(void)
//{
//	if(++MessageCounter >= 50)
//	{
//	MessageCounter = 0;
//		sprintf((char*)Message, "Another message at %lu\r\n", ReadMilliseconds());
//		PutMessage(Message);
//	}
//}
