#define NODE_ADDRESS_EEPROM_ADDRESS 0
//The increment below is designed to give one step on stepper 2 for every 23.818
//steps of stepper 1 in interlocked steppers mode. This should give
//proper pitch for helix.
#define STEPPER_2_INCREMENT 1376

#define SWITCH_DEBOUNCE_MSEC 50

#define SLEW_PERIOD_COUNT 100
#define MAX_MOVE 56000
#define MIN_CYCLE_MOVE 500
#define HOME_MOVE 57000

#define MIN_STEP_PERIOD_COUNT  10

#define MAX_INPUT_VALS 9  //four AD Vals, status bits, two step counts, InflStep and S2Period
#define S1STEP_IV_IDX   5
#define S2STEP_IV_IDX   6
#define RAMP_INFL_IV_IDX   7
#define S2PER_IV_IDX   8
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

#define TX_PIN 1
#define PP_SOL_ENGAGE_MSEC 10
#define PP_SPINUP_MSEC 750
#define PP_FIRE_MSEC 100

#define PP_FIRE_USEC_MIN 920
#define PP_FIRE_USEC_MAX 1760

#define TELLTALE_PIN 13     //D13 B5 SCK LED_BUILTIN
#define TELLTALE_BIT 0X20
#define SET_TELLTALE()      PORTB |= TELLTALE_BIT
#define CLR_TELLTALE()      PORTB &= ~TELLTALE_BIT

#define SOLENOID1_PIN  6    //D6
#define STEPPER4_DIR   6
#define AUX5           6    //D6
#define SET_SOL1()        PORTD |= SOLENOID1_BIT
#define CLR_SOL1()        PORTD &= ~SOLENOID1_BIT

#define SOLENOID2_PIN  5    //D5
#define SOLENOID2_BIT  0X20
#define STEPPER4_STEP  5
#define STEPPER4_STEP_BIT 0X20
#define AUX4           5    //D5
//Step pin is active low
#define SET_STEP4()        PORTD &= ~STEPPER4_STEP_BIT
#define CLR_STEP4()        PORTD |= STEPPER4_STEP_BIT

#define SET_SOL2()        PORTD |= SOLENOID2_BIT
#define CLR_SOL2()        PORTD &= ~SOLENOID2_BIT

#define SOLENOID3_PIN  4    //D4
#define SOLENOID3_BIT  0X10
#define AUX3           4    //D4
//Step pin is active low
#define SET_SOL3()        PORTD |= SOLENOID3_BIT
#define CLR_SOL3()        PORTD &= ~SOLENOID3_BIT

#define SOLENOID4_PIN  3    //D3
#define SOLENOID4_BIT  0X08
#define AUX2           3    //D3
#define SET_SOL4()        PORTD |= SOLENOID4_BIT
#define CLR_SOL4()        PORTD &= ~SOLENOID4_BIT

#define AUX1  2             //D2

#define SERVO1_PIN 12       //D12 B4
#define STEPPER3_DIR  12

#define STEPPER1_DIR  11
#define SERVO2_PIN 11       //D11 B3

#define STEPPER1_STEP  10
#define STEPPER1_STEP_BIT   0X04
#define SERVO3_PIN 10       //D10 B2
//Step pin is active low
#define SET_STEP1()        PORTB &= ~STEPPER1_STEP_BIT
#define CLR_STEP1()        PORTB |= STEPPER1_STEP_BIT

#define STEPPER3_STEP  9
#define STEPPER3_STEP_BIT   0X02
#define SERVO4_PIN 9        //D09 B1
//Step pin is active low
#define SET_STEP3()        PORTB &= ~STEPPER3_STEP_BIT
#define CLR_STEP3()        PORTB |= STEPPER3_STEP_BIT

#define STEPPER2_DIR  8
#define SERVO5_PIN 8        //D08 B0

#define STEPPER2_STEP  7    //D07 D7
#define STEPPER2_STEP_BIT   0X80
#define SERVO6_PIN 7        //D07 D7

#define SET_SOL1()        PORTD &= ~STEPPER2_STEP_BIT

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

void Fire();
typedef union

{
    U16 I;
    U8 B[2];
}BWUnion;

//Stepping and acc/decel ramp defs
#define RAMP_TIME_INCR 0.001
#define PER_CNT_INTRV  4e-6
#define ACCEL  10000.0
#define VELMIN 250.0
#define VELMAX 12500.0
#define RAMP_IDLE  0
#define RAMP_UP    1
#define RAMP_LIM   2
#define RAMP_DOWN  3
