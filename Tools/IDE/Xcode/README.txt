The Xcode projects make extensive use of .xcconfig files to set parameters by default.

Previously, the user had to jump a bunch of hoops.  Hopefully, the new way will streamline things and make everything easier.

Here are the steps to follow:
- When creating new projects, add the Tools/IDE/Xcode/Config/Target directory (but not under any target; it just makes the target available).
- Create your new target, and specify an XCConfig in the Active Target Settings --> Build settings tab.
  ("Based On" in bottom right corner, select a different one for each build configuration, i.e. Debug and Release)
    _____TYPE______|_______NEW_TARGET_TEMPLATE______|___XCCONFIG__________
    OSX shared lib | BSD --> Dynamic Library        | OSX_SharedLib_{Debug,Release}
    OSX static lib | BSD --> Static Library         | OSX_StaticLib_{Debug,Release}
    OSX CLI app    | BSD --> Shell Tool             | OSX_Executable_{Debug,Release}
    OSX GUI app    | Cocoa --> Application          | OSX_Application_{Debug,Release}
    iPhone lib     | Cocoa Touch --> Static Library | iOS_StaticLib_{Debug,Release}
    iPhone app     | Cocoa Touch --> Application    | iOS_Application_{Debug,Release}

Normally, a bunch of "Settings Defined at This Level" get be deleted safely, as they are found in the various .xcconfig files.
Compile flags as well as search paths can usually be done with.
If you specify some additional path, always start with $(inherited) in order to grab the ones from the files.

Also, you need to set:
   DEV_ROOT = $(SRCROOT)/../../..
(adjust for you path) in your Project --> Edit Project Settings --> Build settings.

For naming targets, we use the following convention:
  Target                Product Name       Generated file
  ------                ------------       --------------
  MyStuffApp (iOS)      MyStuff            Tools/IDE/Xcode/build/{Debug,Release}{iphonesimulator,-iphoneos}/MyStuff.app
  MyStuffLib (iOS)      MyStuff            Tools/IDE/Xcode/build/{Debug,Release}{iphonesimulator,-iphoneos}/libMyStuff.a
  MyStuffApp (OSX)      MyStuff            Tools/IDE/Xcode/build/{Debug,Release}/MyStuff.app
  MyStuffLib (OSX)      MyStuff            Tools/IDE/Xcode/build/{Debug,Release}/libMyStuff.a
  MyStuffTest (OSX)     MyStuffTest        Tools/IDE/Xcode/build/{Debug,Release}/MyStuffTest
  MyStuffApp (X11)      MyStuffX11         Tools/IDE/Xcode/build/{Debug,Release}/MyStuffX11
  MyStuffLib (X11)      MyStuffX11         Tools/IDE/Xcode/build/{Debug,Release}/libMyStuffX11.a

When adding Thirdparty libraries, it is better to define the following:
   DEV_LDFLAGS = $(inherited) -lvorbisfile -lvorbis -logg
The reason is that adding "Existing Files..." would be tricky since OSX/Simulator/Device are each in a different directory.
Also, since we already add -L$(DEV_THIRDPARTY)/lib, we only need to use '-lsomeLib' format, so use DEV_LDFLAGS for those.
For system frameworks, since those don't expect specific directory specifier, you could use OTHER_LDFLAGS normally
(or add the frameworks to the target), but I found that using DEV_LDFLAGS was easier because Xcode splits things like
'framework OpenGL' into 2 lines when you open it up again.

It seems static libraries can be hidden inside our libraries (e.g. ogg/vorbis in Snd, or lua in Fusion).
To do this, simply add those in the library.
Unfortunately, frameworks don't seem to work the same, so you'll have to specify them in app bundles,
so might as well have them there only.


======================
======================
OUTDATED CONTENT BELOW
======================
======================

Important notes:
- Start XCode from an X11 term window already set to compile the project.
- USE_HEADERMAP is the secret sauce to fix the StdDef.h recursive includes (need a proper -Idir option, though, but that's set in the project files).
- Everything needs to be built in the same output directory for subtargets to work properly (known bug in XCode).
- Shark doesn't propagate project environment variables, so to use it, you need to:
  - Set ARCHETYPE_DATA by hand.
  - Also set DISPLAY to ":0.0".
  - Make sure X11 accepts external connections ('xhost +').
  (actually, if you add them to the Target Executable Options (Cmd-Opt-X), it works fine)

Simple things in XCode aren't well documented:
  - Subtargets work by drag'n'drop (or "Add existing files...", then you need to slide the lib file into the linked libraries.
  - Static precompiled libraries (Thirdparty) need to be added with "Add existing files...".
  - When a compile fails, click on the "text lines" icon to see the actual command-line used.


How to create new project files:
- Start with an empty project
- Add a new target to the project: BSD Dynamic Library or BSD Shell Tool
- Add include paths ("$(SRCROOT)/../../../Projects" and maybe even "$(SRCROOT)/../../../Thirdparty/install/include")
- Create USE_HEADERMAP variable (leave it unset)
- Add Existing Files and add all of the source files (headers included)
- Add Existing Frameworks where appropriate
- Add Existing Files (Thirdparty/install/lib/* or other Xcode projects) where appropriate
- Good usage is to put all of the source under a "Source" group and all libraries+frameworks under "Libraries"
- Drag'n'drop appropriate libraries to appropriate targets
- Don't forget to set subproject dependencies


Notes on Frameworks, Application, and Bundles in general:
- We probably shouldn't export the header files in the Frameworks, since it's a release of source code.
- Frameworks can still be used to bundle .xib files, etc.
- Dynamic libraries (dylib) should not be copied to frameworks, else we'll get libBase.dylib multiple times.
- Shared libraries and Frameworks need to set their "Install Name" to "@executable_path/../Frameworks/$(EXECUTABLE_PATH)".
  This fixes issues with the dynamic loader which tries to find things under /usr/local/lib by default.
  Not sure why Xcode-launched instances don't get that issue, but it does exist when you move the .app file.
- I don't think there is an official location for dylib files, but everyone seem to place them under the Frameworks
  directory, probably because it's a path looked for by the dynamic linker.
Some links:
  http://lists.apple.com/archives/xcode-users/2008/May/msg00513.html
  http://doc.trolltech.com/4.3/deployment-mac.html
  http://qin.laya.com/tech_coding_help/dylib_linking.html
  http://www.shuningbian.net/2005/02/creating-os-x-application-bundles-step.html
- Cocoa application all need to include {...}/Projects/Fusion/Core/Cocoa/Fusion.nib and set their main nib to 'Fusion'.
  Failure do include the nib file will still run, but crash as soon as any OpenGL call is made (since there is no context).

Known issues:
ISSUE:  The class browser shows very few classes.  Not sure what causes this. <sigh>
REASON: The includes were using -ipath instead of -I.  It looks OK now.

ISSUE: When I try a debug an executable, no breakpoint work.
REASON: LLVM-4.2 doesn't support debug symbols when optimizations are turned on
  http://developer.apple.com/ReleaseNotes/DeveloperTools/RN-llvm-gcc/index.html#//apple_ref/doc/uid/TP40007133-SW7
So I switched to GCC 4.2, since LLVM isn't bringing me much for now.

ISSUE:  Xcode sometimes complain about the SDK being for the wrong platform (iPhone related error for Cocoa target, or vice versa).
REASON: Sounds like Xcode has some bugs.  Closing and reopening the project file (with the right target selected) usually fixes it.
Sometimes, relaunching Xcode helps.  Rebooting OSX has never been a fix (so far).  So check your project files in case the error stays.


Hope this will help someone...

Jocelyn


=============================================================================
UPDATES CONCERNING XCODE 4
=============================================================================

The transition was much less problematic than anticipated.

First of all, project settings are now under the following hierarchy:
  Something.xcodeproj/
  + project.xcworkspace/
  | | contents.xcworkspacedata            // Only a reference to the .xcodeproj directory (typically a parent).
  | + xcshareddata/                       // Data shared across all users.
  | | | WorkspaceSettings.xcsettings      // All of the IDEWorkspaceSharedSettings_* settings.
  | + xcuserdata/                         // Data specific to certain users.
  | | + someuser.xcuserdatad/             // Data specific to user 'someuser'.
  | | | | UserInterfaceState.xcuserstate  // Location of windows, etc.
  | | | | WorkspaceSettings.xcsettings    // All of the IDEWorkspaceUserSettings_* settings.
  + xcshareddata/                         // Project? data shared across all users.
  | + xcschemes/                          // The list of schemes.
  | | | SomeTarget.xcscheme               // The scheme definition for SomeTarget.
  | | | SomeOtherTarget.xcscheme          // The scheme definition for SomeOtherTarget.
  + xcuserdata/                           // User-specific project? data.
  | + someuser.xcuserdatad/               // Project data specific to user 'someuser'.
  | | + xcschemes/                        // Scheme-related settings.
  | | |   xcschememanagement.plist        // The list of scheme, and visibility settings.

None of the xcuserdata/* stuff should be under revision control.

Schemes
-------

Schemes are a new concept in Xcode 4.  They encapsulate both target and some extra settings for typical builds.
A 'shared' scheme is one that ends up in the xcshareddata subdirectory (otherwise, it's xcuserdata/*.xcuserdatad/).
Cmd-< (a.k.a. Cmd-Opt-,) lets you control a few things.

Note: the schemes visibility is per-user, and gets carried across subprojects, so you cannot easily hide BaseTest from Plasma.
(+) All of the target visible from Plasma.
(-) More clutter everywhere.

Build location
--------------
By default, Xcode 4 places build stuff (intermediates, products, etc.) under ~/Library/Developer/Xcode/DerivedData.
Their rationale is that Schemes (which are user-specific) can tweak settings.
This allows to have the same library, but under different scheme settings, to build properly.
The unfortunate side effect is that CGMath recompiles BaseLib, Fusion recompiles BaseLib, Plasma recompiles BaseLib, ...
We can revert to the previous method (i.e. a 'build' subdirectory), but there doesn't seem to be a way to set it in the xcshareddata settings
yet, the user settings can have:
	<key>IDEWorkspaceUserSettings_BuildLocationStyle</key>
	<integer>2</integer>
The better alternative for now is to just change Xcode's default behavior:
  Prefs->Locations->Build Location: Place build products in locations specified by targets

Run location
------------
We need to set a custom working directory for some executables.
Unfortunately, Apple seems to support neither relative paths nor variable expansions.
Because of this, we cannot make our scheme shared (because our paths are unique).
It's an annoyance, but it should only happen once per scheme (and our .app bundles should be fine).

