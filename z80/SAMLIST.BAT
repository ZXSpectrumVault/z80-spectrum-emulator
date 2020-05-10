@echo off
echo:
echo SAMLIST.BAT - Converts the SamRam source files to plain ASCII.
echo:
if (%1)==() goto nopath
set pth=%1
if exist %pth%z80.exe goto testfiles
set pth=%1\
if not exist %pth%z80.exe goto notfound
:testfiles
if not exist samsrc.tap goto notfound
if not exist samlist.z80 goto notfound
:path
%pth%z80 -ts. -tisamsrc -wsamsrc.txt samlist
if not exist samsrc.txt goto error
echo File SAMSRC.TXT created successfully.
goto end
:error
echo File SAMSRC.TXT not created; the file Z80.EXE probably couldn't be located.
echo Run the batchfile again, and make sure you specify the path to the emulator.
goto end
:nopath
echo:
echo Usage: SAMLIST path-to-z80.exe
echo Example: SAMLIST c:\spectrum
echo:
echo This batchfile will run the SAMLIST.Z80 program, which converts the six
echo GENS3 source code files in SAMSRC.TAP into a plain ASCII file SAMSRC.TXT
echo and adds a very few comments.
goto end
:notfound
echo:
echo Either of the following three files were not found:
echo  SAMSRC.TAP        (in the current directory)
echo  SAMLIST.Z80       (in the current directory)
echo  %pth%Z80.EXE
:end
echo:
set pth=
