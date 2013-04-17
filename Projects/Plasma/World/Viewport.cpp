/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/World/Viewport.h>

#include <CGMath/CGMath.h>

#include <Base/Dbg/DebugStream.h>


/*==============================================================================
  UNNAME NAMESPACE
  ==============================================================================*/

UNNAMESPACE_BEGIN

DBG_STREAM( os_can, "Viewport" );

UNNAMESPACE_END


NAMESPACE_BEGIN

/*==============================================================================
  CLASS Camera
==============================================================================*/

//------------------------------------------------------------------------------
//!
Viewport::Viewport()
  : _aspect( 1.0f ),
    _size(0.0f),
    _pos(0.0f),
    _camera( 0 )
{}

//------------------------------------------------------------------------------
//!
Viewport::~Viewport()
{}

//------------------------------------------------------------------------------
//!
int Viewport::fovAxis() const
{
   if( !_camera ) return 1;
   switch( _camera->fovMode() )
   {
      case Camera::FOV_X:    return 0;
      case Camera::FOV_Y:    return 1;
      case Camera::SMALLEST: return _size.x < _size.y ? 0 : 1;
      case Camera::LARGEST:  return _size.x > _size.y ? 0 : 1;
   }
   return 1;
}

//------------------------------------------------------------------------------
//!
Mat4f
Viewport::cameraMatrix() const
{
   return viewportMatrix() * projectionMatrix() * viewMatrix();
}

//------------------------------------------------------------------------------
//!
Mat4f
Viewport::projectionMatrix() const
{
   CHECK( _camera != 0 );

   if( _camera->projection() == Camera::PERSPECTIVE )
   {
      float yOverX = float(_size.y)/_size.x;
      switch( _camera->fovMode() )
      {
         case Camera::FOV_X:
            return Mat4f::perspectiveX(
               _camera->fov()   , yOverX*_aspect,
               _camera->front() , _camera->back(),
               _camera->shearX(), _camera->shearY()
            );
         case Camera::FOV_Y:
            return Mat4f::perspectiveY(
               _camera->fov()   , yOverX*_aspect,
               _camera->front() , _camera->back(),
               _camera->shearX(), _camera->shearY()
            );
         case Camera::SMALLEST:
            return Mat4f::perspectiveSmallest(
               _camera->fov()   , yOverX, _aspect,
               _camera->front() , _camera->back(),
               _camera->shearX(), _camera->shearY()
            );
         case Camera::LARGEST:
         default:
            return Mat4f::perspectiveLargest(
               _camera->fov()   , yOverX, _aspect,
               _camera->front() , _camera->back(),
               _camera->shearX(), _camera->shearY()
            );
      }
   }
   else
   {
      return Mat4f::orthoScale(
         _camera->orthoScale(), _aspect*_size.y/_size.x,
         _camera->front()     , _camera->back(),
         _camera->shearX()    , _camera->shearY()
      );
   }
}

//------------------------------------------------------------------------------
//!
Mat4f
Viewport::viewMatrix() const
{
   CHECK( _camera != 0 );
   return _camera->referential().globalToLocal();
}

//------------------------------------------------------------------------------
//!
Mat4f
Viewport::viewportMatrix() const
{
   return Mat4f::viewport( position(), size() );
}

//------------------------------------------------------------------------------
//! Compute a direction vector of the corresponding
//! given position in viewport.
Vec3f
Viewport::direction( const Vec2f& pos ) const
{
   CHECK( _camera != 0 );

   // Compute x, y, z vector.
   Vec3f x, y, z;
   _camera->orientation().getAxes( x, y, z );

   // Compute new position according to the center of the viewport.
   float yOverX = float(_size.y) / _size.x;
   float sx, sy;
   switch( _camera->fovMode() )
   {
      case Camera::FOV_X:
         Mat4f::perspectiveX_SxSy( _camera->fov(), yOverX*_aspect, sx, sy );
         break;
      case Camera::FOV_Y:
         Mat4f::perspectiveY_SxSy( _camera->fov(), yOverX*_aspect, sx, sy );
         break;
      case Camera::SMALLEST:
         Mat4f::perspectiveSmallest_SxSy( _camera->fov(), yOverX, _aspect, sx, sy );
         break;
      case Camera::LARGEST:
      default:
         Mat4f::perspectiveLargest_SxSy( _camera->fov(), yOverX, _aspect, sx, sy );
   }
   float r    = _aspect * yOverX;
   float shx  = _camera->shearX() * r;
   float shy  = _camera->shearY();

   Vec2f npos = ( pos - _pos ) * 2.0f - _size;
   npos      /= _size;
   npos      += Vec2f( shx, shy );
   npos      /= Vec2f( sx, sy );

   return npos.x * x + npos.y * y - z;
}

//------------------------------------------------------------------------------
//!
Frustumf
Viewport::frustum() const
{
   CHECK( _camera != 0 );

   float fov = CGM::degToRad( _camera->fov() * 0.5f );
   float fovx;
   float fovy;
   int fova = fovAxis();

   if( fova == 0 )
   {
      fovx = fov;
      fovy = CGM::atan( ( _size.y*_aspect / _size.x ) * CGM::tan( fov ) );
   }
   else
   {
      fovy = fov;
      fovx = CGM::atan( ( _size.x / (_size.y*_aspect) ) * CGM::tan( fov ) );
   }

   return Frustumf(
         _camera->referential(),
         _camera->front(), 1000.0f,//_camera->back(),
         fovx, fovy,
         _camera->shearX(), _camera->shearY()
   );
}

NAMESPACE_END
