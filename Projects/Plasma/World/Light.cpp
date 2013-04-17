/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/World/Light.h>
#include <Plasma/World/World.h>

#include <Fusion/Core/Core.h>


USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

UNNAMESPACE_END


/*==============================================================================
   CLASS Light
==============================================================================*/

NAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
Light::Light( RigidBody::Type bodyType ):
   RigidEntity( Entity::LIGHT, bodyType ),
   _shape( POINT ),
   _intensity( 1.0f ),
   _front( 0.1f ), _back( 500.0f ),
   _fov( 90.0f ),
   _dirtyMat( 0xff )
{
}

//------------------------------------------------------------------------------
//!
Light::~Light()
{
}

//------------------------------------------------------------------------------
//!
void Light::shape( Shape s )
{
   _shape = s;
}

//------------------------------------------------------------------------------
//!
void Light::intensity( const Vec3f& v )
{
   _intensity = v;
}

//------------------------------------------------------------------------------
//!
void Light::front( float val )
{
   _front     = val;
   _dirtyMat |= PROJ | LIGHT;
}

//------------------------------------------------------------------------------
//!
void Light::back( float val )
{
   _back      = val;
   _dirtyMat |= PROJ | LIGHT;
}

//------------------------------------------------------------------------------
//!
void Light::frontAndBack( float front, float back )
{
   _front     = front;
   _back      = back;
   _dirtyMat |= PROJ | LIGHT;
}


//------------------------------------------------------------------------------
//!
void Light::fov( float val )
{
   _fov = val;
   _dirtyMat |= PROJ | LIGHT;
}


//------------------------------------------------------------------------------
//!
const Mat4f&
Light::projectionMatrix() const
{
   // FIXME This is only for persperctive view. We should also do a ortho.
   if( _dirtyMat & PROJ )
   {
      if( _shape != DIRECTIONAL )
         _projMat = Mat4f::perspectiveX( _fov, 1.0f, _front, _back );
      else
         _projMat = Mat4f::orthoScale( 6.0f, 1.0f, _front, _back );
      _dirtyMat ^= PROJ;
   }

   return _projMat;
}

//------------------------------------------------------------------------------
//!
const Mat4f&
Light::viewMatrix() const
{
   if( _dirtyMat & VIEW )
   {
      _viewMat = referential().globalToLocal();
      _dirtyMat ^= VIEW;
   }

   return _viewMat;
}

//------------------------------------------------------------------------------
//!
const Mat4f&
Light::lightMatrix() const
{
   if( _dirtyMat & LIGHT )
   {
      _lightMat = viewportMatrix() * projectionMatrix() * viewMatrix();
      _dirtyMat ^= LIGHT;
   }
   return _lightMat;
}

//------------------------------------------------------------------------------
//!
Mat4f
Light::viewportMatrix() const
{
   return Mat4f(
      0.5f, 0.0f, 0.0f, 0.5f,
      0.0f, 0.5f, 0.0f, 0.5f,
      0.0f, 0.0f, 0.5f, 0.5f,
      0.0f, 0.0f, 0.0f, 1.0f
   );
}

NAMESPACE_END
