// API.cpp : Defines the exported functions for the DLL application.
//
#include "api.h"
#include "apilinkadapt.h"
//#include "explink.h"
U8 BoardAddress;
DFLOGFPTR LogCB = NULL;
LINK_STAT_CB_P   LinkStatCB = NULL;


#if 0
    //One to one with errors in LINKERR.H at time of this writing, just added leading STAT_
//to produce the corresponding API_STAT value
const API_STAT  LinkToAPIStat[LE_NUM_ERRORS] =
{
  STAT_OK, //LE_NO_ERROR
  STAT_LE_BAD_PARAM,
  STAT_LE_BAD_FUNCODE,
  STAT_LE_BAD_COMMAND,
  STAT_LE_BAD_FORMAT,
  STAT_LE_BAD_TABLE,
  STAT_LE_BAD_LT_PACKET_LEN,
  STAT_LE_BAD_LT_CALL,
  STAT_LE_BAD_SEND,
  STAT_LE_RESPONSE_TIMEOUT,
  STAT_LE_BAD_RESP,
  STAT_LE_BAD_RESP_LEN,
  STAT_LE_BAD_RESP_CS,
  STAT_LE_BAD_STATUS,
  STAT_LE_SLAVE_STATUS,
  STAT_LE_COMM_FAIL,
  STAT_LE_BAD_CMN_CHAN,
  STAT_LE_BAD_CHAN,
  STAT_LE_SHORT_PACKET,
  STAT_LE_BAD_CHECK,
  STAT_LE_BAD_RETURN,
  STAT_LE_BAD_PARAM_FLAG,
  STAT_LE_BAD_BLOCK,
  STAT_LE_BLOCK_SIZE,
  STAT_LE_BLOCK_DOWN,
  STAT_LE_BLOCK_UP,
  STAT_LE_FUN_RETURN,
  STAT_LE_UNEXP_RESP,
  STAT_LE_SEND_BYTE_TO,
  STAT_LE_BABBLE,
  STAT_LE_OVERRUN,
  STAT_LE_UNDERRUN,
  STAT_BAD_API_HANDLE,
  STAT_LE_UNKNOWN,
};
#endif
#if 0
    API_STAT LinkStatToAPIStat(LINK_STAT LStat)
{
	U32 Stat;
	int Index = (int) LStat.Stat;
	if(Index > LE_UNKNOWN)
	{
		Index = LE_UNKNOWN;
	}
	Stat = LStat.FIdx;
	Stat <<= 16;
	Stat |= LinkToAPIStat[Index] & 0XFFFF;
	return Stat;
}
#endif

void SetBoardAddressAdapt(U8 Address);
PPB_API void CALL_CONV SetBoardAddress(U8 Address)
{
    SetBoardAddressAdapt(Address);
}

//---------------------------------------
//  FUNCTION:     RegisterLoggingCallback
//
//  DESCRIPTION:  Supplies pointer to function to call for logging from API to main program
//
//---------------------------------------
PPB_API void CALL_CONV  RegisterLoggingCallback(DFLOGFPTR CallbackPointer)
{
	LogCB = CallbackPointer;
} //endof RegisterLoggingCallback

//---------------------------------------
//  FUNCTION:     RegisterLinkErrorCallback
//
//  DESCRIPTION:  Supplies pointer to function to call for logging from API to main program
//
//---------------------------------------

PPB_API void CALL_CONV  RegisterLinkStatusCallback(LINK_STAT_CB_P CallbackPointer)
{
    LinkStatCB = CallbackPointer;
} //endof RegisterLinkStatusCallback


PPB_API bool CALL_CONV Disconnect(API_DEVICE_HANDLE Handle)
{
	if(CloseAPIHandle(Handle))//First checks if it is a valid connection handle
	{
		return true;
	}
	return false;
}


PPB_API bool CALL_CONV SerialConnect(API_DEVICE_HANDLE &Handle, U8 CommIndex)
{
	//check for room in handle table, if so try connect. If successful
	//enter connection in table and return true, handle returned in Handle.
	if(ConnectToSerial(Handle, CommIndex))
	{
		return true;
	}
	return false;
}


PPB_API U32 CALL_CONV GetMaxLinkReturnSize(API_DEVICE_HANDLE Handle)
{
	U32 Size;
	if( GetMaxLReturnSize(Handle, Size) )
	{
		return Size;
	}
	return 0;
}

PPB_API U32 CALL_CONV GetLinkMaxSendSize(API_DEVICE_HANDLE Handle)
{
	U32 Size;
	if( GetMaxLSendSize(Handle, Size) )
	{
		return Size;
	}
	return 0;
}

U32  PunFloatToLong(float Float)
{
	U32 *LP;
	float *FP;
	FP =  &Float;
	LP = (U32*)FP;
	return *LP;
}

float PunLongToFloat(U32 Long)
{
	U32 *LP;
	float *FP;
	LP = &Long;
	FP = (float *)LP;
	return *FP;
}


#if 0
DECL_SPEC API_STAT CALL_CONV GetVelAcc(API_DEVICE_HANDLE Handle, U8 Channel, float &Velocity, float &Acceleration)
{
	U32 Array[2];
	LINK_STAT LStat;
	LINK_SEL LSel;

	LSel.ChannelIndex = 0;
	LSel.CommType = INVALID_COMM_TYPE;
	LSel.LHand = Handle;
	LGetVelAcc(LSel, &LStat, Channel, Array, 2);
	if(LStat.Stat == LE_NO_ERROR)
	{
		Velocity = PunLongToFloat(Array[0]);
		Acceleration = PunLongToFloat(Array[1]);
	}
	return LinkStatToAPIStat(LStat);
}

DECL_SPEC API_STAT CALL_CONV SetVelAcc(API_DEVICE_HANDLE Handle, U8 Channel, float Velocity, float Acceleration)
{
	LINK_STAT LStat;
	LINK_SEL LSel;
	U32 LVel, LAcc;

	LVel = PunFloatToLong(Velocity);
	LAcc = PunFloatToLong(Acceleration);

	LSel.ChannelIndex = 0;
	LSel.CommType = INVALID_COMM_TYPE;
	LSel.LHand = Handle;
	LSetVelAcc(LSel, &LStat, Channel, LVel, LAcc);
	return LinkStatToAPIStat(LStat);
}
#endif
