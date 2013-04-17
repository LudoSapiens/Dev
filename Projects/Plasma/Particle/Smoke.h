/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_SMOKE_H
#define PLASMA_SMOKE_H

#include <Plasma/StdDefs.h>

#include <Plasma/Particle/ParticleAnimator.h>
#include <Plasma/Particle/ParticleGenerator.h>

#include <CGMath/Distributions.h>

#include <Base/ADT/StringMap.h>

NAMESPACE_BEGIN


/*==============================================================================
  CLASS SmokeGenerator
==============================================================================*/
class SmokeGenerator:
   public ParticleGenerator
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API SmokeGenerator( float energy = CGConstf::infinity() );
   PLASMA_DLL_API virtual ~SmokeGenerator();

   PLASMA_DLL_API virtual ParticleData::DataType  generates() const;

   PLASMA_DLL_API virtual bool  generate( float delta, ParticleEntity& e, ParticleRNG& rng );

   inline void rate( float avg, float var ) { _distRate.set(avg, var); }

   inline void initialPosition( const Vec3f& v ) { _position = v; }
   inline void initialEnergy( float avg, float var ) { _distEnergy.set(avg, var); }
   inline void initialSize( float avg, float var ) { _distEnergy.set(avg, var); }
   inline void initialSpeed( float avg, float var ) { _distSpeed.set(avg, var); }

   inline const Vec3f& initialPosition() const { return _position; }

   inline       UniformAvgVar<float>&  energyDist()       { return _distEnergy; }
   inline const UniformAvgVar<float>&  energyDist() const { return _distEnergy; }

   inline       UniformAvgVar<float>&  rateDist()       { return _distRate; }
   inline const UniformAvgVar<float>&  rateDist() const { return _distRate; }

   inline       UniformAvgVar<float>&  sizeDist()       { return _distSize; }
   inline const UniformAvgVar<float>&  sizeDist() const { return _distSize; }

   inline       UniformAvgVar<float>&  speedDist()       { return _distSpeed; }
   inline const UniformAvgVar<float>&  speedDist() const { return _distSpeed; }

   // VM.
   PLASMA_DLL_API virtual bool performGet( VMState* vm );
   PLASMA_DLL_API virtual bool performSet( VMState* vm );

protected:
   /*----- members -----*/
   Vec3f                 _position;      //!< The position of the particle.
   UniformAvgVar<float>  _distRate;      //!< The rate at which particles are generated (in particles/second).
   UniformAvgVar<float>  _distEnergy;    //!< The energy of the particles.
   UniformAvgVar<float>  _distSize;      //!< The size of the particles.
   UniformAvgVar<float>  _distSpeed;     //!< The speed at which the spark flies.
   UniformSphere<float>  _distDirection; //!< The direction of the spark.
   float                 _potential;

   /*----- methods -----*/
   void  generateParticle( ParticleData& data, ParticleRNG& rng );

}; //class SmokeGenerator


/*==============================================================================
  CLASS SmokeAnimator
==============================================================================*/
class SmokeAnimator:
   public ParticleAnimator
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API SmokeAnimator();
   PLASMA_DLL_API virtual ~SmokeAnimator();

   PLASMA_DLL_API virtual ParticleData::DataType  animates() const;

   PLASMA_DLL_API virtual bool animate( float delta, ParticleEntity& e, ParticleRNG& rng );

protected:
   float  _growingRate;  //!< The rate at which particles grow in size.
}; //class SmokeAnimator


NAMESPACE_END

#endif //PLASMA_SMOKE_H
