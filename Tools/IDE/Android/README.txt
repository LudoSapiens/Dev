--------
OVERVIEW
--------

The emulator is quite slow (read: *VERY* slow).
A trick is to start one and save its snapshot when quitting (options shows in the GUI when starting it up).
Then, don't install anything just yet; just clean things up nicely, and close the window.
Your state will save (take a minute or two) and you can then start it with the snapshot.
If you stop saving the state when quitting (in the android tool's GUI), you'll always start fresh (maybe it's something you prefer).


Projects need an AndroidManifest.xml file, and that might be pretty much it.
For native projects, they go in the 'jni' subdirectory.  You also need an Android.mk to build the files
(and optionally an Application.mk to tweak the platform/libc++/etc).
When .mk files change, you need to refresh the generated build.xml file.
For me, running:
 > android update project -p . -s
is usually sufficient.
It says to regenerate the ant build.xml project files from the current directory, including the subprojects.

You need to compile the native source first:
 > ndk-build NDK_DEBUG=1
Your libraries will get created and everything.

To rebuild the project, you run:
 > ant
or more explicitly:
 > ant debug
and normally, you'd install the generated file using:
 > adb install bin/YourApp-debug.apk
but luckily, all of the above can be done with a single ant command:
 > ant install

To debug, you run:
 > ndk-gdb --start
and then, hit 'c' to continue (I think they placed a default breakpoint).
There seem to be a bug where some symbols can't be found; sometimes, clearing everything will help:
 > ant clean

I believe the following command is sufficient to rebuild everything and test it:
 > ndk-build NDK_DEBUG=1 && ant install && ndk-gdb --start

You can use 'adb' to see the traces of your running application:
 > adb logcat ActivityManager:I native-activity:D stderr stdout \*:S
Prints the current log content for the ActivityManager (application launcher), only [I]nformation stuff,
only the [D]ebug messages for application 'native-activity', as well as anything stderr and stdout,
but makes every other message [S]ilent.
Ref: http://developer.android.com/guide/developing/debugging/debugging-log.html


If you want, you can run plain binaries on Android (i.e. no NativeActivity stuff).
This can be useful for simple regression tests.

To do that, just create plain executable (use BUILD_EXECUTABLE command rather than BUILD_STATIC_LIBRARY in your Android.mk file).
Then, all you need is to compile the executable, copy it to your emulator, and run it.
Let's assume you are making an executable called BaseTest, you'd do:
 > ndk-build
 > adb shell mkdir /data/tmp
 > adb push obj/local/armeabi/BaseTest
 > adb shell "cd /data/tmp && ./BaseTest"

Now, no code is perfect, and you'll likely need to debug it!
For that, make sure you build with debug symbols:
 > ndk-build NDK_DEBUG=1
then when you launch it, launch it with gdbserver like this:
 > adb shell "cd /data/tmp && gdbserver :5039 ./BaseTest"
once it launches, the process will wait for a remote gdb to connect.  Oblige with:
 > gg obj/local/armeabi/BaseTest -ex "target remote :5039"
where 'gg' is the gdb executable from your NDK directory (e.g. "$(NDK)/toolchains/arm-linux-androideabi-4.4.3/prebuilt/darwin-x86/bin/arm-linux-androideabi-gdb").

Hit 'c' to start the app, 'bt' for the stack, etc.

Note: if you get a "connection dropped" error, just make sure the port is properly redirected:
 > adb forward tcp:5039 tcp:5039
and try again.

Some symbols might not be found, and such.  The tool ndk-gdb actually generates a gdb.setup file that looks like this:
---
set solib-search-path /PATH/TO/YOUR/DEV/Tools/IDE/Android/Base/obj/local/armeabi
directory /PATH/TO/YOUR/NDK/platforms/android-9/arch-arm/usr/include /PATH/TO/YOUR/NDK/sources/android/native_app_glue /PATH/TO/YOUR/NDK/sources/cxx-stl/system/include /PATH/TO/YOUR/DEV/Tools/IDE/Android/Base/jni /PATH/TO/YOUR/NDK/sources/cxx-stl/system
file /PATH/TO/YOUR/DEV/Tools/IDE/Android/Base/obj/local/armeabi/BaseTest
target remote :5039
---
(adjust for your environment)
You then need to call gdb like this:
 > gg -x gdb.setup

Some of your executable will use shared libraries.  Android, being a Linux OS, honors the LD_LIBRARY_PATH variable:
 > adb shell "cd /data/tmp && LD_LIBRARY_PATH=. ./BaseTest"
Also, note that the stlport_shared library seems to have a different name in the OS than in the NDK.
I assume you should copy the one from the NDK, but copying the one from the system also works:
 > adb shell "cat /system/lib/libstlport.so > /data/tmp/libstlport_shared.so"
(notice that 'cp' doesn't exist in the emulator...)

Let's bring it all together:
> ndk-build NDK_DEBUG=1 && \
  adb push obj/local/armeabi/BaseTest /data/tmp && \
  adb shell "cd /data/tmp && LD_LIBRARY_PATH=. gdbserver :5039 ./BaseTest set"


------------------
ABOUT JNI BINDINGS
------------------

Java has a tool to generate the header files for native JNI routines.
Here's an example of what you could do:
 > cd FusionApp/bin/classes
 > javah -jni com.ludosapiens.fusion.FusionGLView
This will generate a file for every class present in the specified Java class.
This is how I learned that nativeInit() actually gets mangled into:
 JNIEXPORT void JNICALL Java_com_ludosapiens_fusion_FusionGLView_00024Renderer_nativeInit(JNIEnv *, jobject);
I suppose the 00024 indicates that it is a private class of FusionGLView.
Note: we don't need the header file, since C/C++ accepts function definition without declaration (unless we want to be strict).


---------------------
ABOUT OUR ENVIRONMENT
---------------------

I've decided to modularize the Makefiles a little.
This should bring the same kind of role as the xcconfig files (somewhat).
All of the system-wide settings belong in:
  $(DEV)/Tools/IDE/Android/CommonApplication.mk
while the file:
  $(DEV)/Tools/IDE/Android/CommonAndroid.mk
defines a few useful routines and variables.


------------
ABOUT ASSETS
------------

Android packages things in an .apk file, which is normally a zipped directory structure.
The Java API has an AssetManager class:
  http://developer.android.com/reference/android/content/res/AssetManager.html
which can be used to read the files from the apk, but this API doesn't seem to exist on the native (C/C++) side.

The following link mentions alternatives:
  http://stackoverflow.com/questions/2278359/android-how-to-make-game-asset-files-readable-from-c-code-using-ndk

Google allows to package files in an uncompressed way, and even align them in memory so that they can be easily memory-mapped.
The only downside is that Google might one days uncompress its apk, so retrieving the address of the apk, and it's internal offset for specific asset files might kill forward compatibility.

An alternative I've been considering is to have callbacks in Fusion::ResManager to create the InputStream for a resource's path.
This would allow to either:
1) Read directly from a compressed archive (the apk, without alignment).
2) Uncompress assets each time they are read (maybe using the Java API in the cache directory).
3) Use memory-mapped files once Base supports them.
It is the most flexible approach, but it also needs the most work.

For now, I've just hardcoded the root to be /sdcard/Data, and you must copy everything there by hand:
 > adb push .../Data /sdcard/Data
which takes a few minutes.

The upside is that you don't have to push all of the data assets every time you call 'ant install' (slower packaging times, as well as upload size).


------
ISSUES
------

Sounds like there are issues with base_string in the STLPort static or shared (not that the base libstdc++ doesn't support C++ string at all).
To reproduce, just add a String() into a Map<String>, and you will crash when quitting the app.
Makes me think of the Linux issue with empty strings (the one which forced us to reserve 4 characters).
Methinks we'll want to stop using base_string altogether and write our own replacement.
For now, using:
 APP_STL := gnustl_static
seems to circumvent the problem.

Also, the NDK documentation mentions that static constructors might be called twice (in static libraries only), and that static destructors aren't even called.


--------
COMMENTS
--------

Command-line regression test binaries should probably create lightweight NDK BUILD_EXECUTABLE, while GUI-based ones (starting at Fusion) should use the more thorough approach of NativeActivity (either android_native_app_glue or our own Java interfacing).
