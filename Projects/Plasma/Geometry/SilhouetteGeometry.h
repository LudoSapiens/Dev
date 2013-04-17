/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_SILHOUETTE_GEOMETRY_H
#define PLASMA_SILHOUETTE_GEOMETRY_H

#include <Plasma/StdDefs.h>
#include <Plasma/Geometry/MeshGeometry.h>


NAMESPACE_BEGIN

/*==============================================================================
   CLASS SilhouetteGeometry
==============================================================================*/

class SilhouetteGeometry:
   public MeshGeometry
{
public:

   /*----- methods -----*/

   SilhouetteGeometry();

   void mesh( MeshGeometry*, bool compact = false );
   void update( const Vec3f& pos );

protected:

   /*----- structures -----*/

   struct Edge
   {
      Edge( uint32_t v0, uint32_t v1, uint32_t t0, uint32_t t1 )
      {
         _vIdx[0]   = v0;
         _vIdx[1]   = v1;
         _triIdx[0] = t0;
         _triIdx[1] = t1;
      }
      uint32_t _triIdx[2];
      uint32_t _vIdx[2];
   };

   /*----- methods -----*/

   virtual ~SilhouetteGeometry();
   void computeVertices( MeshGeometry* );
   void computeEdges( bool );

   /*----- data members -----*/

   Vector<bool>      _facing;
   Vector<Vec3f>     _normals;
   Vector<uint32_t>  _triangles;
   Vector<Edge>      _edges;
};

NAMESPACE_END

#endif
