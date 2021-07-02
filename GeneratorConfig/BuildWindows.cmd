::@echo off

set board=%1
set category=%2
set program=%3

cd /D "%~dp0"
cd "../NativePrograms/%category%"


if [%board%] == [ArduinoUnoR3] (
    set MCU=atmega16u2
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


del "%program%.hex"
del "../%program%.hex"


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
::    pause
    exit
)

make BOARD_TYPE="%board%" TARGET="%program%"

del "%program%.map"
del "%program%.eep"
del "%program%.bin"
del "%program%.lss"
del "%program%.sym"
del "%program%.tmp"

move "%program%.hex" "../../%program%-%board%.hex"

::pause