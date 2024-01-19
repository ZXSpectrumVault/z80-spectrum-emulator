@echo off
echo:
echo TAP2TAPE.BAT - Utility to save .TAP files back to tape - (c) 1994 G.A. Lunter
echo                This program may not be distributed.
echo                It is part of the registered Spectrum emulator package.
echo:
set tf=%1
set bopt=-b1
if (%tf%)==() goto usage
:scanpath
set pth=%2
if exist %pth%z80.exe goto testfile
if (%pth%)==() goto noemul
set pth=%2\
if exist %pth%z80.exe goto testfile
if (%2)==(1) goto lptx
if (%2)==(2) goto lptx
if (%2)==(3) goto lptx
if (%2)==(4) goto lptx
echo Emulator not found in path '%2'
goto usage
:noemul
echo Emulator not found in current directory.
goto usage
:lptx
set bopt=-b%2
shift
goto scanpath
:testfile
if exist %tf% goto tapfound
if exist %tf%.tap goto tapfound
echo File '%tf%' (or '%tf%.TAP') not found.
echo:
goto usage
:tapfound
if not exist %pth%tap2tape.z80 goto notfound
echo %pth%z80 %bopt% -00 -tw -ti %tf% %pth%tap2tape
%pth%z80 %bopt% -00 -tw -ti %tf% %pth%tap2tape
goto end
:notfound
echo File '%pth%TAP2TAPE.Z80' not found.
:usage
echo:
echo Usage:   TAP2TAPE tapefile  [LPT port]  [path to z80.exe]
echo:
echo Example: TAP2TAPE program.tap 2 c:\spectrum
echo:
echo ■ The path to the Z80.EXE program and the LPT port used for saving may be
echo   omitted; the default values are the current directory and LPT1 respectively.
echo   They must be specified in the above order.
echo ■ You can abort the program by holding the Space key down.
:end
echo:
set pth=
set tf=
set bopt=
