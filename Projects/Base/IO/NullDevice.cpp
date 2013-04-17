/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/IO/NullDevice.h>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN



UNNAMESPACE_END

//------------------------------------------------------------------------------
//!
NullDevice::NullDevice( Mode mode ):
   IODevice( mode )
{
}

//------------------------------------------------------------------------------
//!
NullDevice::~NullDevice()
{
}

//------------------------------------------------------------------------------
//!
size_t
NullDevice::doRead( char* /*data*/, size_t n )
{
   return n;
}

//------------------------------------------------------------------------------
//!
size_t
NullDevice::doWrite( const char* /*data*/, size_t n )
{
   return n;
}
