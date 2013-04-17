/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef MOTIONBULLET_RIGIDBODY_H
#define MOTIONBULLET_RIGIDBODY_H

#include <MotionBullet/StdDefs.h>
#include <MotionBullet/Collision/CollisionShape.h>
#include <MotionBullet/Collision/CollisionGroup.h>

#include <CGMath/Ref.h>

#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>

class btRigidBody;

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
      KINEMATIC = 2,
   };

   enum TypeMask
   {
      DYNAMIC_MASK   = (1 << DYNAMIC  ),
      STATIC_MASK    = (1 << STATIC   ),
      KINEMATIC_MASK = (1 << KINEMATIC),
      ANY_MASK       = DYNAMIC_MASK|STATIC_MASK|KINEMATIC_MASK,
   };

   /*----- static methods -----*/

   static inline bool collide( const RigidBody& b0, const RigidBody& b1 )
   {
      return (b0.exists() & b1.exists()) != 0;
   }

   static inline bool sense( const RigidBody& b0, const RigidBody& b1 )
   {
      return ((b0.senses() & b1.exists()) | (b1.senses() & b0.exists())) != 0;
   }

   MOTION_DLL_API static void computeShapeProperties(
      const float*    pos,
      int             stride,
      const uint32_t* indices,
      int             numTriangles,
      Reff&           centerOfMass,
      Vec3f&          tensor
   );

   /*----- methods -----*/

   MOTION_DLL_API RigidBody( Type type, void* userRef );
   MOTION_DLL_API virtual ~RigidBody();

   MOTION_DLL_API void type( Type v );
   inline Type type() const;
   inline bool isDynamic() const   { return _type == DYNAMIC; }
   inline bool isKinematic() const { return _type == KINEMATIC; }
   inline bool isStatic() const    { return _type == STATIC; }
   inline uint typeMask() const;

   inline MotionWorld* world() const    { return _world; }

   MOTION_DLL_API void   userRef( void* v );
   MOTION_DLL_API void*  userRef() const;

   // Properties.
   MOTION_DLL_API void friction( float );
   MOTION_DLL_API void restitution( float );
   MOTION_DLL_API void mass( float );

   MOTION_DLL_API float friction() const;
   MOTION_DLL_API float restitution() const;
   inline         float mass() const { return _mass; }

   MOTION_DLL_API void referential( const Reff& );
   MOTION_DLL_API void linearVelocity( const Vec3f& );
   MOTION_DLL_API void angularVelocity( const Vec3f& );

   MOTION_DLL_API const Vec3f& centerPosition() const;
   MOTION_DLL_API Quatf centerOrientation() const;
   MOTION_DLL_API Mat4f transform() const;
   MOTION_DLL_API Reff  referential() const;
   MOTION_DLL_API const Vec3f& linearVelocity() const;
   MOTION_DLL_API const Vec3f& angularVelocity() const;

   inline Vec3f velocity( const Vec3f& pos ) const;

/*
   // Activation.
   inline bool   active() const;
   inline void   activate();
   inline void   deactivate();
*/

   // Collision execution mask (channel).
   inline uint exists() const                        { return _existsMask; }
   inline void exists( uint m )                      { _existsMask = m; }

   // Collision detection mask (channel).
   inline uint senses() const                        { return _sensesMask; }
   inline void senses( uint m )                      { _sensesMask = m; }
   inline bool canSense( const RigidBody& b )        { return (senses() & b.exists()) != 0x0; }

   // Attraction category.
   inline uint attractionCategories() const          { return _attractionCats; }
   inline void attractionCategories( uint c )        { _attractionCats = c; }
   inline void addAttractionCategories( uint c )     { _attractionCats |= c; }
   inline void removeAttractionCategories( uint c )  { _attractionCats &= ~c; }

   // Forces.
   inline void addForce( const Vec3f& force, const Vec3f& relPos );
   MOTION_DLL_API void addForce( const Vec3f& );
   MOTION_DLL_API void addTorque( const Vec3f& );
   MOTION_DLL_API const Vec3f& totalForce() const;
   MOTION_DLL_API const Vec3f& totalTorque() const;

   // Impulse.
   MOTION_DLL_API void applyImpulse( const Vec3f& impulse, const Vec3f& relPos );
   MOTION_DLL_API void applyImpulse( const Vec3f& impulse );
   MOTION_DLL_API void applyTorqueImpulse( const Vec3f& impulse );

   // Collision.
   MOTION_DLL_API void shape( CollisionShape* );
   inline CollisionShape* shape() const { return _shape.ptr(); }

protected:

   /*----- friends -----*/

   friend class MotionWorld;
   friend class Constraint;

   /*----- methods -----*/

   void connect( MotionWorld* );
   void disconnect();
   MOTION_DLL_API void btUpdate( bool mass = false );
   MOTION_DLL_API void btUpdateMass();

private:

   /*----- classes -----*/

   class MotionState;

   /*----- data members -----*/

   Type                _type;
   MotionWorld*        _world;
   btRigidBody*        _body;
   MotionState*        _motionState;
   RCP<CollisionShape> _shape;

   float               _mass;           //!< Cached version of the mass for when a body is static->dynamic->static.
   Vec3f               _totalForce;
   Vec3f               _totalTorque;

   uint                _existsMask;
   uint                _sensesMask;
   uint                _attractionCats; //!< A bitset indicating the types of attractor which can affect this rigid body.  By default, the object is a member of all categories, e.g. ~0x0.
};

//------------------------------------------------------------------------------
//!
inline RigidBody::Type
RigidBody::type() const
{
   return _type;
}

//------------------------------------------------------------------------------
//!
inline uint
RigidBody::typeMask() const
{
   return 1 << _type;
}

//------------------------------------------------------------------------------
//!
inline Vec3f
RigidBody::velocity( const Vec3f& pos ) const
{
   return linearVelocity() + angularVelocity().cross( pos - centerPosition() );
}

//------------------------------------------------------------------------------
//!
inline void
RigidBody::addForce( const Vec3f& force, const Vec3f& relPos )
{
   addForce( force );
   addTorque( relPos.cross( force ) );
}

NAMESPACE_END

#endif
