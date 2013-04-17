/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_BASICNODE_H
#define PLASMA_BASICNODE_H

#include <Plasma/StdDefs.h>
#include <Plasma/Animation/AnimationNode.h>

NAMESPACE_BEGIN

class BasicState;
   
/*==============================================================================
   CLASS BasicNode
==============================================================================*/

class BasicNode :
   public AnimationNode
{

public: 

   /*----- methods -----*/

   BasicNode();

   virtual RCP<AnimationNode> clone() const;
   virtual RCP<AnimationState> createState( AnimationParameter* p ) const;
   virtual void perform( AnimationState*, double deltaTime );

   virtual void getAnimations( Vector< SkeletalAnimation* >& anims );
   virtual void replaceAnimations( 
      const Map< SkeletalAnimation*, RCP<SkeletalAnimation> >& anims 
   );

   inline void addAnimation( SkeletalAnimation* anim ) { _anims.pushBack( anim ); }

protected: 

   /*----- methods -----*/

   virtual ~BasicNode();
   
   void switchAnimation( BasicState* ) const;

private:

   /*----- data members -----*/

   Vector< RCP<SkeletalAnimation> > _anims;
};

/*==============================================================================
   CLASS BasicState
==============================================================================*/

class BasicState :
      public AnimationState
{

public:

   /*----- methods -----*/

   // Attributes getters.
   inline SkeletalAnimation* currentAnimation() const   { return _currentAnim; }
   inline double time() const                           { return _time; }
   
   // Attributes setters.
   inline void currentAnimation( SkeletalAnimation* v ) { _currentAnim = v; }
   inline void time( double v )                         { _time = v; }

protected: 

   /*----- methods -----*/

   SkeletalAnimation* _currentAnim;
   double             _time;
};


NAMESPACE_END

#endif
