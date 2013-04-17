/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <CGMath/Random.h>

#include <Base/Dbg/Defs.h>

#include <cstdlib>
#include <cstring>


/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN


UNNAMESPACE_END


NAMESPACE_BEGIN

namespace CGM
{

//------------------------------------------------------------------------------
//! Generates a series of uints using a linear congruential generator.
//! The generator used is the one popularized by Knuth in his Art of Computer
//! Programming, Vol.2 (which seems to come from Borosh and Niederreiter).
void randomLCG( const uint32_t seed, const uint32_t n, uint32_t* dst )
{
   // For the code below to work, a uint must be 32b.
   // (it requires a mod 2^32, which is implicitly happening here)
   CHECK( sizeof(seed) == 4 );

   //dst[0] = seed;
   dst[0] = 1812433253 * (seed ^ (seed >> 30));

   for( uint32_t i = 1; i < n; ++i )
   {
      uint32_t& prev = dst[i-1];
      dst[i] = 1812433253 * (prev ^ (prev >> 30)) + i;
   }
}

} //namespace CGM


/*==============================================================================
  CLASS RNG_MT
==============================================================================*/
//------------------------------------------------------------------------------
//! Copies the current state.
void
RNG_MT::setState( const uint32_t* srcState )
{
   if( srcState != NULL )
   {
      // Copy specified state.
      memcpy( _state, srcState, sizeof(_state) );
      _index = 0;  // Assume user set a previous state back.
   }
   else
   {
      // Initialize with various random numbers.
      seed( 0 );
   }
}

//------------------------------------------------------------------------------
//! Copies the current state.
void
RNG_MT::getState( uint32_t* dstState ) const
{
   memcpy( dstState, _state, sizeof(_state) );
}

//------------------------------------------------------------------------------
//! Sets a 32b seed for reproduceable sequence.
void
RNG_MT::seed( const uint32_t s )
{
   // Initialize with various random numbers.
   CGM::randomLCG( s, MT_LEN, _state );
   _index = MT_LEN;  // Make sure the twister works on next call.
}

//------------------------------------------------------------------------------
//!
//! Code is loosely based on the following:
//!   http://www.qbrundage.com/michaelb/pubs/essays/random_number_generation
//! which has been released into public domain.
uint32_t
RNG_MT::getUInt()
{
   // Generate all of the values in bursts.
   if( _index >= MT_LEN )
   {
      uint32_t s;
      uint32_t i = 0;
      // Generate first group with an offset of IA.
      for( ; i < MT_IB; ++i )
      {
         s = twist( _state, i, i+1 );
         _state[i] = _state[i + MT_IA] ^ (s >> 1) ^ magic(s);
      }
      // Generate second group with an offset of IA.
      for( ; i < MT_LEN-1; ++i )
      {
         s = twist( _state, i, i+1 );
         _state[i] = _state[i - MT_IB] ^ (s >> 2) ^ magic(s);
      }
      // Generate wrap-around case with an offset of IA.
      s = twist( _state, MT_LEN-1, 0 );
      _state[MT_LEN-1] = _state[MT_IA-1] ^ (s >> 1) ^ magic(s);
      // Reset index.
      _index = 0;
   }

   uint32_t v = _state[_index++];

   v ^= (v >> 11);
   v ^= (v <<  7) & 0x9D2C5680;
   v ^= (v << 15) & 0xEFC60000;
   v ^= (v >> 18);

   return v;
}


/*==============================================================================
  CLASS RNG_WELL
==============================================================================*/
//------------------------------------------------------------------------------
//! Copies the current state.
void
RNG_WELL::setState( const uint32_t* srcState )
{
   if( srcState != NULL )
   {
      // Copy specified state.
      memcpy( _state, srcState, sizeof(_state) );
      _index = 0;  // Assume user set a previous state back.
   }
   else
   {
      // Initialize with various random numbers.
      seed( 0 );
   }
}

//------------------------------------------------------------------------------
//! Copies the current state.
void
RNG_WELL::getState( uint32_t* dstState ) const
{
   memcpy( dstState, _state, sizeof(_state) );
}

//------------------------------------------------------------------------------
//! Sets a 32b seed for reproduceable sequence.
void
RNG_WELL::seed( const uint32_t s )
{
   // Initialize with various random numbers.
   CGM::randomLCG( s, WELL_R, _state );
   _index = 0;  // Start back at the beginning.
}

//------------------------------------------------------------------------------
//! Code is a personnal rewrite of what is provided in the original article.
uint32_t
RNG_WELL::getUInt()
{
   uint32_t z0 = VRm1();
   uint32_t z1 =               V0()    ^ mat0pos(   8, VM1() );
   uint32_t z2 = mat0neg( -19, VM2() ) ^ mat0neg( -14, VM3() );
   newV0()     = mat0neg( -11, z0 ) ^ mat0neg( -7, z1 ) ^ mat0neg( -13, z2 );
   newV1()     = z1 ^ z2;
   _index = (_index + WELL_Rm1) & WELL_MASK;
   //return _state[_index] * 2.32830643653869628906e-10;
   //                        2.3283064365386962890625e-10 //actual complete value
   return _state[_index];
}

NAMESPACE_END
