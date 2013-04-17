/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/IO/MultiDevice.h>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN



UNNAMESPACE_END

//------------------------------------------------------------------------------
//!
MultiDevice::MultiDevice()
{
}

//------------------------------------------------------------------------------
//!
MultiDevice::~MultiDevice()
{
}

//------------------------------------------------------------------------------
//!
bool
MultiDevice::doSeek( size_t pos )
{
   bool ok = true;
   for( Iterator cur = _devices.begin(), end = _devices.end();
        cur != end;
        ++cur )
   {
      ok &= (*cur)->seek( pos );
   }
   return ok;
}

//------------------------------------------------------------------------------
//!
size_t
MultiDevice::doRead( char* /*data*/, size_t /*n*/ )
{
   // Doesn't make sense; we have multiple read sources.
   return 0;
}

//------------------------------------------------------------------------------
//!
size_t
MultiDevice::doWrite( const char* data, size_t n )
{
   size_t ret = INVALID_SIZE;
   for( Iterator cur = _devices.begin(), end = _devices.end();
        cur != end;
        ++cur )
   {
      ret = (*cur)->write( data, n );
   }
   return ret;
}

//------------------------------------------------------------------------------
//!
bool
MultiDevice::doFlush()
{
   bool ok = true;
   for( Iterator cur = _devices.begin(), end = _devices.end();
        cur != end;
        ++cur )
   {
      ok &= (*cur)->flush();
   }
   return ok;
}
