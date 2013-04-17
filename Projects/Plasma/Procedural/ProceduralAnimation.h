/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_PROCEDURAL_ANIMATION_H
#define PLASMA_PROCEDURAL_ANIMATION_H

#include <Plasma/StdDefs.h>
#include <Plasma/Animation/SkeletalAnimation.h>

#include <Fusion/Resource/ResourceTask.h>

#include <Base/Util/RCP.h>
#include <Base/ADT/String.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS ProceduralAnimation
==============================================================================*/

class ProceduralAnimation:
   public ResourceTask
{
public:

   /*----- static methods -----*/

   static void initialize();

   /*----- methods -----*/

   ProceduralAnimation( 
      Resource<SkeletalAnimation>* res, 
      const String&                animPath, 
      const String&                skelId
   );
 
   inline SkeletalAnimation* animation() { return _anim.ptr(); }

protected:

   /*----- methods -----*/

   virtual void execute();

   /*----- data members -----*/

   RCP< Resource<SkeletalAnimation> > _res;
   RCP<SkeletalAnimation>             _anim;
   String                             _animPath;
   String                             _skelId;
};

NAMESPACE_END

#endif
