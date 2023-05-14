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
static U8 BoardAddress = 0;
extern LINK_STAT_CB_P LinkStatCB;
LINK_STAT LastStatus;

//This is here because different implementations may use an external buffer
//rather than the one that is part of LINK_CTRL. Multiple channel implentation
//may use different schemes for different  for different channels. Also
//LCB_TOTSIZE may vary for different implementations.
L_STAT InitLCtrl(LINK_CTRL &LCtrl)
{
	U32 BuffSize = 0;
	//LCtrl.LSel.ChannelIndex = BoardAddress;
	LCtrl.Buffer = LCtrl.LCBuff;
	LCtrl.BuffSize = LCB_TOTSIZE;
	for(U32 i = 0; i < LCtrl.BuffSize; ++i)
	{
		LCtrl.Buffer[i] = 0;//small buffer to aid debugging
	}
	LCtrl.StartIndex = LCtrl.NextIndex = LCB_PRESPACE;
	LCtrl.StartIndex = LCtrl.NextIndex = LCB_PRESPACE;
	LCtrl.FIdx = 255;  //Illegal value
	LCtrl.RtnSize = 0;
	return LE_NO_ERROR;
}

bool GoodLinkStatus(LINK_STAT LStat)
{
	return (LStat.Stat == LE_NO_ERROR);
}


//Used to report result of link transaction
void LinkStatus(LINK_STAT Status)
{
    LastStatus = Status;
    if(LinkStatCB != 0)
    {
        //report link error to foreground via callback if installed.
        (LinkStatCB)(Status.Stat, Status.FIdx, Status.Channel, Status.CommType);
    }
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


 //sets board address for all serial calls on party line serial bus
 void SetBoardAddressAdapt(U8 Address)
 {
     BoardAddress = Address;
 }

 U8 GetBoardAddress()
 {
    return BoardAddress;
 }
