//--------------------------------------------------------------------------*
//                Copyright (C) Bio-Rad Laboratories 2009
//
//--------------------------------------------------------------------------*/
// $Revision: 7 $
// $Author: Dflory $
// $Date: 9/30/12 8:51p $
//
// $History: stats.h $
// 
// 
//---------------------------------------------------------------------------
#ifndef apistats_h
#define apistats_h
#define COMB_API_STAT(SubSys, FIdx, APIStat) ((((U32)SubSys)<<24)|(((U32)(FIdx&0xFF)) << 16)|((U32)APIStat))
#define INVAL_SUBSYS 0XFF
#define INVAL_FIDX   0XFF
//API_STAT: enum for Function Return Status from ApiDLL
enum _API_STAT
{
  STAT_LINK_RSLT_OK = 0,            // Function is completed without Error
  STAT_OK = 0,						// Function is completed without Error
  STAT_PARAM_OUT_OF_RANGE,        // Parameters sent in Function are out of Range
  STAT_RET_VALUE_OUT_OF_RANGE,		// Returned parameter out of Range
  STAT_FW_FILE_DATA_CRC_ERROR,		// Firmware File Data CRC Error
  STAT_MAIN_FW_KEY_ERASE_ERROR,     // Error in Erasing Main FW present Key
  STAT_FW_UPGRADE_ERROR,			// Error in Upgrading Firmware
  STAT_INSTR_NOT_FOUND,		// No connection to device
  STAT_NULL_PTR,			// Param pointer is null
  STAT_OP_NOT_ALLOWED,       // Operation not allowed due to present state of firmware/hardware
  STAT_COMM_BAD_CALL,       //Link bad parameters in call 
  STAT_COMM_BAD_SEND,       //Link error sending packet
  STAT_COMM_BAD_LT_CALL,    //Link bad send payload
  STAT_COMM_RESP_TIMEOUT,   //Link timeout on response
  STAT_COMM_BAD_RESP,       //Link bad response packet length
  STAT_COMM_SLAVE_STATUS,   //Link slave error status
  STAT_COMM_BAD_RTN_IDX,    //Link bad returned function index
  STAT_COMM_BAD_CMN_CHN,    //Link bad channel index for common function
  STAT_COMM_UNEXP_RESP,		//Link response before command send)
  STAT_COMM_BAD_ERR_CODE,   //Link unrecognized error code
  STAT_COMM_CLOSE_FAIL,
  STAT_FC_DEVICES_WITH_SAME_SERIAL_NUMBER,	//Multiple fraction collector with same serial number
  STAT_DEVICE_HANDLE_INVALID,		//Device Handle no more valid 
  STAT_INVALID_NO_OF_ACTIVE_FC_DEVICES,   //Number of active FC devices is invalid, this error can happen, when there was no device connected, and an event of device disconnect is received
  STAT_MORE_THEN_ONE_DEVICE_CONNECTED,	//More then one FC device connected, this error is generated only during firmware upgrade
  STAT_FW_COMPONENT_NOT_FOUND,	// Could not find desired upgrade component in combined firmware image
  STAT_FW_COMPONENT_BAD_PID,   //mismatch of upgrade component PID, file not appropriate for instrument
  STAT_FW_COMPONENT_BAD,	// Firmware component is bad
  STAT_FW_WITH_BOOT_LOADER_ONLY, // Current firmware is boot loader, so boot firmware update not allowed
  STAT_BAD_API_HANDLE,			 //Invalid handle supplied to API call
//We are leaving 100 values for link errors in _API_STAT enum
//The list below matched the link errors at time of writing.(missing the STAT_ lead-in)
//Link errors are translated to API errors via LinkToAPIStat() function
//which has translation array assigning the API STAT_ value to each link error
  	 //Begin link errors. We dont need a STAT_ for LE_NO_ERROR as we tranlate it to STAT_OK	
  STAT_LE_BAD_PARAM = 101,			
  STAT_LE_BAD_FUNCODE, 		
  STAT_LE_BAD_COMMAND, 		
  STAT_LE_BAD_FORMAT,   		
  STAT_LE_BAD_TABLE,   		
  STAT_LE_BAD_LT_PACKET_LEN,	
  STAT_LE_BAD_LT_CALL,		
  STAT_LE_BAD_SEND,			
  STAT_LE_RESPONSE_TIMEOUT,	
  STAT_LE_BAD_RESP,			
  STAT_LE_BAD_RESP_LEN,		
  STAT_LE_BAD_RESP_CS,		
  STAT_LE_BAD_STATUS,		
  STAT_LE_SLAVE_STATUS,		
  STAT_LE_COMM_FAIL,			
  STAT_LE_BAD_CMN_CHAN,		
  STAT_LE_BAD_CHAN,	  		
  STAT_LE_SHORT_PACKET,		
  STAT_LE_BAD_CHECK, 		
  STAT_LE_BAD_RETURN, 		
  STAT_LE_BAD_PARAM_FLAG, 	
  STAT_LE_BAD_BLOCK, 		
  STAT_LE_BLOCK_SIZE, 		
  STAT_LE_BLOCK_DOWN, 		
  STAT_LE_BLOCK_UP, 			
  STAT_LE_FUN_RETURN, 		
  STAT_LE_UNEXP_RESP, 		
  STAT_LE_SEND_BYTE_TO, 		
  STAT_LE_BABBLE, 			
  STAT_LE_OVERRUN, 			
  STAT_LE_UNDERRUN, 			
  STAT_LE_UNKNOWN, 			//Best to avoid changing this block of link errors
							//if new link errors are introduced, add the
							//corresponsing API_STAT below:
  STAT_END_LINK_ERRORS = 199,  //End link errors	
 };

typedef enum _RET_RESULT RET_RESULT;    // Return Status of API Function calls
//typedef uint32_t LINK_STAT;
typedef U32 API_STAT;

#endif // apistats_h
