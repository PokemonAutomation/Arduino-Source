
set board=%1

if [%board%] == [] (
    set /p board="Board Type: "
)

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

:: Just in case the PATH is messed up, but is installed in the usual place.
set PATH=%PATH%C:\WinAVR-20100110\bin;
set PATH=%PATH%C:\WinAVR-20100110\utils\bin;


set programs=

set programs=%programs%;TurboA
set programs=%programs%;MultiGameFossil
set programs=%programs%;MassRelease
set programs=%programs%;SurpriseTrade
set programs=%programs%;TradeBot
set programs=%programs%;ClothingBuyer
set programs=%programs%;BallThrower

set programs=%programs%;DateSpam-WattFarmer
set programs=%programs%;DateSpam-BerryFarmer
set programs=%programs%;DateSpam-LotoFarmer
set programs=%programs%;DateSpam-StowOnSideFarmer
set programs=%programs%;DateSpam-DailyHighlightFarmer

set programs=%programs%;ShinyHunt-Regi
set programs=%programs%;ShinyHunt-SwordsOfJustice
set programs=%programs%;ShinyHuntUnattended-Regi
set programs=%programs%;ShinyHuntUnattended-SwordsOfJustice
set programs=%programs%;ShinyHuntUnattended-StrongSpawn
set programs=%programs%;ShinyHuntUnattended-Regigigas
set programs=%programs%;ShinyHuntUnattended-Regigigas2
set programs=%programs%;ShinyHuntUnattended-IoATrade

set programs=%programs%;BeamReset
set programs=%programs%;EventBeamFinder
set programs=%programs%;DaySkipperJPN
set programs=%programs%;DaySkipperEU
set programs=%programs%;DaySkipperUS
set programs=%programs%;DaySkipperJPN-7.8k

set programs=%programs%;DenRoller
set programs=%programs%;AutoHost-Rolling
set programs=%programs%;AutoHost-MultiGame
set programs=%programs%;FriendDelete

set programs=%programs%;EggFetcher2
set programs=%programs%;EggHatcher
set programs=%programs%;EggCombined2
set programs=%programs%;EggSuperCombined2

set programs=%programs%;FastCodeEntry
set programs=%programs%;GodEggItemDupe
set programs=%programs%;GodEggDuplication

set programs=%programs%;PABotBase
set programs=%programs%;Sandbox
set programs=%programs%;CustomProgram



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
echo Starting build... This make take a while if your computer is slow.
echo.

if not exist obj\ (
    mkdir obj\
)


::  Build one first to build all the shared libraries.
::  Only then can we run the rest in parallel.

for %%p in (%programs%) do (
    set first=%%p
    @call Scripts\BuildOne.cmd %board% %%p > %%p.log  2>&1
    goto :done
)
:done

for %%p in (%programs%) do (
    if exist %%p.c (
        if [%%p] NEQ [%first%] (
            echo > %%p.tmp
            START /B Scripts\BuildOne.cmd %board% %%p > %%p.log 2>&1
        )
    )
)

::@echo on

:loop
for %%p in (%programs%) do (
    if exist %%p.tmp goto :loop
)

for %%p in (%programs%) do (
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


