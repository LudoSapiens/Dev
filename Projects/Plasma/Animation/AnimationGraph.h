/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_ANIMATIONGRAPH_H
#define PLASMA_ANIMATIONGRAPH_H

#include <Plasma/StdDefs.h>
#include <Plasma/Animation/AnimationNode.h>

#include <Fusion/VM/VMObject.h>

#include <Base/ADT/Map.h>
#include <Base/ADT/Pair.h>
#include <Base/ADT/Vector.h>
#include <Base/Util/RCObject.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS AnimationGraph
==============================================================================*/

//! A class describing a motion graph with transitions for skeletal animation.

class AnimationGraph
   : public RCObject
{

public:

   /*----- classes -----*/

   class Transition;
   class Path;

   /*----- types and enumerations ----*/

   typedef Map< ConstString, RCP<AnimationNode> > NodeContainer;
   typedef Pair< AnimationNode*, AnimationNode* > TransitionPair;
   typedef Map< TransitionPair, RCP<Transition> > TransitionContainer;

   /*----- methods -----*/

   PLASMA_DLL_API AnimationGraph();

   PLASMA_DLL_API RCP<AnimationGraph> clone() const;

   PLASMA_DLL_API void getAnimations( Vector< SkeletalAnimation* >& );
   PLASMA_DLL_API void replaceAnimations( 
      const Map< SkeletalAnimation*, RCP<SkeletalAnimation> >&
   );

   // Nodes.
   inline uint numNodes() const { return uint(_nodes.size()); }
   inline AnimationNode* node( const ConstString& name ) const;
   inline void addNode( AnimationNode* node );
   PLASMA_DLL_API void removeNode( AnimationNode* node );

   // Event nodes.
   inline void  onFall( const ConstString& name );
   inline const ConstString&  onFall() const;

   // Transitions.
   inline uint numTransitions() const { return uint(_transitions.size()); }
   inline Transition* transition( AnimationNode*, AnimationNode* );
   PLASMA_DLL_API void addTransition(
      AnimationNode* node1,
      AnimationNode* node2,
      float          duration,
      bool           interruptable = true
   );
   PLASMA_DLL_API void addTransitions(
      AnimationNode* node1,
      AnimationNode* node2,
      float          duration
   );
   PLASMA_DLL_API void removeTransition( Transition* );
   PLASMA_DLL_API void removeTransitionsWith( AnimationNode* );

   // Paths.
   RCP<Path> path( AnimationNode* from, AnimationNode* to );

protected:

   /*----- methods -----*/

   PLASMA_DLL_API virtual ~AnimationGraph();

   RCP<Path>  searchPath( const TransitionPair& pair );

private:

   /*----- data members -----*/

   NodeContainer       _nodes;
   TransitionContainer _transitions;
   ConstString         _onFall;
};

//------------------------------------------------------------------------------
//!
inline void 
AnimationGraph::addNode( AnimationNode* node )
{
   _nodes[node->name()] = node;
}

//------------------------------------------------------------------------------
//!
inline AnimationNode*
AnimationGraph::node( const ConstString& name ) const
{
   NodeContainer::ConstIterator it = _nodes.find( name );
   if( it != _nodes.end() ) return (*it).second.ptr();
   return 0;
}

//------------------------------------------------------------------------------
//!
inline void
AnimationGraph::onFall( const ConstString& name )
{
   _onFall = name;
}

//------------------------------------------------------------------------------
//!
inline const ConstString&
AnimationGraph::onFall() const
{
   return _onFall;
}

//------------------------------------------------------------------------------
//!
inline AnimationGraph::Transition*
AnimationGraph::transition( AnimationNode* node1, AnimationNode* node2 )
{
   TransitionContainer::ConstIterator it = _transitions.find( TransitionPair( node1, node2 ) );
   if( it != _transitions.end() ) return (*it).second.ptr();
   return 0;
}

/*==============================================================================
   CLASS AnimationGraph::Transition
==============================================================================*/

class AnimationGraph::Transition
   : public RCObject
{

public:

   /*----- methods -----*/

   Transition(
      AnimationNode* node1,
      AnimationNode* node2,
      float          duration,
      bool           interruptible = true
   );
   
   inline AnimationNode* node1() const { return _node1; }
   inline AnimationNode* node2() const { return _node2; }

   inline void duration( float d )     { _duration = d; }
   inline float duration() const       { return _duration; }

   inline void interruptible( bool v ) { _interruptible = v; }
   inline bool interruptible() const   { return _interruptible; }

protected:

   /*----- data members -----*/

   AnimationNode*  _node1;    //!< The source animation.
   AnimationNode*  _node2;    //!< The destination animation.
   float           _duration; //!< The duration of the transition.
   bool            _interruptible; //!< Indicates if the transition can be interrupted or not.
};


/*==============================================================================
  CLASS AnimationGraph::Path
==============================================================================*/

class AnimationGraph::Path
   : public RCObject
{
public:

   /*----- types -----*/

   typedef Vector< RCP<Transition> >  TransitionContainer;

   /*----- methods -----*/

   PLASMA_DLL_API Path();

   inline void add( Transition* t )    { _transitions.pushBack(t); }
   inline void add( Path* p )          { _transitions.append(p->_transitions); }
   inline void remove( Transition* t ) { _transitions.remove(t); }

   inline uint numNodes()      const   { uint nt = numTransitions(); return (nt == 0) ? 0 : nt + 1; }
   inline uint numTransitions() const  { return (uint)_transitions.size(); }

   inline       AnimationNode* node( uint idx )       { return (idx == 0) ? _transitions[0]->node1() : _transitions[idx-1]->node2(); }
   inline const AnimationNode* node( uint idx ) const { return (idx == 0) ? _transitions[0]->node1() : _transitions[idx-1]->node2(); }
   inline       AnimationNode* firstNode()            { return _transitions.front()->node1(); }
   inline const AnimationNode* firstNode() const      { return _transitions.front()->node1(); }
   inline       AnimationNode* lastNode()             { return _transitions.back()->node2(); }
   inline const AnimationNode* lastNode() const       { return _transitions.back()->node2(); }

   inline       Transition* transition( uint idx )       { return _transitions[idx].ptr(); }
   inline const Transition* transition( uint idx ) const { return _transitions[idx].ptr(); }
   inline void  reserveTransitions( uint n )             { _transitions.reserve(n); }

   PLASMA_DLL_API bool  isValid() const;

   PLASMA_DLL_API float  computeDuration() const;

   PLASMA_DLL_API RCP<Path>  clone( uint sidx = 0, uint eidx = (uint)-1 );

protected:

   /*----- data members -----*/

   TransitionContainer  _transitions;

}; //class AnimationGraph::Path

NAMESPACE_END

#endif //PLASMA_ANIMATIONGRAPH_H
