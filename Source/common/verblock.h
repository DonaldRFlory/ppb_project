//---------------------------------------------------------------------------
//
//   Copyright (C) Bio-Rad Laboratories 3016
//
//   Module: VerDate.h
//
//  Purpose: Definition for Life Science firmware standard version block.
//
//   Created by: Don Flory
//   Date: 2-16-2016
//
//   Modified: 
//
//---------------------------------------------------------------------------

#ifndef VERBLOCK_H
#define VERBLOCK_H
#include "type.h"

#define RESTART_MAGIC_ONE 0xA5A5
#define RESTART_MAGIC_TWO 0x1234
#define MAIN_PROGRAM_KEY_WORD 0XA5A51234

//The Bio-Rad VID and several existing device PID's
#define BIORAD_VID  0x0614
#define LOCUST_THERMAL_PID		0x1410
#define LOCUST_OPTICS_PID		0x1411
#define GALILEO_DRIVBD_PID		0x1416
#define TESLA_BOARD_PID			0x058C
#define FRACTION_COLLECTOR_PID	0x040E

#define VERSION_BLOCK_KEY 0xA6252B15
#define BOOT_PROGRAM_ID_VAL 0
#define MAIN_PROGRAM_ID_VAL 1
#define KEY_SECTOR_ID_VAL 2
#define DEF_COMP_ID_MASK 0X0000000F

#define CUR_SSP_VERSION	2

//Standard codes for GetSlaveParameter() and SetSlaveParameter() link functions used
//by Universal Upgrader Utility
#define SSP_SSPVERSION         	1000  // Get only version of Standard Slave Parameters. Starting out at 1
#define SSP_PROGVERSION        	1001  //Get only  Index=0 for running prog version, Index=1 boot program version
#define SSP_CS_START           	1002  //Get/Set
#define SSP_CS_END             	1003  //Get/Set
#define SSP_CS_CALC            	1004  //Get only	 Index: 0 for sum by U8s, 1 for sum by U32's, 2 for ANSI CRC-16 
#define SSP_RESET_CPU          	1005  // Set, requires magic codes SetSlaveParameter(SSP_RESET_CPU, RESTART_MAGIC_ONE, RESTART_MAGIC_TWO);
#define SSP_RESTART_MAGIC	   	1006  // Get-only returns  ((RESTART_MAGIC_ONE << 16)| RESTART_MAGIC_TWO) that is 0XA5A51234. 
#define SSP_ERASE_MAIN_PROG_KEY 1007  //Set only SetSlaveParameter(SSP_ERASE_MAIN_PROGRAM_EKY, RESTART_MAGIC_ONE, RESTART_MAGIC_TWO);
#define SSP_SET_MAIN_PROG_KEY 	1008  //Set only SetSlaveParameter(SSP_ERASE_MAIN_PROGRAM_EKY, RESTART_MAGIC_ONE, RESTART_MAGIC_TWO);
#define SSP_VERBLOCK_ADDRESS   	1009  // Get only
#define SSP_VERBLOCK_LENGTH    	1010  // Get only
#define	SSP_ERASE_START			1011  //Get/Set
#define	SSP_ERASE_END			1012  //Get/Set
#define	SSP_UPGRADE_ERASE		1013  //Set only SetSlaveParameter(SSP_UPGRADE_ERASE, RESTART_MAGIC_ONE, RESTART_MAGIC_TWO);
#define	SSP_MAX_SEND_SIZE		1014  //Get only
#define	SSP_MAX_RETURN_SIZE		1015  //Get only

// The ProgStartAddress and ProgMaxAddress are the runtime locations for
// which the program is linked. This may not be where the program is stored
// in flash if it is loaded form FLASH into RAM before execution. If this
// is the case, StorageOffset specifies the offset from the runtime location
// to the FLASH storage location. It is an unsigned long value which when
// added to the ProgStartAddress with modulo 32 bit addition, produces the
// storage location in Non-volatile memory to which the code is loaded by
// a programmer or update program. It the program executes from FLASH,
// StorageOffset is zero.
// For a system where main program code executes from RAM (usually for
// improved speed with fast RAM and slow FLASH), typically the Boot code
// will execute from FLASH. The key sector has no executable code, and
// also will typically not be copied to RAM. Hence both Boot and Key
// sector components will have StorageOffset of zero.
// NOTE ON TargetID: Component ID will be assumed to be in bottom two bits of
// TargetID. 0 = BOOT, 1=MAIN, 2 = KeySector, 3 = unassigned. This
// will be pulled out by VerDate utility to label combined image file.
// Verdate will support a bit mask for the component ID so the bits used
// may be changed for future projects but VerDate will still assume the
// value of 1 for MAIN and extract that components version to label the file.
//Following was renamed from VERSION_BLOCK to FW_VERSION_BLOCK to differentiate
//it from earlier VERSION_BLOCK used on Western Doc.
typedef struct _FW_VERSION_BLOCK
{
	unsigned long VerBlockKey;
	unsigned long NotVerBlockKey;
	unsigned long ProgKeyAddress;
	unsigned long ProgStartAddress;
	unsigned long ProgMaxAddress;
	unsigned long ProgramVersion;
	unsigned long TargetID;
	unsigned long StorageOffset;
} FW_VERSION_BLOCK;

typedef struct 
{
	unsigned short I1;
	unsigned short I2;
}	TEST_STRUCT;
#endif //of ifndef VERBLOCK_H





