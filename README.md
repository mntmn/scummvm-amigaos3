# ScummVM 1.8.1 AmigaOS 3 Port

I obtained this code from TuKo (Vampire Team). The original patches for AmigaOS 3.x support are from NovaCoder and updates have been made by relnev. My goal is to bring development of this fork to the public on GitHub and contribute some clean-ups and fixes and make it work with my graphics card MNT VA2000.

## Readme from Aminet Binaries

The readme on aminet contains some hints about compilation:

These are binaries of ScummVM 1.8.1 with RTG AmigaOS 3 support, using
the work done by NovaCoder for ScummVM 1.5.0 and continued by relnev.

http://aminet.net/package/game/misc/ScummVM_RTG

These binaries were built using the m68k-amigaos netsurf toolchain.

The scummvm 1.8.1 configure flags used were:

--host=m68k-unknown-amigaos --disable-all-engines 
--enable-engine=sword1,tinsel,agos,kyra,dreamweb,sky,
scumm,queen,lure,agi,sci,scumm-7-8,sci32 --disable-mt32emu 
--enable-release --disable-hq-scalers

And the CXXFLAGS used were:
-Os -ffast-math -fsingle-precision-constant -fweb -frename-registers 
-finline-functions -funit-at-a-time -fomit-frame-pointer 
-fno-exceptions -fno-rtti -DNDEBUG
