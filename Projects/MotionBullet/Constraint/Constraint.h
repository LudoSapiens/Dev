/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef MOTIONBULLET_CONSTRAINT_H
#define MOTIONBULLET_CONSTRAINT_H

#include <MotionBullet/StdDefs.h>

#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>

class btTypedConstraint;
class btRigidBody;
class btDiscreteDynamicsWorld;

NAMESPACE_BEGIN

class MotionWorld;
class RigidBody;

/*==============================================================================
   CLASS Constraint
==============================================================================*/
//! Base class for handling constraints on rigid bodies.

class Constraint
   : public RCObject
{

public: 

   /*----- methods -----*/

   MOTION_DLL_API virtual void preStep( double step );
   MOTION_DLL_API virtual void postStep( double step );

protected: 
   
   /*----- methods -----*/

   Constraint();
   virtual ~Constraint();

   virtual btTypedConstraint* btConstraint() = 0;

   btRigidBody* btBody( RigidBody* body );
   btDiscreteDynamicsWorld* btWorld();

   friend class MotionWorld;
   void connect( MotionWorld* w );
   void disconnect();

   /*----- data members -----*/

   MotionWorld* _world;
};

NAMESPACE_END

#endif
