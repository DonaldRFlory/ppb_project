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
        private int CycleS0Counter;
        private bool CycleS0Two;
        private U16[] USecVals = new U16[4];
        private U16[] ADVals = new U16[4];
        private int[] ServoState = new int[4];
        private int[] CycleVal = new int[4];
        private int[] CycleStep = new int[4];
        private int[] CycleLimit = new int[4];
        private int[] CycleTimer = new int[4];
        private int[] CycleDelay = new int[4];
        private int C_ID = 0;
        private bool Firing;
        private int FireTimer;

        private void LocSetServoUsec(U8 ServoID, U16 USecValue)
        {
            ServoID &= 3;
            USecVals[ServoID] = USecValue;
        }

        private void InitServoCycle()
        {
            cb0.Checked =  cb1.Checked = cb2.Checked = cb3.Checked = false;
            Serv0Init();
            Serv1Init();
            Serv2Init();
            Serv3Init();
        }

        private void Serv0Init()
        {
            ServoState[0] = (int)SERV_STAT.SS_INIT;
            CycleDelay[0] = 0;
            tbarServ0.Minimum = (int)nudServ0Min.Value;//these will force value into range
            tbarServ0.Maximum = (int)nudServ0Max.Value;
            tbarServ0.Value = (int)tbarServ0.Maximum;//force a change and set to min so text box is updated
            tbarServ0.Value = (int)tbarServ0.Minimum;
            LocSetServoUsec((U8) 0, (U16) tbarServ0.Value);
        }

        private void Serv1Init()
        {
            ServoState[1] = (int)SERV_STAT.SS_INIT;
            CycleDelay[1] = 0;
            tbarServ1.Minimum = (int)nudServ1Min.Value;//these will force value into range
            tbarServ1.Maximum = (int)nudServ1Max.Value;
            tbarServ1.Value = (int)tbarServ1.Maximum;//force a change and set to min so text box is updated
            tbarServ1.Value = (int)tbarServ1.Minimum;
            LocSetServoUsec((U8) 1, (U16) tbarServ1.Value);
        }

        private void Serv2Init()
        {
            ServoState[2] = (int)SERV_STAT.SS_INIT;
            CycleDelay[2] = 0;
            tbarServ2.Minimum = (int)nudServ2Min.Value;//these will force value into range
            tbarServ2.Maximum = (int)nudServ2Max.Value;
            tbarServ2.Value = (int)tbarServ2.Maximum;//force a change and set to min so text box is updated
            tbarServ2.Value = (int)tbarServ2.Minimum;
            LocSetServoUsec((U8) 2, (U16) tbarServ2.Value);
        }

        private void Serv3Init()
        {
            ServoState[3] = (int)SERV_STAT.SS_INIT;
            CycleDelay[3] = 0;
            tbarServ3.Minimum = (int)nudServ3Min.Value;//these will force value into range
            tbarServ3.Maximum = (int)nudServ3Max.Value;
            tbarServ3.Value = (int)tbarServ3.Maximum;//force a change and set to min so text box is updated
            tbarServ3.Value = (int)tbarServ3.Minimum;
            LocSetServoUsec((U8) 1, (U16) tbarServ3.Value);
        }

        //to avoid exceptions when limits change during cycling
        private void SetTBarValue(int IndexValue)
        {
            IndexValue &= 3;
            int Value = CycleVal[IndexValue];
            switch(IndexValue)
            {
                default:
                    Value = Value < tbarServ0.Minimum ? tbarServ0.Minimum : Value > tbarServ0.Maximum ? tbarServ0.Maximum : Value;
                    tbarServ0.Value = Value;
                    break;

                case 1:
                    Value = Value < tbarServ1.Minimum ? tbarServ1.Minimum : Value > tbarServ1.Maximum ? tbarServ1.Maximum : Value;
                    tbarServ1.Value = Value;
                    break;

                case 2:
                    Value = Value < tbarServ2.Minimum ? tbarServ2.Minimum : Value > tbarServ2.Maximum ? tbarServ2.Maximum : Value;
                    tbarServ2.Value = Value;
                    break;

                case 3:
                    Value = Value < tbarServ3.Minimum ? tbarServ3.Minimum : Value > tbarServ3.Maximum ? tbarServ3.Maximum : Value;
                    tbarServ3.Value = Value;
                    break;

                }
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
                SetTBarValue(C_ID);
                LocSetServoUsec((U8)C_ID, (U16)tbarServ0.Value);
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
                SetTBarValue(C_ID);
                LocSetServoUsec((U8)C_ID, (U16)tbarServ1.Value);
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
                SetTBarValue(C_ID);
                LocSetServoUsec((U8)C_ID, (U16)tbarServ2.Value);
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
                SetTBarValue(C_ID);
                LocSetServoUsec((U8)C_ID, (U16)tbarServ3.Value);
            }
        }
        private void StartFireSequence()
        {
            Firing = true;
            FireTimer = 0;
        }

        private void FireServe()
        {
            if (Firing)
            {
                if (FireTimer++ == 0)
                {
                    tbarServ0.Value = tbarServ0.Maximum;


                }
                else if (FireTimer++ > 15)
                {
                    tbarServ0.Value = tbarServ0.Minimum;
                    Firing = false;
                }

                LocSetServoUsec((U8)0, (U16)tbarServ0.Value);
            }
        }

        private void UpdateSensors()
        {
           DataUpdate(USecVals, 4, ADVals, 4);
           tbarSensor1.Value = ADVals[0] & 0x3FF;
           tbarSensor2.Value = ADVals[1] & 0x3FF;
           tbSensor3.Value = ADVals[2] & 0x3FF;
           tbSensor4.Value = ADVals[3] & 0x3FF;
        }

        private void InitServoTab()
        {
            InitServoCycle();
           // UpdateTRB0();
        }
        private void S0B4()
        {
            tbarServ0.Value = (int) nudS0Pos4.Value;
        }

        private void S0B3()
        {
            tbarServ0.Value = (int) nudS0Pos3.Value;
        }

        private void S0B2()
        {
            tbarServ0.Value = (int) nudS0Pos2.Value;
        }

        private void S0B1()
        {
            tbarServ0.Value = (int) nudS0Pos1.Value;
        }

        private void CycleS0Serve()
        {
            ++CycleS0Counter;
            if(CycleS0Counter >= 12)
            {
                if(CycleS0Two)
                {
                    S0B3();
                }
                else
                {
                    S0B1();
                }
                CycleS0Two = !CycleS0Two;
                CycleS0Counter = 0;
            }
        }
    }
}
