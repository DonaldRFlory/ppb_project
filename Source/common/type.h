/****************************************************************************
 *   $Id:: type.h 3632 2010-06-01 22:54:42Z usb00423                        $
 *   Project: NXP LPC13xx software example
 *
 *   Description:
 *     This file contains different type definition.
 *
 ****************************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
****************************************************************************/
//#pragma message("compiling type.h")
#include "hostopsys.h"
#ifndef TYPE_H
#define TYPE_H
#ifndef TRUE
    #define TRUE 1
    #define FALSE 0
#endif

#define INT_IS_16_BITS
/* exact-width unsigned integer types */
//This version is for Arduino nano
#ifdef INT_IS_16_BITS
typedef char S8;
typedef unsigned char U8;
typedef unsigned char UCHAR;
typedef unsigned short U16;
typedef short 		S16;
typedef unsigned long U32;
typedef long S32;
typedef unsigned   long long U64;

typedef unsigned char * UP_PTR_U8;
typedef unsigned short * UP_PTR_U16;
typedef unsigned long * UP_PTR_U32;
typedef unsigned char * DOWN_PTR_U8;
typedef unsigned short * DOWN_PTR_U16;
typedef unsigned long * DOWN_PTR_U32;
typedef float   * DOWN_PTR_FLOAT;
typedef float   * UP_PTR_FLOAT;
#else
typedef char S8;
typedef unsigned char U8;
typedef unsigned char UCHAR;
typedef unsigned short U16;
typedef short 		S16;
typedef unsigned int U32;
typedef int S32;
typedef unsigned   long long U64;

typedef unsigned char * UP_PTR_U8;
typedef unsigned short * UP_PTR_U16;
typedef unsigned int * UP_PTR_U32;
typedef unsigned char * DOWN_PTR_U8;
typedef unsigned short * DOWN_PTR_U16;
typedef unsigned int * DOWN_PTR_U32;
typedef float   * DOWN_PTR_FLOAT;
typedef float   * UP_PTR_FLOAT;
#endif

#pragma message("end of compiling type.h")

#endif  /*TYPE_H*/
