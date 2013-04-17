/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_TRAVELNODE_H
#define PLASMA_TRAVELNODE_H

#include <Plasma/StdDefs.h>
#include <Plasma/Animation/AnimationNode.h>


NAMESPACE_BEGIN

class TravelState;

/*==============================================================================
   CLASS TravelNode
==============================================================================*/

class TravelNode :
   public AnimationNode
{

public:

   /*----- methods -----*/

   TravelNode();

   virtual RCP<AnimationNode> clone() const;
   virtual RCP<AnimationState> createState( AnimationParameter* p ) const;
   virtual void perform( AnimationState*, double deltaTime );

   virtual void getAnimations( Vector< SkeletalAnimation* >& anims );
   virtual void replaceAnimations(
      const Map< SkeletalAnimation*, RCP<SkeletalAnimation> >& anims
   );

   void addAnimation( SkeletalAnimation* anim );

protected:

   /*----- methods -----*/

   virtual ~TravelNode();

private:

   /*----- data members -----*/

   Vector< RCP<SkeletalAnimation> > _anims;
   Vector<float>                    _speeds;
   Vector<float>                    _distances;
};

/*==============================================================================
   CLASS TravelState
==============================================================================*/

class TravelState :
   public AnimationState
{

public:

   /*----- methods -----*/

   virtual void reset()                    { _hasArrived = false; }

   // Attributes getters.
   inline double time() const              { return _time; }
   inline bool hasArrived() const          { return _hasArrived; }
   inline float orientationAngle() const   { return _angle; }

   // Attributes setters.
   inline void time( double v )            { _time = v; }
   inline void hasArrived( bool v )        { _hasArrived = v; }
   inline void orientationAngle( float v ) { _angle = v; }

protected:

   /*----- methods -----*/

   double _time;
   bool   _hasArrived;
   float  _angle;
};


NAMESPACE_END

#endif
