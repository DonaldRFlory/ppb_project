#include "type.h"
#include "ppb.h"
#include <math.h>
typedef union
{
    U16 I;
    U8 B[2];
}BWUnion;

//The Period values will always be 3 counts less than desired (in 4Usec counts) to account for 12 USec step pulse

#define MIN_STEP_CLOCKS  12
#define MAX_STEP_CLOCKS  400

bool SpeedUp;

//If true, means stepper was controlled by link call since last activated via Pot
//on A/D input, so the ManageSteppers() function will not turn it off when in
//the dead-zone at middle of Pot range. If Pot stays in dead-zone, link functions
//can freely control the steppers. If Pot for a stepper leaves dead-zone, the
//Pot regains control.
bool S1LinkControl, S2LinkControl;

U16 S2Accumulator;
bool SteppersInterlocked = true;
bool S1Busy, S2Busy;
U16 S1StepCount, S2StepCount;
BWUnion S1Period, S2Period, S2PeriodCount, S1PeriodCount;

void ManageSteppers();
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
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(TELLTALE_PIN, OUTPUT);

  pinMode(SOLENOID1_PIN, OUTPUT);
  pinMode(SOLENOID2_PIN, OUTPUT);
  pinMode(SOLENOID3_PIN, OUTPUT);
  pinMode(SOLENOID4_PIN, OUTPUT);

  pinMode(STEPPER1_ENABLE, OUTPUT);
  pinMode(STEPPER1_STEP, OUTPUT);
  pinMode(STEPPER1_DIR, OUTPUT);//Step1 direction
  pinMode(STEPPER2_ENABLE, OUTPUT);
  pinMode(STEPPER2_STEP, OUTPUT);
  pinMode(STEPPER2_DIR, OUTPUT);//Step1 direction

  digitalWrite(STEPPER1_DIR, HIGH);
  digitalWrite(STEPPER1_ENABLE, HIGH);
  digitalWrite(STEPPER1_STEP, HIGH);

  digitalWrite(STEPPER2_DIR, HIGH);
  digitalWrite(STEPPER2_ENABLE, HIGH);
  digitalWrite(STEPPER2_STEP, HIGH);

  pinMode(AUX1, INPUT_PULLUP);

  Serial.begin(115200);
  // Timer0 is already used for millis() - we'll just interrupt somewhere
  // in the middle and call the "Compare A" function below
  TCCR0A = 0;
  TIMSK0 |= _BV(OCIE0A);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(SOLENOID1_PIN, LOW);
  digitalWrite(SOLENOID2_PIN, LOW);
  digitalWrite(SOLENOID3_PIN, LOW);
  digitalWrite(SOLENOID4_PIN, LOW);
  //  Serial.write("Link fun count = ");
  //  Serial.println(LinkCount);
  S1Period.I = 65535;
  S2Period.I = 65535;
}

void DoMsecTick();
int Count1, Count2, Divide1, Divide2, DivideCount1 = 16, DivideCount2 = 16;
bool OddInterrupt;

extern volatile unsigned long timer0_overflow_count;
unsigned long OldMsec;
void loop()
{
  unsigned long temp;
  // put your main code here, to run repeatedly:


  if(S1StepCount == 0)
  {
   // S1StepCount = 2000;
  }

  if(S2StepCount == 0)
  {
    //S2StepCount = 2000;
  }


  DoLink();
  if (OldMsec != timer0_overflow_count)
  {
    OldMsec = timer0_overflow_count;
    digitalWrite(TELLTALE_PIN, HIGH);
    DoMsecTick();
    ManageSteppers();//control speed of two stepper controllers
                     //based on setting of Sensors 3 and 4
    digitalWrite(TELLTALE_PIN, LOW);
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
      //as one msec has elapsed since we saw xmit buffer empty
      LinkState = L_IDLE;
      return;

  }
  //TurnLEDOn();
  //when we get here, we have a command packet
  //to execute
  --LinkBuffer[0];//They need count of raw packet only, excluding the length byte
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

U16 InputVals[MAX_INPUT_VALS];

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
      InputVals[4] = 0;
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
        InputVals[4] = 1;
        //put switch close action here if any
       }
    }
  }
}

//We are going to call DoSteppers() at beginning of DoMsecTick()
//to activate step pulse for any active stepper whose step period has
//elapsed. We count on delay introduced by analog read to time the pulse(s)
//and will reset all pulse lines at end of DoMsecTick(). This will ensure
//minimum pulse width of 10 USec required by stepper controllers.
void DoMsecTick()
{
  static unsigned char Commutator = 0;

#if 0
  if(SpeedUp)
  {
    --S1Period.I;
    if(S1Period.I < MIN_STEP_CLOCKS)
    {
        S1Period.I = MIN_STEP_CLOCKS;
        SpeedUp = false;
    }
  }
  else
  {
    ++S1Period.I;
    if(S1Period.I > MAX_STEP_CLOCKS)
    {
        S1Period.I = MAX_STEP_CLOCKS;
        SpeedUp = true;
    }
  }
 #endif

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
    default:
    case 0:
    DoSwitch(); //Every MSEC_COMMUTATOR_DIVIDE (4) milliseconds
     InputVals[0] = analogRead(A0);
      break;

    case 1:

      InputVals[1] = analogRead(A1);
      break;

    case 2:
      InputVals[2] = analogRead(A2);
      break;

    case 3:
      InputVals[3] =  analogRead(A3);
      break;
  }

}

ISR(TIMER0_COMPA_vect)
{
    if(SteppersInterlocked)
    {
        digitalWrite(STEPPER2_STEP, HIGH); //inactive
    }
    if(S1Busy)
    {
        digitalWrite(STEPPER1_STEP, HIGH); //inactive
        if(S1PeriodCount.B[1] != 0)
        {
            --S1PeriodCount.B[1];
            return;  //leaving OCR0A alone causes full timer cycle or 1024 USec
        }
        if(S1PeriodCount.B[0] != 0)//short period portion
        {
            OCR0A += S1PeriodCount.B[0]; //so we time fractional part (or whole cycle if fractional part is zero)
            S1PeriodCount.B[0] = 0; //clear the short period part
            return;
        }
        --S1StepCount;
        S1Busy = false; //we must have finished timing this step
        //and we fall through to check if more steps are required
    }

    //if we get here we are not BUSY
    if(S1StepCount)
    {
        if(SteppersInterlocked)
        {
            S2Accumulator += STEPPER_2_INCREMENT;
            if(S2Accumulator >= 0X8000)
            {
                S2Accumulator &= ~0x8000;
                digitalWrite(STEPPER2_STEP, LOW); //activate step pulse
            }
        }
        digitalWrite(STEPPER1_STEP, LOW); //activate step pulse
        S1Busy = true;
        S1PeriodCount.I = S1Period.I;
        OCR0A += 8;//to time step pulse
    }
    else
    {
        S1Busy = 0;
    }
}

//This interrupt will not occur if SteppersInterlocked is true.
//Instead, stepper 2 will be controlled in an interlocked fashion by
//TIMER0_COMPA ISR above.
ISR(TIMER0_COMPB_vect)
{
    if(S2Busy)
    {
        digitalWrite(STEPPER2_STEP, HIGH); //inactive
        if(S2PeriodCount.B[1] != 0)
        {
            --S2PeriodCount.B[1];
            return;  //leaving OCR0B alone causes full timer cycle or 1024 USec
        }
        if(S2PeriodCount.B[0] != 0)//short period portion
        {
            OCR0B += S2PeriodCount.B[0]; //so we time fractional part (or whole cycle if fractional part is zero)
            S2PeriodCount.B[0] = 0; //clear the short period part
            return;
        }
        --S2StepCount;
        S2Busy = false; //we must have finished timing this step
        //and we fall through to check if more steps are required
    }

    //if we get here we are not BUSY
    if(S2StepCount)
    {
        digitalWrite(STEPPER2_STEP, LOW); //activate step pulse
        S2Busy = true;
        S2PeriodCount.I = S2Period.I;
        OCR0B += 16;//to time step pulse
    }
    else
    {
        S2Busy = 0;
    }
}

//THis really should be in linkfuns.cpp but for some reason HIGH is not defined there
U8 SetSolenoid(U8 SolIndex, U8 Value)
{
  switch (SolIndex)
  {
    default:
    case 0:
      digitalWrite(SOLENOID1_PIN, Value > 0 ? HIGH : LOW);
      break;


    case 1:
      digitalWrite(SOLENOID2_PIN, Value > 0 ? HIGH : LOW);
      break;

    case 2:
      digitalWrite(SOLENOID3_PIN, Value > 0 ? HIGH : LOW);
      break;

    case 3:
      digitalWrite(SOLENOID4_PIN, Value > 0 ? HIGH : LOW);
      break;
  }
}

bool Toggle1Flag, Toggle2Flag;

bool ActiveRange(U16 PotVal)
{

    if((PotVal > 462) && (PotVal  < 562))
    {
        return false;
    }
    return true;
}

bool PositiveDir(U16 PotVal)
{
    return (PotVal > 512);
}

U16 PotToCount(U16 PotVal)
{
    float Vel, Counts;
    int Index = PotVal & 0X3FF;
    Index -= 512;
    if(Index < 0)
    {
        Index = -Index;
    }
    Index = Index - 50;
    if(Index < 0)
    {
        return 65535; //as slow as we can go
    }
    Vel = 30 * (exp(Index/70.0) - 1) - .41;
    if(Vel < 4)
    {
        Vel = 4;
    }
    else if (Vel > 20000)
    {
        Vel = 20000;
    }
    Counts = (1/Vel)/4e-6;
    return (U16)Counts;
}


static U8 MSCounter;
static U16 S1PVal, S2PVal;
//THis is called every millisecond but does something only
//every 16 milliseconds
void ManageSteppers()
{
    U8 SaveSREG = SREG;
    //We take A/D reading on each channel every 4 milliseconds.
    //So each channel only changes every four calls. So we end
    //up adding the same value in four times for each of the two
    //channels we are using to control stepper rate. This just
    //makes coding easier. Since we only adjust steppers every
    //16 times we get an average of four readings for each.
    S1PVal += InputVals[2];
    S2PVal += InputVals[3];

    ++MSCounter;
    if(MSCounter < 16)
    {
        return;
    }
    MSCounter = 0;
    S1PVal >>= 4;  //divide averagers by 16//
    S2PVal >>= 4;
    if(S1LinkControl)
    {
        if(ActiveRange(S1PVal))//if Pot enters active range we take over here
        {
            S1LinkControl = false;
        }
    }
    else
    {
        if(PositiveDir(S1PVal))
        {
            digitalWrite(STEPPER1_DIR, LOW);
        }
        else
        {
            digitalWrite(STEPPER1_DIR, HIGH);
        }

        if(ActiveRange(S1PVal))
        {
            S1StepCount = 40000; //arbitrary value to keep it going
        }
        else
        {
            cli();
            S1Busy = false;
            S1StepCount = 0; //stop it
            SREG = SaveSREG;
        }

        S1Period.I = PotToCount(S1PVal);
    }

    if(SteppersInterlocked)
    {
        //In interlocked operation, Stepper 2 always goes same direction as Stepper 1.
        //and is stepped by Stepper 1 ISR.
        if(PositiveDir(S1PVal))
        {
            digitalWrite(STEPPER2_DIR, LOW);
        }
        else
        {
            digitalWrite(STEPPER2_DIR, HIGH);
        }
        if(ActiveRange(S2PVal))//if Pot enters active range we take over here
        {
            SetSteppersInterlocked(false);
        }

    }
    else
    {
        if(S2LinkControl)
        {
            if(ActiveRange(S2PVal))//if Pot enters active range we take over here
            {
                S2LinkControl = false;
            }
        }
        else
        {
            if(!ActiveRange(S2PVal))//if Pot enters inactive range and not in S2LinkControl
            {                       //we return to interlocked mode
                SetSteppersInterlocked(true);
            }
            if(PositiveDir(S2PVal))
            {
                digitalWrite(STEPPER2_DIR, LOW);
            }
            else
            {
                digitalWrite(STEPPER2_DIR, HIGH);
            }

            if(ActiveRange(S2PVal))
            {
                S2StepCount = 40000; //arbitrary value to keep it going
            }
            else
            {
                cli();
                S2Busy = false;
                S2StepCount = 0; //stop it
                SREG = SaveSREG;
            }

            S2Period.I = PotToCount(S2PVal);
        }
    }
    S1PVal = 0; //and reset the averagers since we just used them
    S2PVal = 0;
}

//Originally only supporting stepper indices of 0 or 1. Then
//added 0X2 flag to StepperIndex to indicate desire to only set period
void SetStep(U8 StepperIndex, U16 PeriodCount, U16 Steps, U8 Dir)
{
    bool SetCount = (StepperIndex & 2) == 0;
    StepperIndex &= 1;//limit to zero or 1
    PeriodCount = PeriodCount < 10 ? 10 : PeriodCount; //allow 10 thru 65535
    if(StepperIndex)
    {
        S2LinkControl = true;
        TIMSK0 &= ~_BV(OCIE0B);
        S2Period.I = PeriodCount;
        if(SetCount)
        {
            digitalWrite(STEPPER2_DIR, Dir > 0 ? HIGH : LOW);
            if(Steps == 0)
            {
                S2Busy = false;
            }
            S2StepCount = Steps;
        }
        TIMSK0 |= _BV(OCIE0B);
    }
    else
    {
        S1LinkControl = true;
        TIMSK0 &= ~_BV(OCIE0A);
        S1Period.I = PeriodCount;
        if(SetCount)
        {
            digitalWrite(STEPPER1_DIR, Dir > 0 ? HIGH : LOW);
            if(Steps == 0)
            {
                S1Busy = false;
            }
            S1StepCount = Steps;
        }
        TIMSK0 |= _BV(OCIE0A);
    }
}

void SetSteppersInterlocked(bool InterlockedMode)
{
    if(InterlockedMode)
    {
        SteppersInterlocked = true;
        TIMSK0 &= ~_BV(OCIE0B);
    }
    else
    {
        TIMSK0 |= _BV(OCIE0B);
        SteppersInterlocked = false;
    }
}
