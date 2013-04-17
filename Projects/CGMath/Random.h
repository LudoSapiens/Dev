/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef CGMATH_RANDOM_H
#define CGMATH_RANDOM_H

#include <CGMath/StdDefs.h>

#include <cstdio>

NAMESPACE_BEGIN

namespace CGM
{

//------------------------------------------------------------------------------
//! Generates a series of uints using a linear congruential generator.
//! The generator used is the one popularized by Knuth in his Art of Computer
//! Programming, Vol.2 (which seems to come from Borosh and Niederreiter).
CGMATH_DLL_API void  randomLCG( const uint32_t seed, const uint32_t n, uint32_t* dst );

} // namespace CGM


/*==============================================================================
  CLASS RNG_MT
==============================================================================*/
//! A (pseudo)random number generator implementing the Mersenne Twister algorithm.
class RNG_MT
{
public:

   enum
   {
      MT_LEN       = 624,
      MT_IA        = 397,
      MT_IB        = MT_LEN - MT_IA,
      MT_MASK_SIGN = 0x80000000,
      MT_MASK_INT  = 0x7FFFFFFF,
      MT_MATRIX_A  = 0x9908B0DF
   };

   /*----- methods -----*/

   RNG_MT( const uint32_t* initState = NULL ) { setState(initState); }
   RNG_MT( const RNG_MT& mtg ) { *this = mtg; }

   //------------------------------------------------------------------------------
   //! Copies the current state.
   CGMATH_DLL_API void  setState( const uint32_t* srcState = NULL );

   //------------------------------------------------------------------------------
   //! Copies the current state.
   CGMATH_DLL_API void  getState( uint32_t* dstState ) const;

   //------------------------------------------------------------------------------
   //! Sets a 32b seed for reproduceable sequence.
   CGMATH_DLL_API void  seed( const uint32_t s );

   //------------------------------------------------------------------------------
   //! Returns a randomized uint, in range [0, 2^32).
   CGMATH_DLL_API uint32_t  getUInt();
   
   //------------------------------------------------------------------------------
   //! Returns a randomized uint, in range [0, n).
   inline uint32_t  getUInt( uint32_t n ) { return getUInt() % n; }

   //------------------------------------------------------------------------------
   //! Returns a randomized double, in range [0, 1).
   inline double  getDouble() { return getUInt() * (1.0/4294967296.0); }

   //------------------------------------------------------------------------------
   //! Returns a randomized double, in range [0, 1).
   inline double  operator()() { return getDouble(); }

   //------------------------------------------------------------------------------
   //! Generates a random number using the specified distribution.
   template< typename D >
   typename D::ReturnType operator()( const D& d ) { return d( *this ); }

   //------------------------------------------------------------------------------
   //! Generates a random number using the specified distribution with 1 parameter.
   template< typename D, typename P >
   typename D::ReturnType operator()( const D& d, const P& p ) { return d( *this, p ); }

protected:

   /*----- data members -----*/

   uint32_t  _state[MT_LEN]; //!< The current state.
   uint32_t  _index;         //!< The current index.

   uint32_t  twist( const uint32_t* b, const uint32_t i, const uint32_t j ) const
   {
      return (b[i] & MT_MASK_SIGN) | (b[j] & MT_MASK_INT);
   }

   uint32_t  magic( const uint32_t s ) const
   {
      return ((s)&1) * MT_MATRIX_A;
   }

private:
}; //class RNG_MT


/*==============================================================================
  CLASS RNG_WELL
==============================================================================*/
//! A (pseudo)random number generator implementing the WELL algorithm.
class RNG_WELL
{
public:

   enum
   {
      WELL_R    = 32,
      WELL_Rm1  = WELL_R-1,
      WELL_M1   = 3,
      WELL_M2   = 24,
      WELL_M3   = 10,
      WELL_MASK = 0x0000001F
   };

   /*----- methods -----*/

   RNG_WELL( const uint32_t* initState = NULL ) { setState(initState); }
   RNG_WELL( const RNG_WELL& mtg ) { *this = mtg; }

   //------------------------------------------------------------------------------
   //! Copies the current state.
   CGMATH_DLL_API void  setState( const uint32_t* srcState = NULL );

   //------------------------------------------------------------------------------
   //! Copies the current state.
   CGMATH_DLL_API void  getState( uint32_t* dstState ) const;

   //------------------------------------------------------------------------------
   //! Sets a 32b seed for reproduceable sequence.
   CGMATH_DLL_API void  seed( const uint32_t s );

   //------------------------------------------------------------------------------
   //! Returns a randomized uint, in range [0, 2^32).
   CGMATH_DLL_API uint32_t  getUInt();

   //------------------------------------------------------------------------------
   //! Returns a randomized uint, in range [0, n).
   inline uint32_t  getUInt( uint32_t n ) { return getUInt() % n; }
   
   //------------------------------------------------------------------------------
   //! Returns a randomized double, in range [0, 1).
   inline double  getDouble() { return getUInt() * (1.0/4294967296.0); }

   //------------------------------------------------------------------------------
   //! Returns a randomized double, in range [0, 1).
   inline double  operator()() { return getDouble(); }

   //------------------------------------------------------------------------------
   //! Generates a random number using the specified distribution.
   template< typename D >
   typename D::ReturnType operator()( const D& d ) { return d( *this ); }

   //------------------------------------------------------------------------------
   //! Generates a random number using the specified distribution with 1 parameter.
   template< typename D, typename P >
   typename D::ReturnType operator()( const D& d, const P& p ) { return d( *this, p ); }

protected:

   /*----- data members -----*/

   uint32_t  _state[WELL_R]; //!< The current state.
   uint32_t  _index;         //!< The current index.

   uint32_t  mat0pos( const int t, const uint32_t v ) const
   {
      return v ^ (v >> t);
   }

   uint32_t  mat0neg( const int t, const uint32_t v ) const
   {
      return v ^ (v << -t);
   }

   uint32_t  V0()   const { return _state[_index]; }
   uint32_t  VM1()  const { return _state[(_index+WELL_M1 ) & WELL_MASK]; }
   uint32_t  VM2()  const { return _state[(_index+WELL_M2 ) & WELL_MASK]; }
   uint32_t  VM3()  const { return _state[(_index+WELL_M3 ) & WELL_MASK]; }
   uint32_t  VRm1() const { return _state[(_index+WELL_Rm1) & WELL_MASK]; }
   uint32_t& newV0() { return _state[(_index+WELL_Rm1) & WELL_MASK]; }
   uint32_t& newV1() { return _state[_index]; }

private:
}; //class RNG_WELL


NAMESPACE_END

#endif //CGMATH_RANDOM_H
