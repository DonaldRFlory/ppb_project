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
        private bool MaxMin0Top;
        private int CycleS0Counter;
        private bool CycleS0Two;
        public const U16 SERVO_UPDATE_FLAG = 0X8000;
        //For USecVals, first block of 16 is for Arduino bd servo outputs, only first 6 used
        //Second block of 16 is for External PCA9685 board 1
        //Third block of 16 is for External PCA9685 board 2
        //Fourth block of 16 is for External PCA9685 board 3
        //For all values, the low 12 bits are USec value potentialy 0-4095
        //The high bit (0x8000) signals that value has been updated
        //and needs to be updated on Arduino board

        private U16[] USecVals = new U16[64];
        private U16[] InputVals = new U16[10];
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
            if (ServoID > 5) ServoID = 5;
            USecVals[ServoID] = USecValue;
        }

        private void InitServoCycle()
        {
            //cb0.Checked =  cb1.Checked = cb2.Checked = cb3.Checked = false;
            Serv0Init();
            Serv1Init();
            Serv2Init();
            Serv3Init();
        }

        private void Serv0Init()
        {
            ServoState[0] = (int)SERV_STAT.SS_INIT;
            CycleDelay[0] = 0;
            //tbar1.Value = (int)tbar1.Maximum;//force a change and set to min so text box is updated
            //tbar1.Value = (int)tbar1.Minimum;
            LocSetServoUsec((U8) 0, (U16) tbar1.Value);
        }

        private void Serv1Init()
        {
            ServoState[1] = (int)SERV_STAT.SS_INIT;
            CycleDelay[1] = 0;
           //tbarServ1.Value = (int)tbarServ1.Maximum;//force a change and set to min so text box is updated
            //tbarServ1.Value = (int)tbarServ1.Minimum;
     //       LocSetServoUsec((U8) 1, (U16) tbarServ1.Value);
        }

        private void Serv2Init()
        {
            ServoState[2] = (int)SERV_STAT.SS_INIT;
            CycleDelay[2] = 0;
           // tbarServ2.Value = (int)tbarServ2.Maximum;//force a change and set to min so text box is updated
           // tbarServ2.Value = (int)tbarServ2.Minimum;
           // LocSetServoUsec((U8) 2, (U16) tbarServ2.Value);
           // LocSetServoUsec((U8) 4, (U16) tbarServ2.Value); //init shadow servo channel
        }

        private void Serv3Init()
        {
            ServoState[3] = (int)SERV_STAT.SS_INIT;
            CycleDelay[3] = 0;
            //tbarServ3.Value = (int)tbarServ3.Maximum;//force a change and set to min so text box is updated
           // tbarServ3.Value = (int)tbarServ3.Minimum;
            //LocSetServoUsec((U8) 3, (U16) tbarServ3.Value);
            //LocSetServoUsec((U8) 5, (U16) tbarServ3.Value);//init shadow servo channel
        }

        //to avoid exceptions when limits change during cycling
        private void SetTBarValue(int IndexValue)
        {
            IndexValue &= 3;
            int Value = CycleVal[IndexValue];
            switch(IndexValue)
            {
                default:
                    Value = Value < tbar1.Minimum ? tbar1.Minimum : Value > tbar1.Maximum ? tbar1.Maximum : Value;
                    tbar1.Value = Value;
                    break;

                case 1:
                   // Value = Value < tbarServ1.Minimum ? tbarServ1.Minimum : Value > tbarServ1.Maximum ? tbarServ1.Maximum : Value;
                   // tbarServ1.Value = Value;
                    break;

                case 2:
                   // Value = Value < tbarServ2.Minimum ? tbarServ2.Minimum : Value > tbarServ2.Maximum ? tbarServ2.Maximum : Value;
                  //  tbarServ2.Value = Value;
                    break;

                case 3:
                   // Value = Value < tbarServ3.Minimum ? tbarServ3.Minimum : Value > tbarServ3.Maximum ? tbarServ3.Maximum : Value;
                   // tbarServ3.Value = Value;
                    break;

                }
        }

        //Functions to set local values of extended servo set
        //Indices 0-15 (only 0-5 currenly active) are Servo ouputs of Arduino board
        //Indices 16-31 are extended group 1 controlled by PCA9685 board 1
        //Indices 32-47 are extended group 2 controlled by PCA9685 board 2
        //Indices 48-63 are extended group 3 controlled by PCA9685 board 3
//        U8 SetServoUsec(U8 ServoIndex,U16 USec);
        private void SetServoUsecLoc(U8 ServoIndex, U16 USec)
        {
            if(cbServoDirect.Checked)
            {
                LoadServoUsec(ServoIndex, USec);
            }
            else
            {
                USecVals[ServoIndex] = (U16) (USec + SERVO_UPDATE_FLAG);
            }
        }

         private U32 SetPegUsecLoc(U32 MinUsec, U32 MaxUsec, int Value, U8 Index)
        {
            U32 UsecVal;
            Value = Value > 10000 ? 10000 : Value < 0 ? 0 : Value;
            MaxUsec = MaxUsec > 2500 ? 2500 : MaxUsec;
            MinUsec = MinUsec < 200 ? 200 : MinUsec > MaxUsec ? MaxUsec : MinUsec;
            UsecVal = (U32)Value * (MaxUsec - MinUsec) / 10000 + MinUsec;
            SetServoUsecLoc(Index, (U16) UsecVal);
            return UsecVal;
        }

        //offset of 0 up to 32, or LEN - 4
        public const U16 WAVE_ARRAY_LEN = 36;
        private int[] WaveArray = {
                                        8500, 8000, 7500, 7000, 7500,
                                        8000, 8500, 9000, 9500, 10000,
                                        10000, 10000, 10000, 10000, 10000,
                                        10000, 10000, 10000, 10000, 10000,
                                        10000, 10000, 10000, 10000, 10000,
                                        10000, 10000, 10000, 10000, 10000,
                                        10000, 10000, 10000, 10000, 9500,
                                        9000
                                };

        int WaveOffset;
        int WaveDivisor;
        bool WaveDir;

        private U32 PegMin(U8 PegIndex)
        {
            U32 Val;
            switch(PegIndex)
            {
                default:
                case 0:
                    StrToU32(tbMin01.Text, out Val);
                    break;

                case 1:
                    StrToU32(tbMin02.Text, out Val);
                    break;

                case 2:
                    StrToU32(tbMin03.Text, out Val);
                    break;

                case 3:
                    StrToU32(tbMin04.Text, out Val);
                    break;

                case 4:
                    StrToU32(tbMin05.Text, out Val);
                    break;

                case 5:
                    StrToU32(tbMin06.Text, out Val);
                    break;

                case 6:
                    StrToU32(tbMin07.Text, out Val);
                    break;

                case 7:
                    StrToU32(tbMin08.Text, out Val);
                    break;
            }
            return Val;
        }

        private U32 PegMax(U8 PegIndex)
        {
            U32 Val;
            switch(PegIndex)
            {
                default:
                case 0:
                    StrToU32(tbMax01.Text, out Val);
                    break;

                case 1:
                    StrToU32(tbMax02.Text, out Val);
                    break;

                case 2:
                    StrToU32(tbMax03.Text, out Val);
                    break;

                case 3:
                    StrToU32(tbMax04.Text, out Val);
                    break;

                case 4:
                    StrToU32(tbMax05.Text, out Val);
                    break;

                case 5:
                    StrToU32(tbMax06.Text, out Val);
                    break;

                case 6:
                    StrToU32(tbMax07.Text, out Val);
                    break;

                case 7:
                    StrToU32(tbMax08.Text, out Val);
                    break;
            }
            return Val;
        }



        private void SetPegs()
        {
            int Index, Value;
            for(byte i = 0; i < 6; ++i)
            {
                Index = (6 * i) - WaveOffset + 3;
                Index = Index >= WAVE_ARRAY_LEN ? (Index - WAVE_ARRAY_LEN) : (Index < 0 ) ? Index + WAVE_ARRAY_LEN : Index;
                Value = WaveArray[Index];
                SetPegUsecLoc(PegMin(i), PegMax(i), Value, (byte)(i + EXT_SERVO_BASE_INDEX));
            }
        }

        private void WaveServe()
       {
            //if(++WaveDivisor < 5)
            //    return;
            WaveDivisor = 0;
            if(WaveDir)
            {
                --WaveOffset;
                if(WaveOffset < 0)
                {
                    WaveOffset = 0;
                    WaveDir = false;
                }
            }
            else
            {
                ++WaveOffset;
                if(WaveOffset >= (WAVE_ARRAY_LEN - 5))
                {
                    WaveOffset = WAVE_ARRAY_LEN - 5;
                    WaveDir = true;
                }
            }
            SetPegs();
       }

        //Index 0, User servo 1
        private void Serv0Serve()
        {
            C_ID = 0;
            CycleTimer[C_ID] += 10;
            if(CycleTimer[C_ID] >= CycleDelay[C_ID])
            {
                CycleTimer[C_ID] = 0;
                switch (ServoState[C_ID])
                {
                    case (int)SERV_STAT.SS_INIT:
                        CycleVal[C_ID] = tbar1.Value;
                              ServoState[C_ID] = (int)SERV_STAT.SS_UP;
                        break;

                    case (int)SERV_STAT.SS_UP:
                        CycleVal[C_ID] += CycleStep[C_ID];
                        if(CycleVal[C_ID] >= CycleLimit[C_ID])
                        {
                            CycleVal[C_ID] = CycleLimit[C_ID];
                            ServoState[C_ID] = (int)SERV_STAT.SS_TOP;  //and start top dwell
                        }
                        break;

                    case (int)SERV_STAT.SS_TOP:
                        ServoState[C_ID] = (int)SERV_STAT.SS_DOWN;
                        break;

                    case (int)SERV_STAT.SS_DOWN:
                        CycleVal[C_ID] -= CycleStep[C_ID];
                        if(CycleVal[C_ID] <= CycleLimit[C_ID])
                        {
                            CycleVal[C_ID] = CycleLimit[C_ID];//on or over limit so limit it there
                            ServoState[C_ID] = (int)SERV_STAT.SS_BOTTOM; //and start bottom dwell
                        }
                        break;

                    case (int)SERV_STAT.SS_BOTTOM:
                         ServoState[C_ID] = (int)SERV_STAT.SS_UP;
                        break;

                }
                SetTBarValue(C_ID);
                LocSetServoUsec((U8)C_ID, (U16)tbar1.Value);
            }
        }

        bool ExtServ1MaxFlag;
        bool ExtServ2MaxFlag;
        //handles cycling of Ext Servos group 1
        private void Serv1Serve()
        {
            C_ID = 1;
            CycleTimer[C_ID] += 10;
            if(CycleTimer[C_ID] >= 300)
            {
                CycleTimer[C_ID] = 1;
                if(ExtServ1MaxFlag)
                {
                    if(cbS01.Checked)
                    {
                        SetMax01();
                    }
                    if(cbS02.Checked)
                    {
                        SetMax02();
                    }
                    if(cbS03.Checked)
                    {
                        SetMax03();
                    }
                    if(cbS04.Checked)
                    {
                        SetMax04();
                    }
                    if(cbS05.Checked)
                    {
                        SetMax05();
                    }
                    if(cbS06.Checked)
                    {
                        SetMax06();
                    }
                    if(cbS07.Checked)
                    {
                        SetMax07();
                    }
                    if(cbS08.Checked)
                    {
                        SetMax08();
                    }
                    if(cbS09.Checked)
                    {
                        SetMax09();
                    }
                    if(cbS10.Checked)
                    {
                        SetMax10();
                    }
                    if(cbS11.Checked)
                    {
                        SetMax11();
                    }
                    if(cbS12.Checked)
                    {
                        SetMax12();
                    }
                    if(cbS13.Checked)
                    {
                        SetMax13();
                    }
                    if(cbS14.Checked)
                    {
                        SetMax14();
                    }
                    if(cbS15.Checked)
                    {
                        SetMax15();
                    }
                    if(cbS16.Checked)
                    {
                        SetMax16();
                    }
                }
                else
                {
                    if(cbS01.Checked)
                    {
                        SetMin01();
                    }
                    if(cbS02.Checked)
                    {
                        SetMin02();
                    }
                    if(cbS03.Checked)
                    {
                        SetMin03();
                    }
                    if(cbS04.Checked)
                    {
                        SetMin04();
                    }
                    if(cbS05.Checked)
                    {
                        SetMin05();
                    }
                    if(cbS06.Checked)
                    {
                        SetMin06();
                    }
                    if(cbS07.Checked)
                    {
                        SetMin07();
                    }
                    if(cbS08.Checked)
                    {
                        SetMin08();
                    }
                    if(cbS09.Checked)
                    {
                        SetMin09();
                    }
                    if(cbS10.Checked)
                    {
                        SetMin10();
                    }
                    if(cbS11.Checked)
                    {
                        SetMin11();
                    }
                    if(cbS12.Checked)
                    {
                        SetMin12();
                    }
                    if(cbS13.Checked)
                    {
                        SetMin13();
                    }
                    if(cbS14.Checked)
                    {
                        SetMin14();
                    }
                    if(cbS15.Checked)
                    {
                        SetMin15();
                    }
                    if(cbS16.Checked)
                    {
                        SetMin16();
                    }
                }
                ExtServ1MaxFlag = !ExtServ1MaxFlag;
            }
        }

        private void Serv2Serve()
        {
            C_ID = 2;
            CycleTimer[C_ID] += 10;
            if(CycleTimer[C_ID] >= 300)
            {
                CycleTimer[C_ID] = 1;
                if(ExtServ2MaxFlag)
                {
                    if(cbS17.Checked)
                    {
                        SetMax17();
                    }
                    if(cbS18.Checked)
                    {
                        SetMax18();
                    }
                    if(cbS19.Checked)
                    {
                        SetMax19();
                    }
                    if(cbS20.Checked)
                    {
                        SetMax20();
                    }
                    if(cbS21.Checked)
                    {
                        SetMax21();
                    }
                    if(cbS22.Checked)
                    {
                        SetMax22();
                    }
                    if(cbS23.Checked)
                    {
                        SetMax23();
                    }
                    if(cbS24.Checked)
                    {
                        SetMax24();
                    }
                    if(cbS25.Checked)
                    {
                        SetMax25();
                    }
                    if(cbS26.Checked)
                    {
                        SetMax26();
                    }
                    if(cbS27.Checked)
                    {
                        SetMax27();
                    }
                    if(cbS28.Checked)
                    {
                        SetMax28();
                    }
                    if(cbS29.Checked)
                    {
                        SetMax29();
                    }
                    if(cbS30.Checked)
                    {
                        SetMax30();
                    }
                    if(cbS31.Checked)
                    {
                        SetMax31();
                    }
                    if(cbS32.Checked)
                    {
                        SetMax32();
                    }
                }
                else
                {
                    if(cbS17.Checked)
                    {
                        SetMin17();
                    }
                    if(cbS18.Checked)
                    {
                        SetMin18();
                    }
                    if(cbS19.Checked)
                    {
                        SetMin19();
                    }
                    if(cbS20.Checked)
                    {
                        SetMin20();
                    }
                    if(cbS21.Checked)
                    {
                        SetMin21();
                    }
                    if(cbS22.Checked)
                    {
                        SetMin22();
                    }
                    if(cbS23.Checked)
                    {
                        SetMin23();
                    }
                    if(cbS24.Checked)
                    {
                        SetMin24();
                    }
                    if(cbS25.Checked)
                    {
                        SetMin25();
                    }
                    if(cbS26.Checked)
                    {
                        SetMin26();
                    }
                    if(cbS27.Checked)
                    {
                        SetMin27();
                    }
                    if(cbS28.Checked)
                    {
                        SetMin28();
                    }
                    if(cbS29.Checked)
                    {
                        SetMin29();
                    }
                    if(cbS30.Checked)
                    {
                        SetMin30();
                    }
                    if(cbS31.Checked)
                    {
                        SetMin31();
                    }
                    if(cbS32.Checked)
                    {
                        SetMin32();
                    }
                }
                ExtServ2MaxFlag = !ExtServ2MaxFlag;
            }
        }

        //Index 3, User servo 4
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
                  //      CycleVal[C_ID] = tbarServ3.Value;
                  //      ServoState[C_ID] = (int)SERV_STAT.SS_UP;
                        break;

                    case (int)SERV_STAT.SS_UP:
                        CycleVal[C_ID] += CycleStep[C_ID];
                        if(CycleVal[C_ID] >= CycleLimit[C_ID])
                        {
                            CycleVal[C_ID] = CycleLimit[C_ID];
                            ServoState[C_ID] = (int)SERV_STAT.SS_TOP;  //and start top dwell
                        }
                        break;

                    case (int)SERV_STAT.SS_TOP:
                       ServoState[C_ID] = (int)SERV_STAT.SS_DOWN;
                        break;

                    case (int)SERV_STAT.SS_DOWN:
                        CycleVal[C_ID] -= CycleStep[C_ID];
                        if(CycleVal[C_ID] <= CycleLimit[C_ID])
                        {
                            CycleVal[C_ID] = CycleLimit[C_ID];//on or over limit so limit it there
                            ServoState[C_ID] = (int)SERV_STAT.SS_BOTTOM; //and start bottom dwell
                        }
                        break;

                    case (int)SERV_STAT.SS_BOTTOM:
                        ServoState[C_ID] = (int)SERV_STAT.SS_UP;
                        break;

                }
                SetTBarValue(C_ID);
               // LocSetServoUsec((U8)C_ID, (U16)tbarServ3.Value);
                //We added User Servo 6 and control it along with User Servo 4 if
                //S5 Checkbox is checked
            }
        }
        private void StartFireSequence()
        {
            Firing = true;
            FireTimer = 0;
        }

        private void FireServe()
        {  //called every 50 msec.
            if (Firing)
            {
                if (FireTimer++ == 0)
                {
                    tbar1.Value = tbar1.Maximum;
                }
                else if (FireTimer++ > 15)
                {
                    //total of 750 msec. at maximum
                    tbar1.Value = tbar1.Minimum;
                    Firing = false;
                }

                LocSetServoUsec((U8)0, (U16)tbar1.Value);
            }
        }

        private void UpdateSensors()
        {
           DataUpdate(USecVals, 6, InputVals, 10);
           //DFDEBUG
          // GetSlaveParameter(0, 0);

           tbarSensor1.Value = InputVals[0] & 0x3FF;
           tbSensor1.Text = InputVals[0].ToString();
           tbarSensor2.Value = InputVals[1] & 0x3FF;
           tbSensor2.Text = InputVals[1].ToString();
           tbSensor3.Text = tbarSensor3.Value.ToString();
           tbarSensor3.Value = InputVals[2] & 0x3FF;
           tbarSensor4.Value = InputVals[3] & 0x3FF;
           tbSensor4.Text = tbarSensor4.Value.ToString();
           cbSwitch1.Checked = (InputVals[4] & 1) != 0;
           cbLim1.Checked = (InputVals[4] & 2) != 0;
           cbLim2.Checked = (InputVals[4] & 4) != 0;
           cbLim3.Checked = (InputVals[4] & 8) != 0;
           cbLim4.Checked = (InputVals[4] & 16) != 0;
           tbS1Steps.Text = InputVals[5].ToString();
           tbS2Steps.Text = InputVals[6].ToString();
           tbInfl1.Text  = InputVals[7].ToString();
           tbS2Period.Text = InputVals[8].ToString();
        }

        private void InitServoTab()
        {
            InitServoCycle();
           // UpdateTRB0();
        }

        private void SafeTBarSet(System.Windows.Forms.TrackBar TB, int Value)
        {
                Value = Value > TB.Maximum ? TB.Maximum : Value;
                Value = Value < TB.Minimum ? TB.Minimum : Value;
                TB.Value = Value;
        }

        private void S0B1()
        {
            //tbarServ0.Value = (int) nudS0Pos1.Value;
        }

        private void S0B2()
        {
            //tbarServ0.Value = (int) nudS0Pos2.Value;
        }

        private void S0B3()
        {
             //tbarServ0.Value = (int) nudS0Pos3.Value;
        }

        private void S0B4()
        {
            //tbarServ0.Value = (int) nudS0Pos4.Value;
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
