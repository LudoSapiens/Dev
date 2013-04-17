/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef SND_STDDEFS_H
#define SND_STDDEFS_H

#include <StdDefs.h>

// DLL Export and Import definition.
//
// When COMPILING_SND is defined (making the library), symbols are exported.
// Otherwise (using the library), symbols are imported.
#ifndef SND_DLL_API

#if defined(_WIN32) && (!defined(COMPILING_STATIC) || COMPILING_STATIC == 0)
//------------------------------------------------------------------------------
//! Windows
#  ifdef COMPILING_SND
#    define SND_DLL_API __declspec( dllexport )
#  else
#    define SND_DLL_API __declspec( dllimport )
#  endif
//------------------------------------------------------------------------------

#else
//------------------------------------------------------------------------------
//! Others
#  define SND_DLL_API
//------------------------------------------------------------------------------

#endif

#endif //SND_DLL_API


//------------------------------------------------------------------------------
//!
#define SND_MAKE_MANAGERS_FRIENDS() \
   friend class NullManager; \
   friend class OpenALManager; \
   friend class Manager


//---------------------
// API support control
//---------------------
//Null is for all platforms
#ifndef SND_NULL_SUPPORT
#  define SND_NULL_SUPPORT 1
#endif

//OpenAL is for [almost] all platforms
#ifndef SND_OPENAL_SUPPORT
#  if defined(__ANDROID__)
#    define SND_OPENAL_SUPPORT 0
#  else
#    define SND_OPENAL_SUPPORT 1
#  endif
#endif

//----------------------
// OpenAL configuration
//----------------------
#if SND_OPENAL_SUPPORT
// Nothing special
#endif //SND_OPENAL_SUPPORT

#endif //SND_STDDEFS_H
