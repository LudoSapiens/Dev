/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_PROCEDURAL_MESH_H
#define PLASMA_PROCEDURAL_MESH_H

#include <Plasma/StdDefs.h>
#include <Plasma/Procedural/Component.h>
#include <Plasma/Procedural/ProceduralContext.h>
#include <Plasma/Geometry/MeshGeometry.h>

#include <Base/MT/Task.h>
#include <Base/Util/RCP.h>
#include <Base/ADT/Map.h>
#include <Base/ADT/String.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS ProceduralMesh
==============================================================================*/

class ProceduralMesh:
   public Task
{
public:

   /*----- static methods -----*/

   static void initialize();

   /*----- methods -----*/

   ProceduralMesh( Resource<Geometry>* res,  const String& path );
   ProceduralMesh( Resource<Geometry>* res, const String& path, const Table& params );

   // Collision Shapes.
   inline void  addShape( CollisionShape* shape, const Reff& ref ) { _collisionShapes[shape] = ref; }
   inline void  lastCollision( CollisionShape* v )                 { _lastCollision = v; }
   inline void  autoCollision( int v )                             { _autoCollision = v; }
   inline const Reff&  shapeReferential( CollisionShape* shape ) const;

   inline MeshGeometry* geometry() { return _geom.ptr(); }

protected:

   /*----- data types -----*/

   typedef Map< RCP<CollisionShape>, Reff >  ShapeInfoContainer;

   /*----- methods -----*/

   virtual void execute();

   /*----- data members -----*/

   RCP< Resource<Geometry> > _res;
   RCP<MeshGeometry>         _geom;
   RCP<const Table>          _params;
   String                    _path;
   ShapeInfoContainer        _collisionShapes;  // Holds all transient collision shapes (even dropped ones), along with a ref for each.
   CollisionShape*           _lastCollision;    // The last collision shape specified in collision() call.
   int                       _autoCollision;    // A flag indicating which auto collision type to use.
};

//------------------------------------------------------------------------------
//!
inline const Reff&
ProceduralMesh::shapeReferential( CollisionShape* shape ) const
{
   ShapeInfoContainer::ConstIterator it = _collisionShapes.find( shape );
   CHECK( it != _collisionShapes.end() );
   if( it != _collisionShapes.end() )
   {
      return (*it).second;
   }
   else
   {
      static Reff identity = Reff::identity();
      return identity;
   }
}

NAMESPACE_END

#endif
