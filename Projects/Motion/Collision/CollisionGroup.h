/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef MOTION_COLLISIONGROUP_H
#define MOTION_COLLISIONGROUP_H

#include <Motion/StdDefs.h>
#include <Motion/Collision/CollisionShape.h>

#include <CGMath/Ref.h>
#include <CGMath/AABBox.h>

#include <Base/Util/RCP.h>
#include <Base/ADT/Vector.h>

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

   MOTION_DLL_API CollisionGroup();
   MOTION_DLL_API ~CollisionGroup();

   // Shapes.
   inline void addShape( const Reff&, CollisionShape* );
   inline void removeShape( CollisionShape* );
   inline void clearShapes();

   inline CollisionShape* shape( uint ) const;
   inline uint numShapes() const;
   inline const Reff& shapeRef( uint ) const;

   MOTION_DLL_API virtual Vec3f  getFarthestPointAlong( const Reff& ref, const Vec3f& dir ) const;

protected: 
   
   /*----- data types -----*/
   struct ChildShape
   {
      Reff                 _ref;
      RCP<CollisionShape>  _shape;
      ChildShape(){}
      ChildShape( const Reff& ref, CollisionShape* shape ): _ref( ref ), _shape( shape ) {}
      bool operator==( const ChildShape& inst ) { return _shape == inst._shape; }
   };

   /*----- methods -----*/

   /*----- data members -----*/

   Vector< ChildShape >          _shapes;
};

//------------------------------------------------------------------------------
//!
inline void
CollisionGroup::addShape( const Reff& ref, CollisionShape* shape )
{
   _shapes.pushBack( ChildShape(ref, shape) );
   
   // Update bb.
   if( numShapes() == 1 )
   {
      _bbox.set(
         ref.orientation().toMatrix3(),
         ref.position(),
         shape->boundingBox()
      );
      _bbox.grow( 0.01f );
   }
   else
   {
      AABBoxf bbox;
      bbox.set(
         ref.orientation().toMatrix3(),
         ref.position(),
         shape->boundingBox()
      );
      bbox.grow( 0.01f );
      _bbox |= bbox;
   }
}

//------------------------------------------------------------------------------
//!
inline void
CollisionGroup::removeShape( CollisionShape* shape )
{
   _shapes.remove( ChildShape(Reff(), shape) );
   
   // Update bb.
   if( !_shapes.empty() )
   {
      _bbox.set(
         _shapes[0]._ref.orientation().toMatrix3(),
         _shapes[0]._ref.position(),
         _shapes[0]._shape->boundingBox()
      );
      
      for( uint i = 1; i < numShapes(); ++i )
      {
         AABBoxf bbox;
         bbox.set(
            _shapes[i]._ref.orientation().toMatrix3(),
            _shapes[i]._ref.position(),
            _shapes[i]._shape->boundingBox()
         );
         _bbox |= bbox;
      }
      _bbox.grow( 0.01f );
   }
}

//------------------------------------------------------------------------------
//!
inline void
CollisionGroup::clearShapes()
{
   _shapes.clear();
   _bbox = AABBoxf::empty();
}

//------------------------------------------------------------------------------
//!
inline CollisionShape*
CollisionGroup::shape( uint id ) const
{
   return _shapes[id]._shape.ptr();
}

//------------------------------------------------------------------------------
//!
inline uint 
CollisionGroup::numShapes() const
{
   return (uint)_shapes.size();
}

//------------------------------------------------------------------------------
//!
inline const Reff&
CollisionGroup::shapeRef( uint id ) const
{
   return _shapes[id]._ref;
}


NAMESPACE_END

#endif
