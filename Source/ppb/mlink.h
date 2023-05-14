//---------------------------------------------------------------------------
//
//                 Copyright (C) Bio-Rad Laboratories 1997
//
// Include file: MLINK.H
//
// Created:   Don Flory circa 1989
//
// Modified:
//            DF 1-24-94 Removed link errors enum. Link error handling was
//              merged with root error handling.
//---------------------------------------------------------------------------
#ifndef MLINK_H
#define MLINK_H

#include "type.h"
#include "linkerror.h"
#include "link.h"
#include "linkctrl.h"

#define NO_CHAR -1
#define INVALID_FUNCTION_INDEX 255
#define INVALID_CHANNEL_INDEX 255

typedef union
{
    float F[sizeof(U64)/sizeof(float)];
    U64 X64;
    U32 X32[sizeof(U64)/sizeof(U32)];
    U16 X16[sizeof(U64)/sizeof(U16)];
    U8 X8[sizeof(U64)/sizeof(U8)];
}B64Union;

#ifndef LIC_DEFINED
#define LIC_DEFINED 1
typedef union
{
    unsigned long l;
    unsigned short i[sizeof(long)/sizeof(unsigned short)];
    unsigned char c[sizeof(long)];
}LICUnion;

typedef union
{
    unsigned int I;
    unsigned short S[sizeof(unsigned int)/sizeof(unsigned short)];
    unsigned char C[sizeof(unsigned int)];
}ISCUnion;

typedef union
{
    unsigned int i;
    unsigned char c[sizeof(int)];
}ICUnion;
#endif

#ifndef MAX_ARGS
#define MAX_ARGS 4
#endif

typedef struct
{
  unsigned char fsz, asz[MAX_ARGS];
}MLFUN_DEF;

//Master side link function definition structure.
typedef struct
{
  unsigned int FunctionCount;
  MLFUN_DEF *FDef;
} MLINK_DEF;

U64 Link(LINK_SEL LSel, MLINK_DEF *MDef, U8 FunCode, ...);
L_STAT LinkTransact(LINK_CTRL &LCtrl);


//Functions for legacy programs using old style link calls------------
bool LinkBlockDown(U8 *Source, U32 Dest, U32 Count);
bool LinkBlockUp(U32 Src, U8 * Dest, U32 Count);


U32 LinkStatToAPIStat(LINK_STAT LStat);

//Legacy error handling
void DisplayLinkError(U16 FunCode, LINK_STAT LStat);
//int SetLittleEndian(int State);
void LinkError(LINK_STAT LStat);
void ClearLinkError(void);
#endif   //of #ifdef MLINK_H
