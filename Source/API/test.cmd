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
echo .
echo Start Building Link list files
..\Tools\ldfutil -x0 -i..\Common\mainll.h -p..\..\output\API\dlllink.h -m..\..\output\API\dlllink.cpp -s..\..\output\API\junk.out

echo End of Building Link list files
echo.
exit /B 0
