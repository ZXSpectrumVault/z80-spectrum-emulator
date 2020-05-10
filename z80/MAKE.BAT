rem
rem To build the emulator from the source files, you need:
rem
rem  - a86 version 3.22   (version 3.14 is no good)
rem  - Turbo C v2.0 or Borland C v3.1   (other versions no problem)
rem  - Microsoft Linker version 2.40 or Turbo Link version 5.1
rem    (Turbo Link versions 1 to 3 all have difficulty linking the emulator)
rem
rem The object file INTROSCR.OBJ is supplied.
rem
a86 z80.8 z80.obj
a86 misc.8 misc.obj
a86 mdrv.8 mdrv.obj
a86 video.8 video.obj
a86 cif.8 cif.obj
a86 vidtab.8 vidtab.obj
a86 tables.8 tables.obj
a86 emul.8 ddfd.8 dd.8 dd.8 cb.8 ed.8 emul.obj
bcc -O -d -ml -c -oxtra.obj xtra.c
tlink xtra z80 misc mdrv video cif vidtab tables introscr emul,z80,z80;
z80
