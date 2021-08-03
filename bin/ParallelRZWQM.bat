@echo off
set "numDir=0"
::setlocal EnableDelayedExpansion
:: error handle
if [%1] == [] GOTO :error
if [%2] == [] GOTO :error
:: setup for the bat file
set "exeDir=%~dp0"
set "directories=%1"
set "numCores=%2"
set /A "completedDirs=1"
set "fileLock=%temp%\filewait%random%.lock

:: get a list of all the scenarios in an array
for /F "delims=" %%f in (!directories!) do (
    set /A numDir=numDir+1
    set "dirList[!numDir!]=%%f"   
)

if NOT !numDir!==0 goto :exeloop
:workloop
if NOT !completedDirs! gtr !numDir! goto :exeloop

goto :exit


:exeloop
set "currDir=!cd!"
:: calculate number of directories run so far:
set /a "offset=!completedDirs!+!numCores!-1"
:: if new offset > # directories, calculate "offset - overage"
if  !offset! gtr !numDir! (
    set /a "overage=!offset!-!numDir!"
    set /a "offset=offset-!overage!"
)
:: start the science in the directories
for /L %%i in (%completedDirs%,1,%offset%) do (
    cd !dirList[%%i]!
    start "" 9>"%fileLock%%%i" !exeDir!rzwqmrelease.exe 
    cd !currDir!
)
:: wait for the science to complete in all the directories:
:Wait
::1>nul 2>nul ping /n2 ::1
:: pause for 5 seconds
timeout /T 5 /nobreak 1>nul 2>nul
:: check the files, if any are still locked, wait some more
for %%F in ("%fileLock%*") do (
 (call ) 9>"%%F" || goto :Wait
) 2>nul
:: delete all the lock files
del "%fileLock%*" 
:: update the # completed directories
set /a completedDirs=completedDirs+!numCores!
:: see if more work to be done
goto :workloop

:: error if either %1 or %1 are empty
:error
echo/ "Must provide two arguments"
echo/ "Usage: ParallelRZWQM.bat <Path to pestargs.txt that lists all the scenarios to run> <num cores to use>"
goto :exit


:: finish bat
:exit
::endlocal
@echo on
::exit