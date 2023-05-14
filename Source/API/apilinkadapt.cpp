//--------------------------------------------------------------------------
//
//  Module:    APILINKADAPT.CPP
//
//  Purpose:   Utilities to adapt possible several link configurations for universal upgrader
//				It uses LinkBlockDown(), SetSlaveParameter() and GetSlaveParameter.
//
//
//   Created by:     Don Flory
//   Date: 09-19-2016
//
//   Modified:
//
//---------------------------------------------------------------------------
#include <Windows.h>
#include "mlink.h"
#include <stdio.h>
#include "link.h"
#include "Api.h"
#include "apilinkadapt.h"
#include "verblock.h"
//#include "explink.h"
#include <atlstr.h>

#define STD_BAUD_RATE 115200
#define DEF_MAX_RETURN_SIZE 200
#define DEF_MAX_SEND_SIZE 200
extern int LinkIsLittleEndian;
//We are going to support up to ten connections. We will keep track of the
//type of connection, HID (streaming) or serial
//We will default max send and max return size to 32. User can
//query device and update this to the actual max supported by device if desired
struct _HANDLE_TABLE
{
	HANDLE Handle;
	U32 MaxSendSize;
	U32 MaxReturnSize;
	bool IsHID;
} HandleTable[MAX_HANDLES] = { {NULL, 0, 0, false}, {NULL, 0, 0, false},
							   {NULL, 0, 0, false}, {NULL, 0, 0, false},
							   {NULL, 0, 0, false}, {NULL, 0, 0, false},
							   {NULL, 0, 0, false}, {NULL, 0, 0, false},
							   {NULL, 0, 0, false}, {NULL, 0, 0, false},
							 };

//Returns index 0-9 if Handle is open, otherwise returns -1
//However if Handle is NULL, returns of 0-9 to indicate that
//the returned index in the HandleTable is empty. THis seems
//screwy as it also sets up CommType from this empty entry???
//Sets up CommType based on info in HandleTable.
int GetHandleIndex(API_DEVICE_HANDLE Handle, U8 &CommType)
{
	for(int i = 0; i < MAX_HANDLES; ++i)
	{
		if(Handle == HandleTable[i].Handle)
		{
			CommType = HandleTable[i].IsHID ? COMM_TYPE_HID : COMM_TYPE_SERIAL;
			return i;
		}
	}
	return -1;
}

//If successful, queries device and gets actual send and return size limits from device,
//storing the info along with the handle in the local HandleTable.
//Not required, but allows for most optimal block transfers.
//This is called once after device comm channel is opened.
bool InitDeviceCommunication(HANDLE Handle)
{
	int Size;
	LINK_SEL LSel;
	U8 CommType;
	int Index = 0;
	Index =  GetHandleIndex(Handle, CommType);
	if((Index < 0) || (Index >= MAX_HANDLES))
	{
		return false;
	}

	LSel.LHand = Handle;		//ignoring Channel, not used in this implementation

	//Size = (U16)GetSlaveParameter(LSel, &LStat, SSP_MAX_SEND_SIZE, 0);
	//if((Size == 0) || (LStat.Stat != LE_NO_ERROR))
	//{
	//	return false;
	//}
	Size = 61; //max ardino send is 63 bytes, and we have status byte and length byte

	HandleTable[Index].MaxSendSize = Size;

	//Size = (U16)GetSlaveParameter(LSel, &LStat, SSP_MAX_RETURN_SIZE, 0);
	//if((Size == 0) || (LStat.Stat != LE_NO_ERROR))
	//{
	//	return false;
	//}
	HandleTable[Index].MaxReturnSize = Size;
	return true;
}

bool CloseAPIHandle(HANDLE Handle)
{
	U8 CommType;
	int Index = 0;
	Index =  GetHandleIndex(Handle, CommType);
	if(Index >= 0)
	{
		CloseHandle(Handle);
		HandleTable[Index].Handle = NULL;
		return true;
	}
	return false;
}


static int LinkErrorCount = 0;

//We are passed a raw link packet in LCtrl
L_STAT LinkTransact(LINK_CTRL &LCtrl)
{
	LINK_STAT LStat;
	LStat = SerialTransact(LCtrl);
	return LStat.Stat;
}



//This implementation just sets up LHand in stubs.
//We fill in the handle index into our open handle table which
//contains info on max transfer sizes as well as setting
//read and write sizes for HID device.
bool ValidateLinkSel(LINK_SEL &LSel)
{
	int Index = 0;
	Index =  GetHandleIndex(LSel.LHand, LSel.CommType);

	if((Index < 0) || (Index >= MAX_HANDLES) || (LSel.LHand == NULL))
	{
		return false;
	}
	if(LSel.CommType == COMM_TYPE_HID)
	{
		LSel.ReadSize = 65;
		LSel.WriteSize = 65;
	}
    LSel.ChannelIndex = Index;
	return true;
}

//Used by API call. Name conflicted with exported name
bool GetMaxLReturnSize(HANDLE Handle, U32 &Size)
{
	int HandleIndex;
	U8 CommType = COMM_TYPE_DEFAULT;

	HandleIndex = GetHandleIndex(Handle, CommType);
	if((HandleIndex < 0) || (HandleIndex >= MAX_HANDLES))
	{
		return false; //Handle not open
	}

	Size = (U16)HandleTable[HandleIndex].MaxReturnSize;
	return true;
}

//Used by API call. Name conflicted with exported name
bool GetMaxLSendSize(HANDLE Handle, U32 &Size)
{
	int HandleIndex;
	U8 CommType = COMM_TYPE_DEFAULT;

	HandleIndex = GetHandleIndex(Handle, CommType);
	if((HandleIndex < 0) || (HandleIndex >= MAX_HANDLES))
	{
		return false; //Handle not open
	}

	Size = HandleTable[HandleIndex].MaxSendSize;
	return true;
}

//Maximum payload we can return to link functions. It is the total of any return
//value bytes from the link function plus any BLOCK_UP bytes.
//It depend on the particular link we are calling. We may have several so
//LSel allows us to determine which one the call is for.
U32 GetMaxLinkReturnSize(LINK_SEL LSel)
{
	return HandleTable[LSel.ChannelIndex].MaxReturnSize;
}


//Maximum payload we can send in a single link call. It includes length of FIdx, any parameters and
//block xfer counts, plus the block transfer bytes themselves if any.
//Any wrapper stuff transport adds is additional and must fit into the packet we ultimately send.
U32 GetMaxLinkSendSize(LINK_SEL LSel)
{
	return HandleTable[LSel.ChannelIndex].MaxSendSize;
}


//Following are default values for read and write timeout for
//serial communication from host:
//DFDEBUG changed interval from 2 to 4 for Arduino
#define READ_INTERVAL_TO	4
#define READ_TOTAL_TO_MULT	1
#define READ_TOTAL_TO_CONST	 60
#define WRITE_TOTAL_TO_MULT  0
#define WRITE_TOTAL_TO_CONST 40
COMMTIMEOUTS CommTimeOuts =
{
	READ_INTERVAL_TO,		//ReadIntervalTimeout
	READ_TOTAL_TO_MULT,		//ReadTotalTimeoutMultiplier
	READ_TOTAL_TO_CONST,	//ReadTotalTimeoutConstant
	WRITE_TOTAL_TO_MULT,	//WriteTotalTimeoutMultiplier
	WRITE_TOTAL_TO_CONST,	//WriteTotalTimeoutConstant
};


void SetStdTimeouts(HANDLE hCommPort)
{
	CommTimeOuts.ReadIntervalTimeout = READ_INTERVAL_TO;
	CommTimeOuts.ReadTotalTimeoutMultiplier = READ_TOTAL_TO_MULT;
	CommTimeOuts.ReadTotalTimeoutConstant = READ_TOTAL_TO_CONST;
	SetCommTimeouts(hCommPort, &CommTimeOuts);
}

void SetFlushTimeouts(HANDLE hCommPort)
{
	CommTimeOuts.ReadIntervalTimeout = 1;
	CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
	CommTimeOuts.ReadTotalTimeoutConstant = 2;
	SetCommTimeouts(hCommPort, &CommTimeOuts);
}

int FlushRead(HANDLE hCommPort, UCHAR *Buff, int Size)
{
	ULONG BytesRead;
	SetFlushTimeouts(hCommPort);
	ReadFile(hCommPort, Buff, Size, &BytesRead, NULL);
	SetStdTimeouts(hCommPort);
	return (int) BytesRead;
}

HANDLE OpenCommPort( long BaudRate, int PortIndex )
{
	DCB CommDCB;
	COMMPROP CommProp;
	HANDLE hCommPort;
	TCHAR DevControl[80];
	int ReturnVal;

	FillMemory(&CommDCB, sizeof(CommDCB), 0);
	CommDCB.DCBlength = sizeof(CommDCB);
	FillMemory(&CommProp, sizeof(CommProp), 0);

	wsprintf( DevControl, TEXT("\\\\.\\COM%u"), PortIndex+1 );
	hCommPort = CreateFile( DevControl, GENERIC_READ | GENERIC_WRITE,
					0,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
					NULL );


	if(hCommPort == INVALID_HANDLE_VALUE)
	{
		return hCommPort;
	}

	wsprintf( DevControl, TEXT("COM%u: baud=%lu parity=n data=8 stop=1"), PortIndex+1, BaudRate);
	ReturnVal = BuildCommDCB( DevControl, &CommDCB );

	if(ReturnVal == 0)
	{
		return INVALID_HANDLE_VALUE;
	}

	SetStdTimeouts(hCommPort);

	SetCommState(hCommPort, &CommDCB);
	GetCommProperties(hCommPort, &CommProp);

	SetupComm(hCommPort, 512, 512);
	SetCommMask( hCommPort, EV_CTS|EV_DSR);
	return hCommPort;
}

bool ConnectToSerial(HANDLE & Handle, U8 CommIndex)
{
	U8 CommType;
	int HandleIndex;
	HandleIndex = GetHandleIndex(NULL, CommType);
	if((HandleIndex < 0) || (HandleIndex >= MAX_HANDLES))
	{
		return false; //no room for another open handle
	}
	Handle = OpenCommPort(STD_BAUD_RATE, CommIndex);
	if (Handle == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	HandleTable[HandleIndex].Handle = Handle;
	HandleTable[HandleIndex].MaxReturnSize = DEF_MAX_RETURN_SIZE;
	HandleTable[HandleIndex].MaxSendSize = DEF_MAX_SEND_SIZE;
	HandleTable[HandleIndex].IsHID = false;
	if(InitDeviceCommunication(Handle)) //query device for actual size limits
	{
		return true;
	}
	CloseAPIHandle(Handle);
	return false;
}
