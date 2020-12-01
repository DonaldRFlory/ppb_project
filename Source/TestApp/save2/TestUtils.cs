//This file contains helper functions for the test menus
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
		private const U16 INIT_CRC	 = 0;
		private const U16 CRCPOLY	 = 0XA001; // ANSI CRC-16,  CCITT: 0x8408
		private const U16 CHAR_BIT	 = 8;
		private const U16 UCHAR_MAX	 = 255;

		public U16 ComputeCRC(byte [] Buff, U32 CharCount)
		{
			U16 CRC;
			U16 [] CRCTable = new U16[UCHAR_MAX + 1];

			U16 j, r;
			for (U16 i = 0; i <= 255; i++)
			{
				r = i;
				for (j = 0; j < CHAR_BIT; j++)
				{
					if ((r & 1)!=0)
					{
				        r = (U16)((r >> 1) ^ CRCPOLY);
					}
					else
					{
			        	r >>= 1;
			      	}
			    }
		    	CRCTable[i] = r;
			}
			CRC = INIT_CRC;
			for(U32 i=0; i < CharCount; ++i)
			{
				CRC = (U16)(CRCTable[(CRC ^ (Buff[i] & 0xFF)) & 0xFF] ^ (CRC >> CHAR_BIT));
			}
			return CRC;
		}

		bool MemoryBlockUp(U32 Src, U8[] Dest, U32 Count)
		{
//			U16 Chunk;
//			int Idx;
//			U16 MaxPayloadSize = (U16)GetMaxLinkReturnSize();
//            byte[] Buff = new byte[MaxPayloadSize];
//
//            ProgBarLabel.Text = "Memory Block Up";
//            ProgBarLabel.Refresh();
//            progressBar1.Show();
//            progressBar1.Maximum = (int)Count;
//			Chunk = (U16)(MaxPayloadSize-1);//account for U8 MasterBlockUp() return value
//			for (Idx = 0; Idx < Count; Idx += Chunk)
//			{
//				if(Idx + Chunk > Count)
//				{ //handle possible shorter final packet
//					Chunk = (U16)(Count - Idx);
//				}
//				if(MasterBlockUp(Src, ref Buff, Chunk) == 0)
//				{
//		            ProgBarLabel.Text = "";
//		            progressBar1.Hide();
//					return false;
//				}
//                for (int i = 0; i < Chunk; ++i)
//                {
//                   Dest[Idx + i] = Buff[i];
//               }
//			Src += Chunk;
//           	progressBar1.Value = (int)Idx + 1;
//           	progressBar1.Value = (int)Idx;
//		}
//            ProgBarLabel.Text = "";
//            progressBar1.Hide();
			return true;
		}


		bool MemoryBlockDown(U8[] Src, U32 Count, U32 Dest)
		{
//			U16 Chunk;
//			int Idx;
//			U16 MaxPayloadSize = (U16)GetMaxLinkSendSize();
//			//Maximum chunk we can send in one call is packet size less the
//			//address and count arguments		Dest		  Count	 FIdx
//			Chunk = (U16)(MaxPayloadSize - sizeof(U32) - sizeof(U16) - 1);
//    		Chunk = (U16)(Chunk & ~0X03); //so we can program flash, chunks need to be even mod4
//    		byte[] Buff = new byte[MaxPayloadSize];
//
//            ProgBarLabel.Text = "Memory Block Down";
//            ProgBarLabel.Refresh();
//            progressBar1.Show();
//            progressBar1.Maximum = (int)Count;
//			for (Idx = 0; Idx < Count; Idx += Chunk)
//			{
//				if(Idx + Chunk > Count)
//				{ //handle possible shorter final packet
//					Chunk = (U16)(Count - Idx);
//				}
//                for (int i = 0; i < Chunk; ++i)
//                {
//                    Buff[i] = Src[Idx + i];
//                }
//				if(MasterBlockDown(ref Buff, Chunk, Dest) == 0)
//				{
//		            ProgBarLabel.Text = "";
//		            progressBar1.Hide();
//					return false;
//				}
//				Dest += Chunk;
//            	progressBar1.Value = (int)Idx + 1;
//            	progressBar1.Value = (int)Idx;
//			}
//            ProgBarLabel.Text = "";
//          progressBar1.Hide();
			return true;
		}

		public void GetProcGUID()
		{
			
		}

		public U32 CalcMemCRC(U32 Start, U32 End)
		{
            //SetSlaveParameter(SSP_CS_START, Start, 0);
            //SetSlaveParameter(SSP_CS_END, End, 0);
            //return GetSlaveParameter(SSP_CS_CALC, 2);
            return 0;

        }

		public U32 FileDownload(U32 Start)
		{
		    byte[] Buff = null;
            string FileName;
            if (!GetInputString("Enter file path", out FileName))
            {
                return 0;
            }
            FileStream fs = new FileStream(FileName, FileMode.Open, FileAccess.Read);
		    BinaryReader br = new BinaryReader(fs);
		    int NumBytes = (int)new FileInfo(FileName).Length;
		    Buff = br.ReadBytes(NumBytes);
			MemoryBlockDown(Buff, (U32)NumBytes, Start);
            return (U32)NumBytes;
        }

		public U16 UploadToFile(U32 Start, U32 Length)
		{
			U16 CRC = 0;
			byte [] Buffer = new byte[Length];
            string Path;
			if(!GetInputString("Enter file path", out Path))
			{
				return 0;
			}
			if(!MemoryBlockUp(Start, Buffer, Length))
			{
				return 0;
			}
			using(BinaryWriter Writer = new BinaryWriter(File.Open(Path, FileMode.Create)))
			{
				Writer.Write(Buffer);
			}

			CRC = ComputeCRC(Buffer, Length);
            return CRC;
        }
	}
}
