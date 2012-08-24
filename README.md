OpenPuush
=========

OpenPuush is meant to be a clone of the Puush application (http://puu.sh). The reasoning behind this project is that
Puush is an extremely useful utility that I wish was able on more platforms than just on Windows and OSX.

OpenPuush is open-source under the terms of the GPLv3 license (for icons, etc see graphics/LICENSE).

Relationship to Puush
--------
OpenPuush is in no way derivative of Puush in the sense that no reverse engineering (eg disassembling, etc) was done to
clone functionality. All the features included in OpenPuush that are one-to-one with features of Puush are derived solely
by inspecting the behavior of the application from a user's point-of-view as well as plenty of common sense.

Similarly, the icons for OpenPuush represent the functionality of the application as well as its current state of
operation.

Technical Details
---------
OpenPuush uses Qt and Qxt for its gui and core functionality, thus it is as portable as Qxt + Qt are. In practice though,
I'm intending to make this portable to the three major operating systems (Linux with X11, Windows, and OSX).

How it works
---------
OpenPuush works by uploading screenshots/files/plaintext to your Dropbox "public" folder and linking to them (in the future,
the urls will be shortened).

Compiling
---------
To build OpenPuush, all you need to do is cd into the directory with the code and run `qmake` (possibly with whatever
`-spec` option you want) and then compile with `make` or whatever.