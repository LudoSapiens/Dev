/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef MOTION_RIGIDBODY_H
#define MOTION_RIGIDBODY_H

#include <Motion/StdDefs.h>
#include <Motion/Collision/CollisionGroup.h>

#include <CGMath/Ref.h>

#include <Base/Dbg/Defs.h>
#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>

NAMESPACE_BEGIN

class MotionWorld;

/*==============================================================================
   CLASS RigidBody
==============================================================================*/
//!
class RigidBody
   : public RCObject
{

public:

   /*----- types and enumerations ----*/

   enum Flags
   {
      RIGID_BODY_FLAGS_NONE   = 0x0000,
      RIGID_BODY_FLAGS_ACTIVE = 0x0001,
      RIGID_BODY_FLAGS_ALL    = ~RIGID_BODY_FLAGS_NONE
   };

   enum Type
   {
      DYNAMIC   = 0,
      STATIC    = 1,
      KINEMATIC = 2
   };

   /*----- static methods -----*/

   static inline bool  canCollide( const RigidBody& bodyA, const RigidBody& bodyB )
   {
      return (
              (bodyA.collisionCategories() & bodyB.collisionMask()) |
              (bodyB.collisionCategories() & bodyA.collisionMask())
             ) != 0;
   }

   static inline bool  triggersCallback( const RigidBody& bodyA, const RigidBody& bodyB )
   {
      return (
              (bodyA.collisionCategories() & bodyB.callbackMask()) |
              (bodyB.collisionCategories() & bodyA.callbackMask())
             ) != 0;
   }

   /*----- methods -----*/

   MOTION_DLL_API RigidBody( Type type, void* userData = NULL );
   MOTION_DLL_API virtual ~RigidBody();

   inline uint type() const;
   
   inline MotionWorld* world() const    { return _world; }

   inline void   userData( void* data ) { _userData = data; }
   inline void*  userData() const       { return _userData; }

   // Properties.
   inline void friction( float );
   inline void restitution( float );
   inline void mass( float );

   inline float friction() const;
   inline float restitution() const;
   inline float mass() const;

   inline float friction( const RigidBody& ) const;
   inline float restitution( const RigidBody& ) const;

   MOTION_DLL_API void referential( const Reff& );
   MOTION_DLL_API void position( const Vec3f& );
   MOTION_DLL_API void orientation( const Quatf& );
   MOTION_DLL_API void linearVelocity( const Vec3f& );
   MOTION_DLL_API void angularVelocity( const Vec3f& );

   inline const Reff& referential() const;
   inline const Vec3f& position() const;
   inline const Quatf& orientation() const;
   inline const Vec3f& linearVelocity() const;
   inline const Vec3f& angularVelocity() const;

   inline Vec3f velocity( const Vec3f& pos ) const;
   inline Vec3f velocityPrev( const Vec3f& pos ) const;
   inline Vec3f lookAhead( const Vec3f& pos, double step ) const;
   inline Quatf lookAhead( const Quatf& ori, double step ) const;

   // Activation.
   inline bool   active() const;
   inline void   activate();
   inline void   deactivate();

   // Category.
   inline uint  collisionCategories() const                     { return _collisionCats; }
   inline void  collisionCategories( uint c )                   { _collisionCats = c; }
   inline void  addCollisionCategories( uint c )                { _collisionCats |= c; }
   inline void  removeCollisionCategories( uint c )             { _collisionCats &= ~c; }

   // Collision mask.
   inline uint  collisionMask() const                           { return _collisionMask; }
   inline void  collisionMask( uint m )                         { _collisionMask = m; }
   inline void  acceptCollisionsWith( uint cat )                { _collisionMask |= cat; }
   inline void  ignoreCollisionsWith( uint cat )                { _collisionMask &= ~cat; }
   inline bool  canCollide( const RigidBody& body ) const       { return (body.collisionCategories() & collisionMask()) != 0; }

   // Callback mask.
   inline uint  callbackMask() const                            { return _callbackMask; }
   inline void  callbackMask( uint m )                          { _callbackMask = m; }
   inline void  reportCollisionsWith( uint cat )                { _callbackMask |= cat; }
   inline void  silenceCollisionsWith( uint cat )               { _callbackMask &= ~cat; }
   inline bool  triggersCallback( const RigidBody& body ) const { return (body.collisionCategories() & callbackMask()) != 0; }

   // Attraction category.
   inline uint  attractionCategories() const                    { return _attractionCats; }
   inline void  attractionCategories( uint c )                  { _attractionCats = c; }
   inline void  addAttractionCategories( uint c )               { _attractionCats |= c; }
   inline void  removeAttractionCategories( uint c )            { _attractionCats &= ~c; }

   // Forces.
   inline void addForce( const Vec3f& force, const Vec3f& relPos );
   inline void addForce( const Vec3f& );
   inline void addTorque( const Vec3f& );
   inline const Vec3f& totalForce() const;
   inline const Vec3f& totalTorque() const;

   // Impulse.
   inline void applyImpulse( const Vec3f& impulse, const Vec3f& pos );
   inline void applyImpulse( const Vec3f& impulse );
   inline void applyTorqueImpulse( const Vec3f& impulse );
   inline void applyImpulsePrev( const Vec3f& impulse, const Vec3f& pos );

   // Collision.
   inline void shape( CollisionShape* shape ) { _shape = shape; }
   inline CollisionShape* shape() const { return _shape.ptr(); }
   inline const AABBoxf& boundingBox() const;

   // Simulation.
   MOTION_DLL_API void applyForces( double step );
   MOTION_DLL_API void applyVelocities( double step );

   // Matrix K for resolving constraints.
   MOTION_DLL_API Mat3f computeK( const Vec3f& p ) const;

   // Matrix L for resolving constraints.
   MOTION_DLL_API Mat3f computeL() const;

   inline const Reff&  simReferential() const;
   inline const Vec3f& simPosition() const;
   inline const Quatf& simOrientation() const;
   inline       Mat4f  simTransform() const;

   // Use these carefully.
   MOTION_DLL_API void simReferential( const Reff& );
   MOTION_DLL_API void simPosition( const Vec3f& );
   MOTION_DLL_API void simOrientation( const Quatf& );

protected:

   /*----- friends -----*/

   friend class MotionWorld;

   /*----- methods -----*/

   inline void connect( MotionWorld* );
   inline void disconnect();

   inline void update( float );
   void updateRefDerivedData();

private:

   /*----- data members -----*/

   uint           _type;
   void*          _userData;
   MotionWorld*   _world;
   float          _friction;
   float          _restitution;
   float          _mass;
   float          _invMass;

   Reff           _interpolatedRef;
   Vec3f          _linearVelocity;
   Vec3f          _angularVelocity;

   Vec3f          _totalForce;
   Vec3f          _totalTorque;

   Mat3f          _inertiaTensor;
   Mat3f          _invInertiaTensor;
   Mat3f          _invWorldInertiaTensor;

   Mat3f          _prevInvWorldInertiaTensor;
   Vec3f          _prevPos;

   Reff                 _simRef;
   RCP<CollisionShape>  _shape;

   uint           _flags;
   uint           _collisionCats;  //!< A bitset indicating all of the collision categories this rigid body is a member of.  By default, the object is a member of category 0x01.
   uint           _collisionMask;  //!< A bitmask indicating the categories that can collide with the body.
   uint           _callbackMask;   //!< A bitmask indicating the categories which trigger a callback when collisions occur.
   uint           _attractionCats; //!< A bitset indicating the types of attractor which can affect this rigid body.  By default, the object is a member of all categories, e.g. ~0x0.
};

//------------------------------------------------------------------------------
//!
inline uint
RigidBody::type() const
{
   return _type;
}

//------------------------------------------------------------------------------
//!
inline void
RigidBody::friction( float friction )
{
   _friction = friction;
}

//------------------------------------------------------------------------------
//!
inline void
RigidBody::restitution( float restitution )
{
   _restitution = restitution;
}

//------------------------------------------------------------------------------
//!
inline void
RigidBody::mass( float mass )
{
   _mass = mass;
   _invMass = 1.0f / mass;

   // FIXME: Temp.
   _inertiaTensor = Mat3f::identity() * ( mass * 0.4f * 0.5f*0.5f );
   _invInertiaTensor = _inertiaTensor.getInversed();
   updateRefDerivedData();
}

//------------------------------------------------------------------------------
//!
inline float
RigidBody::friction() const
{
   return _friction;
}

//------------------------------------------------------------------------------
//!
inline float
RigidBody::restitution() const
{
   return _restitution;
}

//------------------------------------------------------------------------------
//!
inline float
RigidBody::mass() const
{
   return _mass;
}

//------------------------------------------------------------------------------
//!
inline float
RigidBody::friction( const RigidBody& body ) const
{
   return ( _friction + body._friction ) * 0.5f;
}

//------------------------------------------------------------------------------
//!
inline float
RigidBody::restitution( const RigidBody& body ) const
{
   return ( _restitution + body._restitution ) * 0.5f;
}

//------------------------------------------------------------------------------
//!
inline const Reff&
RigidBody::referential() const
{
   return _interpolatedRef;
}

//------------------------------------------------------------------------------
//!
inline const Vec3f&
RigidBody::position() const
{
   return _interpolatedRef.position();
}

//------------------------------------------------------------------------------
//!
inline const Quatf&
RigidBody::orientation() const
{
   return _interpolatedRef.orientation();
}

//------------------------------------------------------------------------------
//!
inline const Vec3f&
RigidBody::linearVelocity() const
{
   return _linearVelocity;
}

//------------------------------------------------------------------------------
//!
inline const Vec3f&
RigidBody::angularVelocity() const
{
   return _angularVelocity;
}

//------------------------------------------------------------------------------
//!
inline Vec3f
RigidBody::velocity( const Vec3f& pos ) const
{
   return _linearVelocity + _angularVelocity.cross( pos - _simRef.position() );
}

//------------------------------------------------------------------------------
//!
inline Vec3f
RigidBody::velocityPrev( const Vec3f& pos ) const
{
   return _linearVelocity + _angularVelocity.cross( pos - _prevPos );
}

//------------------------------------------------------------------------------
//!
inline Vec3f
RigidBody::lookAhead( const Vec3f& pos, double step ) const
{
   Vec3f c = _simRef.position() + _linearVelocity*(float)step;
   Vec3f r = pos - _simRef.position();

   float avLen = _angularVelocity.length();
   if( avLen > 0.0f )
   {
      Quatf drot = Quatf::axisAngle( _angularVelocity / avLen, avLen*(float)step );
      r = drot * r;
   }
   return r + c;
}

//------------------------------------------------------------------------------
//!
inline Quatf
RigidBody::lookAhead( const Quatf& ori, double step ) const
{
   float avLen = _angularVelocity.length();
   if( avLen > 0.0f )
   {
      Quatf drot = Quatf::axisAngle( _angularVelocity / avLen, avLen*(float)step );
      return ( drot * ori ).normalize();
   }
   return ori;
}

//------------------------------------------------------------------------------
//!
inline bool
RigidBody::active() const
{
   return (_flags & RIGID_BODY_FLAGS_ACTIVE) != 0;
}

//------------------------------------------------------------------------------
//!
inline void
RigidBody::activate()
{
   _flags |= RIGID_BODY_FLAGS_ACTIVE;
}

//------------------------------------------------------------------------------
//!
inline void
RigidBody::deactivate()
{
   _flags &= ~RIGID_BODY_FLAGS_ACTIVE;
}

//------------------------------------------------------------------------------
//!
inline void
RigidBody::addForce( const Vec3f& force, const Vec3f& relPos )
{
   addForce( force );
   addTorque( relPos.cross( force ) );
}

//------------------------------------------------------------------------------
//!
inline void
RigidBody::addForce( const Vec3f& force )
{
   _totalForce += force;
}

//------------------------------------------------------------------------------
//!
inline void
RigidBody::addTorque( const Vec3f& torque )
{
   _totalTorque += torque;
}

//------------------------------------------------------------------------------
//!
inline const Vec3f&
RigidBody::totalForce() const
{
   return _totalForce;
}

//------------------------------------------------------------------------------
//!
inline const Vec3f&
RigidBody::totalTorque() const
{
   return _totalTorque;
}

//------------------------------------------------------------------------------
//!
inline void
RigidBody::applyImpulse( const Vec3f& impulse, const Vec3f& pos )
{
   if( _type == DYNAMIC )
   {
      applyImpulse( impulse );
      applyTorqueImpulse( (pos - _simRef.position()).cross( impulse ) );
   }
}

//------------------------------------------------------------------------------
//!
inline void
RigidBody::applyImpulse( const Vec3f& impulse )
{
   _linearVelocity += impulse * _invMass;
}

//------------------------------------------------------------------------------
//!
inline void
RigidBody::applyTorqueImpulse( const Vec3f& impulse )
{
   _angularVelocity += _invWorldInertiaTensor * impulse;
}

//------------------------------------------------------------------------------
//!
inline void
RigidBody::applyImpulsePrev( const Vec3f& impulse, const Vec3f& pos )
{
   if( _type == DYNAMIC )
   {
      _linearVelocity  += impulse * _invMass;
      _angularVelocity += _prevInvWorldInertiaTensor * (pos - _prevPos).cross( impulse );
   }
}

//------------------------------------------------------------------------------
//!
inline void
RigidBody::connect( MotionWorld* w )
{
   _world = w;
}

//------------------------------------------------------------------------------
//!
inline void
RigidBody::disconnect()
{
   _world = NULL;
}

//------------------------------------------------------------------------------
//!
inline void
RigidBody::update( float t )
{
   _interpolatedRef = _interpolatedRef.slerp( _simRef, t );
}

//------------------------------------------------------------------------------
//!
inline const AABBoxf&
RigidBody::boundingBox() const
{
   CHECK( _shape.isValid() );
   return _shape->boundingBox();
}

//------------------------------------------------------------------------------
//!
inline const Reff&
RigidBody::simReferential() const
{
   return _simRef;
}

//------------------------------------------------------------------------------
//!
inline const Vec3f&
RigidBody::simPosition() const
{
   return _simRef.position();
}

//------------------------------------------------------------------------------
//!
inline const Quatf&
RigidBody::simOrientation() const
{
   return _simRef.orientation();
}

//------------------------------------------------------------------------------
//!
inline Mat4f
RigidBody::simTransform() const
{
   return _simRef.localToGlobal();
}

NAMESPACE_END

#endif
