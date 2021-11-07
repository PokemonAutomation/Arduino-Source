
set board=%1

if [%board%] == [] (
    set /p board="Board Type: "
)

if [%board%] == [ArduinoUnoR3] (
    set MCU=atmega16u2
)
if [%board%] == [ArduinoLeonardo] (
    set MCU=atmega32u4
)
if [%board%] == [ProMicro] (
    set MCU=atmega32u4
)
if [%board%] == [Teensy2] (
    set MCU=atmega32u4
)
if [%board%] == [TeensyPP2] (
    set MCU=at90usb1286
)

:: Just in case the PATH is messed up, but is installed in the usual place.
set PATH=%PATH%C:\WinAVR-20100110\bin;
set PATH=%PATH%C:\WinAVR-20100110\utils\bin;




echo Checking make command...
echo.

make -v

echo.
echo.
if %errorlevel% NEQ 0 (
    echo make not found. Please install WinAVR.
    echo.
    pause
    exit
)


:: Check MCU Compatibility
if exist "obj\*.o" (
    echo Existing build found. Checking MCU compatibility.
    if not exist obj\build-%MCU% (
        echo Incompatible MCU, cleaning...
        @call !Cleanup.cmd >NUL 2>&1
    ) else (
        echo Existing build is compatible.
    )
) else (
    echo No build found, continuing...
)
copy NUL obj\build-%MCU% > NUL


echo.
echo Starting build... This may take a while if your computer is slow.
echo.

if not exist obj\ (
    mkdir obj\
)


::  Build one first to build all the shared libraries.
::  Only then can we run the rest in parallel.

for /F "tokens=*" %%p in (ProgramList.txt) do (
    set first=%%p
    if exist %%p.c (
        @call %~dp0BuildOne.cmd %board% %%p > %%p.log  2>&1
        goto :done
    )
)
:done

for /F "tokens=*" %%p in (ProgramList.txt) do (
    if exist %%p.c (
        if [%%p] NEQ [%first%] (
            echo > %%p.tmp
            START /B %~dp0BuildOne.cmd %board% %%p > %%p.log 2>&1
        )
    )
)

::@echo on

:loop
for /F "tokens=*" %%p in (ProgramList.txt) do (
    if exist %%p.tmp goto :loop
)

for /F "tokens=*" %%p in (ProgramList.txt) do (
    if exist %%p.c (
        if not exist %%p.hex (
            echo.
            echo **Failed to build %%p.hex**.
            echo Please see %%p.log for details.
        )
    )
)

echo.
echo All Done! Double-check that the .hex have been made or updated.
::echo Errors will logged in the respective .log file.
::echo.

echo You can now close this window.


