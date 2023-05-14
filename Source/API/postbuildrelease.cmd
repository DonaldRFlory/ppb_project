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

copy /y ..\..\Output\API\release\PPBAPI.DLL ..\..\Output\TestApp\Release

echo.
exit /B 0
