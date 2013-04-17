/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_DBG_DEFS_H
#define BASE_DBG_DEFS_H

#include <Base/StdDefs.h>
#include <Base/Util/CPU.h>

#include <cstdio>
#include <cstdlib>

#if defined(_MSC_VER)
#include <intrin.h>
#endif

NAMESPACE_BEGIN


//------------------------------------------------------------------------------
//! This file contains various utility defines that should help debugging.
//!
//! DBG_HALT()
//! Stops execution, trying to end up in the debugger when possible.
//!
//! DBG( expr )
//! This macro only execute the specified expression if we are in a debug build.
//!
//! DBG_BEGIN()
//! <...code...>
//! DBG_END()
//! These macros surround code that will only get executed in a debug build.
//!
//! CHECK( cond )
//! This macro guarantees that the condition is met before proceeding.
//! If it is not, the execution halts with an error.
//!
//! Some comments:
//!
//! Use of do { ... } while( false ) is to force the user to terminate with a semi-colon.
//!
//! This sizeof trick is so that the compiler doesn't generate any code, yet still checks syntax.

#if defined(_MSC_VER)
// Taken from: http://msdn.microsoft.com/en-us/library/f408b4et.aspx
// Included <intrin.h> outside the namespace above.
#  define DBG_HALT()  __debugbreak()
//#  define DBG_HALT() __asm { int 3 }  // Alternate solution for VS.

#elif defined(__APPLE__)
// Taken from: http://developer.apple.com/documentation/DeveloperTools/Conceptual/XcodeProjectManagement/090-Running_Programs/chapter_10_section_3.html
#  if CPU_ARCH == CPU_ARCH_X86
#    define DBG_HALT()  asm("int $3")
#  elif CPU_ARCH == CPU_ARCH_PPC
#    define DBG_HALT()  asm("trap")  // Was asm { trap }, but requires -fasm-blocks, only available in Apple's GCC.
#  elif CPU_ARCH == CPU_ARCH_ARM
#    define DBG_HALT()  asm( "bkpt 0" )
#  endif

#elif defined(__GNUC__)
#  if CPU_ARCH == CPU_ARCH_X86
#    define DBG_HALT() asm("int $3")
#  endif

#endif

#if !defined(DBG_HALT)
// Fallback solution.
#  include <cassert>
#  define DBG_HALT()  assert( false )
#endif


#if defined( _DEBUG )

///////////////////////////////
// Macros used in debug builds.

#define DBG( expr ) \
   expr

#define DBG_BEGIN() \
   do { } while( false )

#define DBG_END() \
   do { } while( false )

#define CHECK( cond ) \
   do \
   { \
      if( !(cond) ) \
      { \
         printf( "error: \"%s\"\n func: %s\n file: %s\n line: %d\n", #cond, __FUNCTION__, __FILE__, __LINE__ ); \
         abort(); \
      } \
   } while( false )

#else

/////////////////////////////////
// Macros used in release builds.

#define DBG( expr )

#define DBG_BEGIN() \
   if( false ) \
   { \
      do { } while( false )

#define DBG_END() \
   } \
   do { } while( false )

#define CHECK( expr ) \
   do \
   { \
      (void)sizeof( expr ); \
   } while( false )

#endif


NAMESPACE_END

#endif //BASE_DBG_DEFS_H
