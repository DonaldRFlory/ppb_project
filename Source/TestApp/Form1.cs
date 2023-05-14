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

namespace TestApp
{

    public partial class Form1 : Form
    {
        public const U8 EXT_SERVO_BASE_INDEX = 16;
        public const U8 EXT_SERVO_BASE_BK2_INDEX = 32;
        public const U16 MAX_CONNECTIONS = 10;
        private int StatusMessageTimer;
        string UpgradeStatusStr;
        private bool Started, StartedUp;
        private U32 m_MemAddress;
        private StreamWriter LogFile;
        string LogPath;

        public Form1()
        {
            DateTime localDate = DateTime.Now;
            InitializeComponent();
            InputVals[0] = InputVals[0] = InputVals[0] = InputVals[0] = 0;
            USecVals[0] = USecVals[0] = USecVals[0] = USecVals[0] = 1500;
            m_MemAddress = 0;
            //Delegates = new MD[100];
            InitMenu();
            API.SetBoardAddress((byte)nudBoardAddress.Value);
            InitServoTab();
            LogPath = @".\TestLog" + ".txt";
            try
            {
                LogFile = File.AppendText(LogPath);
            }
            catch
            {
                StringBuilder SB = new StringBuilder();
                SB.AppendFormat("Cannot open log file {0}\r\n", LogPath);
                MessageBox.Show(SB.ToString());
                this.Close();
            }

            LogFile.Write("TestApp Started Up at: {0}\r\n", localDate.ToString());
            LogFile.Flush();
            SetupCallbackFunctions();
            if (!LoadMenu("TestMenu.txt"))
            {
                return;
            }
            CheckMenu();
            DrawMenu("MainMenu");
            InitConnection();
            rbSelectSerial.Checked = true;
            //MenuListBox.Focus();
            if (MenuListBox.Items.Count != 0)
            {
                MenuListBox.SetSelected(0, true);
            }
            tabControl1.SelectedIndex = 1;
            Started = true;
        }

        //So when user wants to open a connection, they open
        //it here. Connection manager makes the connection if possible
        //and returns an index. User may specify an index. For comm
        //ports, this is the connection index. For HID it is the discovery
        //index. They may have called the HIDCount API function to
        //determine how many are available. We will not let them connect
        //to an already open comm port or HID index.
        //Values stored are in order of connection index and only
        //as far as NumConnections.
        //The error detection and restart stuff I put in for single connection
        //over USB no longer makes much sense. Need to detect and show
        //errors in status box but forget the automatic reconnect
        //So how to test it all. Need a way to connect to a particular type
        //and index. Need a disconnect button. Good enough to disconnect the
        //current channel. Need a way to select a channel using numeric up/down
        //control. Maybe a checkbox to indicate the type of connection for current index.
        //Also need a connection box with desired type checkbox, Index, and
        private Connection CMgr;
        public class Connection
        {
            public Connection()
            {
                DevHandles = new API_DEVICE_HANDLE[MAX_CONNECTIONS];
                IsHID = new bool[MAX_CONNECTIONS];
                IsConnected = new bool[MAX_CONNECTIONS];
                ConIndices = new byte[MAX_CONNECTIONS];
                CurIndex = 0;
                _NumConnections = 0;
            }
            private U8 _NumConnections;
            public bool Select(U8 Channel, out bool IsUSB)
            {
                IsUSB = false;
                if ((Channel < MAX_CONNECTIONS) && IsConnected[Channel])
                {
                    IsUSB = IsHID[Channel];
                    CurIndex = Channel;
                    return true;
                }
                return false;
            }
            public U8 NumConnections
            {
                get
                {
                    return _NumConnections;
                }
            }
            private API_DEVICE_HANDLE[] DevHandles;
            private byte[] ConIndices;
            private bool[] IsHID;
            private bool[] IsConnected;
            private U8 CurIndex;

            public API_DEVICE_HANDLE CurHandle()
            {
                if (_NumConnections != 0)
                {
                    return DevHandles[CurIndex];
                }
                else
                {
                    return (System.IntPtr)0;
                }
            }

            public bool Disconnect()
            {
                if ((_NumConnections == 0) || !IsConnected[CurIndex])
                {
                    return false;
                }
                if (APIWrap.Disconnect(DevHandles[CurIndex]))
                {
                    IsConnected[CurIndex] = false;
                    --_NumConnections;
                    return true;
                }
                return false;
            }

            //Returns -1 on failure, positive ChannelID on success
            //New connection becomes selected channel.
            public int Connect(bool HIDFlag, U8 Index)
            {
                if (_NumConnections >= MAX_CONNECTIONS)
                {
                    return -1;
                }
                for (int i = 0; i < _NumConnections; ++i)
                {
                    if (IsConnected[i] && (IsHID[i] == HIDFlag) && (ConIndices[i] == Index))
                    {
                        return -1;
                    }
                }
                //So.. we have room for a new connection and have no
                //connection to the specified target
                for (U8 i = 0; i < MAX_CONNECTIONS; ++i)
                {   //find the first empty slot in connection array
                    if (!IsConnected[i])
                    {
                        if (HIDFlag)
                        {
                            API_DEVICE_HANDLE Handle = (System.IntPtr)0;
                            if (API.HIDConnect(ref Handle, OUR_VID, OUR_PID, Index))
                            {
                                DevHandles[i] = Handle;
                                ++_NumConnections;
                                IsConnected[i] = true;
                                IsHID[i] = true;
                                ConIndices[i] = Index;
                                CurIndex = i;
                                return i;
                            }
                        }
                        else
                        {
                            if (API.SerialConnect(ref DevHandles[i], Index))
                            {
                                ++_NumConnections;
                                IsConnected[i] = true;
                                IsHID[i] = false;
                                ConIndices[i] = Index;
                                CurIndex = i;
                                return i;
                            }
                        }
                        break;
                    }
                }
                return -1;
            }
        }


        //API call wrapper function
        public bool GetVelAcc(U8 Channel, out float Velocity, out float Acceleration)
        {
            Velocity = Acceleration = 0;
            return false;
        }

        //API call wrapper function
        public U16 GetMaxLinkSendSize()
        {
            return (U16)API.GetMaxLinkSendSize(CMgr.CurHandle());
        }

        //API call wrapper function
        public U16 GetMaxLinkReturnSize()
        {
            return (U16)API.GetMaxLinkReturnSize(CMgr.CurHandle());
        }

        private void InitConnection()
        {
            CMgr = new Connection();
        }
        //        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        //		public delegate bool UpgradeCB(
        //			[MarshalAs(UnmanagedType.LPArray, SizeParamIndex=1)]
        //			U16 []String,
        //			U32 StrLen,
        //			U8 Status,
        //			U32 PBPos,
        //			 U32 PBRange);
        static class API  //These are manually generated DLL exports
        {
            [DllImport(@".\PPBAPI.dll")]
            public static extern void RegisterLoggingCallback(LoggingCB CallbackPointer);
            [DllImport(@".\PPBAPI.dll")]
            public static extern void RegisterLinkStatusCallback(LinkStatusCB CallbackPointer);
            [DllImport(@".\PPBAPI.dll")]
            public static extern bool Disconnect(API_DEVICE_HANDLE Handle);
            [DllImport(@".\PPBAPI.dll")]
            public static extern bool SerialConnect(ref API_DEVICE_HANDLE Handle, int Index);
            [DllImport(@".\PPBAPI.dll")]
            public static extern U8 HIDCount(U16 VID, U16 PID);
            [DllImport(@".\PPBAPI.dll")]
            public static extern bool HIDConnect(ref API_DEVICE_HANDLE Handle, U16 VID, U16 PID, U8 Index);
            [DllImport(@".\PPBAPI.dll")]
            public static extern U32 GetMaxLinkSendSize(API_DEVICE_HANDLE APIHandle);
            [DllImport(@".\PPBAPI.dll")]
            public static extern U32 GetMaxLinkReturnSize(API_DEVICE_HANDLE APIHandle);
            [DllImport(@".\PPBAPI.dll")]
            public static extern void SetBoardAddress(U8 Address);
        }
        static class APIWrap  //These are wrappers for API calls above
        {
            //return true if status is OK
            public static bool StatusOK(API_STAT Status)
            {
                return ((int)(Status & 0XFFFF) == (int)API_RSLT.OK);
            }

            public static bool Disconnect(API_DEVICE_HANDLE Handle)
            {
                return API.Disconnect(Handle);
            }

            //This will connect to the Index'th HID device matching our
            //OUR_VID and OUR_PID, assuming there
            //is room in the DLL's channel info table
            //It will fail if all ten channels are in use.
            public static bool ConnectToUSB(ref API_DEVICE_HANDLE Handle, U8 Index)
            {
                return API.HIDConnect(ref Handle, OUR_VID, OUR_PID, Index);
            }

            public static bool SerialConnect(ref API_DEVICE_HANDLE Handle, U8 Index)
            {
                return API.SerialConnect(ref Handle, Index);
            }
        }

        private void MessageServe()
        {
            U16 TraceBuffCount = InputVals[9];
            if (ConnectStatusOK)
            {
                U16 Count, MaxChunk, ChunkSize;
                MaxChunk = (U16)(GetMaxLinkReturnSize() - 1);//account for return value byte
                while (TraceBuffCount != 0)
                {
                    ChunkSize = (TraceBuffCount > MaxChunk) ? MaxChunk : TraceBuffCount;
                    if (ChunkSize == 0)
                    {
                        return;
                    }
                    TraceBuffCount -= ChunkSize;
                    U8[] Buff = new U8[ChunkSize];
                    Count = ReadTraceBuffer(Buff, (U8)ChunkSize);
                    for (U16 i = 0; i < Count; ++i)
                    {
                        if ((Buff[i] < 1) || (Buff[i] >= 127))//check for printable ASCII or control chars except null
                        {//No
                            Buff[i] = 42; //replace with '*'
                        }
                    }
                    string String = System.Text.Encoding.ASCII.GetString(Buff);//make it a string
                    rtbMessageBuff.AppendText(String);
                    if (cbMessBuffAutoScroll.Checked)
                    {
                        rtbMessageBuff.SelectionStart = rtbMessageBuff.Text.Length;
                        rtbMessageBuff.ScrollToCaret();
                    }
                }
            }
        }

        private bool GetInputString(string Prompt, out string Input)
        {
            EntryDialog EDInstance = new EntryDialog(Prompt);
            DialogResult dialogResult = EDInstance.ShowDialog(this);
            Input = EDInstance.Input;
            return (dialogResult == DialogResult.OK);
        }

        private bool ConnectStatusOK;
        //Check status and report in status window, return true/false
        private bool APIStatusOK(API_STAT Status)
        {
            StringBuilder SB = new StringBuilder();
            if ((int)(Status & 0XFFFF) == (int)API_RSLT.OK)
            {
                LabAPI.BackColor = System.Drawing.Color.Green;
                ConnectStatusOK = true;
                TBConnectStatus.Text = "";
                return true;
            }
            else
            {
                LabAPI.BackColor = System.Drawing.Color.Red;
                ConnectStatusOK = false;
                //report the error here TODO:
                SB.AppendFormat(" {0,5}{1,11}{2, 15}", Status >> 24, (Status >> 16) & 0XFF, Status & 0xFF);
                if ((Status & 0XFF) == (int)API_RSLT.COMM_BAD_SEND)
                {

                }
                TBConnectStatus.Text = SB.ToString();
                return false;
            }
        }

        private void MemBlockTest()
        {
            //Encoding enc = new ASCIIEncoding();
            //string S2 = "This is a fairly lengthy string which I want to download to the link buffer on the slave and retrieve in several parts. We want to see some text that we can easily recognize when we read it back up";
            //byte [] Bytes = enc.GetBytes(S2);
            //MasterBlockDown(ref Bytes, (ushort)Bytes.Length, 0X1FFF1000);
            //MasterBlockUp(0X1FFF1020, ref Bytes, (ushort)Bytes.Length);
            //string decodedString = enc.GetString(Bytes);
            //MasterBlockUp(0X1FFF1030, ref Bytes, (ushort)Bytes.Length);
            //decodedString = enc.GetString(Bytes);
        }

        private void UpdateConnectionStatus(bool NewState)
        {

        }

        private void CBUSBConnection_CheckedChanged(object sender, EventArgs e)
        {

        }

        private void CBCycleS0_CheckedChanged(object sender, EventArgs e)
        {

        }

        private void tabControl1_SelectedIndexChanged(object sender, EventArgs e)
        {

            string Name = tabControl1.TabPages[tabControl1.SelectedIndex].Name;
            if (Name == "tabMenuPage")
            {
                MenuListBox.Focus();
                //MessageBox.Show("Hello");
            }
            else if (Name == "tabMemoryPage")
            {
                DisplayMemPage(m_MemAddress);
            }
        }

        private int TenMsecDivide;
        private void timer1_Tick(object sender, EventArgs e)
        {
            //cb0-cb3 are labeled Selected Servos for Cycling
            if (cb0.Checked)
            {
                Serv0Serve();
            }
            if (cb1.Checked)
            {
                Serv1Serve();  //if checked, called every ten msec
            }
            if (cb2.Checked)
            {
                Serv2Serve();  //if checked, called every ten msec
            }




            if (++TenMsecDivide < 5)
            {
                return;
            }

            //-------------------------Stuff below this point happens every 50 msec--------------------------------
            //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


            TenMsecDivide = 0;
            if(cbPinWave.Checked)
            {
                WaveServe();
            }
            if (cbSensorUpdate.Checked)
            {
                FireServe();
                UpdateSensors();//For now, on every 5th tick, we do a transaction with board.
                MessageServe();//Uses trace buff count read by UpdateSensors() above.
            }

            if (StatusMessageTimer > 0)
            {
                --StatusMessageTimer;
                if (StatusMessageTimer == 0)
                {
                    ProgBarLabel.Text = "";
                }
            }
            if (Started & !StartedUp)
            {
                StartedUp = true;
                tabControl1.SelectedIndex = 0;
            }
            if (tabControl1.TabPages[tabControl1.SelectedIndex].Name == "tpServo")
            {
                //UpdateServoStatusDisplay();
                //UpdatePosStatusDisplay();
            }

            //tbMilliseconds.Text = GetSlaveParameter(1, 0).ToString();
        }

        private void MenuListBox_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            int Index = this.MenuListBox.IndexFromPoint(e.Location);
            if (Index != System.Windows.Forms.ListBox.NoMatches)
            {
                ExecuteMenuSelection();
            }
        }

        private void MenuListBox_SelectedIndexChanged(object sender, EventArgs e)
        {
        }


        private void BTestEntry_Click(object sender, EventArgs e)
        {
            string Input;
            GetInputString("Enter a Value:", out Input);
            MessageBox.Show(Input, "The input was:");
        }


        private void MyAcceptButton_Click(object sender, EventArgs e)
        {
            string Name = tabControl1.TabPages[tabControl1.SelectedIndex].Name;
            if (Name == "tabMenuPage")
            {
                ExecuteMenuSelection();
                MenuListBox.Focus();
            }
            else if (Name == "tabMemoryPage")
            {
                UpdateMemPage();
            }
        }

        protected override bool IsInputKey(Keys keyData)
        {
            switch (keyData)
            {
                case Keys.Down:
                case Keys.Up:
                case Keys.PageUp:
                case Keys.PageDown:
                    return true;
            }
            return base.IsInputKey(keyData);
        }

        private void EscapeButton_Click(object sender, EventArgs e)
        {
            PopMenu();
            MenuListBox.Focus();
        }

        private void MenuListBox_KeyPress(object sender, KeyPressEventArgs e)
        {
            char C = e.KeyChar;
            int Index = MenuListBox.SelectedIndex;
            for (int i = 0; i < TheMenu.CurrentPage.Items.Count; ++i)
            {
                if (TheMenu.CurrentPage.Items[i].Activator == C)
                {
                    MenuListBox.SelectedIndex = i;
                    ExecuteMenuSelection();
                    break;
                }
            }
        }

        //public delegate bool UpgradeCB(U8 Status, char []String, U32 StrLen, U32 PBPos, U32 PBRange);
        public bool UGCallbackFunction(U16[] U16Array, U32 StrLen, U8 Status, U32 PBPos, U32 PBRange)
        {
            char[] ChrArray = new char[StrLen];

            for (int i = 0; i < StrLen; ++i)
            {
                ChrArray[i] = (char)U16Array[i];
            }
            progressBar1.Step = 1;
            progressBar1.Show();
            UpgradeStatusStr = new string(ChrArray, 0, (int)StrLen);
            progressBar1.Maximum = (int)PBRange;
            //Note on strange setting of progressBar1.Value below
            //Apparently there is an animation done by Windows Aero
            //theme which produces the delay. The workaround is to
            //set to one more than the position you want then back
            //off to the position you want. The animation only
            //happens for increases in position!
            if (PBPos < PBRange)
            {
                progressBar1.Value = (int)PBPos + 1;
                progressBar1.Value = (int)PBPos;
            }
            else
            {
                progressBar1.Value = (int)PBPos;
            }
            ProgBarLabel.Text = UpgradeStatusStr;
            progressBar1.Refresh();
            ProgBarLabel.Refresh();
            return false;
        }

        private void rbSelectUSB_CheckedChanged(object sender, EventArgs e)
        {

        }

        private void rbSelectSerial_CheckedChanged(object sender, EventArgs e)
        {

        }

        private void bConnect_Click(object sender, EventArgs e)
        {
            int Channel;
            byte Index;
            API_DEVICE_HANDLE Handle = (System.IntPtr)0;
            try
            {
                Index = (byte)Convert.ToInt32(nudIndex.Value);
                if (rbSelectUSB.Checked)
                {
                    Channel = CMgr.Connect(true, Index);//make a USB connection
                    if (Channel >= 0)
                    {
                        rbUSB.Checked = true;
                        nudCurrentChannel.Value = Channel;
                        return;
                    }
                }
                else
                {
                    Channel = CMgr.Connect(false, Index);//make a serial connection
                    if (Channel >= 0)
                    {
                        rbSerial.Checked = true;
                        nudCurrentChannel.Value = Channel;
                        return;
                    }
                }
            }
            catch
            {
                MessageBox.Show("Exception during connect attempt");
                return;
            }
            MessageBox.Show("Connection failed");
        }

        private void rbSerial_CheckedChanged(object sender, EventArgs e)
        {
            if (rbSerial.Checked)
            {
                rbUSB.Checked = false;
            }
        }

        private void rbUSB_CheckedChanged(object sender, EventArgs e)
        {
            if (rbUSB.Checked)
            {
                rbSerial.Checked = false;
            }
        }

        private void SelectNewChannel()
        {
            bool IsUSB;
            U8 NewChannel = (U8)Convert.ToInt32(nudCurrentChannel.Value);
            if (CMgr.Select(NewChannel, out IsUSB))
            {
                if (IsUSB)
                {
                    rbSerial.Checked = false;
                    rbUSB.Checked = true;
                }
                else
                {
                    rbSerial.Checked = true;
                    rbUSB.Checked = false;
                }
            }
            else
            {
                rbSerial.Checked = false;
                rbUSB.Checked = false;
            }
        }

        private void nudCurrentChannel_ValueChanged(object sender, EventArgs e)
        {
            SelectNewChannel();
        }

        private void bDisconnect_Click(object sender, EventArgs e)
        {
            if (!CMgr.Disconnect())
            {
                MessageBox.Show("Disconnect failed!!");
                return;
            }
            SelectNewChannel();
        }



        private void bGoPosA0_Click(object sender, EventArgs e)
        {
            //MovePosA(0);
        }

        private void bGoPosB0_Click(object sender, EventArgs e)
        {
            //MovePosB(0);
        }
        private void bReset0_Click(object sender, EventArgs e)
        {
            //ResetServo(0);
        }


        private void bReset1_Click(object sender, EventArgs e)
        {
            //ResetServo(1);
        }

        private void bReset2_Click(object sender, EventArgs e)
        {
            //ResetServo(2);


        }

        private void bReset3_Click(object sender, EventArgs e)
        {
            //ResetServo(3);
        }

        private void bDisableAll_Click(object sender, EventArgs e)
        {

        }
        private void bFire_Click(object sender, EventArgs e)
        {
            StartFireSequence();
        }

        public void LogToMessageBuffer(string LogString)
        {
            rtbMessageBuff.Invoke(new MethodInvoker(delegate
            {
                rtbMessageBuff.AppendText(LogString);
                if (cbMessBuffAutoScroll.Checked)
                {
                    rtbMessageBuff.SelectionStart = rtbMessageBuff.Text.Length;
                    rtbMessageBuff.ScrollToCaret();
                }
            }));
        }

        private void bMessageBufferClear_Click(object sender, EventArgs e)
        {
            rtbMessageBuff.Clear();
        }


        private void trbServ0_Scroll(object sender, EventArgs e)
        {
            //tbServ0Pos.Text = trbServ0.Value.ToString();
        }


        private void trbServ0_ValueChanged(object sender, EventArgs e)
        {
            //          tbServ0Pos.Text = trbServ0.Value.ToString();
        }



        private void nudTRB0Max_ValueChanged(object sender, EventArgs e)
        {
            //  trbServ0.Maximum = (int)nudTRB0Max.Value;
            //  tbServ0Pos.Text = trbServ0.Value.ToString();
        }

        private void nudTRB0Min_ValueChanged(object sender, EventArgs e)
        {
            //  trbServ0.Minimum = (int)nudTRB0Min.Value;
            //   tbServ0Pos.Text = trbServ0.Value.ToString();
        }

        private void cb0_CheckedChanged(object sender, EventArgs e)
        {
            //Serv0Init();
            if (cb0.Checked)
            {
                //SetVA1();
            }
        }



        private void button4_Click(object sender, EventArgs e)
        {
            S0B4();
            //LocSetServoUsec((U8) 0, (U16) nudS0Pos4.Value);
        }

        private void button3_Click(object sender, EventArgs e)
        {
            S0B3();
            //LocSetServoUsec((U8) 0, (U16) nudS0Pos3.Value);
        }

        private void button2_Click(object sender, EventArgs e)
        {
            S0B2();
            //LocSetServoUsec((U8) 0, (U16) nudS0Pos2.Value);
        }

        private void button1_Click(object sender, EventArgs e)
        {
            S0B1();
            //LocSetServoUsec((U8) 0, (U16) nudS0Pos1.Value);
        }

        private void cbSensorUpdate_CheckedChanged(object sender, EventArgs e)
        {

        }

        private void bMaxMin0_Click(object sender, EventArgs e)
        {
            if(MaxMin0Top)
            {
                S0B1();
            }
            else
            {
                S0B4();
            }
            MaxMin0Top = !MaxMin0Top;
        }


        private void nudBoardAddress_ValueChanged(object sender, EventArgs e)
        {
            API.SetBoardAddress((byte)nudBoardAddress.Value);
        }

        private void groupBox2_Enter(object sender, EventArgs e)
        {

        }

        private void button5_Click(object sender, EventArgs e)
        {
            SetSlaveParameter(30, 0, 0);
        }

        private void button6_Click(object sender, EventArgs e)
        {
            SetSlaveParameter(31, 0, 0);//SegStep()
        }

        private void button7_Click(object sender, EventArgs e)
        {
            MessageServe();
        }

        private void SetMin01()
        {
            U32 USec;
            StrToU32(tbMin01.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_INDEX + 0, (U16) USec);
        }

        private void bMin01_Click(object sender, EventArgs e)
        {
            SetMin01();
        }

        private void SetMin02()
        {
            U32 USec;
            StrToU32(tbMin02.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_INDEX + 1, (U16) USec);
        }

        private void bMin02_Click(object sender, EventArgs e)
        {
            SetMin02();
        }

        private void SetMin03()
        {
            U32 USec;
            StrToU32(tbMin03.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_INDEX + 2, (U16) USec);
        }

        private void bMin03_Click(object sender, EventArgs e)
        {
            SetMin03();
        }

        private void SetMin04()
        {
            U32 USec;
            StrToU32(tbMin04.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_INDEX + 3, (U16) USec);
        }

        private void bMin04_Click(object sender, EventArgs e)
        {
            SetMin04();
        }

        private void SetMin05()
        {
            U32 USec;
            StrToU32(tbMin05.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_INDEX + 4, (U16) USec);
        }

        private void bMin05_Click(object sender, EventArgs e)
        {
            SetMin05();
        }

        private void SetMin06()
        {
            U32 USec;
            StrToU32(tbMin06.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_INDEX + 5, (U16) USec);
        }

        private void bMin06_Click(object sender, EventArgs e)
        {
            SetMin06();
        }

        private void SetMin07()
        {
            U32 USec;
            StrToU32(tbMin07.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_INDEX + 6, (U16) USec);
        }

        private void bMin07_Click(object sender, EventArgs e)
        {
            SetMin07();
        }

        private void SetMin08()
        {
            U32 USec;
            StrToU32(tbMin08.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_INDEX + 7, (U16) USec);
        }

        private void bMin08_Click(object sender, EventArgs e)
        {
            SetMin08();
        }

        private void SetMin09()
        {
            U32 USec;
            StrToU32(tbMin09.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_INDEX + 8, (U16) USec);
        }

        private void bMin09_Click(object sender, EventArgs e)
        {
            SetMin09();
        }

        private void SetMin10()
        {
            U32 USec;
            StrToU32(tbMin10.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_INDEX + 9, (U16) USec);
        }

        private void bMin10_Click(object sender, EventArgs e)
        {
            SetMin10();
        }

        private void SetMin11()
        {
            U32 USec;
            StrToU32(tbMin11.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_INDEX + 10, (U16) USec);
        }

        private void bMin11_Click(object sender, EventArgs e)
        {
            SetMin11();
        }

        private void SetMin12()
        {
            U32 USec;
            StrToU32(tbMin12.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_INDEX + 11, (U16) USec);
        }

        private void bMin12_Click(object sender, EventArgs e)
        {
            SetMin12();
        }

        private void SetMin13()
        {
            U32 USec;
            StrToU32(tbMin13.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_INDEX + 12, (U16) USec);
        }

        private void bMin13_Click(object sender, EventArgs e)
        {
            SetMin13();
        }

        private void SetMin14()
        {
            U32 USec;
            StrToU32(tbMin14.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_INDEX + 13, (U16) USec);
        }

        private void bMin14_Click(object sender, EventArgs e)
        {
            SetMin14();
        }

        private void SetMin15()
        {
            U32 USec;
            StrToU32(tbMin15.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_INDEX + 14, (U16) USec);
        }

        private void bMin15_Click(object sender, EventArgs e)
        {
            SetMin15();
        }

        private void SetMin16()
        {
            U32 USec;
            StrToU32(tbMin16.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_INDEX + 15, (U16) USec);
        }

        private void bMin16_Click(object sender, EventArgs e)
        {
            SetMin16();
        }

        private void SetMax01()
        {
            U32 USec;
            StrToU32(tbMax01.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_INDEX + 0, (U16) USec);
        }

        private void bMax01_Click(object sender, EventArgs e)
        {
            SetMax01();
        }

        private void SetMax02()
        {
            U32 USec;
            StrToU32(tbMax02.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_INDEX + 1, (U16) USec);
        }

        private void bMax02_Click(object sender, EventArgs e)
        {
            SetMax02();
        }

        private void SetMax03()
        {
            U32 USec;
            StrToU32(tbMax03.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_INDEX + 2, (U16) USec);
        }

        private void bMax03_Click(object sender, EventArgs e)
        {
            SetMax03();
        }

        private void SetMax04()
        {
            U32 USec;
            StrToU32(tbMax04.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_INDEX + 3, (U16) USec);
        }

        private void bMax04_Click(object sender, EventArgs e)
        {
            SetMax04();
        }

        private void SetMax05()
        {
            U32 USec;
            StrToU32(tbMax05.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_INDEX + 4, (U16) USec);
        }

        private void bMax05_Click(object sender, EventArgs e)
        {
            SetMax05();
        }

        private void SetMax06()
        {
            U32 USec;
            StrToU32(tbMax06.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_INDEX + 5, (U16) USec);
        }

        private void bMax06_Click(object sender, EventArgs e)
        {
            SetMax06();
        }

        private void SetMax07()
        {
            U32 USec;
            StrToU32(tbMax07.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_INDEX + 6, (U16) USec);
        }

        private void bMax07_Click(object sender, EventArgs e)
        {
            SetMax07();
        }

        private void SetMax08()
        {
            U32 USec;
            StrToU32(tbMax08.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_INDEX + 7, (U16) USec);
        }

        private void bMax08_Click(object sender, EventArgs e)
        {
            SetMax08();
        }

        private void SetMax09()
        {
            U32 USec;
            StrToU32(tbMax09.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_INDEX + 8, (U16) USec);
        }

        private void bMax09_Click(object sender, EventArgs e)
        {
            SetMax09();
        }

        private void SetMax10()
        {
            U32 USec;
            StrToU32(tbMax10.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_INDEX + 9, (U16) USec);
        }

        private void bMax10_Click(object sender, EventArgs e)
        {
            SetMax10();
        }

        private void SetMax11()
        {
            U32 USec;
             StrToU32(tbMax11.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_INDEX + 10, (U16) USec);
        }

        private void bMax11_Click(object sender, EventArgs e)
        {
            SetMax11();
        }

        private void SetMax12()
        {
            U32 USec;
            StrToU32(tbMax12.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_INDEX + 11, (U16) USec);
        }

        private void bMax12_Click(object sender, EventArgs e)
        {
            SetMax12();
        }

        private void SetMax13()
        {
            U32 USec;
            StrToU32(tbMax13.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_INDEX + 12, (U16) USec);
        }

        private void bMax13_Click(object sender, EventArgs e)
        {
            SetMax13();
        }

        private void SetMax14()
        {
            U32 USec;
            StrToU32(tbMax14.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_INDEX + 13, (U16) USec);
        }

        private void bMax14_Click(object sender, EventArgs e)
        {
            SetMax14();
        }

        private void SetMax15()
        {
            U32 USec;
            StrToU32(tbMax15.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_INDEX + 14, (U16) USec);
        }

        private void bMax15_Click(object sender, EventArgs e)
        {
            SetMax15();
        }

        private void SetMax16()
        {
            U32 USec;
            StrToU32(tbMax16.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_INDEX + 15, (U16) USec);
        }

        private void bMax16_Click(object sender, EventArgs e)
        {
            SetMax16();
        }

        private void SetMin17()
        {
            U32 USec;
            StrToU32(tbMin17.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_BK2_INDEX + 0, (U16) USec);
        }

        private void bMin17_Click(object sender, EventArgs e)
        {
            SetMin17();
        }

        private void SetMin18()
        {
            U32 USec;
            StrToU32(tbMin18.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_BK2_INDEX + 1, (U16) USec);
        }

        private void bMIn18_Click(object sender, EventArgs e)
        {
            SetMin18();
        }

        private void SetMin19()
        {
            U32 USec;
            StrToU32(tbMin19.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_BK2_INDEX + 2, (U16) USec);
        }

        private void bMin19_Click(object sender, EventArgs e)
        {
            SetMin19();
            U32 USec;
            StrToU32(tbMin19.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_BK2_INDEX + 2, (U16) USec);
        }

        private void SetMin20()
        {
            U32 USec;
            StrToU32(tbMin20.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_BK2_INDEX + 3, (U16) USec);
        }

        private void bMin20_Click(object sender, EventArgs e)
        {
            SetMin20();
        }

        private void SetMin21()
        {
            U32 USec;
            StrToU32(tbMin21.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_BK2_INDEX + 4, (U16) USec);
        }

        private void bMin21_Click(object sender, EventArgs e)
        {
            SetMin21();
        }

        private void SetMin22()
        {
            U32 USec;
            StrToU32(tbMin22.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_BK2_INDEX + 5, (U16) USec);
        }

        private void bMin22_Click(object sender, EventArgs e)
        {
            SetMin22();
        }

        private void SetMin23()
        {
            U32 USec;
            StrToU32(tbMin23.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_BK2_INDEX + 6, (U16) USec);
        }

        private void bMin23_Click(object sender, EventArgs e)
        {
            SetMin23();
        }

        private void SetMin24()
        {
            U32 USec;
            StrToU32(tbMin24.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_BK2_INDEX + 7, (U16) USec);
        }

        private void bMin24_Click(object sender, EventArgs e)
        {
            SetMin24();
        }

        private void SetMin25()
        {
            U32 USec;
            StrToU32(tbMin25.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_BK2_INDEX + 8, (U16) USec);
        }

        private void bMin25_Click(object sender, EventArgs e)
        {
            SetMin25();
        }

        private void SetMin26()
        {
            U32 USec;
            StrToU32(tbMin26.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_BK2_INDEX + 9, (U16) USec);
        }

        private void bMin26_Click(object sender, EventArgs e)
        {
            SetMin26();
        }

        private void SetMin27()
        {
            U32 USec;
            StrToU32(tbMin27.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_BK2_INDEX + 10, (U16) USec);
        }

        private void bMin27_Click(object sender, EventArgs e)
        {
            SetMin27();
        }

        private void SetMin28()
        {
            U32 USec;
            StrToU32(tbMin28.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_BK2_INDEX + 11, (U16) USec);
        }

        private void bMIn28_Click(object sender, EventArgs e)
        {
            SetMin28();
        }

        private void SetMin29()
        {
            U32 USec;
            StrToU32(tbMin29.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_BK2_INDEX + 12, (U16) USec);
        }

        private void bMin29_Click(object sender, EventArgs e)
        {
            SetMin29();
        }

        private void SetMin30()
        {
            U32 USec;
            StrToU32(tbMin30.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_BK2_INDEX + 13, (U16) USec);
        }

        private void bMin30_Click(object sender, EventArgs e)
        {
            SetMin30();
        }

        private void SetMin31()
        {
            U32 USec;
            StrToU32(tbMin31.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_BK2_INDEX + 14, (U16) USec);
        }

        private void bMin31_Click(object sender, EventArgs e)
        {
            SetMin31();
        }

        private void SetMin32()
        {
            U32 USec;
            StrToU32(tbMin32.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_BK2_INDEX + 15, (U16) USec);
        }

        private void bMin32_Click(object sender, EventArgs e)
        {
            SetMin32();
        }

        private void SetMax17()
        {
            U32 USec;
            StrToU32(tbMax17.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_BK2_INDEX + 0, (U16) USec);
        }

        private void bMax17_Click(object sender, EventArgs e)
        {
            SetMax17();
        }

        private void SetMax18()
        {
            U32 USec;
            StrToU32(tbMax18.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_BK2_INDEX + 1, (U16) USec);
        }

        private void bMax18_Click(object sender, EventArgs e)
        {
            SetMax18();
        }

        private void SetMax19()
        {
            U32 USec;
            StrToU32(tbMax19.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_BK2_INDEX + 2, (U16) USec);
        }

        private void bMax19_Click(object sender, EventArgs e)
        {
            SetMax19();
        }

        private void SetMax20()
        {
            U32 USec;
            StrToU32(tbMax20.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_BK2_INDEX + 3, (U16) USec);
        }

        private void bMax20_Click(object sender, EventArgs e)
        {
            SetMax20();
        }

        private void SetMax21()
        {
            U32 USec;
            StrToU32(tbMax21.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_BK2_INDEX + 4, (U16) USec);
        }

        private void bMax21_Click(object sender, EventArgs e)
        {
            SetMax21();
        }

        private void SetMax22()
        {
            U32 USec;
            StrToU32(tbMax22.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_BK2_INDEX + 5, (U16) USec);
        }

        private void bMax22_Click(object sender, EventArgs e)
        {
            SetMax22();
        }

        private void SetMax23()
        {
            U32 USec;
            StrToU32(tbMax23.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_BK2_INDEX + 6, (U16) USec);
        }

        private void bMax23_Click(object sender, EventArgs e)
        {
            SetMax23();
        }

        private void SetMax24()
        {
            U32 USec;
            StrToU32(tbMax24.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_BK2_INDEX + 7, (U16) USec);
        }

        private void bMax24_Click(object sender, EventArgs e)
        {
            SetMax24();
        }

        private void SetMax25()
        {
            U32 USec;
            StrToU32(tbMax25.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_BK2_INDEX + 8, (U16) USec);
        }

        private void bMax25_Click(object sender, EventArgs e)
        {
            SetMax25();
        }

        private void SetMax26()
        {
            U32 USec;
            StrToU32(tbMax26.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_BK2_INDEX + 9, (U16) USec);
        }

        private void bMax26_Click(object sender, EventArgs e)
        {
            SetMax26();
        }

        private void SetMax27()
        {
            U32 USec;
            StrToU32(tbMax27.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_BK2_INDEX + 10, (U16) USec);
        }

        private void bMax27_Click(object sender, EventArgs e)
        {
            SetMax27();
        }

        private void SetMax28()
        {
            U32 USec;
            StrToU32(tbMax28.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_BK2_INDEX + 11, (U16) USec);
        }

        private void bMax28_Click(object sender, EventArgs e)
        {
            SetMax28();
        }

        private void SetMax29()
        {
            U32 USec;
            StrToU32(tbMax29.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_BK2_INDEX + 12, (U16) USec);
        }

        private void bMax29_Click(object sender, EventArgs e)
        {
            SetMax29();
        }

        private void SetMax30()
        {
            U32 USec;
            StrToU32(tbMax30.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_BK2_INDEX + 13, (U16) USec);
        }

        private void bMax30_Click(object sender, EventArgs e)
        {
            SetMax30();
        }

        private void SetMax31()
        {
            U32 USec;
            StrToU32(tbMax31.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_BK2_INDEX + 14, (U16) USec);
        }

        private void bMax31_Click(object sender, EventArgs e)
        {
            SetMax31();
        }

        private void SetMax32()
        {
            U32 USec;
            StrToU32(tbMax32.Text, out USec);
            SetServoUsecLoc(EXT_SERVO_BASE_BK2_INDEX + 15, (U16) USec);
        }

        private void bMax32_Click(object sender, EventArgs e)
        {
            SetMax32();
        }

        private void button1_Click_1(object sender, EventArgs e)
        {
            GetSlaveParameter(0, 0);
        }

        private void tbar1_Scroll(object sender, EventArgs e)
        {
            U32 Temp;
            U32 UsecMin, UsecMax;
            StrToU32(tbMin01.Text, out UsecMin);
            StrToU32(tbMax01.Text, out UsecMax);

            Temp = SetPegUsecLoc(UsecMin, UsecMax, tbar1.Value, EXT_SERVO_BASE_INDEX + 0);
            tbVal1.Text = Temp.ToString();

            Temp = (U32)tbar1.Value / 10;
            cbVal1.Text = Temp.ToString();
        }

        private void tbar2_Scroll(object sender, EventArgs e)
        {
            U32 Temp;
            U32 UsecMin, UsecMax;
            StrToU32(tbMin02.Text, out UsecMin);
            StrToU32(tbMax02.Text, out UsecMax);

            Temp = SetPegUsecLoc(UsecMin, UsecMax, tbar2.Value, EXT_SERVO_BASE_INDEX + 1);
            tbVal2.Text = Temp.ToString();

            Temp = (U32)tbar2.Value / 10;
            cbVal2.Text = Temp.ToString();
        }

        private void tbar3_Scroll(object sender, EventArgs e)
        {
            U32 Temp;
            U32 UsecMin, UsecMax;
            StrToU32(tbMin03.Text, out UsecMin);
            StrToU32(tbMax03.Text, out UsecMax);

            Temp = SetPegUsecLoc(UsecMin, UsecMax, tbar3.Value, EXT_SERVO_BASE_INDEX + 2);
            tbVal3.Text = Temp.ToString();

            Temp = (U32)tbar3.Value / 10;
            cbVal3.Text = Temp.ToString();
        }

        private void tbar4_Scroll(object sender, EventArgs e)
        {
            U32 Temp;
            U32 UsecMin, UsecMax;
            StrToU32(tbMin04.Text, out UsecMin);
            StrToU32(tbMax04.Text, out UsecMax);

            Temp = SetPegUsecLoc(UsecMin, UsecMax, tbar4.Value, EXT_SERVO_BASE_INDEX + 3);
            tbVal4.Text = Temp.ToString();

            Temp = (U32)tbar4.Value / 10;
            cbVal4.Text = Temp.ToString();
        }


        private void tbar5_Scroll(object sender, EventArgs e)
        {
            U32 Temp;
            U32 UsecMin, UsecMax;
            StrToU32(tbMin05.Text, out UsecMin);
            StrToU32(tbMax05.Text, out UsecMax);

            Temp = SetPegUsecLoc(UsecMin, UsecMax, tbar5.Value, EXT_SERVO_BASE_INDEX + 4);
            tbVal5.Text = Temp.ToString();

            Temp = (U32)tbar5.Value / 10;
            cbVal5.Text = Temp.ToString();
        }

        private void tbar6_Scroll(object sender, EventArgs e)
        {
            U32 Temp;
            U32 UsecMin, UsecMax;
            StrToU32(tbMin06.Text, out UsecMin);
            StrToU32(tbMax06.Text, out UsecMax);

            Temp = SetPegUsecLoc(UsecMin, UsecMax, tbar6.Value, EXT_SERVO_BASE_INDEX + 5);
            tbVal6.Text = Temp.ToString();

            Temp = (U32)tbar6.Value / 10;
            cbVal6.Text = Temp.ToString();
        }

        private void tbar7_Scroll(object sender, EventArgs e)
        {
            U32 Temp;
            U32 UsecMin, UsecMax;
            StrToU32(tbMin07.Text, out UsecMin);
            StrToU32(tbMax07.Text, out UsecMax);

            Temp = SetPegUsecLoc(UsecMin, UsecMax, tbar7.Value, EXT_SERVO_BASE_INDEX + 6);
            tbVal7.Text = Temp.ToString();

            Temp = (U32)tbar7.Value / 10;
            cbVal7.Text = Temp.ToString();
        }

        private void tbar8_Scroll(object sender, EventArgs e)
        {
            U32 Temp;
            U32 UsecMin, UsecMax;
            StrToU32(tbMin08.Text, out UsecMin);
            StrToU32(tbMax08.Text, out UsecMax);

            Temp = SetPegUsecLoc(UsecMin, UsecMax, tbar8.Value, EXT_SERVO_BASE_INDEX + 7);
            tbVal8.Text = Temp.ToString();

            Temp = (U32)tbar8.Value / 10;
            cbVal8.Text = Temp.ToString();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
        }
    }
}
