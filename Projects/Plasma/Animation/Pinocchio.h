/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_PINOCCHIO_H
#define PLASMA_PINOCCHIO_H

#include <Plasma/StdDefs.h>

NAMESPACE_BEGIN

class Skeleton;
class MeshGeometry;
class SurfaceGeometry;

/*==============================================================================
   CLASS Pinocchio 
==============================================================================*/

class Pinocchio
{
public:

   /*----- static methods -----*/

   static void recomputeVertexWeights( SurfaceGeometry* geom, Skeleton* skeleton );
   static void recomputeVertexWeights( 
      MeshGeometry* geom, 
      Skeleton*     skeleton, 
      int           weightsAttrID, 
      int           bonesAttrID 
   );
};

NAMESPACE_END

#endif
