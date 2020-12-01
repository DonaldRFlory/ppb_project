//---------------------------------------------------------------------------
//
//                 Copyright (C) Bio-Rad Laboratories 1997
//
//  Module:        LINKEROR.H
//  Purpose:       Definitions of link error codes
//
//  Created by:    Don Flory
//  Date:          February 6,2018
//
//  Modified by:
//
//
//
//
//---------------------------------------------------------------------------
#ifndef LINK_ERROR_H
#define LINK_ERROR_H
#pragma message("compiling linkerror.h")

#undef ErrorDef
#undef ErrorDefEnumSet
// First produce an error enum
#define ErrorDef( id, strtext) id,
#define ErrorDefEnumSet(id, Num, strtext)  id = Num,

typedef enum
{
	 #include "linkerrdef.h"
} LinkErrorIDs;
#undef  ErrorDef
#undef  ErrorDefEnumSet
#endif //of #ifndef LINK_ERROR_H
