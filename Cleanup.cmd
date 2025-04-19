
rd /s /q "..\build-SerialPrograms*"

del 3rdPartyBinaries\opencv_world460d.dll

cd %~dp0\SerialPrograms
@call Cleanup.cmd



rd /s /q "..\build-HexGenerator-Desktop_Qt_5_12_12_MinGW_32_bit-Debug"
rd /s /q "..\build-HexGenerator-Desktop_Qt_5_12_12_MinGW_32_bit-Release"
rd /s /q "..\build-SerialPrograms-Desktop_Qt_5_12_12_MSVC2017_64bit-Release"
rd /s /q "..\build-SerialPrograms-Desktop_Qt_5_12_12_MSVC2017_64bit-RelWithDebInfo"
rd /s /q "..\build-SerialPrograms-Desktop_Qt_6_3_1_MSVC2019_64bit-Debug"
rd /s /q "..\build-SerialPrograms-Desktop_Qt_6_3_1_MSVC2019_64bit-RelWithDebInfo"
rd /s /q "..\build-SerialPrograms-Desktop_Qt_6_3_2_MSVC2019_64bit-Debug"
rd /s /q "..\build-SerialPrograms-Desktop_Qt_6_3_2_MSVC2019_64bit-RelWithDebInfo"
rd /s /q "..\build-SerialPrograms-Desktop_Qt_6_4_0_MSVC2019_64bit-Debug"
rd /s /q "..\build-SerialPrograms-Desktop_Qt_6_4_0_MSVC2019_64bit-RelWithDebInfo"
rd /s /q "..\build-SerialPrograms-Desktop_Qt_6_5_2_MSVC2019_64bit-Debug"
rd /s /q "..\build-SerialPrograms-Desktop_Qt_6_5_2_MSVC2019_64bit-RelWithDebInfo"
rd /s /q "..\build-SerialPrograms-Desktop_Qt_6_5_3_MSVC2019_64bit-Debug"
rd /s /q "..\build-SerialPrograms-Desktop_Qt_6_5_3_MSVC2019_64bit-RelWithDebInfo"
