/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Animation/BasicNode.h>

#include <CGMath/Random.h>

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

RNG_WELL _rng;

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   CLASS BasicNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
BasicNode::BasicNode()
{
}

//------------------------------------------------------------------------------
//!
BasicNode::~BasicNode()
{
}

//------------------------------------------------------------------------------
//!
RCP<AnimationNode>
BasicNode::clone() const
{
   RCP<BasicNode> node( new BasicNode() );
   node->_name  = _name;
   node->_anims = _anims;
   return node;
}

//------------------------------------------------------------------------------
//!
void BasicNode::getAnimations( Vector< SkeletalAnimation* >& anims )
{
   for( uint i = 0; i < _anims.size(); ++i )
   {
      anims.pushBack( _anims[i].ptr() );
   }
}

//------------------------------------------------------------------------------
//!
void BasicNode::replaceAnimations(
   const Map< SkeletalAnimation*, RCP<SkeletalAnimation> >& anims
)
{
   for( uint i = 0; i < _anims.size(); ++i )
   {
      auto it = anims.find( _anims[i].ptr() );
      if( it != anims.end() ) _anims[i] = (*it).second;
   }
}

//------------------------------------------------------------------------------
//!
RCP<AnimationState>
BasicNode::createState( AnimationParameter* p ) const
{
   RCP<BasicState> state = new BasicState();

   state->parameters( p );
   state->canExit( true );
   state->pose( new SkeletalPose( Reff::identity(), _anims[0]->pose(0)->numBones() ) );
   state->time(0);
   switchAnimation( state.ptr() );

   return state;
}

//------------------------------------------------------------------------------
//!
void
BasicNode::perform( AnimationState* state, double deltaTime )
{
   BasicState* curState = (BasicState*)state;

   double time = curState->time() + deltaTime;
   SkeletalAnimation* anim = curState->currentAnimation();

   // Loop animation?
   if( time > anim->duration() )
   {
      time -= anim->duration();
      switchAnimation( curState );
      anim = curState->currentAnimation();
      time = CGM::fmod( time, (double)anim->duration() );
   }

   // Compute current pose.
   float t;
   SkeletalPose* p0;
   SkeletalPose* p1;
   SkeletalPose* cp = curState->pose();

   anim->getPoses( (float)time, p0, p1, t );
   CHECK( p0->numBones() == p1->numBones() );
   CHECK( p0->numBones() == cp->numBones() );
   cp->referential( p0->referential().slerp( p1->referential(), t ) );
   for( uint i = 0; i < cp->numBones(); ++i )
   {
      cp->bones()[i] = p0->bones()[i].slerp( p1->bones()[i], t );
   }

   curState->orientation( curState->parameters()->entity()->orientation() );

   // Update state time.
   curState->time( time );
}

//------------------------------------------------------------------------------
//!
void
BasicNode::switchAnimation( BasicState* state ) const
{
   state->currentAnimation( _anims[_rng.getUInt( uint(_anims.size()) )].ptr() );
   state->orientation( state->parameters()->entity()->orientation() );

   // FIXME: Maybe animation should contain speed and direction.
   Vec3f vel = state->currentAnimation()->velocity();
   state->speed( length(vel) );
   state->direction( state->speed() == 0.0f ? Vec3f(0.0f,0.0f,1.0f) : normalize(vel) );
}


NAMESPACE_END
