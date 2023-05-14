//The increment below is designed to give one step on stepper 2 for every 23.818
//steps of stepper 1 in interlocked steppers mode. This should give
//proper pitch for helix.
#define STEPPER_2_INCREMENT 1376

#define SWITCH_DEBOUNCE_MSEC 50

#define MAX_INPUT_VALS 5
#define IV_SWITCH_BIT   1
#define IV_LIMIT1_BIT   2
#define IV_LIMIT2_BIT   4
#define IV_LIMIT3_BIT   8
#define IV_LIMIT4_BIT   16
#define MAX_OUTPUT_VALS 6

#define PPF_IDLE    0
#define PPF_SOL_ENGAGE 1
#define PPF_SPINUP 2
#define PPF_FIRE 3

#define TX_PIN 1
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

#define SOLENOID3_PIN  4    //D4
#define AUX3           4    //D4

#define SOLENOID4_PIN  3    //D3
#define AUX2           3    //D3

#define AUX1  2             //D2

#define STEPPER1_ENABLE   12
#define SERVO1_PIN 12       //D12 B4

#define STEPPER1_DIR  11
#define SERVO2_PIN 11       //D11 B3

#define STEPPER1_STEP  10
#define STEPPER1_STEP_BIT   0X04
#define SERVO3_PIN 10       //D10 B2
//Step pin is active low
#define SET_STEP1()        PORTB &= ~STEPPER1_STEP_BIT
#define CLR_STEP1()        PORTB |= STEPPER1_STEP_BIT

#define STEPPER2_ENABLE   9
#define SERVO4_PIN 9        //D09 B1

#define STEPPER2_DIR  8
#define SERVO5_PIN 8        //D08 B0

#define STEPPER2_STEP  7    //D07 D7
#define STEPPER2_STEP_BIT   0X80
#define SERVO6_PIN 7        //D07 D7
//Step pin is active low
#define SET_STEP2()        PORTD &= ~STEPPER2_STEP_BIT
#define CLR_STEP2()        PORTD |= STEPPER2_STEP_BIT

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
