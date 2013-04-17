/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef MOTION_STDDEFS_H
#define MOTION_STDDEFS_H

#include <StdDefs.h>

// DLL Export and Import definition.
//
// When COMPILING_MOTION is defined (making the library), symbols are exported.
// Otherwise (using the library), symbols are imported.
#ifndef MOTION_DLL_API

#if defined(_WIN32) && (!defined(COMPILING_STATIC) || COMPILING_STATIC == 0)
//------------------------------------------------------------------------------
//! Windows
#  ifdef COMPILING_MOTION
#    define MOTION_DLL_API __declspec( dllexport )
#  else
#    define MOTION_DLL_API __declspec( dllimport )
#  endif
//------------------------------------------------------------------------------

#else
//------------------------------------------------------------------------------
//! Others
#  define MOTION_DLL_API
//------------------------------------------------------------------------------

#endif

#endif //MOTION_DLL_API

#endif //MOTION_STDDEFS_H
