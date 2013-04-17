/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_PARTICLE_ANIMATOR_H
#define PLASMA_PARTICLE_ANIMATOR_H

#include <Plasma/StdDefs.h>

#include <Plasma/World/ParticleEntity.h>

#include <Fusion/VM/VM.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS ParticleAnimator
==============================================================================*/
class ParticleAnimator:
   public RCObject
{
public:

   /*----- methods -----*/

   inline ParticleAnimator() {}
   inline virtual ~ParticleAnimator() {}

   PLASMA_DLL_API virtual ParticleData::DataType  animates() const = 0;

   // Cleans up outdated particles, and animates the remaining ones.
   PLASMA_DLL_API virtual bool animate( float delta, ParticleEntity& e, ParticleRNG& rng ) = 0;

   // VM.
   PLASMA_DLL_API virtual void init( VMState* vm );
   PLASMA_DLL_API virtual bool performGet( VMState* vm );
   PLASMA_DLL_API virtual bool performSet( VMState* vm );

}; //class ParticleAnimator


NAMESPACE_END

#endif //PLASMA_PARTICLE_ANIMATOR_H
