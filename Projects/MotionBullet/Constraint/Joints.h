/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef MOTIONBULLET_JOINTS_H
#define MOTIONBULLET_JOINTS_H

#include <MotionBullet/StdDefs.h>
#include <MotionBullet/Constraint/Constraint.h>

#include <CGMath/Mat3.h>
#include <CGMath/Vec3.h>
#include <CGMath/Ref.h>

class btPoint2PointConstraint;
class btHingeConstraint;
class btGeneric6DofConstraint;

NAMESPACE_BEGIN

class RigidBody;

/*==============================================================================
   CLASS BallJoint
==============================================================================*/
//! 3 degrees of freedom constraint. Only rotational motion is allowed
//! between two bodies.
class BallJoint
   : public Constraint
{

public:

   /*----- methods -----*/

   MOTION_DLL_API BallJoint( RigidBody*, RigidBody* );
   MOTION_DLL_API virtual ~BallJoint();

   // Attributes.
   MOTION_DLL_API void anchor( const Vec3f& pos );

protected:

   /*----- friends -----*/

   friend class MotionWorld;

   /*----- methods -----*/

   virtual btTypedConstraint* btConstraint();

   /*----- data members -----*/

   RCP<RigidBody>           _bodyA;
   RCP<RigidBody>           _bodyB;
   btPoint2PointConstraint* _btConstraint;
};

/*==============================================================================
   CLASS HingeJoint
==============================================================================*/
//! 1 degree of freedom constraint. Only rotational motion around an axis is
//! allowed between two bodies.
class HingeJoint
   : public Constraint
{

public:

   /*----- methods -----*/

   MOTION_DLL_API HingeJoint( RigidBody*, RigidBody* );
   MOTION_DLL_API virtual ~HingeJoint();

   // Attributes.
   MOTION_DLL_API void anchor( const Vec3f& pos );
   MOTION_DLL_API void axis( const Vec3f& dir );

protected:

   /*----- friends -----*/

   friend class MotionWorld;

   /*----- methods -----*/

   virtual btTypedConstraint* btConstraint();

   /*----- data members -----*/

   RCP<RigidBody>     _bodyA;
   RCP<RigidBody>     _bodyB;
   btHingeConstraint* _btConstraint;
};

/*==============================================================================
   CLASS FixJoint
==============================================================================*/
//! 0 degrees of freedom constraint.
class FixJoint
   : public Constraint
{

public:

   /*----- methods -----*/

   MOTION_DLL_API FixJoint( RigidBody*, RigidBody* );
   MOTION_DLL_API virtual ~FixJoint();

   // Attributes.
   MOTION_DLL_API void anchor( const Reff& ref );

protected:

   /*----- friends -----*/

   friend class MotionWorld;

   /*----- methods -----*/

   virtual btTypedConstraint* btConstraint();

   /*----- data members -----*/

   RCP<RigidBody>           _bodyA;
   RCP<RigidBody>           _bodyB;
   btGeneric6DofConstraint* _btConstraint;
};

NAMESPACE_END

#endif
