using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Runtime.InteropServices;
using System.Linq;
using System.Text;
using System.IO;
using System.Threading.Tasks;
using System.Windows.Forms;
using API_DEVICE_HANDLE = System.IntPtr;
using U32 = System.UInt32;
using U16 = System.UInt16;
using U8 = System.Byte;

namespace TestApp
{
	public partial class Form1 : Form
    {

        private LinkStatusCB LinkStatusFPtr;
    	private LoggingCB LoggingFPtr;

        public void SetupCallbackFunctions()
        {
			//Set up the DFlogging callback in the dll
			LoggingFPtr = new LoggingCB(LoggingCallbackFunction);
			try
			{
				//I think the only way this hits the catch is if the call fails because
				//DLL is not found, causing an exception.
				API.RegisterLoggingCallback(LoggingFPtr);
			}
        	catch
        	{
				StringBuilder SB = new StringBuilder();
        	    SB.AppendFormat("SetLoggingCallback failed\r\n");
        	    MessageBox.Show(SB.ToString());
        	}

			//Set up the LinkStatus callback in the dll
			LinkStatusFPtr = new LinkStatusCB(LinkStatusCallbackFunction);
			try
			{
			    API.RegisterLinkStatusCallback(LinkStatusFPtr);
			}
        	catch
        	{
				StringBuilder SB = new StringBuilder();
        	    SB.AppendFormat("SetLinkStatusCallback failed\r\n");
        	    MessageBox.Show(SB.ToString());
        	}
        }

		[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
		public delegate void LoggingCB(
			[MarshalAs(UnmanagedType.LPArray, SizeParamIndex=1)]
			char []String,
			U32 StrLen,
			bool ContinueEntry);

		[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
		public delegate void LinkStatusCB(
			U8  Stat,
			U8	FIdx,
			U8	Channel,
			U8	CommType);

		public void LinkStatusCallbackFunction(U8 Stat, U8 FIdx, U8 Channel, U8 CommType)
 		{
 		    StringBuilder SB = new StringBuilder();

			if (!APIStatusOK(Stat))
			{
				SB.AppendFormat("LinkStat = {0} FIdx = {1} \r\n", Stat, FIdx);

				LogToMessageBuffer(SB.ToString());
				LogFile.Write(SB.ToString());
				LogFile.Flush();
			}
		}

		 public void LoggingCallbackFunction(char[]CString, U32 StrLen, bool ContinueEntry)
 		{
            string LogString = new string(CString, 0, (int)StrLen);

			if(!ContinueEntry)
			{
				LogFile.Write("APILog:");
			}
			LogFile.Write(LogString);
            LogFile.Flush();
			LogToMessageBuffer(LogString);
		}

	}
}
