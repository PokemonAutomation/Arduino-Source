@echo off

cd /D "%~dp0"

if not exist Scripts\ (
    echo.
    echo Please unzip the folder before you can use it.
    echo.
    pause
    exit
)

@call Scripts/BuildAll.cmd ArduinoUnoR3


