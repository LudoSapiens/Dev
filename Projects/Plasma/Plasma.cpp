/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Plasma.h>
#include <Plasma/Action/Action.h>
#include <Plasma/Geometry/Material.h>
#include <Plasma/Procedural/Component.h>
#include <Plasma/Procedural/ProceduralGeometry.h>
#include <Plasma/Procedural/ProceduralMaterial.h>
#include <Plasma/Procedural/ProceduralMesh.h>
#include <Plasma/Procedural/ProceduralWorld.h>
#include <Plasma/Procedural/ProceduralSkeleton.h>
#include <Plasma/Procedural/ProceduralAnimation.h>
#include <Plasma/Procedural/ProceduralAnimationGraph.h>
#include <Plasma/DataFlow/ProceduralDataFlow.h>
#include <Plasma/DataFlow/DFAnimNodes.h>
#include <Plasma/DataFlow/DFBakers.h>
#include <Plasma/DataFlow/DFGeomNodes.h>
#include <Plasma/DataFlow/DFGeomGenerator.h>
#include <Plasma/DataFlow/DFGeomModifier.h>
#include <Plasma/DataFlow/DFImageNodes.h>
#include <Plasma/DataFlow/DFImageGenerator.h>
#include <Plasma/DataFlow/DFPolygonNodes.h>
#include <Plasma/DataFlow/DFCollisionNodes.h>
#include <Plasma/DataFlow/DFNodeAttr.h>
#include <Plasma/DataFlow/DFStrokesNodes.h>
#include <Plasma/DataFlow/DFStrokesTool.h>
#include <Plasma/DataFlow/DFTrees.h>
#include <Plasma/DataFlow/DFWorldNodes.h>
#include <Plasma/Manipulator/Controller.h>
#include <Plasma/Manipulator/Manipulator.h>
#include <Plasma/Manipulator/MultiCharacterController.h>
#include <Plasma/Manipulator/CameraManipulator.h>
#include <Plasma/Manipulator/TouchCameraManipulator.h>
#include <Plasma/Manipulator/FlyByManipulator.h>
#include <Plasma/Render/DebugGeometry.h>
#include <Plasma/Resource/ResExporter.h>
#include <Plasma/Resource/ResManager.h>
#include <Plasma/Resource/ResourceVM.h>
#include <Plasma/Stimulus/EventStimuli.h>
#include <Plasma/Stimulus/Orders.h>
#include <Plasma/Widget/DFViewer.h>
#include <Plasma/Widget/DFScreen.h>
#include <Plasma/Widget/PlasmaBrowser.h>
#include <Plasma/Widget/PlasmaScreen.h>
#include <Plasma/World/Brain.h>

#include <Plasma/Render/PlasmaBaker.h>

#include <Fusion/Core/Core.h>
#include <Fusion/Resource/ResManager.h>
#include <Fusion/VM/VMRegistry.h>

#include <Base/ADT/StringMap.h>
#include <Base/Dbg/DebugStream.h>
#include <Base/MT/TaskQueue.h>
#include <Base/Util/Bits.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

DBG_STREAM( os_plasma, "Plasma" );

//RCP<SoundManager>    _soundManager;

//------------------------------------------------------------------------------
//!
uint  _numThreads = 0;

//------------------------------------------------------------------------------
//!
enum
{
   ATTRIB_DEBUG,
   ATTRIB_DEBUG_GEOM,
   ATTRIB_GEOM_ERRROR,
   ATTRIB_META_SURFACE_DEBUG,
   ATTRIB_NUM_THREADS,
   ATTRIB_PHYSICS_FPS,
   ATTRIB_RENDERING_FPS,
   ATTRIB_WIREFRAME,
};

//------------------------------------------------------------------------------
//!
StringMap _plasma_attr(
   "debug"           , ATTRIB_DEBUG,
   "debugGeometry"   , ATTRIB_DEBUG_GEOM,
   "geometricError"  , ATTRIB_GEOM_ERRROR,
   "metaSurfaceDebug", ATTRIB_META_SURFACE_DEBUG,
   "numThreads"      , ATTRIB_NUM_THREADS,
   "physicsFPS"      , ATTRIB_PHYSICS_FPS,
   "renderingFPS"    , ATTRIB_RENDERING_FPS,
   "wireframe"       , ATTRIB_WIREFRAME,
   ""
);

//------------------------------------------------------------------------------
//!
int plasmaDebugVM( VMState* vm )
{
   StdErr << "***DEBUG***" << nl;

   World* w = (World*)VM::toProxy( vm, 1 );
   RCP<CubemapBaker> b = new CubemapBaker();
   b->add( w, Vec3f( 0.0f, 2.0f, 0.0f), Vec2i(64), String("testcube") );
   Core::addAnimator( b );

   return 0;
}

//------------------------------------------------------------------------------
//!
int plasmaGetVM( VMState* vm )
{
   const char* attr = VM::toCString( vm, -1 );
   switch( _plasma_attr[attr] )
   {
      case ATTRIB_DEBUG:
         VM::push( vm, plasmaDebugVM );
         return 1;
      case ATTRIB_DEBUG_GEOM:
         VM::push( vm, Plasma::debugGeometry() );
         return 1;
      case ATTRIB_GEOM_ERRROR:
         VM::push( vm, Plasma::geometricError() );
         return 1;
      case ATTRIB_META_SURFACE_DEBUG:
         VM::push( vm, MetaSurface::debugLevel() );
         return 1;
      case ATTRIB_NUM_THREADS:
         VM::push( vm, _numThreads );
         return 1;
      case ATTRIB_PHYSICS_FPS:
         VM::push( vm, Plasma::physicsFPS() );
         return 1;
      case ATTRIB_RENDERING_FPS:
         VM::push( vm, Plasma::renderingFPS() );
         return 1;
      case ATTRIB_WIREFRAME:
         VM::push( vm, Plasma::showWireframe() );
         return 1;
      default:;
   }
   return 0;
}

//------------------------------------------------------------------------------
//!
int plasmaSetVM( VMState* vm )
{
   const char* attr = VM::toCString( vm, -2 );
   switch( _plasma_attr[attr] )
   {
      case ATTRIB_DEBUG:
         // Read-only.
         return 0;
      case ATTRIB_DEBUG_GEOM:
         Plasma::debugGeometry( VM::toBoolean( vm, -1 ) );
         return 0;
      case ATTRIB_GEOM_ERRROR:
         Plasma::geometricError( VM::toFloat( vm, -1 ) );
         return 0;
      case ATTRIB_META_SURFACE_DEBUG:
         MetaSurface::debugLevel( VM::toUInt( vm, -1 ) );
         return 0;
      case ATTRIB_NUM_THREADS:
         _numThreads = VM::toUInt( vm, -1 );
         return 0;
      case ATTRIB_PHYSICS_FPS:
         Plasma::physicsFPS( VM::toFloat( vm, -1 ) );
         return 0;
      case ATTRIB_RENDERING_FPS:
         Plasma::renderingFPS( VM::toFloat( vm, -1 ) );
         return 0;
      case ATTRIB_WIREFRAME:
         Plasma::showWireframe( VM::toBoolean( vm, -1 ) );
         return 0;
      default:;
   }
   return 0;
}

//------------------------------------------------------------------------------
//!
int plasmaConfigVM( VMState* vm )
{
   if( VM::isTable( vm, -1 ) )
   {
      VM::push( vm ); // Start iterating at index 0 (nil).
      while( VM::next( vm, -2 ) )
      {
         plasmaSetVM( vm );
         VM::pop( vm, 1 ); // Pop the value, keep the key.
      }
   }
   return 0;
}

//------------------------------------------------------------------------------
//!
void plasmaInitVM( VMState* vm, uint /*mask*/ )
{
   // Create a table 'PlasmaConfig' and assign a meta-table which send
   // set and get events.
   VM::newTable( vm );
   VM::newMetaTable( vm, "PlasmaConfig" );
   VM::set( vm, -1, "__index"   , plasmaGetVM );
   VM::set( vm, -1, "__newindex", plasmaSetVM );
   VM::setMetaTable( vm, -2 );
   VM::setGlobal( vm, "PlasmaConfig" );
}

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Plasma
==============================================================================*/

//------------------------------------------------------------------------------
//!
TaskQueue* Plasma::_dispatchQueue = NULL;

//------------------------------------------------------------------------------
//!
bool Plasma::_wireframe       = false;
bool Plasma::_dbgGeom         = false;
float Plasma::_geometricError = 0.004f;
float Plasma::_physicsFPS     = 60.0f;
float Plasma::_renderingFPS   = 60.0f;

//------------------------------------------------------------------------------
//!
void
Plasma::init()
{
   DBG_BLOCK( os_plasma, "Plasma::init()" );
   VMRegistry::add( "Plasma", plasmaConfigVM, VM_CAT_CFG );
   VMRegistry::add( plasmaInitVM, VM_CAT_APP );

   //CHECK( _soundManager.isNull() );
   //_soundManager = RCP<SoundManager>( new SoundManager() );

   // Initialize extra ResManager routines.
   ResManager::initializePlasma();
   ResExporter::initialize();
   ResourceVM::initialize();

   // Initialize classes.
   Material::initialize();
   World::initialize();
   Entity::initialize();
   Geometry::initialize();
   Component::initialize();
   ProceduralGeometry::initialize();
   ProceduralMaterial::initialize();
   ProceduralMesh::initialize();
   ProceduralWorld::initialize();
   ProceduralSkeleton::initialize();
   ProceduralAnimation::initialize();
   ProceduralAnimationGraph::initialize();
   ProceduralDataFlow::initialize();
   Manipulator::initialize();
   CameraManipulator::initialize();
   TouchCameraManipulator::initialize();
   FlyByManipulator::initialize();
   Brain::initialize();
   Receptor::initialize();
   EventStimuli::initialize();
   Orders::initialize();
   Action::initialize();
   Controller::initialize();
   MultiCharacterController::initialize();

   DFNodeAttr::initialize();
   initializeAnimNodes();
   initializeBakers();
   initializeGeomNodes();
   initializeGeomGenerator();
   initializeGeomModifier();
   initializeCollisionNodes();
   initializeImageNodes();
   initializeImageGenerators();
   initializePolygonNodes();
   initializeStrokes();
   initializeStrokesNodes();
   initializeTreeNodes();
   initializeWorldNodes();

   DebugGeometry::initialize();

   // UI.
   DFViewer::initialize();
   DFScreen::initialize();
   PlasmaBrowser::initialize();
   PlasmaScreen::initialize();

   Core::callWhenReadyToExec( Core::ReadyToExecDelegate(&Plasma::exec) );
}

//------------------------------------------------------------------------------
//!
void
Plasma::exec()
{
   DBG_BLOCK( os_plasma, "Plasma::exec()" );
   CHECK( _dispatchQueue == NULL ); // Don't support reallocation of threads for now.

   // Allocate VMs for executing.
   _dispatchQueue = new TaskQueue( _numThreads );
   Brain::initVMs();
}

//------------------------------------------------------------------------------
//!
void
Plasma::term()
{
   DBG_BLOCK( os_plasma, "Plasma::term()" );

   terminateWorldNodes();
   terminateTreeNodes();
   terminateStrokesNodes();
   terminateStrokes();
   terminatePolygonNodes();
   terminateImageGenerators();
   terminateImageNodes();
   terminateCollisionNodes();
   terminateGeomModifier();
   terminateGeomGenerator();
   terminateGeomNodes();
   terminateAnimNodes();
   DFNodeSpec::unregisterAll();
   DFNodeAttr::terminate();

   DebugGeometry::terminate();

   Orders::terminate();
   EventStimuli::terminate();
   Action::terminate();
   Brain::termVMs();
   delete _dispatchQueue;
   _dispatchQueue = NULL;

   ProceduralMaterial::terminate();
   Material::terminate();
   ResManager::terminatePlasma();
   //_soundManager = NULL;
}

#if 0
//------------------------------------------------------------------------------
//!
const RCP<SoundManager>&
Plasma::soundManager()
{
   return _soundManager;
}
#endif


/*==============================================================================
  CLASS PlasmaApp
==============================================================================*/

//------------------------------------------------------------------------------
//!
RCP<PlasmaApp>
PlasmaApp::create( int argc, char* argv[] )
{
   return RCP<PlasmaApp>( new PlasmaApp(argc, argv) );
}

//------------------------------------------------------------------------------
//!
PlasmaApp::PlasmaApp( int argc, char* argv[], const String& name ):
   FusionApp( argc, argv, name )
{
   DBG_BLOCK( os_plasma, "PlasmaApp::PlasmaApp()" );
   Plasma::init();
}

//------------------------------------------------------------------------------
//!
PlasmaApp::~PlasmaApp()
{
   DBG_BLOCK( os_plasma, "PlasmaApp::~PlasmaApp()" );
   Plasma::term();
}

//------------------------------------------------------------------------------
//!
void
PlasmaApp::printInfo( TextStream& os ) const
{
   FusionApp::printInfo( os );
   os << nl;
   os << "-------------------------" << nl;
   os << "INFORMATION ABOUT PLASMA:" << nl;
   os << "-------------------------" << nl;
   MotionWorld::printInfo( os );
   //Snd::Manager::printInfo( os );
   os << "-------------------------" << nl;
}

NAMESPACE_END
