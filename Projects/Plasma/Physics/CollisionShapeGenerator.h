/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_COLLISION_SHAPE_GENERATOR_H
#define PLASMA_COLLISION_SHAPE_GENERATOR_H

#include <Plasma/StdDefs.h>

NAMESPACE_BEGIN

class Geometry;
class CollisionShape;

/*==============================================================================
  CLASS CollisionShapeGenerator
==============================================================================*/
class CollisionShapeGenerator
{
public:

   /*----- methods -----*/

   CollisionShapeGenerator();
   ~CollisionShapeGenerator();

   CollisionShape*  generate( int type, const Geometry& geom );

   CollisionShape*  generateSphere( const Geometry& geom );
   CollisionShape*  generateBoxAtOrigin( const Geometry& geom );
   CollisionShape*  generateBoxInGroup( const Geometry& geom );
   //CollisionShape*  generateCylinder( const Geometry& geom );
   //CollisionShape*  generateCone( const Geometry& geom );
   CollisionShape*  generateConvexHull( const Geometry& geom );
   //CollisionShape*  generateSphereHull( const Geometry& geom );
   CollisionShape*  generateTrimesh( const Geometry& geom );

protected:

   /*----- data members -----*/

   /* members... */

   /*----- methods -----*/

   /* methods... */

private:
}; //class CollisionShapeGenerator


NAMESPACE_END

#endif //PLASMA_COLLISION_SHAPE_GENERATOR_H
