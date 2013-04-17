/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Motion/Constraint/CharacterConstraint.h>
#include <Motion/Collision/BasicShapes.h>
#include <Motion/World/MotionWorld.h>
#include <Motion/World/RigidBody.h>

#include <Base/Dbg/DebugStream.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

DBG_STREAM( os_cc, "CharacterConstraint" );

//------------------------------------------------------------------------------
//!
bool 
collide( RigidBody* body, CollisionShape* shape, const Reff& shapeRef, const MotionWorld* world )
{
   AABBoxf box;
   box.set( Mat3f::identity(), shapeRef.position(), shape->boundingBox() );
   
   for( uint b = 0; b < world->staticBodies().size(); ++b )
   {
      if( box.isOverlapping( world->staticBodies()[b]->boundingBox() ) )
      {
         return true;
      }
   }
   for( uint b = 0; b < world->rigidBodies().size(); ++b )
   {
      RigidBody* curBody = world->rigidBodies()[b].ptr();
      if( curBody != body && box.isOverlapping( curBody->boundingBox() ) )
      {
         return true;
      }
   }
   for( uint b = 0; b < world->kinematicBodies().size(); ++b )
   {
      if( box.isOverlapping( world->kinematicBodies()[b]->boundingBox() ) )
      {
         return true;
      }
   }
   return false;
}


UNNAMESPACE_END


NAMESPACE_BEGIN


/*==============================================================================
   CLASS CharacterConstraint
==============================================================================*/

//------------------------------------------------------------------------------
//!
CharacterConstraint::CharacterConstraint( RigidBody* body ):
   _conLinVel( true ),
   _body( body ),
   _linVel( Vec3f::zero() ),
   _maxF( 200.0f )
{
   _shape = new SphereShape( 0.3f );
}

//------------------------------------------------------------------------------
//!
CharacterConstraint::~CharacterConstraint()
{
}

//------------------------------------------------------------------------------
//!
void
CharacterConstraint::connect( MotionWorld* w )
{
   Constraint::connect(w);
}

//------------------------------------------------------------------------------
//!
void
CharacterConstraint::disconnect()
{
   Constraint::disconnect();
}

//------------------------------------------------------------------------------
//!
void
CharacterConstraint::prePositionStep( double step )
{
   DBG_MSG( os_cc, "CC: body.pos=" << _body->position() );

   // Test if body is on ground.
#if 0
   // FIXME
   _shape->globalReferential( _body->simReferential() );
   _onGround = collide( _body.ptr(), _shape, _body->world() );
#else
   _onGround = false;
#endif

   if( _conLinVel && _onGround )
   {
      _pSum   = Vec3f::zero();
      _maxP   = (float)(_maxF * step);

      _desPos = _body->simPosition() + _linVel * (float)step;

      // Compute kinv matrix (consider a second entity as static, which is K=0).
      _kinv   = _body->computeK( _body->simPosition() ).inverse();
   }

   _desOri = _body->simOrientation();
   _linv   = _body->computeL().inverse();
}

//------------------------------------------------------------------------------
//!
bool
CharacterConstraint::solvePosition( double step )
{
   float posAttained = true;
   float oriAttained = true;

   if( _conLinVel && _onGround)
   {
      // Adjust position.
      const float maxDistance = 0.001f * 0.001f; // 1 mm precision.

      Vec3f expP        = _body->lookAhead( _body->simPosition(), step );
      const Vec3f& desP = _desPos;
      Vec3f difP        = desP - expP;
      difP.y            = 0.0f;
      float d           = difP.sqrLength();

      if( d > maxDistance )
      {
         // Compute correction impulse.
         Vec3f p = _kinv * ( difP * (float)(1.0/step) );

         // Clamp p to whatever is allowed by _maxP.
         Vec3f pSumOld = _pSum;
         _pSum        += p;
         _pSum.maxLength( _maxP );
         p             = _pSum - pSumOld;

         DBG_MSG( os_cc, "difP=" << difP << " d=" << d << ">" << maxDistance
                      << " p=" << p << " |p|=" << p.length()
                      << " pSum=" << _pSum << " |pSum|=" << _pSum.length() );

         // Apply impulse.
         _body->applyImpulse( p, _body->simPosition() );

         // Do another iteration.
         posAttained = false;
      }
   }


/*
   // Adjust orientation.
   const float maxAngle = 0.0001f;

   Quatf  expO = _body->lookAhead( _body->simOrientation(), step );
   Quatf  desO = _desOri;
   Quatf  difO = desO.getInversed() * expO;

   Vec3f  axis;
   float  angle;
   difO.toAxisAngle( axis, angle );
   if( angle > maxAngle )
   {
      Vec3f difW = axis * ( angle * (float)(1.0/step) );
      Vec3f p    = _linv * difW;
      DBG_MSG( os_cc, "difW=" << difW << " axis=" << axis << " angle=" << angle << ">" << maxAngle );

      // Apply impulse.
      _body->applyTorqueImpulse( -p );

      // Do another iteration.
      oriAttained = false;
   }
*/

   return posAttained && oriAttained;
}

//------------------------------------------------------------------------------
//!
void
CharacterConstraint::preVelocitiesStep()
{
   // Nothing to do.
}

//------------------------------------------------------------------------------
//!
bool
CharacterConstraint::solveVelocities()
{
   if( _conLinVel && _onGround )
   {
      // Restore the desired velocity.
      Vec3f v = _body->linearVelocity();
      v.x     = _linVel.x;
      v.z     = _linVel.z;
      _body->linearVelocity( v );
   }

   // Avoid quaternion drift by forcing the exact same orientation as at the beginning.
   _body->simOrientation( _desOri );

   // Stop any angular momentum.
   _body->angularVelocity( Vec3f::zero() );

   return true;
}

NAMESPACE_END
