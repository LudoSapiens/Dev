/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Widget/PlasmaScreen.h>
#include <Plasma/Manipulator/ManipulatorGroup.h>
#include <Plasma/Intersector.h>
#include <Plasma/Plasma.h>
#include <Plasma/World/World.h>
#include <Plasma/Resource/ResManager.h>

#include <Fusion/VM/VMObjectPool.h>
#include <Fusion/Core/Core.h>
#include <Fusion/Core/Event.h>
#include <Fusion/Core/Key.h>

#include <Base/ADT/StringMap.h>
#include <Base/Dbg/DebugStream.h>
#include <Base/Util/Platform.h>

// Hard-coding renderer (for debug purposes).
#define PLASMA_RENDERER_FORWARD        1
#define PLASMA_RENDERER_FORWARD_FIXED  0
#define PLASMA_RENDERER_FORWARD_HDR    0

#if !defined(PLASMA_RENDERER_DEFERRED)
#  if PLAT_MOBILE
#    define PLASMA_RENDERER_DEFERRED 0
#  else
#    define PLASMA_RENDERER_DEFERRED 0 //1
#  endif
#endif

#if !defined(PLASMA_RENDERER_FORWARD)
#  define PLASMA_RENDERER_FORWARD 1
#endif

#if !defined(PLASMA_RENDERER_FORWARD_FIXED)
#  if PLAT_MOBILE
#    define PLASMA_RENDERER_FORWARD_FIXED 1
#  else
#    define PLASMA_RENDERER_FORWARD_FIXED 0
#  endif
#endif

#if !defined(PLASMA_RENDERER_FORWARD_HDR)
#  if PLAT_MOBILE
#    define PLASMA_RENDERER_FORWARD_HDR 0
#  else
#    define PLASMA_RENDERER_FORWARD_HDR 1
#  endif
#endif

#if PLASMA_RENDERER_DEFERRED
#include <Plasma/Render/DeferredRenderer.h>
#endif

#if PLASMA_RENDERER_FORWARD
#include <Plasma/Render/ForwardRenderer.h>
#endif

#if PLASMA_RENDERER_FORWARD_FIXED
#include <Plasma/Render/ForwardRendererFixed.h>
#endif

#if PLASMA_RENDERER_FORWARD_HDR
#include <Plasma/Render/ForwardRendererHDR.h>
#endif


/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

DBG_STREAM( os_pv, "PlasmaScreen" );

//------------------------------------------------------------------------------
//!
enum
{
   ATTRIB_CAMERA,
   ATTRIB_CAMERA_ORIENTATION,
   ATTRIB_CAMERA_POSITION,
   ATTRIB_CLEAR_DEPTH,
   ATTRIB_BACK_MANIPULATOR,
   ATTRIB_FRONT_MANIPULATOR,
   ATTRIB_MANIPULATOR,
   ATTRIB_ADD_MANIPULATOR,
   ATTRIB_ADD_MANIPULATOR_FRONT,
   ATTRIB_ADD_MANIPULATOR_BACK,
   ATTRIB_CLEAR_MANIPULATORS,
   ATTRIB_REMOVE_MANIPULATOR,
   ATTRIB_RENDER_SIZE,
   ATTRIB_WORLD,
};

StringMap _attributes(
   "camera",              ATTRIB_CAMERA,
   "cameraOrientation",   ATTRIB_CAMERA_ORIENTATION,
   "cameraPosition",      ATTRIB_CAMERA_POSITION,
   "clearDepth",          ATTRIB_CLEAR_DEPTH,
   "backManipulator",     ATTRIB_BACK_MANIPULATOR,
   "frontManipulator",    ATTRIB_FRONT_MANIPULATOR,
   "manipulator",         ATTRIB_MANIPULATOR,
   "addManipulator",      ATTRIB_ADD_MANIPULATOR,
   "addManipulatorFront", ATTRIB_ADD_MANIPULATOR,
   "addManipulatorBack",  ATTRIB_ADD_MANIPULATOR,
   "clearManipulators",   ATTRIB_CLEAR_MANIPULATORS,
   "removeManipulator",   ATTRIB_REMOVE_MANIPULATOR,
   "renderSize",          ATTRIB_RENDER_SIZE,
   "world",               ATTRIB_WORLD,
   ""
);

// TODO: Possibly move the manipulator API into ManipulatorGroup.

//------------------------------------------------------------------------------
//!
int backManipulatorVM( VMState* vm )
{
   PlasmaScreen* screen = (PlasmaScreen*)VM::thisPtr( vm );
   ManipulatorGroup* m  = screen->manipulators();

   if( m->manipulators().empty() ) return 0;

   VM::pushProxy( vm, m->manipulators().back().ptr() );
   return 1;
}

//------------------------------------------------------------------------------
//!
int frontManipulatorVM( VMState* vm )
{
   PlasmaScreen* screen = (PlasmaScreen*)VM::thisPtr( vm );
   ManipulatorGroup* m  = screen->manipulators();

   if( m->manipulators().empty() ) return 0;

   VM::pushProxy( vm, m->manipulators().front().ptr() );
   return 1;
}

//------------------------------------------------------------------------------
//!
int manipulatorVM( VMState* vm )
{
   PlasmaScreen* screen = (PlasmaScreen*)VM::thisPtr( vm );
   int id               = VM::toUInt( vm, 1 );
   ManipulatorGroup* m  = screen->manipulators();

   if( m->manipulators().empty() ) return 0;

   int s = int(m->manipulators().size())-1;
   VM::pushProxy( vm, m->manipulators()[CGM::clamp( id, 0, s )].ptr() );

   return 1;
}

//------------------------------------------------------------------------------
//!
int addManipulatorVM( VMState* vm )
{
   PlasmaScreen* screen = (PlasmaScreen*)VM::thisPtr( vm );
   screen->manipulators()->add( (Manipulator*)VM::toProxy( vm, 1 ), VM::toInt( vm, 2 ) );
   return 0;
}

//------------------------------------------------------------------------------
//!
int addManipulatorFrontVM( VMState* vm )
{
   PlasmaScreen* screen = (PlasmaScreen*)VM::thisPtr( vm );
   screen->manipulators()->addFront( (Manipulator*)VM::toProxy( vm, 1 ) );
   return 0;
}

//------------------------------------------------------------------------------
//!
int addManipulatorBackVM( VMState* vm )
{
   PlasmaScreen* screen = (PlasmaScreen*)VM::thisPtr( vm );
   screen->manipulators()->addBack( (Manipulator*)VM::toProxy( vm, 1 ) );
   return 0;
}

//------------------------------------------------------------------------------
//!
int removeManipulatorVM( VMState* vm )
{
   PlasmaScreen* screen = (PlasmaScreen*)VM::thisPtr( vm );
   screen->manipulators()->remove( (Manipulator*)VM::toProxy( vm, 1 ) );
   return 0;
}

//------------------------------------------------------------------------------
//!
int clearManipulatorsVM( VMState* vm )
{
   PlasmaScreen* screen = (PlasmaScreen*)VM::thisPtr( vm );
   screen->manipulators()->clear();
   return 0;
}

//------------------------------------------------------------------------------
//!
const char* _plasmaScreen_str_ = "plasmaScreen";

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   CLASS PlasmaScreen
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
PlasmaScreen::initialize()
{
   VMObjectPool::registerObject(
      "UI",
      _plasmaScreen_str_,
      stdCreateVM<PlasmaScreen>,
      stdGetVM<PlasmaScreen>,
      stdSetVM<PlasmaScreen>
   );
}

//------------------------------------------------------------------------------
//!
PlasmaScreen::PlasmaScreen():
   _autoSize(true), _cameraId(0)
{
   DBG_BLOCK( os_pv, "PlasmaScreen::PlasmaScreen" );

   // Renderer.

#if   PLASMA_RENDERER_DEFERRED
   _renderer = new DeferredRenderer();
#elif PLASMA_RENDERER_FORWARD_HDR
   _renderer = new ForwardRendererHDR();
#elif PLASMA_RENDERER_FORWARD && PLASMA_RENDERER_FORWARD_FIXED
   if( Core::gfxVersion() == 1 )
   {
      _renderer = new ForwardRendererFixed();
   }
   else
   {
      _renderer = new ForwardRenderer();
   }
#elif PLASMA_RENDERER_FORWARD
   _renderer = new ForwardRenderer();
#elif PLASMA_RENDERER_FORWARD_FIXED
   _renderer = new ForwardRendererFixed();
#endif

   // Receive events.
   intangible( false );

   // Manipulator.
   _manips = new ManipulatorGroup();
   _manips->widget( this );
   _manips->viewport( &_viewport );
}

//------------------------------------------------------------------------------
//!
PlasmaScreen::~PlasmaScreen()
{
}

//------------------------------------------------------------------------------
//!
void PlasmaScreen::renderSize( const Vec2i& size )
{
   if( size.x == -1 || size.y == -1 )
   {
      _autoSize = true;
      _renderer->size( actualSize() );
   }
   else
   {
      _renderer->size( size );
      _autoSize = false;
   }
}

//------------------------------------------------------------------------------
//!
void PlasmaScreen::world( World* w )
{
   if( w == _viewport.world() ) return;
   _viewport.world( w );
   setCamera();
}

//------------------------------------------------------------------------------
//!
void PlasmaScreen::setCamera()
{
   World* w = world();

   if( !w )
   {
      // Reset camera.
      if( _viewport.camera() )
      {
         _viewport.camera(0);
         _manips->onCameraChange();
      }
      return;
   }

   int numWorldCameras = w->numCameras();

   // Change to a unique viewport camera.
   if( _cameraId < 0 || numWorldCameras == 0 )
   {
      initCameras();
      int numVpCameras = int(_cameras.size());
      int camId        = CGM::min( -_cameraId-1, numVpCameras-1 );
      _viewport.camera( _cameras[camId].ptr() );
   }
   else
   {
      // Change to a world camera.
      int camId = CGM::min( _cameraId, numWorldCameras-1 );
      _viewport.camera( w->camera( camId ) );
   }
   // Notify manipulator(s).
   _manips->onCameraChange();
}


//------------------------------------------------------------------------------
//!
void PlasmaScreen::closeWorldCb( Resource<World>* res )
{
   World* w = res->data();
   if( world() != w ) return;
   _viewport.world(0);
   _viewport.camera(0);
}

//------------------------------------------------------------------------------
//!
void PlasmaScreen::render( const RCP<Gfx::RenderNode>& rn )
{
   if(  actualSize().sqrLength() > 0 )
   {
      _renderer->beginFrame();
      _renderer->render( rn, world(), &_viewport );
      _renderer->endFrame();
   }

   _manips->render( rn );
   HotspotContainer::render( rn );
}

//------------------------------------------------------------------------------
//!
void PlasmaScreen::performSetGeometry()
{
   if( _autoSize ) _renderer->size( actualSize() );
   _viewport.region( globalPosition(), actualSize() );
   HotspotContainer::performSetGeometry();
   _manips->onViewportChange();
}

//------------------------------------------------------------------------------
//!
void PlasmaScreen::performSetPosition()
{
   _viewport.region( globalPosition(), actualSize() );
   HotspotContainer::performSetPosition();
   _manips->onViewportChange();
}

//------------------------------------------------------------------------------
//!
void PlasmaScreen::onPointerPress( const Event& ev )
{
   HotspotContainer::onPointerPress( ev );
   _manips->onPointerPress( ev );
}

//------------------------------------------------------------------------------
//!
void PlasmaScreen::onPointerRelease( const Event& ev )
{
   HotspotContainer::onPointerRelease( ev );
   _manips->onPointerRelease( ev );
}

//------------------------------------------------------------------------------
//!
void PlasmaScreen::onPointerMove( const Event& ev )
{
   HotspotContainer::onPointerMove( ev );
   _manips->onPointerMove( ev );
}

//------------------------------------------------------------------------------
//!
void PlasmaScreen::onPointerCancel( const Event& ev )
{
   HotspotContainer::onPointerMove( ev );
   _manips->onPointerCancel( ev );
}

//------------------------------------------------------------------------------
//!
void PlasmaScreen::onPointerScroll( const Event& ev )
{
   HotspotContainer::onPointerScroll( ev );
   _manips->onPointerScroll( ev );
}

//------------------------------------------------------------------------------
//!
void PlasmaScreen::onKeyPress( const Event& ev )
{
   HotspotContainer::onKeyPress( ev );
   _manips->onKeyPress( ev );
}

//------------------------------------------------------------------------------
//!
void PlasmaScreen::onKeyRelease( const Event& ev )
{
   HotspotContainer::onKeyRelease( ev );
   _manips->onKeyRelease( ev );
}

//------------------------------------------------------------------------------
//!
void PlasmaScreen::onChar( const Event& ev )
{
   HotspotContainer::onChar( ev );
   _manips->onChar( ev );
}

//------------------------------------------------------------------------------
//!
void PlasmaScreen::onAccelerate( const Event& ev )
{
   //Widget::onAccelerate( ev );
   _manips->onAccelerate( ev );
}

//------------------------------------------------------------------------------
//!
void PlasmaScreen::initCameras()
{
   // Already initialized?
   if( !_cameras.empty() ) return;

   // Perspective camera.
   RCP<Camera> c = new Camera( RigidBody::KINEMATIC );
   c->position( Vec3f( 0.0f, 1.0f, 10.0f ) );
   c->lookAt( Vec3f(0.0f), Vec3f( 0.0f, 1.0f, 0.0f ) );
   _cameras.pushBack( c );

   // Front.
   c = new Camera( RigidBody::KINEMATIC );
   c->projection( Camera::ORTHO );
   c->position( Vec3f( 0.0f, 1.0f, 100.0f ) );
   c->lookAt( Vec3f( 0.0f, 1.0f, 0.0f ), Vec3f( 0.0f, 1.0f, 0.0f ) );
   _cameras.pushBack( c );

   // Side.
   c = new Camera( RigidBody::KINEMATIC );
   c->projection( Camera::ORTHO );
   c->position( Vec3f( -100.0f, 1.0f, 0.0f ) );
   c->lookAt( Vec3f( 0.0f, 1.0f, 0.0f ), Vec3f( 0.0f, 1.0f, 0.0f ) );
   _cameras.pushBack( c );

   // Top.
   c = new Camera( RigidBody::KINEMATIC );
   c->projection( Camera::ORTHO );
   c->position( Vec3f( 0.0f, 100.0f, 0.0f ) );
   c->lookAt( Vec3f( 0.0f, 1.0f, 0.0f ), Vec3f( 0.0f, 0.0f, -1.0f ) );
   _cameras.pushBack( c );
}

//------------------------------------------------------------------------------
//!
void PlasmaScreen::camera( int c )
{
   _cameraId = c;
   if( world() ) setCamera();
}

//------------------------------------------------------------------------------
//!
const char* PlasmaScreen::meta() const
{
   return _plasmaScreen_str_;
}

//------------------------------------------------------------------------------
//!
void PlasmaScreen::init( VMState* vm )
{
   bool cd;
   if( VM::get( vm, 1, "clearDepth", cd ) ) _renderer->clearDepth( cd );

   if( VM::get( vm, 1, "manipulator" ) )
   {
      _manips->addFront( (Manipulator*)VM::toProxy( vm, -1 ) );
      VM::pop( vm, 1 );
   }

   if( VM::get( vm, 1, "renderSize" ) )
   {
      renderSize( VM::toVec2i( vm, -1 ) );
      VM::pop( vm, 1 );
   }

   if( VM::get( vm, 1, "world" ) )
   {
      world( (World*)VM::toProxy( vm, -1 ) );
      VM::pop( vm, 1 );
   }

   if( VM::get( vm, 1, "camera" ) )
   {
      camera( VM::toInt( vm, -1 ) );
      VM::pop( vm, 1 );
   }

   HotspotContainer::init( vm );
}

//------------------------------------------------------------------------------
//!
bool PlasmaScreen::performGet( VMState* vm )
{
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_CAMERA:
         VM::push( vm, _cameraId );
         return true;
      case ATTRIB_CAMERA_ORIENTATION:
         VM::push( vm, viewport().camera()->orientation() );
         return true;
      case ATTRIB_CAMERA_POSITION:
         VM::push( vm, viewport().camera()->position() );
         return true;
      case ATTRIB_CLEAR_DEPTH:
         VM::push( vm, _renderer->clearDepth() );
         return true;
      case ATTRIB_MANIPULATOR:
         VM::push( vm, this, manipulatorVM );
         return true;
      case ATTRIB_BACK_MANIPULATOR:
         VM::push( vm, this, backManipulatorVM );
         return true;
      case ATTRIB_FRONT_MANIPULATOR:
         VM::push( vm, this, frontManipulatorVM );
         return true;
      case ATTRIB_ADD_MANIPULATOR:
         VM::push( vm, this, addManipulatorVM );
         return true;
      case ATTRIB_ADD_MANIPULATOR_FRONT:
         VM::push( vm, this, addManipulatorFrontVM );
         return true;
      case ATTRIB_ADD_MANIPULATOR_BACK:
         VM::push( vm, this, addManipulatorBackVM );
         return true;
      case ATTRIB_CLEAR_MANIPULATORS:
         VM::push( vm, this, clearManipulatorsVM );
         return true;
      case ATTRIB_REMOVE_MANIPULATOR:
         VM::push( vm, this, removeManipulatorVM );
         return true;
      case ATTRIB_RENDER_SIZE: VM::push( vm, _renderer->size() );
         return true;
      case ATTRIB_WORLD:
         VM::pushProxy( vm, world() );
         return true;
   }

   return HotspotContainer::performGet( vm );
}

//------------------------------------------------------------------------------
//!
bool PlasmaScreen::performSet( VMState* vm )
{
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_CAMERA:
         camera( VM::toInt( vm, 3 ) );
         return true;
      case ATTRIB_CAMERA_ORIENTATION:
      case ATTRIB_CAMERA_POSITION:
         // Read-only.
         return false;
      case ATTRIB_CLEAR_DEPTH:
         _renderer->clearDepth( VM::toBoolean( vm, 3 ) );
         return true;
      case ATTRIB_BACK_MANIPULATOR:
      case ATTRIB_FRONT_MANIPULATOR:
      case ATTRIB_MANIPULATOR:
      case ATTRIB_ADD_MANIPULATOR:
      case ATTRIB_ADD_MANIPULATOR_FRONT:
      case ATTRIB_ADD_MANIPULATOR_BACK:
      case ATTRIB_CLEAR_MANIPULATORS:
      case ATTRIB_REMOVE_MANIPULATOR:
         return false;
      case ATTRIB_RENDER_SIZE:
         renderSize( VM::toVec2i( vm, 3 ) );
         return true;
      case ATTRIB_WORLD:
         world( (World*)VM::toProxy( vm, 3 ) );
         return true;
   }

   return HotspotContainer::performSet( vm );
}

//------------------------------------------------------------------------------
//!
bool PlasmaScreen::isAttribute( const char* name ) const
{
   if( _attributes[ name ] != StringMap::INVALID ) return true;
   return HotspotContainer::isAttribute( name );
}


NAMESPACE_END
