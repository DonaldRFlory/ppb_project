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

namespace TestApp
{

	public partial class Form1 : Form
    {
        private void UpdateHardware(bool ForceFlag)
        {
        }
        //we have notion of last value set to hardware for servos,
        //and if they have changed, we send commands to hardware
        //we might as well use trackbar position as master record
        //of where servo is.



       //so we need a function to update the servo on the board when we change
       //it in any way in UI.
       //1) When we change limits and force a change in value. The
       //     trackbar just quietly changes value without telling anyone.

       //2) When sweep function changes it. Sweep function should be monitoring
       //   trackbar limits and modifying it's ramping accordingly.


        //private void UpdateTRBServ0()
        //{
        //    decimal Value = trbServ0.Value;
        //    Value = Value < trbServ0.Minimum ? trbServ0.Minimum : Value > trbServ0.Maximum ? trbServ0.Maximum : Value;
        //    trbServ0.Value = (int)Value;
        //}

        private void InitServoTab()
        {
           // UpdateTRB0();
        }
}
}
