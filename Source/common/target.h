//*****************************************************************************
//    target.h:  Header file for NXP LPC17xx Family Microprocessors
// 				Lens board version
//    Copyright(C) 2008, NXP Semiconductor
//    All rights reserved.
// 
// 
//******************************************************************************/
#ifndef __TARGET_H 
#define __TARGET_H

#ifdef __cplusplus
   extern "C" {
#endif
#include "lpc13xx.h"
#include "system_LPC13xx.h"
#include "type.h"
#define USING_SER_LINK 1
//#define USING_HID_LINK 1
#define USING_HID_LINK 0

#define D2I2C_CMD_ADDR 	0XA0
#define D2I2C_CTRL_ADDR	0XB0
#define BOARD_RESET_KEY		0xA5
#define MAX_D2PACK_BYTES 32  //max link command packet payload for D2 board
#define D2LINK_BUFF_SIZE 33  //but buffer one byte longer to allow for checksum

#define D2LINK_TIMEOUT_MSEC 200 


void PostLogicError(U16 ErrorIdentifier);
#define BAD_EXMODE_LOGIC_ERROR 1
#define RCV_CHAR_LOGIC_ERROR	2
#define SEND_CHAR_LOGIC_ERROR	3

U32 ReadMilliseconds(void);
#define MESSAGE_BUFF_LEN 1000

#ifdef __cplusplus
   }
#endif
 
#endif /* end __TARGET_H */
/******************************************************************************
**                            End Of File
******************************************************************************/
