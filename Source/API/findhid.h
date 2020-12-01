//--------------------------------------------------------------------------
//
//    Copyright (C) Bio-Rad Laboratories 2004
//
//  Module:  FINDHID.H   
//
//  Purpose:   Structures and prototypes for FindHID function and helpers.
//                   
// $Revision: 1 $
// $Author: Dflory $
// $Date: 10/26/05 3:28p $
//
// $History: findhid.h $
// 
// *****************  Version 1  *****************
// User: Dflory       Date: 10/26/05   Time: 3:28p
// Created in $/Sierra/Sierra/source/include
// 
// *****************  Version 1  *****************
// User: Dflory       Date: 10/17/05   Time: 8:18p
// Created in $/sierra/source/include
// 
// *****************  Version 2  *****************
// User: Donflory     Date: 8/14/04    Time: 3:22p
// Updated in $/imagerfx/SOURCE/INCLUDE
// Added SetHIDBuffers() function prototype.
// 
// *****************  Version 1  *****************
// User: Donflory     Date: 7/10/04    Time: 12:11p
// Created in $/imagerfx/SOURCE/INCLUDE
// 
//              
//---------------------------------------------------------------------------
#ifndef FINDHID_H
#define FINDHID_H
#include "mlink.h"
extern "C"
{
	#include <setupapi.h>
	#include <hidsdi.h>
}
#define MAX_FINDHID_DEVS 10
typedef struct
{
	USHORT VID;
	USHORT PID;
	USHORT Usage;
	USHORT FoundPIDs[MAX_FINDHID_DEVS];
	USHORT Count;
	bool Enumerate;
	USHORT Nth;
} FHID_CTRL;

bool FindHID( FHID_CTRL &Ctrl, HIDP_CAPS &HIDCaps, HANDLE &Handle); 
void InitFindHIDCtrl( FHID_CTRL &Ctrl ); 
int SetHIDBuffers(HANDLE Handle, int NumBuffers);
bool SendUSBReport(LINK_SEL &LSel, UCHAR *ReportBuff, int TimeoutMsec, UCHAR FlushResponse = 1);
bool GetUSBReport(LINK_SEL &LSel, UCHAR *ReportBuff, int TimeoutMsec);
bool QuickGetResponse(LINK_SEL &LSel, UCHAR *RtnBuff);
#endif
