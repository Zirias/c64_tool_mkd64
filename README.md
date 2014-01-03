mkd64
=====

mkd64 is a modular tool for creating .D64 images (disk images for a C64 floppy drive). The core program is responsible for writing tracks and sectors, while loadable modules do everything else, for example writing a directory.

usage
-----

The mkd64 command line works like a minimal scripting language: It starts with global options for loading modules and specifying things like output files or the disk name. The first '-f' options switches to file mode, every file starts with '-f' and an optional name to load from your harddrive and ends with '-w' for writing the file to the disk image.

mkd64 will never complain about unknown options but silently ignore them. This is because every option gets also fed to every loaded module, so modules can extend the available options.

features
--------

- can create images without directory
- module 'cbmdos' writes directory in original format (sector interleave 4)
- module API for other directory formats
- interleave can be specified per file
- can create a 'map file' containing files and their start track/sector
- start track and sector of a file can be specified explicitly
- for lots of files on the image, options may be read from a file

build
-----

In short: just type 'make'. You will need GNU make and the GCC C-compiler for this to work. For windows, use MinGW (and DON'T use MSYS, because the Makefile assumes the shell to be CMD.EXE).

install
-------

Put mkd64 (or mkd64.exe) and all modules (*.so or *.dll) in the same directory. That's it.

binaries
--------

I create win32 binaries for every tagged release in-tree, see directory win32bin. There's also a directory win64bin, but no guarantee this will be updated every time. In fact, right now mkd64 doesn't take any advantage of 64bit on windows, it just gets bigger, so there's not much of a point, as long as WoW64 clobbers any windows installation anyways ;)

help
----

type 'mkd64 -h' to get an overview of all core options.
type 'mkd64 -h [module]' for help with module [module].

modules
=======

cbmdos
------

This is the only module for now. It reserves track 18 and uses it to write the BAM and directory entries like an original floppy dos. Use 'mkd64 -h cbmdos' for details.

module API
==========

See modapi.txt. Documentation is still work in progress.

license
=======

&copy; Felix Palmen <felix@palmen-it.de>

This code is free software and may be used or modified for any purpose, as long as the following conditions are met:

- All source or binary distributions (read: any other way to get mkd64 than the original github site) MUST include this license info and the above copyright notice.
- Distributions of modified versions MUST include a notice about the modifications, or, better yet, use a different name and refer to the original mkd64.

development
===========

Patches and comments are welcome. For the future, the following features are planned:

- support 40track formats
- custom directory formats in new modules
