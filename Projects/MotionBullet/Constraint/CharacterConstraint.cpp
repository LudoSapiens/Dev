/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <MotionBullet/Constraint/CharacterConstraint.h>
#include <MotionBullet/World/MotionWorld.h>
#include <MotionBullet/World/RigidBody.h>

#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>

#include <Base/Dbg/Defs.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

/*==============================================================================
   class ClosestHit
==============================================================================*/


class ClosestHit:
   public btCollisionWorld::ClosestRayResultCallback
{
public:

   /*----- methods -----*/

   ClosestHit( btRigidBody* b ):
      btCollisionWorld::ClosestRayResultCallback( btVector3(0.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.0) )
   {
      _body = b;
      m_closestHitFraction = 1.0;
   }

   virtual btScalar addSingleResult( btCollisionWorld::LocalRayResult& rayResult, bool worldNormal )
   {
      if( rayResult.m_collisionObject == _body )
      {
         return 10.0;
      }
      return ClosestRayResultCallback::addSingleResult( rayResult, worldNormal );
   }

   void rayTest( float start, float end, btDiscreteDynamicsWorld* w )
   {
      btVector3 pos = _body->getCenterOfMassPosition();
      btVector3 dir( 0.0f, 1.0f, 0.0f );
      btVector3 rayEnd   = pos + dir * end;
      btVector3 rayStart = pos + dir * start;
      w->rayTest( rayStart, rayEnd, *this );
   }

protected:

   /*----- members -----*/

   btRigidBody* _body;
};


UNNAMESPACE_END


NAMESPACE_BEGIN


/*==============================================================================
   CLASS CharacterConstraint
==============================================================================*/

//------------------------------------------------------------------------------
//!
CharacterConstraint::CharacterConstraint( RigidBody* body ):
   _falling( false ),
   _conLinVel( true ),
   _body( body ),
   _linVel( Vec3f::zero() ),
   _maxF( 200.0f ),
   _oLinVel( 0.0f ),
   _fallDur( 0.0f )
{
   btBody( _body.ptr() )->setSleepingThresholds(0.0, 0.0);
   btBody( _body.ptr() )->setAngularFactor(0.0);
}

//------------------------------------------------------------------------------
//!
CharacterConstraint::~CharacterConstraint()
{
}

//------------------------------------------------------------------------------
//!
btTypedConstraint*
CharacterConstraint::btConstraint()
{
   return 0;
}

//------------------------------------------------------------------------------
//!
void
CharacterConstraint::preStep( double )
{
   if( _conLinVel && !_falling )
   {
      // We want to keep the velocity in the direction of the gravity but we
      // don't want to climb to fast.
      Vec3f linVel = _linVel;
      linVel.y     = _body->linearVelocity().y;
      if( linVel.y > 0.0f ) linVel.y *= 0.5f;
      _body->linearVelocity( linVel );
   }
   else
   {
      Vec3f linVel = _linVel;
      linVel.y     = _body->linearVelocity().y; // Assume gravity is -Y exclusively.
      //_body->linearVelocity( linVel );
   }
   _oLinVel = _body->linearVelocity();
}

//------------------------------------------------------------------------------
//!
void
CharacterConstraint::postStep( double step )
{
   Vec3f gd;
   float gn;
   _body->world()->getGravity( _body.ptr(), gd, gn );
   Vec3f vel    = _body->linearVelocity();
   Vec3f a      = vel - _oLinVel;
   float d      = dot( a, gd ) / (float)step;  // Divide by step cheaper here than when computing 'a'.
   bool falling = false;

   // We are falling -> detect if we are landing.
   if( _falling )
   {
      // We are landing if the region (point) under the character is occluded.
      ClosestHit cb( btBody( _body.ptr() ) );
      cb.rayTest( 0.5f, -0.045f, btWorld() ); // For now the character foot are at -0.04!

      falling = !cb.hasHit();
   }
   else
   {
      // Are we going down?
      bool fallingGravity = (vel.y < 0.0f) && (d > (gn * 0.97f));  // Within 3% of the gravity.

      // We are falling only if the "step" is deeper than a constant.
      if( fallingGravity )
      {
         ClosestHit cb( btBody( _body.ptr() ) );
         cb.rayTest( 0.5f, -0.6f, btWorld() );

         // Is the hole deeper than (0.6-0.04) meters?
         falling = !cb.hasHit();
      }
   }

   // Compute some other useful variables.
   if( falling != _falling )
   {
      _fallDur  = 0.0f;
   }
   else
   if( falling )
   {
      _fallDur += (float)step;
   }
   _falling = falling;
}

NAMESPACE_END
