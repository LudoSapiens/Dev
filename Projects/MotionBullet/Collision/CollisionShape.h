/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef MOTIONBULLET_COLLISIONSHAPE_H
#define MOTIONBULLET_COLLISIONSHAPE_H

#include <MotionBullet/StdDefs.h>

#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>

class btCollisionShape;

NAMESPACE_BEGIN

class CollisionGroup;

/*==============================================================================
   CLASS CollisionShape
==============================================================================*/
//!
class CollisionShape
   : public RCObject
{

public: 

   /*----- types and enumerations ----*/

   enum {
      GROUP      = 0,
      SPHERE     = 1,
      BOX        = 2,
      CYLINDER   = 3,
      CONE       = 4,
      CONVEXHULL = 5,
      SPHEREHULL = 6,
      TRIMESH    = 7
   };

   /*----- methods -----*/

   inline uint type() const { return _type; }

protected: 
   
   /*----- friends -----*/

   friend class CollisionGroup;
   friend class MotionWorld;
   friend class RigidBody;

   /*----- methods -----*/

   CollisionShape( uint type );
   virtual ~CollisionShape();

   virtual btCollisionShape*  bulletCollisionShape() = 0;

   /*----- data members -----*/

   uint _type;
};


NAMESPACE_END

#endif
