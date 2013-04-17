/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_BASE_PARTICLES_H
#define PLASMA_BASE_PARTICLES_H

#include <Plasma/StdDefs.h>

#include <Plasma/Particle/ParticleAnimator.h>
#include <Plasma/Particle/ParticleGenerator.h>

//#include <CGMath/Distributions.h>

NAMESPACE_BEGIN


/*==============================================================================
  CLASS LineGenerator
==============================================================================*/
class LineGenerator:
   public ParticleGenerator
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API LineGenerator();

   PLASMA_DLL_API virtual ParticleData::DataType  generates() const;

   PLASMA_DLL_API virtual bool  generate( float delta, ParticleEntity& e, ParticleRNG& rng );

   inline float  rate() const    { return _rate; }
   inline void   rate( float v ) { _rate = v;    }

   inline const Vec3f&  source() const           { return _src; }
   inline         void  source( const Vec3f& v ) { _src = v;    }

   inline const Vec3f&  velocity() const           { return _vel; }
   inline         void  velocity( const Vec3f& v ) { _vel = v;    }

protected:

   /*----- data members -----*/

   float  _rate;      //!< Number of particles to generate every second.
   Vec3f  _src;       //!< Source position emitting particles.
   Vec3f  _vel;       //!< Velocity of the generated particles.
   float  _residual;  //!< Unused residual energy from the previous generations.

private:
}; //class LineGenerator


/*==============================================================================
  CLASS LineAnimator
==============================================================================*/
class LineAnimator:
   public ParticleAnimator
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API LineAnimator();

   PLASMA_DLL_API virtual ParticleData::DataType  animates() const;

   PLASMA_DLL_API virtual bool  animate( float delta, ParticleEntity& e, ParticleRNG& rng );

   inline const Vec3f&  destination() const           { return _dst; }
   inline         void  destination( const Vec3f& v ) { _dst = v;    }

   inline float  distanceToDie() const    { return _dtd; }
   inline  void  distanceToDie( float v ) { _dtd = v; }

protected:

   /*----- data members -----*/

   Vec3f  _dst;   //!< Source position emitting particles.
   float  _dtd;   //!< Distance to die.

private:
}; //class LineAnimator


NAMESPACE_END

#endif //PLASMA_BASE_PARTICLES_H
