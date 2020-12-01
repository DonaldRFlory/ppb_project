//--------------------------------------------------------------------------
//	Definitions for trajectory generator.
//--------------------------------------------------------------------------

typedef union
{
    long long ll;
    long l[sizeof(long long)/sizeof(long)];
    short s[sizeof(long long)/sizeof(short)];
    char c[sizeof(long long)/sizeof(char)];
}LLUnion;

#define TICKS_PER_SECOND 4096
#define MAX_PULLOUT_STEPS 10
#define NUM_SEG_INFLECTIONS 2
//53,687,091.20275
#define VEL_SHIFT 12 //divides by 4096
#define MIN_MOVE	(0.00001)
#define POS_REG_SCALE ((double)4294967296ll)
#define VEL_REG_SCALE ((double)4294967296ll)
//13421.77280
#define ACC_REG_SCALE (VEL_REG_SCALE/TICKS_PER_SECOND)

//--------------------------------------------------------------------------
//	Definitions for PID control loops.
//--------------------------------------------------------------------------

//PWM servicee is at 4KHz rate, every 250 microseconds
#define ITERM_MAX_WINDUP 125000

//Note: If these values are changed, they must also be changed in Servo.CS as C# does not allow header files
#define ITERM_MAX 4000
#define PTERM_MAX 1000000
#define DTERM_MAX 250000

#define DRIVE_LIMIT PWM_FULL_ON
#define MAX_ERROR  2000
#define MAX_DEL_ERROR 2000

#define MIN_ACCEL (.001)
#define MIN_VEL (.001)
#define MAX_VEL (1000000.0)
#define MAX_ACCEL (1000000.0)

//One more than number of steps (NSteps must be even)
#define NUM_LIST_SEGS 10
#define NUM_LIST_POINTS (NUM_LIST_SEGS + 1)
#define NUM_PERIOD_SEGS (4*NUM_LIST_SEGS)
#define NUM_PERIOD_POINTS (NUM_PERIOD_SEGS + 1)
//Both min and max ticks must be even
#define MIN_PTICKS (30)
#define MAX_PTICKS (100000)
#define MAX_PLIST_VEL (TICKS_PER_SECOND/(1.0 * MIN_PTICKS))
#define MIN_PLIST_VEL (TICKS_PER_SECOND/(1.0 * MAX_PTICKS))
#define MIN_PLIST_ACC (.00001)
#define MAX_PLIST_ACC (100000.)


//Servo commands
//The first three may be loaded to ServCmd[4] to control trajectory generator ISR.
//The rest are used only in calls to ServoCommand() function.
#define SCMD_NONE			0
#define SCMD_RUN			1
#define SCMD_STOP			2
#define SCMD_RESET_POS		3
#define SCMD_ENABLE_PID		4
#define SCMD_DISABLE_PID	5
#define SCMD_STOP_IMMEDIATE	6

//SS_STATE
#define SS_IDLE		0
#define SS_STOP_ERR 1
#define SS_STOP		2
#define SS_MOVE		3


void PIDServe(void);
U8 ServoMove(U8 Channel, U32 Position);
U8 MoveR(unsigned char Channel, long Steps);

//Clears QCount, QErrors, Servo Position, Acceleration, and Velocity for
//the selected servo. In addition cancels any trajectory sequence.
void ResetPIDs(void);
void ResetServos(void);
void ResetServo(U8 Channel);
void SetPIDEnable(U8 Channel, U8 OnFlag);
void SetCommandPosition(U8 Channel, U32 Position);

//Block transfer function prototypes:
//This will return for the selected channel,
//PTerm, ITerm, DTerm or a subset thereof based on UPCount
//U8 GetPIDData(U8 Channel, UP_PTR_U32 UpWords, U16 UpCount); //host side
U8 GetPIDData(U8 Channel, U16 UpCount); //slave side

//This will set for the selected channel,
//PTerm, ITerm, DTerm or a subset thereof based on DownCount
//U8 SetPIDData(U8 Channel, DOWN_PTR_U32 DownWords, U16 DownCount);  //host side
U8 SetPIDData(U8 Channel, U16 DownCount); //slave side

//This will return for the selected channel,
//QCount, QErrors, ServCmdPosHigh, ServCmdPosLow, CurAccHigh, CurrAccLow, CurVelHigh,
// CurVelLow, PIDDrive, PIDIAccum
//or these values up to the limit of UpCount
//void GetServoData(U8 Channel, UP_PTR_U32 UpWords, U16 UpCount);  //host side
U8 GetServoData(U8 Channel, U16 UpCount); //slave side

//--------------------------------------------------------------------------
