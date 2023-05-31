#include "project.h"
#include <math.h>
bool SegmentSwitch;
extern U8 StrBuffer[64];
void WriteTraceBuffer(U8* InBuff, U16 Count);
int XSteps, XStepsSave, YSteps, YStepsSave;
void RotServe();

U8 ResetStepPos(U8 Index);
STEP_CTRL StepCtrl[MAX_STEPPERS];


//Steppers 2 and 3 only work in link control mode.

void InitializeRamp(U8 SI, U16 MoveSteps, U16 PeriodCount, bool RampActive);
extern U16 InputVals[MAX_INPUT_VALS];

bool TickFlag;

U8 ResetStepPos(U8 Index)
{
    if(Index <= 3)
    {
        StepCtrl[Index].StepPos = 0;
        return 1;
    }
    return 0;
}

//Radius is in steps
//We will require that radius is at least 5 steps. Then circumference is at least 31.
//We will use 100 segments if we can. That means for radius 16 and above we have 100 segments,
//starting with a repeat of 1 at 16. Then as  radius increases, we get more repeats until repeats
//increases to 255. Above that point, we increase segments to keep repeats at 255.
#define MIN_ROT_RADIUS 5
U8 SetRotRadius(U16 Radius)
{
}

void InitStepping()
{
    for(int i = 0; i < MAX_STEPPERS; ++i)
    {
        StepCtrl[i].StepCount = 0;
    }
}

ISR(TIMER0_COMPA_vect)
{
    if(StepCtrl[0].Busy)
    {
        if(StepCtrl[0].FullCounts != 0)
        {
            --StepCtrl[0].FullCounts;
            if(StepCtrl[0].AdjustFlag)
            {
                OCR0A += 128; //we want half cycle due to counts we added
                              //to the low period byte count
                StepCtrl[0].AdjustFlag = false;
            }
            return;  //leaving OCR0A alone causes full timer cycle or 1024 USec
        }
        StepCtrl[0].Busy = false; //we must have finished timing this step
        //and we fall through to check if more steps are required
    }
    //if we get here we are not BUSY
    if(StepCtrl[0].StepCount)
    {
        SET_STEP1();
        if(StepCtrl[0].Dir)
        {
            ++StepCtrl[0].StepPos;
        }
        else
        {
            --StepCtrl[0].StepPos;
        }
        if(StepCtrl[0].NewPeriod)
        {   //need to load next ramp period value
            StepCtrl[0].NewPeriod = false;
            StepCtrl[0].Period.I = StepCtrl[0].NextPeriod.I;
        }
        --StepCtrl[0].StepCount;
        StepCtrl[0].Busy = true;
        StepCtrl[0].FullCounts = StepCtrl[0].Period.B[1];
        OCR0A += StepCtrl[0].Period.B[0];
        if(StepCtrl[0].Period.B[0] < MIN_STEP_PERIOD_COUNT)
        {
            OCR0A += 128;
            StepCtrl[0].AdjustFlag = true;
        }
        else
        {
            StepCtrl[0].AdjustFlag = false;
        }
        CLR_STEP1();
        CLR_STEP2();
    }
    else
    {
        TIMSK0 &= ~_BV(OCIE0A);
        StepCtrl[0].Busy = false;
    }
}

ISR(TIMER0_COMPB_vect)
{
    if(StepCtrl[1].Busy)
    {
        if(StepCtrl[1].FullCounts != 0)
        {
            --StepCtrl[1].FullCounts;
            if(StepCtrl[1].AdjustFlag)
            {
                OCR0B += 128; //we want half cycle due to counts we added
                              //to the low period byte count
                StepCtrl[1].AdjustFlag = false;
            }
            return;  //leaving OCR0B alone causes full timer cycle or 1024 USec
        }
        StepCtrl[1].Busy = false; //we must have finished timing this step
        //and we fall through to check if more steps are required
    }

    //if we get here we are not BUSY
    if(StepCtrl[1].StepCount)
    {
        SET_STEP2();
        if(StepCtrl[1].Dir)
        {
            ++StepCtrl[1].StepPos;
        }
        else
        {
            --StepCtrl[1].StepPos;
        }
        if(StepCtrl[1].NewPeriod)
        {   //need to load next ramp period value
            StepCtrl[1].NewPeriod = false;
            StepCtrl[1].Period.I = StepCtrl[1].NextPeriod.I;
        }
        --StepCtrl[1].StepCount;
        StepCtrl[1].Busy = true;
        StepCtrl[1].FullCounts = StepCtrl[1].Period.B[1];
        OCR0B += StepCtrl[1].Period.B[0];
        if(StepCtrl[1].Period.B[0] < MIN_STEP_PERIOD_COUNT)
        {
            OCR0B += 128;
            StepCtrl[1].AdjustFlag = true;
        }
        else
        {
            StepCtrl[1].AdjustFlag = false;
        }
        CLR_STEP2();
    }
    else
    {
        TIMSK0 &= ~_BV(OCIE0B);
        StepCtrl[1].Busy = false;
    }
}

bool ISRToggleFlag;
U16 T2Count;

ISR(TIMER2_OVF_vect)
{
    ++T2Count;

    TickFlag = true;
}


 ISR(TIMER2_COMPA_vect)
{
    if(StepCtrl[2].Busy)
    {
        if(StepCtrl[2].FullCounts != 0)
        {
            --StepCtrl[2].FullCounts;
            if(StepCtrl[2].AdjustFlag)
            {
                OCR2A += 128; //we want half cycle due to counts we added
                              //to the low period byte count
                StepCtrl[2].AdjustFlag = false;
            }
            return;  //leaving OCR2A alone causes full timer cycle or 1024 USec
        }
        StepCtrl[2].Busy = false; //we must have finished timing this step
        //and we fall through to check if more steps are required
    }

    //if we get here we are not BUSY
    if(StepCtrl[2].StepCount)
    {
        SET_STEP3();
        if(StepCtrl[2].Dir)
        {
            ++StepCtrl[2].StepPos;
        }
        else
        {
            --StepCtrl[2].StepPos;
        }
        if(StepCtrl[2].NewPeriod)
        {   //need to load next ramp period value
            StepCtrl[2].Period.I = StepCtrl[2].NextPeriod.I;
            StepCtrl[2].NewPeriod = false;
        }
        --StepCtrl[2].StepCount;
        StepCtrl[2].Busy = true;
        StepCtrl[2].FullCounts = StepCtrl[2].Period.B[1];
        OCR2A += StepCtrl[2].Period.B[0];
        if(StepCtrl[2].Period.B[0] < MIN_STEP_PERIOD_COUNT)
        {
            OCR2A += 128;
            StepCtrl[2].AdjustFlag = true;
        }
        else
        {
            StepCtrl[2].AdjustFlag = false;
        }
        CLR_STEP3();
    }
    else
    {
        TIMSK2 &= ~ _BV(OCIE2A);    //disable output compare A on timer 2
        StepCtrl[2].Busy = false;
    }
}

ISR(TIMER2_COMPB_vect)
{
    if(StepCtrl[3].Busy)
    {
        if(StepCtrl[3].FullCounts != 0)
        {
            --StepCtrl[3].FullCounts;
            if(StepCtrl[3].AdjustFlag)
            {
                OCR2B += 128; //we want half cycle due to counts we added
                              //to the low period byte count
                StepCtrl[3].AdjustFlag = false;
            }
            return;  //leaving OCR2B alone causes full timer cycle or 1024 USec
        }
        StepCtrl[3].Busy = false; //we must have finished timing this step
        //and we fall through to check if more steps are required
    }

    //if we get here we are not BUSY
    if(StepCtrl[3].StepCount)
    {
        SET_STEP4();
        if(StepCtrl[3].Dir)
        {
            ++StepCtrl[3].StepPos;
        }
        else
        {
            --StepCtrl[3].StepPos;
        }
        if(StepCtrl[3].NewPeriod)
        {   //need to load next ramp period value
            StepCtrl[3].NewPeriod = false;
            StepCtrl[3].Period.I = StepCtrl[3].NextPeriod.I;
        }
        --StepCtrl[3].StepCount;
        StepCtrl[3].Busy = true;
        StepCtrl[3].FullCounts = StepCtrl[3].Period.B[1];
        OCR2B += StepCtrl[3].Period.B[0];
        if(StepCtrl[3].Period.B[0] < MIN_STEP_PERIOD_COUNT)
        {
            OCR2B += 128;
            StepCtrl[3].AdjustFlag = true;
        }
        else
        {
            StepCtrl[3].AdjustFlag = false;
        }
        CLR_STEP4();
    }
    else
    {
        TIMSK2 &= ~ _BV(OCIE2B);    //disable output compare B on timer 2
        StepCtrl[3].Busy = false;
    }
}

//AD notes:
//one of four AD channels is converted each millisecond
//in a four millisecond round robin sequence.
//The AD CHannels uses are A0, A1, A6, and A7, stored
//respectively in InputVals[0]-InputVals[3].
//Used A6 and A7 since on nano, they are only usaable for A/D
//A2-A5 can also be used for digital I/O.


U8 CyclicMoveMask;

float RVMin, RVMax, RAccel;
//In a bit of a kludge, we are going to require calling SetRampParams() function
//to set non-standard VStart and Accel values
void SetRampParams(float VMin, float VMax, float Accel)
{
    RVMin = VMin;
    RVMax = VMax;
    RAccel = Accel;
}

//Originally only supporting stepper indices of 0 or 1. Then
//added 0X10 flag to StepperIndex to indicate desire to only set period
//added 0X20 flag to StepperIndex to indicate desire to do ramped move
//added 0X40 flag to StepperIndex to indicate desire to do cyclic move
//When a stepcount is loaded to a stepper, the stepper interrupt is disabled.
//Interrupt is enabled when the stepper is served in the msec serve in it's
//four msec time slot.
void SetStep(U8 StepperIndex, U16 PeriodCount, U16 Steps, U8 Flags)
{
    bool Dir;
    sprintf(StrBuffer, "SetStep Idx = %d Flags = %d\n", StepperIndex, Flags);
    WriteTraceBuffer(StrBuffer, strlen(StrBuffer));
    StepperIndex &= 3;//limit to zero through three
    Dir = (Flags & SS_POS_DIR) != 0;

    PeriodCount = PeriodCount < MIN_STEP_PERIOD_COUNT ? MIN_STEP_PERIOD_COUNT : PeriodCount; //allow 10 thru 65535
    switch(StepperIndex)
    {
        case 0:  //S1
            StepCtrl[0].Dir = Dir;
            InitializeRamp(0, Steps, PeriodCount, (Flags & SS_DO_RAMP) != 0);
            TIMSK0 &= ~_BV(OCIE0A);
            digitalWrite(STEPPER1_DIR, Dir ? HIGH : LOW);
            StepCtrl[0].Busy = false;
            StepCtrl[0].StepCount = Steps;
            break;

        case 1:  //S2
            StepCtrl[1].Dir = Dir;
            InitializeRamp(1, Steps, PeriodCount, (Flags & SS_DO_RAMP) != 0);
            TIMSK0 &= ~_BV(OCIE0B);
            digitalWrite(STEPPER2_DIR, Dir ? HIGH : LOW);
            if(Steps == 0)
            {
                StepCtrl[1].Busy = false;
            }
            StepCtrl[1].StepCount = Steps;
            break;

        case 2:  //S3
            StepCtrl[2].Dir = Dir;
            InitializeRamp(2, Steps, PeriodCount, (Flags & SS_DO_RAMP) != 0);
            TIMSK2 &= ~ _BV(OCIE2A);    //disable output compare A on timer 2
            digitalWrite(STEPPER3_DIR, Dir ? HIGH : LOW);
            StepCtrl[2].Busy = false;
            StepCtrl[2].StepCount = Steps;
            break;

        case 3:  //S4
            StepCtrl[3].Dir = Dir;
            InitializeRamp(3, Steps, PeriodCount, (Flags & SS_DO_RAMP) != 0);
            TIMSK2 &= ~ _BV(OCIE2B);    //disable output compare B on timer 2
            digitalWrite(STEPPER4_DIR, Dir ? HIGH : LOW);
            StepCtrl[3].Busy = false;
            StepCtrl[3].StepCount = Steps;
            break;
    }
}

//Ramp is called for each stepper once every 4 millisecond in staggered fashion so one is called each millisecond
void Ramp(U8 SI)
{
    u16 StepCount;
    switch(StepCtrl[SI].RampState)
    {
        case RAMP_IDLE:
        default:
            break;

        case RAMP_UP:
            noInterrupts();
            StepCount = StepCtrl[SI].StepCount;
            interrupts();

            //Intentionally not monitoring for velocity overshoot and undershoot. Assuming ramp setup and Infl1
            //take care of that.
            if(StepCount > StepCtrl[SI].Infl1)
            {
                StepCtrl[SI].Vel += StepCtrl[SI].VIncr;//ramp velocity up
                StepCtrl[SI].NextPeriod.I = (U16) ((1/StepCtrl[SI].Vel)/PER_CNT_INTRV);
                StepCtrl[SI].NewPeriod = true;
                break;
            }
            if(StepCount <= StepCtrl[SI].Infl2)
            {
                StepCtrl[SI].RampState = RAMP_DOWN;
            }
            else
            {
                StepCtrl[SI].RampState = RAMP_LIM;
            }
            break;

        case RAMP_LIM:
            noInterrupts();
            StepCount = StepCtrl[SI].StepCount;
            interrupts();

            if(StepCount <= StepCtrl[SI].Infl2)
            {
                StepCtrl[SI].RampState = RAMP_DOWN;
            }
            break;

        case RAMP_DOWN:
            noInterrupts();
            StepCount = StepCtrl[SI].StepCount;
            interrupts();

            if(StepCount)
            {
                StepCtrl[SI].Vel -= StepCtrl[SI].VIncr;//ramp velocity down
                StepCtrl[SI].NextPeriod.I = (U16) ((1/StepCtrl[SI].Vel)/PER_CNT_INTRV);
                StepCtrl[SI].NewPeriod = true;
            }
            else
            {
                StepCtrl[SI].RampState = RAMP_IDLE;
            }
            break;
    }
}

//float RVMin, RVMax, RAccel;
//Must be called before move is started
void InitializeRamp(U8 SI, U16 MoveSteps, U16 PeriodCount, bool RampActive)
{
    float TCrit, SCrit;
    if(!RampActive)
    {
        StepCtrl[SI].Period.I = PeriodCount;
        StepCtrl[SI].Infl1 = StepCtrl[SI].Infl2 = 0;
        StepCtrl[SI].NewPeriod = false;
        StepCtrl[SI].RampState = RAMP_IDLE;
        return;
    }

    TCrit = ((float)RVMax - RVMin)/RAccel;
    SCrit = (RVMin * TCrit) + (RAccel * TCrit * TCrit / 2);
    if(MoveSteps > 2*SCrit)
    { //have plateau
        StepCtrl[SI].Infl1 = MoveSteps - (U16)SCrit;  //inflection steps are values of StepCount
        StepCtrl[SI].Infl2 = (U16)SCrit;              //which counts down from MoveSteps
    }
    else
    {
        StepCtrl[SI].Infl1 = StepCtrl[SI].Infl2 = MoveSteps/2;
    }

    StepCtrl[SI].RampState = RAMP_UP;
    StepCtrl[SI].Vel = RVMin;
    StepCtrl[SI].Period.I = (U16) ((1.0/RVMin)/PER_CNT_INTRV);
    StepCtrl[SI].VIncr = RAccel * RAMP_TIME_INCR;
    StepCtrl[SI].NewPeriod = false;
}

U8 Limit(U8 SI)
{
    switch(SI)
    {
       case 0:
       default:
           return LIMIT1();
           break;

       case 1:
           return LIMIT2();
           break;

       case 2:
           return LIMIT3();
           break;

       case 3:
           return LIMIT4();
           break;
    }
}

U16 MaxSteps[4] =
{
    MAX_S1_STEPS,
    MAX_S2_STEPS,
    MAX_S3_STEPS,
    MAX_S4_STEPS
};


U8 CMDelay[4];

U8 HomeStepper(U8 Index)
{
    if(Index < 3)
    {
        StepCtrl[Index].CycleState = HOME_INIT;
        return 1;
    }
    return 0;
}

U8 CycleStepper(U8 Index, bool Enable)
{
    if(Index < 3)
    {
        if(Enable)
        {
            StepCtrl[Index].CycleState = CYCLE_START;
        }
        else
        {
            StepCtrl[Index].CycleState = CYCLE_IDLE;
        }
        return 1;
    }
    return 0;
}

//This is called every millisec to manage any cyclic moves enabled for steppers.
//(Called for each stepper every four milliseconds in round robin fashion)
 void  DoCycleMove(U8 SI)
 {
     if(CMDelay[SI])
     {
         --CMDelay[SI];
         return;
     }

     switch(StepCtrl[SI].CycleState)
     {
        case CYCLE_IDLE:
            break;

        case HOME_INIT:
            if(Limit(SI))
            {
                SetStep(SI, MIN_PERIOD, BACKOUT_STEPS, SS_POS_DIR);
                StepCtrl[SI].CycleState = HOME_BACKOUT;
                CMDelay[SI] = 50;
            }
            else
            {
                SetStep(SI, MIN_PERIOD, MaxSteps[SI] + BACKOUT_STEPS, 0);
                StepCtrl[SI].CycleState = HOME_HOME;
                CMDelay[SI] = 50;
            }
            break;

        case HOME_BACKOUT:
            SetStep(SI, MIN_PERIOD, 2 * BACKOUT_STEPS, 0);
            StepCtrl[SI].CycleState = HOME_HOME;
            CMDelay[SI] = 50;
            break;



        case HOME_HOME:
            StepCtrl[SI].CycleState = HOME_BACKOFF;
            SetStep(SI, MIN_PERIOD, BACKOFF_STEPS, SS_POS_DIR);
            break;

        case HOME_WAIT1:
            StepCtrl[SI].CycleState = CYCLE_IDLE;
            break;

        case HOME_BACKOFF:
            StepCtrl[SI].CycleState = CYCLE_IDLE;
            break;

        case CYCLE_START:
            SetStep(SI, MIN_PERIOD, MaxSteps[SI], SS_DO_RAMP + SS_POS_DIR);
            StepCtrl[SI].CycleState = CYCLE_POS;
            CMDelay[SI] = 50;
            break;

        case CYCLE_POS:
            SetStep(SI, MIN_PERIOD, MaxSteps[SI], SS_DO_RAMP);
            StepCtrl[SI].CycleState = CYCLE_NEG;
            CMDelay[SI] = 50;
            break;

        case CYCLE_NEG:
            SetStep(SI, MIN_PERIOD, MaxSteps[SI], SS_DO_RAMP + SS_POS_DIR);
            StepCtrl[SI].CycleState = CYCLE_POS;
            CMDelay[SI] = 50;
            break;
     }
 }
