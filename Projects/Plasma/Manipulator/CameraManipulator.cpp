/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Manipulator/CameraManipulator.h>
#include <Plasma/Intersector.h>
#include <Plasma/World/Camera.h>

#include <Fusion/Core/Core.h>
#include <Fusion/Core/Event.h>
#include <Fusion/Core/Key.h>
#include <Fusion/VM/VMRegistry.h>
#include <Fusion/VM/VMObjectPool.h>

#include <CGMath/CGMath.h>

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

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
  CLASS CameraManipulator
==============================================================================*/

//------------------------------------------------------------------------------
//!
void CameraManipulator::initialize()
{
   VMRegistry::add( "CamManip", camManipVM, VM_CAT_CFG );

   VMObjectPool::registerCreate(
      "UI",
      "cameraManipulator",
      stdCreateVM<CameraManipulator>
   );
}

//------------------------------------------------------------------------------
//!
CameraManipulator::CameraManipulator()
   : _primaryAxis( 0, 1, 0 ),
     _poi( Vec3f::zero() ),
     _isLocked( true ),
     _mode( 0 )
{
}

//------------------------------------------------------------------------------
//!
CameraManipulator::~CameraManipulator()
{}

//------------------------------------------------------------------------------
//!
void
CameraManipulator::lookAt
( float px, float py, float pz,
  float ax, float ay, float az,
  float ux, float uy, float uz )
{
   if( camera() == NULL )
   {
      return;
   }

   camera()->position( Vec3f(px, py, pz) );
   camera()->lookAt( Vec3f(ax, ay, az), Vec3f(ux, uy, uz) );

   _poi(0) = ax;
   _poi(1) = ay;
   _poi(2) = az;
}

//------------------------------------------------------------------------------
//!
void
CameraManipulator::pointOfInterest( const Vec3f& pos )
{
   _poi = pos;
}

//------------------------------------------------------------------------------
//!
void
CameraManipulator::primaryAxis( const Vec3f& axis )
{
   _primaryAxis = axis;
   _primaryAxis.normalize();
}

//------------------------------------------------------------------------------
//!
void
CameraManipulator::render( const RCP<Gfx::RenderNode>& /*rn*/ )
{
#if GFX
   if(  camera().isNull() )
   {
      return;
   }

   const Mat4f& m = camera()->cameraMatrix();

   // 3d pos for referential.
   Vec3f pos = camera()->position() -
      camera()->referential().orientation().getAxisZ();

   // Compute scaling factor.
   Vec3f csPos = camera()->viewingMatrix() * pos;
   float sf = ( -50 * csPos.z ) /
      ( viewport()->size().y *
        camera()->projectionMatrix()._11 );

   // Compute vertices position.
   Vec3f o = m | pos;
   Vec3f x = m | ( pos + Vec3f( sf,   0,   0 ) );
   Vec3f y = m | ( pos + Vec3f(   0, sf,   0 ) );
   Vec3f z = m | ( pos + Vec3f(   0,   0, sf ) );

   // Manip pos.
   Vec3f mpos(
      viewport()->position().x +
      viewport()->size().x - 40,
      viewport()->position().y + 40,
      0
   );
   Vec3f d = o - mpos;

   o -= d;
   x -= d;
   y -= d;
   z -= d;

   // Draw
   GfxState::set( TextureObject() );

   float c1 = 0.5f;
   float c2 = 0.2f;

   GL::lineWidth( 5 );
   GL::begin( GL::LINES );
   {
      GL::color3f( c1, c2, c2 );
      GL::vertex2fv( o.ptr() );
      GL::vertex2fv( x.ptr() );

      GL::color3f( c2, c1, c2 );
      GL::vertex2fv( o.ptr() );
      GL::vertex2fv( y.ptr() );

      GL::color3f( c2, c2, c1 );
      GL::vertex2fv( o.ptr() );
      GL::vertex2fv( z.ptr() );
   }
   GL::end();

   GL::lineWidth( 1 );
   GL::begin( GL::LINES );
   {
      GL::color3f( 1, 0, 0 );
      GL::vertex2fv( o.ptr() );
      GL::vertex2fv( x.ptr() );

      GL::color3f( 0, 1, 0 );
      GL::vertex2fv( o.ptr() );
      GL::vertex2fv( y.ptr() );

      GL::color3f( 0, 0, 1 );
      GL::vertex2fv( o.ptr() );
      GL::vertex2fv( z.ptr() );
   }
   GL::end();
#endif
}

//------------------------------------------------------------------------------
//!
void
CameraManipulator::onCameraChange()
{
   _primaryAxis = Vec3f( 0, 1, 0 );
   _poi = Vec3f::zero();
   //_isLocked = true;
   //_mode = 0;
}

//------------------------------------------------------------------------------
//!
bool
CameraManipulator::onPointerPress( const Event& ev )
{
   _mode = 0;

   if( !camera() || !viewport()->world() ) return false;

   // Picking.
   if( ev.value() == 1 && Core::isKeyPressed( Key::SHIFT ) )
   {
      Rayf ray(
         camera()->position(),
         viewport()->direction( ev.position() )
      );
      Intersector::Hit hit;
      if( Intersector::trace( viewport()->world(), ray, hit ) )
      {
         pointOfInterest( hit._pos );
         return true;
      }
   }

   _mode          = ev.value();
   _grabPos       = ev.position();
   _ref           = camera()->referential();
   _grabPoi       = _poi;
   _secondaryAxis = _ref.orientation().getAxisX();

   if( Core::isKeyPressed( Key::ALT ) )
   {
      const int altModes[] = { 1, 2, 3, 2 };
      _mode = altModes[_mode % 4];
   }

   Vec3f y = _ref.orientation().getAxisY();

   if( _primaryAxis.dot( y ) < 0 )
   {
      _direction = -1.0f;
   }
   else
   {
      _direction =  1.0f;
   }

   const Mat4f& mat = viewport()->cameraMatrix();
   Vec3f tc  = mat | _poi;
   Vec3f tc2 = mat | ( _poi + y );
   _scale = 1.0f / ( tc2 - tc ).length();
   return true;
}

//------------------------------------------------------------------------------
//!
bool
CameraManipulator::onPointerRelease( const Event& /*ev*/ )
{
   _mode = 0;
   return false;
}

//------------------------------------------------------------------------------
//!
bool
CameraManipulator::onPointerMove( const Event& ev )
{
   if( _mode == 0 ) return false;

   switch( _mode )
   {
      case 1:
      {
         float radP = CGM::degToRad( _grabPos.x - ev.position().x );
         float radS = CGM::degToRad( ev.position().y - _grabPos.y );
         radP *= _direction;
         if( _isLocked  )
         {
            orbit( radP*0.75f, radS*0.75f );
         }
         else
         {
            tilt( radP*0.25f, radS*0.25f );
         }
         return true;
      } break;
      case 2: forward( ev.position() );
         return true;
      case 3: pan( ev.position() );
         return true;
      default:;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
bool
CameraManipulator::onPointerScroll( const Event& ev )
{
   Camera* cam = camera();
   if( cam )
   {
      _ref = cam->referential();
      if( cam->projection() == Camera::PERSPECTIVE )
      {
         Vec2f pos = _grabPos + Vec2f( 0.0f, ev.valueFloat() );
         forward( pos );
      }
      else
      {
         float scale = cam->orthoScale();
         float f     = CGM::pow( 1.1f, ev.valueFloat() );
         scale       = scale * f;
         scale       = CGM::clamp( scale, 0.5f, 100.0f );
         cam->orthoScale( scale );
      }
   }
   return false;
}

//------------------------------------------------------------------------------
//!
bool
CameraManipulator::onAccelerate( const Event& ev )
{
   Vec3f d( ev.dx(), ev.dy(), ev.dz() );
   //_accel = d - ((d*0.1f)+(_accel*0.9f));
   _accel = (d * 0.1f) + (_accel * (1.0f - 0.1f));
   if( !camera() ) return false;

   //StdErr << "Gravity: " << _accel << "\n";
   //camera()->position( camera()->position() + _accel*0.1f );
   return false;
}

//------------------------------------------------------------------------------
//! Lock or unlock the camera motion
/*! If true, every movement of the camera is center on the point of interest.
 *  If false, the point of interest move with the camera.
 */
void
CameraManipulator::lock( bool lock )
{
   _isLocked = lock;
}

//------------------------------------------------------------------------------
//!
void
CameraManipulator::forward( const Vec2f& pos )
{
   float distance = pos.y - _grabPos.y;
   distance *= -0.5f;
   switch( camera()->fovMode() )
   {
      case Camera::FOV_X:
         distance *= camera()->fov() / viewport()->size().x;
         break;
      case Camera::FOV_Y:
         distance *= camera()->fov() / viewport()->size().y;
         break;
      case Camera::SMALLEST:
         distance *= camera()->fov() / viewport()->size().min();
         break;
      case Camera::LARGEST:
      default:
         distance *= camera()->fov() / viewport()->size().max();
         break;
   }
   distance *= _flipForward;

   if( _isLocked )
   {
      distance = 1.0f - 1.0f/pow( 2.0f, distance );

      if( distance > 0.99f )
      {
         distance = 0.99f;
      }

      camera()->position( _ref.position()*( 1.0f - distance ) + _poi * distance );
   }
   else
   {
      Vec3f vec = _ref.orientation().getAxisZ() * distance * 10.0f;

      camera()->position( _ref.position() + vec );
      _poi = _grabPoi + vec;
   }
}

//------------------------------------------------------------------------------
//!
void
CameraManipulator::orbit( float primAngle, float secAngle )
{
   primAngle *= _flipOrbitH;
   secAngle  *= _flipOrbitV;

   // Rotate the referential.
   camera()->referential( _ref.getRotated( _poi, _primaryAxis, primAngle ) );

   // Rotate the SecondaryAxis.
   Quatd quat = Quatd::axisAngle( _primaryAxis, primAngle );

   Vec3f secondaryAxis = quat.toMatrix() * _secondaryAxis;

   // Rotate the referential.
   Reff ref = camera()->referential().getRotated( _poi, secondaryAxis, secAngle );
   ref.orientation().normalize();
   camera()->referential( ref );
}

//------------------------------------------------------------------------------
//!
void
CameraManipulator::tilt( float primAngle, float secAngle )
{
   primAngle *= _flipTiltH;
   secAngle  *= _flipTiltV;

   // TODO
   // Clamping of secondary angle...

   // Rotate the referential.
   camera()->referential( _ref.getRotated( _primaryAxis, primAngle ) );

   // Calculate the rotation quaternion and it matrix.
   Quatd quat = Quatd::axisAngle( _primaryAxis, primAngle );
   Mat4d mat = quat.toMatrix();

   // Rotate the SecondaryAxis.
   Vec3f secondaryAxis = mat * _secondaryAxis;

   // Rotate the referential.
   Reff ref = camera()->referential().getRotated( secondaryAxis, secAngle );
   ref.orientation().normalize();
   camera()->referential( ref );

   // Move the POI.
   _poi = mat * ( _grabPoi - _ref.position() );

   quat = Quatd::axisAngle( secondaryAxis, secAngle );
   _poi = quat.toMatrix()*_poi;
   _poi += _ref.position();
}

//------------------------------------------------------------------------------
//!
void
CameraManipulator::pan( const Vec2f& pos )
{
   float lateral  = _grabPos.x - pos.x;
   float vertical = pos.y - _grabPos.y;

   lateral  *= _scale;
   vertical *= _scale;

   if( _isLocked )
   {
      // move camera position
      Vec3f x, y, z;
      _ref.orientation().getAxes( x, y, z );

      Vec3f vec = x * lateral - y * vertical;
      camera()->position( _ref.position() + vec );
   }
   else
   {
      Vec3f x, y, z;
      _ref.orientation().getAxes( x, y, z );

      Vec3f vec = x * lateral - y * vertical;

      camera()->position( _ref.position() + vec );
      _poi = _grabPoi + vec;
   }
}

NAMESPACE_END
