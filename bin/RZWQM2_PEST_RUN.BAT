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
for %%f in (pest\*.in) do (
	"!exeDir!PEST\par2par" "%%f"
)
:: execute the science
"!exeDir!rzwqmrelease.exe"
:: CB2
endlocal
:: done
@echo on

:: CB1
:: get from the file the two arguments being passed.
:: RM obsolete, the interface does not execute in  way to make this section
:: useful. 
::set count=0
::FOR /F "delims=" %%a in (%1) do (
  ::  set /A count+= 1
    ::set "var!count!=%%a"
::)
::set "var=!var:*:=!"
:: CD To the project directory
::cd "!var1!"




:: CB2
:: RM obsolete, GUI Does not execute in this fashion.
:: case of PEST :
::		CD back to same directory.
:: case of BEOPEST:
::		We had run in a slave directory, CD Back to the parent
::		so we can copy results.
::cd "!var2!"