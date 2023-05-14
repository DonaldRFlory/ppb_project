//--------------------------------------------------------------------------
//
//        Copyright (C) Bio-Rad Laboratories 2016
//
//  Module:    LINKADAPT.H
//
//  Purpose:   Header for ttilities to adapt possible several link configurations for universal upgrader
//
//
//
//   Created by:     Don Flory
//   Date: 09-19-2016
//
//   Modified:
//
//---------------------------------------------------------------------------

#include "API.h"

#define COMM_TYPE_DEFAULT 0
#define COMM_TYPE_SERIAL 1
#define COMM_TYPE_HID 2

//bool ConnectToUSB(HANDLE & Handle, U16 VID, U16 PID, U8 Index);
bool ConnectToSerial(HANDLE & Handle, U8 CommIndex);
bool CloseAPIHandle(HANDLE Handle);

U32 GetMaxLinkSendSize(LINK_SEL LSel);
U32 GetMaxLinkReturnSize(LINK_SEL LSel);
bool GetMaxLReturnSize(HANDLE Handle, U32 &Size);
bool GetMaxLSendSize(HANDLE Handle, U32 &Size);

bool InitDeviceCommunication(HANDLE Handle);
bool ValidateLinkSel(LINK_SEL &LSel);

LINK_STAT StreamHIDTransact(LINK_CTRL &LCtrl);
LINK_STAT SerialTransact(LINK_CTRL &LCtrl);
int FlushRead(HANDLE hCommPort, UCHAR *Buff, int Size);
int GetHandleIndex(API_DEVICE_HANDLE Handle, U8 &CommType);
