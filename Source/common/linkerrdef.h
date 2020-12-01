//--------------------------------------------------------------------------
//
//   Copyright (C) Bio-Rad Laboratories 2019
//
//   Purpose:   Defines link errors for CFX third generation (and others).
//
//   Created by:  Don Flory
//   Date:  2-22-2019
//
// --------------------------------------------------------------------------
#ifndef LINK_ERROR_H
#define LINK_ERROR_H
#endif
ErrorDef(LE_NO_ERROR,  Link transaction completed successfully)
ErrorDef(LE_BAD_PARAM,		  Bad parameters in link call)
ErrorDef(LE_BAD_COMMAND, 	  Bad command packet link call)
ErrorDef(LE_BAD_FORMAT,   	  Badly formatted link packet)
ErrorDef(LE_BAD_TABLE,   	  Bad table)
ErrorDef(LE_BAD_CHAN,	  	  Bad channel index)
ErrorDef(LE_BAD_RETURN, 	  Too many bytes returned by slave)
ErrorDef(LE_BAD_BLOCK, 	  Block transfer error)
ErrorDef(LE_BLOCK_SIZE, 	  Block transfer block too large)
ErrorDef(LE_BLOCK_DOWN, 	  Block down error)
ErrorDef(LE_BLOCK_UP, 		  Block up error)
ErrorDef(LE_MUTEX_EXISTS,     Attempt to create existing mutex)				//Start of newer third generation error definitions:
ErrorDef(LE_MUTEX_CREATE_FAIL,Mutex creation failed)
ErrorDef(LE_MUTEX_CLOSE_FAIL, Mutex close failed)
ErrorDef(LE_MUTEX_TIMEOUT,    Timeout on mutex acquire)
ErrorDef(LE_MUTEX_RELEASE_FAIL, Mutex not released)
ErrorDef(LE_BAD_CHECK_D,       Bad check code on device response)
ErrorDef(LE_BAD_CHECK_I,       Bad check code on instr. response)
ErrorDef(LE_BAD_CRC_CD,        Bad cmd CRC at device)
ErrorDef(LE_BAD_CRC_CI,        Bad cmd CRC at instrument)
ErrorDef(LE_BAD_CRC_RD,        Bad rsp CRC from device) 						//20
ErrorDef(LE_BAD_CRC_RI,        Bad rsp CRC from instrument)
ErrorDef(LE_BAD_FLAGS,         Bad flags in function processing)
ErrorDef(LE_BAD_MODE,	       Bad link mode)
ErrorDef(LE_BAD_FORMAT_CD,     Bad cmd format at device)
ErrorDef(LE_BAD_FORMAT_CI,     Bad cmd format at instrument)
ErrorDef(LE_BAD_FORMAT_M,      Badly formatted link call at master)
ErrorDef(LE_BAD_FORMAT_RD,     Bad rsp format from device)
ErrorDef(LE_BAD_FORMAT_RI,     Bad rsp format from instrument)
ErrorDef(LE_BAD_FUNCODE_M,     Bad function code at master)
ErrorDef(LE_BAD_FUNCODE_S,     Bad function code at slave)  					//30
ErrorDef(LE_BAD_HANDLE,		   Bad handle in link call)
ErrorDef(LE_BAD_LT_CALL_D,     Bad params in LinkTrans. call to device)
ErrorDef(LE_BAD_LT_CALL_I,     Bad params in LinkTrans. call to instr.)
ErrorDef(LE_BAD_MUTEX_HANDLE,  Bad mutex handle)
ErrorDef(LE_BAD_SEND_D,        Error sending packet to device)  				//35
ErrorDef(LE_BAD_SEND_I,        Error sending packet to instrument)
ErrorDef(LE_RESP_TIMEOUT_D,    Time out on device response)
ErrorDef(LE_RESP_TIMEOUT_I,    Time out on instrument response)
ErrorDef(LE_SHORT_PACKET_CD,   Short cmd device)
ErrorDef(LE_SHORT_PACKET_CI,   Short cmd at instrument)                         //40
ErrorDef(LE_SHORT_PACKET_RD,   Short rsp from device)
ErrorDef(LE_SHORT_PACKET_RI,   Short rsp from instrument)
ErrorDef(LE_PARAM_OUT_OF_RANGE,API Param out of range)
ErrorDef(LE_OP_NOT_ALLOWED,    Call not allowed in current state)
ErrorDef(LE_FAULT_CONDITION,   Link function reported fault condition)         //45
ErrorDef(LE_BLOCK_UP_SHORT,    Less than requested block up bytes were returned) //this may in some cases be only a warning
ErrorDef(LE_BLOCK_DOWN_I,      Failed to get block down bytes at remote instrument)
ErrorDef(LE_BLOCK_UP_I,        Failed to send block up bytes at remote instrument)
ErrorDef(LE_FTDI_ERR1,          Error return from FTDI driver at CmdIn)
ErrorDef(LE_FTDI_ERR2,          Error return 2 from FTDI driver at CmdIn)   //50
ErrorDef(LE_CMDIN_TIMEOUT,        Timed out waiting for command packet to complete at CmdIn)
ErrorDef(LE_UNKNOWN, 	   Bad error code)  								//52
ErrorDef(LE_NUM_ERRORS, 	   Bad error code)  								//52
