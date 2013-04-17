/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_RIGID_ENTITY_H
#define PLASMA_RIGID_ENTITY_H

#include <Plasma/StdDefs.h>
#include <Plasma/World/Entity.h>

#if MOTION_BULLET
#include <MotionBullet/World/RigidBody.h>
#include <MotionBullet/Collision/CollisionShape.h>
#else
#include <Motion/World/RigidBody.h>
#include <Motion/Collision/CollisionShape.h>
#endif

NAMESPACE_BEGIN

/*==============================================================================
  CLASS RigidEntity
==============================================================================*/

//!

class RigidEntity
   : public Entity
{

public:

   /*----- methods -----*/

   PLASMA_DLL_API RigidEntity( RigidBody::Type bodyType );
   PLASMA_DLL_API virtual ~RigidEntity();

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
   inline bool canSense( const RigidEntity& e ) const;

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

   inline void applyImpulse( const Vec3f& impulse, const Vec3f& relPos );
   inline void applyImpulse( const Vec3f& impulse );
   inline void applyTorqueImpulse( const Vec3f& impulse );

   inline void addForce( const Vec3f& );
   inline void addTorque( const Vec3f& );
   inline void addForce( const Vec3f& force, const Vec3f& relPos );
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

protected:

   /*----- friends -----*/

   friend class World;

   /*----- methods -----*/

   RigidEntity( Type type, RigidBody::Type bodyType );

   virtual void connect( World* w );
   virtual void disconnect();

private:

   /*----- data members -----*/
   RCP<RigidBody>  _body;

};

//------------------------------------------------------------------------------
//!
inline bool canCollide( const RigidEntity& a, const RigidEntity& b )
{
   return RigidBody::collide( *(a.body()), *(b.body()) );
}

//------------------------------------------------------------------------------
//!
inline RigidBody*
RigidEntity::body() const
{
   return _body.ptr();
}

//------------------------------------------------------------------------------
//!
inline void
RigidEntity::mass( float val )
{
   _body->mass( val );
}

//------------------------------------------------------------------------------
//!
inline void
RigidEntity::friction( float friction )
{
   _body->friction( friction );
}

//------------------------------------------------------------------------------
//!
inline void
RigidEntity::restitution( float restitution )
{
   _body->restitution( restitution );
}

//------------------------------------------------------------------------------
//!
inline float
RigidEntity::mass() const
{
   return _body->mass();
}

//------------------------------------------------------------------------------
//!
inline float
RigidEntity::friction() const
{
   return _body->friction();
}

//------------------------------------------------------------------------------
//!
inline float
RigidEntity::restitution() const
{
   return _body->restitution();
}

//------------------------------------------------------------------------------
//!
inline uint
RigidEntity::exists() const
{
   return _body->exists();
}

//------------------------------------------------------------------------------
//!
inline void
RigidEntity::exists( uint c )
{
   _body->exists(c);
}

//------------------------------------------------------------------------------
//!
inline uint
RigidEntity::senses() const
{
   return _body->senses();
}

//------------------------------------------------------------------------------
//!
inline void
RigidEntity::senses( uint c )
{
   _body->senses(c);
}

//------------------------------------------------------------------------------
//!
inline uint
RigidEntity::attractionCategories() const
{
   return _body->attractionCategories();
}

//------------------------------------------------------------------------------
//!
inline void
RigidEntity::attractionCategories( uint c )
{
   _body->attractionCategories(c);
}

//------------------------------------------------------------------------------
//!
inline bool
RigidEntity::canSense( const RigidEntity& e ) const
{
   return _body->canSense( *(e._body) );
}

//------------------------------------------------------------------------------
//!
inline void
RigidEntity::linearVelocity( const Vec3f& v )
{
   _body->linearVelocity( v );
}

//------------------------------------------------------------------------------
//!
inline void
RigidEntity::angularVelocity( const Vec3f& v )
{
   _body->angularVelocity( v );
}

//------------------------------------------------------------------------------
//!
inline const Vec3f&
RigidEntity::linearVelocity() const
{
   return _body->linearVelocity();
}

//------------------------------------------------------------------------------
//!
inline const Vec3f&
RigidEntity::angularVelocity() const
{
   return _body->angularVelocity();
}

//------------------------------------------------------------------------------
//!
inline void
RigidEntity::applyImpulse( const Vec3f& impulse, const Vec3f& relPos )
{
   _body->applyImpulse( impulse, relPos );
}

//------------------------------------------------------------------------------
//!
inline void
RigidEntity::applyImpulse( const Vec3f& impulse )
{
   _body->applyImpulse( impulse );
}

//------------------------------------------------------------------------------
//!
inline void
RigidEntity::applyTorqueImpulse( const Vec3f& impulse )
{
   _body->applyTorqueImpulse( impulse );
}

//------------------------------------------------------------------------------
//!
void
RigidEntity::addForce( const Vec3f& force )
{
   _body->addForce( force );
}

//------------------------------------------------------------------------------
//!
void
RigidEntity::addTorque( const Vec3f& force )
{
   _body->addTorque( force );
}

//------------------------------------------------------------------------------
//!
void
RigidEntity::addForce( const Vec3f& force, const Vec3f& relPos )
{
   _body->addForce( force, relPos );
}

//------------------------------------------------------------------------------
//!
inline const Vec3f&
RigidEntity::totalForce() const
{
   return _body->totalForce();
}

//------------------------------------------------------------------------------
//!
inline const Vec3f&
RigidEntity::totalTorque() const
{
   return _body->totalTorque();
}

//------------------------------------------------------------------------------
//!
inline RigidBody::Type
RigidEntity::bodyType() const
{
   return _body->type();
}

//------------------------------------------------------------------------------
//!
inline bool
RigidEntity::isStatic() const
{
   return _body->type() == RigidBody::STATIC;
}

//------------------------------------------------------------------------------
//!
inline bool
RigidEntity::isDynamic() const
{
   return _body->type() == RigidBody::DYNAMIC;
}

//------------------------------------------------------------------------------
//!
inline bool
RigidEntity::isKinematic() const
{
   return _body->type() == RigidBody::KINEMATIC;
}

//------------------------------------------------------------------------------
//!
inline void
RigidEntity::makeStatic()
{
   _body->type( RigidBody::STATIC );
}

//------------------------------------------------------------------------------
//!
inline void
RigidEntity::makeDynamic()
{
   _body->type( RigidBody::DYNAMIC );
}

//------------------------------------------------------------------------------
//!
inline void
RigidEntity::makeKinematic()
{
   _body->type( RigidBody::KINEMATIC );
}

NAMESPACE_END

#endif  //PLASMA_RIGID_ENTITY_H
