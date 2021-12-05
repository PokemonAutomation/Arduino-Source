::  Clean the build. Delete all build artifacts.

cd %~dp0

del *.log
del *.d
del *.elf
del *.hex
del *.map
del *.o
del *.eep
del *.bin
del *.lss
del *.sym
del *.tmp

del obj\*.d
del obj\*.o
del obj\build-*

::pause
