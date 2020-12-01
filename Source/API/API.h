//API.h
#include "type.h"
#include "stats.h"
#include "mlink.h"
#include "linkerror.h"
typedef U32 API_STAT;
typedef void * HANDLE;
typedef void * API_DEVICE_HANDLE;
#define CALL_CONV __stdcall

// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the DLL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// EXPORT_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef DLL_EXPORTS
#define PPB_API __declspec(dllexport)
#define DECL_SPEC __declspec(dllexport)
#else
#define PPB_API __declspec(dllimport)
#define DECL_SPEC __declspec(dllimport)
#endif
#define MAX_HANDLES 10
//#define ERRSUBSYS_POWER_PAC 0x03000000L

extern "C" {
DECL_SPEC API_STAT CALL_CONV Disconnect(API_DEVICE_HANDLE Handle);
DECL_SPEC API_STAT CALL_CONV SerialConnect(API_DEVICE_HANDLE &Handle, U8 CommIndex);
DECL_SPEC API_STAT CALL_CONV HIDConnect(API_DEVICE_HANDLE &Handle, U16 VID, U16 PID, U8 Index);
DECL_SPEC API_STAT CALL_CONV HIDCount(int &Count, U16 VID, U16 PID);
DECL_SPEC API_STAT CALL_CONV GetMaxLinkReturnSize(API_DEVICE_HANDLE Handle, U32 &Size);
DECL_SPEC API_STAT CALL_CONV GetMaxLinkSendSize(API_DEVICE_HANDLE Handle, U32 &Size);
//DECL_SPEC API_STAT CALL_CONV GetVelAcc(API_DEVICE_HANDLE Handle, U8 Channel, float &Velocity, float &Acceleration);
//DECL_SPEC API_STAT CALL_CONV SetVelAcc(API_DEVICE_HANDLE Handle, U8 Channel, float Velocity, float Acceleration);
} // End of Extern "C"
