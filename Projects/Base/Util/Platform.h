/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_PLATFORM_H
#define BASE_PLATFORM_H

/**
 * Defines various values based on the current platform being compiled for.
 * Some defines are orthogonal to others, while other can overlap.
 *   PLAT_ANDROID: Set when compiling for Google's Android platform (PLAT_MOBILE=1).
 *   PLAT_APPLE  : Set for every Apple-related platforms.
 *   PLAT_CYGWIN : Set when compiling under Cygwin (PLAT_WINDOWS=PLAT_POSIX=1).
 *   PLAT_IPHONE : Set when compiling for iPhoneOS, either PLAT_IPHONE_DEVICE or PLAT_IPHONE_SIMULATOR (PLAT_APPLE=PLAT_MOBILE=1, PLAT_MACOSX=0).
 *   PLAT_LINUX  : Set when compiling for Linux (PLAT_MACOSX=PLAT_WINDOWS=0).
 *   PLAT_MACOSX : Set when compiling for MacOSX (PLAT_IPHONE=0).
 *   PLAT_MINGW  : Set when compiling under MinGW (PLAT_WINDOWS=PLAT_POSIX=1).
 *   PLAT_MOBILE : Set for mobile platforms (either PLAT_ANDROID, PLAT_IPHONE is set).
 *   PLAT_POSIX  : Set for POSIX-compliant platform (necessary?).
 *   PLAT_WINDOWS: Set for all Windows-based compiles.
 */

// Values of PLAT_IPHONE.
#define PLAT_IPHONE_DEVICE     1
#define PLAT_IPHONE_SIMULATOR  2

// PLAT_APPLE
// PLAT_IPHONE
// PLAT_MACOSX
#if __APPLE__
#  if !defined(PLAT_APPLE)
#    define PLAT_APPLE 1
#  endif
#  include <TargetConditionals.h>
#  if !defined(PLAT_IPHONE)
#    if TARGET_OS_IPHONE
#      if !defined(TARGET_IPHONE_SIMULATOR)
#        define PLAT_IPHONE 1 // PLAT_IPHONE_DEVICE
#      else
#        define PLAT_IPHONE 2 // PLAT_IPHONE_SIMULATOR
#      endif
#    else
#      define PLAT_IPHONE 0
#    endif
#  endif //PLAT_IPHONE
#  if !defined(PLAT_MACOSX)
#    if PLAT_IPHONE
#      define PLAT_MACOSX 0
#    else
#      define PLAT_MACOSX 1
#    endif
#  endif //PLAT_MACOSX
#else
#  if !defined(PLAT_APPLE)
#    define PLAT_APPLE 0
#  endif
#  if !defined(PLAT_IPHONE)
#    define PLAT_IPHONE 0
#  endif
#  if !defined(PLAT_MACOSX)
#    define PLAT_MACOSX 0
#  endif
#endif //__APPLE__

// PLAT_WINDOWS
// PLAT_CYGWIN
// PLAT_MINGW
#ifdef _WIN32
#  if !defined(PLAT_WINDOWS)
#    define PLAT_WINDOWS 1
#  endif
#  if !defined(PLAT_CYGWIN)
#    if defined(__CYGWIN__)
#      define PLAT_CYGWIN 1
#    else
#      define PLAT_CYGWIN 0
#    endif
#  endif
#  if !defined(PLAT_MINGW)
#    if defined(_POSIX_VERSION) && !defined(__CYGWIN__)
#      define PLAT_MINGW 1
#    else
#      define PLAT_MINGW 0
#    endif
#  endif
#else
#  if !defined(PLAT_WINDOWS)
#    define PLAT_WINDOWS 0
#  endif
#  if !defined(PLAT_CYGWIN)
#    define PLAT_CYGWIN 0
#  endif
#  if !defined(PLAT_MINGW)
#    define PLAT_MINGW 0
#  endif
#endif //_WIN32

// PLAT_ANDROID
#if !defined(PLAT_ANDROID)
#  if defined(__ANDROID__)
#    define PLAT_ANDROID 1
#  else
#    define PLAT_ANDROID 0
#  endif
#endif

// PLAT_MOBILE
#if !defined(PLAT_MOBILE)
#  if PLAT_ANDROID || PLAT_IPHONE
#    define PLAT_MOBILE 1
#  else
#    define PLAT_MOBILE 0
#  endif
#endif

// PLAT_POSIX
#if !defined(PLAT_POSIX)
#  if _POSIX_VERSION
#    define PLAT_POSIX 1
#  else
#    define PLAT_POSIX 0
#  endif
#endif

#endif //BASE_PLATFORM_H
