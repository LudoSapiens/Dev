/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Particle/Spark.h>

#include <Base/ADT/StringMap.h>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
enum
{
   ATTRIB_INITIAL_COLORS,
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
   "initialColors",    ATTRIB_INITIAL_COLORS,
   "initialEnergy",    ATTRIB_INITIAL_ENERGY,
   "initialPosition",  ATTRIB_INITIAL_POSITION,
   "initialSize",      ATTRIB_INITIAL_SIZE,
   "initialSpeed",     ATTRIB_INITIAL_SPEED,
   "rate",             ATTRIB_RATE,
   ""
);

UNNAMESPACE_END

/*==============================================================================
  CLASS SparkGenerator
==============================================================================*/
//------------------------------------------------------------------------------
//!
SparkGenerator::SparkGenerator( float energy ):
   ParticleGenerator( energy ),
   _position( 0.0f, 0.0f, 0.0f ),
   _distRate( 600.0f, 200.0f ),
   //_distFrequency( 1.0f, 0.0f ),
   _distEnergy( 0.5f, 0.2f ),
   _distSize( 0.1f, 0.0f ),
   _distSpeed( 10.0f, 0.5f ),
   _distDirection(),
   //_distColors( Vec3f(2.0f, 2.0f, 1.6f), Vec3f(1.0f, 1.0f, 1.0f) )
   _distColors( Vec3f(1.0f, 1.0f, 1.0f), Vec3f(1.0f, 1.0f, 1.0f) )
{
}

//------------------------------------------------------------------------------
//!
SparkGenerator::~SparkGenerator()
{
}

//------------------------------------------------------------------------------
//!
ParticleData::DataType
SparkGenerator::generates() const
{
   return ParticleData::SIZE | ParticleData::COLOR | ParticleData::LINEAR_VELOCITY;
}

//------------------------------------------------------------------------------
//!
bool
SparkGenerator::generate( float delta, ParticleEntity& e, ParticleRNG& rng )
{
   if( done() )  return true;

   ParticleData& data = e.data();

   Vec3f pos = _position;
   if( !e.isRelative() )  pos += e.position();
   float numSparks = _distRate( rng ) * delta;
   while( numSparks > 0.0f )
   {
      // Generate a particle.
      if( generateParticleStream( pos, data, rng ) )
      {
         --numSparks;
      }
      else
      {
         break;
      }
   }

   useEnergy( delta );

   //StdErr << "Once generated:" << nl;
   //data.print();
   //if( !data.empty() )
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

//------------------------------------------------------------------------------
//!
bool
SparkGenerator::generateParticleStream( const Vec3f& pos, ParticleData& data, ParticleRNG& rng )
{
   uint32_t num = CGM::min( data.available(), rng.getUInt( 8 ) + 9 );

   if( num > 0 )
   {
      float energy = _distEnergy( rng );
      float size   = _distSize( rng );
      Vec4f color  = Vec4f( _distColors( rng ), 1.0f );
      Vec3f dir    = _distDirection( rng );
      float speed  =_distSpeed( rng );
      Vec3f vel    = dir * speed;

#if 1
      energy = 1.0f;
      size   = 1.0f;
      color  = Vec4f( 1.0f, 0.0f, 0.0f, 1.0f );
      dir    = Vec3f( 0.0f, 1.0f, 0.0f );
      speed  = 6.0f;
      vel    = dir * speed;
#endif

      for( uint32_t i = 0; i < num; ++i )
      {
         float f = 0.5f + float(i)*0.5f/num;
         ParticleData::Iterator cur = data.addParticle();
         cur.position( pos + dir*(float(i)*0.04f) );
         cur.size( size*i );
         cur.color( color*f );
         cur.energy( energy );
         cur.linearVelocity( vel );
         vel.y -= (9.8f*0.04f/speed);  // Vary next particle in the loop.
      }

      return true;
   }
   else
   {
      return false;
   }


}

//------------------------------------------------------------------------------
//!
bool
SparkGenerator::performGet( VMState* vm )
{
   if( ParticleGenerator::performGet(vm) )
   {
      return true;
   }

   switch( _attributes[VM::toCString(vm, -1)] )
   {
      case ATTRIB_INITIAL_COLORS:
         VM::newTable( vm );
         VM::push( vm, _distColors.endpointA() );
         VM::seti( vm, -2, 1 );
         VM::push( vm, _distColors.endpointB() );
         VM::seti( vm, -2, 2 );
         return true;
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
SparkGenerator::performSet( VMState* vm )
{
   if( ParticleGenerator::performSet(vm) )
   {
      return true;
   }

   Vec2f v2;
   switch( _attributes[VM::toCString(vm, -2)] )
   {
      case ATTRIB_INITIAL_COLORS:
         VM::geti( vm, -1, 1 );
         VM::geti( vm, -1, 2 );
         initialColors( VM::toVec3f(vm, -2), VM::toVec3f(vm, -1) );
         VM::pop( vm, 2 );
         return true;
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
  CLASS SparkAnimator
==============================================================================*/
//------------------------------------------------------------------------------
//!
SparkAnimator::SparkAnimator():
   _damping( 1.0f )
{
}

//------------------------------------------------------------------------------
//!
SparkAnimator::~SparkAnimator()
{
}

//------------------------------------------------------------------------------
//!
ParticleData::DataType
SparkAnimator::animates() const
{
   return ParticleData::LINEAR_VELOCITY;
}

//------------------------------------------------------------------------------
//!
bool
SparkAnimator::animate( float delta, ParticleEntity& e, ParticleRNG& /*rng*/ )
{
   // Update energies and expire dead particles.
   ParticleData& data = e.data();
   for( auto cur = data.iter(); cur(); cur.next() )
   {
      float& energy = cur.energy();
      energy -= delta;
      if( energy > 0.0f )
      {
         Vec3f& pos = cur.position();
         Vec3f& vel = cur.linearVelocity();
         pos += vel * delta;
         if( pos.y > 0.0f || true )
         {
            vel.y -= 9.8f*delta; // Drop faster and faster.
         }
         else
         {
            vel.y *= -0.4f;  // Fake bounce (FIXME: Use physics).
         }
      }
      else
      {
         data.removeParticle( cur );
      }
   }

   //StdErr << "Once animated:" << nl;
   //data.print();
   //if( !data.empty() )
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
   //getchar();

   return false;
}
