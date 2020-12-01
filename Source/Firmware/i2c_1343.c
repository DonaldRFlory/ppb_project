/*****************************************************************************
 *   i2c.c:  I2C C file for NXP LPC13xx Family Microprocessors
 *
 *   Copyright(C) 2008, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2008.07.19  ver 1.00    Preliminary version, first Release
 *
*****************************************************************************/
#include "LPC13xx.h"			/* LPC134x Peripheral Registers */
#include "type.h"
#include "i2c_1343.h"
volatile uint32_t I2CMasterState = I2CSTATE_IDLE;
volatile uint32_t I2CSlaveState = I2CSTATE_IDLE;

volatile uint32_t I2CMode;

volatile uint8_t I2CMasterBuffer[BUFSIZE];
volatile uint8_t I2CSlaveBuffer[BUFSIZE];
volatile uint32_t I2CCount = 0;
volatile uint32_t I2CReadLength;
volatile uint32_t I2CWriteLength;

volatile uint32_t RdIndex = 0;
volatile uint32_t WrIndex = 0;

/* 
From device to device, the I2C communication protocol may vary, 
in the example below, the protocol uses repeated start to read data from or 
write to the device:
For master read: the sequence is: STA,Addr(W),offset,RE-STA,Addr(r),data...STO 
for master write: the sequence is: STA,Addr(W),offset,RE-STA,Addr(w),data...STO
Thus, in state 8, the address is always WRITE. in state 10, the address could 
be READ or WRITE depending on the I2C command.
*/   

/*****************************************************************************
** Function name:		I2C_IRQHandler
**
** Descriptions:		I2C interrupt handler, deal with master mode only.
**
** parameters:			None
** Returned value:		None
** 
*****************************************************************************/


void I2C_IRQHandler(void) 
{
  uint8_t StatValue;

  /* this handler deals with master read and master write only */
  StatValue = LPC_I2C->I2C0STAT;
  switch ( StatValue )
  {
	case 0x08:			/* A Start condition is issued. */
	WrIndex = 0;
	LPC_I2C->I2C0DAT = I2CMasterBuffer[WrIndex++];
	LPC_I2C->I2C0CONCLR = (I2CONCLR_SIC | I2CONCLR_STAC);
	I2CMasterState = I2CSTATE_PENDING;
	break;
	
	case 0x10:			/* A repeated started is issued */
	RdIndex = 0;
	/* Send SLA with R bit set, */
	LPC_I2C->I2C0DAT = I2CMasterBuffer[WrIndex++];
	LPC_I2C->I2C0CONCLR = (I2CONCLR_SIC | I2CONCLR_STAC);
	break;
	
	case 0x18:			/* Regardless, it's a ACK */
        LPC_I2C->I2C0DAT = I2CMasterBuffer[WrIndex++];
        LPC_I2C->I2C0CONCLR = I2CONCLR_SIC;
	break;
	
	case 0x20:
		/*
		 * SLA+W has been transmitted; NOT ACK has been received.
		 * Send a stop condition to terminate the transaction
		 * and signal I2CEngine the transaction is aborted.
		 */
		LPC_I2C->I2C0CONSET = I2CONSET_STO;
		LPC_I2C->I2C0CONCLR = I2CONCLR_SIC;
		I2CMasterState = I2CSTATE_SLA_NACK;
		break;

	case 0x28:
		/*
		 * Data in I2DAT has been transmitted; ACK has been received.
		 * Continue sending more bytes as long as there are bytes to send
		 * and after this check if a read transaction should follow.
		 */
		if ( WrIndex < I2CWriteLength )
		{
			/* Keep writing as long as bytes avail */
			LPC_I2C->I2C0DAT = I2CMasterBuffer[WrIndex++];
		}
		else
		{
			if ( I2CReadLength != 0 )
			{
				/* Send a Repeated START to initialize a read transaction */
				/* (handled in state 0x10)                                */
				LPC_I2C->I2C0CONSET = I2CONSET_STA;	/* Set Repeated-start flag */
			}
			else
			{
				I2CMasterState = I2CSTATE_ACK;
				LPC_I2C->I2C0CONSET = I2CONSET_STO;      /* Set Stop flag */
			}
		}
		LPC_I2C->I2C0CONCLR = I2CONCLR_SIC;
		break;

	case 0x30:
		/*
		 * Data byte in I2DAT has been transmitted; NOT ACK has been received
		 * Send a STOP condition to terminate the transaction and inform the
		 * I2CEngine that the transaction failed.
		 */
		LPC_I2C->I2C0CONSET = I2CONSET_STO;
		LPC_I2C->I2C0CONCLR = I2CONCLR_SIC;
		I2CMasterState = I2CSTATE_NACK;


	case 0x38:		/* Arbitration lost, in this example, we don't
					deal with multiple master situation */
 		/*
		 * Arbitration loss in SLA+R/W or Data bytes.
		 * This is a fatal condition, the transaction did not complete due
		 * to external reasons (e.g. hardware system failure).
		 * Inform the I2CEngine of this and cancel the transaction
		 * (this is automatically done by the I2C hardware)
		 */
		I2CMasterState = I2CSTATE_ARB_LOSS;
		LPC_I2C->I2C0CONCLR = I2CONCLR_SIC;
   break;
    
	case 0x40:
		/*
		 * SLA+R has been transmitted; ACK has been received.
		 * Initialize a read.
		 * Since a NOT ACK is sent after reading the last byte,
		 * we need to prepare a NOT ACK in case we only read 1 byte.
		 */
		if ( I2CReadLength == 1 )
		{
			/* last (and only) byte: send a NACK after data is received */
			LPC_I2C->I2C0CONCLR = I2CONCLR_AAC;
		}
		else
		{
			/* more bytes to follow: send an ACK after data is received */
			LPC_I2C->I2C0CONSET = I2CONSET_AA;
		}
        LPC_I2C->I2C0CONCLR = I2CONCLR_SIC;	/* Clear SI flag */
		break;
    
    
  	case 0x48:
		/*
		 * SLA+R has been transmitted; NOT ACK has been received.
		 * Send a stop condition to terminate the transaction
		 * and signal I2CEngine the transaction is aborted.
		 */
        LPC_I2C->I2C0CONSET = I2CONSET_STO;	/* Set Stop flag */ 
        LPC_I2C->I2C0CONCLR = I2CONCLR_SIC;	/* Clear SI flag */
		I2CMasterState = I2CSTATE_SLA_NACK;
		break;

	case 0x50:
		/*
		 * Data byte has been received; ACK has been returned.
		 * Read the byte and check for more bytes to read.
		 * Send a NOT ACK after the last byte is received
		 */
		I2CSlaveBuffer[RdIndex++] = LPC_I2C->I2C0DAT;
		if ( RdIndex < (I2CReadLength-1) )
		{
			/* lmore bytes to follow: send an ACK after data is received */
			LPC_I2C->I2C0CONSET = I2CONSET_AA;
		}
		else
		{
			/* last byte: send a NACK after data is received */
			LPC_I2C->I2C0CONCLR = I2CONCLR_AAC;
		}
        LPC_I2C->I2C0CONCLR = I2CONCLR_SIC;	/* Clear SI flag */
		break;
  
 	case 0x58:
		/*
		 * Data byte has been received; NOT ACK has been returned.
		 * This is the last byte to read.
		 * Generate a STOP condition and flag the I2CEngine that the
		 * transaction is finished.
		 */
		I2CSlaveBuffer[RdIndex++] = LPC_I2C->I2C0DAT;
		I2CMasterState = I2CSTATE_ACK;
        LPC_I2C->I2C0CONSET = I2CONSET_STO;	/* Set Stop flag */ 
        LPC_I2C->I2C0CONCLR = I2CONCLR_SIC;	/* Clear SI flag */
		break;
   
    
    
	default:
        LPC_I2C->I2C0CONCLR = I2CONCLR_SIC;	
        break;
  }
  return;
}



/*****************************************************************************
** Function name:		I2CStart
**
** Descriptions:		Create I2C start condition, a timeout
**				value is set if the I2C never gets started,
**				and timed out. It's a fatal error. 
**
** parameters:			None
** Returned value:		true or false, return false if timed out
** 
*****************************************************************************/
U32 I2CStart( void )
{
  uint32_t timeout = 0;
  //uint32_t retVal = FALSE;
 
  /*--- Issue a start condition ---*/
  LPC_I2C->I2C0CONSET = I2CONSET_STA;	/* Set Start flag */
    
  /*--- Wait until START transmitted ---*/
  while((I2CMasterState != I2CSTATE_PENDING) && (timeout < MAX_TIMEOUT))
  {
    timeout++;
  }
  
  return (timeout < MAX_TIMEOUT);
}



/*****************************************************************************
** Function name:		I2CStop
**
** Descriptions:		Set the I2C stop condition, if the routine
**				never exit, it's a fatal bus error.
**
** parameters:			None
** Returned value:		true or never return
** 
*****************************************************************************/
U32 I2CStop( void )
{
  uint32_t timeout = 0;

  LPC_I2C->I2C0CONSET = I2CONSET_STO;      /* Set Stop flag */ 
  LPC_I2C->I2C0CONCLR = I2CONCLR_SIC;  /* Clear SI flag */ 
            
  /*--- Wait for STOP detected ---*/
  while(( LPC_I2C->I2C0CONSET & I2CONSET_STO ) && (timeout < MAX_TIMEOUT))
  {
        timeout++;
  }
  return TRUE;
}

/*****************************************************************************
** Function name:		I2CInit
**
** Descriptions:		Initialize I2C controller
**
** parameters:			I2c mode is either MASTER or SLAVE
** Returned value:		true or false, return false if the I2C
**				interrupt handler was not installed correctly
** 
*****************************************************************************/
void I2CInit(void) 
{

	//--- Reset registers ---
	LPC_SYSCON->PRESETCTRL |= (0x01<<1);		// Remove reset

  	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<5);		// Power I2C
  	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<16);		// Power IOCON

	//-Set pins for standard mode I2c
    // LPC_IOCON->PIO0_4 = 0x00000001;       //-SCL
    // LPC_IOCON->PIO0_5 = 0x00000001;       //-SDA
  	LPC_IOCON->PIO0_4 &= ~0x307; //~0x3F;					// I2C I/O config 
  	LPC_IOCON->PIO0_4 |= 0x01;					// I2C SCL 
  	LPC_IOCON->PIO0_5 &=  ~0x307; //~0x3F;	
  	LPC_IOCON->PIO0_5 |= 0x01;					// I2C SDA

  	//--- Clear flags ---
  	LPC_I2C->I2C0CONCLR = I2CONCLR_AAC | I2CONCLR_SIC | I2CONCLR_STAC | I2CONCLR_I2ENC;    
    
    //-Set SCL rate: I2CPCLK/(SCLH+SCLL)
//#if FAST_MODE_PLUS
//  	LPC_IOCON->PIO0_4 |= (0x1<<9);
//  	LPC_IOCON->PIO0_5 |= (0x1<<9);
//  	LPC_I2C->I2C0SCLL   = I2SCLL_HS_SCLL;
//  	LPC_I2C->I2C0SCLH   = I2SCLH_HS_SCLH;
//#else
//  	LPC_I2C->I2C0SCLL   = I2SCLL_SCLL;			//- SCL = 72Mhz / (350+350) = 100Khz
//  	LPC_I2C->I2C0SCLH   = I2SCLH_SCLH;
  	LPC_I2C->I2C0SCLL   = 96;			//400Khz
  	LPC_I2C->I2C0SCLH   = 96;
//#endif	

//  if ( I2cMode == I2CSLAVE )
//  {
//	LPC_I2C->I2C0ADR0 = PCF8594_ADDR;
//  }    

    // Clear the STA bit
 //   LPC_I2C->I2C0CONCLR = I2CONCLR_STAC | I2CONCLR_STOC;

    //- Enable Interrupt
    NVIC_EnableIRQ(I2C_IRQn);

  	LPC_I2C->I2C0CONSET = I2CONSET_I2EN;
//  	return( TRUE );
}

/*****************************************************************************
** Function name:		I2CEngine
**
** Descriptions:		The routine to complete a I2C transaction
**				from start to stop. All the intermitten
**				steps are handled in the interrupt handler.
**				Before this routine is called, the read
**				length, write length, I2C master buffer,
**				and I2C command fields need to be filled.
**				see i2cmst.c for more details. 
**
** parameters:			None
** Returned value:		true or false, return false only if the
**				start condition can never be generated and
**				timed out. 
** 
*****************************************************************************/
U32 I2CEngine( void ) 
{
  I2CMasterState = I2CSTATE_IDLE;
  RdIndex = 0;
  WrIndex = 0;
  if ( I2CStart() != TRUE )
  {
	I2CStop();
	return ( FALSE );
  }

  /* wait until the state is a terminal state */
  while (I2CMasterState < 0x100);

  return ( I2CMasterState );
}


void WriteDAC(U8 ID, U16 Value)
{
	I2CWriteLength = 3;
	I2CReadLength = 0;
	if(ID)
	{
		I2CMasterBuffer[0] = MCP4725_ADDR + 2;
	}
	else
	{
		I2CMasterBuffer[0] = MCP4725_ADDR;
	}
	I2CMasterBuffer[1] = (Value & 0XFFF) >> 8;
	I2CMasterBuffer[2] = (U8)Value;

	I2CMasterState = I2CSTATE_IDLE;
	RdIndex = 0;
	WrIndex = 0;
	/*--- Issue a start condition ---*/
	LPC_I2C->I2C0CONSET = I2CONSET_STA;	/* Set Start flag */
}


/*****************************************************************************
*     FUNCTION NAME  -    x
*
*     DESCRIPTION    -    x
*     REENTRANCE     -    No
*     PARAMETERS     -    None
*     RETURNS        -    None
*     NOTES          -	  None
*****************************************************************************/
/* Write SLA(W), address and one data byte */
// void WriteI2C(void)
// {
//   I2CWriteLength = 6;
//   I2CReadLength = 0;
//   I2CMasterBuffer[0] = PCF8594_ADDR;
//   I2CMasterBuffer[1] = 0x00;		/* address */
//   I2CMasterBuffer[2] = 0x55;		/* Data0 */
//   I2CMasterBuffer[3] = 0xAA;		/* Data1 */
//   I2CMasterBuffer[4] = 0x12;		/* Data0 */
//   I2CMasterBuffer[5] = 0x34;		/* Data1 */
//   I2CEngine();
// }
/*****************************************************************************
*     FUNCTION NAME  -    x
*
*     DESCRIPTION    -    x
*     REENTRANCE     -    No
*     PARAMETERS     -    None
*     RETURNS        -    None
*     NOTES          -	  None
*****************************************************************************/
// void ReadI2C(void)
// {
// U16 i;

// for ( i = 0; i < BUFSIZE; i++ )
//   {
// 	I2CSlaveBuffer[i] = 0x00;
//   }
//   /* Write SLA(W), address, SLA(R), and read one byte back. */
//   I2CWriteLength = 2;
//   I2CReadLength = 4;
//   I2CMasterBuffer[0] = PCF8594_ADDR;
//   I2CMasterBuffer[1] = 0x00;		/* address */
//   I2CMasterBuffer[2] = PCF8594_ADDR | RD_BIT;
//   I2CEngine();
// //  return 0;
// }
/******************************************************************************
**                            End Of File
******************************************************************************/

