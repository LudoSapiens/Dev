/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_CPU_H
#define BASE_CPU_H

#include <Base/StdDefs.h>


////////////////////
//CPU ARCHITECTURE//
////////////////////
// Describes the broad architecture family being compiled for.
#define CPU_ARCH_UNKNOWN 0x00
#define CPU_ARCH_X86     0x01 // Including x86_64; see CPU_SIZE.
#define CPU_ARCH_PPC     0x02
#define CPU_ARCH_ARM     0x03

#if !defined(CPU_ARCH)

// x86 architecture.
#if defined(__i386__) || defined(_M_IX86) || defined(_M_X64) || defined(__x86_64__)
#  define CPU_ARCH  CPU_ARCH_X86

// PowerPC architecture.
#elif defined(__powerpc__) || defined(__PPC__)
#  define CPU_ARCH  CPU_ARCH_PPC

// ARM architecture.
#elif defined(__arm__) || defined(__ARM__)
#  define CPU_ARCH  CPU_ARCH_ARM

#else
#  define CPU_ARCH  CPU_ARCH_UNKNOWN
#  error "Unsupported architecture detection.  Please fix."

#endif

#endif //CPU_ARCH


//////////////////
//CPU ENDIANNESS//
//////////////////
// Describes the byte order of the architecture being compiled for.
#define CPU_ENDIAN_UNKNOWN 0x00
#define CPU_ENDIAN_LITTLE  0x01
#define CPU_ENDIAN_BIG     0x02

#if !defined(CPU_ENDIANNESS)

// LITTLE_ENDIAN is explicitly defined (and alone).
#if (defined(LITTLE_ENDIAN) || defined(_LITTLE_ENDIAN)) && !(defined(BIG_ENDIAN) || defined(_BIG_ENDIAN))
#  define CPU_ENDIANNESS  CPU_ENDIAN_LITTLE

// BIG_ENDIAN is explicitly defined (and alone).
#elif (defined(BIG_ENDIAN) || defined(_BIG_ENDIAN)) && !(defined(LITTLE_ENDIAN) || defined(LITTLE_ENDIAN))
#  define CPU_ENDIANNESS  CPU_ENDIAN_BIG

// x86 platform (Intel).
#elif CPU_ARCH == CPU_ARCH_X86
#  define CPU_ENDIANNESS  CPU_ENDIAN_LITTLE

// PPC platform (IBM).
#elif CPU_ARCH == CPU_ARCH_PPC
#  define CPU_ENDIANNESS  CPU_ENDIAN_BIG

// ARM platform.
#elif CPU_ARCH == CPU_ARCH_ARM
#  define CPU_ENDIANNESS  CPU_ENDIAN_LITTLE

// Unknown.
#else
#  define CPU_ENDIANNESS  CPU_ENDIAN_UNKNOWN
#  error "Unsupported endianness detection.  Please fix."

#endif

#endif //CPU_ENDIANNESS


////////////
//CPU SIZE//
////////////
// Describes the width of a typical integer register for the architecture being compiled for.
// For 32b architectures, it is set to 32, for 64b it is 64, ...
// Used the following references:
//   http://predef.sourceforge.net/prearch.html
//   http://gcc.gnu.org/ml/gcc/2000-03/msg00749.html

#if !defined(CPU_SIZE)

// 64b architectures.
#if defined(__arch64__) || defined(__LP64__) || defined(_LP64) || defined(__64BIT__) \
    || defined(__x86_64__) || defined(__amd64__) || defined(_M_X64) \
    || defined(__ppc64__) || defined(__powerpc64__) \
    || defined(__IA64__) || defined(_M_IA64)
#  define CPU_SIZE  64

// 32b architectures (assumed for now that if the arch is known and not 64b, it's 32b).
#elif CPU_ARCH != CPU_ARCH_UNKNOWN
#  define CPU_SIZE  32

// Unknown.
#else
#  define CPU_SIZE  0
#  error "Unsupported CPU size detection.  Please fix."

#endif

#endif //CPU_SIZE


#endif //BASE_CPU_H
