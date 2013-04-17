/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Resource/ResourceVM.h>
#include <Plasma/Resource/ResExporter.h>
#include <Plasma/Resource/ResManager.h>
#include <Plasma/Animation/SkeletalAnimation.h>
#include <Plasma/Animation/Puppeteer.h>
#include <Plasma/DataFlow/DFGraph.h>
#include <Plasma/DataFlow/DFNodeAttr.h>
#include <Plasma/World/World.h>
#include <Plasma/World/WorldVM.h>

#include <Fusion/VM/VMObjectPool.h>

#include <Base/ADT/StringMap.h>
#include <Base/IO/FileDevice.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

struct VMCb
{
   template< typename T > void executeCb( Resource<T>* res )
   {
      T* t = data( res );
      _pool->add( t, t );
      VMState* vm = _ref.vm();
      VM::push( vm, _ref );
      VM::pushProxy( vm, t );
      VM::ecall( vm, 1, 0 );
      delete this;
   }

   RCP<VMObjectPool> _pool;
   VMRef             _ref;
};

/*==============================================================================
   World
==============================================================================*/

//------------------------------------------------------------------------------
//!
int newWorldVM( VMState* vm )
{
   VMObjectPool* pool = (VMObjectPool*)VM::thisPtr( vm );
   int numArgs        = VM::getTop(vm);
   String name        = VM::toString( vm, 1 );

   RCP< Resource<World> > res;

   // No parameters?
   if( numArgs == 1 || !VM::isTable( vm, 2 ) )
   {
      res = ResManager::newWorld( name, 0 );
   }
   else
   {
      RCP<Table> params( new Table );
      VM::toTable( vm, 2, *params );
      res = ResManager::newWorld( name, *params, 0 );
   }

   // Do we have a callback?
   if( VM::isFunction( vm, numArgs ) )
   {
      if( res.isValid() )
      {
         VMCb* cb  = new VMCb;
         cb->_pool = pool;
         VM::toRef( vm, numArgs, cb->_ref );
         res->callOnLoad( makeDelegate( cb, &VMCb::executeCb<World> ) );
      }
      else
      {
         VM::pushProxy( vm, 0 );
         VM::ecall( vm, 1, 0 );
      }
   }

   return 0;
}

/*==============================================================================
   DFGraph
==============================================================================*/

//------------------------------------------------------------------------------
//!
enum
{
   DFGRAPH_ADD_ON_MODIFY,
   DFGRAPH_ADD_ON_REMOVE,
   DFGRAPH_ADD_ON_UPDATE,
   DFGRAPH_CREATE_NODE,
   DFGRAPH_EXPORT,
   DFGRAPH_FIND_SPOT,
   DFGRAPH_GET_BOUNDING_BOX,
   DFGRAPH_NAME,
   DFGRAPH_OUTPUT,
   DFGRAPH_OUTPUT_TYPE,
   DFGRAPH_PATH,
   DFGRAPH_REMOVE_ALL,
   DFGRAPH_REMOVE_ON_MODIFY,
   DFGRAPH_REMOVE_ON_REMOVE,
   DFGRAPH_REMOVE_ON_UPDATE,
   DFGRAPH_SAVE,
};

StringMap _attr_dfgraph(
   "addOnModify"   ,  DFGRAPH_ADD_ON_MODIFY,
   "addOnRemove"   ,  DFGRAPH_ADD_ON_REMOVE,
   "addOnUpdate"   ,  DFGRAPH_ADD_ON_UPDATE,
   "createNode"    ,  DFGRAPH_CREATE_NODE,
   "export"        ,  DFGRAPH_EXPORT,
   "findSpot"      ,  DFGRAPH_FIND_SPOT,
   "getBoundingBox",  DFGRAPH_GET_BOUNDING_BOX,
   "name"          ,  DFGRAPH_NAME,
   "output"        ,  DFGRAPH_OUTPUT,
   "outputType"    ,  DFGRAPH_OUTPUT_TYPE,
   "path"          ,  DFGRAPH_PATH,
   "removeAll"     ,  DFGRAPH_REMOVE_ALL,
   "removeOnModify",  DFGRAPH_REMOVE_ON_MODIFY,
   "removeOnRemove",  DFGRAPH_REMOVE_ON_REMOVE,
   "removeOnUpdate",  DFGRAPH_REMOVE_ON_UPDATE,
   "save"          ,  DFGRAPH_SAVE,
   ""
);

//-----------------------------------------------------------------------------
//!
int df_getNodeSpecs( VMState* vm )
{
   Vector<const DFNodeSpec*>  specs;
   DFNodeSpec::getAll( specs );
   VM::newTable( vm );                       // [..., specsTable]
   for( uint i = 0; i < specs.size(); ++i )
   {
      const DFNodeSpec* spec = specs[i];
      VM::newTable( vm );                    // [..., specsTable, specTable]

      VM::push( vm, "type" );                // [..., specsTable, specTable, "type"]
      VM::push( vm, spec->typeName() );      // [..., specsTable, specTable, "type", type]
      VM::set( vm, -3 );                     // [..., specsTable, specTable]

      VM::push( vm, "name" );                // [..., specsTable, specTable, "name"]
      VM::push( vm, spec->name() );          // [..., specsTable, specTable, "name", name]
      VM::set( vm, -3 );                     // [..., specsTable, specTable]

      VM::push( vm, "label" );               // [..., specsTable, specTable, "label"]
      VM::push( vm, spec->label() );         // [..., specsTable, specTable, "label", label]
      VM::set( vm, -3 );                     // [..., specsTable, specTable]

      VM::push( vm, "info" );                // [..., specsTable, specTable, "info"]
      VM::push( vm, spec->info() );          // [..., specsTable, specTable, "info", info]
      VM::set( vm, -3 );                     // [..., specsTable, specTable]

      VM::push( vm, "icon" );                // [..., specsTable, specTable, "icon"]
      if( !spec->icon().empty() )
         VM::push( vm, spec->icon() );       // [..., specsTable, specTable, "icon", icon]
      else
         VM::push( vm );                     // [..., specsTable, specTable, "icon", nil]
      VM::set( vm, -3 );                     // [..., specsTable, specTable]

      VM::seti( vm, -2, i+1 );               // [..., specsTable]
   }
   return 1;
}

//------------------------------------------------------------------------------
//!
int df_setAttributes( VMState* vm )
{
   // Retrieve node.
   DFNode* node = (DFNode*)VM::toPtr( vm, 1 );
   if( node == nullptr )  return 0;

   // Retrieve editor.
   RCP<DFNodeEditor> editor = node->edit();
   if( editor.isNull() ) return 0;

   // Retrieve attributes values in table format.
   RCP<DFNodeAttrStates> states = new DFNodeAttrStates();

   VM::push( vm );
   while( VM::next( vm, -2 ) )
   {
      states->set( VM::toUInt( vm, -2 ), VM::toVariant( vm, -1 ) );
      VM::pop( vm );
   }

   // Update attributes.
   editor->updateAttributes( *states );

   return 0;
}

//------------------------------------------------------------------------------
//!
int df_getAttributes( VMState* vm )
{
   DFNode* node = (DFNode*)VM::toPtr( vm, 1 );
   if( node == nullptr )  return 0;

   // Retrieve attributes from the requested node.
   RCP<DFNodeEditor> editor    = node->edit();
   RCP<DFNodeAttrList> attribs = editor.isValid() ? editor->attributes() : nullptr;

   // No attributes?
   if( attribs.isNull() ) return 0;

   ResourceVM::push( vm, *attribs );
   return 1;
}

//------------------------------------------------------------------------------
//!
int df_getAttributesStates( VMState* vm )
{
   DFNode* node = (DFNode*)VM::toPtr( vm, 1 );
   if( node == nullptr )  return 0;

   // Retrieve attributes from the requested node.
   RCP<DFNodeEditor> editor     = node->edit();
   RCP<DFNodeAttrStates> states = editor.isValid() ? editor->attributesStates() : nullptr;

   if( states.isNull() ) return 0;

   VM::newTable( vm );
   for( auto cur = states->begin(); cur != states->end(); ++cur )
   {
      VM::push( vm, cur->_id );
      VM::push( vm, cur->_value );
      VM::set( vm, -3 );
   }

   return 1;
}

//------------------------------------------------------------------------------
//!
int df_getGraph( VMState* vm )
{
   DFNode* node = (DFNode*)VM::toPtr( vm, 1 );
   if( node )
   {
      VM::pushProxy( vm, node->graph() );
      return 1;
   }
   return 0;
}

//------------------------------------------------------------------------------
//!
int df_getGraphPath( VMState* vm )
{
   switch( VM::type(vm, 1) )
   {
      case VM::STRING:
      {
         String id = VM::toString( vm, 1 );
         VM::push( vm, ResManager::getGraphPath( id ) );
         return 1;
      }  break;
      case VM::OBJECT:
      {
         DFGraph* graph = (DFGraph*)VM::toProxy( vm, 1 );
         VM::push( vm, ResManager::getGraphPath( graph ) );
         return 1;
      }  break;
      default:
      {
         StdErr << "ERROR - getGraphPath() argument must be a graph or an id." << nl;
         return 0;
      }
   }
}

//-----------------------------------------------------------------------------
//!
void df_init( VMState* vm, uint /*mask*/ )
{
   VM::registerFunction( vm, "DataFlow", "setAttributes",       df_setAttributes       );
   VM::registerFunction( vm, "DataFlow", "getAttributes",       df_getAttributes       );
   VM::registerFunction( vm, "DataFlow", "getAttributesStates", df_getAttributesStates );
   VM::registerFunction( vm, "DataFlow", "getGraph",            df_getGraph            );
   VM::registerFunction( vm, "DataFlow", "getGraphPath",        df_getGraphPath        );
   VM::registerFunction( vm, "DataFlow", "getNodeSpecs",        df_getNodeSpecs        );
}

//------------------------------------------------------------------------------
//!
int dfgraph_create( VMState* vm )
{
   VMObjectPool* pool = (VMObjectPool*)VM::thisPtr( vm );
   DFGraph* graph     = new DFGraph();
   pool->add( graph, graph );
   VM::pushProxy( vm, graph );
   return 1;
}

//------------------------------------------------------------------------------
//!
int dfgraph_addOnModifyVM( VMState* vm )
{
   DFGraph* graph = (DFGraph*)VM::thisPtr( vm );
   VMRef    ref;
   DFNode*  node = (DFNode*)VM::toPtr( vm, 1 );
   CHECK( node );
   CHECK( node->graph() == graph );
   VM::toRef( vm, 2, ref );
   graph->msg().addOnModify( node, ref );
   return 0;
}

//------------------------------------------------------------------------------
//!
int dfgraph_removeOnModifyVM( VMState* vm )
{
   DFGraph* graph = (DFGraph*)VM::thisPtr( vm );
   VMRef    ref;
   DFNode*  node = (DFNode*)VM::toPtr( vm, 1 );
   CHECK( node );
   CHECK( node->graph() == graph );
   VM::toRef( vm, 2, ref );
   graph->msg().removeOnModify( node, ref );
   return 0;
}

//------------------------------------------------------------------------------
//!
int dfgraph_addOnRemoveVM( VMState* vm )
{
   DFGraph* graph = (DFGraph*)VM::thisPtr( vm );
   VMRef    ref;
   DFNode*  node = (DFNode*)VM::toPtr( vm, 1 );
   CHECK( node );
   CHECK( node->graph() == graph );
   VM::toRef( vm, 2, ref );
   graph->msg().addOnRemove( node, ref );
   return 0;
}

//------------------------------------------------------------------------------
//!
int dfgraph_removeOnRemoveVM( VMState* vm )
{
   DFGraph* graph = (DFGraph*)VM::thisPtr( vm );
   VMRef    ref;
   DFNode*  node = (DFNode*)VM::toPtr( vm, 1 );
   CHECK( node );
   CHECK( node->graph() == graph );
   VM::toRef( vm, 2, ref );
   graph->msg().removeOnRemove( node, ref );
   return 0;
}

//------------------------------------------------------------------------------
//!
int dfgraph_addOnUpdateVM( VMState* vm )
{
   DFGraph* graph = (DFGraph*)VM::thisPtr( vm );
   VMRef    ref;
   DFNode*  node;
   switch( VM::getTop(vm) )
   {
      case 1:
         // Graph update.
         VM::toRef( vm, 1, ref );
         graph->msg().addOnUpdate( ref );
         break;
      case 2:
         // Node update.
         node = (DFNode*)VM::toPtr( vm, 1 );
         CHECK( node );
         CHECK( node->graph() == graph );
         VM::toRef( vm, 2, ref );
         graph->msg().addOnUpdate( node, ref );
         break;
      default:
         StdErr << "dfgraph_addOnUpdateVM() - Invalid number of arguments." << nl;
         break;
   }
   return 0;
}

//------------------------------------------------------------------------------
//!
int dfgraph_removeOnUpdateVM( VMState* vm )
{
   DFGraph* graph = (DFGraph*)VM::thisPtr( vm );
   VMRef    ref;
   DFNode*  node;
   switch( VM::getTop(vm) )
   {
      case 1:
         // Graph update.
         VM::toRef( vm, 1, ref );
         graph->msg().removeOnUpdate( ref );
         break;
      case 2:
         // Node update.
         node = (DFNode*)VM::toPtr( vm, 1 );
         CHECK( node );
         CHECK( node->graph() == graph );
         VM::toRef( vm, 2, ref );
         graph->msg().removeOnUpdate( node, ref );
         break;
      default:
         StdErr << "dfgraph_removeOnUpdateVM() - Invalid number of arguments." << nl;
         break;
   }
   return 0;
}

//------------------------------------------------------------------------------
//!
int dfgraph_removeAllVM( VMState* vm )
{
   DFGraph* graph = (DFGraph*)VM::thisPtr( vm );
   VMRef    ref;
   DFNode*  node = (DFNode*)VM::toPtr( vm, 1 );
   CHECK( node );
   CHECK( node->graph() == graph );
   graph->msg().removeAll( node );
   return 0;
}

//-----------------------------------------------------------------------------
//!
int dfgraph_createNodeVM( VMState* vm )
{
   DFGraph*         graph = (DFGraph*)VM::thisPtr( vm );
   ConstString       type = VM::toConstString( vm, 1 );
   const DFNodeSpec* spec = DFNodeSpec::get( type );
   if( spec == nullptr )
   {
      StdErr << "ERROR: Attempted to create a DFNode of unknown type '" << type << "'." << nl;
      return 0;
   }

   RCP<DFNode> node = spec->create( vm, 2 );
   if( node.isNull() )
   {
      StdErr << "ERROR: Could not create node of type '" << type << "'." << nl;
      return 0;
   }

   Vec2i p(0);
   int   w = node->width();
   VM::get( vm, 2, "position", p );
   VM::get( vm, 2, "width", w );
   DFGraph::UpdateLock lock( graph );
   if( !graph->addNode( node.ptr(), p, w ) )
   {
      StdErr << "ERROR: Could not add '" << type << "' node into the graph (" << p << ", " << w << ")." << nl;
      return 0;
   }

   VM::push( vm, true );
   return 1;
}

//-----------------------------------------------------------------------------
//!
int dfgraph_findSpotVM( VMState* vm )
{
   DFGraph* graph = (DFGraph*)VM::thisPtr( vm );
   int      width = VM::toInt( vm, 1 );
   Vec2i   corner = VM::toVec2i( vm, 2 );
   Vec2i    range = VM::toVec2i( vm, 3 );
   if( VM::getTop(vm) <= 3 )
   {
      VM::push( vm, graph->findSpot(width, corner, range) );
   }
   else
   {
      bool vFirst = VM::toBoolean( vm, 4 );
      VM::push( vm, graph->findSpot(width, corner, range, vFirst) );
   }
   return 1;
}

//-----------------------------------------------------------------------------
//!
int dfgraph_getBoundingBoxVM( VMState* vm )
{
   DFGraph* graph = (DFGraph*)VM::thisPtr( vm );
   AARecti bbox = graph->getBoundingBox();
   VM::newTable( vm );
   if( bbox.isEmpty() )
   {
      VM::push( vm, "position" );
      VM::push( vm, Vec2i(0) );
      VM::set( vm, -3 );
      VM::push( vm, "size" );
      VM::push( vm, Vec2i(0) );
      VM::set( vm, -3 );
   }
   else
   {
      VM::push( vm, "position" );
      VM::push( vm, bbox.position() );
      VM::set( vm, -3 );
      VM::push( vm, "size" );
      VM::push( vm, bbox.size() );
      VM::set( vm, -3 );
   }
   return 1;
}

//-----------------------------------------------------------------------------
//!
int dfgraph_saveVM( VMState* vm )
{
   DFGraph* graph  = (DFGraph*)VM::thisPtr( vm );
   String filename = VM::toString( vm, 1 );
   TextStream os( new FileDevice(filename, IODevice::MODE_WRITE|IODevice::MODE_NEWFILE) );
   StreamIndent indent;
   VM::push( vm, graph->dump(os, indent) );
   return 1;
}

//------------------------------------------------------------------------------
//!
int dfgraph_exportVM( VMState* vm )
{
   DFGraph* graph  = (DFGraph*)VM::thisPtr( vm );
   String filename = VM::toString( vm, 1 );
   if( !graph ) return 0;

   // Read parameters into a table.
   RCP<Table> params;
   if( VM::getTop(vm) > 1 )
   {
      params = new Table();
      VM::toTable( vm, 2, *params );
   }

   VM::push( vm, ResExporter::save( *graph, filename, params.ptr() ) );
   return 0;
}

//------------------------------------------------------------------------------
//!
int dfgraph_get( VMState* vm )
{
   DFGraph*    obj = (DFGraph*)VM::toProxy( vm, 1 );
   const char* key = VM::toCString( vm, 2 );
   switch( _attr_dfgraph[key] )
   {
      case DFGRAPH_ADD_ON_MODIFY:
         VM::push( vm, obj, dfgraph_addOnModifyVM );
         return 1;
      case DFGRAPH_ADD_ON_REMOVE:
         VM::push( vm, obj, dfgraph_addOnRemoveVM );
         return 1;
      case DFGRAPH_ADD_ON_UPDATE:
         VM::push( vm, obj, dfgraph_addOnUpdateVM );
         return 1;
      case DFGRAPH_CREATE_NODE:
         VM::push( vm, obj, dfgraph_createNodeVM );
         return 1;
      case DFGRAPH_EXPORT:
         VM::push( vm, obj, dfgraph_exportVM );
         return 1;
      case DFGRAPH_FIND_SPOT:
         VM::push( vm, obj, dfgraph_findSpotVM );
         return 1;
      case DFGRAPH_GET_BOUNDING_BOX:
         VM::push( vm, obj, dfgraph_getBoundingBoxVM );
         return 1;
      case DFGRAPH_NAME:
         VM::push( vm, ResManager::getGraphName(obj) );
         return 1;
      case DFGRAPH_OUTPUT:
         VM::push( vm, obj->output() );
         return 1;
      case DFGRAPH_OUTPUT_TYPE:
      {
         const char* type = nullptr;
         DFNode* outNode  = obj->output();
         if( outNode ) type = outNode->output()->typeAsStr();
         VM::push( vm, type );
         return 1;
      }
      case DFGRAPH_PATH:
         VM::push( vm, ResManager::getGraphPath(obj) );
         return 1;
      case DFGRAPH_REMOVE_ALL:
         VM::push( vm, obj, dfgraph_removeAllVM );
         return 1;
      case DFGRAPH_REMOVE_ON_MODIFY:
         VM::push( vm, obj, dfgraph_removeOnModifyVM );
         return 1;
      case DFGRAPH_REMOVE_ON_REMOVE:
         VM::push( vm, obj, dfgraph_removeOnRemoveVM );
         return 1;
      case DFGRAPH_REMOVE_ON_UPDATE:
         VM::push( vm, obj, dfgraph_removeOnUpdateVM );
         return 1;
      case DFGRAPH_SAVE:
         VM::push( vm, obj, dfgraph_saveVM );
         return 1;
      default:
         break;
   }
   return 0;
}

//------------------------------------------------------------------------------
//!
int dfgraph_set( VMState* vm )
{
   DFGraph*    obj = (DFGraph*)VM::toProxy( vm, 1 );
   const char* key = VM::toCString( vm, 2 );
   // Third argument is the value being assigned.
   switch( _attr_dfgraph[key] )
   {
      case DFGRAPH_ADD_ON_MODIFY:
      case DFGRAPH_ADD_ON_UPDATE:
      case DFGRAPH_CREATE_NODE:
      case DFGRAPH_EXPORT:
      case DFGRAPH_FIND_SPOT:
      case DFGRAPH_GET_BOUNDING_BOX:
      case DFGRAPH_NAME:
         // Read-only.
         return 0;
      case DFGRAPH_OUTPUT:
         obj->output( (DFNode*)VM::toPtr(vm, 3) );
         return 0;
      case DFGRAPH_OUTPUT_TYPE:
      case DFGRAPH_REMOVE_ON_MODIFY:
      case DFGRAPH_REMOVE_ON_UPDATE:
      case DFGRAPH_SAVE:
         // Read-only.
         return 0;
      default:
         break;
   }
   return 0;
}

//------------------------------------------------------------------------------
//!
int loadDFGraphVM( VMState* vm )
{
   VMObjectPool* pool = (VMObjectPool*)VM::thisPtr( vm );
   int numArgs        = VM::getTop(vm);
   String name        = VM::toString( vm, 1 );

   RCP< Resource<DFGraph> > res = ResManager::getGraph( name, 0 );

   // Do we have a callback?
   if( VM::isFunction( vm, numArgs ) )
   {
      if( res.isValid() )
      {
         VMCb* cb  = new VMCb;
         cb->_pool = pool;
         VM::toRef( vm, numArgs, cb->_ref );
         res->callOnLoad( makeDelegate( cb, &VMCb::executeCb<DFGraph> ) );
      }
      else
      {
         VM::pushProxy( vm, 0 );
         VM::ecall( vm, 1, 0 );
      }
   }

   return 0;
}


/*==============================================================================
   SkeletalAnimation
==============================================================================*/

// Animation attributes.
enum {
   ATTRIB_ANIM_NUMPOSES,
   ATTRIB_ANIM_RATE,
   ATTRIB_ANIM_SAVE
};

StringMap _animAttributes(
   "numPoses", ATTRIB_ANIM_NUMPOSES,
   "rate",     ATTRIB_ANIM_RATE,
   "save",     ATTRIB_ANIM_SAVE,
   ""
);

//------------------------------------------------------------------------------
//!
int animSaveVM( VMState* vm )
{
   SkeletalAnimation* anim = (SkeletalAnimation*)VM::thisPtr( vm );
   String path             = VM::toString( vm, 1 );
   if( !anim ) return 0;
   VM::push( vm, ResExporter::save( *anim, path ) );
   return 1;
}

//------------------------------------------------------------------------------
//!
int anim_create( VMState* vm )
{
   VMObjectPool* pool      = (VMObjectPool*)VM::thisPtr( vm );
   SkeletalAnimation* anim = new SkeletalAnimation();
   pool->add( anim, anim );
   VM::pushProxy( vm, anim );
   return 1;
}

//------------------------------------------------------------------------------
//!
int anim_get( VMState* vm )
{
   SkeletalAnimation* a = (SkeletalAnimation*)VM::toProxy( vm, 1 );

   switch( _animAttributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_ANIM_NUMPOSES:
         VM::push( vm, a->numPoses() );
         return 1;
      case ATTRIB_ANIM_RATE:
         VM::push( vm, a->rate() );
         return 1;
      case ATTRIB_ANIM_SAVE:
         VM::push( vm, a, animSaveVM );
         return 1;
  }
   return 0;
}

//------------------------------------------------------------------------------
//!
int anim_set( VMState* )
{
   // Nothing for now.
   return 0;
}

//------------------------------------------------------------------------------
//!
int loadAnimVM( VMState* vm )
{
   VMObjectPool* pool = (VMObjectPool*)VM::thisPtr( vm );
   int numArgs        = VM::getTop(vm);
   String name        = VM::toString( vm, 1 );

   RCP< Resource<SkeletalAnimation> > res = ResManager::getAnimation( name, 0 );

   // Do we have a callback?
   if( VM::isFunction( vm, numArgs ) )
   {
      if( res.isValid() )
      {
         VMCb* cb  = new VMCb;
         cb->_pool = pool;
         VM::toRef( vm, numArgs, cb->_ref );
         res->callOnLoad( makeDelegate( cb, &VMCb::executeCb<SkeletalAnimation> ) );
      }
      else
      {
         VM::pushProxy( vm, 0 );
         VM::ecall( vm, 1, 0 );
      }
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int concatenateAnimVM( VMState* vm )
{
   VMObjectPool* pool             = (VMObjectPool*)VM::thisPtr( vm );
   SkeletalAnimation* anim1       = (SkeletalAnimation*)VM::toProxy( vm, 1 );
   SkeletalAnimation* anim2       = (SkeletalAnimation*)VM::toProxy( vm, 2 );
   RCP<SkeletalAnimation> newAnim = Puppeteer::concatenate( anim1, anim2 );
   VM::pushProxy( vm, newAnim.ptr() );

   pool->add( newAnim.ptr(), newAnim.ptr() );
   return 1;
}

//------------------------------------------------------------------------------
//!
int cutAnimVM( VMState* vm )
{
   VMObjectPool* pool             = (VMObjectPool*)VM::thisPtr( vm );
   SkeletalAnimation* anim        = (SkeletalAnimation*)VM::toProxy( vm, 1 );
   uint start                     = VM::toUInt( vm, 2 );
   uint end                       = VM::toUInt( vm, 3 );
   RCP<SkeletalAnimation> newAnim = Puppeteer::cut( anim, start, end );
   VM::pushProxy( vm, newAnim.ptr() );

   pool->add( newAnim.ptr(), newAnim.ptr() );
   return 1;
}

//------------------------------------------------------------------------------
//!
int cycleAnimVM( VMState* vm )
{
   VMObjectPool* pool             = (VMObjectPool*)VM::thisPtr( vm );
   SkeletalAnimation* anim        = (SkeletalAnimation*)VM::toProxy( vm, 1 );
   float fraction                 = VM::toFloat( vm, 2 );
   RCP<SkeletalAnimation> newAnim = Puppeteer::cycle( anim, fraction );
   VM::pushProxy( vm, newAnim.ptr() );

   pool->add( newAnim.ptr(), newAnim.ptr() );
   return 1;
}

//------------------------------------------------------------------------------
//!
int resampleAnimVM( VMState* vm )
{
   VMObjectPool* pool             = (VMObjectPool*)VM::thisPtr( vm );
   SkeletalAnimation* anim        = (SkeletalAnimation*)VM::toProxy( vm, 1 );
   float rate                     = VM::toFloat( vm, 2 );
   RCP<SkeletalAnimation> newAnim = Puppeteer::resample( anim, rate );
   VM::pushProxy( vm, newAnim.ptr() );

   pool->add( newAnim.ptr(), newAnim.ptr() );
   return 1;
}

//------------------------------------------------------------------------------
//!
int reverseAnimVM( VMState* vm )
{
   VMObjectPool* pool             = (VMObjectPool*)VM::thisPtr( vm );
   SkeletalAnimation* anim        = (SkeletalAnimation*)VM::toProxy( vm, 1 );
   RCP<SkeletalAnimation> newAnim = Puppeteer::reverse( anim );
   VM::pushProxy( vm, newAnim.ptr() );

   pool->add( newAnim.ptr(), newAnim.ptr() );
   return 1;
}

UNNAMESPACE_END


/*==============================================================================
   NAMESPACE RESOURCEVM
==============================================================================*/

NAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
void ResourceVM::initialize()
{
   // World.
   VMObjectPool::registerObject(
      "RES",
      "world",
      world_create,
      world_get,
      world_set
   );
   VMObjectPool::registerCreate( "RES", "newWorld", newWorldVM );

   // Data flow graph.
   VMObjectPool::registerObject(
      "RES",
      DFGraph::staticMeta(),
      dfgraph_create,
      dfgraph_get,
      dfgraph_set
   );
   VMObjectPool::registerCreate( "RES", "loadDataFlowGraph", loadDFGraphVM );

   VMRegistry::add( df_init, VM_CAT_APP );

   // Animation.
   VMObjectPool::registerObject(
      "RES",
      "skeletalAnimation",
      anim_create,
      anim_get,
      anim_set
   );
   VMObjectPool::registerCreate( "RES", "loadAnimation",        loadAnimVM );
   VMObjectPool::registerCreate( "RES", "concatenateAnimation", concatenateAnimVM );
   VMObjectPool::registerCreate( "RES", "cutAnimation",         cutAnimVM );
   VMObjectPool::registerCreate( "RES", "cycleAnimation",       cycleAnimVM );
   VMObjectPool::registerCreate( "RES", "resampleAnimation",    resampleAnimVM );
   VMObjectPool::registerCreate( "RES", "reverseAnimation",     reverseAnimVM );
}

//------------------------------------------------------------------------------
//!
void ResourceVM::push( VMState* vm, const DFNodeAttr& attr )
{
   VM::newTable( vm );

   if( attr.extras() )
   {
      // Do the extras first so that 'id' or 'type'
      // below get precedence.
      VM::push( vm, -1, *(attr.extras()) );
   }

   VM::push( vm, "type" );
   VM::push( vm, attr.type() );
   VM::set( vm, -3 );

   VM::push( vm, "id" );
   VM::push( vm, attr.id() );
   VM::set( vm, -3 );

   if( attr.pid() != DFNodeAttr::INVALID_ID )
   {
      VM::push( vm, "pid" );
      VM::push( vm, attr.pid() );
      VM::set( vm, -3 );
   }

   if( attr.sid() != DFNodeAttr::INVALID_ID )
   {
      VM::push( vm, "sid" );
      VM::push( vm, attr.sid() );
      VM::set( vm, -3 );
   }

   VM::push( vm, "label" );
   VM::push( vm, attr.label() );
   VM::set( vm, -3 );

   // Attributes.
   const DFNodeAttrList* attribs = attr.attributes();
   if( attribs )
   {
      uint idx = 0;
      for( auto cur = attribs->begin(); cur != attribs->end(); ++cur )
      {
         ResourceVM::push( vm, *cur );
         VM::seti( vm, -2, ++idx );
      }
   }
}

//------------------------------------------------------------------------------
//!
void ResourceVM::push( VMState* vm, const DFNodeAttrList& attrs )
{
   VM::newTable( vm );
   uint idx = 0;
   for( auto cur = attrs.begin(); cur != attrs.end(); ++cur )
   {
      ResourceVM::push( vm, *cur );
      VM::seti( vm, -2, ++idx );
   }
}


NAMESPACE_END


