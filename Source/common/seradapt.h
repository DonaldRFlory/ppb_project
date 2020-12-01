//Functions and structures to handle serial port adaptation to
//link processing

typedef struct 
{
	unsigned char * LBuff;
	unsigned int MaxReturnSize;
	unsigned int LCount;
	unsigned char Error;
	unsigned char State;
}SER_CTRL;

#define SER_IDLE		0
#define SER_RECEIVE		1
#define SER_EXECUTE		2
#define SER_RESULT		3
#define SER_FAULT		4

#define PACKET_BUFF_OFFSET 4   //room for two length bytes, + status byte + return FIdx
#define PACKET_OVERHEAD_BYTES (PACKET_BUFF_OFFSET + 2)   //the +2 is the CRC bytes
//The length byte(s) are the count of actual payload bytes in command and response packets

#define BAUD_RATE  115200
#define PACKET_BUFFER_SIZE 256

#ifdef __cplusplus
   extern "C" {
#endif
void UARTInit (void);
U8 SerialLinkTimeout(void);
void SerialLinkTimer(void);//ISR context, called every millisecond
char ValidatePacket(void);
void PostSerialResult(U16 ByteCount, U8 Status, U8 FIdx);
void InitSerialLink(void);
U16 SerMaxLinkSend(void);
U16 SerMaxLinkReturn(void);
#ifdef __cplusplus
   }
#endif

