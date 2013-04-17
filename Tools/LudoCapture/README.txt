LudoCapture
===========

Version 1.0.0
2011-09-21


This tool was created to give us motion capture files as quickly as possible from a Kinect motion sensor.
It is somewhat rough, and is by no means a full-fledged utility (which would probably embedded better).

Prerequisites
-------------

You need to have OpenNI compiled and functional.
I used the unstable branch of the official Git repository.
More info here: http://openni.org/

Here's pretty much what I did:
 > git clone https://github.com/OpenNI/OpenNI
 > cd OpenNI
 > git checkout --track -b unstable origin/unstable
 > ( cd Platform/Linux-x86/Build && make )

To run a sample test (the one I used as basis for LudoCapture):
 > ( cd Platform/Linux-x86/Bin/Release && ./Sample-NiUserTracker )


Also of mention are the following libraries:

OpenKinect (a.k.a. libfreenect)
http://openkinect.org/
http://openkinect.org/wiki/Getting_Started
  > git clone https://github.com/OpenKinect/libfreenect
  > cd libfreenect
  > git checkout --track -b unstable origin/unstable
  > mkdir build
  > cd build
  > cmake ..
  > make

libusb
http://www.libusb.org/


For both (libfreenect and libusb) with Homebrew (BREW_HOME=/usr/local by default):
  > cd ${BREW_HOME}/Library/Formula
  > curl --insecure -O "https://github.com/OpenKinect/libfreenect/raw/master/platform/osx/homebrew/libfreenect.rb"
  > curl --insecure -O "https://github.com/OpenKinect/libfreenect/raw/master/platform/osx/homebrew/libusb-freenect.rb"
  > brew install libfreenect


Compiling
---------

Once all of the prerequisites are built, all you need to do is set an environment variable
pointing to your OpenNI directory, then build:
  > export OPENNI_DIR=/path/to/your/OpenNI
  > make

The binary will under Bin/Release/LudoTracker.

Here are the typical keys:
    q   - Quit the application (saving outstanding captures, if any).
  <spc> - Start/stop a capture (must wait for the skeleton to appear).
  <esc> - Abort a capture (this actually stops it, then deletes the generated file).

The generated files are of the form:
  YYYYMMDD_hhmmss_p.ranim
where:
  YYYYMMDD represents the current date.
    hhmmss represents the current time.
         p represents the current actor ID (can theoretically capture more than one person at a time).

The format is pretty much self-explanatory: it's a Lua table with multiple bone positions (not orientations)
in "absolute value" (relative to the library's origin), which look like millimeters.

