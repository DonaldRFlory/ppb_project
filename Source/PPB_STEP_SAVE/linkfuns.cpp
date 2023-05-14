#include <Servo.h>
#include "type.h"
#include "ppb.h"
#include "slvmainll.h"
#include "slink.h"
#include "slavparm.h"

void SetSteppersInterlocked(bool InterlockedMode);
extern bool SteppersInterlocked;

U8 SetServoUsec(U8 ServoIndex, U16 USec);
//DFDEBUG
void TurnLEDOn();

extern U16 InputVals[MAX_INPUT_VALS];

//This is a link function implementing the Slave side of
//the host block transfer function:
//U8 DataUpdate(DOWN_PTR_U16 USecsPtr, U8 USecCount, UP_PTR_U16 ADValsPtr, U8 ADValCount);
//We are prepared here to accept up to 6 servo microsecond values, and send back
//up to five input values. The first four input values hold most recent A/D values for the
//four reflective sensor A/D inputs. The fifth input value is 1 if Sw1
//is closed and 0 if Sw1 is open.
U8 DataUpdate(U8 USecCount, U8 ValCount)
{
    //Note: This will enable servos if called!
    U16 USec;
    USecCount = USecCount > MAX_OUTPUT_VALS ? MAX_OUTPUT_VALS : USecCount; //limit to MAX_OUTPUT_VALS
    ValCount = ValCount > MAX_INPUT_VALS ? MAX_INPUT_VALS : ValCount; //limit to MAX_INPUT_VALS
    for(int i =0; i < USecCount; ++i)
    {
        RcvU16(&(USec));
        //SetServoUsec(i, USec);
    }
    for(int i =0; i < ValCount; ++i)
    {
        SendU16(InputVals[i]);
    }

    return 0X1;
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
            return SteppersInterlocked ? 1 : 0;

        default:
            return 0;
    }
}

U32 SetSlaveParameter(U8 ParCode, U16 Param1, U16 Param2)
{
    switch (ParCode)
    {
        case SPAR_STEPPERS_INTERLOCKED:
            SetSteppersInterlocked(Param1 != 0 ? true : false);
            return 0;

        default:
            return 0;
    }
}

//THis seems to be some test code to experiment with ramping the value for Servo4 on the Arduino to get
//smoother motion of attached servo than I can get by incrementing position on Windows C# test appllication,
//and sending positions to Arduino. The link calls to Arduino are somewhat slow.
//The test setup just ramps Servo4 from minimum to maximum USec values with 4 Msec interval
//between updates.
//---------------------------------------------------------------
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
