/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/World/Camera.h>

#include <CGMath/CGMath.h>

#include <Base/Dbg/DebugStream.h>


/*==============================================================================
  UNNAME NAMESPACE  
  ==============================================================================*/

UNNAMESPACE_BEGIN

DBG_STREAM( os_can, "Camera" );

UNNAMESPACE_END


NAMESPACE_BEGIN

/*==============================================================================
  CLASS Camera
==============================================================================*/

//------------------------------------------------------------------------------
//!
Camera::Camera( RigidBody::Type bodyType ):
   RigidEntity( CAMERA, bodyType ),
   _focalLength( 2.0f ),
   _fov( 40.0f ),
   _back( 10000.0f ),
   _front( 0.1f ),
   _scale( 10.0f ),
   _shearX( 0.0f ),
   _shearY( 0.0f ),
   _projType( PERSPECTIVE ),
   _fovMode( LARGEST )
{}

//------------------------------------------------------------------------------
//!
Camera::~Camera()
{}

//------------------------------------------------------------------------------
//! 
void 
Camera::projection( ProjectionType type )
{
   _projType = type;
}

//------------------------------------------------------------------------------
//!
void
Camera::focalLength( float length )
{
   _focalLength = length;
}

//------------------------------------------------------------------------------
//!
void
Camera::fov( float fov )
{
   _fov = fov;
}

//------------------------------------------------------------------------------
//!
void
Camera::fovMode( FOVMode fovMode )
{
   _fovMode = fovMode;
}

//------------------------------------------------------------------------------
//!
void
Camera::front( float front )
{
   _front = front;
}

//------------------------------------------------------------------------------
//!
void
Camera::back( float back )
{
   _back  = back;
}

//------------------------------------------------------------------------------
//!
void
Camera::frontAndBack( float front, float back )
{
   _front = front;
   _back  = back;
}

//------------------------------------------------------------------------------
//! 
void
Camera::orthoScale( float scale )
{
   _scale = scale;
}

//------------------------------------------------------------------------------
//! Defines the X and Y shear factors directly.
void
Camera::shear( float shx, float shy )
{
   _shearX = shx;
   _shearY = shy;
}

//------------------------------------------------------------------------------
//! Specifies the shear values using a normal specifing the direction of the
//! viewplane, in camera space.
//! The vector (0,0,1) defines no shearing.
void
Camera::shear( const Vec3f& viewNormal )
{
   _shearX = viewNormal.x;
   _shearY = viewNormal.y;
}

//------------------------------------------------------------------------------
//! Optimizes the front and back distances based on the specified bounding box.
//! This is a little simplistic, but efficient.
//! We simply convert the box's center point into camera space, and
//! clamp to valid range.
void
Camera::optimizeFrontAndBack( const AABBoxf& box )
{
   // 1. Compute the box "radius", which is the distance of any corner to its center.
   Vec3f boxCenter = box.center();
   float boxRadius = (box.corner(0, 0, 0) - boxCenter).length();

   // 2. Transform the center into camera space.
   boxCenter = referential().globalToLocal() * boxCenter;
   boxCenter.z = -boxCenter.z; // Flip the Z to make it positive.

   // 3. Update the front, with a minimum of 0.1.
   double d = boxCenter.z - boxRadius;
   if( d < 0.1 )  d = 0.1;
   front( (float)d );

   // 4. Update the back, with a maximum of 10000.0/0.1 the front.
   d = boxCenter.z + boxRadius;
   d = CGM::min( d, d*10000.0/0.1 );
   d = CGM::max( d, front() + 0.1 );
   back( (float)d );
}

NAMESPACE_END
