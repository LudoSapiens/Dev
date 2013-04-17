/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef MOTION_COLLISIONGROUP_H
#define MOTION_COLLISIONGROUP_H

#include <MotionBullet/StdDefs.h>
#include <MotionBullet/Collision/CollisionShape.h>

#include <CGMath/Ref.h>

#include <Base/Util/RCP.h>
#include <Base/ADT/Vector.h>

class btCompoundShape;

NAMESPACE_BEGIN

/*==============================================================================
   CLASS CollisionGroup
==============================================================================*/
//!
class CollisionGroup:
   public CollisionShape
{

public: 
   
   /*----- methods -----*/

   MOTION_DLL_API CollisionGroup( uint n = 1 );
   MOTION_DLL_API ~CollisionGroup();

   // Shapes.
   MOTION_DLL_API void addShape( const Reff&, CollisionShape* );
   MOTION_DLL_API void removeShape( CollisionShape* );
   MOTION_DLL_API void clearShapes();

   MOTION_DLL_API Reff referential( uint id ) const;

   inline CollisionShape* shape( uint id ) const { return _shapes[id].ptr(); }
   inline uint numShapes() const                 { return (uint)_shapes.size(); }

   MOTION_DLL_API void centerOfMass( const Reff& );
   MOTION_DLL_API void inertiaTensor( const Vec3f& );
   const Reff& centerOfMass() const              { return _com; }
   const Vec3f& inertiaTensor() const            { return _tensor; }

protected:

   /*----- methods -----*/

   virtual btCollisionShape*  bulletCollisionShape();

   /*----- data members -----*/
   
   btCompoundShape*              _btCompoundShape;
   Vector< RCP<CollisionShape> > _shapes;
   Reff                          _com;
   Vec3f                         _tensor;
};


NAMESPACE_END

#endif
