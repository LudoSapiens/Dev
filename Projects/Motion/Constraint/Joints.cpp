/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Motion/Constraint/Joints.h>
#include <Motion/World/RigidBody.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

UNNAMESPACE_END


NAMESPACE_BEGIN

/*==============================================================================
   CLASS BallJoint
==============================================================================*/

//------------------------------------------------------------------------------
//!
BallJoint::BallJoint( RigidBody* bodyA, RigidBody* bodyB ) :
   _bodyA( bodyA ), _bodyB( bodyB )
{
}

//------------------------------------------------------------------------------
//!
BallJoint::~BallJoint()
{
}

//------------------------------------------------------------------------------
//!
void
BallJoint::anchor( const Vec3f& pos )
{
   // Compute anchor position into local body coordinates system.
   _anchorA = _bodyA->simReferential().globalToLocal() * pos;
   _anchorB = _bodyB->simReferential().globalToLocal() * pos;
}

//------------------------------------------------------------------------------
//!
void
BallJoint::prePositionStep( double /*step*/ )
{
   // Compute word anchors.
   _worldAnchorA = _bodyA->simTransform() * _anchorA;
   _worldAnchorB = _bodyB->simTransform() * _anchorB;
   
   // Compute kinv matrix.
   _kinv = (_bodyA->computeK( _worldAnchorA ) + _bodyB->computeK( _worldAnchorB ) ).inverse();
}

//------------------------------------------------------------------------------
//!
bool
BallJoint::solvePosition( double step )
{
   const float maxDistance = 0.001f * 0.001f; // 1 mm precision.
   
   // Compute the predicted distance between anchor points.
   
   // 1. Compute future positions.
   Vec3f predA = _bodyA->lookAhead( _worldAnchorA, step );
   Vec3f predB = _bodyB->lookAhead( _worldAnchorB, step );
   Vec3f d = predA - predB;
   
   // 2. Sqr distance.
   float dist = d.sqrLength();
   
   // Check if the constraint is satisfied.
   if( dist > maxDistance )
   {            
      // Compute correction impulse.
      Vec3f p = _kinv * ( -d * (float)(1.0/step) );
      
      // Apply impulse.
      _bodyA->applyImpulse(  p, _worldAnchorA );
      _bodyB->applyImpulse( -p, _worldAnchorB );
      
      // Do another iteration.
      return true;
   }
   
   return false;
}

//------------------------------------------------------------------------------
//!
void
BallJoint::preVelocitiesStep()
{
   // Compute word anchors.
   _worldAnchorA = _bodyA->simTransform() * _anchorA;
   _worldAnchorB = _bodyB->simTransform() * _anchorB;
   
   // Compute kinv matrix.
   _kinv = (_bodyA->computeK( _worldAnchorA ) + _bodyB->computeK( _worldAnchorB ) ).inverse();
}

//------------------------------------------------------------------------------
//!
bool
BallJoint::solveVelocities()
{
   const float maxVelocity = 0.001f * 0.001f;
   
   // Compute the velocity difference.
   Vec3f ua = _bodyA->velocity( _worldAnchorA );
   Vec3f ub = _bodyB->velocity( _worldAnchorB );
   Vec3f du = ub-ua;
   
   // 2. Sqr velocity.
   float vel = du.sqrLength();
   
   // Check if the constraint is satisfied.
   if( vel > maxVelocity )
   {            
      // Compute correction impulse.
      Vec3f p = _kinv * du;
      
      // Apply impulse.
      _bodyA->applyImpulse(  p, _worldAnchorA );
      _bodyB->applyImpulse( -p, _worldAnchorB );
      
      // Do another iteration.
      return true;
   }
   
   return false;
}

/*==============================================================================
   CLASS HingeJoint
==============================================================================*/

//------------------------------------------------------------------------------
//!
HingeJoint::HingeJoint( RigidBody* bodyA, RigidBody* bodyB ) :
   _bodyA( bodyA ), _bodyB( bodyB )
{
}

//------------------------------------------------------------------------------
//!
HingeJoint::~HingeJoint()
{
}

//------------------------------------------------------------------------------
//!
void
HingeJoint::anchor( const Vec3f& pos )
{
   // Compute anchor position into local body coordinates system.
   _anchorA = _bodyA->simReferential().globalToLocal() * pos;
   _anchorB = _bodyB->simReferential().globalToLocal() * pos;
}

//------------------------------------------------------------------------------
//!
void
HingeJoint::axis( const Vec3f& dir )
{
   _axisA = _bodyA->simOrientation().getInversed().toMatrix3() * dir;
   _axisB = _bodyB->simOrientation().getInversed().toMatrix3() * dir;
}

//------------------------------------------------------------------------------
//!
void
HingeJoint::prePositionStep( double /*step*/ )
{
   // TOOD
   
   // Compute word anchors.
   _worldAnchorA = _bodyA->simTransform() * _anchorA;
   _worldAnchorB = _bodyB->simTransform() * _anchorB;
   
   // Compute kinv matrix.
   _kinv = (_bodyA->computeK( _worldAnchorA ) + _bodyB->computeK( _worldAnchorB ) ).inverse();
}

//------------------------------------------------------------------------------
//!
bool
HingeJoint::solvePosition( double step )
{
   const float maxDistance = 0.001f * 0.001f; // 1 mm precision.
   
   bool modified = false;
   
   // Positional constraint.
   
   // 1. Compute future positions.
   Vec3f predA = _bodyA->lookAhead( _worldAnchorA, step );
   Vec3f predB = _bodyB->lookAhead( _worldAnchorB, step );
   Vec3f d = predA - predB;
   
   // 2. Sqr distance.
   float dist = d.sqrLength();
   
   // Check if the constraint is satisfied.
   if( dist > maxDistance )
   {            
      // Compute correction impulse.
      Vec3f p = _kinv * ( -d * (float)(1.0/step) );
         
      // Apply impulse.
      _bodyA->applyImpulse(  p, _worldAnchorA );
      _bodyB->applyImpulse( -p, _worldAnchorB );
      
      // Do another iteration.
      modified = true;
   }
   
   // Rotational constraint.
   
   
   return modified;
}

//------------------------------------------------------------------------------
//!
void
HingeJoint::preVelocitiesStep()
{
}

//------------------------------------------------------------------------------
//!
bool
HingeJoint::solveVelocities()
{
   return false;
}

NAMESPACE_END
