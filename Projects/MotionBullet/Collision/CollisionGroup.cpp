/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <MotionBullet/Collision/CollisionGroup.h>

#include <BulletCollision/CollisionShapes/btCompoundShape.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS CollisionGroup
==============================================================================*/

//------------------------------------------------------------------------------
//!
CollisionGroup::CollisionGroup( uint n ):
   CollisionShape( GROUP ), _com( Reff::identity() ), _tensor( 1.0f )
{
   _btCompoundShape = new btCompoundShape( n >= 2 ); // Use AABB tree structure when we expect enough child shapes.
}

//------------------------------------------------------------------------------
//!
CollisionGroup::~CollisionGroup()
{
   delete _btCompoundShape;
}

//------------------------------------------------------------------------------
//!
void
CollisionGroup::addShape( const Reff& ref, CollisionShape* shape )
{
   _shapes.pushBack( shape );

   Reff lref = _com.getInversed()*ref;

   btTransform t;
   t.setOrigin(
      btVector3( lref.position().x, lref.position().y, lref.position().z )
   );
   t.setRotation(
      btQuaternion(
         lref.orientation().x(),
         lref.orientation().y(),
         lref.orientation().z(),
         lref.orientation().w()
      )
   ); 

   _btCompoundShape->addChildShape( t, shape->bulletCollisionShape() );
}

//------------------------------------------------------------------------------
//!
void
CollisionGroup::removeShape( CollisionShape* shape )
{
   _btCompoundShape->removeChildShape( shape->bulletCollisionShape() );
   _shapes.remove( shape );
}

//------------------------------------------------------------------------------
//!
void
CollisionGroup::clearShapes()
{
   _shapes.clear();
   delete _btCompoundShape;
   _btCompoundShape = new btCompoundShape( false );
}

//------------------------------------------------------------------------------
//! 
Reff
CollisionGroup::referential( uint id ) const
{
   const btTransform& src = _btCompoundShape->getChildTransform(id);
   Reff dst;
   dst.scale( 1.0f );
   dst.position( src.getOrigin().x(), src.getOrigin().y(), src.getOrigin().z() );
   btQuaternion q( src.getRotation() );
   dst.orientation( Quatf( q.x(), q.y(), q.z(), q.w() ) );
   return _com * dst;
}

//------------------------------------------------------------------------------
//! 
btCollisionShape*
CollisionGroup::bulletCollisionShape()
{
   return _btCompoundShape;
}

//------------------------------------------------------------------------------
//! 
void
CollisionGroup::centerOfMass( const Reff& com )
{
   // Compute difference transform.
   Reff xform = com.getInversed() * _com;

   btTransform t;
   t.setOrigin(
      btVector3( xform.position().x, xform.position().y, xform.position().z )
   );
   t.setRotation(
      btQuaternion(
         xform.orientation().x(),
         xform.orientation().y(),
         xform.orientation().z(),
         xform.orientation().w()
      )
   );

   // Update transform.
   _com = com;

   // Update children transforms.
   for( int i = 0; i < _btCompoundShape->getNumChildShapes(); ++i )
   {
      _btCompoundShape->updateChildTransform( i, t * _btCompoundShape->getChildTransform(i) );
   }
}

//------------------------------------------------------------------------------
//! 
void
CollisionGroup::inertiaTensor( const Vec3f& tensor )
{
   _tensor = tensor;
}

NAMESPACE_END
