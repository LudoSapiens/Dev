/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Manipulator/TouchCameraManipulator.h>

#include <Fusion/VM/VMRegistry.h>
#include <Fusion/VM/VMObjectPool.h>

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

float _flipForward = 1.0f;
float _flipOrbitH  = 1.0f;
float _flipOrbitV  = 1.0f;
float _flipTiltH   = 1.0f;
float _flipTiltV   = 1.0f;

int camManipVM( VMState* vm )
{
   if( VM::isTable( vm, -1 ) )
   {
      int v;
      if( VM::get( vm, -1, "flipForward", v ) )  _flipForward = v ? -1.0f : 1.0f;
      if( VM::get( vm, -1, "flipOrbitH" , v ) )  _flipOrbitH  = v ? -1.0f : 1.0f;
      if( VM::get( vm, -1, "flipOrbitV" , v ) )  _flipOrbitV  = v ? -1.0f : 1.0f;
      if( VM::get( vm, -1, "flipTiltH"  , v ) )  _flipTiltH   = v ? -1.0f : 1.0f;
      if( VM::get( vm, -1, "flipTiltV"  , v ) )  _flipTiltV   = v ? -1.0f : 1.0f;
   }
   return 0;
}

//------------------------------------------------------------------------------
//!
void computePinchData(
   const Pointer& movingPtr,
   const Pointer& otherPtr,
   float          sizeToDouble,
   Vec2f&         trans,
   float&         zoom
)
{
   const Vec2f& p0_pos    = movingPtr.position();
   const Vec2f& p0_oldPos = movingPtr.lastPosition();
   const Vec2f& p1_pos    = otherPtr.position();

   // Determine zoom factor based on distance.
   float oldDist = CGM::sqrt( sqrLength(p1_pos - p0_oldPos) );
   float newDist = CGM::sqrt( sqrLength(p1_pos - p0_pos   ) );

   // Determine translation based on centroid shift.
   // This is equivalent to doing:
   //   trans = (p0_oldPos+p1_pos)/2 - (p0_pos+p1_pos)/2
   trans  = (p0_oldPos - p0_pos)*0.5f;
   zoom   = (sizeToDouble + oldDist - newDist)/sizeToDouble;
}

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   CLASS TouchCameraManipulator
==============================================================================*/

//------------------------------------------------------------------------------
//!
void TouchCameraManipulator::initialize()
{
   VMRegistry::add( "CamManip", camManipVM, VM_CAT_CFG );
   VMObjectPool::registerCreate(
      "UI",
      "touchCameraManipulator",
      stdCreateVM<TouchCameraManipulator>
   );
}

//------------------------------------------------------------------------------
//!
TouchCameraManipulator::TouchCameraManipulator():
   _poi( Vec3f(0.0f) ),
   _grabbed( false )
{

}

//------------------------------------------------------------------------------
//!
TouchCameraManipulator::~TouchCameraManipulator()
{}

//------------------------------------------------------------------------------
//!
bool TouchCameraManipulator::onPointerPress( const Event& ev )
{
   _activePointers.add( ev.pointerID() );
   if( !camera() ) return false;

   if( _activePointers.size() == 1 ) grabCamera();
   return true;
}

//------------------------------------------------------------------------------
//!
bool TouchCameraManipulator::onPointerRelease( const Event& ev )
{
   _activePointers.remove( ev.pointerID() );

   if( _activePointers.empty() ) releaseCamera();

   return true;
}

//------------------------------------------------------------------------------
//!
bool TouchCameraManipulator::onPointerMove( const Event& ev )
{
   if( !camera() || !_grabbed ) return false;

   Pointer& ptr = ev.pointer();

   // Orbiting.
   if( _activePointers.size() == 1 && ptr.pressedState() == 2 )
   {
      const Vec2f& curPos = ev.position();
      const Vec2f& oldPos = ptr.lastPosition();
      Vec2f v             = oldPos-curPos;
      orbit( v );
   }
   if( _activePointers.size() == 1 && ptr.pressedState() == 8 )
   {
      const Vec2f& curPos = ev.position();
      const Vec2f& oldPos = ptr.lastPosition();
      Vec2f v             = oldPos-curPos;
      translateLocal( v, 1.0f );
   }
   // Forward or pan.
   if( _activePointers.size() == 2 )
   {
      uint opid     = getOtherActivePointerID( ev.pointerID() );
      Pointer& optr = Core::pointer( opid );
      handlePinch( ptr, optr );
   }

   return true;
}

//------------------------------------------------------------------------------
//!
bool TouchCameraManipulator::onPointerScroll( const Event& ev )
{
   Camera* cam = camera();

   if( !cam ) return false;

   if( cam->projection() == Camera::PERSPECTIVE )
   {
      grabCamera();
      float sizeToDouble = viewport()->size().length() * 0.5f;
      float zoom         = (sizeToDouble + 10.0f*ev.valueFloat())/sizeToDouble;
      translateLocal( Vec2f(0.0f), zoom );
   }
   else
   {
      float scale = cam->orthoScale();
      float f     = CGM::pow( 1.1f, ev.valueFloat() );
      scale       = scale * f;
      scale       = CGM::clamp( scale, 0.5f, 1000.0f );
      cam->orthoScale( scale );
   }

   return true;
}

//------------------------------------------------------------------------------
//!
bool TouchCameraManipulator::onPointerCancel( const Event&  ev )
{
   _activePointers.remove( ev.pointerID() );
   if( _activePointers.empty() ) releaseCamera();

   return false;
}

//------------------------------------------------------------------------------
//!
void
TouchCameraManipulator::grabCamera()
{
   _grabbed       = true;
   _ref           = camera()->referential();
   _primaryAxis   = Vec3f(0.0f, 1.0f, 0.0f);
   _secondaryAxis = _ref.orientation().getAxisX();

   Vec3f y        = _ref.orientation().getAxisY();
   _direction     = _primaryAxis.dot( y ) < 0.0f ? -1.0f : 1.0f;

   _offset        = Vec3f(0.0f);
   _angles        = Vec2f(0.0f);
}

//------------------------------------------------------------------------------
//!
void TouchCameraManipulator::releaseCamera()
{
   _grabbed = false;
}

//------------------------------------------------------------------------------
//!
void TouchCameraManipulator::updateCamera()
{
   // Rotate the referential.
   Reff ref = _ref.getRotated( _poi, _primaryAxis, _angles.x );

   // Rotate the SecondaryAxis.
   Quatd quat = Quatd::axisAngle( _primaryAxis, _angles.x );
   Vec3f secondaryAxis = quat.toMatrix() * _secondaryAxis;

   // Rotate the referential.
   ref = ref.getRotated( _poi, secondaryAxis, _angles.y );
   ref.orientation().normalize();
   ref.translateLocal( _offset );
   camera()->referential( ref );
}

//------------------------------------------------------------------------------
//!
void
TouchCameraManipulator::orbit( const Vec2f& pixels )
{
   Vec2f angles = pixels * (CGConstf::pi()/256.0f);
   angles.x    *= _direction * _flipOrbitH;
   angles.y    *= -1.0f * _flipOrbitV;

   _angles += angles;

   updateCamera();
}

//------------------------------------------------------------------------------
//! Moves the camera along its local referential according to a specified
//! pan vector (in pixels) and zoom factor (0=get to POI, 2=double the distance).
void
TouchCameraManipulator::translateLocal( const Vec2f& pan, const float zoom )
{
   float curRadius = CGM::length( _poi - camera()->position() );
   float f         = CGM::atan( CGM::degToRad( camera()->fov() * 0.5f ) ) * 2.0f * curRadius;

   int a           = viewport()->fovAxis();
   Vec2f offset    = pan;
   offset         *= f / viewport()->size()(a);

   float radius    = curRadius * zoom;
   radius          = CGM::clamp( radius, 0.5f, 1000.0f );

   _offset        += Vec3f( offset.x, offset.y, radius-curRadius );
   updateCamera();
}

//------------------------------------------------------------------------------
//!
void
TouchCameraManipulator::handlePinch( const Pointer& ptr, const Pointer& optr )
{
   float sizeToDouble = viewport()->size().length() * 0.5f;
   Vec2f trans;
   float zoom;
   computePinchData( ptr, optr, sizeToDouble, trans, zoom );
   translateLocal( trans, zoom );
}

//------------------------------------------------------------------------------
//!
uint
TouchCameraManipulator::getOtherActivePointerID( uint pid )
{
   for( auto cur = _activePointers.begin(); cur != _activePointers.end(); ++cur )
   {
      if( *cur != pid ) return *cur;
   }
   return pid;
}

NAMESPACE_END
