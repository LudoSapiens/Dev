/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_SKELETALENTITY_H
#define PLASMA_SKELETALENTITY_H

#include <Plasma/StdDefs.h>
#include <Plasma/World/Entity.h>
#include <Plasma/Animation/Skeleton.h>

#if MOTION_BULLET
#include <MotionBullet/Collision/CollisionShape.h>
#include <MotionBullet/Constraint/CharacterConstraint.h>
#include <MotionBullet/World/RigidBody.h>
#else
#include <Motion/Collision/CollisionShape.h>
#include <Motion/Constraint/CharacterConstraint.h>
#include <Motion/World/RigidBody.h>
#endif

NAMESPACE_BEGIN


/*==============================================================================
  CLASS SkeletalEntity
==============================================================================*/

//!

class SkeletalEntity
   : public Entity
{

public:

   /*----- methods -----*/

   PLASMA_DLL_API SkeletalEntity();
   PLASMA_DLL_API virtual ~SkeletalEntity();

   // Physical attributes.
   inline RigidBody* body() const;

   inline void mass( float );
   inline void friction( float );
   inline void restitution( float );

   inline float mass() const;
   inline float friction() const;
   inline float restitution() const;

   // Collision.
   inline uint exists() const;
   inline void exists( uint m );
   inline uint senses() const;
   inline void senses( uint m );
   inline uint attractionCategories() const;
   inline void attractionCategories( uint c );

   // Position and orientation.
   PLASMA_DLL_API virtual void referential( const Reff& ref );
   PLASMA_DLL_API virtual void position( const Vec3f& pos );
   PLASMA_DLL_API virtual void orientation( const Quatf& orient );
   PLASMA_DLL_API virtual void lookAt( const Vec3f& at, const Vec3f& up );
   PLASMA_DLL_API virtual void rotate( const Vec3f& pos, const Vec3f axis, float angle );

   using Entity::referential;
   using Entity::position;
   using Entity::orientation;
   using Entity::transform;

   // Geometry.
   PLASMA_DLL_API virtual void geometry( Geometry* geom );
   using Entity::geometry;

   // Velocities and forces.
   inline void linearVelocity( const Vec3f& v );
   inline void angularVelocity( const Vec3f& v );

   inline const Vec3f& linearVelocity() const;
   inline const Vec3f& angularVelocity() const;

   inline void constrainLinearVelocity( const bool v );
   inline bool constrainLinearVelocity() const;

   inline void applyImpulse( const Vec3f& impulse, const Vec3f& relPos );
   inline void applyImpulse( const Vec3f& impulse );
   inline void applyTorqueImpulse( const Vec3f& impulse );

   inline void addForce( const Vec3f& f );
   inline void addTorque( const Vec3f& t );
   inline void addForce( const Vec3f& f, const Vec3f& relPos );
   inline const Vec3f& totalForce() const;
   inline const Vec3f& totalTorque() const;

   // State.
   inline RigidBody::Type  bodyType() const;
   inline bool  isStatic() const;
   inline bool  isDynamic() const;
   inline bool  isKinematic() const;
   inline void  makeStatic();
   inline void  makeDynamic();
   inline void  makeKinematic();

   inline bool  falling() const;
   inline float fallDuration() const;

   // Skeleton.
   inline Skeleton* skeleton() const;

   inline const Skeleton::Instance& instance() const;
   inline Skeleton::Instance& instance();

   inline const Skeleton::MatrixContainer& transforms() const;
   inline const RCP<Gfx::ConstantBuffer>& boneConstants() const;

   inline uint setPose( SkeletalAnimation* anim, float time );
   inline void setPose( const Vector<Reff>& pose );

protected:

   /*----- friends -----*/

   friend class World;

   /*----- methods -----*/

   virtual void connect( World* w );
   virtual void disconnect();

private:


   /*----- data members -----*/

   RCP<RigidBody>            _body;
   Skeleton::Instance        _skeleton;
   RCP<CharacterConstraint>  _constraint;
};

//------------------------------------------------------------------------------
//!
inline RigidBody*
SkeletalEntity::body() const
{
   return _body.ptr();
}

//------------------------------------------------------------------------------
//!
inline void
SkeletalEntity::mass( float val )
{
   if( !isStatic() ) _body->mass( val );
}

//------------------------------------------------------------------------------
//!
inline void
SkeletalEntity::friction( float friction )
{
   _body->friction( friction );
}

//------------------------------------------------------------------------------
//!
inline void
SkeletalEntity::restitution( float restitution )
{
   _body->restitution( restitution );
}

//------------------------------------------------------------------------------
//!
inline float
SkeletalEntity::mass() const
{
   return _body->mass();
}

//------------------------------------------------------------------------------
//!
inline float
SkeletalEntity::friction() const
{
   return _body->friction();
}

//------------------------------------------------------------------------------
//!
inline float
SkeletalEntity::restitution() const
{
   return _body->restitution();
}

//------------------------------------------------------------------------------
//!
inline uint
SkeletalEntity::exists() const
{
   return _body->exists();
}

//------------------------------------------------------------------------------
//!
inline void
SkeletalEntity::exists( uint c )
{
   _body->exists(c);
}

//------------------------------------------------------------------------------
//!
inline uint
SkeletalEntity::senses() const
{
   return _body->senses();
}

//------------------------------------------------------------------------------
//!
inline void
SkeletalEntity::senses( uint c )
{
   _body->senses(c);
}

//------------------------------------------------------------------------------
//!
inline uint
SkeletalEntity::attractionCategories() const
{
   return _body->attractionCategories();
}

//------------------------------------------------------------------------------
//!
inline void
SkeletalEntity::attractionCategories( uint c )
{
   _body->attractionCategories(c);
}

//------------------------------------------------------------------------------
//!
inline void
SkeletalEntity::linearVelocity( const Vec3f& v )
{
   if( _constraint.isValid() )
   {
      _constraint->linearVelocity( v );
   }
   else
   {
      _body->linearVelocity( v );
   }
}

//------------------------------------------------------------------------------
//!
inline void
SkeletalEntity::angularVelocity( const Vec3f& v )
{
   _body->angularVelocity( v );
}

//------------------------------------------------------------------------------
//!
inline const Vec3f&
SkeletalEntity::linearVelocity() const
{
   return _body->linearVelocity();
}

//------------------------------------------------------------------------------
//!
inline const Vec3f&
SkeletalEntity::angularVelocity() const
{
   return _body->angularVelocity();
}

//------------------------------------------------------------------------------
//!
inline void
SkeletalEntity::applyImpulse( const Vec3f& impulse, const Vec3f& relPos )
{
   _body->applyImpulse( impulse, relPos );
}

//------------------------------------------------------------------------------
//!
inline void
SkeletalEntity::applyImpulse( const Vec3f& impulse )
{
   _body->applyImpulse( impulse );
}

//------------------------------------------------------------------------------
//!
inline void
SkeletalEntity::applyTorqueImpulse( const Vec3f& impulse )
{
   _body->applyTorqueImpulse( impulse );
}

//------------------------------------------------------------------------------
//!
void
SkeletalEntity::addForce( const Vec3f& f )
{
   _body->addForce( f );
}

//------------------------------------------------------------------------------
//!
void
SkeletalEntity::addTorque( const Vec3f& f )
{
   _body->addTorque( f );
}

//------------------------------------------------------------------------------
//!
void
SkeletalEntity::addForce( const Vec3f& f, const Vec3f& relPos )
{
   _body->addForce( f, relPos );
}

//------------------------------------------------------------------------------
//!
inline const Vec3f&
SkeletalEntity::totalForce() const
{
   return _body->totalForce();
}

//------------------------------------------------------------------------------
//!
inline const Vec3f&
SkeletalEntity::totalTorque() const
{
   return _body->totalTorque();
}

//------------------------------------------------------------------------------
//!
inline Skeleton*
SkeletalEntity::skeleton() const
{
   return _skeleton.skeleton();
}

//------------------------------------------------------------------------------
//!
inline const Skeleton::Instance&
SkeletalEntity::instance() const
{
   return _skeleton;
}

//------------------------------------------------------------------------------
//!
inline Skeleton::Instance&
SkeletalEntity::instance()
{
   return _skeleton;
}

//------------------------------------------------------------------------------
//!
inline const Skeleton::MatrixContainer&
SkeletalEntity::transforms() const
{
   return _skeleton.transforms();
}

//------------------------------------------------------------------------------
//!
inline const RCP<Gfx::ConstantBuffer>&
SkeletalEntity::boneConstants() const
{
   return _skeleton.constants();
}

//------------------------------------------------------------------------------
//!
inline uint
SkeletalEntity::setPose( SkeletalAnimation* anim, float time )
{
   return _skeleton.setPose( anim, time );
}

//------------------------------------------------------------------------------
//!
inline void
SkeletalEntity::setPose( const Vector<Reff>& pose )
{
   _skeleton.setPose( pose );
}

//------------------------------------------------------------------------------
//!
inline void
SkeletalEntity::constrainLinearVelocity( const bool v )
{
   CHECK( _constraint.isValid() );
   _constraint->constrainLinearVelocity( v );
}

//------------------------------------------------------------------------------
//!
inline bool
SkeletalEntity::constrainLinearVelocity() const
{
   CHECK( _constraint.isValid() );
   return _constraint->constrainLinearVelocity();
}

//------------------------------------------------------------------------------
//!
inline RigidBody::Type
SkeletalEntity::bodyType() const
{
   return _body->type();
}

//------------------------------------------------------------------------------
//!
inline bool
SkeletalEntity::isStatic() const
{
   return _body->type() == RigidBody::STATIC;
}

//------------------------------------------------------------------------------
//!
inline bool
SkeletalEntity::isDynamic() const
{
   return _body->type() == RigidBody::DYNAMIC;
}

//------------------------------------------------------------------------------
//!
inline bool
SkeletalEntity::isKinematic() const
{
   return _body->type() == RigidBody::KINEMATIC;
}

//------------------------------------------------------------------------------
//!
inline void
SkeletalEntity::makeStatic()
{
   _body->type( RigidBody::STATIC );
}

//------------------------------------------------------------------------------
//!
inline void
SkeletalEntity::makeDynamic()
{
   _body->type( RigidBody::DYNAMIC );
}

//------------------------------------------------------------------------------
//!
inline void
SkeletalEntity::makeKinematic()
{
   _body->type( RigidBody::KINEMATIC );
}

//------------------------------------------------------------------------------
//!
inline bool
SkeletalEntity::falling() const
{
   CHECK( _constraint.isValid() );
   return _constraint->falling();
}

//------------------------------------------------------------------------------
//!
inline float
SkeletalEntity::fallDuration() const
{
   CHECK( _constraint.isValid() );
   return _constraint->fallDuration();
}

NAMESPACE_END

#endif
