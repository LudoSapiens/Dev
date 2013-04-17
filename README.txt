These are the core libraries and tools used by Ludo Sapiens.

It is released under the MIT license (see LICENSE.txt), and
contains some contributions by other folks (see AUTHORS.txt).


============
ORGANIZATION
============

The main directories are:
* Data (assets, such as script, texture, geometry, animation, etc.)
* Projects (actual source code, see below)
* Thirdparty (libraries we are using)
* Tools (IDE project files, some scripts, etc.)

The Projects directory contain the following libraries:
* Base (abstract data types, input/output, multi-threading, etc.)
* CGMath (vectors, matrices, ray-casting, etc.)
* Gfx (wrapper for OGL/GLES/D3D)
* Snd (sound abstraction library, supports OpenAL)
* Fusion (2D UI with event, cross-platform OSX/IOS/WIN32/ANDROID)
* MotionBullet (physics library, using Bullet)
* Motion (our own physics library; deprecated)
* Plasma (3D world, geometry, skeletal animation)

Here are the dependencies:

+-+-+-+-+-+--> Base
| | | | | |
+-+-+-+-+-o--> CGMath
| | | | |
+-|-+-|-o----> Gfx
| | | |
+-|-+-o------> Snd
| | |
+-|-o--------> Fusion
| |
+-o----------> MotionBullet
|
o------------> Plasma

'o' depends on '+', e.g. MotionBullet depends on Base and CGMath.


=========
COMPILING
=========

To compile, you first need to build the third-party libraries.

Base requires ZLib (for GZippedFileDevice).
Snd can be built with LibOGG and LibVorbis.
Fusion requires Lua (for scripts, such as UI, worlds, geometry).
MotionBullet requires Bullet.

We have created a Makefile to compile all of those libraries.
To compile, the following should work:
  > cd Thirdparty
  > make all
Various commands are available in the Makefile.  To see, call:
  > make help
or look at the Makefile itself for even more details.

This should build all of the required libraries, and store them
inside ${DEV_ROOT}/Thirdparty/platform.

Once you have those compiled, you can simply open up either the
Xcode or Visual Studio project files located under:
  Tools/IDE/Xcode (for OSX/iOS)
  Tools/IDE/Visual Studio/2011 (for Windows)
and select PlasmaApp, and compile normally.

Note that thirdparty libraries are not properly specified as
dependencies under Xcode, so recompiling them won't force a relink.

Also note that the Visual Studio 2010 project is direly outdated
(sorry).


Notes about targets
-------------------

Every target ending with Test (BaseTest, FusionTest, etc.) are meant
to be unit tests, run on the command line.

Every target ending in App (FusionApp, PlasmaApp) are meant to be standalone
GUI applications.


=======
RUNNING
=======

Each application relies on a Lua script to create the GUI.
As arguments to the executable, you need to pass the absolute/relative paths
of the Data subdirectories you want to use.  If you specify a script as an
argument, it will get executed; this is how we can run multiple applications
using the same executable.

For example, if you set the following command arguments in Visual Studio:
  common tool/plasma
it will automatically search up to find a 'Data' directory, then add its
'common' subdirectory (to that we can find 'theme/default'), and then try
to execute 'Data/tool/plasma/main.ui', which is the first 'main.ui' file
it will find.  If you wanted to, you could have appended 'test/fusion' to
run a simple GUI test, or append to that another 'script/app/font' to run
another application related to fonts.

Notes about iOS
---------------

Because of the way iOS applications are sandboxed, we need to bundle
the asset files along with the application bundle.

For that, we have created a manifest file to list all of the files
to bundle with the application.  See under Data/manifest for examples.

The format is rather simple: every file or directory listed will be included
in a subdirectory inside a Data directory.  You can even exclude some
files by prefixing them with a minus, or rename the destination, e.g.:
  dirA
  dirB
  -dirB/fileB1
  dirD -> dirC
  dirD/fileD1 -> dirC/fileC1
  -dirD/fileD2

The script responsible for this work is Tools/Scripts/LudoSync.py.
It is called using a CopyData build phase to every Xcode application target.
It can also be used to prepare a release package for other platforms,
or to rename assets, if necessary.

Also, due to a limitation by Apple's own bundle-creating script, removing
an asset might still keep it inside the final bundle, because Apple doesn't
remove outdated files sitting in the bundle directory.
For that reason, it is recommended to get rid of the bundle's directory
(such as by a clean) when generating final executables, as well as verifying
the bundle's content when deploying.

Arguments order
---------------

The arguments listed at invocation have increasing priority, meaning
that later arguments override previous ones, e.g.:
  > PlasmaApp.exe common test/regression test/fusion
will run Data/test/fusion/main.ui, and assets under Data/test/regression
will have precedence over the ones in Data/common.  It also means that
adding script/app/event, such as:
  > PlasmaApp.exe common test/regression test/fusion script/app/event
will now launch Data/test/fusion/script/app/event.ui instead.
