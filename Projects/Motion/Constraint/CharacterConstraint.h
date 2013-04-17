/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef MOTION_CHARACTER_CONSTRAINT_H
#define MOTION_CHARACTER_CONSTRAINT_H

#include <Motion/StdDefs.h>
#include <Motion/Constraint/Constraint.h>

#include <CGMath/Mat3.h>
#include <CGMath/Ref.h>
#include <CGMath/Vec3.h>

NAMESPACE_BEGIN


class RigidBody;
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
   
   inline bool onGround() const { return _onGround; }

   // Solving constraint.
   MOTION_DLL_API virtual void prePositionStep( double step );
   MOTION_DLL_API virtual bool solvePosition( double step );

   MOTION_DLL_API virtual void preVelocitiesStep();
   MOTION_DLL_API virtual bool solveVelocities();

protected:

   /*----- friends -----*/

   friend class MotionWorld;

   /*----- methods -----*/

   virtual void connect( MotionWorld* w );
   virtual void disconnect();

   /*----- data members -----*/
   
   RCP<CollisionShape> _shape;
   bool                _onGround;
   
   bool            _conLinVel;  //!< A bit indicating whether or not we contrain the linear velocity.

   RCP<RigidBody>  _body;    //!< The associated rigid body.
   Vec3f           _linVel;  //!< The desired linear velocity.
   float           _maxF;    //!< The maximum force which can be applied.

   // Cached values for more efficient simulation.
   Mat3f           _kinv;
   Vec3f           _desPos;  //!< The desired position (calculated using the simulation speed).
   Mat3f           _linv;    //!< This is J^-1^-1, therefore J.
   Quatf           _desOri;  //!< The desired orientation (considered intact for now).
   float           _maxP;    //!< The maximum impulse length allowed for the current step.

   Vec3f           _pSum;    //!< The total accumulated impulse so far.
};


NAMESPACE_END

#endif //MOTION_CHARACTER_CONSTRAINT_H
