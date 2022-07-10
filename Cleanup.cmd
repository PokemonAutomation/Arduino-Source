
rd /s /q "..\build-SerialPrograms*"

::cd %~dp0\ClientProject
::@call Cleanup.cmd

cd %~dp0\NativePrograms
@call Cleanup.cmd

cd %~dp0\HexGenerator
@call Cleanup.cmd

cd %~dp0\HexGeneratorLauncher
@call Cleanup.cmd

cd %~dp0\SerialPrograms
@call Cleanup.cmd


