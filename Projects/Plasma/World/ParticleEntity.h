/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_PARTICLE_ENTITY_H
#define PLASMA_PARTICLE_ENTITY_H

#include <Plasma/StdDefs.h>

#include <Plasma/Particle/ParticleData.h>
#include <Plasma/World/Entity.h>

#include <Base/Dbg/Defs.h>
#include <Base/Util/Enum.h>

NAMESPACE_BEGIN

class ParticleAnimator;
class ParticleGenerator;

/*==============================================================================
  CLASS ParticleEntity
==============================================================================*/
class ParticleEntity:
   public Entity
{
public:

   /*----- types -----*/

   enum State
   {
      NOTHING         = 0x00,
      IS_RELATIVE     = 0x01,
      IS_SPRITE       = 0x02,
      NEEDS_SORTING   = 0x04,
      NEEDS_PHYSICS   = 0x08,
   };

   /*----- methods -----*/

   PLASMA_DLL_API ParticleEntity();
   PLASMA_DLL_API ~ParticleEntity();

   inline const RCP<ParticleGenerator>&  generator() const { return _generator; }
   PLASMA_DLL_API void  generator( ParticleGenerator* g );

   inline const RCP<ParticleAnimator>&  animator() const { return _animator; }
   PLASMA_DLL_API void  animator( ParticleAnimator* a );

   inline       ParticleData&  data()       { return _data; }
   inline const ParticleData&  data() const { return _data; }

   inline State  state() const    { return _state; }
   inline void   state( State s ) { _state = s;    }

   inline bool   isRelative() const;
   inline void   isRelative( bool v );

   inline bool   isSprite() const;
   inline void   isSprite( bool v );

   inline bool   needsSorting() const;
   inline void   needsSorting( bool v );

   inline bool   needsPhysics() const;
   inline void   needsPhysics( bool v );

   PLASMA_DLL_API void  simulate( float delta, ParticleRNG& rng );

   PLASMA_DLL_API void  init( uint32_t capacity );

   ParticleRNG&  rng() { return _rng; }

protected:

   /*----- data members -----*/

   RCP<ParticleGenerator>  _generator;
   RCP<ParticleAnimator>   _animator;
   ParticleData            _data;
   State                   _state;     //!< A bitvector of various state values:
                                       //!< _state[0]   Is sprite.
                                       //!< _state[1]   Is relative.
                                       //!< _state[2]   Needs sorting.
                                       //!< _state[3]   Needs physics.
                                       //!< _state[4]   Requires particle colors buffer.
                                       //!< _state[5]   Requires particle sizes buffer.
                                       //!< _state[6]   Requires particle orientations buffer.

   ParticleRNG  _rng;  // TEMP: in an action context instead.
   /*----- methods -----*/

private:
}; //class ParticleEntity

GEN_ENUM_BITWISE_OPS( ParticleEntity::State )

//------------------------------------------------------------------------------
//!
inline bool
ParticleEntity::isRelative() const
{
   return (_state & IS_RELATIVE) != 0x0;
}

//------------------------------------------------------------------------------
//!
inline void
ParticleEntity::isRelative( bool v )
{
   _state &= ~IS_RELATIVE;
   _state |= (v ? IS_RELATIVE : NOTHING);
}

//------------------------------------------------------------------------------
//!
inline bool
ParticleEntity::isSprite() const
{
   return (_state & IS_SPRITE) != 0x0;
}

//------------------------------------------------------------------------------
//!
inline void
ParticleEntity::isSprite( bool v )
{
   _state &= ~IS_SPRITE;
   _state |= (v ? IS_SPRITE : NOTHING);
}

//------------------------------------------------------------------------------
//!
inline bool
ParticleEntity::needsSorting() const
{
   return (_state & NEEDS_SORTING) != 0x0;
}

//------------------------------------------------------------------------------
//!
inline void
ParticleEntity::needsSorting( bool v )
{
   _state &= ~NEEDS_SORTING;
   _state |= (v ? NEEDS_SORTING : NOTHING);
}

//------------------------------------------------------------------------------
//!
inline bool
ParticleEntity::needsPhysics() const
{
   return (_state & NEEDS_PHYSICS) != 0x0;
}

//------------------------------------------------------------------------------
//!
inline void
ParticleEntity::needsPhysics( bool v )
{
   _state &= ~NEEDS_PHYSICS;
   _state |= (v ? NEEDS_PHYSICS : NOTHING);
}


NAMESPACE_END

#endif //PLASMA_PARTICLE_ENTITY_H
