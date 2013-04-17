/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/IO/MemoryDevice.h>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN



UNNAMESPACE_END

//------------------------------------------------------------------------------
//!
MemoryDevice::MemoryDevice( Mode mode, size_t s ):
   IODevice( mode ),
   _data( s ),
   _pos( 0 )
{
}

//------------------------------------------------------------------------------
//!
void
MemoryDevice::reset()
{
   _data.clear();
   _pos = 0;
}

//------------------------------------------------------------------------------
//!
MemoryDevice::~MemoryDevice()
{
}

//------------------------------------------------------------------------------
//!
bool
MemoryDevice::doSeek( size_t pos )
{
   if( pos < _data.size() )
   {
      _pos = pos;
      return true;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
size_t
MemoryDevice::doPos() const
{
   return _pos;
}

//------------------------------------------------------------------------------
//!
size_t
MemoryDevice::doRead( char* data, size_t n )
{
   size_t s = _data.size();
   size_t e = _pos + n;
   if( e >= s )
   {
      n -= (e - s);
   }
   memcpy( data, _data.data() + _pos, n );
   _pos += n;
   return n;
}

//------------------------------------------------------------------------------
//!
size_t
MemoryDevice::doPeek( char* data, size_t n )
{
   size_t s = _data.size();
   size_t e = _pos + n;
   if( e >= s )
   {
      n -= (e - s);
   }
   memcpy( data, _data.data() + _pos, n );
   return n;
}

//------------------------------------------------------------------------------
//!
size_t
MemoryDevice::doWrite( const char* data, size_t n )
{
   size_t s = _data.size();
   size_t e = _pos + n;
   if( e >= s )
   {
      _data.resize( e );
   }
   memcpy( _data.data() + _pos, data, n );
   _pos += n;
   return n;
}
