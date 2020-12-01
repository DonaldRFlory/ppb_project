#include <Servo.h>
#include "type.h"
extern int  LinkCount;
void LinkServe();
extern U8 LinkBuffer[];
#define FAN_0   544
#define FAN_1   1225
#define FAN_2   1250
#define FAN_3   1275
#define FAN_4   1300
#define FAN_5   1325
#define FAN_6   1350
#define FAN_7   1375
#define FAN_8   1400
#define FAN_9   2400
#define TELLTALE_PIN 10
#define SERVO1_PIN 12
#define SERVO2_PIN 11
#define SERVO3_PIN 10
#define SERVO4_PIN 9
static U8 FanStepIndex = 1;
static int FanValue;
int MsecTickCount = 0;
static bool MsecTick;

Servo Servo1, Servo2, Servo3, Servo4;
int ServoPos = DEFAULT_PULSE_WIDTH;
void setup() {
  // put your setup code here, to run once:
 pinMode(LED_BUILTIN, OUTPUT);
 pinMode(TELLTALE_PIN, OUTPUT);

 Serial.begin(115200);
 // Timer0 is already used for millis() - we'll just interrupt somewhere
  // in the middle and call the "Compare A" function below
  OCR0A = 0x80;
  TIMSK0 |= _BV(OCIE0A);
  Servo1.attach(SERVO1_PIN);
  Servo2.attach(SERVO2_PIN);
  Servo3.attach(SERVO3_PIN);
  Servo4.attach(SERVO4_PIN);
//we will assume fan is on servo 4
  FanValue = FAN_8;
  Servo4.write(FanValue);
  digitalWrite(LED_BUILTIN, LOW);
//  Serial.write("Link fun count = ");
//  Serial.println(LinkCount);
}

void DoMsecTick();

void loop() {
  // put your main code here, to run repeatedly:

  if(MsecTick)
  {
    ++MsecTickCount;
    MsecTick = false;
    DoMsecTick();
  }
}

#define TX_BUFFER_SIZE 63   //max it will actually hold, array size is one byte more
#define L_FAULT 2
#define L_IDLE 0
#define L_RCV 1
#define L_FAULT 2
#define L_RSP 3
#define L_FINAL 4
U8 LinkState = L_IDLE;

void FlushSerialIn(int Bytes)
{
    U8 Dummy;
    while(Bytes > 0)
    {
        Dummy = (U8)Serial.read();
        --Bytes;
    }
}

//So... We are going to ignore bad received packets and make no response, we
//will go to fault state and keep flushing input data until we have a msec
//tick with nothing received.

static unsigned char Count = 0;//to time LED flash when link function execution sets it.
static bool LEDOn = false;
void TurnLEDOn()
{
    digitalWrite(LED_BUILTIN, HIGH);
    Count = 0;
    LEDOn = true;
}


void DoLink()
{
    int AvailCount;
    static U8 BytesRead;
    U8 IdleFlag = 0;
    AvailCount = Serial.available();//in case we need it below
    switch(LinkState)
    {
    case L_IDLE:
        IdleFlag = 1;
    case L_RCV:
        if(AvailCount)
        {
            if(IdleFlag)
            {
                LinkBuffer[0] = (U8)(Serial.read() + 1);//for now count of all payload bytes in packet, including length byte
                --AvailCount;
                BytesRead = 1;
            }
            if(((AvailCount + BytesRead) > LinkBuffer[0]) || (LinkBuffer[0] == 1))
            {
                LinkState = L_FAULT;//too much data there or zero length command
                return; //fault state will flush it
            }
            BytesRead += (U8)Serial.readBytes(&(LinkBuffer[BytesRead]), AvailCount);//read all that are available
            if(BytesRead >= LinkBuffer[0])
            {
                //we have them all so break and drop down to execute
                break;
            }
            LinkState = L_RCV; //more to come
        }
        return;

    case L_FAULT:
        if(AvailCount)
        {
            FlushSerialIn(AvailCount);//flush anything received since last tick
            //we stay in L_FAULT until we have a tick with nothing new received
        }
        else
        {
            LinkState = L_IDLE;//nothing else received so go to idle
        }
        return;

    case L_RSP:	 //waiting for xmit buffer to empty
    	if(Serial.availableForWrite() >= TX_BUFFER_SIZE)
    	{
            LinkState = L_FINAL;
    	}
        return;

    case L_FINAL:
            //We will turn off TX enable here when we set up shared bus
            //as one msec has elapsed since we saw xmit buffer empty
            LinkState = L_IDLE;
            return;

    }
    //TurnLEDOn();
    //when we get here, we have a command packet
    //to execute
    --LinkBuffer[0];//They need count of raw packet only, excluding the length byte
    LinkServe();//execute it, if result produced, length will be in LinkBuffer[0]
    if(LinkBuffer[0])
    {
      Serial.write(LinkBuffer, LinkBuffer[0] + 1);//want to send the length byte as well as packet whose length it specifies
      LinkState = L_RSP;
    }
    else
    {
      LinkState = L_IDLE;
    }
}

U16 ADVals[4];

void DoMsecTick()
{
  static unsigned char Commutator = 0;

  ++Commutator;
  Commutator &= 3;
  DoLink();
  ++Count;
  if(Count == 0)
  {
    if(LEDOn)
    {
      digitalWrite(LED_BUILTIN, LOW);
      LEDOn = false;
    }
//    else
//    {
//      digitalWrite(LED_BUILTIN, HIGH);
//    }
//    LEDOn = !LEDOn;
  }
  switch(Commutator)
  { //one of four AD channels converted each millisecond
    default:
    case 0:
       ADVals[0] = analogRead(A0);
       break;

    case 1:
        ADVals[1] = analogRead(A1);
        break;

    case 2:
        ADVals[2] = analogRead(A2);
        break;

    case 3:
        ADVals[3] = analogRead(A3);
        break;
  }

}

// Interrupt is called once a millisecond,
ISR(TIMER0_COMPA_vect)
{
  MsecTick = true;
}
