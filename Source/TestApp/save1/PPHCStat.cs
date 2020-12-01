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

namespace TestApp
{
	enum LED_ID {PPHC_LED_VOLTS = 0, PPHC_LED_AMPS,	PPHC_LED_WATTS,	PPHC_LED_PAUSE,	PPHC_LED_RUN,PPHC_LED_BACKLIGHT};
	enum CTRL_MODE { CM_VOLTS, CM_MILLIAMPS, CM_WATTS };
	
	
    public partial class Form1 : Form
    {
    	int i;
		StringBuilder SB = new StringBuilder(32);
		private void ShowLED(int Index, bool On)
        {
        }

        private void UpdateLCDWindow()
        {
  

		}
		
		private void UpdateControlStatus()
        {
		
        }
    }
}
