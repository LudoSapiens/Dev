/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef MOTION_COLLISIONSHAPE_H
#define MOTION_COLLISIONSHAPE_H

#include <Motion/StdDefs.h>

#include <CGMath/Ref.h>
#include <CGMath/AABBox.h>

#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>

NAMESPACE_BEGIN

class CollisionInfo;
class CollisionPair;
class CollisionGroup;

class SphereShape;
class BoxShape;

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

   /*----- static methods -----*/
   
   MOTION_DLL_API static bool collide( CollisionPair& pair );
   
   MOTION_DLL_API static bool collide( 
      CollisionShape* a, const Reff& refA,
      CollisionShape* b, const Reff& refB,
      CollisionInfo& info
   );
   
   MOTION_DLL_API static bool collideSphereSphere( 
      SphereShape* a, const Reff& refA,
      SphereShape* b, const Reff& refB,
      CollisionInfo& info,
      bool reverse = false
   );
   
   MOTION_DLL_API static bool collideSphereBox( 
      SphereShape* a, const Reff& refA,
      BoxShape*    b, const Reff& refB,
      CollisionInfo& info,
      bool reverse = false
   );
   
   MOTION_DLL_API static bool collideGJK( 
      CollisionShape* a, const Reff& refA,
      CollisionShape* b, const Reff& refB,
      CollisionInfo& info
   );
   
   /*----- methods -----*/

   inline uint type() const { return _type; }

   inline float margin() const;

   inline const AABBoxf& boundingBox() const;

   //------------------------------------------------------------------------------
   //! Returns the farthest point in the shape along the specified direction.
   //! Also referred to as the support function.
   //! The returned position is in global coordinates and does not take the margin
   //! of the object into account.
   MOTION_DLL_API virtual Vec3f  getFarthestPointAlong( const Reff& ref, const Vec3f& dir ) const = 0;

protected: 
   
   /*----- methods -----*/

   CollisionShape( uint type );
   virtual ~CollisionShape();

   /*----- data members -----*/

   uint            _type;
   float           _margin;
   AABBoxf         _bbox; // local bounding box. TO REMOVE?
};

//------------------------------------------------------------------------------
//!
inline float 
CollisionShape::margin() const
{
   return _margin;
}

//------------------------------------------------------------------------------
//!
inline const AABBoxf& 
CollisionShape::boundingBox() const
{
   return _bbox;
}

NAMESPACE_END

#endif
