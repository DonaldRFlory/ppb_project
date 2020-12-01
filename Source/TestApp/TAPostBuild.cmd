rem For reason known only to them, Microsoft makes the working directory during execution of this batch file
rem be the output directory of the project.
cd ..\..\..\Source\CSTestApp

copy /y TestMenu.txt ..\..\output\TestApp\Debug\	/y
copy /y TestMenu.txt ..\..\output\TestApp\Release\ /y
exit 0
