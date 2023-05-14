//*****************************************************************************
//	linkadapt.c
//
// This module adapts generic slave link functions to use multiple
// physical links.
//
//*****************************************************************************
#include "hostopsys.h"
#include "link.h"
#include "slink.h"
//#include "apilinkadapt.h"
#include "linkctrl.h"

//Modify this to return number of physical links
int GetNumLinks(void)
{
    return 1;
}

//static U8 LinkIndex;//Set during LinkServe processing to indicate the link we are
	//servicing. It will allow us to possibly return function def from a different
	//LinkDef table for each link in GetFDef().

void SetLinkIndex(U8 Index)
{
//    LinkIndex = Index;
}


U8 LinkBuffer[63];
static U8 LinkReturnStatus;

//This should be modified to initialize all implemented slave links
void InitSlaveLink(void)
{
}

U32 GetMaxLinkReturnSize(U8 Channel)
{
    return 0;//This should be setup based on physical transport
        //it is the maximum allowed size of raw link return packet
        //dictated by the physical layer buffering and packe t size
}

//Dummy, not supported on 3GCommAPI
void PostLogicError(U16 ErrorIdentifier)
{

}

//Multiple links may share the same linklist
//or they may have distinct linklists.
//Modify this to handle various LinkDef structures
extern int LinkCount;
extern struct LinkDef  SDef[];

U32 GetCurFDef(struct LinkDef **FDefP)
{
	*FDefP = &(SDef[0]);
	return (U32)LinkCount;
}


int GetLinkCommandPacket(U8 **CmdPtrAddress, U8 **RspPtrAddress, U32 *MaxReturnSizeAddress)
{
	U8 Length;
	*CmdPtrAddress = &(LinkBuffer[1]); //we put length byte then raw packet in LinkBuffer
    *RspPtrAddress  = &(LinkBuffer[2]);//leave room for length byte and return status
    *MaxReturnSizeAddress = 61;//size of max raw return packet, serial buffer holds 63 bytes and we add length byte
                                // and status before raw packet
    Length = LinkBuffer[0];
    LinkBuffer[0] = 0;//clear length in command packet so we can see if a result gets posted
    return Length;
}

void PostLinkResult(U32 ByteCount, U8 ReturnStatus, U8 FIdx)
{
    LinkBuffer[0] = (U8)(ByteCount+1);
    LinkBuffer[1] = ReturnStatus;
}
