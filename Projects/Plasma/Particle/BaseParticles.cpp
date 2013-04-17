/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Particle/BaseParticles.h>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN



UNNAMESPACE_END

/*==============================================================================
  CLASS LineGenerator
==============================================================================*/

//------------------------------------------------------------------------------
//!
LineGenerator::LineGenerator():
   _rate( 20.0f ),
   _src( Vec3f(0.0f, 0.0f, 0.0f) ),
   _vel( Vec3f(8.0f, 0.0f, 0.0f) ),
   _residual( 0.0f )
{
}

//------------------------------------------------------------------------------
//!
ParticleData::DataType
LineGenerator::generates() const
{
   return ParticleData::SIZE | ParticleData::COLOR | ParticleData::LINEAR_VELOCITY;
}

//------------------------------------------------------------------------------
//!
bool
LineGenerator::generate( float delta, ParticleEntity& e, ParticleRNG& rng )
{
   if( done() )  return true;

   delta = CGM::min( delta, 1.0f/16.0f ); // Don't fall below 16fps.
   ParticleData& data = e.data();
   _residual += float(rng()) * delta * _rate;

   uint32_t n = CGM::min( data.available(), uint32_t(_residual) );
   //StdErr << "Creating " << n << " particles (res=" << _residual << " av=" << data.available() << ")." << nl;
   for( uint32_t i = 0; i < n; ++i )
   {
      // Generate a particle and animate its first frame.
      ParticleData::Iterator cur = data.addParticle();
      float t = float(rng()) * delta; // Another time, to get varied particle over the subline.
      float c = 0.1f + float(rng())*0.9f;
      cur.position( _src + _vel*t );
      cur.size( 150.0f );
      cur.color( Vec4f(c, 0.0f, 0.0f, c*0.125f) );
      cur.energy( CGConstf::infinity() );
      cur.linearVelocity( _vel );
   }
   _residual -= float(n);

   return false;
}


/*==============================================================================
  CLASS LineAnimator
==============================================================================*/
//------------------------------------------------------------------------------
//!
LineAnimator::LineAnimator():
   _dst( Vec3f(0.0f, 0.0f, 2.0f) ),
   _dtd( 1.0f/16.0f )
{
}

//------------------------------------------------------------------------------
//!
ParticleData::DataType
LineAnimator::animates() const
{
   return ParticleData::LINEAR_VELOCITY;
}

//------------------------------------------------------------------------------
//!
bool
LineAnimator::animate( float delta, ParticleEntity& e, ParticleRNG& /*rng*/ )
{
   delta = CGM::min( delta, 1.0f/16.0f ); // Don't fall below 16fps.
   ParticleData& data = e.data();
   for( auto cur = data.iter(); cur(); cur.next() )
   {
      Vec3f&  pos = cur.position();
      Vec3f&  vel = cur.linearVelocity();
      float  spd2 = vel.sqrLength();
      if( spd2 > 1e-6f )
      {
         // Only care if the particle moves.
         Vec3f dir = _dst - pos;
         float len = dir.length();
         float spd = CGM::sqrt( spd2 );
         float dist = len - (spd*delta); // Distance after the timestep.
         if( dist <= _dtd )
         {
            // Will be close enough to distanceToDie, or overshoot.
            data.removeParticle( cur );
            continue;
         }

         vel  = dir * (spd/len); // Rescale to honor original speed.
         pos += vel * delta;
      }
      else
      {
         // Non-moving particles will never reach the destination.
         StdErr << "Non-moving particles expired." << nl;
         data.removeParticle( cur );
      }
   }

   //StdErr << "Once animated:" << nl;
   //data.printInfo();
   //if( !data.empty() && false )
   //{
   //   ParticleData::Iterator it = data.iter();
   //   StdErr << "last"
   //          << " pos=" << it.position()
   //          << " size=" << it.size()
   //          << " col=" << it.color()
   //          << " nrg=" << it.energy()
   //          << " vel=" << it.linearVelocity()
   //          << nl;
   //}
   ////getchar();

   return false;
}
