/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_STDDEFS_H
#define FUSION_STDDEFS_H

#include <StdDefs.h>

// DLL Export and Import definition.
//
// When COMPILING_FUSION is defined (making the library), symbols are exported.
// Otherwise (using the library), symbols are imported.
#ifndef FUSION_DLL_API

#if defined(_WIN32) && (!defined(COMPILING_STATIC) || COMPILING_STATIC == 0)
//------------------------------------------------------------------------------
//! Windows
#  ifdef COMPILING_FUSION
#    define FUSION_DLL_API __declspec( dllexport )
#  else
#    define FUSION_DLL_API __declspec( dllimport )
#  endif
//------------------------------------------------------------------------------

#else
//------------------------------------------------------------------------------
//! Others
#  define FUSION_DLL_API
//------------------------------------------------------------------------------

#endif

#endif //FUSION_DLL_API

#endif //FUSION_STDDEFS_H
