/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_PARTICLE_GENERATOR_H
#define PLASMA_PARTICLE_GENERATOR_H

#include <Plasma/StdDefs.h>

#include <Plasma/World/ParticleEntity.h>

#include <Fusion/VM/VM.h>

#include <Base/Util/RCObject.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS ParticleGenerator
==============================================================================*/
class ParticleGenerator:
   public RCObject
{
public:

   /*----- methods -----*/

   inline ParticleGenerator( const float energy = CGConstf::infinity() ): _energy(energy) {}
   inline virtual ~ParticleGenerator() {}

   PLASMA_DLL_API virtual ParticleData::DataType  generates() const = 0;

   // Performs the actual particle generation for a delta time.
   PLASMA_DLL_API virtual bool  generate( float delta, ParticleEntity& e, ParticleRNG& rng ) = 0;

   inline   void  energy( float e )    { _energy = e;    }
   inline  float  energy() const       { return _energy; }
   inline   void  useEnergy( float e ) { _energy -= e;   }

   inline   bool  done() const         { return _energy <= 0.0f; }

   // VM.
   PLASMA_DLL_API void init( VMState* vm );
   PLASMA_DLL_API virtual bool performGet( VMState* vm );
   PLASMA_DLL_API virtual bool performSet( VMState* vm );

protected:
   float   _energy;    //!< The life energy left in the generator (<=0 means one-timer generator).

}; //class ParticleGenerator

NAMESPACE_END

#endif //PLASMA_PARTICLE_GENERATOR_H
