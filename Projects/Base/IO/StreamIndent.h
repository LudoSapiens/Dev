/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_STREAM_INDENT_H
#define BASE_STREAM_INDENT_H

#include <Base/StdDefs.h>

#include <Base/IO/TextStream.h>

NAMESPACE_BEGIN


/*==============================================================================
  CLASS StreamIndent
==============================================================================*/
class StreamIndent
{
public:

   /*----- methods -----*/

   StreamIndent( const uint spacing = 3, const uint maxIndent = 20 ):
      _buffer( NULL )
   {
      allocateBuffer( NULL, spacing, maxIndent );
   }

   StreamIndent( const char* prefix, const uint spacing = 3, const uint maxIndent = 20 ):
      _buffer( NULL )
   {
      allocateBuffer( prefix, spacing, maxIndent );
   }

   StreamIndent&  increase( const uint spacing )
   {
      *_curEnd = ' ';
      _curEnd += spacing;
      *_curEnd = '\0';
      return *this;
   }

   StreamIndent&  decrease( const uint spacing )
   {
      *_curEnd = ' ';
      _curEnd -= spacing;
      *_curEnd = '\0';
      return *this;
   }

   StreamIndent&  operator++() { return increase(_sizeSpacing); }

   StreamIndent&  operator--() { return decrease(_sizeSpacing); }

   StreamIndent&  operator+=( const uint n ) { return increase(n*_sizeSpacing); }

   StreamIndent&  operator-=( const uint n ) { return decrease(n*_sizeSpacing); }

   operator const char* () const { return _buffer; }

   TextStream&  operator<<( TextStream& os ) const
   {
      return os << _buffer;
   }

protected:

   /*----- data members -----*/

   char*  _buffer;
   char*  _curEnd;
   uint   _sizePrefix;
   uint   _sizeSpacing;

   /*----- methods -----*/

   void  deallocateBuffer()
   {
      delete [] _buffer;
      _buffer = NULL;
   }

   void  allocateBuffer( const char* prefix, const uint spacing, const uint maxIndent )
   {
      // 1. Deallocate any previously-allocated buffer (if any).
      deallocateBuffer();

      // 2. Calculate required size.
      _sizePrefix  = (prefix != NULL) ? (uint)strlen(prefix) : 0;
      _sizeSpacing = spacing;
      uint sizeTotal = _sizePrefix + _sizeSpacing*maxIndent + 1; // Add trailing '\0'.

      // 3. Allocate the buffer.
      _buffer = new char[sizeTotal];

      // 4. Fill in the buffer.
      if( prefix != NULL )
      {
         strcpy( _buffer, prefix );
      }
      for( uint i = _sizePrefix; i < sizeTotal; ++i )
      {
         _buffer[i] = ' ';
      }

      // 5. Initialize state.
      _curEnd = _buffer + _sizePrefix;
      *_curEnd = '\0';
   }

}; //class StreamIndent


NAMESPACE_END

#endif //BASE_STREAM_INDENT_H
