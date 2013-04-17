/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_COMPILER_H
#define BASE_COMPILER_H

//------------------------------------------------------------------------------
// This header file contains a few compiler utility macros.
//------------------------------------------------------------------------------

#define COMPILER_UNKNOWN  0
#define COMPILER_GCC      1
#define COMPILER_MSVC     2


#if defined(__GNUC__)
#  define COMPILER  COMPILER_GCC
// Define __GNUC_VERSION__ to represent VVRRPP (e.g. 40301 for GCC 4.3.1).
#  if !defined(__GNUC_VERSION__)
#    if defined(__GNUC_PATCHLEVEL__)
#      define __GNUC_VERSION__ ((__GNUC__*10000) + (__GNUC_MINOR__*100) + (__GNUC_PATCHLEVEL__))
#    else
#      define __GNUC_VERSION__ ((__GNUC__*10000) + (__GNUC_MINOR__*100))
#    endif
#  endif //!defined(__GNUC_VERSION__)
#endif //defined(__GNUC__)


#if defined(_MSC_VER)
#  define COMPILER  COMPILER_MSVC
#endif //defined(_MSC_VER)


#if !defined(COMPILER)
#  define COMPILER  COMPILER_UNKNOWN
#endif // !defined(COMPILER)

// Define some C++11 flag (for more succinct detection).
#if (__cplusplus >= 201103L) || (__clang_major__ >= 2) || (__GNUC_VERSION__ >= 40400)
#  define COMPILER_CXX11  1
#else
#  define COMPILER_CXX11  0
#endif

#endif //BASE_COMPILER_H
