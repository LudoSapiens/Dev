/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Motion/Attractor/BasicAttractors.h>
#include <Motion/World/RigidBody.h>


NAMESPACE_BEGIN

/*==============================================================================
   CLASS DirectionalAttractor
==============================================================================*/

//------------------------------------------------------------------------------
//!
DirectionalAttractor::DirectionalAttractor() :
   Attractor(),
   _acceleration( Vec3f::zero() )
{
}

//------------------------------------------------------------------------------
//!
DirectionalAttractor::DirectionalAttractor( const Vec3f& acceleration ) :
   Attractor(),
   _acceleration( acceleration )
{
}

//------------------------------------------------------------------------------
//!
DirectionalAttractor::~DirectionalAttractor()
{
}

//------------------------------------------------------------------------------
//!
void 
DirectionalAttractor::addForce( const Vector< RCP<RigidBody> >& bodies )
{
   for( uint b = 0; b < bodies.size(); ++b )
   {
      if( attracts( bodies[b] ) )
      {
         bodies[b]->addForce( _acceleration * bodies[b]->mass() );
      }
   }
}

/*==============================================================================
   CLASS PointAttractor
==============================================================================*/

//------------------------------------------------------------------------------
//!
PointAttractor::PointAttractor()  :
   Attractor()
{
}

//------------------------------------------------------------------------------
//!
PointAttractor::~PointAttractor()
{
}

//------------------------------------------------------------------------------
//!
void 
PointAttractor::addForce( const Vector< RCP<RigidBody> >& )
{
   // TODO
}

/*==============================================================================
   CLASS BeamAttractor
==============================================================================*/

//------------------------------------------------------------------------------
//!
BeamAttractor::BeamAttractor() :
   Attractor()
{
}

//------------------------------------------------------------------------------
//!
BeamAttractor::~BeamAttractor()
{
}

//------------------------------------------------------------------------------
//!
void 
BeamAttractor::addForce( const Vector< RCP<RigidBody> >& )
{
   // TODO
}

NAMESPACE_END
