:
:This make batchfile assembles the shareware version of the emulator.
:
:To assemble the .8 files, you need A86 version 3.22.  Older versions
:don't work properly.
:
:To compile the XTRA.C file I use Borland C++ version 4.02, but any other
:compiler should work.  Except for the switches -ml that selects the large
:memory model, and -oxtra.obj that specifies the output object file, they
:are insignificant.  The compiler warns about three 'possible incorrect
:assignments'.  Those are perfectly correct assignments, ignore the warning.
:
:I use Turbo Link version 5.1 and 6.10.  Previous versions sometimes gave
:strange errors, and also the Microsoft linker sometimes gave up for dubious
:reasons.  They may work though, but if you experience problems, use Tlink
:version 5.1 or 6.10.
:
a86 emul.8 xdcb.8 xdcb.8 dd.8 dd.8 cb.8 ed.8 emul.obj
a86 video.8 video.obj
a86 cif.8 cif.obj
a86 z80p1.8 z80p2.8 mdrvp1.8 mdrvp2.8 z80.obj
a86 miscp1.8 miscp2.8 miscp3.8 misc.obj
a86 vidtab.8 vidtab.obj
a86 tables.8 tables.obj
a86 voc.8 voc.obj
a86 discp1.8 discp2.8 disc.obj
bcc -1 -O -d -ml -c -oxtra.obj xtra.c
tlink %1 xtra disc z80 misc voc video cif vidtab tables emul introscr,z80,z80;
z80
del emul.obj
del video.obj
del cif.obj
del z80.obj
del misc.obj
del vidtab.obj
del tables.obj
del voc.obj
del disc.obj
del xtra.obj
del z80.map
del *.sym
