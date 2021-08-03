@echo off
setlocal EnableDelayedExpansion
:: RZWQM2 PEST BAT FILE
:: VERSION 1.4
:: UPDATED 09.23.2015
::
:: Main idea: Run par2par on the .in files in the pest directory we are executing in.
::          Then run RZWQMrelease 
::
:: When this script is launched, it is launched from the bin dir.
:: store this dir to execute par2par and the science.
set "exeDir=%~dp0"
:: CB1
:: run the par2par.exe on the par2par and p2pcul .in files.
:: this loads the values into the science input files.
for %%f in (.\pest\*.in) do (
	"!exeDir!PEST\par2par" "%%f"
)
set newpath=%1
set newpath=!newpath:"=!
!exeDir!ParallelRZWQM.bat !newpath! 4

::set newpath=%1
::set newpath=!newpath:"=!
::for /F "delims=" %%f in (!newpath!) do (
::    cd %%f
::    !exedir!RZWQMrelease.exe
::    cd ..
::)

:: CB2
endlocal
:: done
@echo on
