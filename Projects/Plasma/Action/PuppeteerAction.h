/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_PUPPETEERACTION_H
#define PLASMA_PUPPETEERACTION_H

#include <Plasma/StdDefs.h>

#include <Plasma/Action/Action.h>
#include <Plasma/World/SkeletalEntity.h>
#include <Plasma/Animation/AnimationGraph.h>

NAMESPACE_BEGIN


/*==============================================================================
  CLASS PuppeteerAction
==============================================================================*/

//! Handle skeletal animation and transition and blending.
//!
class PuppeteerAction:
   public Action
{
public:

   /*----- static methods -----*/

   PLASMA_DLL_API static void initialize();
   PLASMA_DLL_API static void terminate();

   /*----- methods -----*/

   PLASMA_DLL_API PuppeteerAction();

   PLASMA_DLL_API static  const ConstString&  actionType();
   PLASMA_DLL_API virtual const ConstString&  type() const;

   PLASMA_DLL_API virtual bool execute( Entity& entity, double time, double delta );

   // Attributes.
   inline void entity( SkeletalEntity* ent )   { _params.entity( ent ); updatePose(); }
   inline       SkeletalEntity* entity()       { return _params.entity(); }
   inline const SkeletalEntity* entity() const { return _params.entity(); }

   // Graph.
   inline void graph( AnimationGraph* gr )         { _graph = gr; }
   inline const RCP<AnimationGraph>& graph() const { return _graph; }

   // Commands.
   bool setState( const ConstString& name, const bool mandatory = false );

   // VM.
   virtual bool performGet( VMState* );
   virtual bool performSet( VMState* );

protected:

   /*----- methods -----*/

   PLASMA_DLL_API virtual ~PuppeteerAction();

   PLASMA_DLL_API void updatePose();
   void checkFalling();
   void startFalling();

private:

   /*----- data members -----*/

   RCP<AnimationGraph>        _graph;        //!< The graph used to define valid transitions.

   // Information about the current goal.
   AnimationNode*             _desNode;      //!< The user-specified desired animation node.
   AnimationNode*             _manNode;      //!< Indicates the mandatory (NULL means no mandatory node is needed).

   // Information about the current transition.
   RCP<AnimationGraph::Path>  _curPath;      //!< The path containing the transition.
   uint                       _trIdx;        //!< The transition index.
   double                     _trTime;       //!< The current time inside the transition.

   // Information about the current node.
   AnimationNode*             _curNode;      //!< The current animation node.

   // Information shared by both transition and animation cases.
   RCP<AnimationState>        _node1State;   //!< The state for curTrans->node1 (in transition) or _curNode.
   RCP<AnimationState>        _node2State;   //!< The state for curTrans->node2 (in transition) or NULL otherwise.

   RCP<SkeletalPose>          _pose;         //!< The pose reused in real-time blending.

   AnimationParameter         _params;
   bool                       _lastFalling;  //!< The previous value of falling (from the CharacterConstraint).
   bool                       _reallyFalling;//!< A flag indicating that a started fall is now considered by the action.
   float                      _fallDistSq;   //!< The minimum fall distance (squared) to consider a fall an actual, real fall.
   float                      _fallMinDur;   //!< The minimum fall duration to consider a fall an actual, real fall.
};


NAMESPACE_END

#endif // PLASMA_PUPPETEERACTION_H
