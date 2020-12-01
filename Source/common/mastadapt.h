//--------------------------------------------------------------------------*
//
//                  Copyright (C) Bio-Rad Laboratories 2020
//
//  MODULE :        mastadapt.c
//  Purpose:        Adapter functions to connect generic master.c to
//                  a specific project
//
//

#include "linkctrl.h"
#include "linkerror.h"
#include "api.h"
#define VALIDATE_LINK_SEL(LSel)  ValidateLinkSel(LSel)
#define MAX_SEND_SIZE(LSel)  GetMaxLinkSendSize(LSel)
#define MAX_RETURN_SIZE(LSel) GetMaxLinkReturnSize(LSel)
#define LINK_TRANSACT(LCtrl) LinkTransact(LCtrl)

bool ValidateLinkSel(LINK_SEL &LSel);
U32 GetMaxLinkSendSize(LINK_SEL LSel);
U32 GetMaxLinkReturnSize(LINK_SEL LSel);
L_STAT LinkTransact(LINK_CTRL &LCtrl);

U8 InitLCtrl(LINK_CTRL &LCtrl); //Mainly to setup buffer and indices and buffer length
U32 LinkStatToAPIStat(LINK_STAT LStat);
bool GoodLinkStatus(LINK_STAT LStat);
bool DummyTransaction(LINK_CTRL &LCtrl);
LINK_STAT CMAPILinkTransact(LINK_CTRL &LCtrl);
