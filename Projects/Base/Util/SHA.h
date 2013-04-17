/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_SHA_H
#define BASE_SHA_H

#include <Base/StdDefs.h>

#include <Base/ADT/String.h>
#include <Base/Dbg/Defs.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS SHA1
==============================================================================*/
//!< Implements SHA-1 as documented in:
//!<   http://csrc.nist.gov/publications/fips/fips180-2/fips180-2withchangenotice.pdf
//!< Also provides some utility routines to send message in parts.
class SHA1
{
public:

   /*----- types -----*/

   enum
   {
      // Result hash of an empty file.
      EMPTY_H_0 = 0xDA39A3EE,
      EMPTY_H_1 = 0x5E6B4B0D,
      EMPTY_H_2 = 0x3255BFEF,
      EMPTY_H_3 = 0x95601890,
      EMPTY_H_4 = 0xAFD80709,

      // Initial hash value.
      H_0 = 0x67452301,
      H_1 = 0xEFCDAB89,
      H_2 = 0x98BADCFE,
      H_3 = 0x10325476,
      H_4 = 0xC3D2E1F0,

      // Constants used during execution.
      K_00_19 = 0x5A827999,
      K_20_39 = 0x6ED9EBA1,
      K_40_59 = 0x8F1BBCDC,
      K_60_79 = 0xCA62C1D6
   };

   /*==============================================================================
     CLASS Digest
   ==============================================================================*/
   class Digest
   {
   public:

      /*----- methods -----*/

      inline Digest();
      inline Digest( const uint32_t* data );
      inline Digest( uint32_t h0, uint32_t h1, uint32_t h2, uint32_t h3, uint32_t h4 );

      inline void  reset();

      inline String  str() const;

      inline operator       uint32_t*()       { return _H; }
      inline operator const uint32_t*() const { return _H; }

      inline bool operator==( const Digest& d ) const;
      inline bool operator< ( const Digest& d ) const;
      inline bool operator<=( const Digest& d ) const { return (*this) < d || (*this) == d; }
      inline bool operator> ( const Digest& d ) const { return !((*this) <= d); }
      inline bool operator>=( const Digest& d ) const { return !((*this) <  d); }

   protected:
      /*----- data members -----*/
      friend class SHA1;
      uint32_t  _H[5];   //!< The words of the hash.
   }; //class Digest


   /*----- static functions -----*/

   static inline uint32_t Ch( uint32_t x, uint32_t y, uint32_t z );
   static inline uint32_t Parity( uint32_t x, uint32_t y, uint32_t z );
   static inline uint32_t Maj( uint32_t x, uint32_t y, uint32_t z );
   static inline uint32_t ROTL( uint32_t n, uint32_t word );

   /*----- methods -----*/

   BASE_DLL_API SHA1();

   // High-level API.
   inline       void  put( const char* str );
   inline       void  put( const char* str, size_t n );
   BASE_DLL_API void  put( const uint8_t* data, size_t n );

   BASE_DLL_API void  put( uint8_t  v );
   BASE_DLL_API void  put( uint16_t v );
   BASE_DLL_API void  put( uint32_t v );
   BASE_DLL_API void  put( uint64_t v );
   BASE_DLL_API void  put( float    v );
   BASE_DLL_API void  put( double   v );
#if defined(__APPLE__)
   inline       void  put( size_t   v );
#endif
   inline       void  put( void*    v );

   inline const Digest&  digest() const { return _digest; }

   BASE_DLL_API void  begin();
   BASE_DLL_API const Digest&  end();

protected:

   /*----- data types -----*/

   typedef size_t  SizeType;

   /*----- data members -----*/

   uint32_t  _W[80];  //!< The 80 words of the message schedule (first 16 are the current message block).
   uint32_t  _a;      //!< The first working variable.
   uint32_t  _b;      //!< The second working variable.
   uint32_t  _c;      //!< The third working variable.
   uint32_t  _d;      //!< The fourth working variable.
   uint32_t  _e;      //!< The fifth and last working variable.
   Digest    _digest; //!< The words of the hash.

   uint32_t  _numBlocks;  //!< The total number of 512b blocks that were run.
   SizeType  _curSizeB;   //!< Number of valid bytes in the current block (_W[0] to _W[15]).

   /*----- methods -----*/

   inline void  checkBlock();
   void  initBlock();
   void  runBlock();

private:
}; //class SHA1

//------------------------------------------------------------------------------
//!
inline uint32_t
SHA1::Ch( uint32_t x, uint32_t y, uint32_t z )
{
   return (x & y) ^ (~x & z);
}

//------------------------------------------------------------------------------
//!
inline uint32_t
SHA1::Parity( uint32_t x, uint32_t y, uint32_t z )
{
   return x ^ y ^ z;
}

//------------------------------------------------------------------------------
//!
inline uint32_t
SHA1::Maj( uint32_t x, uint32_t y, uint32_t z )
{
   return (x & y) ^ (x & z) ^ (y & z);
}

//------------------------------------------------------------------------------
//!
inline uint32_t
SHA1::ROTL( uint32_t n, uint32_t word )
{
   return (word << n) | (word >> (32-n));
}

//------------------------------------------------------------------------------
//!
inline void
SHA1::put( const char* str )
{
   put( (const uint8_t*)str, strlen(str) );
}

//------------------------------------------------------------------------------
//!
inline void
SHA1::put( const char* str, size_t s )
{
   put( (const uint8_t*)str, s );
}

//------------------------------------------------------------------------------
//!
inline void
SHA1::checkBlock()
{
   CHECK( _curSizeB <= 64 );
   if( _curSizeB == 64 )
   {
      runBlock();
      initBlock();
   }
}

//------------------------------------------------------------------------------
//!
inline
SHA1::Digest::Digest()
{
   // Initialize with the digest of an empty file.
   _H[0] = EMPTY_H_0;
   _H[1] = EMPTY_H_1;
   _H[2] = EMPTY_H_2;
   _H[3] = EMPTY_H_3;
   _H[4] = EMPTY_H_4;
}

//------------------------------------------------------------------------------
//!
inline
SHA1::Digest::Digest( const uint32_t* data )
{
   _H[0] = data[0];
   _H[1] = data[1];
   _H[2] = data[2];
   _H[3] = data[3];
   _H[4] = data[4];
}

//------------------------------------------------------------------------------
//!
inline
SHA1::Digest::Digest( uint32_t h0, uint32_t h1, uint32_t h2, uint32_t h3, uint32_t h4 )
{
   _H[0] = h0;
   _H[1] = h1;
   _H[2] = h2;
   _H[3] = h3;
   _H[4] = h4;
}

//------------------------------------------------------------------------------
//!
inline void
SHA1::Digest::reset()
{
   _H[0] = H_0;
   _H[1] = H_1;
   _H[2] = H_2;
   _H[3] = H_3;
   _H[4] = H_4;
}

//------------------------------------------------------------------------------
//!
inline String
SHA1::Digest::str() const
{
   return String().format( "%08x%08x%08x%08x%08x", _H[0], _H[1], _H[2], _H[3], _H[4] );
}

//------------------------------------------------------------------------------
//!
inline bool
SHA1::Digest::operator==( const Digest& d ) const
{
   return (_H[0] == d._H[0]) &&
          (_H[1] == d._H[1]) &&
          (_H[2] == d._H[2]) &&
          (_H[3] == d._H[3]) &&
          (_H[4] == d._H[4]);
}

//------------------------------------------------------------------------------
//!
inline bool
SHA1::Digest::operator<( const Digest& d ) const
{
   if( _H[0] != d._H[0] )  return _H[0] < d._H[0];
   if( _H[1] != d._H[1] )  return _H[1] < d._H[1];
   if( _H[2] != d._H[2] )  return _H[2] < d._H[2];
   if( _H[3] != d._H[3] )  return _H[3] < d._H[3];
                           return _H[4] < d._H[4];
}

#if defined(__APPLE__)
#  if CPU_SIZE == 32
inline void  SHA1::put( size_t   v ) { put( (uint32_t)v ); }
#  elif CPU_SIZE == 64
inline void  SHA1::put( size_t   v ) { put( (uint64_t)v ); }
#  else
#    error TODO
#  endif
#endif

#if CPU_SIZE == 32
inline void  SHA1::put( void*    v ) { put( (uint32_t)v ); }
#elif CPU_SIZE == 64
inline void  SHA1::put( void*    v ) { put( (uint64_t)v ); }
#else
#error TODO
#endif


NAMESPACE_END

#endif //BASE_SHA_H
