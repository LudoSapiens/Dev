/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_BINARY_STREAM_H
#define BASE_BINARY_STREAM_H

#include <Base/StdDefs.h>

#include <Base/ADT/String.h>
#include <Base/IO/IODevice.h>
#include <Base/Util/RCP.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS BinaryStream
==============================================================================*/

//! Wrapper stream class for text manipulation.

class BinaryStream
{
public:

   /*----- types -----*/
   enum Endian
   {
      ENDIAN_NATIVE,
      ENDIAN_LITTLE,
      ENDIAN_BIG
   };

   /*----- methods -----*/

   BASE_DLL_API  BinaryStream( IODevice* device, Endian endian = ENDIAN_NATIVE );
   BASE_DLL_API ~BinaryStream();

   inline IODevice*  device() const { return _device.ptr(); }
   inline void  device( IODevice* device ) { _device = device; }

   inline bool ok() const { return _device.isValid() && _device->ok(); }

   inline Endian  endian() const { return _endian; }
   BASE_DLL_API void  endian( Endian e );

   BASE_DLL_API bool  flush();

   // Input routines.
   inline size_t  read( char* data, size_t n ) { return _device->read(data, n); }
   BASE_DLL_API BinaryStream&  operator>>(   String&  str );
   //inline     BinaryStream&  operator>>(     char*  str );
   inline       BinaryStream&  operator>>(     char&  val );
   //inline     BinaryStream&  operator>>(    uchar&  val );
   //inline     BinaryStream&  operator>>(    short&  val );
   //inline     BinaryStream&  operator>>(   ushort&  val );
#if BASE_INT_NOT_INT32_T
   inline       BinaryStream&  operator>>(      int&  val );
   inline       BinaryStream&  operator>>(     uint&  val );
#endif
   inline       BinaryStream&  operator>>(    float&  val );
   inline       BinaryStream&  operator>>(   double&  val );
   inline       BinaryStream&  operator>>(   int8_t&  val );
   inline       BinaryStream&  operator>>(  uint8_t&  val );
   inline       BinaryStream&  operator>>(  int16_t&  val );
   inline       BinaryStream&  operator>>( uint16_t&  val );
   inline       BinaryStream&  operator>>(  int32_t&  val );
   inline       BinaryStream&  operator>>( uint32_t&  val );
   inline       BinaryStream&  operator>>(  int64_t&  val );
   inline       BinaryStream&  operator>>( uint64_t&  val );
#if defined(__APPLE__)
   inline       BinaryStream&  operator>>(   size_t&  val );
#endif

   // Output routines.
   inline size_t  write( const char* data, size_t n ) { return _device->write(data, n); }
   BASE_DLL_API BinaryStream&  operator<<( const  String&  str );
   BASE_DLL_API BinaryStream&  operator<<( const    char*  str );
   inline       BinaryStream&  operator<<( const     char  val );
   //inline     BinaryStream&  operator<<( const    uchar  val );
   //inline     BinaryStream&  operator<<( const    short  val );
   //inline     BinaryStream&  operator<<( const   ushort  val );
#if BASE_INT_NOT_INT32_T
   inline       BinaryStream&  operator<<( const      int  val );
   inline       BinaryStream&  operator<<( const     uint  val );
#endif
   inline       BinaryStream&  operator<<( const    float  val );
   inline       BinaryStream&  operator<<( const   double  val );
   inline       BinaryStream&  operator<<( const   int8_t  val );
   inline       BinaryStream&  operator<<( const  uint8_t  val );
   inline       BinaryStream&  operator<<( const  int16_t  val );
   inline       BinaryStream&  operator<<( const uint16_t  val );
   inline       BinaryStream&  operator<<( const  int32_t  val );
   inline       BinaryStream&  operator<<( const uint32_t  val );
   inline       BinaryStream&  operator<<( const  int64_t  val );
   inline       BinaryStream&  operator<<( const uint64_t  val );
#if defined(__APPLE__)
   inline       BinaryStream&  operator<<( const   size_t  val );
#endif

protected:

   /*----- types -----*/
   typedef  void  (*EndianFunc)( void* );

   /*----- members -----*/
   RCP<IODevice>  _device;
   Endian         _endian;
   EndianFunc     _swapRead16;
   EndianFunc     _swapRead32;
   EndianFunc     _swapRead64;
   EndianFunc     _swapWrite16;
   EndianFunc     _swapWrite32;
   EndianFunc     _swapWrite64;

private:
};

#define GEN_IN_OPER_1( type ) \
   inline BinaryStream& \
   BinaryStream::operator>>( type& val ) \
   { \
      char* ptr = (char*)&val; \
      read( ptr, sizeof(type) ); \
      return (*this); \
   }

#define GEN_IN_OPER_2( type, size ) \
   inline BinaryStream& \
   BinaryStream::operator>>( type& val ) \
   { \
      char* ptr = (char*)&val; \
      read( ptr, sizeof(type) ); \
      _swapRead##size( ptr ); \
      return (*this); \
   }

////------------------------------------------------------------------------------
////!
//BinaryStream&
//BinaryStream::operator>>( char* str )
//{
//   // TODO.
//   return *this;
//}

GEN_IN_OPER_1(     char     )
//GEN_IN_OPER_1(  uchar     )
//GEN_IN_OPER_2(  short, 16 )
//GEN_IN_OPER_2( ushort, 16 )
#if BASE_INT_NOT_INT32_T
GEN_IN_OPER_2(      int, 32 )
GEN_IN_OPER_2(     uint, 32 )
#endif
GEN_IN_OPER_2(    float, 32 )
GEN_IN_OPER_2(   double, 64 )
GEN_IN_OPER_1(   int8_t     )
GEN_IN_OPER_1(  uint8_t     )
GEN_IN_OPER_2(  int16_t, 16 )
GEN_IN_OPER_2( uint16_t, 16 )
GEN_IN_OPER_2(  int32_t, 32 )
GEN_IN_OPER_2( uint32_t, 32 )
GEN_IN_OPER_2(  int64_t, 64 )
GEN_IN_OPER_2( uint64_t, 64 )
#if defined(__APPLE__)
#if CPU_SIZE > 32
GEN_IN_OPER_2(   size_t, 64 )
#else
GEN_IN_OPER_2(   size_t, 32 )
#endif
#endif

#undef GEN_IN_OPER2
#undef GEN_IN_OPER1

// A simple typedef to allow stream manipulator functions.
typedef BinaryStream& (*BinaryStreamFunc)( BinaryStream& );

//------------------------------------------------------------------------------
//!
inline BinaryStream&
operator<<( BinaryStream& ts, BinaryStreamFunc func )
{
   return (*func)( ts );
}

//------------------------------------------------------------------------------
//!
inline BinaryStream&  flush( BinaryStream& os )
{
   os.flush();
   return os;
}

#define GEN_OUT_OPER_1( type ) \
   inline BinaryStream& \
   BinaryStream::operator<<( type val ) \
   { \
      char* ptr = (char*)&val; \
      write( ptr, sizeof(type) ); \
      return (*this); \
   }

#define GEN_OUT_OPER_2( type, size ) \
   inline BinaryStream& \
   BinaryStream::operator<<( type val ) \
   { \
      char* ptr = (char*)&val; \
      _swapWrite##size( ptr ); \
      write( ptr, sizeof(type) ); \
      return (*this); \
   }

GEN_OUT_OPER_1(     char     )
//GEN_OUT_OPER_1(  uchar     )
//GEN_OUT_OPER_2(  short, 16 )
//GEN_OUT_OPER_2( ushort, 16 )
#if BASE_INT_NOT_INT32_T
GEN_OUT_OPER_2(      int, 32 )
GEN_OUT_OPER_2(     uint, 32 )
#endif
GEN_OUT_OPER_2(    float, 32 )
GEN_OUT_OPER_2(   double, 64 )
GEN_OUT_OPER_1(   int8_t     )
GEN_OUT_OPER_1(  uint8_t     )
GEN_OUT_OPER_2(  int16_t, 16 )
GEN_OUT_OPER_2( uint16_t, 16 )
GEN_OUT_OPER_2(  int32_t, 32 )
GEN_OUT_OPER_2( uint32_t, 32 )
GEN_OUT_OPER_2(  int64_t, 64 )
GEN_OUT_OPER_2( uint64_t, 64 )
#if defined(__APPLE__)
#if CPU_SIZE > 32
GEN_OUT_OPER_2(   size_t, 64 )
#else
GEN_OUT_OPER_2(   size_t, 32 )
#endif
#endif

#undef GEN_OUT_OPER


NAMESPACE_END

#endif //BASE_BINARY_STREAM_H
