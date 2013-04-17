/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Animation/JumpNode.h>

#include <Plasma/Animation/Puppeteer.h>
#include <Plasma/Stimulus/EventStimuli.h>
#include <Plasma/World/World.h>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
inline void
runPerform(
   double           deltaTime,
   AnimationNode*   n0,
   AnimationState*  s0,
   AnimationNode*   n1,
   AnimationState*  s1,
   float            f,
   AnimationState*  dst
)
{
   n0->perform( s0, deltaTime );
   n1->perform( s1, deltaTime );

   Puppeteer::blendPoses(
      s0->pose(),
      s1->pose(),
      f,
      *dst->pose()
   );
   float speed = CGM::linear2( s0->speed(), s1->speed(), f );
   dst->speed( speed );

#if 0
   Vec2f angVel = CGM::linear(
      s0->angularVelocity(),
      s1->angularVelocity() - s0->angularVelocity(),
      f
   );
   dst->angularVelocity( angVel );
#else
   Quatf o = s0->orientation().slerp( s1->orientation(), f );
   dst->orientation( o );
#endif
}

//------------------------------------------------------------------------------
//!
inline void
runPerform(
   double           deltaTime,
   AnimationNode*   srcNode,
   AnimationState*  srcState,
   AnimationState*  dstState
)
{
   // Simple hack: replace the src->pose with dst->pose
   // so that the result doesn't need to be copied.
   RCP<SkeletalPose> savedPose = srcState->pose();
   srcState->pose( dstState->pose() ); // replace
   srcNode->perform( srcState, deltaTime );
   srcState->pose( savedPose.ptr() );  // restore

   dstState->speed( srcState->speed() );
   dstState->direction( srcState->direction() );
#if 0
   dstState->angularVelocity( srcState->angularVelocity() );
#else
   dstState->orientation( srcState->orientation() );
#endif
}

UNNAMESPACE_END


/*==============================================================================
   CLASS JumpNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
JumpNode::JumpNode()
{
}

//------------------------------------------------------------------------------
//!
JumpNode::~JumpNode()
{
}

//------------------------------------------------------------------------------
//!
RCP<AnimationNode>
JumpNode::clone() const
{
   RCP<JumpNode> node( new JumpNode() );
   node->_name                 = _name;
   node->_launching            = _launching->clone();
   node->_flying               = _flying->clone();
   node->_landing              = _landing->clone();
   node->_launchBlendStartTime = _launchBlendStartTime;
   node->_launchBlendEndTime   = _launchBlendEndTime;
   node->_landBlendTime        = _landBlendTime;
   return node;
}

//------------------------------------------------------------------------------
//!
void JumpNode::getAnimations( Vector< SkeletalAnimation* >& anims )
{
   _launching->getAnimations( anims );
   _flying->getAnimations( anims );
   _landing->getAnimations( anims );
}

//------------------------------------------------------------------------------
//!
void JumpNode::replaceAnimations(
   const Map< SkeletalAnimation*, RCP<SkeletalAnimation> >& anims
)
{
   _launching->replaceAnimations( anims );
   _flying->replaceAnimations( anims );
   _landing->replaceAnimations( anims );
}

//------------------------------------------------------------------------------
//!
RCP<AnimationState>
JumpNode::createState( AnimationParameter* p ) const
{
   RCP<JumpState> state = new JumpState();

   Vec3f dir = p->entity()->orientation().getAxisZ();
   dir.y     = 0.0f;

   state->parameters( p );
   state->canExit( false );
   state->speed( 0.0f );
   state->direction( normalize( dir ) );

   // Specific parameters.
   state->time( 0.0 );
   state->launchingState( _launching->createState(p).ptr() );
   state->flyingState( _flying->createState(p).ptr() );
   state->landingState( _landing->createState(p).ptr() );

   // Special case.
   uint nBones = state->flyingState()->pose()->numBones();
   state->pose( new SkeletalPose( Reff::identity(), nBones ) );

   return state;
}

//------------------------------------------------------------------------------
//!
void
JumpNode::perform( AnimationState* state, double deltaTime )
{
   JumpState* curState = (JumpState*)state;

   double time = curState->time() + deltaTime;

   // Start by handling landing case (since it resets time to 0).
   if( curState->canExit() )
   {
      // Blend flying with landing.
      float f = (_landBlendTime != 0.0) ? (float)(time/_landBlendTime) : 1.0f;
      runPerform(
         deltaTime,
         _flying.ptr(),  curState->flyingState(),
         _landing.ptr(), curState->landingState(),
         f,
         curState
      );
      curState->parameters()->entity()->constrainLinearVelocity( true );
      // Update the current time.
      curState->time( time );
      return;
   }

   if( curState->time() == 0.0 )
   {
      // At the beginning, must save the original desired velocity.
      //curState->initialVelocity( curState->parameters()->velocity() );
   }

   if( time < _launchBlendStartTime )
   {
      // Only advance launching.
      runPerform( deltaTime, _launching.ptr(), curState->launchingState(), curState );
      curState->parameters()->entity()->constrainLinearVelocity( true );
   }
   else
   if( time < _launchBlendEndTime )
   {
      // Blend launching with flying.
      float f = (float)((time - _launchBlendStartTime)/(_launchBlendEndTime - _launchBlendStartTime));
      runPerform(
         deltaTime,
         _launching.ptr(), curState->launchingState(),
         _flying.ptr(), curState->flyingState(),
         f, curState
      );
      curState->parameters()->entity()->constrainLinearVelocity( false );
      if( curState->time() < _launchBlendStartTime )
      {
         applyVelocity( curState );
      }
      //curState->velocity( curState->initialVelocity() );
   }
   else
   {
      // Update velocity.
      Vec3f velocity = curState->parameters()->direction();
      Vec3f dvel     = velocity - curState->velocity();
      float dvelLen  = dvel.length();
      float maxVel   = curState->parameters()->maxAcceleration()*(float)deltaTime;
      if( dvelLen > maxVel )
      {
         dvel    *= maxVel/dvelLen;
         velocity = curState->velocity()+dvel;
      }

      // Only advance flying.
      runPerform( deltaTime, _flying.ptr(), curState->flyingState(), curState );
      curState->parameters()->entity()->constrainLinearVelocity( false );

      //curState->velocity( curState->initialVelocity() );
      //curState->velocity( velocity );
      curState->speed( length(velocity) );

      if( !curState->parameters()->entity()->falling() )
      {
         // TODO: do the following based on receptor contacts.
         curState->canExit( true );
         time = 0.0;
         if( curState->parameters()->entity()->brain() )
         {
            curState->parameters()->entity()->stimulate( new LandStimulus() );
         }
      }
   }

   // Update the current time.
   curState->time( time );
}

//------------------------------------------------------------------------------
//!
void
JumpNode::applyVelocity( JumpState* state ) const
{
   AnimationParameter&  params = *state->parameters();
   SkeletalEntity*      entity = params.entity();
   Vec3f vel = state->initialVelocity() * params.jumpScale() + params.jumpConst();
   vel       = entity->orientation() * vel;
   Vec3f p   = entity->mass() * vel;
   entity->applyImpulse( p );
}

