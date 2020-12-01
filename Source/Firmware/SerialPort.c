//*****************************************************************************
//				SerialPort.c
//
//*****************************************************************************
#define SERIAL_PORT	1

#include "lpc13xx.h"                    // LPC13xx definitions
#include "type.h"
#include "target.h"
#include "link.h"
#include "seradapt.h"
#include <limits.h>

static SER_CTRL SerCtrl;
static int SerialTimeouts;

U16 ComputeCRC(U8 * Buff, U32 CharCount);
extern volatile U8 SerLinkTimer; //Counted down by timer ISR if it is non-zero
static void StartTransmit(void);

static U16 ReceiveIndex; //Indexes into PacketBuff as chars are received in appropriate states
static U16 ResponseIndex; //Indexes into PacketBuff as chars are returned in response
static U16 PacketLength;//length of packet less length byte (Payload + CRC1, CRC2)
unsigned int LBabbles;
unsigned int LTimeouts;

//Communication is strictly half-duplex, Command, response.
U8 PacketBuffer[PACKET_BUFFER_SIZE];

void InitSerialLink(void)
{
	SerCtrl.MaxReturnSize = PACKET_BUFFER_SIZE - PACKET_OVERHEAD_BYTES;
	SerCtrl.State = SER_IDLE;
	SerCtrl.LBuff  = &PacketBuffer[PACKET_BUFF_OFFSET];
}


//This is called frequently by background loop function LinkServe()
int GetSerialCommandPacket(U8 **CmdPtrAddress, U8 **RspPtrAddress, U16 *MaxReturnSizeAddress)
{
 	switch(SerCtrl.State)
	{
		default:
		case SER_IDLE:
			return 0;

		case SER_FAULT:
		   	SerCtrl.State = SER_IDLE; //don't think we can get here but..
		   	return 0;

		case SER_RECEIVE:
			if(SerLinkTimer == 0)
			{
				++SerialTimeouts;
				SerCtrl.State = SER_FAULT;//to freeze ISR
				PostSerialResult(0, LE_SHORT_PACKET, 255); //invalid function index
			}
			return 0;

		case SER_RESULT:
			if(SerLinkTimer == 0)
			{ //Mainly so we never hang up out of sequence(while debugging perhaps)
				SerCtrl.State = SER_IDLE;
				++SerialTimeouts;
			}
			return 0;

		case SER_EXECUTE:
			if(!ValidatePacket())
			{
				return 0;
			}
			*MaxReturnSizeAddress = SerCtrl.MaxReturnSize;
			*CmdPtrAddress = *RspPtrAddress	= SerCtrl.LBuff;
			return SerCtrl.LCount;
	}

}
//----------------------------------------------------------------------------
//  Initialize UART pins, Baudrate
//----------------------------------------------------------------------------
void UARTInit (void)
{
	unsigned int FDiv;
	unsigned int RegVal;

	ComputeCRC(0, 0); //just to get CRC table set up in advance
    LPC_SYSCON->SYSAHBCLKCTRL |= (1<<16);		// Power IOCON

	NVIC_DisableIRQ(UART_IRQn);

  	LPC_IOCON->PIO1_6 &= ~0x07;    /*  UART I/O config */
  	LPC_IOCON->PIO1_6 |= 0x01;     /* UART RXD */
  	LPC_IOCON->PIO1_7 &= ~0x07;
  	LPC_IOCON->PIO1_7 |= 0x01;     /* UART TXD */

	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<12);		// Power UART

	LPC_SYSCON->UARTCLKDIV = 1;					// Enable UART Clock

    LPC_UART->U0LCR    = 0x83;                  // 8 bits, no Parity, 1 Stop bit
//    LPC_UART->U0LCR    = 0x87;                  // 8 bits, no Parity, 2 Stop bits

	RegVal = LPC_SYSCON->UARTCLKDIV;
    FDiv = (((SystemCoreClock/LPC_SYSCON->SYSAHBCLKDIV)/RegVal)/16)/BAUD_RATE ;	/*baud rate */

    LPC_UART->U0DLM = FDiv / 256;
    LPC_UART->U0DLL = FDiv % 256;

    LPC_UART->U0LCR    = 0x03;                  // DLAB = 0
	LPC_UART->U0FCR    = 0x07;					// Enable and reset TX and RX FIFO.

    LPC_UART->U0IER    = 0x03;					// enable rx,tx ints
	SerCtrl.State = SER_IDLE;
	NVIC_EnableIRQ(UART_IRQn);
}


static void StartTransmit(void)
{	//start the send by loading first character
	LPC_UART->U0THR = PacketBuffer[ResponseIndex++];
} // end startTI



static U8 TwoLengthBytes;
/*****************************************************************************
*     FUNCTION NAME  -    UART_IRQHandler
*
*     DESCRIPTION    -    Serial ISR
*     REENTRANCE     -    No
*     PARAMETERS     -    None
*     RETURNS        -    None
*     NOTES          -	  None
*****************************************************************************/
void UART_IRQHandler(void)
{
	char InByte;
	char IntSrc;


	IntSrc = LPC_UART->U0IIR & 0x0F;

	switch(IntSrc)
	{

		case 0x04:	// Received char.
			InByte = LPC_UART->U0RBR;
			switch(SerCtrl.State)
			{
				case SER_IDLE:
			 		//Command packet format:
			 	   // one or two LengthByte(s), 1 to 32767 payload bytes, 2 bytes of CRC (ANSI CRC-16)
				   //we always want link payload in buffer starting at index 4, so there is
				   //room for response with two length byte, Stat byte, and return FIDx
					ReceiveIndex = 2;
					if(InByte & 0x80)
					{
						PacketBuffer[ReceiveIndex++] = InByte;//and put most sig byte of length into buffer along with hi-bit flag
						TwoLengthBytes = 1;					  //for later use in CRC calculation
						PacketLength = InByte & 0X7F;
					}
					else
					{
						PacketBuffer[ReceiveIndex++] = 0;//Most sig byte of length is zero
						PacketBuffer[ReceiveIndex++] = InByte;//Least  sig byte of length is 0-127,
						TwoLengthBytes = 0;
						PacketLength = InByte + PACKET_OVERHEAD_BYTES;;
					}
					SerLinkTimer = 3;//reset the interchar timer
					SerCtrl.State = SER_RECEIVE;
					break;

				case SER_RECEIVE:
					SerLinkTimer = 3;//reset the interchar timer
					if(TwoLengthBytes)
					{
						TwoLengthBytes = 0;
						PacketLength <<=  8;
						PacketLength += PACKET_OVERHEAD_BYTES;
						PacketLength += InByte;
					}
					else if(ReceiveIndex < PacketLength)
					{
						PacketBuffer[ReceiveIndex++] = InByte;
						if(ReceiveIndex >= PacketLength)
						{
							SerCtrl.State = SER_EXECUTE;//may fail immediately due to CRC check
						}
					}
					break;

				case SER_EXECUTE:
				case SER_RESULT:
					++LBabbles;//count unexpected received characters
					break;

				default:
					SerCtrl.State = SER_IDLE;//just in case we get into an unknown state
					break;
			}//end switch(SerCtrl.State)
			break;//end case 0x04 received char

		case 0x02:	// Transmit holding register empty.
			if(ResponseIndex < PacketLength)
			{
				LPC_UART->U0THR = PacketBuffer[ResponseIndex++];
			}
			else
			{ //we just finished sending the last character
				SerCtrl.State = SER_IDLE;
			}
			break;

		default:
			break;	// not sure here...
	} // end switch(IntSrc)
} 	// end UART_IRQHandler


char ValidatePacket(void)
{
	U16 CRC, CompCRC;

	//We always have length bytes in [2] and [3].
	//Any packet payload starts at [4] and must have at least
	//one byte of payload plus two CRC bytes, for a total 6
	if(ReceiveIndex < 6)
	{
		PostSerialResult(0, LE_BAD_FORMAT, 255);
		return FALSE;
	}
	--ReceiveIndex;
	CRC = PacketBuffer[ReceiveIndex--];
	CRC |= ((U16)PacketBuffer[ReceiveIndex]) << 8;
	if(PacketBuffer[2]) //if two length	bytes
	{
		CompCRC = ComputeCRC(&PacketBuffer[2], ReceiveIndex - 2);
	}
	else
	{
		CompCRC = ComputeCRC(&PacketBuffer[3], ReceiveIndex - 3);
	}
	SerCtrl.LCount = ReceiveIndex - 4;
	if(CompCRC != CRC)
	{
		PostSerialResult(0, LE_BAD_CHECK, 255);
		return FALSE;
	}
	return TRUE;
}

//The return value if any is in buffer starting at PacketBuffer[PACKET_BUFF_OFFSET]
void PostSerialResult(U16 ByteCount, U8 Status, U8 FIdx)
{
	U16 CRC;
	ResponseIndex = PACKET_BUFF_OFFSET;
   	PacketBuffer[--ResponseIndex] = FIdx;
	PacketBuffer[--ResponseIndex] = Status;
	ByteCount += 2;
	if(ByteCount > 0X7F)
	{
		PacketBuffer[--ResponseIndex] = (U8) ByteCount;
		PacketBuffer[--ResponseIndex] = (UCHAR)((ByteCount >> 8) | 0X80);
		PacketLength = ByteCount + 2;
		CRC = ComputeCRC(PacketBuffer, PacketLength);
	}
	else
	{
		PacketBuffer[--ResponseIndex] = (U8) ByteCount;
		PacketLength = ByteCount + 1;
		CRC = ComputeCRC(&PacketBuffer[ResponseIndex], PacketLength);
	}
	PacketBuffer[ResponseIndex + PacketLength++] = (U8)(CRC >> 8);
	PacketBuffer[ResponseIndex + PacketLength++] = (U8)CRC;
	PacketLength += ResponseIndex; //PacketLength is actually one more than max index for send
	SerLinkTimer = 20;
    SerCtrl.State = SER_RESULT;
	StartTransmit();//transmit the first character here
}

U16 SerMaxLinkSend(void)
{
	return PACKET_BUFFER_SIZE - PACKET_OVERHEAD_BYTES;
}

U16 SerMaxLinkReturn(void)
{
	return PACKET_BUFFER_SIZE - PACKET_OVERHEAD_BYTES;
}

//*****************************************************************************
//             END FILE:   SerialPort.c
//*****************************************************************************
