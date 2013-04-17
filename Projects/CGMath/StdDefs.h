/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef CGMATH_STDDEFS_H
#define CGMATH_STDDEFS_H

#include <StdDefs.h>

// DLL Export and Import definition.
//
// When COMPILING_CGMATH is defined (making the library), symbols are exported.
// Otherwise (using the library), symbols are imported.
#ifndef CGMATH_DLL_API

#if defined(_WIN32) && (!defined(COMPILING_STATIC) || COMPILING_STATIC == 0)
//------------------------------------------------------------------------------
//! Windows
#  ifdef COMPILING_CGMATH
#    define CGMATH_DLL_API __declspec( dllexport )
#  else
#    define CGMATH_DLL_API __declspec( dllimport )
#  endif
//------------------------------------------------------------------------------

#else
//------------------------------------------------------------------------------
//! Others
#  define CGMATH_DLL_API
//------------------------------------------------------------------------------

#endif

#endif //CGMATH_DLL_API

#endif //CGMATH_STDDEFS_H
