//Functions to handle streamed reports consisting of a series of 64 byte
//packets. The HID Out and In reports are set for 64 bytes. We use the
//first byte of each report to indicate it's position in a macro-report.
//or link packet.

//Values for flag bits in first byte of each 64 byte HID report
#define HID_FLAG_MASK 0XC0
#define FLAG_COUNT_MASK 0X3F
#define START_FLAG 0X40
#define MIDDLE_FLAG 0X80
#define START_END_FLAG 0X00
#define END_FLAG 0XC0

//Total for longest link packet with START, 63 MIDDLE and END packets
#define MAX_LINK_PACKETS 65

#define HID_IDLE		0
#define HID_RECEIVE	1
#define HID_EXECUTE	2
#define HID_RESULT	3

//Bio-Rad
#define BIORAD_VID  0x0614
#define FRACTION_COLLECTOR_PID	0x040E

//NXP
#define USB_VENDOR_ID 0x1FC9
#define USB_PROD_ID   0x0003
#define USB_DEVICE	  0x0100

typedef struct
{
	U8 * LBuff;
	U8 State;
	U16 ByteCount;
	U16 ByteIndex;
	U8 PacketCount;
	U8 PacketIndex;
	U16 MaxReturnSize;
} HID_CTRL;

#ifdef __cplusplus
   extern "C" {
#endif

void InitHIDLink(void);
void HIDInit(void);
void PostHIDResult(U16 ByteCount, U8 Status, U8 FIdx);
int PostResponse(void);
void HIDLinkServe(void);
U16 HIDMaxLinkSend(void);
U16 HIDMaxLinkReturn(void);

#ifdef __cplusplus
   }
#endif
