/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef MOTIONBULLET_COLLISION_INFO_H
#define MOTIONBULLET_COLLISION_INFO_H

#include <MotionBullet/StdDefs.h>
#include <MotionBullet/World/RigidBody.h>

#include <Base/Util/RCP.h>
#include <Base/ADT/Vector.h>

#include <CGMath/Vec3.h>
#include <CGMath/Mat3.h>

NAMESPACE_BEGIN


/*==============================================================================
  CLASS CollisionPair
==============================================================================*/

class CollisionPair
{
public:

   /*----- methods -----*/

   CollisionPair( const RCP<RigidBody>& bodyA, const RCP<RigidBody>& bodyB )
   {
      if( bodyA.ptr() < bodyB.ptr() )
      {
         _bodyA = bodyA.ptr();
         _bodyB = bodyB.ptr();
      }
      else
      {
         _bodyA = bodyB.ptr();
         _bodyB = bodyA.ptr();
      }
   }
   
   ~CollisionPair()
   {
   }
      
      
   // Accessors.
   RigidBody* bodyA() { return _bodyA; }
   const RigidBody* bodyA() const { return _bodyA; }

   RigidBody* bodyB() { return _bodyB; }
   const RigidBody* bodyB() const { return _bodyB; }

   void worldPosition( const Vec3f& v ) { _worldPosition = v; }
   const Vec3f& worldPosition() const { return _worldPosition; }

   // Operators.
   bool operator<( const CollisionPair& p ) const
   {
      if( _bodyA < p._bodyA )
      {
         return true;
      }
      if( _bodyA == p._bodyA && _bodyB < p._bodyB )
      {
         return true;
      }
      return false;
   }
   
   bool operator==( const CollisionPair& p ) const
   {
      return ( _bodyA == p._bodyA && _bodyB == p._bodyB );
   }
   bool operator!=( const CollisionPair& p ) const
   {
      return ( _bodyA != p._bodyA || _bodyB != p._bodyB );
   }

private:
   
   /*----- data members -----*/

   RigidBody*         _bodyA;
   RigidBody*         _bodyB;
   Vec3f              _worldPosition;
}; 

NAMESPACE_END

#endif
