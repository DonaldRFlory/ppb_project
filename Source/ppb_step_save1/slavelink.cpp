// ..\..\output\ppb_arduino\slavelink.cpp**************************************
// This file is machine generated from the link definition file
// Output\ppb_arduino\slvmainll.h by LDFUTIL V2_015
// Processed in Mode: 0 - Standard stubs Mode
// It should not be directly edited.

#include "slink.h"
#include "slvmainll.h"


struct LinkDef  SDef[12]=
{
	{(void (*)())MasterBlockDown, 1, 1, 2, 0, 0},
	{(void (*)())MasterBlockUp, 1, 2, 1, 0, 0},
	{(void (*)())GetSlaveParameter, 4, 1, 1, 0, 0},
	{(void (*)())SetSlaveParameter, 4, 1, 2, 2, 0},
	{(void (*)())SetServoUsec, 1, 1, 2, 0, 0},
	{(void (*)())GetServoUsec, 2, 1, 0, 0, 0},
	{(void (*)())DataUpdate, 1, 1, 1, 0, 0},
	{(void (*)())SetSolenoid, 1, 1, 1, 0, 0},
	{(void (*)())ServoSlewMove, 0, 1, 2, 4, 0},
	{(void (*)())SetStep, 0, 1, 2, 2, 1},
	{(void (*)())ProgramEEPROM, 0, 2, 1, 0, 0},
	{(void (*)())ReadEEPROM, 1, 2, 0, 0, 0}
};

int  LinkCount = 12;
