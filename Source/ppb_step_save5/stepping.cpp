#include "project.h"
#include <math.h>
bool SegmentSwitch;
extern U8 StrBuffer[64];
void WriteTraceBuffer(U8* InBuff, U16 Count);
int XSteps, XStepsSave, YSteps, YStepsSave;
void RotServe();

//this is all about capturing total x and y steps taken during a circle and
//then setting this flag so that background loop can print them. For debugging
//of x and y creep.
bool CircleStartFlag;
//phasing in use of StepCtrl. Initially it is used only for Stepper 3 until debugged
STEP_CTRL StepCtrl[MAX_STEPPERS];

//Steppers 1 and 2 (for roller and linear slide respectively of helix forming fixture)
//may be operated using potentiometers
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
U8 S2Dir;//this is used to inform limit switch monitoring which limit switch should stop motion
bool RotMode;//circular pattern on Steppers 3 and 4

ROT_CONTROL RotControl;
ROT_SEG RotSegs[2];

//Make sure to set up rotation parameters before calling with Active true
//Calling with Active false will kill any active moves on steppers 3 and 4
//in preparation for setting up RotMode
void RotModeControl(bool Active)
{
    if(Active)
    {
        RotServe();//make sure both segment controls are filled
        RotServe();
        TIMSK2 &= ~(_BV(OCIE2B) | _BV(OCIE2A)); //disable output compare A and B on timer 2
//        StepCtrl[2].Period.I = 1000;
        RotControl.RotActive = true;
        TIMSK2 |= _BV(OCIE2A); //enable output compare A on timer 2
    }
    else
    {
        RotControl.RotActive = false;
        TIMSK2 &= ~(_BV(OCIE2B) | _BV(OCIE2A)); //disable output compare A and B on timer 2
    }
}

//Radius is in steps
//We will require that radius is at least 5 steps. Then circumference is at least 31.
//We will use 100 segments if we can. That means for radius 16 and above we have 100 segments,
//starting with a repeat of 1 at 16. Then as  radius increases, we get more repeats until repeats
//increases to 255. Above that point, we increase segments to keep repeats at 255.
#define MIN_ROT_RADIUS 5
U8 SetRotRadius(U16 Radius)
{
    float Circum;

    if(Radius < MIN_ROT_RADIUS)
    {
        return 0;
    }

    XSteps = YSteps = 0;
    if(RotControl.State == RS_UNINITED)
    {
        RotControl.State = RS_INIT1;
        RotControl.LastPoint.Y = 0;
        RotControl.LastPoint.X = Radius;
        RotControl.Segment = 0;
        RotControl.AFlag = true;
        RotSegs[0].FullFlag = false;
        RotSegs[1].FullFlag = false;
    }

    Circum = Radius * (2 * PI);
    if(Circum >= 20)
    {
        RotControl.Segments = 20;
    }
    else
    {
        RotControl.Segments = (U16)Circum;
    }
    RotControl.Radius = Radius;
    RotControl.Cycles = (U16)(Circum/RotControl.Segments);
    sprintf(StrBuffer, "SRR R = %d Cycles = %u\n", Radius, RotControl.Cycles);
    WriteTraceBuffer(StrBuffer, strlen(StrBuffer));
    RotControl.SegAngle = 2*PI/RotControl.Segments;
}

void RotServe()
{
    long Delta;
    int X, Y;
    U8 CtrlIdx = 0; //ControlIndex
    float CurAngle;

    if(RotSegs[0].FullFlag)
    {
        if(RotSegs[1].FullFlag)
        {
            return; //nothing to do
        }
        CtrlIdx = 1;
    }
    //CtrlIdx is set to empty segment control
    RotSegs[CtrlIdx].CircleStart = (RotControl.Segment == 0);//mark new segment as start of new circle

    RotSegs[CtrlIdx].Cycles = RotControl.Cycles;
    RotSegs[CtrlIdx].XAccum = RotSegs[CtrlIdx].YAccum = 0;
    CurAngle = RotControl.SegAngle * RotControl.Segment;
    if(++RotControl.Segment >= RotControl.Segments)
    {
        RotControl.Segment = 0;
    }
    X = (int)(cos(CurAngle) * RotControl.Radius);
    Delta = (long)X - RotControl.LastPoint.X;
    if(Delta < 0)
    {
        Delta = -Delta;
        RotSegs[CtrlIdx].XDir = 0;
    }
    else
    {
        RotSegs[CtrlIdx].XDir = 1;
    }
    Delta = (Delta * 0X8000)/RotControl.Cycles; //scale and divide by number of cycles
    if(RotSegs[CtrlIdx].XDir == 0)
    {
        RotControl.LastPoint.X -= (int)((RotControl.Cycles * Delta) >> 15);
    }
    else
    {
        RotControl.LastPoint.X += (int)((RotControl.Cycles * Delta) >> 15);
    }
    RotSegs[CtrlIdx].XIncr = (U16)(Delta);

    Y = (int)(sin(CurAngle) * RotControl.Radius);

    Delta = (long)Y - RotControl.LastPoint.Y;
    if(Delta < 0)
    {
        Delta = -Delta;
        RotSegs[CtrlIdx].YDir = 0;
    }
    else
    {
        RotSegs[CtrlIdx].YDir = 1;
    }
    Delta = (Delta * 0X8000)/RotControl.Cycles;//scale and divide by number of cycles
    if(RotSegs[CtrlIdx].YDir == 0)
    {
        RotControl.LastPoint.Y -= (int)((RotControl.Cycles * Delta) >> 15);
    }
    else
    {
        RotControl.LastPoint.Y += (int)((RotControl.Cycles * Delta) >> 15);
    }
    RotSegs[CtrlIdx].YIncr = (U16)(Delta);
    sprintf(StrBuffer, "RS (%d, %d) %d %u %d\n", X, Y, RotControl.Segment, RotSegs[CtrlIdx].XIncr, RotSegs[CtrlIdx].YIncr);
    WriteTraceBuffer(StrBuffer, strlen(StrBuffer));

    RotSegs[CtrlIdx].FullFlag = true;
    if(RotControl.State == RS_INIT1) //if we are generating first segment
    { //We need to setup motor directions for first segment
        digitalWrite(STEPPER3_DIR, RotSegs[0].XDir);
        digitalWrite(STEPPER4_DIR, RotSegs[0].YDir);
        RotControl.State = RS_INIT2;
    }
}

void InitStepping()
{
    for(int i = 0; i < MAX_STEPPERS; ++i)
    {
        StepCtrl[i].StepCount = 0;
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
            if(S2Accumulator >= 0X4000)
            {
                S2Accumulator &= ~0x4000;
                SET_STEP2();
            }
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

void RotISR()
{
    if(RotControl.AFlag)
    {   //we are using first SEGMENT structure at index 0
        //the code in the two branches is nearly identical but
        //we duplicate for speed of execution in ISR
        RotSegs[0].XAccum += RotSegs[0].XIncr;
        RotSegs[0].YAccum += RotSegs[0].YIncr;
        if(RotSegs[0].XAccum & 0X8000)
        {   //if accumulated a full step
            SET_STEP3();  //step X
            if(RotSegs[0].XDir == 1)
            {
                ++XSteps;
            }
            else
            {
                --XSteps;
            }
            RotSegs[0].XAccum &= ~0x8000; //clear the full step bit
        }
        if(RotSegs[0].YAccum & 0X8000)
        {   //if accumulated a full step
            SET_STEP4();  //step Y
            if(RotSegs[0].YDir == 1)
            {
                ++YSteps;
            }
            else
            {
                --YSteps;
            }
            RotSegs[0].YAccum &= ~0x8000; //clear the full step bit
        }
        --RotSegs[0].Cycles;
        if(RotSegs[0].Cycles == 0)
        {   //time to switch segment control sets
            SegmentSwitch = true; //to trigger setting new direction outside RotISR()
            SET_SOL4();
            if(RotSegs[1].FullFlag)
            {  //switching to set 1
                if(RotSegs[1].CircleStart)
                {
                    XStepsSave = XSteps;
                    YStepsSave = YSteps;
                    //this is all about capturing total x and y steps taken during a circle and
                    //then setting this flag so that background loop can print them. For debugging
                    //of x and y creep.
                    CircleStartFlag = true;
                }
                RotControl.AFlag = false;
                RotSegs[0].FullFlag = false;
            }
            else
            {   //segment underrun so we shut down
                TIMSK2 &= ~_BV(OCIE2A); //disble output compare A on timer 2
            }
        }
    }
    else
    {   //we are using second SEGMENT structure at index 1
        RotSegs[1].XAccum += RotSegs[1].XIncr;
        RotSegs[1].YAccum += RotSegs[1].YIncr;
        if(RotSegs[1].XAccum & 0X8000)
        {   //if accumulated a full step
            SET_STEP3();  //step X
            if(RotSegs[1].XDir == 1)
            {
                ++XSteps;
            }
            else
            {
                --XSteps;
            }
            RotSegs[1].XAccum &= ~0x8000; //clear the full step bit
        }
        if(RotSegs[1].YAccum & 0X8000)
        {   //if accumulated a full step
            SET_STEP4();  //step Y
            if(RotSegs[1].YDir == 1)
            {
                ++YSteps;
            }
            else
            {
                --YSteps;
            }
            RotSegs[1].YAccum &= ~0x8000; //clear the full step bit
        }
        --RotSegs[1].Cycles;
        if(RotSegs[1].Cycles == 0)
        {   //time to switch segment control sets
            SegmentSwitch = true; //to trigger setting new direction outside RotISR()
            CLR_SOL4();
            if(RotSegs[0].FullFlag)
            {   //switching to set 0
                if(RotSegs[0].CircleStart)
                {
                    XStepsSave = XSteps;
                    YStepsSave = YSteps;
                    //this is all about capturing total x and y steps taken during a circle and
                    //then setting this flag so that background loop can print them. For debugging
                    //of x and y creep.
                    CircleStartFlag = true;
                }
                RotSegs[0].XAccum = RotSegs[0].YAccum = 0;
                RotSegs[0].XAccum = RotSegs[0].YAccum = 0;
                RotControl.AFlag = true;
                RotSegs[1].FullFlag = false;
            }
            else
            {   //segment underrun so we shut down
                TIMSK2 &= ~_BV(OCIE2A); //disable output compare A on timer 2
            }
        }
    }

}


void RotISRWrapper()
{ //for calls over link to simulate interrupt
    RotISR();
    CLR_STEP3();//done outside RotISR to get a little more time for pulse
    CLR_STEP4();
    if(SegmentSwitch)
    {
        if(RotControl.AFlag)
        {
            digitalWrite(STEPPER3_DIR, RotSegs[0].XDir);
            digitalWrite(STEPPER4_DIR, RotSegs[0].YDir);
        }
        else
        {
            digitalWrite(STEPPER3_DIR, RotSegs[1].XDir);
            digitalWrite(STEPPER4_DIR, RotSegs[1].YDir);
        }
        SegmentSwitch = false;
    }
}

//For link test only
//First, make sure both RotSegs are full, then
//call RotISR until at least one RotSeg is not full
void SegStep()
{
    RotServe(); //generate segment data if needed
    while(RotSegs[0].FullFlag && RotSegs[1].FullFlag)
    {
        RotISRWrapper();
    }
}


//ref only:
//typedef struct
//{
//    U16 XAccum, YAccum;
//    U16 XIncr, YIncr;
//    U8 CycleCount ;
//    U8 Dir; //only looked at when switching to data set. ISR sets direction of approp motor
//}ROT_SEG;
 ISR(TIMER2_COMPA_vect)
{
    if(RotControl.RotActive)
    {
        RotISR();
        OCR2A += StepCtrl[2].Period.B[0];
        CLR_STEP3();//done outside RotISR to get a little more time for pulse
        CLR_STEP4();

        if(SegmentSwitch)
        {
            if(RotControl.AFlag)
            {
                digitalWrite(STEPPER3_DIR, RotSegs[0].XDir);
                digitalWrite(STEPPER4_DIR, RotSegs[0].YDir);
            }
            else
            {
                digitalWrite(STEPPER3_DIR, RotSegs[1].XDir);
                digitalWrite(STEPPER4_DIR, RotSegs[1].YDir);
            }
            SegmentSwitch = false;
        }
    }
    else
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
        //So.. We want to check here if a new linked move has been posted.
        //If so, it may be a zero length move which requires timing none-the-less.
        //If there is a new move posted, and it is zero steps, we set StepCount to 1 and
        //load Period.i with the period, and set AdjustFlag to true to signal
        //that no step pulse should be generated.
        //To start linked move sequence, the background process masks OCIE2B,
        //then posts the first move. If move is zero length, it sets AdjustFlag
        //and sets StepCount to 1, otherwise it clears AdjustFlag and
        //sets StepCount to desired steps. Also loads Period.I with desired
        //period and sets OCR2B to a few counts beyond where timer counter is currently
        // and unmasks OCR2B.
        TIMSK2 &= ~ _BV(OCIE2B);    //disable output compare B on timer 2
        StepCtrl[3].Busy = false;
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

//Mangages movement of steppers for tube wrapper. If pots are in active
//range, they take over control and move the wrapper and linear slide
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

float RVMin, RVMax, RAccel;
//In a bit of a kludge, we are going to require calling SetVMinAccel() function
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
            S1  = true;
            InitializeRamp(0, Steps, PeriodCount, SetRamp);
            if(SetCount)
            {
                TIMSK0 &= ~_BV(OCIE0A);
                digitalWrite(STEPPER1_DIR, Dir > 0 ? HIGH : LOW);
                if(Steps == 0)
                {
                    StepCtrl[0].Busy = false;
                }
                StepCtrl[0].StepCount = Steps;
            }
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
            if(SetCount)
            {
                TIMSK0 &= ~_BV(OCIE0B);
                S2Dir = Dir;
                digitalWrite(STEPPER2_DIR, Dir > 0 ? HIGH : LOW);
                if(Steps == 0)
                {
                    StepCtrl[1].Busy = false;
                }
                StepCtrl[1].StepCount = Steps;
            }
            break;

        case 2:  //S3
            InitializeRamp(2, Steps, PeriodCount, SetRamp);
            if(SetCount)
            {
                TIMSK2 &= ~ _BV(OCIE2A);    //disable output compare A on timer 2
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
                TIMSK2 &= ~ _BV(OCIE2B);    //disable output compare B on timer 2
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
                break;
            }
            StepCtrl[SI].RampState = RAMP_IDLE;
            break;
    }
}

//float RVMin, RVMax, RAccel;
//Should be called before move is started
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
//    Ramp(SI);
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
