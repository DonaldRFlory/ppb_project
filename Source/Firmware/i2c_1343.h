/*****************************************************************************
 *   i2c1343.h:  Header file for NXP LPC13xx Family Microprocessors
 *
 *   Copyright(C) 2006, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2006.07.19  ver 1.00    Preliminary version, first Release
 *
******************************************************************************/
#ifndef __I2C_H 
#define __I2C_H
#include "type.h"

#define FAST_MODE_PLUS	0

#define BUFSIZE			32
#define MAX_TIMEOUT		0x00FFFFFF

#define I2CMASTER		0x01
#define I2CSLAVE		0x02

#define PCF8594_ADDR	0xA0
#define READ_WRITE		0x01
#define RD_BIT			0x01

//#define I2C_IDLE			0
//#define I2C_STARTED			1
//#define I2C_RESTARTED		2
//#define I2C_REPEATED_START	3
//#define DATA_ACK			4
//#define DATA_NACK			5

#define I2CSTATE_IDLE       0x000
#define I2CSTATE_PENDING    0x001
#define I2CSTATE_ACK        0x101
#define I2CSTATE_NACK       0x102
#define I2CSTATE_SLA_NACK   0x103
#define I2CSTATE_ARB_LOSS   0x104



#define I2CONSET_I2EN		0x00000040  /* I2C Control Set Register */
#define I2CONSET_AA			0x00000004
#define I2CONSET_SI			0x00000008
#define I2CONSET_STO		0x00000010
#define I2CONSET_STA		0x00000020

#define I2CONCLR_AAC		0x00000004  /* I2C Control clear Register */
#define I2CONCLR_SIC		0x00000008
#define I2CONCLR_STAC		0x00000020
#define I2CONCLR_I2ENC		0x00000040
// ??????
#define I2CONCLR_STOC		0x00000020

#define I2DAT_I2C			0x00000000  /* I2C Data Reg */
#define I2ADR_I2C			0x00000000  /* I2C Slave Address Reg */
#define I2SCLH_SCLH			0x00000180  /* I2C SCL Duty Cycle High Reg */
#define I2SCLL_SCLL			0x00000180  /* I2C SCL Duty Cycle Low Reg */
//#define I2SCLH_SCLH			0x00000350  /* I2C SCL Duty Cycle High Reg */
//#define I2SCLL_SCLL			0x00000350  /* I2C SCL Duty Cycle Low Reg */
#define I2SCLH_HS_SCLH		0x00000020  /* Fast Plus I2C SCL Duty Cycle High Reg */
#define I2SCLL_HS_SCLL		0x00000020  /* Fast Plus I2C SCL Duty Cycle Low Reg */

#define MCP4725_ADDR  0XC0

void WriteDAC(U8 ID, U16 Value);
extern void I2C_IRQHandler( void );
extern void I2CInit(void) ;
extern U32 I2CStart( void );
extern U32 I2CStop( void );
extern U32 I2CEngine( void );

// void WriteI2C(void);
// void ReadI2C(void);

#endif /* end __I2C_H */
/****************************************************************************
**                            End Of File
*****************************************************************************/
