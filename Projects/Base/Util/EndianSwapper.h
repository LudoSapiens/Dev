/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_ENDIAN_SWAPPER_H
#define BASE_ENDIAN_SWAPPER_H

#include <Base/StdDefs.h>
#include <Base/Util/CPU.h>

NAMESPACE_BEGIN

namespace EndianSwapper
{

   /*----- static methods -----*/

   static inline void swap8( uint8_t& a, uint8_t& b );
   static inline void swap16( uint16_t& a, uint16_t& b );
   static inline void swap32( uint32_t& a, uint32_t& b );
   static inline void swap64( uint64_t& a, uint64_t& b );

   static inline void byteSwap16( void* ptr );
   static inline void byteSwap32( void* ptr );
   static inline void byteSwap64( void* ptr );

   static inline void bigToLittle_16b( void* ptr );
   static inline void bigToLittle_32b( void* ptr );
   static inline void bigToLittle_64b( void* ptr );

   static inline void littleToBig_16b( void* ptr );
   static inline void littleToBig_32b( void* ptr );
   static inline void littleToBig_64b( void* ptr );

   static inline void nativeToLittle_16b( void* ptr );
   static inline void nativeToLittle_32b( void* ptr );
   static inline void nativeToLittle_64b( void* ptr );

   static inline void nativeToBig_16b( void* ptr );
   static inline void nativeToBig_32b( void* ptr );
   static inline void nativeToBig_64b( void* ptr );

   static inline void littleToNative_16b( void* ptr );
   static inline void littleToNative_32b( void* ptr );
   static inline void littleToNative_64b( void* ptr );

   static inline void bigToNative_16b( void* ptr );
   static inline void bigToNative_32b( void* ptr );
   static inline void bigToNative_64b( void* ptr );

   inline uint16_t get16BE( uint8_t byte0, uint8_t byte1 );
   inline uint16_t get16BE( const uint8_t* bytes );
   inline uint16_t get16BE( const char* bytes );

   inline uint32_t get32BE( uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3 );
   inline uint32_t get32BE( const uint8_t* bytes );
   inline uint32_t get32BE( const char* bytes );
   inline uint32_t get32BE( uint16_t word0, uint16_t word1 );
   inline uint32_t get32BE( const uint16_t* words );

   inline uint64_t get64BE( uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3,
                            uint8_t byte4, uint8_t byte5, uint8_t byte6, uint8_t byte7 );
   inline uint64_t get64BE( const uint8_t* bytes );
   inline uint64_t get64BE( const char* bytes );
   inline uint64_t get64BE( uint16_t word0, uint16_t word1, uint16_t word2, uint16_t word3 );
   inline uint64_t get64BE( const uint16_t* words );
   inline uint64_t get64BE( uint32_t dword0, uint32_t dword1 );
   inline uint64_t get64BE( const uint32_t* dwords );

   inline uint16_t get16LE( uint8_t byte0, uint8_t byte1 );
   inline uint16_t get16LE( const uint8_t* bytes );
   inline uint16_t get16LE( const char* bytes );

   inline uint32_t get32LE( uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3 );
   inline uint32_t get32LE( const uint8_t* bytes );
   inline uint32_t get32LE( const char* bytes );
   inline uint32_t get32LE( uint16_t word0, uint16_t word1 );
   inline uint32_t get32LE( const uint16_t* words );

   inline uint64_t get64LE( uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3,
                            uint8_t byte4, uint8_t byte5, uint8_t byte6, uint8_t byte7 );
   inline uint64_t get64LE( const uint8_t* bytes );
   inline uint64_t get64LE( const char* bytes );
   inline uint64_t get64LE( uint16_t word0, uint16_t word1, uint16_t word2, uint16_t word3 );
   inline uint64_t get64LE( const uint16_t* words );
   inline uint64_t get64LE( uint32_t dword0, uint32_t dword1 );
   inline uint64_t get64LE( const uint32_t* dwords );


}; //namespace EndianSwapper

/**
 * Swaps 2 bytes with one another
 */
inline void
EndianSwapper::swap8
( uint8_t& a, uint8_t& b )
{
    uint8_t tmp = a;
    a = b;
    b = tmp;
}

/**
 * Swaps 2 16b chunks with one another
 */
inline void
EndianSwapper::swap16
( uint16_t& a, uint16_t& b )
{
    uint16_t tmp = a;
    a = b;
    b = tmp;
}

/**
 * Swaps 2 32b chunks with one another
 */
inline void
EndianSwapper::swap32
( uint32_t& a, uint32_t& b )
{
    uint32_t tmp = a;
    a = b;
    b = tmp;
}

/**
 * Swaps 2 64b chunks with one another
 */
inline void
EndianSwapper::swap64
( uint64_t& a, uint64_t& b )
{
    uint64_t tmp = a;
    a = b;
    b = tmp;
}


/**
 * Does in-place swapping of bytes, regardless of platform.
 */
inline void
EndianSwapper::byteSwap16
( void* ptrV )
{
   uint8_t* ptr = (uint8_t*)ptrV;
   swap8( ptr[0], ptr[1] );
}


/**
 * Does in-place swapping of bytes, regardless of platform.
 */
inline void
EndianSwapper::byteSwap32
( void* ptr )
{
   uint8_t* ptr8 = (uint8_t*)ptr;
   swap8( ptr8[0], ptr8[3] );
   swap8( ptr8[1], ptr8[2] );
}


/**
 * Does in-place swapping of bytes, regardless of platform.
 */
inline void
EndianSwapper::byteSwap64
( void* ptr )
{
   uint32_t* ptr32 = (uint32_t*)ptr;
   byteSwap32(ptr32);
   byteSwap32(ptr32+1);
   swap32( ptr32[0], ptr32[1] );
}


/**
 * Does in-place swapping of bytes, regardless of platform.
 */
inline void
EndianSwapper::bigToLittle_16b
( void* ptr )
{
   byteSwap16(ptr);
}


/**
 * Does in-place swapping of bytes, regardless of platform.
 */
inline void
EndianSwapper::bigToLittle_32b
( void* ptr )
{
   byteSwap32(ptr);
}


/**
 * Does in-place swapping of bytes, regardless of platform.
 */
inline void
EndianSwapper::bigToLittle_64b
( void* ptr )
{
   byteSwap64(ptr);
}


/**
 * Does in-place swapping of bytes, regardless of platform.
 */
inline void
EndianSwapper::littleToBig_16b
( void* ptr )
{
   byteSwap16(ptr);
}


/**
 * Does in-place swapping of bytes, regardless of platform.
 */
inline void
EndianSwapper::littleToBig_32b
( void* ptr )
{
   byteSwap32(ptr);
}


/**
 * Does in-place swapping of bytes, regardless of platform.
 */
inline void
EndianSwapper::littleToBig_64b
( void* ptr )
{
   byteSwap64(ptr);
}


/**
 * Does in-place endianness byte-swap, from the platform's native to little.
 */
inline void
EndianSwapper::nativeToLittle_16b
( void* ptr )
{
#if (CPU_ENDIANNESS == CPU_ENDIAN_BIG)
   bigToLittle_16b(ptr);
#else
   unused(ptr);
#endif
}


/**
 * Does in-place endianness byte-swap, from the platform's native to little.
 */
inline void
EndianSwapper::nativeToLittle_32b
( void* ptr )
{
#if (CPU_ENDIANNESS == CPU_ENDIAN_BIG)
   bigToLittle_32b(ptr);
#else
   unused(ptr);
#endif
}


/**
 * Does in-place endianness byte-swap, from the platform's native to little.
 */
inline void
EndianSwapper::nativeToLittle_64b
( void* ptr )
{
#if (CPU_ENDIANNESS == CPU_ENDIAN_BIG)
   bigToLittle_64b(ptr);
#else
   unused(ptr);
#endif
}


/**
 * Does in-place endianness byte-swap, from the platform's native to big.
 */
inline void
EndianSwapper::nativeToBig_16b
( void* ptr )
{
#if (CPU_ENDIANNESS == CPU_ENDIAN_LITTLE)
   littleToBig_16b(ptr);
#endif
}


/**
 * Does in-place endianness byte-swap, from the platform's native to big.
 */
inline void
EndianSwapper::nativeToBig_32b
( void* ptr )
{
#if (CPU_ENDIANNESS == CPU_ENDIAN_LITTLE)
   littleToBig_32b(ptr);
#endif
}


/**
 * Does in-place endianness byte-swap, from the platform's native to big.
 */
inline void
EndianSwapper::nativeToBig_64b
( void* ptr )
{
#if (CPU_ENDIANNESS == CPU_ENDIAN_LITTLE)
   littleToBig_64b(ptr);
#endif
}


/**
 * Does in-place endianness byte-swap, from the platform's little to native.
 */
inline void
EndianSwapper::littleToNative_16b
( void* ptr )
{
#if (CPU_ENDIANNESS == CPU_ENDIAN_BIG)
   littleToBig_16b(ptr);
#else
   unused(ptr);
#endif
}


/**
 * Does in-place endianness byte-swap, from the platform's little to native.
 */
inline void
EndianSwapper::littleToNative_32b
( void* ptr )
{
#if (CPU_ENDIANNESS == CPU_ENDIAN_BIG)
   littleToBig_32b(ptr);
#else
   unused(ptr);
#endif
}


/**
 * Does in-place endianness byte-swap, from the platform's little to native.
 */
inline void
EndianSwapper::littleToNative_64b
( void* ptr )
{
#if (CPU_ENDIANNESS == CPU_ENDIAN_BIG)
   littleToBig_64b(ptr);
#else
   unused(ptr);
#endif
}


/**
 * Does in-place endianness byte-swap, from the platform's big to native.
 */
inline void
EndianSwapper::bigToNative_16b
( void* ptr )
{
#if (CPU_ENDIANNESS == CPU_ENDIAN_LITTLE)
   bigToLittle_16b(ptr);
#else
   unused(ptr);
#endif
}


/**
 * Does in-place endianness byte-swap, from the platform's big to native.
 */
inline void
EndianSwapper::bigToNative_32b
( void* ptr )
{
#if (CPU_ENDIANNESS == CPU_ENDIAN_LITTLE)
   bigToLittle_32b(ptr);
#else
   unused(ptr);
#endif
}


/**
 * Does in-place endianness byte-swap, from the platform's big to native.
 */
inline void
EndianSwapper::bigToNative_64b
( void* ptr )
{
#if (CPU_ENDIANNESS == CPU_ENDIAN_LITTLE)
   bigToLittle_64b(ptr);
#else
   unused(ptr);
#endif
}

namespace EndianSwapper
{

//-----------------------------------------------------------------------------
//!
inline uint16_t get16BE( uint8_t byte0, uint8_t byte1 )
{
   uint16_t tmp;
#if (CPU_ENDIANNESS == CPU_ENDIAN_LITTLE)
   // Need to swap.
   tmp = byte0;
   tmp <<= 8;
   tmp |= byte1;
#else
   tmp = byte1;
   tmp <<= 8;
   tmp |= byte0;
#endif
   return tmp;
}

//-----------------------------------------------------------------------------
//!
inline uint16_t get16BE( const uint8_t* bytes )
{
   return get16BE( bytes[0], bytes[1] );
}

//-----------------------------------------------------------------------------
//!
inline uint16_t get16BE( const char* bytes )
{
   return get16BE( (const uint8_t*)bytes );
}

//-----------------------------------------------------------------------------
//!
inline uint32_t get32BE( uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3 )
{
   uint32_t tmp;
#if (CPU_ENDIANNESS == CPU_ENDIAN_LITTLE)
   // Need to swap.
   tmp = byte0;
   tmp <<= 8;
   tmp |= byte1;
   tmp <<= 8;
   tmp |= byte2;
   tmp <<= 8;
   tmp |= byte3;
#else
   tmp = byte3;
   tmp <<= 8;
   tmp |= byte2;
   tmp <<= 8;
   tmp |= byte1;
   tmp <<= 8;
   tmp |= byte0;
#endif
   return tmp;
}

//-----------------------------------------------------------------------------
//!
inline uint32_t get32BE( const uint8_t* bytes )
{
   return get32BE( bytes[0], bytes[1], bytes[2], bytes[3] );
}

//-----------------------------------------------------------------------------
//!
inline uint32_t get32BE( const char* bytes )
{
   return get32BE( (const uint8_t*)bytes );
}

//-----------------------------------------------------------------------------
//!
inline uint32_t get32BE( uint16_t word0, uint16_t word1 )
{
   uint32_t tmp;
#if (CPU_ENDIANNESS == CPU_ENDIAN_LITTLE)
   // Need to swap.
   tmp = word0;
   tmp <<= 16;
   tmp |= word1;
#else
   tmp = word1;
   tmp <<= 16;
   tmp |= word0;
#endif
   return tmp;
}

//-----------------------------------------------------------------------------
//!
inline uint32_t get32BE( const uint16_t* words )
{
   return get32BE( words[0], words[1] );
}

//-----------------------------------------------------------------------------
//!
inline uint64_t get64BE( uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3,
                         uint8_t byte4, uint8_t byte5, uint8_t byte6, uint8_t byte7 )
{
   uint64_t tmp;
#if (CPU_ENDIANNESS == CPU_ENDIAN_LITTLE)
   // Need to swap.
   tmp = byte0;
   tmp <<= 8;
   tmp |= byte1;
   tmp <<= 8;
   tmp |= byte2;
   tmp <<= 8;
   tmp |= byte3;
   tmp <<= 8;
   tmp |= byte4;
   tmp <<= 8;
   tmp |= byte5;
   tmp <<= 8;
   tmp |= byte6;
   tmp <<= 8;
   tmp |= byte7;
#else
   tmp = byte7;
   tmp <<= 8;
   tmp |= byte6;
   tmp <<= 8;
   tmp |= byte5;
   tmp <<= 8;
   tmp |= byte4;
   tmp <<= 8;
   tmp |= byte3;
   tmp <<= 8;
   tmp |= byte2;
   tmp <<= 8;
   tmp |= byte1;
   tmp <<= 8;
   tmp |= byte0;
#endif
   return tmp;
}

//-----------------------------------------------------------------------------
//!
inline uint64_t get64BE( const uint8_t* bytes )
{
   return get64BE( bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5], bytes[6], bytes[7] );
}

//-----------------------------------------------------------------------------
//!
inline uint64_t get64BE( const char* bytes )
{
   return get64BE( (const uint8_t*)bytes );
}

//-----------------------------------------------------------------------------
//!
inline uint64_t get64BE( uint16_t word0, uint16_t word1, uint16_t word2, uint16_t word3 )
{
   uint64_t tmp;
#if (CPU_ENDIANNESS == CPU_ENDIAN_LITTLE)
   // Need to swap.
   tmp = word0;
   tmp <<= 16;
   tmp |= word1;
   tmp <<= 16;
   tmp |= word2;
   tmp <<= 16;
   tmp |= word3;
#else
   tmp = word3;
   tmp <<= 16;
   tmp |= word2;
   tmp <<= 16;
   tmp |= word1;
   tmp <<= 16;
   tmp |= word0;
#endif
   return tmp;
}

//-----------------------------------------------------------------------------
//!
inline uint64_t get64BE( const uint16_t* words )
{
   return get64BE( words[0], words[1], words[2], words[3] );
}

//-----------------------------------------------------------------------------
//!
inline uint64_t get64BE( uint32_t dword0, uint32_t dword1 )
{
   uint64_t tmp;
#if (CPU_ENDIANNESS == CPU_ENDIAN_LITTLE)
   // Need to swap.
   tmp = dword0;
   tmp <<= 32;
   tmp |= dword1;
#else
   tmp = dword1;
   tmp <<= 32;
   tmp |= dword0;
#endif
   return tmp;
}

//-----------------------------------------------------------------------------
//!
inline uint64_t get64BE( const uint32_t* dwords )
{
   return get64BE( dwords[0], dwords[1] );
}

//-----------------------------------------------------------------------------
//!
inline uint16_t get16LE( uint8_t byte0, uint8_t byte1 )
{
   uint16_t tmp;
#if (CPU_ENDIANNESS == CPU_ENDIAN_BIG)
   // Need to swap.
   tmp = byte0;
   tmp <<= 8;
   tmp |= byte1;
#else
   tmp = byte1;
   tmp <<= 8;
   tmp |= byte0;
#endif
   return tmp;
}

//-----------------------------------------------------------------------------
//!
inline uint16_t get16LE( const uint8_t* bytes )
{
   return get16LE( bytes[0], bytes[1] );
}

//-----------------------------------------------------------------------------
//!
inline uint16_t get16LE( const char* bytes )
{
   return get16LE( (const uint8_t*)bytes );
}

//-----------------------------------------------------------------------------
//!
inline uint32_t get32LE( uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3 )
{
   uint32_t tmp;
#if (CPU_ENDIANNESS == CPU_ENDIAN_BIG)
   // Need to swap.
   tmp = byte0;
   tmp <<= 8;
   tmp |= byte1;
   tmp <<= 8;
   tmp |= byte2;
   tmp <<= 8;
   tmp |= byte3;
#else
   tmp = byte3;
   tmp <<= 8;
   tmp |= byte2;
   tmp <<= 8;
   tmp |= byte1;
   tmp <<= 8;
   tmp |= byte0;
#endif
   return tmp;
}

//-----------------------------------------------------------------------------
//!
inline uint32_t get32LE( const uint8_t* bytes )
{
   return get32LE( bytes[0], bytes[1], bytes[2], bytes[3] );
}

//-----------------------------------------------------------------------------
//!
inline uint32_t get32LE( const char* bytes )
{
   return get32LE( (const uint8_t*)bytes );
}

//-----------------------------------------------------------------------------
//!
inline uint32_t get32LE( uint16_t word0, uint16_t word1 )
{
   uint32_t tmp;
#if (CPU_ENDIANNESS == CPU_ENDIAN_BIG)
   // Need to swap.
   tmp = word0;
   tmp <<= 16;
   tmp |= word1;
#else
   tmp = word1;
   tmp <<= 16;
   tmp |= word0;
#endif
   return tmp;
}

//-----------------------------------------------------------------------------
//!
inline uint32_t get32LE( const uint16_t* words )
{
   return get32LE( words[0], words[1] );
}

//-----------------------------------------------------------------------------
//!
inline uint64_t get64LE( uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3,
                         uint8_t byte4, uint8_t byte5, uint8_t byte6, uint8_t byte7 )
{
   uint64_t tmp;
#if (CPU_ENDIANNESS == CPU_ENDIAN_BIG)
   // Need to swap.
   tmp = byte0;
   tmp <<= 8;
   tmp |= byte1;
   tmp <<= 8;
   tmp |= byte2;
   tmp <<= 8;
   tmp |= byte3;
   tmp <<= 8;
   tmp |= byte4;
   tmp <<= 8;
   tmp |= byte5;
   tmp <<= 8;
   tmp |= byte6;
   tmp <<= 8;
   tmp |= byte7;
#else
   tmp = byte7;
   tmp <<= 8;
   tmp |= byte6;
   tmp <<= 8;
   tmp |= byte5;
   tmp <<= 8;
   tmp |= byte4;
   tmp <<= 8;
   tmp |= byte3;
   tmp <<= 8;
   tmp |= byte2;
   tmp <<= 8;
   tmp |= byte1;
   tmp <<= 8;
   tmp |= byte0;
#endif
   return tmp;
}

//-----------------------------------------------------------------------------
//!
inline uint64_t get64LE( const uint8_t* bytes )
{
   return get64LE( bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5], bytes[6], bytes[7] );
}

//-----------------------------------------------------------------------------
//!
inline uint64_t get64LE( const char* bytes )
{
   return get64LE( (const uint8_t*)bytes );
}

//-----------------------------------------------------------------------------
//!
inline uint64_t get64LE( uint16_t word0, uint16_t word1, uint16_t word2, uint16_t word3 )
{
   uint64_t tmp;
#if (CPU_ENDIANNESS == CPU_ENDIAN_BIG)
   // Need to swap.
   tmp = word0;
   tmp <<= 16;
   tmp |= word1;
   tmp <<= 16;
   tmp |= word2;
   tmp <<= 16;
   tmp |= word3;
#else
   tmp = word3;
   tmp <<= 16;
   tmp |= word2;
   tmp <<= 16;
   tmp |= word1;
   tmp <<= 16;
   tmp |= word0;
#endif
   return tmp;
}

//-----------------------------------------------------------------------------
//!
inline uint64_t get64LE( const uint16_t* words )
{
   return get64LE( words[0], words[1], words[2], words[3] );
}

//-----------------------------------------------------------------------------
//!
inline uint64_t get64LE( uint32_t dword0, uint32_t dword1 )
{
   uint64_t tmp;
#if (CPU_ENDIANNESS == CPU_ENDIAN_BIG)
   // Need to swap.
   tmp = dword0;
   tmp <<= 32;
   tmp |= dword1;
#else
   tmp = dword1;
   tmp <<= 32;
   tmp |= dword0;
#endif
   return tmp;
}

//-----------------------------------------------------------------------------
//!
inline uint64_t get64LE( const uint32_t* dwords )
{
   return get64LE( dwords[0], dwords[1] );
}

} // namespace EndianSwapper

NAMESPACE_END

#endif //BASE_ENDIAN_SWAPPER_H
