/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Snd/Source.h>

#include <Snd/Manager.h>

USING_NAMESPACE

using namespace Snd;


UNNAMESPACE_BEGIN

UNNAMESPACE_END


/*==============================================================================
  CLASS Source
==============================================================================*/

//------------------------------------------------------------------------------
//!
Source::Source( Manager* mgr ):
   _mgr( mgr ),
   _id( INVALID_ID ),
   _params( 0x0 )
{
   setDefaults();
}

//------------------------------------------------------------------------------
//!
Source::~Source
( void )
{
   terminate();
}

//------------------------------------------------------------------------------
//!
void
Source::terminate()
{
   if( _id != INVALID_ID )
   {
      _mgr->stop( this );
      _mgr->bind( this, NULL );
      _mgr->release( this );
   }
}

//------------------------------------------------------------------------------
//!
void
Source::setDefaults
( void )
{
   pitch( 1.0f );
   gain( 1.0f );
   position( Vec3f::zero() );
   velocity( Vec3f::zero() );
   looping( false );
   relative( false );
}

//------------------------------------------------------------------------------
//!
void
Source::setBGM
( void )
{
   pitch( 1.0f );
   gain( 1.0f );
   position( Vec3f::zero() );
   velocity( Vec3f::zero() );
   looping( false );
   relative( true );
}
