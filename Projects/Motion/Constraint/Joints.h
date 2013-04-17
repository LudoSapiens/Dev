/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef MOTION_JOINTS_H
#define MOTION_JOINTS_H

#include <Motion/StdDefs.h>
#include <Motion/Constraint/Constraint.h>

#include <CGMath/Mat3.h>
#include <CGMath/Vec3.h>

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

   // Solving constraint.
   MOTION_DLL_API virtual void prePositionStep( double step );
   MOTION_DLL_API virtual bool solvePosition( double step );

   MOTION_DLL_API virtual void preVelocitiesStep();
   MOTION_DLL_API virtual bool solveVelocities();


protected: 
   
   /*----- friends -----*/

   friend class MotionWorld;
   
   /*----- methods -----*/

   /*----- data members -----*/

   RCP<RigidBody> _bodyA;
   RCP<RigidBody> _bodyB;
   
   Vec3f          _anchorA;
   Vec3f          _anchorB;
   Vec3f          _worldAnchorA;
   Vec3f          _worldAnchorB;
   Mat3f          _kinv;
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

   // Solving constraint.
   MOTION_DLL_API virtual void prePositionStep( double step );
   MOTION_DLL_API virtual bool solvePosition( double step );

   MOTION_DLL_API virtual void preVelocitiesStep();
   MOTION_DLL_API virtual bool solveVelocities();


protected: 
   
   /*----- friends -----*/

   friend class MotionWorld;
   
   /*----- methods -----*/

   /*----- data members -----*/

   RCP<RigidBody> _bodyA;
   RCP<RigidBody> _bodyB;
   
   Vec3f          _anchorA;
   Vec3f          _anchorB;
   Vec3f          _worldAnchorA;
   Vec3f          _worldAnchorB;
   Mat3f          _kinv;
   
   Vec3f          _axisA;
   Vec3f          _axisB;
};

NAMESPACE_END

#endif
