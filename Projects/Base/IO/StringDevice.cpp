/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/IO/StringDevice.h>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN



UNNAMESPACE_END

//------------------------------------------------------------------------------
//!
StringDevice::StringDevice( String& str, Mode mode ):
   IODevice( mode ),
   _string( &str ),
   _pos( 0 )
{
   reset();
}

//------------------------------------------------------------------------------
//!
StringDevice::~StringDevice()
{
}

//------------------------------------------------------------------------------
//!
void
StringDevice::reset()
{
   _string->clear();
   _pos = 0;
}

//------------------------------------------------------------------------------
//!
bool
StringDevice::doSeek( size_t pos )
{
   if( pos < _string->size() )
   {
      _pos = pos;
      return true;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
size_t
StringDevice::doPos() const
{
   return _pos;
}

//------------------------------------------------------------------------------
//!
size_t
StringDevice::doRead( char* data, size_t n )
{
   String::SizeType s = _string->size();
   String::SizeType e = _pos + n;
   if( e >= s )
   {
      n -= (e - s);
   }
   memcpy( data, _string->cstr() + _pos, n );
   _pos += n;
   return n;
}

//------------------------------------------------------------------------------
//!
size_t
StringDevice::doPeek( char* data, size_t n )
{
   String::SizeType s = _string->size();
   String::SizeType e = _pos + n;
   if( e >= s )
   {
      n -= (e - s);
   }
   memcpy( data, _string->cstr() + _pos, n );
   return n;
}

//------------------------------------------------------------------------------
//!
size_t
StringDevice::doWrite( const char* data, size_t n )
{
   size_t s = _string->size();
   size_t e = _pos + n;
   if( e >= s )
   {
      _string->erase( _pos );     // Drop data until the end.
      _string->append( data, n ); // Append new stuff.
   }
   else
   {
      // Note: There doesn't seem to be any efficient replace( pos, size, data, size ).
      _string->erase( _pos, n );        // Drop data inside the string.
      _string->insert( _pos, data, n ); // Insert new stuff.
   }
   _pos += n;
   return n;
}
