/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef MOTION_MOTIONWORLD_H
#define MOTION_MOTIONWORLD_H

#include <Motion/StdDefs.h>

#include <Motion/Attractor/Attractor.h>
#include <Motion/Collision/CollisionInfo.h>
#include <Motion/Constraint/Joints.h>
#include <Motion/World/RigidBody.h>

#include <Base/ADT/Set.h>
#include <Base/ADT/Vector.h>
#include <Base/Msg/Delegate.h>
#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>

NAMESPACE_BEGIN

class SequentialImpulseSolver;
class ImpulseSolver;
class NextSolver;

class CharacterConstraint;

/*==============================================================================
   CLASS MotionWorld
==============================================================================*/
//!
class MotionWorld 
   : public RCObject
{

public: 

   /*----- classes -----*/

   struct CollisionConstraint
   {
      CollisionConstraint( RigidBody* a, RigidBody* b, CollisionInfo::Contact* c ) :
         _bodyA( a ), _bodyB( b ), _contact( c ) {}
      
      RigidBody*              _bodyA;
      RigidBody*              _bodyB;
      CollisionInfo::Contact* _contact;
   };

   /*----- types and enumerations ----*/

   typedef Set< CollisionPair >              CollisionContainer;
   typedef Vector< RCP<Constraint> >         ConstraintContainer;
   typedef Vector< CollisionConstraint >     CollisionConstraintContainer;
   typedef Delegate1< const CollisionPair& > CollisionCallback;

   /*----- static methods -----*/
   MOTION_DLL_API static void printInfo( TextStream& os );

   /*----- methods -----*/

   MOTION_DLL_API MotionWorld();

   // Rate.
   inline double simulationRate () const   { return _simulationRate;  }
   inline double simulationDelta() const   { return _simulationDelta; }

   inline void simulationRate ( double r ) { _simulationRate = r;     _simulationDelta = 1.0/r; }
   inline void simulationDelta( double d ) { _simulationRate = 1.0/d; _simulationDelta = d;     }

   // Time.
   inline double time          () const { return _time;           }
   inline double simulationTime() const { return _simulationTime; }

   inline void time          ( double t ) { _time = t;           }
   inline void simulationTime( double t ) { _simulationTime = t; }

   // Bodies.
   static MOTION_DLL_API RCP<RigidBody> createRigidBody();
   static MOTION_DLL_API RCP<RigidBody> createStaticBody();
   static MOTION_DLL_API RCP<RigidBody> createKinematicBody();
   MOTION_DLL_API void addBody( RigidBody* );
   MOTION_DLL_API void removeBody( RigidBody* );
   MOTION_DLL_API void removeAllBodies();
   
   inline const Vector< RCP<RigidBody> >& staticBodies() const    { return _staticBodies; }
   inline const Vector< RCP<RigidBody> >& rigidBodies() const     { return _rigidBodies; }
   inline const Vector< RCP<RigidBody> >& kinematicBodies() const { return _kinematicBodies; }

   // Attractors.
   MOTION_DLL_API void addAttractor( Attractor* );
   MOTION_DLL_API void removeAttractor( Attractor* );
   MOTION_DLL_API void removeAllAttractors();

   // Constraints.
   MOTION_DLL_API BallJoint* createBallJoint( RigidBody*, RigidBody* );
   MOTION_DLL_API HingeJoint* createHingeJoint( RigidBody*, RigidBody* );
   MOTION_DLL_API CharacterConstraint* createCharacterConstraint( RigidBody* );

   MOTION_DLL_API void addConstraint( Constraint* );
   MOTION_DLL_API void removeConstraint( Constraint* );
   MOTION_DLL_API void removeAllConstraints();
   
   inline ConstraintContainer& constraints();
   inline const ConstraintContainer& constraints() const;

   // Simulation.
   MOTION_DLL_API bool stepSimulation( double step );
   
   // CollisionInfo.
   inline CollisionContainer& collisions();
   inline const CollisionContainer& collisions() const;
   
   inline CollisionConstraintContainer& collisionConstraints();
   inline const CollisionConstraintContainer& collisionConstraints() const;
   
   // Collision callback.
   inline void collisionCallback( const CollisionCallback& c ) { _collisionCallback = c; }
   inline void clearCollisionCallback() { _collisionCallback.clear(); }

protected: 

   /*----- methods -----*/

   virtual ~MotionWorld();

private: 

   /*----- methods -----*/

   void singleStepSimulation( double step );
   void collisionDetection();

   /*----- data members -----*/

   double                       _simulationRate;
   double                       _simulationDelta;

   double                       _time;
   double                       _simulationTime;
   //RCP<SequentialImpulseSolver> _solver;
   RCP<ImpulseSolver>           _solver;
   //RCP<NextSolver>              _solver;

   Vector< RCP<RigidBody> >     _rigidBodies;
   Vector< RCP<RigidBody> >     _staticBodies;
   Vector< RCP<RigidBody> >     _kinematicBodies;

   Vector< RCP<Attractor> >     _attractors;
   ConstraintContainer          _constraints;

   CollisionContainer           _collisions;
   CollisionConstraintContainer _collisionConstraints;

   CollisionCallback            _collisionCallback;
};

//------------------------------------------------------------------------------
//!
inline MotionWorld::ConstraintContainer& 
MotionWorld::constraints()
{
   return _constraints;
}

//------------------------------------------------------------------------------
//!
inline const MotionWorld::ConstraintContainer& 
MotionWorld::constraints() const
{
   return _constraints;
}

//------------------------------------------------------------------------------
//!
inline MotionWorld::CollisionContainer& 
MotionWorld::collisions()
{
   return _collisions;
}

//------------------------------------------------------------------------------
//!
inline const MotionWorld::CollisionContainer& 
MotionWorld::collisions() const
{
   return _collisions;
}

//------------------------------------------------------------------------------
//!
inline MotionWorld::CollisionConstraintContainer& 
MotionWorld::collisionConstraints()
{
   return _collisionConstraints;
}

//------------------------------------------------------------------------------
//!
inline const MotionWorld::CollisionConstraintContainer& 
MotionWorld::collisionConstraints() const
{
   return _collisionConstraints;
}

NAMESPACE_END


#endif
