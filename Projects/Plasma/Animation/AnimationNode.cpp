/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Animation/AnimationNode.h>

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

UNNAMESPACE_END


NAMESPACE_BEGIN

/*==============================================================================
   CLASS AnimationNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
AnimationNode::AnimationNode()
{
}

//------------------------------------------------------------------------------
//!
AnimationNode::~AnimationNode()
{
}

//------------------------------------------------------------------------------
//!
void AnimationNode::getAnimations( Vector< SkeletalAnimation* >& )
{
}

//------------------------------------------------------------------------------
//!
void AnimationNode::replaceAnimations(
   const Map< SkeletalAnimation*, RCP<SkeletalAnimation> >&
)
{
}

/*==============================================================================
   CLASS AnimationState
==============================================================================*/

//------------------------------------------------------------------------------
//!
AnimationState::~AnimationState()
{
}

//------------------------------------------------------------------------------
//!
void AnimationState::reset()
{
}

NAMESPACE_END
