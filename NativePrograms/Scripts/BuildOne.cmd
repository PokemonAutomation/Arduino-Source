::echo off

set board=%1
set program=%2

if [%board%] == [] (
    set /p board="Board Type: "
)
if [%program%] == [] (
    set /p program="Program Name: "
)

del "%program%.hex"
make BOARD_TYPE="%board%" TARGET="%program%"
avr-size --mcu=%MCU% -C %program%.elf

::if %errorlevel% NEQ 0 (
::   echo Build Failed. Error %errorlevel%
::   del "%program%.hex"
::   pause
::)

del "%program%.map"
del "%program%.eep"
del "%program%.bin"
del "%program%.lss"
del "%program%.sym"
del "%program%.tmp"

::pause