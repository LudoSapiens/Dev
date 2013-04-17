/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef MOTION_IMPULSESOLVER_H
#define MOTION_IMPULSESOLVER_H

#include <Motion/StdDefs.h>

#include <Motion/World/RigidBody.h>
#include <Motion/World/MotionWorld.h>

#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>

NAMESPACE_BEGIN


/*==============================================================================
   CLASS ImpulseSolver
==============================================================================*/
//!
class ImpulseSolver 
   : public RCObject
{

public: 

   /*----- methods -----*/

   MOTION_DLL_API ImpulseSolver( MotionWorld* );

   MOTION_DLL_API void solveCollisions();
   MOTION_DLL_API void solveConstraints( double step );
   MOTION_DLL_API void solveVelocities();

protected: 

   /*----- methods -----*/

   virtual ~ImpulseSolver();

private: 

   /*----- data members -----*/

   MotionWorld* _world;

   Vector< MotionWorld::CollisionConstraint* > _contacts;
};

NAMESPACE_END


#endif
