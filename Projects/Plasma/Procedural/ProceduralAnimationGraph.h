/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_PROCEDURAL_ANIMATIONGRAPH_H
#define PLASMA_PROCEDURAL_ANIMATIONGRAPH_H

#include <Plasma/StdDefs.h>
#include <Plasma/Animation/AnimationGraph.h>
#include <Plasma/Procedural/ProceduralContext.h>

#include <Fusion/Resource/ResourceTask.h>

#include <Base/Util/RCP.h>
#include <Base/ADT/String.h>

NAMESPACE_BEGIN

class BasicNode;
class TravelNode;

/*==============================================================================
   CLASS AnimationGraphContext
==============================================================================*/

class AnimationGraphContext:
   public ProceduralContext
{
public:

   /*----- types -----*/

   typedef Pair< BasicNode*,  Resource<SkeletalAnimation>* > BasicAnimPair;
   typedef Pair< TravelNode*, Resource<SkeletalAnimation>* > TravelAnimPair;

   /*----- methods -----*/

   AnimationGraphContext( Task* t ): ProceduralContext( ANIMGRAPH, t ) {}

   inline void setBasicAnimation( BasicNode* n, Resource<SkeletalAnimation>* res )
   {
      if( res ) _basicRes.pushBack( BasicAnimPair( n, res ) );
   }
   inline void setTravelAnimation( TravelNode* n, Resource<SkeletalAnimation>* res )
   {
      if( res ) _travelRes.pushBack( TravelAnimPair( n, res ) );
   }
   inline void keepResource( RCObject* res ) { _res.pushBack( res ); }

   /*----- data members -----*/

   AnimationGraph*               _graph;
   Vector< RCP<AnimationNode > > _nodes;
   Vector< RCP<RCObject> >       _res;
   Vector< BasicAnimPair >       _basicRes;
   Vector< TravelAnimPair >      _travelRes;
   Set< AnimationNode* >         _subNodes;
};


/*==============================================================================
   CLASS ProceduralAnimationGraph
==============================================================================*/

class ProceduralAnimationGraph:
   public ResourceTask
{
public:

   /*----- static methods -----*/

   static void initialize();

   /*----- methods -----*/

   ProceduralAnimationGraph( Resource<AnimationGraph>* res, const String& path );
 
protected:

   /*----- methods -----*/

   virtual void execute();

   /*----- data members -----*/

   RCP< Resource<AnimationGraph> > _res;
   String                          _path;
};

NAMESPACE_END

#endif
