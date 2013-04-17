/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_SPARK_H
#define PLASMA_SPARK_H

#include <Plasma/StdDefs.h>

#include <Plasma/Particle/ParticleAnimator.h>
#include <Plasma/Particle/ParticleGenerator.h>

#include <CGMath/Distributions.h>

NAMESPACE_BEGIN


/*==============================================================================
  CLASS SparkGenerator
==============================================================================*/
class SparkGenerator:
   public ParticleGenerator
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API SparkGenerator( float energy = CGConstf::infinity() );
   PLASMA_DLL_API virtual ~SparkGenerator();

   PLASMA_DLL_API virtual ParticleData::DataType  generates() const;

   PLASMA_DLL_API virtual bool  generate( float delta, ParticleEntity& e, ParticleRNG& rng );

   inline void rate( float avg, float var ) { _distRate.set(avg, var); }

   inline void initialColor( const Vec3f& c0 ) { _distColors.set(c0, c0); }
   inline void initialColors( const Vec3f& c0, const Vec3f& c1 ) { _distColors.set(c0, c1); }
   inline void initialPosition( const Vec3f& v ) { _position = v; }
   inline void initialEnergy( float avg, float var ) { _distEnergy.set(avg, var); }
   inline void initialSize( float avg, float var ) { _distEnergy.set(avg, var); }
   inline void initialSpeed( float avg, float var ) { _distSpeed.set(avg, var); }

   inline const Vec3f& initialPosition() const { return _position; }

   inline       UniformSegment<float>&  colorDist()       { return _distColors; }
   inline const UniformSegment<float>&  colorDist() const { return _distColors; }

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
   UniformSegment<float> _distColors;    //!< The color of the spark (between 2).

   /*----- methods -----*/
   bool  generateParticleStream( const Vec3f& pos, ParticleData& data, ParticleRNG& rng );

}; //class SparkGenerator


/*==============================================================================
  CLASS SparkAnimator
==============================================================================*/
class SparkAnimator:
   public ParticleAnimator
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API SparkAnimator();
   PLASMA_DLL_API virtual ~SparkAnimator();

   PLASMA_DLL_API virtual ParticleData::DataType  animates() const;

   PLASMA_DLL_API virtual bool animate( float delta, ParticleEntity& e, ParticleRNG& rng );

protected:
   float  _damping;  //!< A damping factor.
}; //class SparkAnimator


NAMESPACE_END

#endif //PLASMA_SPARK_H
