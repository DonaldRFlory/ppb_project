#include "type.h"
#include "slvmainll.h"
#include <servo.h>
#include "slink.h"
//DFDEBUG
void TurnLEDOn();

extern U16 ADVals[4];

extern Servo Servo1, Servo2, Servo3, Servo4;
U8 DataUpdate(U8 USecCount, U8 ADValCount)
{

    U16 USec;
    USecCount = USecCount > 4 ? 4 : USecCount; //limit to 4
    ADValCount = ADValCount > 4 ? 4 : ADValCount;//limit to 4
    for(int i =0; i < USecCount; ++i)
    {
        RcvU16(&(USec));
        SetServoUsec(i, USec);
    }
    for(int i =0; i < ADValCount; ++i)
    {
        SendU16(ADVals[i]);
    }

    return 0X1;
}


U8 SetServoUsec(U8 ServoIndex, U16 USec)
{
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
    }
    return 23;
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

}

U32 SetSlaveParameter(U8 ParCode, U16 Param1, U16 Param2)
{

}
