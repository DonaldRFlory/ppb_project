@echo OFF
rem --------------------------------------------------------------------------
rem               Copyright (C) Bio-Rad Laboratories 2016
rem File:         PreBuild2013.cmd
rem Purpose:      PreBuild for API DLL
rem
rem
rem
rem  Created by:   Don Flory
rem --------------------------------------------------------------------------

rem   Current directory must be the location of this script.

echo Start Building Link list files
echo.

rem //Following produces DLL exports stubs with 13 name prefix. This
rem //allows a single test program to call functions in multiple sub-instruments without
rem  //name collisions:

md ..\..\Output\
md ..\..\Output\API\

rem Standard stubs for link functions called within DLL
..\Tools\ldfutil -x0 -i..\Common\mainll.h -p..\..\output\API\dlllink.h -m..\..\output\API\dlllink.cpp -s..\..\output\API\junk.out

rem the automatic DLL exports for link functions
..\Tools\ldfutil -x2  -HELHdr.h  -i..\Common\mainll.h -p..\..\output\API\ExpLink.h -m..\..\output\API\ExpLink.cpp -s..\..\output\API\junk1.out -B

rem ..\tools\internal\ldfutil -x4   -i..\Common\mainll.h -p..\..\output\API\ExpAdpt.h -m..\..\output\API\ExpAdpt.cpp -s..\..\output\API\junk2.out

echo End of Building Link list files
echo.
exit /B 0
