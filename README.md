OpenPuush
=========

OpenPuush is meant to be a clone of the Puush application (http://puu.sh). The reasoning behind this project is that
Puush is an extremely useful utility that I wish was able on more platforms than just on Windows and OSX.

OpenPuush is open-source under the terms of the GPLv3 license (for icons, etc see graphics/LICENSE).

Technical Details
---------
OpenPuush used Qt and Qxt for its gui and core functionality, thus it is as portable as Qxt + Qt are. In practice though,
I'm intending to make this portable to the three major operating systems (Linux with X11, Windows, and OSX).

Compiling
---------
To build OpenPuush, all you need to do is cd into the directory with the code and run `qmake -project`. After this is done,
edit the generated `.pro` file and add the lines:

```
CONFIG += qxt
QXT += core gui
QT += core gui network qxt
```

When this is done, run `qmake` (possibly with whatever `-spec` option you want) and then compile with `make` or whatever.