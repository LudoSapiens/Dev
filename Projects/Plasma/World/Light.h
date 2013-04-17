/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef LIGHT_H
#define LIGHT_H

#include <Plasma/StdDefs.h>

#include <Plasma/World/RigidEntity.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS Light
==============================================================================*/

class Light:
   public RigidEntity
{
public:

   /*----- enumerations -----*/

   enum Shape
   {
      DIRECTIONAL,
      GEOMETRY,
      POINT,
      SPOT
   };

   /*----- methods -----*/

   PLASMA_DLL_API Light( RigidBody::Type bodyType );
   PLASMA_DLL_API virtual ~Light();

   PLASMA_DLL_API const Mat4f& projectionMatrix() const;
   PLASMA_DLL_API const Mat4f& viewMatrix() const;
   PLASMA_DLL_API const Mat4f& lightMatrix() const;
   PLASMA_DLL_API Mat4f viewportMatrix() const;

   PLASMA_DLL_API void shape( Shape s );
   PLASMA_DLL_API void intensity( const Vec3f& v );
   PLASMA_DLL_API void fov( float );
   PLASMA_DLL_API void back( float );
   PLASMA_DLL_API void front( float );
   PLASMA_DLL_API void frontAndBack( float front, float back );

   inline Shape shape() const              { return _shape; }
   inline const Vec3f& intensity() const   { return _intensity; }
   inline float fov() const                { return _fov; }
   inline float back() const               { return _back; }
   inline float front() const              { return _front; }

protected:

   /*----- methods -----*/

   friend class World;

   /*----- types and enumerations ----*/

   enum {
      PROJ  = 1,
      VIEW  = 2,
      LIGHT = 4
   };

   /*----- data members -----*/

   Shape                    _shape;
   Vec3f                    _intensity;

   float                    _front;
   float                    _back;
   float                    _fov;

   mutable Mat4f           _projMat;
   mutable Mat4f           _viewMat;
   mutable Mat4f           _lightMat;
   mutable int             _dirtyMat; // FIXME: rethink the concept!
};

NAMESPACE_END

#endif
