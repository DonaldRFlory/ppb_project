#include "type.h"
#include <EEPROM.h>
#include "ppb.h"
#include <math.h>
U8 NodeAddress = 0;
U8 RampDivide;
void InitStepping();
extern U8 StrBuffer[64];
extern U8 S2Dir;
extern bool S1LinkControl, S2LinkControl;
extern U16 T2Count;
extern bool TickFlag;

extern int XStepsSave, YStepsSave;

//this is all about capturing total x and y steps taken during a circle and
//then setting this flag so that background loop can print them. For debugging
//of x and y creep.
extern bool CircleStartFlag;
extern STEP_CTRL StepCtrl[MAX_STEPPERS];
//#define MAX_INPUT_VALS 10
//four AD Vals, status bits, two step counts, TB_CNT, S2Period and finally TraceBuffCount
U16 InputVals[MAX_INPUT_VALS];
//The Period values will always be 3 counts less than desired (in 4Usec counts) to account for 12 USec step pulse


void WriteTraceBuffer(U8* InBuff, U16 Count);
void SetRampParams(float VMin, float VMax, float Accel);
void  DoCycleMove(U8 StepperIndex);
U8 SetServoUsec(U8 ServoIndex, U16 USec); //in linkfuns.cpp
void Servo4Serve();                       //in linkfuns.cpp
extern int  LinkCount;
void LinkServe();
extern U8 LinkBuffer[];
static bool MsecTick;

bool SwitchOn;
void setup()
{
  // put your setup code here, to run once:

  pinMode(TX_PIN, INPUT_PULLUP);//leave TX pin as input until we get link command addressed to us

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(TELLTALE_PIN, OUTPUT);

  pinMode(SOLENOID1_PIN, OUTPUT);
  pinMode(SOLENOID2_PIN, OUTPUT);
  pinMode(SOLENOID3_PIN, OUTPUT);
  pinMode(SOLENOID4_PIN, OUTPUT);

  pinMode(STEPPER1_STEP, OUTPUT);
  pinMode(STEPPER1_DIR, OUTPUT);//Step1 direction
  pinMode(STEPPER2_STEP, OUTPUT);
  pinMode(STEPPER2_DIR, OUTPUT);//Step2 direction
  pinMode(STEPPER3_STEP, OUTPUT);
  pinMode(STEPPER3_DIR, OUTPUT);
  pinMode(STEPPER4_STEP, OUTPUT);
  pinMode(STEPPER4_DIR, OUTPUT);

  digitalWrite(STEPPER1_DIR, HIGH);
  digitalWrite(STEPPER1_STEP, HIGH);

  digitalWrite(STEPPER2_DIR, HIGH);
  digitalWrite(STEPPER2_STEP, HIGH);

  pinMode(AUX1, INPUT_PULLUP);
  pinMode(A2, INPUT_PULLUP);//Limit sw 1
  pinMode(A3, INPUT_PULLUP);//Limit sw 2
  pinMode(A4, INPUT_PULLUP);//Limit sw 3
  pinMode(A5, INPUT_PULLUP);//Limit sw 4

  NodeAddress = EEPROM.read(NODE_ADDRESS_EEPROM_ADDRESS);
  Serial.begin(115200);
  //Timer setup -------------------------------------
  TCCR0A = 0;
 // TIMSK0 |= _BV(OCIE0A);
  TCCR2A = 0;
  TCCR2B = 4;//divide by 64 for 4USec clock period
  TIMSK2 = _BV(TOIE2);
  //OCIE0A OCIE0B TOIE0;
  //OCIE2A OCIE2B TOIE2;

  SetRampParams(VELMIN, VELMAX, ACCEL);

  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(SOLENOID1_PIN, LOW);
  digitalWrite(SOLENOID2_PIN, LOW);
  digitalWrite(SOLENOID3_PIN, LOW);
  digitalWrite(SOLENOID4_PIN, LOW);
  //  Serial.write("Link fun count = ");
  //  Serial.println(LinkCount);

  InitStepping();
  sprintf(StrBuffer, "Hello at startup\n");
  WriteTraceBuffer(StrBuffer, strlen(StrBuffer));
}



void DoMsecTick();
int Count1, Count2, Divide1, Divide2, DivideCount1 = 16, DivideCount2 = 16;
bool OddInterrupt;
extern void Ramp(U8 StepperIndex);

unsigned long OldMsec;
void loop()
{
  unsigned long temp;
  // put your main code here, to run repeatedly:

  if(LIMIT1())
  {
    InputVals[4] |= IV_LIMIT1_BIT;
    if(!StepCtrl[0].Dir)
    {
        noInterrupts();
        StepCtrl[0].StepCount = 0;
        interrupts();
    }
  }
  else
  {
    InputVals[4] &= ~IV_LIMIT1_BIT;
  }

  if(LIMIT2())
  {
    InputVals[4] |= IV_LIMIT2_BIT;
    if(!StepCtrl[1].Dir)
    {
        noInterrupts();
        StepCtrl[1].StepCount = 0;
        interrupts();
    }
  }
  else
  {
    InputVals[4] &= ~IV_LIMIT2_BIT;
  }

  if(LIMIT3())
  {
    InputVals[4] |= IV_LIMIT3_BIT;
    if(!StepCtrl[2].Dir)
    {
        noInterrupts();
        StepCtrl[2].StepCount = 0;
        interrupts();
    }
  }
  else
  {
    InputVals[4] &= ~IV_LIMIT3_BIT;
  }

  if(LIMIT4())
  {
    InputVals[4] |= IV_LIMIT4_BIT;
    if(!StepCtrl[3].Dir)
    {
      noInterrupts();
      StepCtrl[3].StepCount = 0;
      interrupts();
    }
  }
  else
  {
    InputVals[4] &= ~IV_LIMIT4_BIT;
  }

  DoLink();
  if(TickFlag)
  {
    SET_TELLTALE();
    //SET_STEP3();
    TickFlag = 0;
    DoMsecTick(); //actually 1.024 msec tick
    CLR_TELLTALE();
  }
}

U8 LinkState = L_IDLE;

void FlushSerialIn(int Bytes)
{
  U8 Dummy;
  while (Bytes > 0)
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
  switch (LinkState)
  {
    case L_IDLE:
      IdleFlag = 1;
    case L_RCV:
      if (AvailCount)
      {
        if (IdleFlag)
        {
          LinkBuffer[0] = (U8)(Serial.read() + 1);//for now count of all payload bytes in packet, including length byte
          --AvailCount;
          BytesRead = 1;
        }
        if (((AvailCount + BytesRead) > LinkBuffer[0]) || (LinkBuffer[0] == 1))
        {
          LinkState = L_FAULT;//too much data there or zero length command
          return; //fault state will flush it
        }
        BytesRead += (U8)Serial.readBytes(&(LinkBuffer[BytesRead]), AvailCount);//read all that are available
        if (BytesRead >= LinkBuffer[0])
        {
          //we have them all so break and drop down to execute
          break;
        }
        LinkState = L_RCV; //more to come
      }
      return;

    case L_FAULT:
      if (AvailCount)
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
      if (Serial.availableForWrite() >= TX_BUFFER_SIZE)
      {
        LinkState = L_FINAL;
      }
      return;

    case L_FINAL:
      //We will turn off TX enable here when we set up shared bus
      pinMode(TX_PIN, INPUT_PULLUP);//set TX pin to input since we are done sending and serial is party line
      //as one msec has elapsed since we saw xmit buffer empty
      LinkState = L_IDLE;
      return;

  }
  //TurnLEDOn();
  //when we get here, we have a command packet
  //to execute
  --LinkBuffer[0];//They need count of raw packet only, excluding the length byte
  //In LinkServe(), we check if it is addressed to us first, and if not we return with 0 in LinkBuffer[0]/
  LinkServe();//execute it, if result produced, length will be in LinkBuffer[0]
  if (LinkBuffer[0])
  {
    Serial.write(LinkBuffer, LinkBuffer[0] + 1);//want to send the length byte as well as packet whose length it specifies
    LinkState = L_RSP;
  }
  else
  {
    LinkState = L_IDLE;
  }
}


U8 SwitchDebounce;
//Called every MSEC_COMMUTATOR_DIVIDE (4) milliseconds
void DoSwitch()
{

  if (SwitchOn)
  {
    if (digitalRead(AUX1))
    { //high means switch is off
      SwitchOn = false;
      SwitchDebounce = 0;
    }
  }
  else
  {
    if (!digitalRead(AUX1))
    { //low means switch is on
      SwitchDebounce += MSEC_COMMUTATOR_DIVIDE;
      if (SwitchDebounce >= SWITCH_DEBOUNCE_MSEC)
      {
        TurnLEDOn();
        SwitchOn = true;
        //put switch close action here if any
       }
    }
  }
}

void DoMsecTick()
{
  static unsigned char Commutator = 0;

  ++Commutator;
  Commutator &= 3;
  ++Count;
  if (Count == 0)
  {
    if (LEDOn)
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

  switch (Commutator)
  { //one of four AD channels is converted each millisecond
    //in a four millisecond round robin sequence.
    //The AD CHannels uses are A0, A1, A6, and A7, stored
    //respectively in InputVals[0]-InputVals[3].
    //Used A6 and A7 since on nano, they are only usable for A/D
    //A2-A5 can also be used for digital I/O.
    default:
    case 0:
      if(!(TIMSK0 & _BV(OCIE0A)))  //if inactive
      {
        if(StepCtrl[0].StepCount)
        {
            OCR0A = TCNT0 + 2; //we want to trigger interrupt in 8 USec to sync with timer ISR at start
            TIMSK0 |= _BV(OCIE0A); //enable the interrupt
        }
        else
        {
            DoCycleMove(0);
        }
      }
      else
      {
        Ramp(0);
      }
      InputVals[0] = analogRead(A0);
      break;

    case 1:
      if(!(TIMSK0 & _BV(OCIE0B)))  //if inactive
      {
        if(StepCtrl[1].StepCount)
        {
            OCR0B = TCNT0 + 2; //we want to trigger interrupt in 8 USec to sync with timer ISR at start
            TIMSK0 |= _BV(OCIE0B); //enable the interrupt
        }
        else
        {
            DoCycleMove(1);
        }
      }
      else
      {
        Ramp(1);
      }
      InputVals[1] = analogRead(A1);
      break;

    case 2:
      if(!(TIMSK2 & _BV(OCIE2A)))  //if inactive
      {
        if(StepCtrl[2].StepCount)
        {
            OCR2A = TCNT2 + 2; //we want to trigger interrupt in 8 USec to sync with timer ISR at start
            TIMSK2 |= _BV(OCIE2A); //enable the interrupt
        }
        else
        {
            DoCycleMove(2);
        }
      }
      else
      {
        Ramp(2);
      }
      InputVals[2] = analogRead(A6);
      break;

    case 3:
      if(!(TIMSK2 & _BV(OCIE2B)))  //if inactive
      {
        if(StepCtrl[3].StepCount)
        {
            OCR2B = TCNT2 + 2; //we want to trigger interrupt in 8 USec to sync with timer ISR at start
            TIMSK2 |= _BV(OCIE2B); //enable the interrupt
        }
        else
        {
            DoCycleMove(3);
        }
      }
      else
      {
        Ramp(3);
      }
      InputVals[3] =  analogRead(A7);
      break;
  }

}


extern U16 TBCount;
extern U8 ParsGroup;
void LoadInputValues()
{
    if(SwitchOn)
    {
        InputVals[4] |= IV_SWITCH_BIT;
    }
    else
    {
        InputVals[4] &= ~IV_SWITCH_BIT;
    }
    //Note: InputVals[4] also has the four limit sensor states loaded into 4 bits.
    noInterrupts();

    InputVals[TBCOUNT_IV_IDX] = TBCount;//trace buffer count, index 9
    switch(ParsGroup)
    {
        case 1:
        default:

            InputVals[S1_IV_IDX] = StepCtrl[0].StepCount;//index 5
            InputVals[S2_IV_IDX] = StepCtrl[1].StepCount;//index 6
            InputVals[S3_IV_IDX] = StepCtrl[2].StepCount;  //index 7
            InputVals[S4_IV_IDX] = StepCtrl[3].StepCount;      //index 8
            break;

        case 2:
            InputVals[S1_IV_IDX] = StepCtrl[0].StepPos; //index 5
            InputVals[S2_IV_IDX] = StepCtrl[1].StepPos; //index 6
            InputVals[S3_IV_IDX] = StepCtrl[2].StepPos; //index 7
            InputVals[S4_IV_IDX] = StepCtrl[3].StepPos; //index 8
            break;

        case 3:
            InputVals[S1_IV_IDX] = StepCtrl[0].Period.I; //index 5
            InputVals[S2_IV_IDX] = StepCtrl[1].Period.I; //index 6
            InputVals[S3_IV_IDX] = StepCtrl[2].Period.I; //index 7
            InputVals[S4_IV_IDX] = StepCtrl[3].Period.I; //index 8
            break;

        case 4:
            InputVals[S1_IV_IDX] = StepCtrl[0].StepPos; //index 5
            InputVals[S2_IV_IDX] = StepCtrl[1].StepPos; //index 6
            InputVals[S3_IV_IDX] = StepCtrl[2].StepPos; //index 7
            InputVals[S4_IV_IDX] = StepCtrl[3].StepPos; //index 8
            break;

    }

    interrupts();
}
