/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/IO/LockedMemoryDevice.h>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN



UNNAMESPACE_END

//------------------------------------------------------------------------------
//!
LockedMemoryDevice::LockedMemoryDevice( char* data, size_t size, Mode mode ):
   IODevice( mode ),
   _data( data ),
   _size( size ),
   _pos( 0 )
{
}

//------------------------------------------------------------------------------
//!
LockedMemoryDevice::LockedMemoryDevice( const char* data, size_t size ):
   IODevice( IODevice::MODE_READ ),
   _data( const_cast<char*>(data) ),
   _size( size ),
   _pos( 0 )
{
}

//------------------------------------------------------------------------------
//!
void
LockedMemoryDevice::reset()
{
   _pos = 0;
}

//------------------------------------------------------------------------------
//!
LockedMemoryDevice::~LockedMemoryDevice()
{
}

//------------------------------------------------------------------------------
//!
void
LockedMemoryDevice::set( char* data, size_t size )
{
   _data = data;
   _size = size;
   size_t last = (_size == 0) ? 0 : _size-1;
   if( _pos >= size )  _pos = last;
}

//------------------------------------------------------------------------------
//!
bool
LockedMemoryDevice::doSeek( size_t pos )
{
   if( pos < _size )
   {
      _pos = pos;
      return true;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
size_t
LockedMemoryDevice::doPos() const
{
   return _pos;
}

//------------------------------------------------------------------------------
//!
size_t
LockedMemoryDevice::doRead( char* data, size_t n )
{
   size_t s = _size;
   size_t e = _pos + n;
   if( e >= s )
   {
      n -= (e - s);
   }
   memcpy( data, _data + _pos, n );
   _pos += n;
   return n;
}

//------------------------------------------------------------------------------
//!
size_t
LockedMemoryDevice::doPeek( char* data, size_t n )
{
   size_t s = _size;
   size_t e = _pos + n;
   if( e >= s )
   {
      n -= (e - s);
   }
   memcpy( data, _data + _pos, n );
   return n;
}

//------------------------------------------------------------------------------
//!
size_t
LockedMemoryDevice::doWrite( const char* data, size_t n )
{
   size_t s = _size;
   size_t e = _pos + n;
   if( e >= s )
   {
      n -= (e - s);
   }
   memcpy( _data + _pos, data, n );
   _pos += n;
   return n;
}
