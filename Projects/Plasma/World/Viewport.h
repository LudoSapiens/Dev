/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_VIEWPORT_H
#define PLASMA_VIEWPORT_H

#include <Plasma/StdDefs.h>
#include <Plasma/World/Camera.h>
#include <Plasma/World/World.h>
#include <CGMath/Frustum.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Viewport
==============================================================================*/
class Viewport
{

public:

   /*----- methods -----*/

   PLASMA_DLL_API Viewport();
   PLASMA_DLL_API ~Viewport();

   // Camera.
   inline void camera( Camera* c ) { _camera = c; }
   inline Camera* camera() const   { return _camera; }

   // World.
   inline void world( World* w )   { _world = w; }
   inline World* world() const     { return _world.ptr(); }

   // Aspect.
   inline void aspect( float aspect ) { _aspect = aspect; }
   inline float aspect() const        { return _aspect; }

   // Viewport.
   void region( const Vec2f& pos, const Vec2f& size )   { _pos = pos; _size = size; }
   inline const Vec2f& position() const                 { return _pos; }
   inline const Vec2f& size() const                     { return _size; }

   PLASMA_DLL_API int fovAxis() const;

   // Matrices.
   PLASMA_DLL_API Mat4f cameraMatrix() const;
   PLASMA_DLL_API Mat4f projectionMatrix() const;
   PLASMA_DLL_API Mat4f viewMatrix() const;
   PLASMA_DLL_API Mat4f viewportMatrix() const;

   // Utilities.
   PLASMA_DLL_API Vec3f direction( const Vec2f& dir ) const;
   PLASMA_DLL_API Frustumf frustum() const;

protected:

   /*----- data members -----*/

   float          _aspect;
   Vec2f          _size;
   Vec2f          _pos;
   Camera*        _camera;
   RCP<World>     _world;
};

NAMESPACE_END

#endif
