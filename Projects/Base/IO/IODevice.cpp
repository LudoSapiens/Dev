/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/IO/IODevice.h>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN



UNNAMESPACE_END

//------------------------------------------------------------------------------
//!
IODevice::IODevice( Mode mode ):
   _mode( mode ),
   _state( STATE_BAD )
{
}

//------------------------------------------------------------------------------
//!
IODevice::~IODevice()
{
   flush();
}

//------------------------------------------------------------------------------
//!
size_t
IODevice::getSize()
{
   // TODO.
   // If uncached, seek to the end, and read position.
   return INVALID_SIZE;
}

//------------------------------------------------------------------------------
//!
size_t
IODevice::read( char* data, size_t n )
{
   if( isReadable() )
   {
      return doRead( data, n );
   }

   return INVALID_SIZE;
}

//------------------------------------------------------------------------------
//!
bool
IODevice::readAll( String& dst )
{
   if( isReadable() )
   {
      size_t s = getSize();
      if( s != INVALID_SIZE )
      {
         dst.reserve( s );
      }
      char tmp[4096+1];
      tmp[4096] = '\0';
      while( true )
      {
         s = doRead( tmp, 4096 );
         if( s != 4096 )
         {
            if( s != INVALID_SIZE )  dst.append( tmp, s );
            break;
         }
         else
         {
            dst.append( tmp, s );
         }
      }
      return true;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
size_t
IODevice::write( const char* data, size_t n )
{
   if( isWritable() )
   {
      return doWrite( data, n );
   }

   return INVALID_SIZE;
}

//------------------------------------------------------------------------------
//!
bool
IODevice::doSeek( size_t /*pos*/ )
{
   return false;
}

//------------------------------------------------------------------------------
//!
size_t
IODevice::doPos() const
{
   return INVALID_SIZE;
}

//------------------------------------------------------------------------------
//!
size_t
IODevice::doPeek( char* /*data*/, size_t /*n*/ )
{
   return 0;
}

//------------------------------------------------------------------------------
//!
bool
IODevice::doFlush()
{
   return true;
}
