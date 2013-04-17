/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_DFGEOMOPS_H
#define PLASMA_DFGEOMOPS_H

#include <Plasma/StdDefs.h>
#include <Plasma/DataFlow/DFGeometry.h>
#include <Plasma/DataFlow/DFPolygonNodes.h>

NAMESPACE_BEGIN


/*==============================================================================
   NAMESPACE DFGeomOps
==============================================================================*/
namespace DFGeomOps
{
   PLASMA_DLL_API RCP<DFGeometry> extrude( const DFPolygon&, float height );
   PLASMA_DLL_API RCP<DFGeometry> clone(
      const DFGeometry& geom,
      uint              num,
      const Vec3f&      offset,
      const Reff&       rotation
   );

};

NAMESPACE_END

#endif
