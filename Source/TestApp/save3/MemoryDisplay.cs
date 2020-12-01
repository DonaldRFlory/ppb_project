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
    public partial class Form1 : Form
    {
        private void MemHexLine(ref StringBuilder Hex, byte [] Bytes, U32 Address, int Index)
		{
        	Hex.AppendFormat("0X{0:X4}_{1:X4}  ", Address >> 16, (U16)Address);
			for(int i = 0; i < 16; ++i)
			{
                Hex.AppendFormat("{0:X2} ", Bytes[Index + i]);
                if (i % 4 == 3)
                {
                    Hex.AppendFormat(" ");
                }
            }
			for(int i = 0; i < 16; ++i)
            if((Bytes[Index + i] < 0X20) || (Bytes[Index + i] > 0x7F))
            {
                Hex.Append("*");
            }
            else
            {
                Hex.Append(System.Text.Encoding.UTF8.GetString(Bytes, Index + i, 1));
            }
        }

        private bool StrToU32(string StrIn, out U32 Value)
        {
            string Temp;
            if(StrIn.StartsWith("0x") || StrIn.StartsWith("0X"))
            {
                Temp = StrIn.Substring(2, StrIn.Length - 2);
	            try
    	        {
                    Value = Convert.ToUInt32(Temp, 16);
                }
	            catch
	            {
                    Value = 0;
                    return false;
	            }
				return true;
    		}
    		else if(U32.TryParse(StrIn, out Value))
			{
				return true;
			}
            Value = 0;
            return false;
       	}

        private void UpdateMemPage()
		{
            string Temp;
            string BoxStr = TBMemAddress.Text;
            BoxStr = BoxStr.TrimEnd();
            try
            {
                if (BoxStr.StartsWith("0x") || BoxStr.StartsWith("0X"))
                {
                    Temp = BoxStr.Substring(2, BoxStr.Length - 2);
                    m_MemAddress = Convert.ToUInt32(Temp, 16);
                    //m_MemAddress = UInt32.Parse(BoxStr);
                }
                else
                {
                    if (BoxStr.Length > 10)
                    {
                        Temp = BoxStr.Substring(0, 8);
                        m_MemAddress = UInt32.Parse(Temp);
                    }
                    else
                    {
                        m_MemAddress = UInt32.Parse(BoxStr);
                    }
                }
            }
            //catch(Exception MyException)
            catch
            {
                m_MemAddress = 0;
            }
            DisplayMemPage(m_MemAddress);
		}

		private void DisplayMemPage(U32 Address)
        {
//			U32 StartAddress = Address;
//            int Index = 0;
//			byte[] Bytes1 = new byte[128];
//			byte[] Bytes2 = new byte[128];
//			StringBuilder Hex = new StringBuilder(100);
//
//			MasterBlockUp(Address, ref Bytes1, 128);
//			MasterBlockUp(Address + 128, ref Bytes2, 128);
// 			MemTextBox.Clear();
//            MemTextBox.Text += "             00 01 02 03  04 05 06 06  08 09 0A 0B  0C 0D 0E 0F" + Environment.NewLine;
//
//	        for(int i = 0; i < 8; ++i)
//	        {
//		        MemHexLine(ref Hex, Bytes1, Address,  Index);
//                Index += 16;
//                Address += 16;
//	            MemTextBox.Text += Hex.ToString();
//                Hex.Clear();
//                MemTextBox.Text += Environment.NewLine;
//			}
//            Index = 0;
//            for (int i = 0; i < 8; ++i)
//            {
//                MemHexLine(ref Hex, Bytes2, Address, Index);
//                Index += 16;
//                Address += 16;
//                MemTextBox.Text += Hex.ToString();
//                Hex.Clear();
//                MemTextBox.Text += Environment.NewLine;
//            }
//            MemTextBox.Text += "             00 01 02 03  04 05 06 06  08 09 0A 0B  0C 0D 0E 0F";
//            Hex.Clear();
//            Hex.AppendFormat("0X{0:X8}", StartAddress);
//            TBMemAddress.Clear();
//            TBMemAddress.Text = Hex.ToString();
//			TBMemAddress.Focus();
		}

        private void ButLineLower_Click(object sender, EventArgs e)
        {
	        m_MemAddress -= 16;
            DisplayMemPage(m_MemAddress);
        }

        private void ButLineHigher_Click(object sender, EventArgs e)
        {
	        m_MemAddress += 16;
            DisplayMemPage(m_MemAddress);
        }

        private void ButPageLower_Click(object sender, EventArgs e)
        {
	        m_MemAddress -= 256;
            DisplayMemPage(m_MemAddress);
        }

        private void ButPageHigher_Click(object sender, EventArgs e)
        {
	        m_MemAddress += 256;
            DisplayMemPage(m_MemAddress);

        }


        private void TBMemAddress_Leave(object sender, EventArgs e)
        {
         //   string BoxStr = TBMemAddress.Text;
         //   Console.Write(BoxStr);
         //   if (BoxStr.StartsWith("0x") || BoxStr.StartsWith("0X"))
         //   {
         //       m_MemAddress = Convert.ToUInt32(BoxStr.Substring(2), 16);
         //       //m_MemAddress = UInt32.Parse(BoxStr);
         //   }
         //   else
         //   {
         //       m_MemAddress = UInt32.Parse(BoxStr);
         //   }
         //   DisplayMemPage(m_MemAddress);
        }
        private void TBMemAddress_KeyDown(object sender, KeyEventArgs e)
        {
            switch(e.KeyCode)
            {
                case Keys.Up:
	    		    m_MemAddress -= 16;
                    break;

                case Keys.Down:
			        m_MemAddress += 16;
                    break;

                case Keys.PageUp:
			        m_MemAddress -= 256;
                    break;

                case Keys.PageDown:
	    		    m_MemAddress += 256;
                    break;

				default:
					return;
            }
	        DisplayMemPage(m_MemAddress);

        }
    }
}
