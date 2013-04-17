/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_CAMERA_H
#define PLASMA_CAMERA_H

#include <Plasma/StdDefs.h>
#include <Plasma/World/RigidEntity.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Camera
==============================================================================*/

class Camera
   : public RigidEntity
{

public:

   /*----- types and enumerations -----*/

   enum ProjectionType {
      PERSPECTIVE,
      ORTHO
   };

   enum FOVMode {
      FOV_X,
      FOV_Y,
      SMALLEST,
      LARGEST,
   };

   /*----- methods -----*/

   PLASMA_DLL_API Camera( RigidBody::Type bodyType );

   PLASMA_DLL_API void projection( ProjectionType );
   PLASMA_DLL_API void focalLength( float length );
   PLASMA_DLL_API void fov( float fov );
   PLASMA_DLL_API void fovMode( FOVMode );
   PLASMA_DLL_API void back( float back );
   PLASMA_DLL_API void front( float front );
   PLASMA_DLL_API void frontAndBack( float front, float back );
   PLASMA_DLL_API void optimizeFrontAndBack( const AABBoxf& box );
   PLASMA_DLL_API void orthoScale( float scale );
   PLASMA_DLL_API void shear( float shx, float shy );
   PLASMA_DLL_API void shear( const Vec3f& viewNormal );

   inline ProjectionType projection() const { return _projType; }
   inline float focalLength() const         { return _focalLength; }
   inline float fov() const                 { return _fov; }
   inline FOVMode fovMode() const           { return _fovMode; }
   inline float back() const                { return _back; }
   inline float front() const               { return _front; }
   inline float orthoScale() const          { return _scale; }
   inline float shearX() const              { return _shearX; }
   inline float shearY() const              { return _shearY; }
   inline Vec2f shear() const               { return Vec2f( _shearX, _shearY ); }

protected:

   /*----- methods -----*/

   virtual ~Camera();

   /*----- data members -----*/

   float          _focalLength;
   float          _fov;
   float          _back;
   float          _front;
   float          _scale;
   float          _shearX;
   float          _shearY;
   ProjectionType _projType;
   FOVMode        _fovMode;
};

NAMESPACE_END

#endif
