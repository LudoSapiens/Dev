/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_RENDER_DEBUG_GEOMETRY_H
#define PLASMA_RENDER_DEBUG_GEOMETRY_H

#include <Plasma/StdDefs.h>

#include <Gfx/Pass/Pass.h>

#include <Base/ADT/Vector.h>
#include <Base/Util/RCObject.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS DebugGeometry
==============================================================================*/
class DebugGeometry:
   public RCObject
{
public:

   /*----- static methods -----*/

   static PLASMA_DLL_API  void  initialize();
   static PLASMA_DLL_API  void  terminate();

   /*----- methods -----*/

   PLASMA_DLL_API DebugGeometry();
   PLASMA_DLL_API ~DebugGeometry();

   inline const Vector<float>&     points() const { return _points;    }
   inline const Vector<float>&      lines() const { return _lines;     }
   inline const Vector<float>&  triangles() const { return _triangles; }

   inline Vector<float>&     points() { return _points;    }
   inline Vector<float>&      lines() { return _lines;     }
   inline Vector<float>&  triangles() { return _triangles; }

   inline uint  numPoints()    const { return    uint(_points.size()) /  7; }
   inline uint  numLines()     const { return     uint(_lines.size()) / 14; }
   inline uint  numTriangles() const { return uint(_triangles.size()) / 21; }

   PLASMA_DLL_API void  clear();

   PLASMA_DLL_API void  computeRenderableGeometry();

   PLASMA_DLL_API void  render( Gfx::Pass& pass );

protected:

   /*----- data members -----*/

   Vector<float>  _points;
   Vector<float>  _lines;
   Vector<float>  _triangles;

   RCP<Gfx::Geometry>  _points_rgeom;
   RCP<Gfx::Geometry>  _lines_rgeom;
   RCP<Gfx::Geometry>  _triangles_rgeom;

private:
}; //class DebugGeometry


NAMESPACE_END

#endif //PLASMA_RENDER_DEBUG_GEOMETRY_H
