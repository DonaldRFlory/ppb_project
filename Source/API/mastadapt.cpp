//--------------------------------------------------------------------------*
//
//                  Copyright (C) Bio-Rad Laboratories 2020
//
//  MODULE :        mastadapt.c
//  Purpose:        Adapter functions to connect generic master.c to
//                  a specific project
//
//
//--------------------------------------------------------------------------*/
#include "mastadapt.h"
#include "linkctrl.h"

//This is here because different implementations may use an external buffer
//rather than the one that is part of LINK_CTRL. Multiple channel implentation
//may use different schemes for different  for different channels. Also
//LCB_TOTSIZE may vary for different implementations.
L_STAT InitLCtrl(LINK_CTRL &LCtrl)
{
	U32 BuffSize = 0;
	//L_STAT LStat;
	//if(LCtrl.LSel.ChannelIndex == DC_CH)
	//{
	//	//So... If we are in local mode, we will not be allowing calls to DC_CH channel
	//	//Hence GetInstrumentBuffer should fail. In Remote mode, there should be
	//	//a buffer for each Instrument.
	//	LStat = CHAPIGetInstrumentBuffer(LCtrl.LSel.LHand, LCtrl.Buffer, BuffSize);
	//	if(LStat != LE_NO_ERROR)
	//	{
	//		return LStat;
	//	}
	//	/LCtrl.BuffSize = BuffSize;
	//}
	//else
	//{ //for device transactions we will use the small buffer in
      //LCtrl structure allocated on the stack.
		LCtrl.Buffer = LCtrl.LCBuff;
		LCtrl.BuffSize = LCB_TOTSIZE;
		for(U32 i = 0; i < LCtrl.BuffSize; ++i)
		{
			LCtrl.Buffer[i] = 0;//small buffer to aid debugging
		}
	//}
	LCtrl.StartIndex = LCtrl.NextIndex = LCB_PRESPACE;
	LCtrl.FIdx = 255;  //Illegal value
	LCtrl.RtnSize = 0;
	return LE_NO_ERROR;
}


#if 0
//-----------------------------------------------------------------
//Convert info in LStat structure to APILinkStat (one format of API_STAT)
// 12-bit LinkStat (in 0-255 range),  4-bit Source, 4-bit Channel, 4-bit unused, 8-bit FIdx
// packed most sig bit to least sig bit in 32 bit word
//------------------------------------------------------------------
U32 LinkStatToAPIStat(LINK_STAT LStat)
{
	API_STAT LinkAPIStat;
	U32 Channel, Stat, Source;

	if(LStat.Stat == LE_NO_ERROR)
	{
		return 0;
	}
	Channel = LStat.Channel & 0X0F;
	Stat = LStat.Stat;
	#if 0
    switch(LStat.CommType)
	{
		case CT_LOCAL:
			Source = ERR_SRC_LINK_L;
			break;

		case CT_INST_1:
			Source = ERR_SRC_LINK_R0;
			break;

		case CT_INST_2:
			Source = ERR_SRC_LINK_R1;
			break;

		case CT_INST_3:
			Source = ERR_SRC_LINK_R2;
			break;

		case CT_INST_4:
			Source = ERR_SRC_LINK_R3;
			break;

		default:
			Source = ERR_SRC_LINK_RX; //unrecognized remote handle
			break;
	}
	#else
	    Source = 0;
	#endif
	LinkAPIStat = (Stat << 20) | (Source << 16) | (Channel << 12) | LStat.FIdx;
	//if(Stat != LE_BLOCK_UP_SHORT)
	//{
	//    CMAPILogError(LinkAPIStat);//we are going to let this slide for benefit of CMAPICFXCommand()
	//}
	return LinkAPIStat;
}
#endif

//for now, we will pack the link call macro status in low byte.
//We may pack more info in higher three bytes.
bool GoodLinkStatus(LINK_STAT LStat)
{
	return (LStat.Stat & 0xFF) == LE_NO_ERROR;
}


//In some setups, it may be desired to not actually do transactions
//destined for certain Channels or Handles and instead to pretend they
//were done successfully. If this function returns TRUE, Link() returns
//zero with LinkStat->Stat set to LE_NO_ERROR. For false return, Link()
//actually calls LinkTransact() and returns its status.
bool DummyTransaction(LINK_CTRL &LCtrl)
{
   	return false;
}

//Checks if LSel values are valid for particular implementation. May include
//validating ChannelIndex, LHand and any other elements. Also sometimes
//sets up ReadSize and WriteSize based on any Channl or CommType for instance
//so that reads and writes at physical level are appropriate. It can also
//be stubbed out and just return true.
//bool ValidateLinkSel(LINK_SEL &LSel)
//{
//    return true;
//}

//This is the maximum raw link return packet, that is return value bytes
//(if any) and block up bytes (if any). This must be adapted based on
//the physical transport wrapper and the buffer size used by LCtrl so that
//the entire physical layer packet will fit in the buffer. With most physical
//links, there are some wrapper and error checking bytes added to the raw
//link packet and value returned here is less than the buffer size.
//U32 GetMaxLinkReturnSize(U8 Channel)
//{
//    return 0;
//}

//This is the maximum raw link command packet, that is, the function code byte,
//and any parameter bytes, and any block down bytes.
//This must be adapted based on
//the physical transport wrapper and the buffer size used by LCtrl so that
//the entire physical layer packet will fit in the buffer. With most physical
//links, there are some wrapper and error checking bytes added to the raw
//link packet and value returned here is less than the buffer size.
U32 GetMaxLinkSendSize(U8 Channel)
{
    return 0;
}
