@echo OFF
rem --------------------------------------------------------------------------
rem               Copyright (C) Bio-Rad Laboratories 2017
rem File:         Postbuild.cmd
rem Purpose:      Postbuild for Main Firmware
rem		  
rem               
rem                  
rem  Created by:   Don Flory 
rem  Cr. Date:     2-01-2017 
rem --------------------------------------------------------------------------

rem   Current directory must be the location of this script.

echo.
md ..\..\Output
md ..\..\Output\Firmware

REM Main board linklist
..\Tools\Internal\ldfutil -C0  -x1 -i..\common\mainll.h -s..\..\Output\Firmware\slavemll.c -q..\..\Output\Firmware\mainll.h

exit /B 0





