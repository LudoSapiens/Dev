/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_ATOMIC_H
#define BASE_ATOMIC_H

#include <Base/StdDefs.h>
#include <Base/Util/Compiler.h>
#include <Base/Util/CPU.h>

//#define BASE_ATOMIC_UNKNOWN       0
#define BASE_ATOMIC_GCC_BUILTINS    1
#define BASE_ATOMIC_WIN_INTRINSICS  2
#define BASE_ATOMIC_OSX_ATOMICS     3

// Auto-detection code.
#if !defined(BASE_ATOMIC)
#  if defined(__APPLE__)
#    define BASE_ATOMIC  BASE_ATOMIC_OSX_ATOMICS
#  elif defined(__GNUC__) && (__GNUC_VERSION__ >= 40100)
#    define BASE_ATOMIC  BASE_ATOMIC_GCC_BUILTINS
#  elif defined(_MSC_VER)
//#    define BASE_ATOMIC  BASE_ATOMIC_WIN_INTRINSICS
#    define BASE_ATOMIC  2
#  endif
#endif

// Required headers (if any).
#if BASE_ATOMIC == BASE_ATOMIC_GCC_BUILTINS
// Nothing to include.
#elif BASE_ATOMIC == BASE_ATOMIC_WIN_INTRINSICS
#  include <intrin.h>
#elif BASE_ATOMIC == BASE_ATOMIC_OSX_ATOMICS
#  include <libkern/OSAtomic.h>
#else
#  error Unsupported compiler: missing atomic routines.
#endif


NAMESPACE_BEGIN


/*==============================================================================
  Cross-platform atomic routines prototypes.
==============================================================================*/

//------------------------------------------------------------------------------
//! Increments var by 1, and returns the new value.
inline int32_t atomicInc( volatile int32_t& var );

//------------------------------------------------------------------------------
//! Decrements var by 1, and returns the new value.
inline int32_t atomicDec( volatile int32_t& var );

//------------------------------------------------------------------------------
//! Adds val to var, and returns the new value.
inline int32_t atomicAdd( volatile int32_t& var, int32_t val );

//------------------------------------------------------------------------------
//! Subtracts val to var, and returns the new value.
inline int32_t atomicSub( volatile int32_t& var, int32_t val );


//------------------------------------------------------------------------------
//! Applies an AND mask to var, and returns the new value.
inline int32_t atomicAND( volatile int32_t& var, int32_t mask );

//------------------------------------------------------------------------------
//! Applies an OR mask to var, and returns the new value.
inline int32_t atomicOR( volatile int32_t& var, int32_t mask );

//------------------------------------------------------------------------------
//! Applies an OR mask to var, and returns the new value.
inline int32_t atomicXOR( volatile int32_t& var, int32_t mask );

//------------------------------------------------------------------------------
//! Applies an NAND mask to var, and returns the new value.
inline int32_t atomicNAND( volatile int32_t& var, int32_t mask );


//------------------------------------------------------------------------------
//! Performs a compare-and-swap operation (i.e. try to replace var with newV but
//! only if it is oldV), and returns whether or not it succeeded.
inline bool atomicCAS( volatile int32_t& var, int32_t oldV, int32_t newV );

//------------------------------------------------------------------------------
//! Performs a compare-and-swap operation (i.e. try to replace a pointer ptr
//! with newV but only if it is oldV), and returns whether or not it succeeded.
inline bool atomicCAS( volatile void*& ptr, void* oldV, void* newV );


/*==============================================================================
  CLASS AtomicInt32
==============================================================================*/
// An atomic integer of 32b.
// Here are links that might be handy:
//   http://en.wikipedia.org/wiki/Compare-and-swap
//   http://en.wikipedia.org/wiki/ABA_problem
//   http://en.wikipedia.org/wiki/Memory_barrier
// The current implementation does not use barrier variants, but does return
// the modified value for the operations.
// This means that the following code:
//   AtomicInt32 ai32;
//   if( --ai32 == 0 )
//      ...
// is much better than:
//   AtomicInt32 ai32 = 2;
//   --ai32;
//   if( ai32 == 0 )
//      ...
// Also, double decrements are NOT valid:
//   --(--ai32);
// as operations don't return AtomitInt32&, but simply int32_t.
class AtomicInt32
{
public:

   /*----- methods -----*/

   inline AtomicInt32() { }
   inline AtomicInt32( int32_t v ): _value( v ) { }

   inline operator int32_t() const { return _value; }

   inline int32_t  operator++() { return atomicInc(_value); }
   inline int32_t  operator--() { return atomicDec(_value); }

   inline int32_t  operator+=( int32_t v ) { return atomicAdd(_value, v); }
   inline int32_t  operator-=( int32_t v ) { return atomicSub(_value, v); }

   inline int32_t  operator&=( int32_t m ) { return atomicAND (_value, m); }
   inline int32_t  operator|=( int32_t m ) { return atomicOR  (_value, m); }
   inline int32_t  operator^=( int32_t m ) { return atomicXOR (_value, m); }

   // Equivalent to oper~(), but since it modifies the bits, made it a clear function.
   // Otherwise:
   //   AtomicInt32 a(1);
   //   AtomicInt32 b = ~a;
   // will yield both 'a' and 'b' being flipped.
   inline int32_t  flip() { return atomicNAND(_value, _value); }

   inline int32_t  operator=( int32_t v );

   inline int32_t  set( int32_t v );

   inline bool  CAS( int32_t oldV, int32_t newV ) { return atomicCAS(_value, oldV, newV); }

protected:

   /*----- data members -----*/
   int32_t  _value;
}; //class AtomicInt32

//------------------------------------------------------------------------------
//! Assigns v atomically, and returns the updated value.
inline int32_t
AtomicInt32::operator=( int32_t v )
{
   int32_t old;
   do
   {
      old = _value;
   } while( !atomicCAS(_value, old, v) );
   return v;
}

//------------------------------------------------------------------------------
//! Assigns v atomically, and returns the previous value.
inline int32_t
AtomicInt32::set( int32_t v )
{
   int old;
   do
   {
      old = _value;
   } while( !atomicCAS(_value, old, v) );
   return old;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// GCC BUILT-INS
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#if BASE_ATOMIC == BASE_ATOMIC_GCC_BUILTINS

// Ref:
//    http://gcc.gnu.org/onlinedocs/gcc/Atomic-Builtins.html
//    http://gcc.gnu.org/onlinedocs/gcc-4.1.0/gcc/Atomic-Builtins.html

inline int32_t  atomicInc( volatile int32_t& var )
{
   return __sync_add_and_fetch( &var, 1 );
}

inline int32_t  atomicDec( volatile int32_t& var )
{
   return __sync_sub_and_fetch( &var, 1 );
}

inline int32_t  atomicAdd( volatile int32_t& var, int32_t val )
{
   return __sync_add_and_fetch( &var, val );
}

inline int32_t  atomicSub( volatile int32_t& var, int32_t val )
{
   return __sync_sub_and_fetch( &var, val );
}

inline int32_t  atomicAND( volatile int32_t& var, int32_t mask )
{
   return __sync_and_and_fetch( &var, mask );
}

inline int32_t  atomicOR( volatile int32_t& var, int32_t mask )
{
   return __sync_or_and_fetch( &var, mask );
}

inline int32_t  atomicXOR( volatile int32_t& var, int32_t mask )
{
   return __sync_xor_and_fetch( &var, mask );
}

inline int32_t  atomicNAND( volatile int32_t& var, int32_t mask )
{
   return __sync_nand_and_fetch( &var, mask );
}

inline bool  atomicCAS( volatile int32_t& var, int32_t oldV, int32_t newV )
{
   return __sync_bool_compare_and_swap( &var, oldV, newV );
}

inline bool  atomicCAS( volatile void*& var, void* oldV, void* newV )
{
   return __sync_bool_compare_and_swap( &var, oldV, newV );
}

#endif //BASE_ATOMIC == BASE_ATOMIC_GCC_BUILTINS


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// VISUAL STUDIO INTINSICS
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#if BASE_ATOMIC == BASE_ATOMIC_WIN_INTRINSICS

// Ref:
//    http://msdn.microsoft.com/en-us/library/5704bbxw.aspx
//    http://msdn.microsoft.com/en-us/library/191ca0sk.aspx
//    http://msdn.microsoft.com/en-us/library/ms686360.aspx
// Also:
//    http://msdn.microsoft.com/en-us/library/bb310595.aspx

inline int32_t  atomicInc( volatile int32_t& var )
{
   return _InterlockedIncrement( (volatile long*)&var );
}

inline int32_t  atomicDec( volatile int32_t& var )
{
   return _InterlockedDecrement( (volatile long*)&var );
}

inline int32_t  atomicAdd( volatile int32_t& var, int32_t val )
{
   // Only _InterlockedExchangeAdd is on all platforms, and it returns the previous value.
   return _InterlockedExchangeAdd( (volatile long*)&var, val ) + val;
}

inline int32_t  atomicSub( volatile int32_t& var, int32_t val )
{
   // Only _InterlockedExchangeAdd is on all platforms, and it returns the previous value.
   return _InterlockedExchangeAdd( (volatile long*)&var, -val ) - val;
}

inline int32_t  atomicAND( volatile int32_t& var, int32_t mask )
{
   // The intrinsic routine returns the previous value.
   return _InterlockedAnd( (volatile long*)&var, mask ) & mask;
}

inline int32_t  atomicOR( volatile int32_t& var, int32_t mask )
{
   // The intrinsic routine returns the previous value.
   return _InterlockedOr( (volatile long*)&var, mask ) | mask;
}

inline int32_t  atomicXOR( volatile int32_t& var, int32_t mask )
{
   // The intrinsic routine returns the previous value.
   return _InterlockedXor( (volatile long*)&var, mask ) ^ mask;
}

inline int32_t  atomicNAND( volatile int32_t& var, int32_t mask )
{
   while( true )
   {
      int32_t res = ~(var & mask);
      // Should we use the barrier version for this?
      if( atomicCAS(var, var, res) )
      {
         return res;
      }
   }
}

inline bool  atomicCAS( volatile int32_t& var, int32_t oldV, int32_t newV )
{
   return _InterlockedCompareExchange( (volatile long*)&var, newV, oldV ) == oldV;
}

inline bool  atomicCAS( void* volatile& ptr, void* oldV, void* newV )
{
// Seems like _InterlockedCompareExchangePointer isn't defined as noted in:
//   http://msdn.microsoft.com/en-us/library/1b4s3xf5.aspx
// Sounds like a bug was file:
//   http://social.msdn.microsoft.com/Forums/fr-FR/vcgeneral/thread/6e6dacd5-545d-4c21-a9cf-2ef18a58b6f3
// So maybe we'll be able to use the proper routine in an upcoming version.
#if CPU_SIZE == 32
# define CL(v)  reinterpret_cast<long&>(v)
   return _InterlockedCompareExchange( reinterpret_cast<volatile long*>(&ptr), CL(newV), CL(oldV) ) == CL(oldV);
#undef CL
#else
   return _InterlockedCompareExchangePointer( &ptr, newV, oldV ) == oldV;
#endif
}

#endif //BASE_ATOMIC == BASE_ATOMIC_WIN_INTRINSICS


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// OSX ATOMICS
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#if BASE_ATOMIC == BASE_ATOMIC_OSX_ATOMICS

// Ref:
//   http://developer.apple.com/documentation/Darwin/Reference/KernelIOKitFramework/OSAtomic_h/index.html
// Found this header through the following podcast:
//   http://www.mac-developer-network.com/shows/podcasts/lnc/lnc032/

inline int32_t  atomicInc( volatile int32_t& var )
{
   return OSAtomicIncrement32( &var );
}

inline int32_t  atomicDec( volatile int32_t& var )
{
   return OSAtomicDecrement32( &var );
}

inline int32_t  atomicAdd( volatile int32_t& var, int32_t val )
{
   return OSAtomicAdd32( val, &var );
}

inline int32_t  atomicSub( volatile int32_t& var, int32_t val )
{
   return OSAtomicAdd32( -val, &var );
}

inline int32_t  atomicAND( volatile int32_t& var, int32_t mask )
{
   return OSAtomicAnd32( mask, (volatile uint32_t*)&var );
}

inline int32_t  atomicOR( volatile int32_t& var, int32_t mask )
{
   return OSAtomicOr32( mask, (volatile uint32_t*)&var );
}

inline int32_t  atomicXOR( volatile int32_t& var, int32_t mask )
{
   return OSAtomicXor32( mask, (volatile uint32_t*)&var );
}

inline int32_t  atomicNAND( volatile int32_t& var, int32_t mask )
{
   while( true )
   {
      int32_t res = ~(var & mask);
      // Should we use the barrier version for this?
      if( atomicCAS(var, var, res) )
      {
         return res;
      }
   }
}

inline bool  atomicCAS( volatile int32_t& var, int32_t oldV, int32_t newV )
{
   return OSAtomicCompareAndSwap32( oldV, newV, &var );
}

inline bool  atomicCAS( void* volatile& var, void* oldV, void* newV )
{
   return OSAtomicCompareAndSwapPtr( oldV, newV, &var );
}

#endif //BASE_ATOMIC == BASE_ATOMIC_OSX_ATOMICS


NAMESPACE_END


#endif //BASE_ATOMIC_H
