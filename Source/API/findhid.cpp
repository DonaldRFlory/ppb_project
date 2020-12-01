#include "stdafx.h"
#include <windows.h>
#include <WinBase.h>
#include  <assert.h>
#include <strsafe.h>
#include "mlink.h"
//#include <logging.h>

#include <stdio.h>

#include "findhid.h"
#define	BIORAD_VID   0x0614 
#define MAX_HID_BUFS    8


void ErrorExit(LPTSTR lpszFunction) 
{ 
    DWORD dw = GetLastError(); 
#if 0
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message and exit the process

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"), 
        lpszFunction, dw, lpMsgBuf); 
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
#endif
    ExitProcess(dw); 
}


//Sets max number of report buffers maintained by windows for incoming reports.
//Returns number of buffers actually set or -1 for any failure.
int SetHIDBuffers(HANDLE Handle, int NumBuffers)
{
  ULONG NumBufs;
  
  HidD_SetNumInputBuffers(Handle, NumBuffers);
  if(HidD_GetNumInputBuffers(Handle, &NumBufs))
    return (int)NumBufs;
  return -1;
}

//---------------------------------------------------------------------------------------
//  FUNCTION:   FindHID
//
//  DESCRIPTION:
//		This simplified version is for Universal Upgrader only!!
//		Search through connected HIDs for device matching supplied VID,
//      in control structure. 
//		Also supports counting of matches only. Returns
//		capabilities, count of matches and/or HANDLE if non-null pointers 
//		supplied. Control structure also provides an Nth variable which, 
//		specifies index of device to open. (Used after enumerating device
//		and selecting one from a list. If Ctrl.Enumerate is set, causes
//		return of handle to the 'Nth' matching device found with the
//		device open.
//
//  PARAMETERS:   
//                
//
//  RETURNS:      TRUE == 1 if successful, else FALSE==0.
//
//  Notes:        Adapted from equivalent iTower function
//                (But I'll use different method to find correct device handle)
//-----------------------------------------------
bool FindHID( FHID_CTRL &Ctrl , HIDP_CAPS &HIDCaps, HANDLE &Handle) 
{
	USHORT    MatchCount = 0;
	HANDLE    hDev = INVALID_HANDLE_VALUE;
	HANDLE    MatchHandle = INVALID_HANDLE_VALUE;
	GUID      m_hidGuid;
	HDEVINFO  m_devInfo;
	DWORD     dwNumCollections = 0;
	DWORD     dwErr = 0, k=1;
	DWORD     dwRequiredSize = 0;
	BOOL      retBool = TRUE;
	BOOL      Close  = TRUE;
	BOOL      bRet = FALSE;
	SP_DEVICE_INTERFACE_DATA      spdev;
	SP_INTERFACE_DEVICE_DETAIL_DATA    spiDevData;
	PSP_INTERFACE_DEVICE_DETAIL_DATA  pSp_Interface_Device_detail_data = NULL;
	DWORD     nwritten= 0;
	LPVOID    pParam = 0;

	// get the GUID for HID
	HidD_GetHidGuid(&m_hidGuid);
	m_devInfo = SetupDiGetClassDevs
	(
		&m_hidGuid,
		NULL,
		NULL,
		DIGCF_INTERFACEDEVICE | DIGCF_PRESENT
	);
	
	if (m_devInfo == INVALID_HANDLE_VALUE)
	{  //not clear why we try twice
	
		m_devInfo = SetupDiGetClassDevs
		(
			&m_hidGuid,
			NULL,
			NULL,
			DIGCF_INTERFACEDEVICE | DIGCF_PRESENT
		);
	
		if (m_devInfo == INVALID_HANDLE_VALUE)
		{
			return false;
		}
	}
	
	spdev.cbSize = sizeof(spdev);
	spiDevData.cbSize = sizeof(spiDevData);
	Ctrl.Count = 0;

	while(retBool = SetupDiEnumDeviceInterfaces
				 	(
						m_devInfo,NULL,&m_hidGuid,dwNumCollections++,&spdev
					)
		 )
	{
		BOOL retIntBool = SetupDiGetDeviceInterfaceDetail
		(
			m_devInfo,         
	    	&spdev,          
	    	NULL,           
	    	0,             
	    	&dwRequiredSize,     
	   		NULL
	   	);
		pSp_Interface_Device_detail_data = (PSP_INTERFACE_DEVICE_DETAIL_DATA)
			new char[sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA) + dwRequiredSize + 1];

		pSp_Interface_Device_detail_data->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
		retIntBool = SetupDiGetDeviceInterfaceDetail
		(
			m_devInfo,				 
			&spdev,					
			pSp_Interface_Device_detail_data,					 
			dwRequiredSize,						 
			NULL,//&dwRequiredSize,		 
			NULL
		);
		hDev = CreateFile
			   (
				   pSp_Interface_Device_detail_data -> DevicePath,		
				   GENERIC_READ|GENERIC_WRITE,	
				   FILE_SHARE_READ | FILE_SHARE_WRITE,									 
				   NULL,	//security attribute (none)								
				   OPEN_EXISTING,					 
				   FILE_FLAG_OVERLAPPED, //0, //FILE_ATTRIBUTE_SYSTEM,			
				   NULL
			   );
	
		if ( hDev == INVALID_HANDLE_VALUE )
		{
			continue;
		}
	
		delete pSp_Interface_Device_detail_data;
	
		// Now we need to determine if this HID is a match
		// By checking its manufacturer and product IDs
		HIDD_ATTRIBUTES hidAttribs;
		PHIDP_PREPARSED_DATA	devData;
		HIDP_CAPS				hidCaps;
	
		BOOL retParsed = HidD_GetPreparsedData ( hDev, &devData );
		NTSTATUS ntStatus = HidP_GetCaps ( devData, &hidCaps );
	
		BOOLEAN bGotAttribs = HidD_GetAttributes(hDev, &hidAttribs );
		
		if (bGotAttribs)
		{
			//We always need a match on VID
			if(hidAttribs.VendorID == Ctrl.VID)
			{
				if((Ctrl.PID == 0XFFFF) || (hidAttribs.ProductID == Ctrl.PID))
				{ //PID of 0XFFFF means match any ProductID
					if(MatchCount >= MAX_FINDHID_DEVS)
					{
						++MatchCount; //indicate we found one more than we could handle
						break;//out of while loop and return
					}
					Ctrl.FoundPIDs[MatchCount] = hidAttribs.ProductID;
					if(!Ctrl.Enumerate)
					{//that is, if not just counting the matches
						if(MatchCount >= Ctrl.Nth)
						{	//return handle to Nth one and copy hidCaps
							++MatchCount;
							MatchHandle = hDev;
							HIDCaps = hidCaps;
							break;//out of while loop and return
						}
					}
					++MatchCount;
				}
			}
		}
		CloseHandle( hDev );
		hDev= INVALID_HANDLE_VALUE;
	}
	if(MatchCount == 0)
	{
		return false;
	}
	Handle = MatchHandle;
	HidD_SetNumInputBuffers(Handle, MAX_HID_BUFS);
	Ctrl.Count = MatchCount;
	return true;
} // endf FindHID
	

void InitFindHIDCtrl( FHID_CTRL &Ctrl ) 
{
	Ctrl.Enumerate = false;
	Ctrl.VID	 = 0XFFFFU;
	Ctrl.PID	 = 0XFFFFU;
	Ctrl.Usage = 0XFFFFU;
	Ctrl.Nth = 0;
}



HANDLE USBHandle;
unsigned int ExtraResponses;


OVERLAPPED WriteOverlapped;
OVERLAPPED ReadOverlapped;
bool GetUSBReport(LINK_SEL &LSel, UCHAR *ReportBuff, int TimeoutMsec)
{
	ULONG      nread;
	bool      l_fRes        = false; 
	DWORD      l_dwRes; 

	if(ReadOverlapped.hEvent == NULL)
	{
		ReadOverlapped.hEvent =  CreateEvent(NULL, true, true, TEXT(""));
		ReadOverlapped.Offset=0;
		ReadOverlapped.OffsetHigh=0;
	}
//	ResetEvent(ReadOverlapped.hEvent);

	// Read response from device using HID report
	l_fRes =  (ReadFile( LSel.LHand, (char *)ReportBuff, LSel.ReadSize, &nread, &ReadOverlapped ) != 0);

	l_dwRes = WaitForSingleObject(ReadOverlapped.hEvent, TimeoutMsec);
	
	if (WAIT_OBJECT_0 != l_dwRes)
	{   // timed out or other failure on status report
	    if (!l_fRes)
	    {   // Read File failed on status report
			//ErrorExit("WaitForSingleObject"); 
			CancelIo(LSel.LHand);
			return false;
	    }
	}
	return true;
}


bool QuickGetResponse(LINK_SEL &LSel, UCHAR *RtnBuff)
{
	//This is a hack 
	ULONG      nread;
	bool      l_fRes        = false; 
	bool      l_dwRes        = false; 
	
	if(ReadOverlapped.hEvent == NULL)
	{
		ReadOverlapped.hEvent =  CreateEvent(NULL, true, true, TEXT(""));
		ReadOverlapped.Offset=0;
		ReadOverlapped.OffsetHigh=0;
	}

	// wait until device signals response ready via status report
	l_fRes =  (ReadFile(LSel.LHand, (char *)RtnBuff, LSel.ReadSize, &nread, &ReadOverlapped) != 0);

	if( WaitForSingleObject(ReadOverlapped.hEvent,0000) != WAIT_OBJECT_0)
	{
		CancelIo(LSel.LHand);
		return false;
	}
	if (!l_fRes)
	{
		return false;
	}
	return true;
}

//note default param:
//USHORT SendUSBReport(UCHAR *ReportBuff, int TimeoutMsec, UCHAR FlushResponse = 1);
bool SendUSBReport(LINK_SEL &LSel, UCHAR *ReportBuff, int TimeoutMsec, UCHAR FlushResponse)
{
	UCHAR TBuff[1024];
	ULONG      nwritten;
	DWORD      l_dwRes; 
	bool    BWriteSucess;
	bool    Result;
	
	if(LSel.LHand == NULL)
	{
		return false;
	}
	if(WriteOverlapped.hEvent == NULL)
	{
		WriteOverlapped.hEvent =  CreateEvent(NULL, true, true, TEXT(""));	 //enable manual reset, initially signalled
		WriteOverlapped.Offset=0;
		WriteOverlapped.OffsetHigh=0;
	}
	Result = (ResetEvent(WriteOverlapped.hEvent) != 0);

	//Flush out any waiting responses
	if(FlushResponse)
	{
	  while( QuickGetResponse(LSel, TBuff))
	    ++ExtraResponses;
	}
	
	BWriteSucess = (WriteFile( LSel.LHand, (char *)ReportBuff, LSel.WriteSize,
	                          &nwritten, &WriteOverlapped) != 0);
	

	l_dwRes = WaitForSingleObject(WriteOverlapped.hEvent, TimeoutMsec);
	
	if(BWriteSucess)
	{
		if(nwritten == LSel.WriteSize) 
		{
	    	return true;
		}
		return false;
	}
	if(l_dwRes == STATUS_TIMEOUT) //0x0000102
	{
		CancelIo(LSel.LHand);
	    return false;
	}
	
	if (WAIT_OBJECT_0 != l_dwRes)	//0x00000000
	{
		ErrorExit(TEXT("WaitForSingleObject in SendUSBReport")); 
		return false;
	}
	return true;
}

