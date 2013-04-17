/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Particle/Smoke.h>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
enum
{
   ATTRIB_INITIAL_ENERGY,
   ATTRIB_INITIAL_POSITION,
   ATTRIB_INITIAL_SIZE,
   ATTRIB_INITIAL_SPEED,
   ATTRIB_RATE,
   NUM_ATTRIBS
};

//------------------------------------------------------------------------------
//!
StringMap _attributes(
   "initialEnergy",    ATTRIB_INITIAL_ENERGY,
   "initialPosition",  ATTRIB_INITIAL_POSITION,
   "initialSize",      ATTRIB_INITIAL_SIZE,
   "initialSpeed",     ATTRIB_INITIAL_SPEED,
   "rate",             ATTRIB_RATE,
   ""
);

UNNAMESPACE_END


//------------------------------------------------------------------------------
//!
SmokeGenerator::SmokeGenerator( float energy ):
   ParticleGenerator( energy ),
   _position( 0.0f, 2.0f, 0.0f ),
   _distRate( 20.0f, 1.0f ),
   _distEnergy( 10.0f, 2.0f ),
   _distSize( 0.0f, 0.0f ),
   _distSpeed( 0.5f, 0.05f ),
   _distDirection(),
   _potential( 0.0f )
{
}

//------------------------------------------------------------------------------
//!
SmokeGenerator::~SmokeGenerator()
{
}

//------------------------------------------------------------------------------
//!
ParticleData::DataType
SmokeGenerator::generates() const
{
   return ParticleData::SIZE | ParticleData::COLOR | ParticleData::LINEAR_VELOCITY;
}

//------------------------------------------------------------------------------
//!
bool
SmokeGenerator::generate( float delta, ParticleEntity& e, ParticleRNG& rng )
{
   if( done() )  return true;

   ParticleData& data = e.data();

   _potential += _distRate( rng ) * delta;
   uint32_t n = CGM::min( uint32_t(_potential), data.available() );
   for( uint32_t i = 0; i < n; ++i )
   {
      generateParticle( data, rng );
   }
   _potential -= float(n);

   useEnergy( delta );

   return false;
}

//------------------------------------------------------------------------------
//!
void
SmokeGenerator::generateParticle( ParticleData& data, ParticleRNG& rng )
{
   Vec3f dir    = _distDirection( rng );
   float speed  = _distSpeed( rng );
   Vec3f vel    = dir * speed;

   auto cur = data.addParticle();
   cur.position( _position );
   cur.size( _distSize( rng ) );
   cur.color( Vec4f(1.0f) );
   cur.energy( _distEnergy( rng ) );
   cur.linearVelocity( vel );
}

//------------------------------------------------------------------------------
//!
bool
SmokeGenerator::performGet( VMState* vm )
{
   if( ParticleGenerator::performGet(vm) )
   {
      return true;
   }

   switch( _attributes[VM::toCString(vm, -1)] )
   {
      case ATTRIB_INITIAL_ENERGY:
         VM::push( vm, Vec2f(_distEnergy.average(), _distEnergy.variance()) );
         return true;
      case ATTRIB_INITIAL_POSITION:
         VM::push( vm, initialPosition() );
         return true;
      case ATTRIB_INITIAL_SIZE:
         VM::push( vm, Vec2f(_distSize.average(), _distSize.variance()) );
         return true;
      case ATTRIB_INITIAL_SPEED:
         VM::push( vm, Vec2f(_distSpeed.average(), _distSpeed.variance()) );
         return true;
      case ATTRIB_RATE:
         VM::push( vm, Vec2f(_distRate.average(), _distRate.variance()) );
         return true;
      default:
         break;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
bool
SmokeGenerator::performSet( VMState* vm )
{
   if( ParticleGenerator::performSet(vm) )
   {
      return true;
   }

   Vec2f v2;
   switch( _attributes[VM::toCString(vm, -2)] )
   {
      case ATTRIB_INITIAL_ENERGY:
         v2 = VM::toVec2f( vm, -1 );
         initialEnergy( v2.x, v2.y );
         return true;
      case ATTRIB_INITIAL_POSITION:
         initialPosition( VM::toVec3f( vm, -1 ) );
         return true;
      case ATTRIB_INITIAL_SIZE:
         v2 = VM::toVec2f( vm, -1 );
         initialSize( v2.x, v2.y );
         return true;
      case ATTRIB_INITIAL_SPEED:
         v2 = VM::toVec2f( vm, -1 );
         initialSpeed( v2.x, v2.y );
         return true;
      case ATTRIB_RATE:
         v2 = VM::toVec2f( vm, -1 );
         rate( v2.x, v2.y );
         return true;
      default:
         break;
   }
   return false;
}


/*==============================================================================
  CLASS SmokeAnimator
==============================================================================*/
//------------------------------------------------------------------------------
//!
SmokeAnimator::SmokeAnimator():
   _growingRate( 0.5f )
{
}

//------------------------------------------------------------------------------
//!
SmokeAnimator::~SmokeAnimator()
{
}

//------------------------------------------------------------------------------
//!
ParticleData::DataType
SmokeAnimator::animates() const
{
   return ParticleData::SIZE | ParticleData::COLOR | ParticleData::LINEAR_VELOCITY;
}

//------------------------------------------------------------------------------
//!
bool
SmokeAnimator::animate( float delta, ParticleEntity& e, ParticleRNG& )
{
   // Update energies and expire dead particles.
   ParticleData& data = e.data();
   for( auto cur = data.iter(); cur(); cur.next() )
   {
      float& energy = cur.energy();
      energy -= delta;
      if( energy > 0.0f )
      {
         Vec3f& pos  = cur.position();
         Vec3f& vel  = cur.linearVelocity();
         float& size = cur.size();

         // Update particle.
         pos   += vel * delta;
         size  += _growingRate * delta;
         vel.y -= delta * 0.04f;
         if( energy < 1.0f )
         {
            cur.color( Vec4f(energy) );  // Take 1 second to make the particle disappear.
         }
      }
      else
      {
         data.removeParticle( cur );
      }

   }
   return false;
}
