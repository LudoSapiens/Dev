/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Manipulator/FlyByManipulator.h>
#include <Plasma/World/Camera.h>

#include <Fusion/Core/Animator.h>
#include <Fusion/Core/Core.h>
#include <Fusion/Core/Event.h>
#include <Fusion/Core/Key.h>
#include <Fusion/VM/VMObjectPool.h>
#include <Fusion/VM/VMRegistry.h>

#include <CGMath/CGMath.h>
#include <CGMath/Geom.h>

#include <Base/ADT/StringMap.h>

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

const uint INVALID_POINTER = (uint)-1;

const char* _flyByManip_meta = "FlyByManipulator";

float _flipVertical = 1.0f;
int   _defaultMode  = FlyByManipulator::MODE_WALKING;

int flyByManipConfVM( VMState* vm )
{
   if( VM::isTable( vm, -1 ) )
   {
      bool b;
      if( VM::get( vm, -1, "flipVertical", b ) )  _flipVertical = b ? -1.0f : 1.0f;
      VM::get( vm, -1, "mode", _defaultMode );
   }
   return 0;
}

//------------------------------------------------------------------------------
//!
enum
{
   ATTRIB_MODE
};

//------------------------------------------------------------------------------
//!
StringMap _attributes(
   "mode",  ATTRIB_MODE,
   ""
);

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
  CLASS CameraAnimator
==============================================================================*/
class FlyByManipulator::CameraAnimator:
   public Animator
{
public:

   /*----- methods -----*/

   CameraAnimator( FlyByManipulator* manip );
   virtual ~CameraAnimator();

   bool exec( double time, double delta );

   inline void performRefresh( bool v ) { _performRefresh = v; }

protected:

   /*----- data members -----*/

   FlyByManipulator*  _manip;
   bool               _performRefresh;

private:
}; //class CameraAnimator

//------------------------------------------------------------------------------
//!
FlyByManipulator::CameraAnimator::CameraAnimator( FlyByManipulator* manip ):
   _manip( manip ),
   _performRefresh( false )
{
}

//------------------------------------------------------------------------------
//!
FlyByManipulator::CameraAnimator::~CameraAnimator()
{
}

//------------------------------------------------------------------------------
//!
bool
FlyByManipulator::CameraAnimator::exec( double /*time*/, double delta )
{
   //StdErr << "Exec" << nl;
   //if( _performRefresh )  _manip->refreshCamera();

   Vec3f t;
   switch( _manip->_mode )
   {
      case MODE_FLYING:
         t = _manip->camera()->orientation() * _manip->_velocity * (_manip->_velScale * float(delta));
         break;
      case MODE_WALKING:
         t = Quatf::axisAngle( Vec3f(0,1,0), _manip->_theta ) * _manip->_velocity * (_manip->_velScale * float(delta));
         break;
   }
   //t *= 0.1f;
   _manip->_position += t;

   _manip->camera()->referential( _manip->toRef() );

   return false;
}


/*==============================================================================
  CLASS FlyByManipulator
==============================================================================*/

//------------------------------------------------------------------------------
//!
void FlyByManipulator::initialize()
{
   VMRegistry::add( "FlyByManip", flyByManipConfVM, VM_CAT_CFG );
   VMObjectPool::registerObject(
      "UI",
      _flyByManip_meta,
      stdCreateVM<FlyByManipulator>,
      stdGetVM<FlyByManipulator>,
      stdSetVM<FlyByManipulator>
   );
   //VMRegistry::add( initVM, VM_CAT_APP );
}

//------------------------------------------------------------------------------
//!
const char*
FlyByManipulator::meta() const
{
   return _flyByManip_meta;
}

//------------------------------------------------------------------------------
//!
void
FlyByManipulator::init( VMState* vm )
{
   //DBG_BLOCK( os_fbm, "FlyByManipulator::init()" );
   if( VM::isTable( vm, -1 ) )
   {
      VM::push( vm );  // Start iterating at index 0 (nil).
      while( VM::next( vm, -2 ) )
      {
         performSet( vm );  // Forward assignments to performSet().
         VM::pop( vm, 1 );  // Pop the value, but keep the key.
      }

      // Creation-only parameters (if any).
   }
}

//------------------------------------------------------------------------------
//!
bool
FlyByManipulator::performGet( VMState* vm )
{
   const char* attr = VM::toCString( vm, 2 );
   switch( _attributes[attr] )
   {
      case ATTRIB_MODE:
         VM::push( vm, mode() );
         return true;
      default: break;
   }
   return Manipulator::performGet( vm );
}

//------------------------------------------------------------------------------
//!
bool
FlyByManipulator::performSet( VMState* vm )
{
   const char* attr = VM::toCString( vm, 2 );
   switch( _attributes[attr] )
   {
      case ATTRIB_MODE:
         mode( VM::toInt( vm, 3 ) );
         return true;
      default: break;
   }
   return Manipulator::performSet( vm );
}

//------------------------------------------------------------------------------
//!
FlyByManipulator::FlyByManipulator():
   _mode( _defaultMode ),
   _thetaF( 1.0f/128.0f ),
   _phiF(   1.0f/128.0f ),
   _phiMin( 0.0f ),
   _phiMax( CGConstf::pi() ),
   _velocity( 0.0f ),
   _velScale( 5.0f ),
   _animActive( false ),
   _pointer( INVALID_POINTER )
{
   _animator = new CameraAnimator( this );
}

//------------------------------------------------------------------------------
//!
FlyByManipulator::~FlyByManipulator()
{
   if( _animActive )
   {
      Core::removeAnimator( _animator.ptr() );
   }
}

//------------------------------------------------------------------------------
//!
void
FlyByManipulator::onCameraChange()
{
   if( camera() )
   {
      readFrom( camera()->referential() );
      camera()->referential( toRef() );
      if( !_animActive )
      {
         _animActive = true;
         Core::addAnimator( _animator.ptr() );
      }
   }
   else
   {
      if( _animActive )
      {
         Core::removeAnimator( _animator.ptr() );
         _animActive = false;
      }
   }
}

//------------------------------------------------------------------------------
//!
bool
FlyByManipulator::onPointerPress( const Event& /*ev*/ )
{
   //StdErr << "PP " << _pointer << " W:" << (void*)widget() << " grabbed:" << (void*)Core::pointer(ev.pointerID()).grabbedWith() << nl;
   return false;
}

//------------------------------------------------------------------------------
//!
bool
FlyByManipulator::onPointerRelease( const Event& ev )
{
   //StdErr << "PR " << _pointer << " W:" << (void*)widget() << " grabbed:" << (void*)Core::pointer(ev.pointerID()).grabbedWith() << nl;
   if( _pointer == INVALID_POINTER )
   {
      //StdErr << "Tempting grab" << nl;
      if( Core::grabPointer( ev.pointerID(), widget() ) )
      {
         _pointer = ev.pointerID();
      }
      else
      {
         StdErr << "ERROR - Could not grab pointer in FlyByManipulator." << nl;
      }
   }
   else
   {
      // Core already released the pointer.
      CHECK( Core::pointer(ev.pointerID()).grabbedWith() == NULL );
      _pointer = INVALID_POINTER;
   }
   //StdErr << "--> " << (void*)Core::pointer(ev.pointerID()).grabbedWith() << nl;
   return false;
}

//------------------------------------------------------------------------------
//!
bool
FlyByManipulator::onPointerMove( const Event& ev )
{
   //StdErr << "PM " << _pointer << " W:" << (void*)widget() << " grabbed:" << (void*)Core::pointer(ev.pointerID()).grabbedWith() << nl;
   if( _pointer == INVALID_POINTER )  return false;

   Vec2f d = ev.position() - Core::pointer(ev.pointerID()).lastPosition();

   // Smoothing the mouse.
   //d *= Vec2f( 1.0f/1024.0f );
   //d = CGM::smoothRampC2( d );
   //d *= Vec2f( 1024.0f );

   _theta -= _thetaF * d.x;
   _theta = CGM::fmod( _theta, CGConstf::pi2() );

   _phi += _phiF * d.y * _flipVertical;
   _phi = CGM::clamp( _phi, _phiMin, _phiMax );

   camera()->referential( toRef() );

   return true;
}

//------------------------------------------------------------------------------
//!
bool
FlyByManipulator::onPointerScroll( const Event& /*ev*/ )
{
   if( camera() )
   {
   }
   return false;
}

//------------------------------------------------------------------------------
//!
bool
FlyByManipulator::onKeyPress( const Event& ev )
{
   if( _pointer == INVALID_POINTER )  return false;
   switch( ev.value() )
   {
      case Key::A :
         _velocity.x = -1.0f;
         break;
      case Key::D :
         _velocity.x = +1.0f;
         break;
      case Key::E :
         _velocity.y = +1.0f;
         break;
      case Key::Q :
         _velocity.y = -1.0f;
         break;
      case Key::S :
         _velocity.z = +1.0f;
         break;
      case Key::W :
         _velocity.z = -1.0f;
         break;
      default:
         return false;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
bool
FlyByManipulator::onKeyRelease( const Event& ev )
{
   switch( ev.value() )
   {
      case Key::A :
         _velocity.x = 0.0f;
         break;
      case Key::D :
         _velocity.x = 0.0f;
         break;
      case Key::E :
         _velocity.y = 0.0f;
         break;
      case Key::Q :
         _velocity.y = 0.0f;
         break;
      case Key::S :
         _velocity.z = 0.0f;
         break;
      case Key::W :
         _velocity.z = 0.0f;
         break;
      default:
         return false;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
void
FlyByManipulator::readFrom( const Reff& ref )
{
   // Update the internal state using the specified referential.
   _position = ref.position();
   Vec3f view = ref.orientation().getAxisZ();
   view.toThetaPhi( _theta, _phi );
   //StdErr << "Reading " << ref << " --> " << _theta << "," << _phi << nl;
}

//------------------------------------------------------------------------------
//!
Reff
FlyByManipulator::toRef() const
{
   // Converts the internal state into a referential.
   Quatf q = Quatf::axisAngle( Vec3f(0.0f, 1.0f, 0.0f), _theta ) *
             Quatf::axisAngle( Vec3f(1.0f, 0.0f, 0.0f), _phi - CGConstf::pi_2() );
   Reff ref;
   ref.position( _position );
   ref.orientation( q );
   //StdErr << "Writing " << _theta << "," << _phi << " --> "  << ref << nl;
   return ref;
}

NAMESPACE_END
