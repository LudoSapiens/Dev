/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_STDDEFS_H
#define BASE_STDDEFS_H

#include <StdDefs.h>

// DLL Export and Import definition.
//
// When COMPILING_BASE is defined (making the library), symbols are exported.
// Otherwise (using the library), symbols are imported.
#ifndef BASE_DLL_API

#if defined(_WIN32) && (!defined(COMPILING_STATIC) || COMPILING_STATIC == 0)
//------------------------------------------------------------------------------
//! Windows
#  ifdef COMPILING_BASE
#    define BASE_DLL_API __declspec( dllexport )
#  else
#    define BASE_DLL_API __declspec( dllimport )
#  endif
//------------------------------------------------------------------------------

#else
//------------------------------------------------------------------------------
//! Others
#  define BASE_DLL_API
//------------------------------------------------------------------------------

#endif

#endif //BASE_DLL_API

#if !defined(BASE_INT_NOT_INT32_T)
#if defined(__GNUC__) && defined(_WIN32)
#  if !defined(__CYGWIN__)
#    define BASE_INT_NOT_INT32_T 1
#  else
#    define BASE_INT_NOT_INT32_T 0
#  endif
#else
#  define BASE_INT_NOT_INT32_T 0
#endif
#endif //!defined(BASE_INT_NOT_INT32_T)
#endif //BASE_STDDEFS_H
