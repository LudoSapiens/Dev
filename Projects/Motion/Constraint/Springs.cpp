/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Motion/Constraint/Springs.h>

#include <Motion/World/RigidBody.h>

#include <Base/Dbg/DebugStream.h>


USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_spr, "Springs" );

UNNAMESPACE_END

/*==============================================================================
   CLASS PosSpring
==============================================================================*/

//------------------------------------------------------------------------------
//!
PosSpring::PosSpring( RigidBody* bodyA, RigidBody* bodyB
) :
   _bodyA( bodyA ), _bodyB( bodyB ),
   _stiffness( 100.0f ), _damping( 1.0f ), _angVelDamp( 0.999f )
{
}

//------------------------------------------------------------------------------
//!
PosSpring::~PosSpring()
{
}

//------------------------------------------------------------------------------
//!
void
PosSpring::anchor( const Vec3f& posA, const Vec3f& posB, float restLength )
{
   _restLength = restLength;

   // Compute anchor position into local body coordinates system.
   _anchorA = _bodyA->simReferential().globalToLocal() * posA;
   _anchorB = _bodyB->simReferential().globalToLocal() * posB;
}

//------------------------------------------------------------------------------
//!
void
PosSpring::anchor( const Vec3f& posA, const Vec3f& posB )
{
   anchor( posA, posB, (posB-posA).length() );
}

//------------------------------------------------------------------------------
//!
void
PosSpring::prePositionStep( double step )
{
   // Compute word anchors.
   Vec3f worldAnchorA = _bodyA->simTransform() * _anchorA;
   Vec3f worldAnchorB = _bodyB->simTransform() * _anchorB;

   Vec3f x    = worldAnchorB-worldAnchorA;
   float dist = x.length();

   if( dist > CGConstf::epsilon() )
   {
      x *= (1.0f/dist);
   }
   else
   {
      x = Vec3f( 0.0f, -1.0f, 0.0f );
   }
   x *= (dist-_restLength);

   // Apply friction using the velocity from B to A.
   Vec3f deltaV = _bodyB->velocity( worldAnchorB ) - _bodyA->velocity( worldAnchorA );

   // Compute impulse to apply.
   // F = -kx - dv = k*(-x) + d*(-v)
   // p = F * t
   Vec3f p = x * (_stiffness * (float)step) + deltaV * (_damping * (float)step);

   _bodyA->applyImpulse(  p, worldAnchorA );
   _bodyB->applyImpulse( -p, worldAnchorB );
}

//------------------------------------------------------------------------------
//!
bool
PosSpring::solvePosition( double step )
{
   return true;
}

//------------------------------------------------------------------------------
//!
void
PosSpring::preVelocitiesStep()
{
}

//------------------------------------------------------------------------------
//!
bool
PosSpring::solveVelocities()
{
   _bodyA->angularVelocity( _bodyA->angularVelocity() * _angVelDamp );
   _bodyB->angularVelocity( _bodyB->angularVelocity() * _angVelDamp );
   return true;
}


/*==============================================================================
   CLASS AnchoredPosSpring
==============================================================================*/

//------------------------------------------------------------------------------
//!
AnchoredPosSpring::AnchoredPosSpring( RigidBody* bodyA ):
   _bodyA( bodyA ), _stiffness( 100.0f ), _damping( 1.0f ), _angVelDamp( 0.999f )
{
}

//------------------------------------------------------------------------------
//!
AnchoredPosSpring::~AnchoredPosSpring()
{
}

//------------------------------------------------------------------------------
//!
void
AnchoredPosSpring::anchor( const Vec3f& posA, const Vec3f& posB, float restLength )
{
   _restLength = restLength;

   // Compute anchor position into local body coordinates system.
   _anchorA = _bodyA->simReferential().globalToLocal() * posA;
   _anchorB = posB;
}

//------------------------------------------------------------------------------
//!
void
AnchoredPosSpring::anchor( const Vec3f& posA, const Vec3f& posB )
{
   anchor( posA, posB, (posB-posA).length() );
}

//------------------------------------------------------------------------------
//!
void
AnchoredPosSpring::anchor( const Vec3f& posB )
{
   _anchorB = posB;
}

//------------------------------------------------------------------------------
//!
void
AnchoredPosSpring::prePositionStep( double step )
{
   DBG_BLOCK( os_spr, "AnchoredPosSpring::prePositionStep(" << step << ")" );

   // Compute world anchors.
   Vec3f worldAnchorA = _bodyA->simTransform() * _anchorA;

   Vec3f x    = (_anchorB - worldAnchorA);
   float dist = x.length();

   if( dist > CGConstf::epsilon() )
   {
      x *= (1.0f/dist);
   }
   else
   {
      x = Vec3f( 0.0f, -1.0f, 0.0f );
   }
   x *= (dist - _restLength);

   // Apply friction using the velocity from B to A.
   Vec3f deltaV = -_bodyA->velocity( worldAnchorA );

   // Compute impulse to apply.
   // F = -kx - dv = k*(-x) + d*(-v)
   // p = F * t
   Vec3f p = x * (_stiffness * (float)step) + deltaV * (_damping * (float)step);

   _bodyA->applyImpulse( p, worldAnchorA );

   DBG_MSG( os_spr, "WorldAnchorA: " << worldAnchorA );
   DBG_MSG( os_spr, "AnchorB: " << _anchorB );
   DBG_MSG( os_spr, "A->B: " << _anchorB - worldAnchorA );
   DBG_MSG( os_spr, "dist: " << dist );
   DBG_MSG( os_spr, "x: " << x );
   DBG_MSG( os_spr, "deltaV: " << deltaV );
   DBG_MSG( os_spr, "p: " << p );
}

//------------------------------------------------------------------------------
//!
bool
AnchoredPosSpring::solvePosition( double /*step*/ )
{
   return true;
}

//------------------------------------------------------------------------------
//!
void
AnchoredPosSpring::preVelocitiesStep()
{
}

//------------------------------------------------------------------------------
//!
bool
AnchoredPosSpring::solveVelocities()
{
   _bodyA->angularVelocity( _bodyA->angularVelocity() * _angVelDamp );
   return true;
}


/*==============================================================================
   CLASS AnchoredSpring
==============================================================================*/

//------------------------------------------------------------------------------
//!
AnchoredSpring::AnchoredSpring( RigidBody* bodyA ):
   _bodyA( bodyA ), _stiffness( 100.0f ), _damping( 1.0f ),
   _maxTorque( 100.0f ), _angVelDamp( 0.999f )
{
}

//------------------------------------------------------------------------------
//!
AnchoredSpring::~AnchoredSpring()
{
}

//------------------------------------------------------------------------------
//!
void
AnchoredSpring::anchor( const Vec3f& posA, const Reff& refB, float restLength )
{
   _restLength = restLength;

   // Compute anchor position into local body coordinates system.
   _anchorA = _bodyA->simReferential().globalToLocal() * posA;
   _anchorB = refB;
}

//------------------------------------------------------------------------------
//!
void
AnchoredSpring::anchor( const Vec3f& posA, const Reff& refB )
{
   anchor( posA, refB, (refB.position()-posA).length() );
}

//------------------------------------------------------------------------------
//!
void
AnchoredSpring::anchor( const Reff& refB )
{
   _anchorB = refB;
}

//------------------------------------------------------------------------------
//!
void
AnchoredSpring::prePositionStep( double step )
{
   DBG_BLOCK( os_spr, "AnchoredSpring::prePositionStep(" << step << ")" );

   // Compute world anchors.
   Vec3f worldAnchorA = _bodyA->simTransform() * _anchorA;

   Vec3f x    = (_anchorB.position() - worldAnchorA);
   float dist = x.length();

   if( dist > CGConstf::epsilon() )
   {
      x *= (1.0f/dist);
   }
   else
   {
      x = Vec3f( 0.0f, -1.0f, 0.0f );
   }
   x *= (dist - _restLength);

   // Apply friction using the velocity from B to A.
   Vec3f deltaV = -_bodyA->velocity( worldAnchorA );

   // Compute impulse to apply.
   // F = -kx - dv = k*(-x) + d*(-v)
   // p = F * t
   Vec3f p = x * (_stiffness * (float)step) + deltaV * (_damping * (float)step);

   _bodyA->applyImpulse( p, worldAnchorA );
/**
   StdErr << "anchorA=" << _anchorA
          << " worldAnchorA=" << worldAnchorA
          << " deltaV=" << deltaV
          << " bodyA=" << _bodyA->referential()
          << " vel=" << _bodyA->linearVelocity()
          << " p=" << p
          << Endl;
**/

   // Compute the transformation needed to end a the desired orientation.
   Quatf curO = _bodyA->simOrientation();
   Quatf desO = _anchorB.orientation();
   Quatf trfO = desO * curO.getInversed();
   
   // Compute the angular velocity related to this transformation.
   Vec3f axis;
   float angle;
   trfO.toAxisAngle( axis, angle );
   Vec3f angularVelocity = axis*(angle/(float)step);
   
   Vec3f diffAV = angularVelocity - _bodyA->angularVelocity();
   
   // Clamp angular velocity.
   float maxAV = _maxTorque*(float)step/_bodyA->mass();
   
   float diffAVLen = diffAV.length();
   if(  diffAVLen > maxAV )
   {
      diffAV *= maxAV/diffAVLen;
   }

   // Apply angular velocity.
   _bodyA->angularVelocity( _bodyA->angularVelocity() + diffAV );
/**
   StdErr << "curO=" << curO
          << " desO=" << desO
          << " angVel=" << angularVelocity
          << " body=" << _bodyA->angularVelocity()
          << " diffAV=" << diffAV
          << Endl;
**/
}

//------------------------------------------------------------------------------
//!
bool
AnchoredSpring::solvePosition( double /*step*/ )
{
   return true;
}

//------------------------------------------------------------------------------
//!
void
AnchoredSpring::preVelocitiesStep()
{
}

//------------------------------------------------------------------------------
//!
bool
AnchoredSpring::solveVelocities()
{
   return true;
}
