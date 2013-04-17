/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_JUMPNODE_H
#define PLASMA_JUMPNODE_H

#include <Plasma/StdDefs.h>
#include <Plasma/Animation/AnimationNode.h>


NAMESPACE_BEGIN

class JumpState;

/*==============================================================================
   CLASS JumpNode
==============================================================================*/

//! A jump node has 3 parts:
//!  - Launching
//!  - Flying
//!  - Landing
//! Only the execution portion can loop.

class JumpNode :
   public AnimationNode
{

public:

   /*----- methods -----*/

   JumpNode();

   virtual RCP<AnimationNode> clone() const;
   virtual RCP<AnimationState> createState( AnimationParameter* p ) const;
   virtual void perform( AnimationState*, double deltaTime );

   virtual void getAnimations( Vector< SkeletalAnimation* >& anims );
   virtual void replaceAnimations( 
      const Map< SkeletalAnimation*, RCP<SkeletalAnimation> >& anims 
   );

   void launching( AnimationNode* n )   { _launching = n; }
   void flying( AnimationNode* n )      { _flying    = n; }
   void landing( AnimationNode* n )     { _landing   = n; }
   AnimationNode*  launching() const    { return _launching.ptr(); }
   AnimationNode*  flying() const       { return _flying.ptr(); }
   AnimationNode*  landing() const      { return _landing.ptr(); }

   double launchBlendStartTime() const  { return _launchBlendStartTime; }
   double launchBlendEndTime() const    { return _launchBlendEndTime; }
   double landBlendTime() const         { return _landBlendTime; }

   void launchBlendStartTime( double t )  { _launchBlendStartTime = t; }
   void launchBlendEndTime( double t )    { _launchBlendEndTime = t; }
   void landBlendTime( double t )         { _landBlendTime = t; }

   void  applyVelocity( JumpState* state ) const;

protected:

   /*----- methods -----*/

   virtual ~JumpNode();

private:

   /*----- data members -----*/

   RCP<AnimationNode>   _launching;
   RCP<AnimationNode>   _flying;
   RCP<AnimationNode>   _landing;

   double               _launchBlendStartTime;
   double               _launchBlendEndTime;
   double               _landBlendTime;
};

/*==============================================================================
   CLASS JumpState
==============================================================================*/

class JumpState :
      public AnimationState
{

public:

   /*----- methods -----*/

   // Attributes getters.
   inline double time() const { return _time; }
   inline AnimationState* launchingState() const { return _launchingState.ptr(); }
   inline AnimationState* flyingState() const    { return _flyingState.ptr(); }
   inline AnimationState* landingState() const   { return _landingState.ptr(); }
   inline const Vec3f& initialVelocity() const   { return _initialVelocity; }

   // Attributes setters.
   inline void time( double v ) { _time = v; }
   inline void initialVelocity( const Vec3f& v )   { _initialVelocity = v; }
   inline void launchingState( AnimationState* s ) { _launchingState = s; }
   inline void flyingState( AnimationState* s )    { _flyingState = s; }
   inline void landingState( AnimationState* s )   { _landingState = s; }

protected:

   /*----- data members -----*/

   double               _time;             //!< The current time inside the node.
   Vec3f                _initialVelocity;  //!< The velocity saved at the beginning of the node's execution.
   RCP<AnimationState>  _launchingState;   //!< The state used for the launching phase.
   RCP<AnimationState>  _flyingState;      //!< The state used for the flying phase.
   RCP<AnimationState>  _landingState;     //!< The state used for the landing phase.
};

NAMESPACE_END

#endif
