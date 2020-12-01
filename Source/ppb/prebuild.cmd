@echo OFF
rem main prebuild file for basic link test setup--------------------------------------------------------------------------
rem calling four different sub-cmd files to ease debugging
rem  Created by:   Don Flory
rem --------------------------------------------------------------------------

echo Start of Building Link list files
echo.

copy /y ..\common\mainll.h .
copy /y ..\common\basicll.h .
if not exist ..\..\output md ..\..\output

rem mode is don't care below. We are generating the slave side cpp file with
rem LinkDef structure:
copy /Y mainll.h  ..\..\output\ppb_arduino\slvmainll.h
rem The copy above  is because a #include of the input file is placed at
rem the top of the slavelink.cpp output file. This should refer to the generated
rem header file which has the UP and DWN pointer parameters stripped out, since
rem the slave version does not have them. After use here in master side form,
rem slavmainll.h is replaced by generated slave side version in later batch
rem file (prebuild4.cmd)

rem so, the input file below though named slvmainll.h is in fact mainll.h
..\tools\ldfutil  -x0 -ioutput\slvmainll.h   -s..\..\output\ppb_arduino\slavelink.cpp

rem mode is don't care below. We are generating the slave side header file with
rem pointer arguments removed for block up or down functions
..\tools\ldfutil -x9 -imainll.h -q..\..\output\ppb_arduino\slvmainll.h

copy /y ..\..\output\ppb_arduino\slvmainll.h .
copy /y ..\..\output\ppb_arduino\slavelink.cpp .
rem copy /Y mainll.h  ..\..\output\ppb_arduino\slvmainll.h
echo.
exit /B 0
