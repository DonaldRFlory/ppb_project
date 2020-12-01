//Functions to handle streamed reports consisting of a series of 64 byte
//packets. The HID Out and In reports are set for 64 bytes. We use the
//first byte of each report to indicate it's position in a macro-report.
//or link packet.
#include "rom_drivers.h"
#include "usb.h"
#include "usbdesc.h"
#include "type.h"
#include "hidadapt.h"
#include "lpc13xx.h"                        /* LPC13xx definitions */


#define     EN_TIMER32_1    (1<<10)
#define     EN_IOCON        (1<<16)
#define     EN_USBREG       (1<<14)

#define PACKET_BUFFER_SIZE 1024

static U8 PacketBuffer[PACKET_BUFFER_SIZE];
HID_DEVICE_INFO HidDevInfo;
USB_DEV_INFO DeviceInfo;
static int HIDTimeouts;
extern volatile U8 HIDLinkTimer; //Counted down by timer ISR if it is non-zero
HID_CTRL HIDCtrl = {HID_IDLE, 0, 0, 0, 0};
static U32 PacketRcvErrors, PacketSndErrors, BabblePackets;


void InitHIDLink(void)
{
	HIDCtrl.State = HID_IDLE;
	HIDCtrl.LBuff = &PacketBuffer[2];
	HIDCtrl.MaxReturnSize = PACKET_BUFFER_SIZE - 2;//we add status byte and return FIdx
}

int GetHIDCommandPacket(U8 **CmdPtrAddress, U8 **RspPtrAddress, U16 *MaxReturnSizeAddress)
{
 	switch(HIDCtrl.State)
	{
		case HID_IDLE:
		default:
				return 0;

		case HID_RECEIVE:
		case HID_RESULT:
			if(HIDLinkTimer == 0)
			{ //Mainly so we never hang up out of sequence(while debugging perhaps)
				HIDCtrl.State = HID_IDLE;
				++HIDTimeouts;
			}
			return 0;

		case HID_EXECUTE:
			*MaxReturnSizeAddress = HIDCtrl.MaxReturnSize;
			*CmdPtrAddress = *RspPtrAddress	= HIDCtrl.LBuff;
			return HIDCtrl.ByteCount;
	}

}

//Set up for send of USB response. Data payload starts at PacketBuffer[2]
//Must only be called when HIDCtrl.State is HID_EXECUTE
void PostHIDResult(U16 ByteCount, U8 Status, U8 FIdx)
{
	//Maximum possible value of PACKET_BUFFER_SIZE is 4095
	//since maximum report we can send is 65 packets of 63 bytes
	#if (PACKET_BUFFER_SIZE > 4095)
		#error Packet buffer size may not be larger than 4095
	#endif
	if(ByteCount > (PACKET_BUFFER_SIZE - 2))
	{
		return;
	}
	PacketBuffer[0] = Status;
	PacketBuffer[1] = FIdx;
	HIDCtrl.ByteCount = ByteCount + 2;
	HIDCtrl.PacketCount = HIDCtrl.ByteCount/63;	//max possible is 65 packets
	if(HIDCtrl.ByteCount%63)
	{
		++HIDCtrl.PacketCount;
	}
	HIDCtrl.ByteIndex = HIDCtrl.PacketIndex = 0;
	HIDCtrl.State = HID_RESULT;
	HIDLinkTimer = HIDCtrl.PacketCount + 5; //Counted down by timer ISR if it is non-zero
	return;
}

//Called each time an HID In report is sent to get the next one
//When we enter HID_RESULT, we set up ByteCount, ByteIndex, PacketCount, and PacketIndex
//before we enter the state to minimize work in this ISR function.
void GetInReport (uint8_t Src[], uint32_t Length)
{
	U8 * Dest = Src;
	U8 Count, i;

	if(HIDCtrl.State != HID_RESULT)
	{
		for(i = 0; i < 64; ++i)
		{
			*Dest++ = (U8)i;//Send a start/end null packet with incremental data
		}
		return;
	}
	if(Length != 64)
	{ //probably can eliminate this for speed after initial testing
		*Dest = 0; //flag a null packet
		++PacketRcvErrors;
		return;
	}
	if(HIDCtrl.PacketIndex)
	{  //we are already in a send sequence
		if(HIDCtrl.PacketIndex >= (HIDCtrl.PacketCount - 1))
		{//this is the END packet
			Count = ((U8)(HIDCtrl.ByteCount - HIDCtrl.ByteIndex)) & (~HID_FLAG_MASK);
			*Dest++ = Count | END_FLAG;
		}
		else
		{	//this is a MIDDLE packet
			Count = 63;
			*Dest++ = (HIDCtrl.PacketIndex & (~HID_FLAG_MASK)) | MIDDLE_FLAG;
		}
		++HIDCtrl.PacketIndex;
	}
	else
	{ //starting to send
		if(HIDCtrl.PacketCount > 1)
		{	//at least a two packet result so we send a START packet
			Count = 63;
			*Dest++ = (HIDCtrl.PacketCount & (~HID_FLAG_MASK)) | START_FLAG;
		}
		else
		{  //send a START_END packet
			//I know.. If someone sets up a zero packet send, we will send one packet anyway.
			//Counting on setup function to do its' job and keep things simple and fast in ISR
			Count = (((U8)HIDCtrl.ByteCount) & (~HID_FLAG_MASK));
			*Dest++ = Count | START_END_FLAG;
		}
		HIDCtrl.PacketIndex = 1;
	}
	for(i = 0; i < Count; ++i)
	{
		*Dest++ = PacketBuffer[HIDCtrl.ByteIndex++];
	}
	if(HIDCtrl.PacketIndex >= HIDCtrl.PacketCount)
	{
		HIDCtrl.State = HID_IDLE; //and we are done
	}
}


//Called each time a Report (packet) is received
ROM ** rom = (ROM **)0x1fff1ff8;//pointer to ROM USB function table

//Length should always be 64 in our implementation
//Called each time an Out report comes in.
//If we want the packet, we have to copy it here
//We ignore all packets unless in HID_IDLE || HID_RECEIVE

void SetOutReport (uint8_t dst[], uint32_t Length)
{
	U8 * Source = dst;
	U8 End = 0, Flags;

	if(Length != 64)
	{
		++PacketSndErrors;
		return;
	}

	Flags = *Source++;
	if(HIDCtrl.State == HID_IDLE)
	{
		HIDCtrl.PacketIndex = HIDCtrl.ByteIndex = 0;
		switch(Flags & HID_FLAG_MASK)
		{
			case START_FLAG:   //at least a START and END packet
				HIDCtrl.ByteCount = 63;
				HIDCtrl.PacketCount = Flags & FLAG_COUNT_MASK;
				break;

			case START_END_FLAG:
				HIDCtrl.ByteCount = (Flags & FLAG_COUNT_MASK);
				if(!HIDCtrl.ByteCount)
				{
					return; //this is a null packet we ignore it.
				}
				HIDCtrl.PacketCount = 1;
				End = 1; //this is final packet
				break;

			case MIDDLE_FLAG: //cannot start with a MIDDLE_FLAG or an END_FLAG
				++PacketRcvErrors;
				return;
		}
		HIDLinkTimer = HIDCtrl.PacketCount + 5; //Counted down by timer ISR if it is non-zero
		HIDCtrl.State = HID_RECEIVE;
	}
	else if(HIDCtrl.State == HID_RECEIVE)
	{ //In middle of multi-packet sequence. Only MIDDLE_FLAG and END_FLAG are appropriate
		switch(Flags & HID_FLAG_MASK)
		{
			case END_FLAG:
				HIDCtrl.ByteCount += (Flags & FLAG_COUNT_MASK);
				End = 1; //final packet
				break;

			case MIDDLE_FLAG:  //PacketIndex in Flags must match our PacketIndex
				if((Flags & (~HID_FLAG_MASK)) != HIDCtrl.PacketIndex)
				{
					++PacketRcvErrors;
					HIDCtrl.State = HID_IDLE;
					return;
				}
				HIDCtrl.ByteCount += 63;
				break;

			default:
				++PacketRcvErrors;
				HIDCtrl.State = HID_IDLE;
				return;
		}
	}
	else
	{
		++BabblePackets;
		return;
	}

	//so we have something to copy
	if(HIDCtrl.ByteCount > PACKET_BUFFER_SIZE)
	{  //Don't overrun link buffer
		++PacketRcvErrors;
		HIDCtrl.State = HID_IDLE;
		return;
	}

	while(HIDCtrl.ByteIndex < HIDCtrl.ByteCount)
	{
		HIDCtrl.LBuff[HIDCtrl.ByteIndex++] = *Source++;
	}

	++HIDCtrl.PacketIndex;
	if(End)
	{
		if(HIDCtrl.PacketIndex < HIDCtrl.PacketCount)
		{//we got an END packet without getting all expected packets
			HIDCtrl.State = HID_IDLE;
			++PacketRcvErrors;
		}
		else
		{  //all good, got expected packets and an END (or was single packet message)
			HIDCtrl.State = HID_EXECUTE;
		}
	}
	else if(HIDCtrl.PacketIndex >= HIDCtrl.PacketCount)
	{//we have counted all expected packets and did not get an END
		HIDCtrl.State = HID_IDLE;
		++PacketRcvErrors;
	}

	//more to come in a multi-packet message
}


USB_IRQHandler(void)
{
  (*rom)->pUSBD->isr();
}

void HIDInit(void)
{
  U32 n;
  HidDevInfo.idVendor = BIORAD_VID;
  HidDevInfo.idProduct = FRACTION_COLLECTOR_PID;
  HidDevInfo.bcdDevice = USB_DEVICE;
  HidDevInfo.StrDescPtr = (uint32_t)&USB_StringDescriptor[0];
  HidDevInfo.InReportCount = 64;
  HidDevInfo.OutReportCount = 64;
  HidDevInfo.SampleInterval = 0x01;
  HidDevInfo.InReport = GetInReport;
  HidDevInfo.OutReport = SetOutReport;

  DeviceInfo.DevType = USB_DEVICE_CLASS_HUMAN_INTERFACE;
  DeviceInfo.DevDetailPtr = (uint32_t)&HidDevInfo;

  /* Enable Timer32_1, IOCON, and USB blocks (for USB ROM driver) */
  LPC_SYSCON->SYSAHBCLKCTRL |= (EN_TIMER32_1 | EN_IOCON | EN_USBREG);

  (*rom)->pUSBD->init_clk_pins();  //Call ROM function to initialize USB clock and USB pins

  /* insert a delay between clk init and usb init */
  for (n = 0; n < 75; n++)
  {
  }

  (*rom)->pUSBD->init(&DeviceInfo); /* USB Initialization */
  (*rom)->pUSBD->connect(TRUE);     /* USB Connect */
}

//Applies to raw link command packet, FIdx + ParamBytes + BlockDown Bytes
U16 HIDMaxLinkSend(void)
{
	return PACKET_BUFFER_SIZE;
}

//Applies to raw link response packet, ReturnBytes + BlockUp Bytes
U16 HIDMaxLinkReturn(void)
{
	return HIDCtrl.MaxReturnSize;
}
