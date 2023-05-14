@echo OFF
rem   Prebuild file for CSTestApp
rem For reasons known only to them, Microsoft makes the working directory during execution of this batch file
rem be the output directory of the project.
rem cd ..\..\..\Source\TestApp

rem ..\tools\internal\ldfutil -HLWPart1.cs -FLINK  -D.\1343API.dll -EPPHC -x6 -i..\Common\mainll.h  -m..\..\output\TestAPP\LinkWrap.cs
..\Tools\ldfutil -HLWPart1.cs -FLINK  -D.\PPBAPI.dll  -x11 -i..\Common\mainll.h  -m..\..\output\TestAPP\LinkWrap.cs >1.txt
..\Tools\ldfutil -HMDPart1.cs -x7 -i..\Common\menufunctions.h  -m..\..\output\TestAPP\MenuDelegates.cs >2.txt
copy /y TestMenu.txt ..\..\output\TestApp\Debug\
copy /y TestMenu.txt ..\..\output\TestApp\Release\
copy /y ..\..\output\TestApp\LinkWrap.cs .
copy /y ..\..\output\TestApp\MenuDelegates.cs .
EXIT  0
