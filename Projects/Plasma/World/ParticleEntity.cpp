/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/World/ParticleEntity.h>

#include <Plasma/Geometry/ParticleGeometry.h>
#include <Plasma/Particle/ParticleAnimator.h>
#include <Plasma/Particle/ParticleGenerator.h>

USING_NAMESPACE


/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//! Determines the data requirements of a ParticleData based on a generator
//! and an animator.
//! All of the data required by the animator must have been generated by
//! the generator.
inline ParticleData::DataType
getRequirements( ParticleGenerator* gen, ParticleAnimator* ani )
{
   CHECK( gen );
   CHECK( ani );
   ParticleData::DataType gTypes = gen->generates();
   ParticleData::DataType aTypes = ani->animates();
   if( (aTypes & gTypes) != aTypes )
   {
      StdErr << "ERROR: Animator works on non-generated data: " << String().format("0x%02x & 0x%02x = 0x%02x.", gTypes, aTypes, (gTypes&aTypes)) << nl;
   }
   return gTypes | aTypes;
}

UNNAMESPACE_END


/*==============================================================================
  CLASS ParticleEntity
==============================================================================*/

//------------------------------------------------------------------------------
//!
ParticleEntity::ParticleEntity():
   Entity( PARTICLE ),
   _state( NOTHING )
{
   isRelative(true);
}

//------------------------------------------------------------------------------
//!
ParticleEntity::~ParticleEntity()
{
}

//------------------------------------------------------------------------------
//!
void
ParticleEntity::generator( ParticleGenerator* g )
{
   _generator = g;
}

//------------------------------------------------------------------------------
//!
void
ParticleEntity::animator( ParticleAnimator* a )
{
   _animator = a;
}

//------------------------------------------------------------------------------
//!
void
ParticleEntity::init( uint32_t capacity )
{
   if( _generator.isValid() && _animator.isValid() )
   {
      _data.allocate( getRequirements(_generator.ptr(), _animator.ptr()), capacity );
      geometry( new ParticleGeometry(this) );
      if( brain() == nullptr )  brain( new Brain() );
      brain()->actions().pushBack( new ParticleAction() );
   }
   else
   {
      const char* firstMissing = _generator.isNull() ? "generator" : "animator";
      StdErr << "ERROR: Initilializing a ParticleEntity without a " << firstMissing << "." << nl;
   }
}

//------------------------------------------------------------------------------
//!
void
ParticleEntity::simulate( float delta, ParticleRNG& rng )
{
   CHECK( _generator.isValid() && _animator.isValid() );
   if( !_generator->done() )  _generator->generate( delta, *this, rng );
   _animator->animate( delta, *this, rng );
}