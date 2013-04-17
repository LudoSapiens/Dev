/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Resource/ResManager.h>

#include <Plasma/Plasma.h>
#include <Plasma/Procedural/ProceduralAnimation.h>
#include <Plasma/Procedural/ProceduralAnimationGraph.h>
#include <Plasma/Procedural/ProceduralSkeleton.h>
#include <Plasma/Procedural/ProceduralGeometry.h>
#include <Plasma/Procedural/ProceduralMaterial.h>
#include <Plasma/Procedural/ProceduralMesh.h>
#include <Plasma/Procedural/ProceduralWorld.h>
#include <Plasma/DataFlow/ProceduralDataFlow.h>
#include <Plasma/Resource/BinaryResourceTask.h>
#include <Plasma/Resource/ResExporter.h>
#include <Plasma/Resource/Serializer.h>
#include <Plasma/Animation/Puppeteer.h>
#include <Plasma/World/Brain.h>
#include <Plasma/World/Sound.h>
#include <Plasma/World/World.h>

#include <Fusion/Core/Core.h>
#include <Fusion/Resource/ResCache.h>
#include <Fusion/VM/VM.h>
#include <Fusion/VM/VMRegistry.h>

#include <Base/Dbg/DebugStream.h>
#include <Base/IO/FileDevice.h>
#include <Base/IO/TextStream.h>
#include <Base/Msg/Delegate.h>
#include <Base/MT/TaskQueue.h>
#include <Base/Util/SHA.h>

USING_NAMESPACE

using ResManager::ClearCallback;


/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

DBG_STREAM( os_rm, "ResManager" );

//-----------------------------------------------------------------------------
//!
String  getDir( const String& str )
{
   String::SizeType pos = str.rfind( "/" );
   if( pos != String::npos )  return str.sub( 0, str.rfind( "/" ) + 1 );
   else                       return str;
}

//------------------------------------------------------------------------------
//! The various managers (for caching).
ResCache< SkeletalAnimation               >*  _rc_anim      = nullptr;
ResCache< AnimationGraph                  >*  _rc_animGraph = nullptr;
ResCache< BrainProgram                    >*  _rc_brainProg = nullptr;
ResCache< Geometry, ResManager::DigestKey >*  _rc_geom      = nullptr;
ResCache< DFGraph                         >*  _rc_graph     = nullptr;
ResCache< Skeleton                        >*  _rc_skeleton  = nullptr;

//------------------------------------------------------------------------------
//!
int
saveWorldVM( VMState* vm )
{
   World* world = (World*)VM::toProxy( vm, 1 );
   if( world == nullptr )  return 0;

   Path path( VM::toCString( vm, 2 ) );
   VM::push( vm, ResExporter::save( *world, path ) );
   return 1;
}

//------------------------------------------------------------------------------
//!
int
saveGeometryVM( VMState* vm )
{
   Geometry* geom = (Geometry*)VM::toProxy( vm, 1 );
   if( geom == nullptr )  return 0;

   Path path( VM::toCString( vm, 2 ) );
   VM::push( vm, ResExporter::save( *geom, path ) );
   return 1;
}

//------------------------------------------------------------------------------
//!
const VM::Reg res_funcs[] = {
   { "saveWorld" ,    saveWorldVM     },
   { "saveGeometry" , saveGeometryVM  },
   { 0, 0 }
};

//------------------------------------------------------------------------------
//!
void
initVM( VMState* vm, uint /*mask*/ )
{
   VM::registerFunctions( vm, "Plasma", res_funcs );
}

UNNAMESPACE_END


//------------------------------------------------------------------------------
// Private Plasma API
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//!
void
ResManager::initializePlasma()
{
   _rc_anim      = new ResCache< SkeletalAnimation   >( "Animation"      );
   _rc_animGraph = new ResCache< AnimationGraph      >( "AnimationGraph" );
   _rc_brainProg = new ResCache< BrainProgram        >( "BrainProgram"   );
   _rc_geom      = new ResCache< Geometry, DigestKey >( "Geometry"       );
   _rc_graph     = new ResCache< DFGraph             >( "DataFlowGraph"  );
   _rc_skeleton  = new ResCache< Skeleton            >( "Skeleton"       );
   VMRegistry::add( initVM, VM_CAT_APP );
   _register( clearPlasma );
}

//------------------------------------------------------------------------------
//!
void
ResManager::terminatePlasma()
{
   _unregister( clearPlasma );
   delete _rc_anim     ; _rc_anim      = nullptr;
   delete _rc_animGraph; _rc_animGraph = nullptr;
   delete _rc_brainProg; _rc_brainProg = nullptr;
   delete _rc_geom     ; _rc_geom      = nullptr;
   delete _rc_graph    ; _rc_graph     = nullptr;
   delete _rc_skeleton ; _rc_skeleton  = nullptr;
}

//------------------------------------------------------------------------------
//!
void
ResManager::clearPlasma()
{
   _rc_anim->clear();
   _rc_animGraph->clear();
   _rc_brainProg->clear();
   _rc_geom->clear();
   _rc_graph->clear();
   _rc_skeleton->clear();
}

//------------------------------------------------------------------------------
// Public Plasma API
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//!
RCP< Resource<DFGraph> >
ResManager::getGraph(
   const String& id,
   Task*         caller
)
{
   RCP< Resource<DFGraph> > res( _rc_graph->get( id ) );

   if( res.isNull() )
   {
      String path = getGraphPath( id );
      if( !path.empty() )
      {
         res = _rc_graph->add( id );
         ProceduralDataFlow* task = new ProceduralDataFlow( res.ptr(), path );
         if( caller )
            caller->spawn( task );
         else
            dispatchQueue()->post( task );
      }
   }

   if( res.isNull() )
   {
      StdErr << "Error with data flow graph: " << id << ".\n";
      StdErr << " --> doesn't know anything about it.\n";
   }
   return res;
}

//------------------------------------------------------------------------------
//!
String ResManager::getGraphName( const Resource<DFGraph>* res )
{
   return _rc_graph->get( res );
}

//------------------------------------------------------------------------------
//!
String ResManager::getGraphName( const DFGraph* graph )
{
   return _rc_graph->get( graph );
}

//------------------------------------------------------------------------------
//!
String ResManager::getGraphPath( const String& id )
{
   const char* ext[] = { ".df", 0 };
   return idToPath( id, ext );
}

//------------------------------------------------------------------------------
//!
RCP< Resource<BrainProgram> >
ResManager::getBrainProgram( const String& id, Task* caller )
{
   DBG_BLOCK( os_rm, "ResManager::getBrainProgram(" << id << ")" );

   RCP< Resource<BrainProgram> > res( _rc_brainProg->get( id ) );

   if( res.isNull() )
   {
      const char* ext[] = { ".brain", 0 };
      String path       = idToPath( id, ext );

      if( !path.empty() )
      {
         res = _rc_brainProg->add( id );
         Task* task = new BrainProgramLoadTask( res.ptr(), path );
         if( caller )
            caller->spawn( task );
         else
            dispatchQueue()->post( task );
      }
      else
      {
         StdErr << "Error with brain '" << id << "'.\n";
         StdErr << " --> doesn't know anything about it.\n";
      }
   }

   return res;
}

//------------------------------------------------------------------------------
//!
void
ResManager::registerBrainProgram( const String& id, BrainProgram* prog )
{
   RCP< Resource<BrainProgram> > res = _rc_brainProg->add( id );
   res->data( prog );
}

//------------------------------------------------------------------------------
//!
String
ResManager::getBrainProgramName( const Resource<BrainProgram>* res )
{
   return _rc_brainProg->get( res );
}

//------------------------------------------------------------------------------
//!
String
ResManager::getBrainProgramName( const BrainProgram* prog )
{
   return _rc_brainProg->get( prog );
}

//------------------------------------------------------------------------------
//!
RCP< Resource<Geometry> >
ResManager::getGeometry( const String& id, Task* caller, bool compiled )
{
   return getGeometry( id, Table::null(), caller, compiled );
}

//------------------------------------------------------------------------------
//!
RCP< Resource<Geometry> >
ResManager::getGeometry( const String& id, const Table& params, Task* caller, bool compiled )
{
   DBG_BLOCK( os_rm, "ResManager::getGeometry(" << id << ")" );
   DigestKey key( id, params );

   // Check if exist.
   RCP< Resource<Geometry> > res( _rc_geom->get( key ) );

   // Create geometry if not.
   // Try Binary MeshGeometry.
   if( res.isNull() )
   {
      const char* ext[] = { ".mesh.bin.gz", ".mesh.bin", 0 };
      String path       = idToPath( id, ext );

      if( !path.empty() )
      {
         res = _rc_geom->add( key );
         BinaryGeometryTask* task = new BinaryGeometryTask(
            res.ptr(),
            &Serializer::loadMeshGeometry,
            path,
            params
         );
         if( caller )
            caller->spawn( task );
         else
            dispatchQueue()->post( task );
      }
   }
   // Try MeshGeometry.
   if( res.isNull() )
   {
      const char* ext[] = { ".mesh", 0 };
      String path       = idToPath( id, ext );

      if( !path.empty() )
      {
         res = _rc_geom->add( key );
         ProceduralMesh* task = new ProceduralMesh( res.ptr(), path, params );
         if( caller )
            caller->spawn( task );
         else
            dispatchQueue()->post( task );
      }
   }
   // Try MetaGeometry.
   if( res.isNull() )
   {
      const char* ext[] = { ".geom", 0 };
      String path       = idToPath( id, ext );

      if( !path.empty() )
      {
         res = _rc_geom->add( key );
         ProceduralGeometry* task = new ProceduralGeometry( res.ptr(), path, params, compiled );
         if( caller )
            caller->spawn( task );
         else
            dispatchQueue()->post( task );
      }
   }

   if( res.isNull() )
   {
      StdErr << "Error with geometry " << id << ".\n";
      StdErr << " --> doesn't know anything about it.\n";
   }

   return res;
}

//------------------------------------------------------------------------------
//!
String
ResManager::getGeometryName( const Resource<Geometry>* res )
{
   return _rc_geom->get( res ).name();
}

//------------------------------------------------------------------------------
//!
String
ResManager::getGeometryName( const Geometry* geom )
{
   return _rc_geom->get( geom ).name();
}

//------------------------------------------------------------------------------
//!
void
ResManager::registerGeometry( const String& id, Geometry* geom )
{
   RCP< Resource<Geometry> > res = _rc_geom->add( id );
   res->data( geom );
}

//------------------------------------------------------------------------------
//!
RCP< Resource<Geometry> >
ResManager::getMeshGeometry( const String& id, const Table& params, Task* caller )
{
   DigestKey key( id, params );

   // Check if it exists.
   RCP< Resource<Geometry> > res = _rc_geom->get( key );

   // Create geometry if not.
   if( res.isNull() )
   {
      const char* ext[] = { ".mesh", 0 };
      String path       = idToPath( id, ext );

      if( !path.empty() )
      {
         res = _rc_geom->add( key );
         ProceduralMesh* task = new ProceduralMesh( res.ptr(), path, params );
         if( caller )
            caller->spawn( task );
         else
            dispatchQueue()->post( task );
      }
      else
      {
         StdErr << "Error with mesh " << id << ".\n";
         StdErr << " --> doesn't know anything about it.\n";
      }
   }

   return res;
}

//------------------------------------------------------------------------------
//!
String
ResManager::getMeshGeometryName( const Resource<Geometry>* res )
{
   return _rc_geom->get( res ).name();
}

//------------------------------------------------------------------------------
//!
String
ResManager::getMeshGeometryName( const Geometry* geom )
{
   return _rc_geom->get( geom ).name();
}

//------------------------------------------------------------------------------
//!
RCP< Resource<MaterialSet> >
ResManager::newMaterialSet( const String& id, Task* caller )
{
   return newMaterialSet( id, NULL, NULL, caller );
}

//------------------------------------------------------------------------------
//!
RCP< Resource<MaterialSet> >
ResManager::newMaterialSet(
   const String& id,
   const Table&  params,
   Task*         caller
)
{
   return newMaterialSet( id, &params, NULL, caller );
}

//------------------------------------------------------------------------------
//!
RCP< Resource<MaterialSet> >
ResManager::newMaterialSet(
   const String&      id,
   const Table*       params,
   const MaterialMap* map,
   Task*              caller
)
{
   DBG_BLOCK( os_rm, "ResManager::newMaterialSet(" << id << (void*)params << (void*)map << (void*)caller << ")" );

   RCP< Resource<MaterialSet> > res;

   const char* ext[] = { ".mat", 0 };
   String path       = idToPath( id, ext );

   if( !path.empty() )
   {
      res = new Resource<MaterialSet>();
      res->state( Resource<MaterialSet>::LOADING );
      ProceduralMaterial* task = new ProceduralMaterial( res.ptr(), id, path, params, map );
      if( caller )
         caller->spawn( task );
      else
         dispatchQueue()->post( task );
   }
   else
   {
      StdErr << "Error with material " << id << ".\n";
      StdErr << " --> doesn't know anything about it.\n";
   }

   return res;
}

//------------------------------------------------------------------------------
//!
RCP< Resource<Skeleton> >
ResManager::getSkeleton( const String& id, Task* caller )
{
   RCP< Resource<Skeleton> > res( _rc_skeleton->get( id ) );

   if( res.isNull() )
   {
      const char* ext[] = { ".skel", 0 };
      String path       = idToPath( id, ext );

      if( !path.empty() )
      {
         res = _rc_skeleton->add( id );
         ProceduralSkeleton* task = new ProceduralSkeleton( res.ptr(), path );
         if( caller )
            caller->spawn( task );
         else
            dispatchQueue()->post( task );
      }
   }

   if( res.isNull() )
   {
      StdErr << "Error with skeleton " << id << ".\n";
      StdErr << " --> doesn't know anything about it.\n";
   }

   return res;
}

//------------------------------------------------------------------------------
//!
String ResManager::getSkeletonName( const Resource<Skeleton>* res )
{
   return _rc_skeleton->get( res );
}

//------------------------------------------------------------------------------
//!
String ResManager::getSkeletonName( const Skeleton* skel )
{
   return _rc_skeleton->get( skel );
}

//------------------------------------------------------------------------------
//!
String ResManager::getSkeletonName( const AnimationGraph* graph )
{
   String str = getAnimationGraphName( graph );
   return getDir( str ) + "body";
}

//------------------------------------------------------------------------------
//!
RCP< Resource<SkeletalAnimation> >
ResManager::getAnimation( const String& id, Task* caller )
{
   RCP< Resource<SkeletalAnimation> > res( _rc_anim->get( id ) );

   if( res.isNull() )
   {
      const char* ext[] = { ".anim", 0 };
      String path       = idToPath( id, ext );

      if( !path.empty() )
      {
         // Compute skeleton path.
         String skelId = getDir( id ) + "body";

         res = _rc_anim->add( id );
         ProceduralAnimation* task = new ProceduralAnimation( res.ptr(), path, skelId );
         if( caller )
            caller->spawn( task );
         else
            dispatchQueue()->post( task );
      }
   }

   if( res.isNull() )
   {
      StdErr << "Error with animation " << id << ".\n";
      StdErr << " --> doesn't know anything about it.\n";
   }

   return res;
}

//------------------------------------------------------------------------------
//!
String ResManager::getAnimationName( const Resource<SkeletalAnimation>* res )
{
   return _rc_anim->get( res );
}

//------------------------------------------------------------------------------
//!
String ResManager::getAnimationName( const SkeletalAnimation* anim )
{
   return _rc_anim->get( anim );
}

//------------------------------------------------------------------------------
//!
RCP< Resource<SkeletalAnimation> >
ResManager::retarget(
   SkeletalAnimation* srcAnim,
   Skeleton*          dstSkel,
   Task*              caller
)
{
   // Create key.
   String id  = getAnimationName( srcAnim );
   String key = id+String(dstSkel);

   RCP< Resource<SkeletalAnimation> > res( _rc_anim->get( key ) );
   if( res.isNull() )
   {
      res = _rc_anim->add( key );
      AnimationRetargetingTask* task = new AnimationRetargetingTask( res.ptr(), srcAnim, dstSkel );
      if( caller )
         caller->spawn( task );
      else
         dispatchQueue()->post( task );
   }
   return res;
}

//------------------------------------------------------------------------------
//!
RCP< Resource<AnimationGraph> >
ResManager::getAnimationGraph( const String& id, Task* caller )
{
   RCP< Resource<AnimationGraph> > res( _rc_animGraph->get( id ) );

   if( res.isNull() )
   {
      const char* ext[] = { ".agraph", 0 };
      String path       = idToPath( id, ext );

      if( !path.empty() )
      {
         res = _rc_animGraph->add( id );
         ProceduralAnimationGraph* task = new ProceduralAnimationGraph( res.ptr(), path );
         if( caller )
            caller->spawn( task );
         else
            dispatchQueue()->post( task );
      }
   }

   if( res.isNull() )
   {
      StdErr << "Error with animation graph: " << id << ".\n";
      StdErr << " --> doesn't know anything about it.\n";
   }
   return res;
}

//------------------------------------------------------------------------------
//!
String ResManager::getAnimationGraphName( const Resource<AnimationGraph>* res )
{
   return _rc_animGraph->get( res );
}

//------------------------------------------------------------------------------
//!
String ResManager::getAnimationGraphName( const AnimationGraph* graph )
{
   return _rc_animGraph->get( graph );
}

//------------------------------------------------------------------------------
//!
RCP< Resource<AnimationGraph> >
ResManager::retarget(
   AnimationGraph* srcGraph,
   Skeleton*       dstSkel,
   Task*           caller
)
{
   // Create key.
   String id  = getAnimationGraphName( srcGraph );
   String key = id+String(dstSkel);

   RCP< Resource<AnimationGraph> > res( _rc_animGraph->get( key ) );

   if( res.isNull() )
   {
      res = _rc_animGraph->add( key );
      GraphRetargetingTask* task = new GraphRetargetingTask( res.ptr(), srcGraph, dstSkel );
      if( caller )
         caller->spawn( task );
      else
         dispatchQueue()->post( task );
   }

   return res;
}

//------------------------------------------------------------------------------
//!
RCP< Resource<World> >
ResManager::newWorld( const String& id, Task* caller )
{
   return newWorld( id, Table::null(), caller );
}

//------------------------------------------------------------------------------
//!
RCP< Resource<World> >
ResManager::newWorld( const String& id, const Table& params, Task* caller )
{
   DBG_BLOCK( os_rm, "ResManager::newWorld(" << id << ")" );

   const char* ext[] = { ".world", 0 };
   String path       = idToPath( id, ext );

   RCP< Resource<World> > res;

   if( !path.empty() )
   {
      res = new Resource<World>();
      Task* task = params.empty() ?
         new ProceduralWorld( res.ptr(), id, path ):
         new ProceduralWorld( res.ptr(), id, path, params );
      if( caller )
         caller->spawn( task );
      else
         dispatchQueue()->post( task );
   }
   else
   {
      StdErr << "Error with world " << id << ".\n";
      StdErr << " --> doesn't know anything about it.\n";
   }

   return res;
}

