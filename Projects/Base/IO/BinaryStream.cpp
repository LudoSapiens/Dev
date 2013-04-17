/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/IO/BinaryStream.h>

#include <Base/Dbg/Defs.h>
#include <Base/IO/FileDevice.h>
#include <Base/IO/NullDevice.h>
#include <Base/Util/EndianSwapper.h>


// Set to 1 if you want the prefixed string size to be store in the
// stream's specified endianness (as opposed the always little endian if 0).
#define STRING_SIZE_ENDIAN_MATCH  1


/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

inline void nothing( void* )
{
}

UNNAMESPACE_END


NAMESPACE_BEGIN

typedef union
{
   char      _char[4];

   int8_t    _int8_t[4];
   int16_t   _int16_t[2];
   int32_t   _int32_t[1];

   uint8_t   _uint8_t[4];
   uint16_t  _uint16_t[2];
   uint32_t  _uint32_t[1];

   float     _float[1];
} StdTypes32;

typedef union
{
   char      _char[8];

   int8_t    _int8_t[8];
   int16_t   _int16_t[4];
   int32_t   _int32_t[2];
   int64_t   _int64_t[1];

   uint8_t   _uint8_t[8];
   uint16_t  _uint16_t[4];
   uint32_t  _uint32_t[2];
   uint64_t  _uint64_t[1];

   float     _float[2];
   double    _double[1];
} StdTypes64;

NAMESPACE_END

//------------------------------------------------------------------------------
//!
BinaryStream::BinaryStream( IODevice* device, Endian e ):
   _device( device )
{
   endian( e );
}

//------------------------------------------------------------------------------
//!
BinaryStream::~BinaryStream()
{
}

//------------------------------------------------------------------------------
//!
bool
BinaryStream::flush()
{
   if( _device.isValid() )
   {
      return _device->flush();
   }
   return true;
}

//------------------------------------------------------------------------------
//!
void
BinaryStream::endian( Endian e )
{
   _endian = e;
   switch( _endian )
   {
      case ENDIAN_NATIVE:
         _swapRead16  = &nothing;
         _swapRead32  = &nothing;
         _swapRead64  = &nothing;
         _swapWrite16 = &nothing;
         _swapWrite32 = &nothing;
         _swapWrite64 = &nothing;
         break;
      case ENDIAN_LITTLE:
         _swapRead16  = &EndianSwapper::littleToNative_16b;
         _swapRead32  = &EndianSwapper::littleToNative_32b;
         _swapRead64  = &EndianSwapper::littleToNative_64b;
         _swapWrite16 = &EndianSwapper::nativeToLittle_16b;
         _swapWrite32 = &EndianSwapper::nativeToLittle_32b;
         _swapWrite64 = &EndianSwapper::nativeToLittle_64b;
         break;
      case ENDIAN_BIG:
         _swapRead16  = &EndianSwapper::bigToNative_16b;
         _swapRead32  = &EndianSwapper::bigToNative_32b;
         _swapRead64  = &EndianSwapper::bigToNative_64b;
         _swapWrite16 = &EndianSwapper::nativeToBig_16b;
         _swapWrite32 = &EndianSwapper::nativeToBig_32b;
         _swapWrite64 = &EndianSwapper::nativeToBig_64b;
         break;
      default:
         CHECK( false );
         _swapRead16  = NULL;
         _swapRead32  = NULL;
         _swapRead64  = NULL;
         _swapWrite16 = NULL;
         _swapWrite32 = NULL;
         _swapWrite64 = NULL;
         break;
   }
}

//------------------------------------------------------------------------------
//!
BinaryStream&
BinaryStream::operator>>( String& str )
{
   uint32_t n;
#if STRING_SIZE_ENDIAN_MATCH
   (*this) >> n;
#else
   size_t s = read( (char*)&n, sizeof(n) );
   CHECK( s == sizeof(n) );
   EndianSwapper::littleToNative_32b( &n );
#endif
   char* tmp = new char[n];
   size_t m = read( tmp, n );
   CHECK( m == n );
   str.clear();
   str.append( tmp, m );
   delete [] tmp;
   return *this;
}

//------------------------------------------------------------------------------
//! First dumps a 32b size, then dumps the characters, without terminating '\0'.
BinaryStream&
BinaryStream::operator<<( const String& str )
{
   uint32_t n = (uint32_t)str.size();
#if STRING_SIZE_ENDIAN_MATCH
   (*this) << n;
#else
   EndianSwapper::nativeToLittle_32b( &n );
   size_t s = write( (char*)&n, sizeof(n) );
   CHECK( s == sizeof(n) );
#endif
   write( str.cstr(), n );
   return *this;
}

//------------------------------------------------------------------------------
//! First dumps a 32b size, then dumps the characters, without terminating '\0'.
BinaryStream&
BinaryStream::operator<<( const char* str )
{
   uint32_t n = (uint32_t)strlen( str );
#if STRING_SIZE_ENDIAN_MATCH
   (*this) << n;
#else
   EndianSwapper::nativeToLittle_32b( &n );
   size_t s = write( (char*)&n, sizeof(n) );
   CHECK( s == sizeof(n) );
#endif
   write( str, n );
   return *this;
}
