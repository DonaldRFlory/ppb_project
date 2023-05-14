@echo OFF
rem --------------------------------------------------------------------------
rem               Copyright (C) Bio-Rad Laboratories 2017
rem File:         PostBuildDebug.cmd
rem Purpose:      Postbuild cmd file for PPHC API DLL
rem
rem  Created by:   Don Flory
rem --------------------------------------------------------------------------

rem   Current directory must be the location of this script.
echo.

md ..\..\Output\TestApp\Debug
copy /y ..\..\Output\API\debug\PPBAPI.DLL ..\..\Output\TestApp\Debug

echo.
exit /B 0
