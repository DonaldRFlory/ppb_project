//Lead-in code from LWPart1.CS ----------------------------------
//If viewing in MENUDELEGATES.CS, do not edit! This code is pasted there from LWPart1.CS by LDFUTIL.
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Runtime.InteropServices;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using API_STAT = System.UInt32;
using API_DEVICE_HANDLE = System.IntPtr;
using U32 = System.UInt32;
using U16 = System.UInt16;
using U8 = System.Byte;
using UP_PTR_FLOAT = System.Single;
using DOWN_PTR_FLOAT = System.Single;
using DOWN_PTR_U32 = System.UInt32;
using UP_PTR_U32 = System.UInt32;
using DOWN_PTR_U16 = System.UInt16;
using UP_PTR_U16 = System.UInt16;
using DOWN_PTR_U8 = System.Byte;
using UP_PTR_U8 = System.Byte;

namespace TestApp
{
    public partial class Form1 : Form
    {
		//Constants relating to Menu Utility
		public const U16 HIDEMENU				= 1;
		public const U16 RTNVAL					= 2;
		public const U16 RTNVAL2				= 4;
		public const U16 MEN_STARTUP			= 64;
		
		//Constants relating to standard link functions
		public const U16 RESTART_MAGIC_ONE 		= 0xA5A5;
		public const U16 RESTART_MAGIC_TWO 		= 0x1234;
		public const U32 SSP_CS_START				= 1002;
		public const U32 SSP_CS_END					= 1003;
		public const U32 SSP_CS_CALC				= 1004;
	
		//HID related defines
//		public const U16 BIORAD_VID				= 0x0614;
//		public const U16 FRACTION_COLLECTOR_PID	= 0x040E;
		public const U16 OUR_VID				= 0x0614;
		public const U16 OUR_PID	= 0x040E;
		

		enum SLAVE_PARAMS {SPAR_PROGVERSION = 0, SPAR_MSECS = 6}; //from slavparm.h
		public enum API_RSLT:int {
  								LINK_RSLT_OK = 0, OK=0, PARAM_OUT_OF_RANGE, RET_VALUE_OUT_OF_RANGE,
								FW_FILE_DATA_CRC_ERROR, MAIN_FW_KEY_ERASE_ERROR, FW_UPGRADE_ERROR,
								INSTR_NOT_FOUND, NULL_PTR, OP_NOT_ALLOWED, COMM_BAD_CALL,
								COMM_BAD_SEND, COMM_BAD_LT_CALL, COMM_RESP_TIMEOUT, COMM_BAD_RESP,
								COMM_SLAVE_STATUS, COMM_BAD_RTN_IDX, COMM_BAD_CMN_CHN, COMM_UNEXP_RESP,
								COMM_BAD_ERR_CODE, COMM_CLOSE_FAIL, FC_DEVICES_WITH_SAME_SERIAL_NUMBER,
								DEVICE_HANDLE_INVALID, INVALID_NO_OF_ACTIVE_FC_DEVICES, MORE_THEN_ONE_DEVICE_CONNECTED,
								FW_COMPONENT_NOT_FOUND, FW_COMPONENT_BAD_PID, FW_COMPONENT_BAD,
								FW_WITH_BOOT_LOADER_ONLY, BAD_API_HANDLE
							
						   };

//End Lead-in code file LWPart1.CS ---------------------------

