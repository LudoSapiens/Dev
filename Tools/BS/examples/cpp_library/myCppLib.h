/*=============================================================================
   Copyright (c) 2006, Ludo Sapiens Inc.
   All rights reserved.

   These coded instructions, statements, and computer programs contain
   unpublished, proprietary information and are protected by Federal copyright
   law. They may not be disclosed to third parties or copied or duplicated in
   any form, in whole or in part, without prior written consent.
=============================================================================*/
#ifndef MYCPPLIB_H_INCLUDED
#define MYCPPLIB_H_INCLUDED


// DLL Export and Import definition.
//
// COMPILING_MYCPPLIB is defined only when compiling MyCppLib as a shared library
#if !defined( MYCPPLIB_DLL_API )
#  if defined( _WIN32 ) && !defined( __GNUC__ )
//!  Windows
#    ifdef COMPILING_MYCPPLIB
#      define MYCPPLIB_DLL_API __declspec( dllexport )
#    else
#      define MYCPPLIB_DLL_API __declspec( dllimport )
#    endif //COMPILING_MYCPPLIB
#  else
//!  Others
#    define MYCPPLIB_DLL_API
#  endif //various platforms
#endif //!defined( MYCPPLIB_DLL_API )

//Safeguarding
#if !defined(__cplusplus)
#error "This header requires the use of a C++ compiler"
#endif //__cplusplus


// Prototypes
MYCPPLIB_DLL_API void myCppLibFoo();


#endif //MYCPPLIB_H_INCLUDED
