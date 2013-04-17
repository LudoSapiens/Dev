/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef MOTIONBULLET_CHARACTER_CONSTRAINT_H
#define MOTIONBULLET_CHARACTER_CONSTRAINT_H

#include <MotionBullet/StdDefs.h>
#include <MotionBullet/World/RigidBody.h>
#include <MotionBullet/Constraint/Constraint.h>

#include <CGMath/Mat3.h>
#include <CGMath/Ref.h>
#include <CGMath/Vec3.h>

NAMESPACE_BEGIN


class CollisionShape;

/*==============================================================================
   CLASS CharacterConstraint
==============================================================================*/
//! A constraint which is used to control a character.
//! This constraint has the property to ignore tangential forces being applied
//! unto the character.
class CharacterConstraint:
   public Constraint
{

public:

   /*----- methods -----*/
   MOTION_DLL_API CharacterConstraint( RigidBody* );
   MOTION_DLL_API virtual ~CharacterConstraint();

   inline void constrainLinearVelocity( const bool v ) { _conLinVel = v; }
   inline bool constrainLinearVelocity() const { return _conLinVel; }

   inline void linearVelocity( const Vec3f& v ) { _linVel = v; }
   inline const Vec3f& linearVelocity() { return _linVel; }

   inline void maxForce( const float f ) { _maxF = f; }
   inline float maxForce() { return _maxF; }
   
   inline bool falling() const { return _falling; }
   inline float fallDuration() const { return _fallDur; }

   virtual void preStep( double step );
   virtual void postStep( double step );

protected:

   /*----- friends -----*/

   friend class MotionWorld;

   /*----- methods -----*/

   virtual btTypedConstraint* btConstraint();

   /*----- data members -----*/
   
   bool            _falling;    //!< A bit indicating that the character is suffering from an acceleration equivalent to the gravity.
   bool            _conLinVel;  //!< A bit indicating whether or not we contrain the linear velocity.

   RCP<RigidBody>  _body;    //!< The associated rigid body.
   Vec3f           _linVel;  //!< The desired linear velocity.
   float           _maxF;    //!< The maximum force which can be applied.
   Vec3f           _oLinVel; //!< The previous linear velocity.
   float           _fallDur; //!< The duration of the latest fall.
};


NAMESPACE_END

#endif
