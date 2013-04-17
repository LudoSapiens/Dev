/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_STDDEFS_H
#define PLASMA_STDDEFS_H

#include <StdDefs.h>

// DLL Export and Import definition.
//
// When COMPILING_PLASMA is defined (making the library), symbols are exported.
// Otherwise (using the library), symbols are imported.
#ifndef PLASMA_DLL_API

#if defined(_WIN32) && (!defined(COMPILING_STATIC) || COMPILING_STATIC == 0)
//------------------------------------------------------------------------------
//! Windows
#  ifdef COMPILING_PLASMA
#    define PLASMA_DLL_API __declspec( dllexport )
#  else
#    define PLASMA_DLL_API __declspec( dllimport )
#  endif
//------------------------------------------------------------------------------

#else
//------------------------------------------------------------------------------
//! Others
#  define PLASMA_DLL_API
//------------------------------------------------------------------------------

#endif

#endif //PLASMA_DLL_API


// MOTION_BULLET is the default.
#ifndef MOTION_BULLET
#define MOTION_BULLET 1
#endif

#endif //PLASMA_STDDEFS_H
