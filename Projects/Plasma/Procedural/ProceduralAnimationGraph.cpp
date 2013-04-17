/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Procedural/ProceduralAnimationGraph.h>
#include <Plasma/Animation/BasicNode.h>
#include <Plasma/Animation/JumpNode.h>
#include <Plasma/Animation/TravelNode.h>
#include <Plasma/Resource/ResManager.h>

#include <Fusion/VM/VMRegistry.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
inline AnimationGraphContext* getContext( VMState* vm )
{
   return (AnimationGraphContext*)VM::userData(vm);
}

/*==============================================================================
   Procedural API/functions.
==============================================================================*/

//------------------------------------------------------------------------------
//!
int basicNodeVM( VMState* vm )
{
   AnimationGraphContext* context = getContext(vm);

   BasicNode* node = new BasicNode();

   const char* name = 0;
   VM::get( vm, -1, "name", name );
   node->name( ConstString(name) );

   for( uint i = 1; VM::geti( vm, 1, i ); ++i )
   {
      context->setBasicAnimation( node, (Resource<SkeletalAnimation>*)VM::toPtr( vm, -1 ) );
      VM::pop(vm);
   }

   context->_nodes.pushBack( node );
   VM::push( vm, node );
   return 1;
}

//------------------------------------------------------------------------------
//!
int jumpNodeVM( VMState* vm )
{
   AnimationGraphContext* context = getContext(vm);

   JumpNode* node = new JumpNode();

   const char* name = 0;
   VM::get( vm, -1, "name", name );
   node->name( ConstString(name) );

   AnimationNode* snode = 0;
   VM::geti( vm, 1, 1 );
   snode = (BasicNode*)VM::toPtr( vm, -1 );
   context->_subNodes.add( snode );
   node->launching( snode );
   VM::pop(vm);

   VM::geti( vm, 1, 2 );
   snode = (BasicNode*)VM::toPtr( vm, -1 );
   context->_subNodes.add( snode );
   node->flying( snode );
   VM::pop(vm);

   VM::geti( vm, 1, 3 );
   snode = (BasicNode*)VM::toPtr( vm, -1 );
   context->_subNodes.add( snode );
   node->landing( snode );
   VM::pop(vm);

   double t = 0.0;
   if( VM::geti( vm, 1, 4 ) )
   {
      t = VM::toNumber( vm, -1 );
      VM::pop( vm );
   }
   node->launchBlendStartTime( t );

   if( VM::geti( vm, 1, 5 ) )
   {
      t = VM::toNumber( vm, -1 );
      VM::pop( vm );
   }
   node->launchBlendEndTime( t );

   if( VM::geti( vm, 1, 6 ) )
   {
      t = VM::toNumber( vm, -1 );
      VM::pop( vm );
   }
   node->landBlendTime( t );

   context->_nodes.pushBack( node );
   VM::push( vm, node );
   return 1;
}

//------------------------------------------------------------------------------
//!
int travelNodeVM( VMState* vm )
{
   AnimationGraphContext* context = getContext(vm);

   TravelNode* node = new TravelNode();

   const char* name = 0;
   VM::get( vm, -1, "name", name );
   node->name( ConstString(name) );

   for( uint i = 1; VM::geti( vm, 1, i ); ++i )
   {
      context->setTravelAnimation( node, (Resource<SkeletalAnimation>*)VM::toPtr( vm, -1 ) );
      VM::pop(vm);
   }

   context->_nodes.pushBack( node );
   VM::push( vm, node );
   return 1;
}

//------------------------------------------------------------------------------
//!
int transitionVM( VMState* vm )
{
   AnimationGraphContext* context = getContext(vm);
   AnimationNode* nodeA = (AnimationNode*)VM::toPtr( vm, 1 );
   AnimationNode* nodeB = (AnimationNode*)VM::toPtr( vm, 2 );
   float duration       = VM::toFloat( vm, 3 );

   context->_graph->addTransition( nodeA, nodeB, duration );

   return 0;
}

//------------------------------------------------------------------------------
//!
int transitionsVM( VMState* vm )
{
   AnimationGraphContext* context = getContext(vm);
   AnimationNode* nodeA = (AnimationNode*)VM::toPtr( vm, 1 );
   AnimationNode* nodeB = (AnimationNode*)VM::toPtr( vm, 2 );
   float duration       = VM::toFloat( vm, 3 );

   context->_graph->addTransitions( nodeA, nodeB, duration );

   return 0;
}

//-----------------------------------------------------------------------------
//!
int onFallVM( VMState* vm )
{
   AnimationGraphContext* context = getContext(vm);
   ConstString name = VM::toCString( vm, 1 );
   context->_graph->onFall( name );
   return 0;
}

//------------------------------------------------------------------------------
//!
int animationVM( VMState* vm )
{
   AnimationGraphContext* context = getContext(vm);
   int n = VM::getTop(vm);

   // First argument.
   String name;
   if( n >= 1 )
   {
      name = VM::toString( vm, 1 );
      name = ResManager::expand( context->curDir(), name );
   }
   else
   {
      StdErr << "animation() - Missing name." << nl;
      return 0;
   }

   RCP< Resource<SkeletalAnimation> > res = ResManager::getAnimation( name, context->task() );
   context->keepResource( res.ptr() );
   VM::push( vm, res.ptr() );
   return 1;
}

//------------------------------------------------------------------------------
//!
const VM::Reg funcs[] = {
   // Node.
   { "basicNode",   basicNodeVM  },
   { "jumpNode",    jumpNodeVM   },
   { "travelNode",  travelNodeVM },
   // Transition.
   { "transition",  transitionVM  },
   { "transitions", transitionsVM },
   // Event.
   { "onFall",      onFallVM },
   // Animation.
   { "animation",   animationVM },
   { 0,0 }
};

//------------------------------------------------------------------------------
//!
void initVM( VMState* vm, uint /*mask*/ )
{
   VM::registerFunctions( vm, "_G", funcs );
}

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   CLASS ProceduralAnimationGraph
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
ProceduralAnimationGraph::initialize()
{
   VMRegistry::add( initVM, VM_CAT_AGRAPH );
}

//------------------------------------------------------------------------------
//!
ProceduralAnimationGraph::ProceduralAnimationGraph(
   Resource<AnimationGraph>* res,
   const String&             path
):
   _res( res ),
   _path( path )
{
}

//------------------------------------------------------------------------------
//!
void
ProceduralAnimationGraph::execute()
{
   RCP<AnimationGraph> graph( new AnimationGraph() );

   // Create working context for this vm.
   AnimationGraphContext context( this );
   context._graph = graph.ptr();
   context.curDir( ResManager::dir( ResManager::getAnimationGraphName(_res.ptr()) ) );

   // Prepare to build the animation.
   VMState* vm = VM::open( VM_CAT_AGRAPH | VM_CAT_MATH, true );

   // Keep context.
   VM::userData( vm, &context );

   // Execute script.
   VM::doFile( vm, _path, 0 );

   // Wait for all auxilliary task (loading resource) to finish.
   waitForAll();

   // Assign all resources.
   for( size_t i = 0; i < context._basicRes.size(); ++i )
   {
      AnimationGraphContext::BasicAnimPair& p = context._basicRes[i];
      p.first->addAnimation( waitForData( p.second ) );
   }
   for( size_t i = 0; i < context._travelRes.size(); ++i )
   {
      AnimationGraphContext::TravelAnimPair& p = context._travelRes[i];
      p.first->addAnimation( waitForData( p.second ) );
   }
   // Add all nodes to the graph.
   for( size_t i = 0; i < context._nodes.size(); ++i )
   {
      AnimationNode* node = context._nodes[i].ptr();
      if( !context._subNodes.has( node ) )
      {
         graph->addNode( node );
      }
   }

   VM::close( vm );

   _res->data( graph.ptr() );
}

NAMESPACE_END
