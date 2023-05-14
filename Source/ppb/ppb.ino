//#define STEPPER_MODE 1

#include "type.h"
#include "ppb.h"
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
Adafruit_PWMServoDriver pwm1 = Adafruit_PWMServoDriver();
Adafruit_PWMServoDriver pwm2 = Adafruit_PWMServoDriver(0X41);
#if 0
Adafruit_PWMServoDriver pwm3 = Adafruit_PWMServoDriver(0X42);
Adafruit_PWMServoDriver pwm4 = Adafruit_PWMServoDriver(0X43);
#endif

extern U8 StrBuffer[64];
extern void WriteTraceBuffer(U8* InBuff, U16 Count);
extern U16 UsecVals[NUM_SERVO_GROUPS * SERVO_GP_SIZE];
U8 SetServoUsec(U8 ServoIndex, U16 Usec); //in linkfuns.cpp
void Servo4Serve();                       //in linkfuns.cpp
extern int  LinkCount;
void LinkServe();
extern U8 LinkBuffer[];
int MsecTickCount = 0;
//bool ServoAutoUpdate = true;
bool ServoAutoUpdate = false;
bool Stepper1Dir;
int Stepper1Count = 0;
static bool MsecTick;
static U8 FireState = PPF_IDLE;
static U16 FireTimer;
//This is for a quick hack to time loading of external servo board values
//via I2C.  I will change the position of one of the servos
//every second.
void UpdateServo();

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

  #ifdef STEPPER_MODE
  pinMode(STEPPER1_DIR, OUTPUT);//Step1 direction
  //Using A4 and A5 as Digital outputs for Stepper1 and Stepper2 disable (when low (Active))
  pinMode(A4, OUTPUT);
  pinMode(A5, OUTPUT);
  digitalWrite(STEPPER1_DIR, HIGH);
  Stepper1Dir = true;
  digitalWrite(A4, HIGH);
  digitalWrite(A5, LOW);
  #else
  pinMode(AUX1, INPUT_PULLUP);
  #endif

  Serial.begin(115200);
  // Timer0 is already used for millis() - we'll just interrupt somewhere
  // in the middle and call the "Compare A" function below
  OCR0A = 0x80;
  TIMSK0 |= _BV(OCIE0A);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(SOLENOID1_PIN, LOW);
  digitalWrite(SOLENOID2_PIN, LOW);
  digitalWrite(SOLENOID3_PIN, LOW);
  digitalWrite(SOLENOID4_PIN, LOW);
  //  Serial.write("Link fun count = ");
  //  Serial.println(LinkCount);
  //Wire.setClock(400000L);//lets run I2C at 400 KHz.

  //ToDo :ultimately start pwm2, pwm3, and pwm4
  pwm1.begin();//for Adafruit servo driver using PCA9685
  pwm1.setOscillatorFrequency(25000000);
  pwm1.setPWMFreq(50);  // Analog servos run at ~50 Hz updates
  delay(10);
  pwm2.begin();//for Adafruit servo driver using PCA9685
  pwm2.setOscillatorFrequency(25000000);
  pwm2.setPWMFreq(50);  // Analog servos run at ~50 Hz updates
  delay(10);

  for(int i = GP1_BASE_INDEX; i < (GP1_BASE_INDEX + NUM_GP1_SERVOS + NUM_GP2_SERVOS); ++i)
  {
    SetServoUsec(i, 2500);
  }

  #if 0
  pwm3.begin();//for Adafruit servo driver using PCA9685
  pwm3.setOscillatorFrequency(25000000);
  pwm3.setPWMFreq(50);  // Analog servos run at ~50 Hz updates
  delay(10);
  pwm4.begin();//for Adafruit servo driver using PCA9685
  pwm4.setOscillatorFrequency(25000000);
  pwm4.setPWMFreq(50);  // Analog servos run at ~50 Hz updates
  delay(10);
  #endif
}

void DoMsecTick();

void loop() {
  // put your main code here, to run repeatedly:

  DoLink();
  if (MsecTick)
  {
    ++MsecTickCount;
    MsecTick = false;
    digitalWrite(TELLTALE_PIN, HIGH);
    DoMsecTick();
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

void Fire()
{
  FireState = PPF_SOL_ENGAGE;
  digitalWrite(SOLENOID1_PIN, HIGH);
  FireTimer = 0;
}

//Called every MSEC_COMMUTATOR_DIVIDE (4) milliseconds
void FireServe()
{
  FireTimer += MSEC_COMMUTATOR_DIVIDE;
  switch (FireState)
  {
    default:
    case PPF_IDLE:
      FireTimer = 0;
      break;

    case PPF_SOL_ENGAGE:
      if (FireTimer >= PP_SOL_ENGAGE_MSEC)
      {
        FireTimer = 0;
        FireState = PPF_SPINUP;
        //Servo1.write(PP_FIRE_USEC_MAX);//spin up the blower
        SetServoUsec(0, PP_FIRE_USEC_MAX);
      }
      break;

    case PPF_SPINUP:
      if (FireTimer >= PP_SPINUP_MSEC)
      {
        digitalWrite(SOLENOID1_PIN, LOW);
        FireTimer = 0;
        FireState = PPF_FIRE;
      }
      break;

    case PPF_FIRE:
      if (FireTimer >= PP_FIRE_MSEC)
      {
        FireTimer = 0;
        FireState = PPF_IDLE;
        //Servo1.write(PP_FIRE_USEC_MIN);//spin the blower down
        SetServoUsec(0, PP_FIRE_USEC_MIN);
      }
      break;
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
        Fire();
      }
    }
  }
}


extern void ServoCycleServe();
bool ServoCycle = false;
//We are going to call DoSteppers() at beginning of DoMsecTick()
//to activate step pulse for any active stepper whose step period has
//elapsed. We count on delay introduced by analog read to time the pulse(s)
//and will reset all pulse lines at end of DoMsecTick(). This will ensure
//minimum pulse width of 10 Usec required by stepper controllers.
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

  #ifdef STEPPER_MODE
        ++Stepper1Count;
        if(Stepper1Count >= 4000)
        {
            Stepper1Count = 0;
            Stepper1Dir = !Stepper1Dir;
            digitalWrite(STEPPER1_DIR, Stepper1Dir ? HIGH : LOW);
        }
        digitalWrite(STEPPER1_STEP, LOW);
  #endif

  switch (Commutator)
  { //one of four AD channels is converted each millisecond
    //in a four millisecond round robin sequence.
    default:
    case 0:
      UpdateServo();
      #ifdef STEPPER_MODE
            digitalWrite(STEPPER1_STEP, LOW);
      #else
        DoSwitch(); //Every MSEC_COMMUTATOR_DIVIDE (4) milliseconds
      #endif
      InputVals[0] = analogRead(A0);
      break;

    case 1:
      if(ServoCycle)
      {
        ServoCycleServe();
      }
      #ifdef STEPPER_MODE
      #else
        FireServe(); //Every MSEC_COMMUTATOR_DIVIDE (4) milliseconds
      #endif
      InputVals[1] = analogRead(A1);
      break;

    case 2:
      UpdateServo();
      InputVals[2] = analogRead(A2);
      break;

    case 3:
      InputVals[3] = analogRead(A3);
      // Ramping test. Will defeat servo pin sharing if enabled
      //Servo4Serve();
      break;
  }
  #ifdef STEPPER_MODE
  digitalWrite(STEPPER1_STEP, HIGH);
  //digitalWrite(A4, LOW);
  #endif

}

// Interrupt is called once a millisecond,
ISR(TIMER0_COMPA_vect)
{
  MsecTick = true;
}


extern U16 TBCount;
//extern U8 ParsGroup;
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
    noInterrupts();

    InputVals[TBCOUNT_IV_IDX] = TBCount;
#if 0
    switch(ParsGroup)
    {
        case 1:
        default:
            InputVals[S1STEP_IV_IDX] = (U16)RotControl.LastPoint.X;
            InputVals[S2STEP_IV_IDX] = (U16)RotControl.LastPoint.Y;
            InputVals[RAMP_INFL_IV_IDX] = (U16)RotControl.Radius;
            InputVals[S2PER_IV_IDX] = (U16)RotControl.Cycles;
            break;

        case 2:
            InputVals[S1STEP_IV_IDX] = (U16)RotSegs[0].XIncr;
            InputVals[S2STEP_IV_IDX] = (U16)RotSegs[0].YIncr;
            InputVals[RAMP_INFL_IV_IDX] = (U16)RotSegs[0].Cycles;
            InputVals[S2PER_IV_IDX] = (U16)RotSegs[0].XAccum;
            break;

        case 3:
            InputVals[S1STEP_IV_IDX] = (U16)RotSegs[1].XIncr;
            InputVals[S2STEP_IV_IDX] = (U16)RotSegs[1].YIncr;
            InputVals[RAMP_INFL_IV_IDX] = (U16)RotSegs[1].Cycles;
            InputVals[S2PER_IV_IDX] = (U16)RotSegs[1].XAccum;
            break;

        case 4:
            InputVals[S1STEP_IV_IDX] = (U16)RotSegs[0].FullFlag;
            InputVals[S2STEP_IV_IDX] = (U16)RotSegs[1].FullFlag;
            InputVals[RAMP_INFL_IV_IDX] = RotControl.RotActive;
            InputVals[S2PER_IV_IDX] = 0;
            break;

    }
#endif

    interrupts();
}
