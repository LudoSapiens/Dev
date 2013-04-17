/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Snd/Listener.h>

#include <Snd/Manager.h>

USING_NAMESPACE

using namespace Snd;


UNNAMESPACE_BEGIN

UNNAMESPACE_END


/*==============================================================================
  CLASS Listener
==============================================================================*/

//------------------------------------------------------------------------------
//!
Listener::Listener( Manager* mgr ):
   _mgr( mgr ),
   _id( INVALID_ID )
{
   setDefaults();
}

//------------------------------------------------------------------------------
//!
Listener::~Listener
( void )
{
   terminate();
}

//------------------------------------------------------------------------------
//!
void
Listener::terminate()
{
   if( _id != INVALID_ID )
   {
      _mgr->release( this );
   }
}

//------------------------------------------------------------------------------
//!
void
Listener::setDefaults
( void )
{
   gain( 1.0f );
   //pitch( 1.0f );
   position( Vec3f::zero() );
   velocity( Vec3f::zero() );
   pointOfInterest( Vec3f(0.0f, 0.0f, -1.0f) );
   upVector( Vec3f(0.0f, 1.0f, 0.0f) );
}
