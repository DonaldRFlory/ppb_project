//--------------------------------------------------------------------------*
//                  Copyright (C) Bio-Rad Laboratories 1998
//
//  Module:          LINK.H
//  Purpose:         Definitions of link information needed by both master
//                   and slave sides of interprocessor link.
//
//
// $Revision: 1 $
// $Author: Dflory $
// $Date: 1/12/12 5:01p $
//
// $History: link.h $
//
//
//--------------------------------------------------------------------------*/
//#pragma message("---3GTestSW/include/link.h")
#ifndef LINK_H
#define LINK_H
#define INC_FIL(Name)    <Encaps(Name)>
#define Encaps(A) A
#include "type.h"

//Some implementations of link have party-line serial bus and allow addressing of eight boards
//with three top bits of function index.
//This leaves us with a maximum of 32 possible link functions in linklist
#define BOARD_ADDRESS_MASK  0XE0

//no built-ins for new LDFUtil
#define LINK_BLOCK_DOWN 0
#define LINK_BLOCK_UP   1
#define LINK_STRING_DOWN 2
#define LINK_STRING_UP   3
#define NUM_LINK_BUILT_INS 0
#define MAX_ARGS 4
//for new ldfutil circa 10-1-2016 we can have up to four pointers and four counts for
//block xfer functions. They are consolidated by ldfutil into max of 4 arguments for link calls
//so link DEF structures need only MAX_ARGS
#define MAX_RAW_ARGS (2 * MAX_ARGS)
#define MAX_FUNS 255


//This is thought to be ridiculously large for a single packet. Mainly
//so we can avoid overflowing int when computing return size for checking
#define MAX_LINK_BLOCK_SIZE  0x10000

//Arg length flags to indicate block xfer counts and use
#define PTR_TYPE_MASK 0XE0
#define F_DOWN_PTR_U8	0X20
#define F_DOWN_PTR_U16  0X40
#define F_DOWN_PTR_U32  0X60
#define F_UP_PTR_U8	  	0XA0
#define F_UP_PTR_U16	0XC0
#define F_UP_PTR_U32	0XE0

//NOTE: F_DOWN_PTR_U32 and F_UP_PTR_U32 in conjunction with F_FLOAT flag indicate a float pointer
//float and U32 are the same size of 4.
//Special flag to indicate float in argsize value
#define F_FLOAT     	0X10
#define ARG_SIZE_MASK  	0X0F

#ifndef TYPE_H
//Link pointer types:
typedef unsigned char * UP_PTR_U8;
typedef unsigned short * UP_PTR_U16;
typedef unsigned int * UP_PTR_U32;
typedef float * UP_PTR_FLOAT;
typedef unsigned char * DOWN_PTR_U8;
typedef unsigned short * DOWN_PTR_U16;
typedef unsigned int * DOWN_PTR_U32;
typedef float * DOWN_PTR_FLOAT;
#endif

#ifndef LINK_ERROR_H
#include "linkerror.h"
#endif

//Byte order note:
//  Bytes are always transmitted in MOTOROLA or BIG_ENDIAN order.
//  If slave uses INTEL (LITTLE_ENDIAN) order, it must reshuffle bytes.

#endif







































