/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_TEXT_STREAM_H
#define BASE_TEXT_STREAM_H

#include <Base/StdDefs.h>

#include <Base/ADT/ConstString.h>
#include <Base/ADT/String.h>
#include <Base/IO/IODevice.h>
#include <Base/Util/RCP.h>
#include <Base/Util/CPU.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS TextStream
==============================================================================*/

//! Wrapper stream class for text manipulation.

class TextStream
{
public:

   /*==============================================================================
     CLASS LineIterator
   ==============================================================================*/
   class LineIterator
   {
   public:

      /*----- methods -----*/

      inline LineIterator(): _ts( NULL ), _buf( NULL ) { }
      inline LineIterator( TextStream* ts, const size_t bufSize = 256, const char delim = '\n' ):
         _ts( ts ), _bufSize(bufSize), _bufPos(bufSize), _delim( delim )
      {
         _buf = new char[bufSize+1];
         _buf[_bufSize] = _delim; // Put sentinel.
         operator++();
      }
      inline ~LineIterator() { delete [] _buf; }
      inline LineIterator( const LineIterator& iter ) { *this = iter; }

      inline bool isValid() const { return (_ts != NULL); }

      inline operator const String&() const { return _line; }
      inline const String&  operator*() { return _line; }
      inline const String*  operator->() { return &_line; }

      BASE_DLL_API LineIterator&  operator++();

      inline LineIterator&  operator=( const LineIterator& iter )
      {
         _ts            = iter._ts;
         _line          = iter._line;
         _bufSize       = iter._bufSize;
         _bufPos        = iter._bufPos;
         _delim         = iter._delim;
         delete [] _buf;
         _buf           = new char[_bufSize+1];
         _buf[_bufSize] = _delim;
         memcpy( _buf, iter._buf, _bufSize );
         return *this;
      }

   protected:

      /*----- data members -----*/

      TextStream*  _ts;
      String       _line;
      size_t       _bufSize;
      size_t       _bufPos;
      char*        _buf;
      char         _delim;

      /*----- methods -----*/
      inline void  readChunk()
      {
      }
      inline void  seekEOL()
      {
         while( _buf[_bufPos] != _delim )
         {
            ++_bufPos;
         }
      }

   private:
   }; //class LineIterator
   

   /*----- methods -----*/

   BASE_DLL_API  TextStream( IODevice* device );
   BASE_DLL_API  TextStream( String& str );
   BASE_DLL_API ~TextStream();

   inline IODevice*  device() const { return _device.ptr(); }
   inline void  device( IODevice* device ) { _device = device; }

   inline bool  ok() const { return (_device != NULL) && _device->ok(); }

   BASE_DLL_API bool  flush();

   // Iterator.
   inline LineIterator  lines( const size_t bufSize = 256, const char delim = '\n' ) { return LineIterator( this, bufSize, delim ); }

   // Input routines.
   BASE_DLL_API TextStream&  operator>>( String&  str );
   //BASE_DLL_API TextStream&  operator>>( char*  str );
   BASE_DLL_API TextStream&  operator>>(   char&  val );
   BASE_DLL_API TextStream&  operator>>(  uchar&  val );
   BASE_DLL_API TextStream&  operator>>(  short&  val );
   BASE_DLL_API TextStream&  operator>>( ushort&  val );
   BASE_DLL_API TextStream&  operator>>(    int&  val );
   BASE_DLL_API TextStream&  operator>>(   uint&  val );
   //BASE_DLL_API TextStream&  operator>>( size_t&  val );
   BASE_DLL_API TextStream&  operator>>(  float&  val );
   BASE_DLL_API TextStream&  operator>>( double&  val );
   BASE_DLL_API TextStream&  operator>>(  void*&  ptr );
   BASE_DLL_API TextStream&  readAll( String& dst );

   // Output routines.
   inline TextStream&  operator<<( const      String&  str );
   inline TextStream&  operator<<( const ConstString&  str );
   inline TextStream&  operator<<( const        char*  str );
   inline TextStream&  operator<<( const         char  val );
   //inline TextStream&  operator<<( const      uchar  val );
   //inline TextStream&  operator<<( const      short  val );
   //inline TextStream&  operator<<( const     ushort  val );
#if BASE_INT_NOT_INT32_T
   inline TextStream&  operator<<( const          int  val );
   inline TextStream&  operator<<( const         uint  val );
#endif
   inline TextStream&  operator<<( const        float  val );
   inline TextStream&  operator<<( const       double  val );
   inline TextStream&  operator<<( const       int8_t  val );
   inline TextStream&  operator<<( const      uint8_t  val );
   inline TextStream&  operator<<( const      int16_t  val );
   inline TextStream&  operator<<( const     uint16_t  val );
   inline TextStream&  operator<<( const      int32_t  val );
   inline TextStream&  operator<<( const     uint32_t  val );
   inline TextStream&  operator<<( const      int64_t  val );
   inline TextStream&  operator<<( const     uint64_t  val );
#if defined(__APPLE__)
   inline TextStream&  operator<<( const       size_t  val );
#  if CPU_SIZE != 32
   inline TextStream&  operator<<( const    ptrdiff_t  val );
#  endif 
#endif
   inline TextStream&  operator<<( const        void*  ptr );

protected:

   RCP<IODevice>  _device;

private:
};

// A simple typedef to allow stream manipulator functions.
typedef TextStream& (*TextStreamFunc)( TextStream& );

//------------------------------------------------------------------------------
//!
inline TextStream&
operator<<( TextStream& ts, TextStreamFunc func )
{
   return (*func)( ts );
}

//------------------------------------------------------------------------------
//!
inline TextStream&  flush( TextStream& os )
{
   os.flush();
   return os;
}

//------------------------------------------------------------------------------
//!
inline TextStream&  endl( TextStream& os )
{
   return os << '\n' << flush;
}

//------------------------------------------------------------------------------
//!
inline TextStream&  nl( TextStream& os )
{
   return os << '\n' << flush;
}

//------------------------------------------------------------------------------
//!
inline TextStream&
TextStream::operator<<( const String& str )
{
   _device->write( str.cstr(), str.size() );
   return *this;
}

//------------------------------------------------------------------------------
//!
inline TextStream&
TextStream::operator<<( const ConstString& str )
{
   _device->write( str.cstr(), str.size() );
   return *this;
}

//------------------------------------------------------------------------------
//!
inline TextStream&
TextStream::operator<<( const char* str )
{
   //return (*this) << String( str );
   _device->write( str, strlen(str) );
   return *this;
}

#define GEN_IN_OPER( type ) \
   inline TextStream& \
   TextStream::operator<<( type val ) \
   { \
      return (*this) << String( val ); \
   }

GEN_IN_OPER( const      char )
//GEN_IN_OPER( const   uchar )
//GEN_IN_OPER( const   short )
//GEN_IN_OPER( const  ushort )
#if BASE_INT_NOT_INT32_T
GEN_IN_OPER( const       int )
GEN_IN_OPER( const      uint )
#endif
GEN_IN_OPER( const     float )
GEN_IN_OPER( const    double )
GEN_IN_OPER( const    int8_t )
GEN_IN_OPER( const   uint8_t )
GEN_IN_OPER( const   int16_t )
GEN_IN_OPER( const  uint16_t )
GEN_IN_OPER( const   int32_t )
GEN_IN_OPER( const  uint32_t )
GEN_IN_OPER( const   int64_t )
GEN_IN_OPER( const  uint64_t )
#if defined(__APPLE__)
GEN_IN_OPER( const    size_t )
# if CPU_SIZE != 32
GEN_IN_OPER( const ptrdiff_t )
# endif
#endif
GEN_IN_OPER( const void* )

#undef GEN_IN_OPER

//------------------------------------------------------------------------------
//!
extern BASE_DLL_API TextStream  StdErr;
extern BASE_DLL_API TextStream  StdIn;
extern BASE_DLL_API TextStream  StdOut;
extern BASE_DLL_API TextStream  StdNull;

NAMESPACE_END

#endif //BASE_TEXT_STREAM_H
