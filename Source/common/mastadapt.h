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
#define GOOD_LINK_STATUS(LStat) (LStat.Stat == LE_NO_ERROR)
#define LINK_STATUS(LStat) LinkStatus(LStat)
#define VALIDATE_LINK_SEL(LSel)  ValidateLinkSel(LSel)
#define MAX_SEND_SIZE(LSel)  GetMaxLinkSendSize(LSel)
#define MAX_RETURN_SIZE(LSel) GetMaxLinkReturnSize(LSel)
#define LINK_TRANSACT(LCtrl) LinkTransact(LCtrl)
#define GET_BOARD_ADDRESS() GetBoardAddress()
#define MAX_FIDX 0X1F
#define BOARD_ADDRESS_MASK 0XE0
#define BOARD_ADDRESS_LEFT_SHIFT 5

bool ValidateLinkSel(LINK_SEL &LSel);
U32 GetMaxLinkSendSize(LINK_SEL LSel);
U32 GetMaxLinkReturnSize(LINK_SEL LSel);
//L_STAT LinkTransact(LINK_CTRL &LCtrl);

U8 InitLCtrl(LINK_CTRL &LCtrl); //Mainly to setup buffer and indices and buffer length
U32 LinkStatToAPIStat(LINK_STAT LStat);
void LinkStatus(LINK_STAT LStat);
bool GoodLinkStatus(LINK_STAT LStat);
bool DummyTransaction(LINK_CTRL &LCtrl);
//LINK_STAT CMAPILinkTransact(LINK_CTRL &LCtrl);
U8 GetBoardAddress();
