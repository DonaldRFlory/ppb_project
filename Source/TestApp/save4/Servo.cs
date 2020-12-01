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
        private enum SERV_STAT:int{SS_INIT = 0, SS_UP, SS_TOP, SS_DOWN, SS_BOTTOM };
        private int[] ServoState = new int[4];
        private int[] CycleVal = new int[4];
        private int[] CycleStep = new int[4];
        private int[] CycleLimit = new int[4];
        private int[] CycleTimer = new int[4];
        private int[] CycleDelay = new int[4];
        private int C_ID = 0;

        private void InitServoCycle()
        {
            cb0.Checked =  cb1.Checked = cb2.Checked = cb3.Checked = false;
            Serv0Init();
        }

        private void Serv0Init()
        {
            ServoState[0] = (int)SERV_STAT.SS_INIT;
            CycleDelay[0] = 0;
            tbarServ0.Minimum = (int)nudServ0Min.Value;//these will force value into range
            tbarServ0.Maximum = (int)nudServ0Max.Value;
        }

        private void Serv0Serve()
        {
            //private enum SERV_STAT:int{SS_INIT = 0, SS_UP, SS_TOP, SS_DOWN, SS_BOTTOM };
            //private int[] ServoState = new int[4];
            //private int[] CycleVal = new int[4];
            //private int[] CycleStep = new int[4];
            //private int[] CycleLimit = new int[4];
            //private int[] CycleTimer = new int[4];
            //private int[] CycleDelay = new int[4];
            C_ID = 0;
            CycleTimer[C_ID] += 10;
            if(CycleTimer[C_ID] >= CycleDelay[C_ID])
            {
                CycleTimer[C_ID] = 0;
                switch (ServoState[C_ID])
                {
                    case (int)SERV_STAT.SS_INIT:
                        CycleVal[C_ID] = tbarServ0.Value;
                        CycleLimit[C_ID] = (int)nudServ0Max.Value;
                        CycleStep[C_ID] = (int)nudServ0UpStep.Value;
                        CycleDelay[C_ID] = (int)nudServ0UpDelay.Value;
                        ServoState[C_ID] = (int)SERV_STAT.SS_UP;
                        break;

                    case (int)SERV_STAT.SS_UP:
                        CycleVal[C_ID] += CycleStep[C_ID];
                        if(CycleVal[C_ID] >= CycleLimit[C_ID])
                        {
                            CycleVal[C_ID] = CycleLimit[C_ID];
                            ServoState[C_ID] = (int)SERV_STAT.SS_TOP;  //and start top dwell
                            CycleDelay[C_ID] = (int)nudServ0UpDwell.Value;
                        }
                        break;

                    case (int)SERV_STAT.SS_TOP:
                        CycleLimit[C_ID] = (int)nudServ0Min.Value;
                        CycleStep[C_ID] = (int)nudServ0DwnStep.Value;
                        CycleDelay[C_ID] = (int)nudServ0DwnDelay.Value;
                        ServoState[C_ID] = (int)SERV_STAT.SS_DOWN;
                        break;

                    case (int)SERV_STAT.SS_DOWN:
                        CycleVal[C_ID] -= CycleStep[C_ID];
                        if(CycleVal[C_ID] <= CycleLimit[C_ID])
                        {
                            CycleVal[C_ID] = CycleLimit[C_ID];//on or over limit so limit it there
                            ServoState[C_ID] = (int)SERV_STAT.SS_BOTTOM; //and start bottom dwell
                            CycleDelay[C_ID] = (int)nudServ0DwnDwell.Value;
                        }
                        break;

                    case (int)SERV_STAT.SS_BOTTOM:
                        CycleLimit[C_ID] = (int)nudServ0Max.Value;
                        CycleStep[C_ID] = (int)nudServ0UpStep.Value;
                        CycleDelay[C_ID] = (int)nudServ0UpDelay.Value;
                        ServoState[C_ID] = (int)SERV_STAT.SS_UP;
                        break;

                }
                tbarServ0.Value = CycleVal[C_ID];
                SetServoUsec((U8)C_ID, (U16)tbarServ0.Value);
            }
        }

        private void Serv1Serve()
        {
            //private enum SERV_STAT:int{SS_INIT = 0, SS_UP, SS_TOP, SS_DOWN, SS_BOTTOM };
            //private int[] ServoState = new int[4];
            //private int[] CycleVal = new int[4];
            //private int[] CycleStep = new int[4];
            //private int[] CycleLimit = new int[4];
            //private int[] CycleTimer = new int[4];
            //private int[] CycleDelay = new int[4];
            C_ID = 1;
            CycleTimer[C_ID] += 10;
            if(CycleTimer[C_ID] >= CycleDelay[C_ID])
            {
                CycleTimer[C_ID] = 1;
                switch (ServoState[C_ID])
                {
                    case (int)SERV_STAT.SS_INIT:
                        CycleVal[C_ID] = tbarServ1.Value;
                        CycleLimit[C_ID] = (int)nudServ1Max.Value;
                        CycleStep[C_ID] = (int)nudServ1UpStep.Value;
                        CycleDelay[C_ID] = (int)nudServ1UpDelay.Value;
                        ServoState[C_ID] = (int)SERV_STAT.SS_UP;
                        break;

                    case (int)SERV_STAT.SS_UP:
                        CycleVal[C_ID] += CycleStep[C_ID];
                        if(CycleVal[C_ID] >= CycleLimit[C_ID])
                        {
                            CycleVal[C_ID] = CycleLimit[C_ID];
                            ServoState[C_ID] = (int)SERV_STAT.SS_TOP;  //and start top dwell
                            CycleDelay[C_ID] = (int)nudServ1UpDwell.Value;
                        }
                        break;

                    case (int)SERV_STAT.SS_TOP:
                        CycleLimit[C_ID] = (int)nudServ1Min.Value;
                        CycleStep[C_ID] = (int)nudServ1DwnStep.Value;
                        CycleDelay[C_ID] = (int)nudServ1DwnDelay.Value;
                        ServoState[C_ID] = (int)SERV_STAT.SS_DOWN;
                        break;

                    case (int)SERV_STAT.SS_DOWN:
                        CycleVal[C_ID] -= CycleStep[C_ID];
                        if(CycleVal[C_ID] <= CycleLimit[C_ID])
                        {
                            CycleVal[C_ID] = CycleLimit[C_ID];//on or over limit so limit it there
                            ServoState[C_ID] = (int)SERV_STAT.SS_BOTTOM; //and start bottom dwell
                            CycleDelay[C_ID] = (int)nudServ1DwnDwell.Value;
                        }
                        break;

                    case (int)SERV_STAT.SS_BOTTOM:
                        CycleLimit[C_ID] = (int)nudServ1Max.Value;
                        CycleStep[C_ID] = (int)nudServ1UpStep.Value;
                        CycleDelay[C_ID] = (int)nudServ1UpDelay.Value;
                        ServoState[C_ID] = (int)SERV_STAT.SS_UP;
                        break;

                }
                tbarServ1.Value = CycleVal[C_ID];
                SetServoUsec((U8)C_ID, (U16)tbarServ1.Value);
            }
        }

        private void Serv2Serve()
        {
            //private enum SERV_STAT:int{SS_INIT = 0, SS_UP, SS_TOP, SS_DOWN, SS_BOTTOM };
            //private int[] ServoState = new int[4];
            //private int[] CycleVal = new int[4];
            //private int[] CycleStep = new int[4];
            //private int[] CycleLimit = new int[4];
            //private int[] CycleTimer = new int[4];
            //private int[] CycleDelay = new int[4];
            C_ID = 2;
            CycleTimer[C_ID] += 10;
            if(CycleTimer[C_ID] >= CycleDelay[C_ID])
            {
                CycleTimer[C_ID] = 0;
                switch (ServoState[C_ID])
                {
                    case (int)SERV_STAT.SS_INIT:
                        CycleVal[C_ID] = tbarServ2.Value;
                        CycleLimit[C_ID] = (int)nudServ2Max.Value;
                        CycleStep[C_ID] = (int)nudServ2UpStep.Value;
                        CycleDelay[C_ID] = (int)nudServ2UpDelay.Value;
                        ServoState[C_ID] = (int)SERV_STAT.SS_UP;
                        break;

                    case (int)SERV_STAT.SS_UP:
                        CycleVal[C_ID] += CycleStep[C_ID];
                        if(CycleVal[C_ID] >= CycleLimit[C_ID])
                        {
                            CycleVal[C_ID] = CycleLimit[C_ID];
                            ServoState[C_ID] = (int)SERV_STAT.SS_TOP;  //and start top dwell
                            CycleDelay[C_ID] = (int)nudServ2UpDwell.Value;
                        }
                        break;

                    case (int)SERV_STAT.SS_TOP:
                        CycleLimit[C_ID] = (int)nudServ2Min.Value;
                        CycleStep[C_ID] = (int)nudServ2DwnStep.Value;
                        CycleDelay[C_ID] = (int)nudServ2DwnDelay.Value;
                        ServoState[C_ID] = (int)SERV_STAT.SS_DOWN;
                        break;

                    case (int)SERV_STAT.SS_DOWN:
                        CycleVal[C_ID] -= CycleStep[C_ID];
                        if(CycleVal[C_ID] <= CycleLimit[C_ID])
                        {
                            CycleVal[C_ID] = CycleLimit[C_ID];//on or over limit so limit it there
                            ServoState[C_ID] = (int)SERV_STAT.SS_BOTTOM; //and start bottom dwell
                            CycleDelay[C_ID] = (int)nudServ2DwnDwell.Value;
                        }
                        break;

                    case (int)SERV_STAT.SS_BOTTOM:
                        CycleLimit[C_ID] = (int)nudServ2Max.Value;
                        CycleStep[C_ID] = (int)nudServ2UpStep.Value;
                        CycleDelay[C_ID] = (int)nudServ2UpDelay.Value;
                        ServoState[C_ID] = (int)SERV_STAT.SS_UP;
                        break;

                }
                tbarServ2.Value = CycleVal[C_ID];
                SetServoUsec((U8)C_ID, (U16)tbarServ2.Value);
            }
        }

        private void Serv3Serve()
        {
            //private enum SERV_STAT:int{SS_INIT = 0, SS_UP, SS_TOP, SS_DOWN, SS_BOTTOM };
            //private int[] ServoState = new int[4];
            //private int[] CycleVal = new int[4];
            //private int[] CycleStep = new int[4];
            //private int[] CycleLimit = new int[4];
            //private int[] CycleTimer = new int[4];
            //private int[] CycleDelay = new int[4];
            C_ID = 3;
            CycleTimer[C_ID] += 10;
            if(CycleTimer[C_ID] >= CycleDelay[C_ID])
            {
                CycleTimer[C_ID] = 0;
                switch (ServoState[C_ID])
                {
                    case (int)SERV_STAT.SS_INIT:
                        CycleVal[C_ID] = tbarServ3.Value;
                        CycleLimit[C_ID] = (int)nudServ3Max.Value;
                        CycleStep[C_ID] = (int)nudServ3UpStep.Value;
                        CycleDelay[C_ID] = (int)nudServ3UpDelay.Value;
                        ServoState[C_ID] = (int)SERV_STAT.SS_UP;
                        break;

                    case (int)SERV_STAT.SS_UP:
                        CycleVal[C_ID] += CycleStep[C_ID];
                        if(CycleVal[C_ID] >= CycleLimit[C_ID])
                        {
                            CycleVal[C_ID] = CycleLimit[C_ID];
                            ServoState[C_ID] = (int)SERV_STAT.SS_TOP;  //and start top dwell
                            CycleDelay[C_ID] = (int)nudServ3UpDwell.Value;
                        }
                        break;

                    case (int)SERV_STAT.SS_TOP:
                        CycleLimit[C_ID] = (int)nudServ3Min.Value;
                        CycleStep[C_ID] = (int)nudServ3DwnStep.Value;
                        CycleDelay[C_ID] = (int)nudServ3DwnDelay.Value;
                        ServoState[C_ID] = (int)SERV_STAT.SS_DOWN;
                        break;

                    case (int)SERV_STAT.SS_DOWN:
                        CycleVal[C_ID] -= CycleStep[C_ID];
                        if(CycleVal[C_ID] <= CycleLimit[C_ID])
                        {
                            CycleVal[C_ID] = CycleLimit[C_ID];//on or over limit so limit it there
                            ServoState[C_ID] = (int)SERV_STAT.SS_BOTTOM; //and start bottom dwell
                            CycleDelay[C_ID] = (int)nudServ3DwnDwell.Value;
                        }
                        break;

                    case (int)SERV_STAT.SS_BOTTOM:
                        CycleLimit[C_ID] = (int)nudServ3Max.Value;
                        CycleStep[C_ID] = (int)nudServ3UpStep.Value;
                        CycleDelay[C_ID] = (int)nudServ3UpDelay.Value;
                        ServoState[C_ID] = (int)SERV_STAT.SS_UP;
                        break;

                }
                tbarServ3.Value = CycleVal[C_ID];
                SetServoUsec((U8)C_ID, (U16)tbarServ3.Value);
            }
        }
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
            InitServoCycle();
           // UpdateTRB0();
        }
}
}
