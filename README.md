# Z80 Spectrum Emulator for DOS

Gerton Lunter's Z80 Spectrum Emulator for DOS was the premier ZX Spectrum emulator in the 1990s - heralding in the Z80 snapshot format as well as many other great features such as real tape support by way of a parallel adapter, 128K, Multiface, TAP and much more.

The package was shareware and registered users were given a copy of the source which could be used as a reference for other emulators but not used as-is.

I recently came across the ZIP of my registered copy and realized the source for this historically important emulator was not online so reached out to Gerton to see if he would grant permission for it to be hosted online, so here it is!

As the emulator was written for DOS (I don't have the source for the later Windows version) there's no easy way to build or run this outside of tools like DOSbox or VMware along with the necessary dependencies which according to make.bat are:

- a86 version 3.22    (version 3.14 is no good)
- Borland C++ v4.02   (but any other compiler should work)
- Turbo Link versions 5.1 and 6.10 

Gerton has kindly also allowed the code to be relicenced under the MIT license. I have left the older more restrictive header licenses in the individual files for historical reasons.