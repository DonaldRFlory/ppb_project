//***************************************************************************
//   servocomm.h
//
//   Description:
//     This file contains definitions for the serial link from main board to 
//     Servo control board.
//
//**************************************************************************

#ifndef __SERVOCOMM_H 
#define __SERVOCOMM_H

#define LINK_TIMEOUT 20  //max 20 milliseconds allowed for servo board transaction
#define SERVO_BUFF_SIZE 256


//States of Servo board serial link handler
#define L_IDLE            	0
#define L_EXECUTE			1
#define L_RESPONSE			2
#define L_RECEIVE			3

#endif /* end __SERVOCOMM_H */
//****************************************************************************
//                            End Of File
//****************************************************************************
