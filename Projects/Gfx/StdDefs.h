/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef GFX_STDDEFS_H
#define GFX_STDDEFS_H

#include <StdDefs.h>

// DLL Export and Import definition.
//
// When COMPILING_GFX is defined (making the library), symbols are exported.
// Otherwise (using the library), symbols are imported.
#ifndef GFX_DLL_API

#if defined(_WIN32) && (!defined(COMPILING_STATIC) || COMPILING_STATIC == 0)
//------------------------------------------------------------------------------
//! Windows
#  ifdef COMPILING_GFX
#    define GFX_DLL_API __declspec( dllexport )
#  else
#    define GFX_DLL_API __declspec( dllimport )
#  endif
//------------------------------------------------------------------------------

#else
//------------------------------------------------------------------------------
//! Others
#  define GFX_DLL_API
//------------------------------------------------------------------------------

#endif

#endif //GFX_DLL_API


//------------------------------------------------------------------------------
//!
#define GFX_MAKE_MANAGERS_FRIENDS() \
   friend class D3DManager;   \
   friend class GLES1Manager; \
   friend class GLES2Manager; \
   friend class GLManager;    \
   friend class NullManager;  \
   friend class Manager


//---------------------
// API support control
//---------------------
//Null is for everyone.
#define GFX_NULL_SUPPORT 1

//Direct3D is Windows-only
#ifndef GFX_D3D_SUPPORT
#  if defined( _WINDOWS ) || defined( _WIN32 )
#    define GFX_D3D_SUPPORT 1
#  endif
#endif

//OpenGL ES 1.1 is for mobile platforms
#ifndef GFX_GLES_1_SUPPORT
#  if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR || __IPHONE_OS_VERSION_MIN_REQUIRED || __ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__
#    define GFX_GLES_1_SUPPORT 1
#  elif defined(__ANDROID__)
#    define GFX_GLES_1_SUPPORT 1
#  else
#    define GFX_GLES_1_SUPPORT 0
#  endif
#endif

//OpenGL ES 2.x is for mobile platforms
#ifndef GFX_GLES_2_SUPPORT
#  if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR || __IPHONE_OS_VERSION_MIN_REQUIRED || __ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__
#    define GFX_GLES_2_SUPPORT 1
#  elif defined(__ANDROID__)
#    define GFX_GLES_2_SUPPORT 1
#  else
#    define GFX_GLES_2_SUPPORT 0
#  endif
#endif

#ifndef GFX_GLES_EGL_SUPPORT
#  define GFX_GLES_EGL_SUPPORT 0
#endif

//OpenGL is for all platforms that don't already have OpenGL ES
#ifndef GFX_OGL_SUPPORT
#  if GFX_GLES_1_SUPPORT || GFX_GLES_2_SUPPORT
#    define GFX_OGL_SUPPORT 0
#  else
#    define GFX_OGL_SUPPORT 1
#  endif
#endif


//----------------------
// OpenGL configuration
//----------------------
#if GFX_OGL_SUPPORT

//GLX is UNIX (Linux and MacOSX), but we require explicit specification
#ifndef GFX_OGL_GLX_SUPPORT
#  define GFX_OGL_GLX_SUPPORT 0
#endif

//WGL is Windows-only
#ifndef GFX_OGL_WGL_SUPPORT
#  if defined( _WINDOWS ) || defined( _WIN32 )
#    define GFX_OGL_WGL_SUPPORT 1
#  endif //WINDOWS
#endif

#endif //GFX_OGL_SUPPORT


#endif //GFX_STDDEFS_H
