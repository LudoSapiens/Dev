/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <MotionBullet/Constraint/Joints.h>
#include <MotionBullet/World/RigidBody.h>

#include <BulletDynamics/ConstraintSolver/btPoint2PointConstraint.h>
#include <BulletDynamics/ConstraintSolver/btHingeConstraint.h>
#include <BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
inline void convert( const Mat4f& src, btTransform& dst )
{
   dst.setOrigin( btVector3(src(0, 3), src(1, 3), src(2, 3)) );
   dst.getBasis().setValue(
      src(0, 0), src(0, 1), src(0, 2),
      src(1, 0), src(1, 1), src(1, 2),
      src(2, 0), src(2, 1), src(2, 2)
   );
}

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
   _btConstraint = new btPoint2PointConstraint(
      *btBody( bodyA ), *btBody( bodyB ), btVector3(0,0,0), btVector3(0,0,0)
   );
}

//------------------------------------------------------------------------------
//!
BallJoint::~BallJoint()
{
   delete _btConstraint;
}

//------------------------------------------------------------------------------
//!
void
BallJoint::anchor( const Vec3f& pos )
{
   // Compute anchor position into local body coordinates system.
   Vec3f anchorA = _bodyA->referential().globalToLocal() * pos;
   Vec3f anchorB = _bodyB->referential().globalToLocal() * pos;

   _btConstraint->setPivotA( btVector3( anchorA.x, anchorA.y, anchorA.z ) );
   _btConstraint->setPivotB( btVector3( anchorB.x, anchorB.y, anchorB.z ) );
}

//------------------------------------------------------------------------------
//!
btTypedConstraint*
BallJoint::btConstraint()
{
   return _btConstraint;
}

/*==============================================================================
   CLASS HingeJoint
==============================================================================*/

//------------------------------------------------------------------------------
//!
HingeJoint::HingeJoint( RigidBody* bodyA, RigidBody* bodyB ) :
   _bodyA( bodyA ), _bodyB( bodyB )
{
   // TODO
}

//------------------------------------------------------------------------------
//!
HingeJoint::~HingeJoint()
{
}

//------------------------------------------------------------------------------
//!
void
HingeJoint::anchor( const Vec3f& /*pos*/ )
{
   // Compute anchor position into local body coordinates system.
   //_anchorA = _bodyA->simReferential().globalToLocal() * pos;
   //_anchorB = _bodyB->simReferential().globalToLocal() * pos;
}

//------------------------------------------------------------------------------
//!
void
HingeJoint::axis( const Vec3f& /*dir*/ )
{
   //_axisA = _bodyA->simOrientation().getInversed().toMatrix3() * dir;
   //_axisB = _bodyB->simOrientation().getInversed().toMatrix3() * dir;
}

//------------------------------------------------------------------------------
//!
btTypedConstraint*
HingeJoint::btConstraint()
{
   return _btConstraint;
}

/*==============================================================================
   CLASS FixJoint
==============================================================================*/

//------------------------------------------------------------------------------
//!
FixJoint::FixJoint( RigidBody* bodyA, RigidBody* bodyB ) :
   _bodyA( bodyA ), _bodyB( bodyB )
{
   btTransform identity;
   convert( Mat4f::identity(), identity );
   _btConstraint = new btGeneric6DofConstraint(
      *btBody( bodyA ), *btBody( bodyB ), identity, identity, false
   );
   _btConstraint->setAngularLowerLimit( btVector3(0.0f,0.0f,0.0f) );
   _btConstraint->setAngularUpperLimit( btVector3(0.0f,0.0f,0.0f) );
}

//------------------------------------------------------------------------------
//!
FixJoint::~FixJoint()
{
   delete _btConstraint;
}

//------------------------------------------------------------------------------
//!
void
FixJoint::anchor( const Reff& ref )
{
   Mat4f m = ref.localToGlobal();
   btTransform ta;
   btTransform tb;
   convert( _bodyA->referential().globalToLocal() * m, ta );
   convert( _bodyB->referential().globalToLocal() * m, tb );
   _btConstraint->setFrames( ta, tb );
}

//------------------------------------------------------------------------------
//!
btTypedConstraint*
FixJoint::btConstraint()
{
   return _btConstraint;
}

NAMESPACE_END
