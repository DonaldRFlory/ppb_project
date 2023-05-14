#include "type.h"
#include "ppb.h"
#include <Wire.h>

extern U8 StrBuffer[64];
extern void WriteTraceBuffer(U8* InBuff, U16 Count);
extern U16 UsecVals[NUM_SERVO_GROUPS * SERVO_GP_SIZE];
U8 SetServoUsec(U8 ServoIndex, U16 Usec); //in linkfuns.cpp
U16 ServoTimer;
#define CS_RANDOM 0
#define CS_RIPPLE1 1
#define CS_RIPPLE2 2
#define CS_RIPPLE3 3
#define CS_RIPPLE4 4
#define CS_RIPPLE5 5
#define CS_RIPPLE6 6
#define CS_RIPPLE7 7
#define CS_RIPPLE8 8

#define CS_NO_INIT 0
#define CS_INIT1    1
static U8 CycleState;

bool CSRandomServe(U8 InitCtrl = CS_NO_INIT);
bool Ripple1Serve(U8 InitCtrl = CS_NO_INIT);
bool Ripple2Serve(U8 InitCtrl = CS_NO_INIT);
bool Ripple3Serve(U8 InitCtrl = CS_NO_INIT);
bool Ripple4Serve(U8 InitCtrl = CS_NO_INIT);
bool Ripple5Serve(U8 InitCtrl = CS_NO_INIT);
bool Ripple6Serve(U8 InitCtrl = CS_NO_INIT);
bool Ripple7Serve(U8 InitCtrl = CS_NO_INIT);
bool Ripple8Serve(U8 InitCtrl = CS_NO_INIT);
int CycleServeDelay = 0;

static U8 CSIteration;
const U8 RowMap[2][NUM_COLS] =
{
 { GP1_BASE_INDEX + 0, GP1_BASE_INDEX + 1, GP1_BASE_INDEX + 2, GP1_BASE_INDEX + 3,
   GP1_BASE_INDEX + 4, GP1_BASE_INDEX + 5, GP1_BASE_INDEX + 6, GP1_BASE_INDEX + 7,
   GP1_BASE_INDEX + 16, GP1_BASE_INDEX + 17, GP1_BASE_INDEX + 18, GP1_BASE_INDEX + 19,
   GP1_BASE_INDEX + 20, GP1_BASE_INDEX + 21, GP1_BASE_INDEX + 22, GP1_BASE_INDEX + 23
 },
 { GP1_BASE_INDEX + 8, GP1_BASE_INDEX + 9, GP1_BASE_INDEX + 10, GP1_BASE_INDEX + 11,
   GP1_BASE_INDEX + 12, GP1_BASE_INDEX + 13, GP1_BASE_INDEX + 14, GP1_BASE_INDEX + 15,
   GP1_BASE_INDEX + 24, GP1_BASE_INDEX + 25, GP1_BASE_INDEX + 26, GP1_BASE_INDEX + 27,
   GP1_BASE_INDEX + 28, GP1_BASE_INDEX + 29, GP1_BASE_INDEX + 30, GP1_BASE_INDEX + 31}
};

bool CSRandomServe(U8 InitCtrl)
{
    long RanVal;

    if(InitCtrl)
    {
        CSIteration = 0;
        CycleState = CS_RANDOM;
        return true;
    }
    if(++ServoTimer == 100)
    {
      for(int i = 16; i < 24; ++i)
      {
          RanVal = random(500, 2500);
          UsecVals[i] = (U16)RanVal | 0X8000;
      }
    }
    else if (ServoTimer == 200)
    {
      for(int i = 24; i < 32; ++i)
      {
        RanVal = random(500, 2500);
        UsecVals[i] = (U16)RanVal | 0X8000;
      }
    }
    else if (ServoTimer == 300)
    {
      for(int i = 32; i < 40; ++i)
      {
        RanVal = random(500, 2500);
        UsecVals[i] = (U16)RanVal | 0X8000;
      }
    }
    else if (ServoTimer >= 400)
    {
      for(int i = 40; i < 48; ++i)
      {
        RanVal = random(500, 2500);
        UsecVals[i] = (U16)RanVal | 0X8000;
      }
      ServoTimer = 0;
      if(++CSIteration >= 3)
      {
        return false;
      }
      else
      {
        return true;
      }
    }
    return true;
}

static U8 RipIdx1, RipIdx2;
static int ColIdx;
//goes from min to max setting all max.
bool Ripple1Serve(U8 InitCtrl)
{
    if(InitCtrl)
    {
        RipIdx1 = MIN_INDEX;
        CycleState = CS_RIPPLE1;
        return true;
    }
    UsecVals[RipIdx1++] = 2500 | 0X8000;
    return (RipIdx1 <=  MAX_INDEX);
}

//goes from max to min setting all to min
bool Ripple2Serve(U8 InitCtrl)
{
    if(InitCtrl)
    {
        RipIdx1 = MAX_INDEX;
        CycleState = CS_RIPPLE2;
        return true;
    }
    UsecVals[RipIdx1] = 500 | 0X8000;
    --RipIdx1;
    return (RipIdx1 >  MIN_INDEX);
}


//goes from min to max setting all to alternate min max. Odd = max
bool Ripple3Serve(U8 InitCtrl)
{
    if(InitCtrl)
    {
        RipIdx1 = MIN_INDEX;
        CycleState = CS_RIPPLE3;
        return true;
    }
    if(RipIdx1 & 1)
    { //odd
        UsecVals[RipIdx1] = 2500 | 0X8000;
    }
    else
    { //even
        UsecVals[RipIdx1] = 500 | 0X8000;
    }
    ++RipIdx1;
    return (RipIdx1 <=  MAX_INDEX);
}


//goes from middle to end setting all to alternate min max.Odd = min
//working in both directions at once.
bool Ripple4Serve(U8 InitCtrl)
{
    //sprintf(StrBuffer, "4");
    //WriteTraceBuffer(StrBuffer, strlen(StrBuffer));
    if(InitCtrl)
    {
        RipIdx1 = MID_INDEX - 1;
        RipIdx2 = MID_INDEX;
        CycleState = CS_RIPPLE4;
        return true;
    }
    if(RipIdx1 & 1)
    { //odd
        UsecVals[RipIdx1] = 500 | 0X8000;
    }
    else
    { //even
        UsecVals[RipIdx1] = 2500 | 0X8000;
    }
    if(RipIdx2 & 1)
    { //odd
        UsecVals[RipIdx2] = 500 | 0X8000;
    }
    else
    { //even
        UsecVals[RipIdx2] = 2500 | 0X8000;
    }
    --RipIdx1;
    ++RipIdx2;
    return (RipIdx1 >  MIN_INDEX);
}

//Goes from bottom to top setting both rows at once to max
bool Ripple5Serve(U8 InitCtrl)
{
    if(InitCtrl)
    {
        ColIdx = 0;
        CycleState = CS_RIPPLE5;
        return true;
    }
    if(ColIdx >= NUM_COLS)
    {
        return false;
    }
    UsecVals[RowMap[0][ColIdx]] = 2500 | 0X8000;
    UsecVals[RowMap[1][ColIdx]] = 2500 | 0X8000;
    ++ColIdx;
    return true;
}

//Goes from top to bottom setting both rows at once to min
bool Ripple6Serve(U8 InitCtrl)
{
    if(InitCtrl)
    {
        ColIdx = NUM_COLS - 1;
        CycleState = CS_RIPPLE6;
        return true;
    }
    if(ColIdx < 0)
    {
        return false;
    }
    UsecVals[RowMap[0][ColIdx]] = 500 | 0X8000;
    UsecVals[RowMap[1][ColIdx]] = 500 | 0X8000;
    --ColIdx;
    return true;
}

//Goes from top and bottom to middle setting both rows at once to max
bool Ripple7Serve(U8 InitCtrl)
{
    if(InitCtrl)
    {
        ColIdx = 0;
        CycleState = CS_RIPPLE7;
        return true;
    }
    if(ColIdx >= (NUM_COLS/2))
    {
        return false;
    }
    UsecVals[RowMap[0][ColIdx]] = 2500 | 0X8000;
    UsecVals[RowMap[1][ColIdx]] = 2500 | 0X8000;
    UsecVals[RowMap[0][NUM_COLS - 1 - ColIdx]] = 2500 | 0X8000;
    UsecVals[RowMap[1][NUM_COLS - 1 - ColIdx]] = 2500 | 0X8000;
    ++ColIdx;
    return true;
}

//Goes from middle to top and bottom setting both rows at once to min
bool Ripple8Serve(U8 InitCtrl)
{
    if(InitCtrl)
    {
        ColIdx = NUM_COLS/2;
        CycleState = CS_RIPPLE8;
        return true;
    }
    if(ColIdx < 0)
    {
        return false;
    }
    UsecVals[RowMap[0][ColIdx]] = 500 | 0X8000;
    UsecVals[RowMap[1][ColIdx]] = 500 | 0X8000;
    UsecVals[RowMap[0][NUM_COLS - 1 - ColIdx]] = 500 | 0X8000;
    UsecVals[RowMap[1][NUM_COLS - 1 - ColIdx]] = 500 | 0X8000;
    --ColIdx;
    return true;
}



void ServoCycleServe()
{
    if(CycleServeDelay)
    {
        --CycleServeDelay;
        return;
    }
    switch(CycleState)
    {
        default:
            CycleState = CS_RANDOM;
            break;

        case CS_RANDOM:
            if(!CSRandomServe())
            {
                Ripple1Serve(CS_INIT1);
            }
            break;

        case CS_RIPPLE1:
            if(++ServoTimer < 50)
                break;
            ServoTimer = 0;
            if(!Ripple1Serve())
            {
                Ripple2Serve(CS_INIT1);
            }
            break;

        case CS_RIPPLE2:
            if(++ServoTimer < 50)
                break;
            ServoTimer = 0;
            if(!Ripple2Serve())
            {
                Ripple3Serve(CS_INIT1);
            }
            break;

        case CS_RIPPLE3:
            if(++ServoTimer < 50)
                break;
            ServoTimer = 0;
            if(!Ripple3Serve())
            {
                Ripple4Serve(CS_INIT1);
            }
            break;

        case CS_RIPPLE4:
            if(++ServoTimer < 50)
                break;
            ServoTimer = 0;
            if(!Ripple4Serve())
            {
                CSIteration = 0;
                Ripple5Serve(CS_INIT1);
            }
            break;

        case CS_RIPPLE5:
            if(++ServoTimer < 50)
                break;
            ServoTimer = 0;
            if(!Ripple5Serve())
            {
                Ripple6Serve(CS_INIT1);
            }
            break;

        case CS_RIPPLE6:
            if(++ServoTimer < 50)
                break;
            ServoTimer = 0;
            if(!Ripple6Serve())
            {
                if(++CSIteration < 4)
                {
                    Ripple5Serve(CS_INIT1);
                }
                else
                {
                    CSIteration = 0;
                    Ripple7Serve(CS_INIT1);
                }
            }
            break;

        case CS_RIPPLE7:
            if(++ServoTimer < 50)
                break;
            ServoTimer = 0;
            if(!Ripple7Serve())
            {                     100;
                Ripple8Serve(CS_INIT1);
            }
        break;

        case CS_RIPPLE8:
           if(++ServoTimer < 50)
               break;
           ServoTimer = 0;
           if(!Ripple8Serve())
           {
               if(++CSIteration < 4)
               {
                   CycleServeDelay = 100;
                   Ripple7Serve(CS_INIT1);
               }
               else
               {
                   CSRandomServe(CS_INIT1);
               }
           }
        break;
    }
}

//We are assuming that if all extended servo groups
//are not fully populated, we will fully populate
//lower groups before starting to populate any higher
//groups.
U8 NextIndex(U8 Index)
{
    ++Index;
    if(Index >= (GP1_BASE_INDEX + (NUM_GP1_SERVOS + NUM_GP2_SERVOS + NUM_GP3_SERVOS)) )
    {
        Index = GP1_BASE_INDEX;
    }
    return Index;
}

//This function is called every other millisecond and if any values need updating,
//writes at most one value to one of the extended servos controlled by PCA9685
//boards. Keeps track of last servo updated and works through servos in order.
U8 NextUpdateIndex = GP1_BASE_INDEX;
void UpdateServo()
{
    U8 StartIndex, Index;
    Index = StartIndex = NextUpdateIndex;
    do
    {
        if((UsecVals[Index] & 0X8000) > 0)
        {
            UsecVals[Index] &= ~0X8000; //clear the update flag
            SetServoUsec(Index, UsecVals[Index]);
             NextUpdateIndex = NextIndex(Index);
            return;
        }
        Index = NextIndex(Index);
    }while (Index != StartIndex);
}
