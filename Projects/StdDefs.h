/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef STDDEFS_H
#define STDDEFS_H

#if defined(_MSC_VER)

// Note: Compiler adds 4000 to warning numbers in range [0, 999].

// Squash:
// C4530: C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc
// C4786: Identifier was truncated to 'number' characters in the debug information
#pragma warning(disable: 530 786)

// Force (at warning level 1):
// C4100: 'identifier' : unreferenced formal parameter
// C4101: 'identifier' : unreferenced local variable
// C4255: 'function': no function prototype given: converting '()' to '(void)'
// C4289: nonstandard extension used : 'var' : loop control variable declared in the for-loop is used outside the for-loop scope
// C4296: 'operator': expression is always false
// C4431: missing type specifier - int assumed. Note: C no longer supports default-int
#pragma warning(1: 100 101 255 289 296 431)

// Some POSIX printf/scanf routines give a deprecated warning.
#define _CRT_SECURE_NO_WARNINGS 1

// Visual Studio Express 2008 still doesn't support C99's stdint.h header.
#if _MSC_VER <= 1500
#define NOSTDINT 1
#endif

#endif //defined(_MSC_VER)

// size_t, ptrdiff_t, NULL.
#include <cstddef>
// int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t (C99).

#if NOSTDINT
typedef   signed char       int8_t;
typedef   signed short      int16_t;
typedef   signed int        int32_t;
typedef   signed long long  int64_t;
typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned long long  uint64_t;
#include <stdlib.h> // to get random()
#else
#include <stdint.h>
#endif

typedef unsigned char  uchar;
typedef unsigned short ushort;
typedef unsigned int   uint;

#define NAMESPACE Dev
#define NAMESPACE_BEGIN namespace Dev {
#define NAMESPACE_END }
#define USING_NAMESPACE using namespace Dev;

#define UNNAMESPACE_BEGIN namespace {           \
   USING_NAMESPACE

#define UNNAMESPACE_END }

// On some platforms, windef.h defines a min and a max macro
#ifndef NOMINMAX
#define NOMINMAX
#endif

#if defined(__CYGWIN__) && !defined(_WIN32)
#define _WIN32
#endif

// GCC prior to 4.6.x does not support nullptr.
#if defined(__GNUC__) && ((__GNUC__ == 4) && (__GNUC_MINOR__ <= 5))
#  define nullptr  NULL
#endif

// Debugging define.
#if ( !defined( __OPTIMIZE__ ) && defined( __GNUC__ ) ) || defined( _DEBUG )
// For now, keep the name as _DEBUG (but avoid warning in VisualStudio)
#if !defined(_DEBUG)
#define _DEBUG
#endif
#endif

// Some trick to ignore some unused parameters in functions.
// Ref: http://herbsutter.com/2009/10/18/mailbag-shutting-up-compiler-warnings/
template< typename T > inline void unused( const T& ) {}

#endif //STDDEFS_H
