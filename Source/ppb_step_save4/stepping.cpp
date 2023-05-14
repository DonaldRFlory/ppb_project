#include "project.h"
#include <math.h>

//phasing in use of StepCtrl. Initially it is used only for Stepper 3 until debugged
STEP_CTRL StepCtrl[MAX_STEPPERS];

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

void InitializeRamp(U8 SI, U16 MoveSteps, U16 PeriodCount, bool RampActive);
void SetSteppersInterlocked(bool InterlockedMode);
extern U16 InputVals[MAX_INPUT_VALS];

bool TickFlag;

//If true, means stepper was controlled by link call since last activated via Pot
//on A/D input, so the ManageSteppers() function will not turn it off when in
//the dead-zone at middle of Pot range. If Pot stays in dead-zone, link functions
//can freely control the steppers. If Pot for a stepper leaves dead-zone, the
//Pot regains control.
bool S1LinkControl, S2LinkControl;
U16 S2Accumulator;
bool SteppersInterlocked = true;
U8 S2Dir;

void InitStepping()
{
    for(int i = 0; i < MAX_STEPPERS; ++i)
    {
        StepCtrl[i].StepCount = 0;
        StepCtrl[i].TrigStep = 0;
        StepCtrl[i].Triggered = false;
    }
    if(digitalRead(AUX1))
    { //high means switch not actuated at startup
      //so we put S1 and S2 under control by link.
      //to have local control, actuate the switch before applying power
      S1LinkControl = S2LinkControl = true;
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
        if(SteppersInterlocked)
        {
            S2Accumulator += STEPPER_2_INCREMENT;
            if(S2Accumulator >= 0X8000)
            {
                S2Accumulator &= ~0x8000;
                SET_STEP2();
            }
        }
        if(!StepCtrl[0].Triggered & (StepCtrl[0].StepCount <= StepCtrl[0].TrigStep))
        {   //need to load next ramp period value
            StepCtrl[0].Period.I = StepCtrl[0].NextPeriod.I;
            StepCtrl[0].Triggered = true;//tell Ramp to generate a new value
            //CLR_SOL3(); //to track Triggered status and Ramp service time
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
        StepCtrl[0].Busy = false;
    }
}

//This interrupt will not occur if SteppersInterlocked is true.
//Instead, stepper 2 will be controlled in an interlocked fashion by
//TIMER0_COMPA ISR above.
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
        if(!StepCtrl[1].Triggered & (StepCtrl[1].StepCount <= StepCtrl[1].TrigStep))
        {   //need to load next ramp period value
            StepCtrl[1].Period.I = StepCtrl[1].NextPeriod.I;
            StepCtrl[1].Triggered = true;//tell Ramp to generate a new value
            //CLR_SOL3(); //to track Triggered status and Ramp service time
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

//float DelayVar;
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
        if(!StepCtrl[2].Triggered & (StepCtrl[2].StepCount <= StepCtrl[2].TrigStep))
        {   //need to load next ramp period value
            StepCtrl[2].Period.I = StepCtrl[2].NextPeriod.I;
            StepCtrl[2].Triggered = true;//tell Ramp to generate a new value
            CLR_SOL3(); //to track Triggered status and Ramp service time
        }
        //DelayVar += 1.0;
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
        if(!StepCtrl[3].Triggered & (StepCtrl[3].StepCount <= StepCtrl[3].TrigStep))
        {   //need to load next ramp period value
            StepCtrl[3].Period.I = StepCtrl[3].NextPeriod.I;
            StepCtrl[3].Triggered = true;//tell Ramp to generate a new value
            CLR_SOL3(); //to track Triggered status and Ramp service time
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
        StepCtrl[3].Busy = 0;
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
            StepCtrl[0].StepCount = 40000; //arbitrary value to keep it going
        }
        else
        {
            noInterrupts();
            StepCtrl[0].Busy = false;
            StepCtrl[0].StepCount = 0; //stop it
            interrupts();
        }

        StepCtrl[0].Period.I = PotToCount(S1PVal);
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
                StepCtrl[1].StepCount = 40000; //arbitrary value to keep it going
            }
            else
            {
                noInterrupts();
                StepCtrl[1].Busy = false;
                StepCtrl[1].StepCount = 0; //stop it
                interrupts();
            }

            StepCtrl[1].Period.I = PotToCount(S2PVal);
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
            InitializeRamp(0, Steps, PeriodCount, SetRamp);
            if(SetCount)
            {
                digitalWrite(STEPPER1_DIR, Dir > 0 ? HIGH : LOW);
                if(Steps == 0)
                {
                    StepCtrl[0].Busy = false;
                }
                StepCtrl[0].StepCount = Steps;
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
            InitializeRamp(1, Steps, PeriodCount, SetRamp);
            S2LinkControl = true;
            SetSteppersInterlocked(false);
            TIMSK0 &= ~_BV(OCIE0B);
            if(SetCount)
            {
                S2Dir = Dir;
                digitalWrite(STEPPER2_DIR, Dir > 0 ? HIGH : LOW);
                if(Steps == 0)
                {
                    StepCtrl[1].Busy = false;
                }
                StepCtrl[1].StepCount = Steps;
            }
            TIMSK0 |= _BV(OCIE0B);
            break;

        case 2:  //S3
            InitializeRamp(2, Steps, PeriodCount, SetRamp);
            if(SetCount)
            {
                digitalWrite(STEPPER3_DIR, Dir > 0 ? HIGH : LOW);
                if(Steps == 0)
                {
                    StepCtrl[2].Busy = false;
                }
                StepCtrl[2].StepCount = Steps;
            }
            break;

        case 3:  //S4
            InitializeRamp(3, Steps, PeriodCount, SetRamp);
            if(SetCount)
            {
                digitalWrite(STEPPER4_DIR, Dir > 0 ? HIGH : LOW);
                if(Steps == 0)
                {
                    StepCtrl[3].Busy = false;
                }
                StepCtrl[3].StepCount = Steps;
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

//extern U16 Values[4];
//bool OneTimeFlag[4];
void Ramp(U8 SI)
{
    U8 Steps;//# of steps we will take before new period value will be used due to TrigStep
    float Vel;

    if(!StepCtrl[SI].Triggered)
    {
        return;
    }

    //We assume in Ramp() code that we will never command moves very close to 65535,
    //specifically closer than the number of steps in ramp segment which might cause
    //U16 overflow. This max is around 50 steps but we are going to limit moves
    //to 65000 giving a 535 step margin
    //Actually, inflection points can never be larger than half the total move or
    //total move - scrit.
    switch(StepCtrl[SI].RampState)
    {
        case RAMP_IDLE:
        default:
            break;

        case RAMP_UP:
            //OneTimeFlag[SI] = true;
            Vel = VELMIN + (ACCEL * StepCtrl[SI].RampTime);//compute Vel based on RampTime
            StepCtrl[SI].NextPeriod.I = (U16) ((1/Vel)/PER_CNT_INTRV);
            Steps = (U8)(Vel * RAMP_TIME_INCR);
            Steps = Steps < 1 ? 1 : Steps; //how many steps in this ramp increment, minimum of 1
            //We are considering last TrigStep to be our current position for the next calculation
            if(StepCtrl[SI].TrigStep <= StepCtrl[SI].Infl1 + Steps)//if we would hit or overshoot Infl1
            {
                StepCtrl[SI].RampTime -= .0042;//empirical fudge factor
                Steps = StepCtrl[SI].TrigStep - StepCtrl[SI].Infl1;
                if(StepCtrl[SI].Infl1 == StepCtrl[SI].Infl2)
                {
                    StepCtrl[SI].RampState = RAMP_DOWN;
                }
                else
                {
                    StepCtrl[SI].RampState = RAMP_LIM;
                }
            }
            StepCtrl[SI].TrigStep -= Steps;
            StepCtrl[SI].RampTime += Steps * StepCtrl[SI].NextPeriod.I * PER_CNT_INTRV;   //time after this ramp increment is done
            break;

        case RAMP_LIM:
            StepCtrl[SI].RampState = RAMP_DOWN;
            StepCtrl[SI].TrigStep = StepCtrl[SI].Infl2;
            StepCtrl[SI].NextPeriod.I = (U16) ((1/VELMAX)/PER_CNT_INTRV);
            break;

        case RAMP_DOWN:
            Vel = VELMIN + (ACCEL * StepCtrl[SI].RampTime);//compute Vel based on RampTime
            //----------------------------------
            //if(OneTimeFlag[SI])
            //{
            //  OneTimeFlag[SI] = false;
            // Values[0] = (U16) (StepCtrl[SI].RampTime * 1000);
            //  Values[1] = StepCtrl[SI].TrigStep;
            //  Values[2] = (U16) (StepCtrl[SI].StepCount);
            //  Values[3] = (U16) Vel;
            //}
            StepCtrl[SI].NextPeriod.I = (U16) ((1/Vel)/PER_CNT_INTRV);
            Steps = (U8)(Vel * RAMP_TIME_INCR);
            Steps = Steps < 1 ? 1 : Steps; //how many steps in this ramp increment
            if(Steps > StepCtrl[SI].TrigStep)
            { //move will end before next ramp adjustment so we are done ramping
                StepCtrl[SI].RampState = RAMP_IDLE;
                StepCtrl[SI].TrigStep = 0;
            }
            else
            {
                StepCtrl[SI].TrigStep -= Steps;//next trigger step
            }
            StepCtrl[SI].RampTime -= Steps * StepCtrl[SI].NextPeriod.I * PER_CNT_INTRV;   //time after this ramp increment is done
            if(StepCtrl[SI].RampTime < 0.0)
            {
                StepCtrl[SI].RampTime = 0.0;
            }
            break;
    }
    SET_SOL3();    //to track Triggered status
    StepCtrl[SI].Triggered = false;
}

void InitializeRamp(U8 SI, U16 MoveSteps, U16 PeriodCount, bool RampActive)
{
    //Should be called before move is started
    float SCrit = (VELMIN * TCRIT) + (ACCEL * TCRIT * TCRIT / 2);
    if(!RampActive)
    {
        StepCtrl[SI].Period.I = PeriodCount;
        StepCtrl[SI].RampState = RAMP_IDLE;
        StepCtrl[SI].TrigStep  = StepCtrl[SI].Infl1 = StepCtrl[SI].Infl2 = 0;
        StepCtrl[SI].Triggered = false;
        return;
    }

    //If ramping, we always start at VELMIN, PeriodCount specified is ignored
    StepCtrl[SI].Period.I = (U16) ((1/VELMIN)/PER_CNT_INTRV);;
    if(MoveSteps > 2*SCrit)
    { //have plateau
        StepCtrl[SI].Infl1 = MoveSteps - (U16)SCrit;  //inflection steps are values of StepCount
        StepCtrl[SI].Infl2 = (U16)SCrit;              //which counts down from MoveSteps
    }
    else
    {
        StepCtrl[SI].Infl1 = StepCtrl[SI].Infl2 = MoveSteps/2;
    }

    StepCtrl[SI].TrigStep = MoveSteps;
    StepCtrl[SI].RampState = RAMP_UP;
    StepCtrl[SI].RampTime = 0.0;
    StepCtrl[SI].Triggered = true;
    Ramp(SI);
}

U8 CMDelay;
 void  DoCycleMove()
 {
    if(CyclicMove && (StepCtrl[1].StepCount == 0))
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
