#define SWITCH_DEBOUNCE_MSEC 50

//We are going to have five groups of servos, each with potentially
//16 servos. The first group will be servos controlled by Arduino pins
//Probably less than 16 will be implemented, (currently six).
//The next four will be controlled by up to four PCA9685 boards.
#define SERVO_GP_SIZE 16
#define GP1_BASE_INDEX 16

//Numbers actually implemented in present configuration
#define NUM_GP0_SERVOS 6
#define NUM_GP1_SERVOS 16
#define NUM_GP2_SERVOS 16
#define NUM_GP3_SERVOS 0
#define MAX_INDEX (GP1_BASE_INDEX + NUM_GP1_SERVOS + NUM_GP2_SERVOS + NUM_GP3_SERVOS - 1)
#define MIN_INDEX (GP1_BASE_INDEX)
#define MID_INDEX (GP1_BASE_INDEX + ((NUM_GP1_SERVOS + NUM_GP2_SERVOS + NUM_GP3_SERVOS) / 2))
#define NUM_COLS ((NUM_GP1_SERVOS + NUM_GP2_SERVOS + NUM_GP3_SERVOS) / 2)
#define MID_INDEX (GP1_BASE_INDEX + ((NUM_GP1_SERVOS + NUM_GP2_SERVOS + NUM_GP3_SERVOS) / 2))

#define NUM_SERVO_GROUPS 5
#define SERVO_UPDATE_FLAG 0X8000

//The input values are data sent back regularly to TestApp over link
#define MAX_INPUT_VALS 10  //four AD Vals, status bits, two step counts, TB_CNT, S2Period and finally TraceBuffCount
#define S1STEP_IV_IDX   5
#define S2STEP_IV_IDX   6
#define RAMP_INFL_IV_IDX  7
#define S2PER_IV_IDX   8
#define TBCOUNT_IV_IDX 9

//These are the bit values in port C corresponding to the Analog pins
#define A0_BIT   1
#define A1_BIT   2
#define A2_BIT   4
#define A3_BIT   8
#define A4_BIT   16
#define A5_BIT   32

//Status bits for input vals index 4
#define IV_SWITCH_BIT   0X01
#define IV_LIMIT1_BIT   0X02
#define IV_LIMIT2_BIT   0X04
#define IV_LIMIT3_BIT   0X08
#define IV_LIMIT4_BIT   0X10
#define IV_S1DIR_BIT    0X20
#define IV_S2DIR_BIT    0X40
#define MAX_OUTPUT_VALS 6

#define PPF_IDLE    0
#define PPF_SOL_ENGAGE 1
#define PPF_SPINUP 2
#define PPF_FIRE 3

#define PP_SOL_ENGAGE_MSEC 10
#define PP_SPINUP_MSEC 750
#define PP_FIRE_MSEC 100

#define PP_FIRE_USEC_MIN 920
#define PP_FIRE_USEC_MAX 1760

#define TELLTALE_PIN 13     //D13 B5 SCK LED_BUILTIN
//#define STEPPER1_ENABLE 6  //shared with SOLENOID1

#define SOLENOID1_PIN  6    //D6
#define AUX5           6    //D6

#define SOLENOID2_PIN  5    //D5
#define AUX4           5    //D5
#define STEPPER2_STEP  5

#define SOLENOID3_PIN  4    //D4
#define AUX3           4    //D4
#define STEPPER2_DIR   4

#define SOLENOID4_PIN  3    //D3
#define AUX2           3    //D3

#define AUX1  2             //D2

#define STEPPER1_ENABLE   12
#define SERVO1_PIN 12       //D12 B4

#define STEPPER1_DIR  11
#define SERVO2_PIN 11       //D11 B3

#define STEPPER1_STEP  11
#define SERVO3_PIN 10       //D10 B2

#define STEPPER2_ENABLE   9
#define SERVO4_PIN 9        //D09 B1

#define STEPPER2_DIR  8
#define SERVO5_PIN 8        //D08 B0

#define STEPPER2_STEP  7
#define SERVO6_PIN 7        //D07 D7

#define TX_BUFFER_SIZE 63   //max it will actually hold, array size is one byte more
#define L_FAULT 2
#define L_IDLE 0
#define L_RCV 1
#define L_FAULT 2
#define L_RSP 3
#define L_FINAL 4

#define MSEC_COMMUTATOR_DIVIDE  4

//PP Cannon fire states:
#define PP_CAN_IDLE 0
#define PP_CAN_IDLE

void Fire();
