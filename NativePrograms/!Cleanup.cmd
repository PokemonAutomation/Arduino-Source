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

::del Libraries\*.d
::del Libraries\*.o
::del Programs\*.d
::del Programs\*.o

del obj\*.d
del obj\*.o
del obj\build-*

del LUFA\Drivers\USB\Core\*.d
del LUFA\Drivers\USB\Core\*.o
del LUFA\Drivers\USB\Core\AVR8\*.d
del LUFA\Drivers\USB\Core\AVR8\*.o
del LUFA\Drivers\USB\Class\Common\*.d
del LUFA\Drivers\USB\Class\Common\*.o

::pause
