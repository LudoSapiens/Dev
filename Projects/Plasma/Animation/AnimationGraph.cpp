/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Animation/AnimationGraph.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN
/*
//------------------------------------------------------------------------------
//!
enum
{
   ATTRIB_ADD_NODE,
   ATTRIB_ADD_TRANSITION,
   ATTRIB_ADD_TRANSITIONS,
   ATTRIB_REMOVE_NODE,
   ATTRIB_REMOVE_TRANSITION,
   NUM_ATTRIBS
};

StringMap _attributes(
   "addNode",          ATTRIB_ADD_NODE,
   "addTransition",    ATTRIB_ADD_TRANSITION,
   "addTransitions",   ATTRIB_ADD_TRANSITIONS,
   "removeNode",       ATTRIB_REMOVE_NODE,
   "removeTransition", ATTRIB_REMOVE_TRANSITION,
   ""
);

//------------------------------------------------------------------------------
//!
int
addNodeVM( VMState* vm )
{
   AnimationGraph* graph = (AnimationGraph*)VM::thisPtr( vm );
   CHECK( graph != NULL );

   RCP<AnimationNode> node = AnimationNodeVM::to( vm, 1 );
   CHECK( node.isValid() );

   graph->addNode( node );

   return 0;
}

//------------------------------------------------------------------------------
//!
int
addTransitionVM( VMState* vm )
{
   AnimationGraph* graph = (AnimationGraph*)VM::thisPtr( vm );
   CHECK( graph != NULL );

   if( VM::getTop( vm ) == 3 )
   {
      graph->addTransition(
         AnimationNodeVM::to( vm, 1 ),
         AnimationNodeVM::to( vm, 2 ),
         (float)VM::toNumber( vm, 3 )
      );
   }
   else
   {
      graph->addTransition(
         AnimationNodeVM::to( vm, 1 ),
         AnimationNodeVM::to( vm, 2 ),
         (float)VM::toNumber( vm, 3 ),
         VM::toBoolean( vm, 4 )
      );
   }
   
   return 0;
}

//------------------------------------------------------------------------------
//!
int
addTransitionsVM( VMState* vm )
{
   AnimationGraph* graph = (AnimationGraph*)VM::thisPtr( vm );
   CHECK( graph != NULL );
   
   graph->addTransitions(
      AnimationNodeVM::to( vm, 1 ),
      AnimationNodeVM::to( vm, 2 ),
      (float)VM::toNumber( vm, 3 )
   );

   return 0;
}

//------------------------------------------------------------------------------
//!
int
removeNodeVM( VMState* vm )
{
   AnimationGraph* graph = (AnimationGraph*)VM::thisPtr( vm );
   CHECK( graph != NULL );

   RCP<AnimationNode> node = AnimationNodeVM::to( vm, 1 );
   CHECK( node.isValid() );

   graph->removeNode( node );

   return 0;
}

//------------------------------------------------------------------------------
//!
int
removeTransitionVM( VMState* vm )
{
   AnimationGraph* graph = (AnimationGraph*)VM::thisPtr( vm );
   CHECK( graph != NULL );
   
   graph->removeTransition(
      graph->transition(
         AnimationNodeVM::to( vm, 1 ).ptr(),
         AnimationNodeVM::to( vm, 2 ).ptr()
      )
   );
   
   return 0;
}
*/

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
  CLASS AnimationGraph::Transition
  ==============================================================================*/

//------------------------------------------------------------------------------
//!
AnimationGraph::Transition::Transition(
   AnimationNode* node1,
   AnimationNode* node2,
   float          duration,
   bool           interruptible
) :
   _node1( node1 ),
   _node2( node2 ),
   _duration( duration ),
   _interruptible( interruptible )
{
}

/*==============================================================================
  CLASS AnimationGraph::Path
  ==============================================================================*/

//------------------------------------------------------------------------------
//!
AnimationGraph::Path::Path()
{
}

//------------------------------------------------------------------------------
//!
bool
AnimationGraph::Path::isValid() const
{
   uint numTrans = numTransitions();
   if( numTrans != 0 )
   {
      const Transition* oldT = transition(0);
      for( uint t = 1; t < numTrans; ++t )
      {
         const Transition* newT = transition(t);
         if( oldT->node2() != newT->node1() ) return false;
         oldT = newT;
      }
   }
   return true;
}

//------------------------------------------------------------------------------
//! Computes the total duration of a path by iterating over all of its transitions.
float
AnimationGraph::Path::computeDuration() const
{
   float dur = 0.0f;
   for( auto cur = _transitions.begin(); cur != _transitions.end(); ++cur )
   {
      dur += (*cur)->duration();
   }
   return dur;
}

//------------------------------------------------------------------------------
//! Makes a copy of the path, from transactions sidx to eidx inclusive.
RCP<AnimationGraph::Path>
AnimationGraph::Path::clone( uint sidx, uint eidx )
{
   RCP<Path> path = new Path();
   uint n = numTransitions();
   if( n != 0 )
   {
      --n; // now last valid index
      CGM::clampMax( sidx, n );
      CGM::clampMax( eidx, n );
      path->reserveTransitions( eidx - sidx + 1 );
      for( uint idx = sidx; idx <= eidx; ++idx )
      {
         path->add( transition(idx) );
      }
   }
   return path;
}

/*==============================================================================
  CLASS AnimationGraph
==============================================================================*/

//------------------------------------------------------------------------------
//!
AnimationGraph::AnimationGraph()
{
}

//------------------------------------------------------------------------------
//!
AnimationGraph::~AnimationGraph()
{
}

//------------------------------------------------------------------------------
//! 
RCP<AnimationGraph>
AnimationGraph::clone() const
{
   RCP<AnimationGraph> graph( new AnimationGraph() );

   // Clone nodes.
   for( auto it = _nodes.begin(); it != _nodes.end(); ++it )
   {
      graph->addNode( (*it).second->clone().ptr() );
   }
   // Clone transitions.
   for( auto it = _transitions.begin(); it != _transitions.end(); ++it )
   {
      graph->addTransition(
         graph->node( (*it).first.first->name() ),
         graph->node( (*it).first.second->name() ),
         (*it).second->duration(),
         (*it).second->interruptible()
      );
   }
   // Jump node.
   graph->_onFall = _onFall;

   return graph;
}

//------------------------------------------------------------------------------
//! 
void AnimationGraph::getAnimations( Vector< SkeletalAnimation* >& anims )
{
   for( auto it = _nodes.begin(); it != _nodes.end(); ++it )
   {
      (*it).second->getAnimations( anims );
   }
}

//------------------------------------------------------------------------------
//! 
void AnimationGraph::replaceAnimations( 
   const Map< SkeletalAnimation*, RCP<SkeletalAnimation> >& anims 
)
{
   for( auto it = _nodes.begin(); it != _nodes.end(); ++it )
   {
      (*it).second->replaceAnimations( anims );
   }
}

//------------------------------------------------------------------------------
//!
void 
AnimationGraph::removeNode( AnimationNode* node )
{
   _nodes.erase( node->name() );
   removeTransitionsWith( node );
}

//------------------------------------------------------------------------------
//!
void 
AnimationGraph::addTransition( 
   AnimationNode* node1,
   AnimationNode* node2,
   float          duration,
   bool           interruptible
)
{
   RCP<Transition> transition = new Transition( node1, node2, duration, interruptible );
   _transitions[TransitionPair( node1, node2 )] = transition;
}

//------------------------------------------------------------------------------
//!
void 
AnimationGraph::addTransitions( 
   AnimationNode* node1,
   AnimationNode* node2,
   float          duration
)
{
   RCP<Transition> transition = new Transition( node1, node2, duration );
   _transitions[TransitionPair( node1, node2 )] = transition;
   
   transition = new Transition( node2, node1, duration );
   _transitions[TransitionPair( node2, node1 )] = transition;
}

//------------------------------------------------------------------------------
//!
void 
AnimationGraph::removeTransition( Transition* tr )
{
   _transitions.erase( TransitionPair( tr->node1(), tr->node2() ) );
}

//------------------------------------------------------------------------------
//!
void 
AnimationGraph::removeTransitionsWith( AnimationNode* node )
{
   for( auto it = _transitions.begin(); it != _transitions.end(); ++it )
   {
      const TransitionPair& pair = (*it).first;
      if( pair.first == node || pair.second == node )
      {
         _transitions.erase( it );
      }
   }
}

//------------------------------------------------------------------------------
//!
RCP<AnimationGraph::Path>
AnimationGraph::path( AnimationNode* from, AnimationNode* to )
{
   TransitionPair pair(from, to);

   // Look for a direct path.
   Transition* tr = transition( from, to );
   if( tr )
   {
      RCP<Path> path = new Path();
      path->add( tr );
      return path;
   }
   // Look for an indirect path.
   RCP<Path> path = searchPath( pair );
   if( path.isValid() ) return path.ptr();
   return 0;
}

//------------------------------------------------------------------------------
//! Search indirect path of length 2 only.
//! Does not check direct paths.
//! Performs in O(n^2), but since this is depth of 2, is O(2n) = O(n).
//! Uses O(1) in memory (2 iterators).
RCP<AnimationGraph::Path>
AnimationGraph::searchPath( const TransitionPair& pair )
{
   RCP<Path> path;
   const AnimationNode* desFromNode = pair.first;
   const AnimationNode* desToNode   = pair.second;
   // Iterate over all of the possible transitions.
   for( auto curA = _transitions.begin(); curA != _transitions.end(); ++curA )
   {
      const RCP<Transition>& trA = (*curA).second;
      // Find transitions starting at the same node.
      if( trA->node1() == desFromNode )
      {
         // Search all of its neighbors, and see if it connects with the 'to' node.
         for( auto curB = _transitions.begin(); curB != _transitions.end(); ++curB )
         {
            const RCP<Transition>& trB = (*curB).second;
            if( trA->node2() == trB->node1() && trB->node2() == desToNode )
            {
               /**
               StdErr << "Found indirect path: "
                      << trA->node1()->name() << "-"
                      << trB->node1()->name() << "-"
                      << trB->node2()->name() << nl;
               **/
               path = new Path();
               path->add( trA.ptr() );
               path->add( trB.ptr() );
               return path; // Early out at first solution found.
            }
         }
      }
   }
   return path;
}
/*
//------------------------------------------------------------------------------
//!
void
AnimationGraph::init( VMState* )
{
   // Nothing todo for now.
}

//------------------------------------------------------------------------------
//!
bool
AnimationGraph::performGet( VMState* vm )
{
   switch( _attributes[VM::toCString( vm, -1 )] )
   {
      case ATTRIB_ADD_NODE:
         VM::push( vm, this, addNodeVM );
         return true;
      case ATTRIB_ADD_TRANSITION:
         VM::push( vm, this, addTransitionVM );
         return true;
      case ATTRIB_ADD_TRANSITIONS:
         VM::push( vm, this, addTransitionsVM );
         return true;
      case ATTRIB_REMOVE_NODE:
         VM::push( vm, this, removeNodeVM );
         return true;
      case ATTRIB_REMOVE_TRANSITION:
         VM::push( vm, this, removeTransitionVM );
         return true;
      default: break;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
bool
AnimationGraph::performSet( VMState* vm )
{
   switch( _attributes[VM::toCString( vm, -2 )] )
   {
      case ATTRIB_ADD_NODE:           return true;
      case ATTRIB_ADD_TRANSITION:     return true;
      case ATTRIB_ADD_TRANSITIONS:    return true;
      case ATTRIB_REMOVE_NODE:        return true;
      case ATTRIB_REMOVE_TRANSITION:  return true;
      default: break;
   }
   return false;
}
*/

NAMESPACE_END
