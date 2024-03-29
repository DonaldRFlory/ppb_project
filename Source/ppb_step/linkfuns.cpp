#include "project.h"
#include <Servo.h>
#include <EEPROM.h>
#include <stdio.h>
#include "type.h"
#include "slvmainll.h"
#include "slink.h"
#include "slavparm.h"

U8 HomeStepper(U8 Index);
U8 CycleStepper(U8 Index, bool enable);
U8 ResetStepPos(U8 Index);
#define TRACE_BUFF_LEN 512
U8 StrBuffer[64];
U8 TraceBuffer[TRACE_BUFF_LEN];
U16 TBInIndex; //where next incoming byte goes
U16 TBOutIndex; //where next byte out comes from
U16 TBCount;
//If in and out indices are equal, buffer is empty
//It can only hold 512 bytes

void WriteTraceBuffer(U8* InBuff, U16 Count)
{
    while(Count)
    {
        --Count;
        TraceBuffer[TBInIndex++] = *InBuff++;
        TBInIndex = (TBInIndex >= TRACE_BUFF_LEN) ? 0 : TBInIndex;
        if(TBCount < TRACE_BUFF_LEN)
        {
            ++TBCount;
        }
        else
        {
            ++TBOutIndex;//overwriting the oldest byte in FIFO
            TBOutIndex = (TBOutIndex >= TRACE_BUFF_LEN) ? 0 : TBOutIndex;
        }
    }
}

extern bool SwitchOn;

U8 SetServoUsec(U8 ServoIndex, U16 USec);
//DFDEBUG
void TurnLEDOn();
void LoadInputValues();

extern U16 InputVals[MAX_INPUT_VALS];

//This is a link function implementing the Slave side of
//the host block transfer function:
//U8 ReadTraceBuffer(UP_PTR_U8 TraceValsPtr, U8 TraceValCount);
//If more bytes requested than are in buffer, returns what is available.
//Returns count of bytes actually sent.
U8 ReadTraceBuffer(U8 ByteCount)
{
    U8 BytesSent = 0;
//so we send all bytes asked for if we have them, otherwise
// we pad with zeros until we have sent what they asked for
//but we return count of legit bytes.
    while(ByteCount)
    {
        if(TBCount)
        {
            ++BytesSent;
            SendU8(TraceBuffer[TBOutIndex++]);
            TBOutIndex = (TBOutIndex >= TRACE_BUFF_LEN) ? 0 : TBOutIndex;
            --TBCount;
        }
        else
        {
            SendU8(0);
        }
        --ByteCount;
    }
    return BytesSent;
}

//This is a link function implementing the Slave side of
//the host block transfer function:
//U8 DataUpdate(DOWN_PTR_U16 USecsPtr, U8 USecCount, UP_PTR_U16 ADValsPtr, U8 ADValCount);
//We are prepared here to accept up to 6 servo microsecond values, and send back
//up to five input values. Of the input values, the first four hold most recent A/D values for the
//four reflective sensor A/D inputs. The fifth input value holds various data bits:
// Bit 0  is debounced status of user pushbutton, 1 indicating closed, 0 indicating open.
// Bit 1-4 are the states of four limit sensors.
U8 DataUpdate(U8 USecCount, U8 ValCount)
{
    //Note: This will enable servos if called!
    U16 USec;
    USecCount = USecCount > MAX_OUTPUT_VALS ? MAX_OUTPUT_VALS : USecCount; //limit to MAX_OUTPUT_VALS
    ValCount = ValCount > MAX_INPUT_VALS ? MAX_INPUT_VALS : ValCount; //limit to MAX_INPUT_VALS
    LoadInputValues();
    for(int i = 0; i < USecCount; ++i)
    {
        RcvU16(&(USec));
        //SetServoUsec(i, USec);
    }
    for(int i = 0; i < ValCount; ++i)
    {
        SendU16(InputVals[i]);
    }

    return 0X1;
}


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

void ServoSlewMove(U8 ServoIndex, U16 USecTarget, float Velocity)
{
//??? I guess TO DO:
}

bool ServosInited = false; //so servos are not attached until we try to use them
            //This allows us to use servo pins for other functions in applications
            //which do not need servos.

Servo Servo1, Servo2, Servo3, Servo4, Servo5, Servo6;
int ServoPos = DEFAULT_PULSE_WIDTH;

void InitServos()
{
    Servo1.attach(SERVO1_PIN);
    Servo2.attach(SERVO2_PIN);
    Servo3.attach(SERVO3_PIN);
    Servo4.attach(SERVO4_PIN);
    Servo5.attach(SERVO5_PIN);
    Servo6.attach(SERVO6_PIN);
    Servo1.write(PP_FIRE_USEC_MIN);
    ServosInited = true;
}

U8 SetServoUsec(U8 ServoIndex, U16 USec)
{
    return;
    if(!ServosInited)
    {
        InitServos();
    }
    //TurnLEDOn();

    switch(ServoIndex)
    {
        case 0:
            Servo1.write(USec);
            break;

        case 1:
            Servo2.write(USec);
            break;

        case 2:
            Servo3.write(USec);
            break;

        case 3:
            Servo4.write(USec);
            break;

        case 4:
            Servo5.write(USec);
            break;

        case 5:
            Servo6.write(USec);
            break;
    }
    return 23;  //?? probably just to check that we were getting a return value
}

U16 GetServoUsec(U8 ServoIndex)
{
    U16 Val;
    TurnLEDOn();
    switch(ServoIndex)
    {
        case 0:
            Val = Servo1.readMicroseconds();
            break;

        case 1:
            Val =  Servo2.readMicroseconds();
            break;

        case 2:
            Val = Servo3.readMicroseconds();
            break;

        case 3:
            Val = Servo4.readMicroseconds();
            break;

        case 4:
            Val = Servo5.readMicroseconds();
            break;

        case 5:
            Val = Servo6.readMicroseconds();
            break;
    }
    return Val;
}

U8 MasterBlockDown(U8 Count, U16 DestAddress)
{

}

U8 MasterBlockUp(U16 SrcAddress, U8 Count)
{

}

U32 GetSlaveParameter(U8 ParCode, U8 Index)
{
    switch (ParCode)
    {
        case SPAR_STEPPERS_INTERLOCKED:
            return 0;

        default:
            return 0;
    }
}

U8 ParsGroup = 1;
U32 SetSlaveParameter(U8 ParCode, U16 Param1, U16 Param2)
{
    switch (ParCode)
    {
        case SPAR_STEPPERS_INTERLOCKED:
            return 0;

        case SPAR_STEPPERS_LOCAL:
            return 0;

        case SPAR_PARS1:
            ParsGroup = 1;
            return 0;

        case SPAR_PARS2:
            ParsGroup = 2;
            return 0;

        case SPAR_PARS3:
            ParsGroup = 3;
            return 0;

        case SPAR_PARS4:
            ParsGroup = 4;
            return 0;

        case SPAR_ROT_RADIUS:
            return 0;

        case SPAR_ROT_ACTIVE:
            return 0;

        case SPAR_ROT_SERVE:
            return 0;

        case SPAR_ROT_ISR:
            return 0;

        case SPAR_RESET_STEP_POS:
            return ResetStepPos((U8)Param1);

        case SPAR_HOME_STEPPER:
            return HomeStepper((U8)Param1);

        case SPAR_CYCLE_STEPPER:
            return CycleStepper((U8)Param1, true);

        case SPAR_STOP_STEPPER_CYCLE:
            return CycleStepper((U8)Param1, false);

        case SPAR_TRACE_TEST:
            sprintf(StrBuffer, "Trace buffer test: %d, %d\n", Param1, Param2);
            WriteTraceBuffer(StrBuffer, strlen(StrBuffer));
            return 0;

    }
}

//---------------------------------------------------------------
//THis seems to be some test code to experiment with ramping the value for Servo4 on the Arduino to get
//smoother motion of attached servo than I can get by incrementing position on Windows C# test appllication,
//and sending positions to Arduino. The link calls to Arduino are somewhat slow.
//The test setup just ramps Servo4 from minimum to maximum USec values with 4 Msec interval
//between updates.
#define SERVO4_MIN 1000
#define SERVO4_MAX 2000
float Servo4Increment = 100.0 / 250; //USec per tick
float Servo4Pos = 1000; //USec
//Called every 4 milliseconds
void Servo4Serve()
{
  Servo4Pos += Servo4Increment;
  if (Servo4Pos >  SERVO4_MAX)
  {
      Servo4Pos = SERVO4_MAX;
      Servo4Increment = -Servo4Increment;
  }
  else if (Servo4Pos <  SERVO4_MIN)
  {
    Servo4Pos = SERVO4_MIN;
    Servo4Increment = -Servo4Increment;
  }
  SetServoUsec(3, (U16)Servo4Pos); //Index 3 is servo4
  //Servo4.write((int)Servo4Pos);
}
//---------------------------------------------------------------

void ProgramEEPROM(U16 Address, U8 Value)
{
    EEPROM.write(Address, Value);
}

U8 ReadEEPROM(U16 Address)
{
    return EEPROM.read(Address);
}
