@echo off
echo off
if (%1)==(unz) goto %1
if exist *.8 goto err6
cls
if (%1)==() goto hlp
if not exist install.bat goto err0
if not exist pkunzip.exe goto err0
if not (%2)==() goto err1
md %1 >nul
cls
echo Installing Z80 version 3.04 to your harddisk: %1\
set x=-xz80.exe
set d=%1
set zd=
:
:Additional archive files must be put AFTER emulsrc.zip and BEFORE z80-304.zip
:
%0 unz emulsrc.zip progs.zip utilsrc.zip utils.zip z80-304.zip z80.zip install.msg
:unz
set cur=%2
set l=%3
:loop
if (%4)==() goto :endl
set l=%l% %4
shift
goto loop
:endl
set testing123=%3%3%3
if not (%testing123%)==(%3%3%3) goto erre
set testing123=
:ff
if exist %cur% goto :good
echo Need file '%cur%' - please insert other disk.
pause
goto ff
:good
if (%l%)==() goto :last
%zd%pkunzip -d -xregister.doc %x% %cur% %d%\
if errorlevel 2 goto err2
if not (%cur%)==(z80-304.zip) goto no304
copy %cur% %d%%cur%
if not exist %d%%cur% goto err4
set x=
:no304
if not (%zd%)==() %zd%install unz %l%
if not exist %d%emulsrc\ed.8 set d=%d%\
set zd=%d%emulsrc\
copy pkunzip.exe %zd%pkunzip.exe
copy install.bat %zd%install.bat
%zd%install unz %l%
:last
echo:>>%d%z80.ini
echo %% The following lines have been added by the INSTALL.BAT program:>>%d%z80.ini
echo:>>%d%z80.ini
echo -xp %d%z80files>>%d%z80.ini
echo -ts %d%tapfiles>>%d%z80.ini
copy %cur% %d%readme
if not exist %d%readme goto err5
del %zd%pkunzip.exe
cls
echo Install successful!
type install.msg
goto end
:hlp
echo:
echo This batch program installs your registered copy of 'Z80' version 3.04
echo onto your hard disk.  You can run it from the original floppy disk or
echo from a hard disk directory.
echo:
echo Usage:   INSTALL drv:\directory
echo Example: INSTALL c:\z80
echo:
echo You will need about 2.5 Mb of free space on your hard disk.
goto end
:erre
set testing123=
echo INSTALL: Not enough environment space!  Type COMMAND /E:1000 and try again.
echo If this doesn't work, change the SHELL= statement in the CONFIG.SYS
echo appropriately (usually adding /E:1000 to the parameter list works), reboot,
echo and try again.
goto end
:err0
echo INSTALL: Installation files (PKUNZIP.EXE, INSTALL.BAT) must be in current
echo directory - install aborted.
goto end
:err1
echo INSTALL: Error - need exactly one argument.  Type 'INSTALL' for help.
goto end
:err2
echo INSTALL: Pkunzip returns with error - install aborted.
goto end
:err4
echo INSTALL: Failed to copy Z80-304.ZIP to %d%Z80-304.ZIP - install aborted.
goto end
:err5
echo INSTALL: Failed to copy INSTALL.MSG to %d%README - install aborted.
goto end
:err6
echo You can safely delete me (install.bat).  I am useless now.
:end
set x=
set d=
set l=
set zd=
set cur=
