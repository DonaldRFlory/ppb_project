#include "project.h"
#include <math.h>

//Steppers 1 and 2 (for roller and linear slide respectively of helix forming fixture)
//may be operated using potentiometers connected to void SetSteppersInterlocked(bool InterlockedMode);
//Pot1, connected to AD6 and read into InputVals[2], is the master control and directly
//controls stepper 1. In Interlocked Mode, it also controls stepper2, the linear slide which
//takes steps at a fraction of stepper1 rate. Roughly 1 step of S2 for 31 steps of S1 (actual ratio
//is 31.xx and we keep track of fractional steps.
//Steppers 1 and 2 can be in link-control mode or pot control mode. If a SetStep command comes in
//over link the stepper involved is put in link control mode.

//Steppers 2 and 3 only work in link control mode.

//Ramping was only implemented on S2.

void SetSteppersInterlocked(bool InterlockedMode);
void InitRamp(U16 Steps);
extern U16 InputVals[MAX_INPUT_VALS];

bool TickFlag;
float CurVel, CurMoveTime, TInfl1, TInfl2;
U16 InflStep, InflStep2;
U8 RampState = RAMP_IDLE;
//If true, means stepper was controlled by link call since last activated via Pot
//on A/D input, so the ManageSteppers() function will not turn it off when in
//the dead-zone at middle of Pot range. If Pot stays in dead-zone, link functions
//can freely control the steppers. If Pot for a stepper leaves dead-zone, the
//Pot regains control.
bool S1LinkControl, S2LinkControl;
U16 S2Accumulator;
bool SteppersInterlocked = true;
bool S1Busy, S2Busy, S3Busy, S4Busy;
U16 S1StepCount, S2StepCount, S3StepCount, S4StepCount;
U8 S2Dir;
bool S1AdjustFlag, S2AdjustFlag, S3AdjustFlag, S4AdjustFlag;  //true means we added 128 to low byte of period to make it  above minimum
                                    //and so need to make first full count cycle counted by high byte
                                    //of period be only 128 rather than 256
BWUnion S1Period, S2Period, S2PeriodCount, S1PeriodCount;
BWUnion S3Period, S4Period, S3PeriodCount, S4PeriodCount;

void InitStepping()
{
    S1Period.I = S2Period.I = S3Period.I = S4Period.I =65535;
    if(digitalRead(AUX1))
    { //high means switch not actuated at startup
      //so we put S1 and S2 under control by link.
      //to have local control, actuate the switch before applying power
      S1LinkControl = S2LinkControl = true;
    }
}

ISR(TIMER0_COMPA_vect)
{
    if(S1Busy)
    {
        if(S1PeriodCount.B[1] != 0)
        {
            --S1PeriodCount.B[1];
            if(S1AdjustFlag)
            {
                OCR0A += 128; //we want half cycle due to counts we added
                              //to the low period byte count
                S1AdjustFlag = false;
            }
            return;  //leaving OCR0A alone causes full timer cycle or 1024 USec
        }
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
                SET_STEP2();
            }
        }
        SET_STEP1();
        --S1StepCount;
        S1Busy = true;
        S1PeriodCount.I = S1Period.I;
        if(S1Period.B[0] < MIN_STEP_PERIOD_COUNT)
        {
            S1Period.B[0] += 128;
            S1AdjustFlag = true;
        }
        else
        {
            S1AdjustFlag = false;
        }
        OCR0A += S1Period.B[0];
        CLR_STEP2();
        CLR_STEP1();
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
        if(S2PeriodCount.B[1] != 0)
        {
            --S2PeriodCount.B[1];
            if(S2AdjustFlag)
            {
                OCR0B += 128; //we want half cycle due to counts we added
                              //to the low period byte count
                S2AdjustFlag = false;
            }
            return;  //leaving OCR0B alone causes full timer cycle or 1024 USec
        }
        S2Busy = false; //we must have finished timing this step
        //and we fall through to check if more steps are required
    }

    //if we get here we are not BUSY
    if(S2StepCount)
    {
        SET_STEP2();
        --S2StepCount;
        S2Busy = true;
        S2PeriodCount.I = S2Period.I;
        if(S2Period.B[0] < MIN_STEP_PERIOD_COUNT)
        {
            S2Period.B[0] += 128;
            S2AdjustFlag = true;
        }
        else
        {
            S2AdjustFlag = false;
        }
        OCR0B += S2Period.B[0];
        CLR_STEP2();
    }
    else
    {
        S2Busy = 0;
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
    if(S3Busy)
    {
        if(S3PeriodCount.B[1] != 0)
        {
            --S3PeriodCount.B[1];
            if(S3AdjustFlag)
            {
                OCR2A += 128; //we want half cycle due to counts we added
                              //to the low period byte count
                S3AdjustFlag = false;
            }
            return;  //leaving OCR2A alone causes full timer cycle or 1024 USec
        }
        S3Busy = false; //we must have finished timing this step
        //and we fall through to check if more steps are required
    }

    //if we get here we are not BUSY
    if(S3StepCount)
    {
        SET_STEP3();
        --S3StepCount;
        S3Busy = true;
        S3PeriodCount.I = S3Period.I;
        if(S3Period.B[0] < MIN_STEP_PERIOD_COUNT)
        {
            S3Period.B[0] += 128;
            S3AdjustFlag = true;
        }
        else
        {
            S3AdjustFlag = false;
        }
        OCR2A += S3Period.B[0];
        CLR_STEP3();
    }
    else
    {
        S3Busy = 0;
    }
}

ISR(TIMER2_COMPB_vect)
{
    if(S4Busy)
    {
        if(S4PeriodCount.B[1] != 0)
        {
            --S4PeriodCount.B[1];
            if(S4AdjustFlag)
            {
                OCR2B += 128; //we want half cycle due to counts we added
                              //to the low period byte count
                S4AdjustFlag = false;
            }
            return;  //leaving OCR2B alone causes full timer cycle or 1024 USec
        }
        S4Busy = false; //we must have finished timing this step
        //and we fall through to check if more steps are required
    }

    //if we get here we are not BUSY
    if(S4StepCount)
    {
        SET_STEP4();
        --S4StepCount;
        S4Busy = true;
        S4PeriodCount.I = S4Period.I;
        if(S4Period.B[0] < MIN_STEP_PERIOD_COUNT)
        {
            S4Period.B[0] += 128;
            S4AdjustFlag = true;
        }
        else
        {
            S4AdjustFlag = false;
        }
        OCR2B += S4Period.B[0];
        CLR_STEP4();
    }
    else
    {
        S4Busy = 0;
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

//AD notes:
//one of four AD channels is converted each millisecond
//in a four millisecond round robin sequence.
//The AD CHannels uses are A0, A1, A6, and A7, stored
//respectively in InputVals[0]-InputVals[3].
//Used A6 and A7 since on nano, they are only usaable for A/D
//A2-A5 can also be used for digital I/O.

static U8 MSCounter;
static U16 S1PVal, S2PVal;
//THis is called every millisecond but does something only
//every 16 milliseconds
void ManageSteppers()
{
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
            noInterrupts();
            S1Busy = false;
            S1StepCount = 0; //stop it
            interrupts();
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
                noInterrupts();
                S2Busy = false;
                S2StepCount = 0; //stop it
                interrupts();
            }

            S2Period.I = PotToCount(S2PVal);
        }
    }
    S1PVal = 0; //and reset the averagers since we just used them
    S2PVal = 0;
}

bool CMSlewDir; //if true initial slew move is positive, else negative.
U16 CMCurMove; //# of steps in next move
bool CMCurDir, CyclicMove;

//Originally only supporting stepper indices of 0 or 1. Then
//added 0X10 flag to StepperIndex to indicate desire to only set period
//added 0X20 flag to StepperIndex to indicate desire to do ramped move
//added 0X40 flag to StepperIndex to indicate desire to do cyclic move
void SetStep(U8 StepperIndex, U16 PeriodCount, U16 Steps, U8 Dir)
{
    bool SetCount = (StepperIndex & 0X10) == 0;  //that is not ramp-only
    bool SetRamp = (StepperIndex & 0X20) != 0; //only supporting ramping for S2 (index == 1)
    bool CMFlag = ((StepperIndex & 0X40) != 0); //only supporting ramping for S2 (index == 1)
    StepperIndex &= 3;//limit to zero through three
    PeriodCount = PeriodCount < MIN_STEP_PERIOD_COUNT ? MIN_STEP_PERIOD_COUNT : PeriodCount; //allow 10 thru 65535
    switch(StepperIndex)
    {
        case 0:  //S1
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
            break;

        case 1:  //S2
            if(CMFlag)
            {
                CMSlewDir = (Dir != 0);
                CyclicMove = (Steps != 0);
                CMCurMove = 0;
                return;
            }
            if(SetRamp)
            {
                PeriodCount = (U16) ((1/VELMIN)/PER_CNT_INTRV);
                InitRamp(Steps);
            }
            S2LinkControl = true;
            SetSteppersInterlocked(false);
            TIMSK0 &= ~_BV(OCIE0B);
            S2Period.I = PeriodCount;
            if(SetCount)
            {
                S2Dir = Dir;
                digitalWrite(STEPPER2_DIR, Dir > 0 ? HIGH : LOW);
                if(Steps == 0)
                {
                    S2Busy = false;
                }
                S2StepCount = Steps;
            }
            TIMSK0 |= _BV(OCIE0B);
            if(SetRamp)
            {
                InitRamp(Steps);
            }
            break;

        case 2:  //S3
            S3Period.I = PeriodCount;
            if(SetCount)
            {
                digitalWrite(STEPPER3_DIR, Dir > 0 ? HIGH : LOW);
                if(Steps == 0)
                {
                    S3Busy = false;
                }
                S3StepCount = Steps;
            }
            break;

        case 3:  //S4
            S4Period.I = PeriodCount;
            if(SetCount)
            {
                digitalWrite(STEPPER4_DIR, Dir > 0 ? HIGH : LOW);
                if(Steps == 0)
                {
                    S4Busy = false;
                }
                S4StepCount = Steps;
            }
            break;
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


void InitRamp(U16 Steps)
{
    float SCrit, TCrit, PlatSteps;
    CurVel = VELMIN + (ACCEL * RAMP_TIME_INCR)/2;

    CLR_SOL3();
    CLR_SOL2();
    TCrit = ((VELMAX - VELMIN)/ACCEL);
    SCrit = (VELMIN * TCrit) + ((TCrit*TCrit*ACCEL)/2);
    PlatSteps = ((float)Steps) - (2*SCrit);
    if(PlatSteps > 0.0)
    {   //vel will reach VELMAX and have RAMP_LIM state
        InflStep = Steps - SCrit -10;
        InflStep2 = SCrit + 335;
    }
    else
    {
        InflStep = Steps/2;
        InflStep2 = 0;
    }
    //CurMoveTime = 0.0;
    RampState = RAMP_UP;
}

void DoRamp()
{
    U16 PerCount, StepsLeft;
    switch(RampState)
    {
        case RAMP_IDLE:
        default:
            break;

        case RAMP_UP:
           // SET_SOL4();
            //CurMoveTime += RAMP_TIME_INCR;
            noInterrupts();
            StepsLeft = S2StepCount;
            interrupts();
            CurVel += ACCEL * RAMP_TIME_INCR;
            //if((CurMoveTime >= TInfl1) || (StepsLeft <= InflStep))
            if(StepsLeft <= InflStep)
            {
                if(InflStep2 != 0)
                {
                    CurVel = VELMAX;
                    RampState = RAMP_LIM;
                    //SET_SOL3();
                    PerCount = (U16) ((1/CurVel)/PER_CNT_INTRV);
                    noInterrupts();
                    S2Period.I = PerCount;
                    interrupts();
                }
                else
                {
                    //SET_SOL2();
                    RampState = RAMP_DOWN;
                }
            }
            else
            {
                PerCount = (U16) ((1/CurVel)/PER_CNT_INTRV);
                noInterrupts();
                S2Period.I = PerCount;
                interrupts();
            }
            break;

        case RAMP_LIM:
            //SET_SOL4();
            noInterrupts();
            StepsLeft = S2StepCount;
            interrupts();
            if(StepsLeft <= InflStep2)
            {
                //CLR_SOL3();
                //SET_SOL2();
                RampState = RAMP_DOWN;
            }
            break;

        case RAMP_DOWN:
            //SET_SOL4();
            noInterrupts();
            StepsLeft = S2StepCount;
            interrupts();
            if(StepsLeft == 0)
            {
                //CLR_SOL2();
                RampState = RAMP_IDLE;
            }
            else
            {
                CurVel -= ACCEL * RAMP_TIME_INCR;
                if(CurVel < VELMIN)
                {
                    CurVel = VELMIN;
                }
                PerCount = (U16) ((1/CurVel)/PER_CNT_INTRV);
                noInterrupts();
                S2Period.I = PerCount;
                interrupts();
            }
            break;

    }
    //CLR_SOL4();
}

U8 CMDelay;
 void  DoCycleMove()
 {
    if(CyclicMove && (S2StepCount == 0))
    {
        if(CMDelay != 0)
        {
            --CMDelay;
            return;
        }
        if(CMCurMove == 0)
        { //time to do the slew move
            SetStep(1, SLEW_PERIOD_COUNT, HOME_MOVE, CMSlewDir);
            CMCurMove = MAX_MOVE;
            CMSlewDir = !CMSlewDir;
            CMCurDir = CMSlewDir;
            CMDelay = 4;
        }
        else
        {
            if(CMCurMove == MIN_CYCLE_MOVE)
            {
                CMCurMove = 0;
                CMDelay = 4;
            }
            else
            {
                SetStep(5, SLEW_PERIOD_COUNT, CMCurMove, CMCurDir);//ramp move
                CMCurMove = .85 * CMCurMove;
                CMCurMove = CMCurMove < MIN_CYCLE_MOVE ? MIN_CYCLE_MOVE : CMCurMove;
                CMCurDir = !CMCurDir;
                CMDelay = 4;
            }
        }
    }
 }
