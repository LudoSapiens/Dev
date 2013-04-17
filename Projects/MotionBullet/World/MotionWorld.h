/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef MOTIONBULLET_MOTIONWORLD_H
#define MOTIONBULLET_MOTIONWORLD_H

#include <MotionBullet/StdDefs.h>

#include <MotionBullet/Attractor/Attractor.h>
#include <MotionBullet/Collision/CollisionInfo.h>
#include <MotionBullet/Constraint/Joints.h>
#include <MotionBullet/World/RigidBody.h>

#include <CGMath/Ray.h>

#include <Base/ADT/Pair.h>
#include <Base/ADT/Set.h>
#include <Base/ADT/Vector.h>
#include <Base/Msg/Delegate.h>
#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>

#include <utility>

class btConstraintSolver;
class btDiscreteDynamicsWorld;
class btBroadphaseInterface;
class btDispatcher;
class btCollisionConfiguration;
class btWorld;

NAMESPACE_BEGIN

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

   struct IntersectionData
   {
      typedef Pair<RigidBody*, Vec3f>  Entry;

      Vector< Entry >  _entries;

      bool  empty() const { return _entries.empty(); }
      uint  numBodies() const { return uint(_entries.size()); }
                  bool   hit( uint idx )       { return _entries[idx].first != nullptr; }
            RigidBody*  body( uint idx )       { return _entries[idx].first; }
      const RigidBody*  body( uint idx ) const { return _entries[idx].first; }
      const Vec3f&  location( uint idx ) const { return _entries[idx].second; }

      void  add( RigidBody* body, const Vec3f& loc ) { _entries.pushBack( Entry(body, loc) ); }

      void  clear() { _entries.clear(); }

      size_t  size() const     { return _entries.size(); }
      void  resize( size_t n ) { _entries.resize( n, Entry(NULL, Vec3f()) ); }

      void  print() const
      {
         uint n = numBodies();
         StdErr << n << " intersection:" << nl;
         for( uint i = 0; i < n; ++i )
         {
            StdErr << "[" << i << "]: body=" << (void*)body(i) << " loc=" << location(i) << nl;
         }
      }

   };

/*
   struct CollisionConstraint
   {
      CollisionConstraint( RigidBody* a, RigidBody* b, CollisionInfo::Contact* c ) :
         _bodyA( a ), _bodyB( b ), _contact( c ) {}

      RigidBody*              _bodyA;
      RigidBody*              _bodyB;
      CollisionInfo::Contact* _contact;
   };
*/
   /*----- types and enumerations ----*/

// typedef Set< CollisionPair >                           CollisionContainer;
// typedef Vector< CollisionConstraint >                  CollisionConstraintContainer;
   typedef Vector< RCP<Constraint> >                      ConstraintContainer;
   typedef Delegate1< const CollisionPair& >              CollisionCallback;
   typedef Delegate3< void*, const Reff&, const Mat4f& >  SetTransformCallback;
   typedef Delegate3< const void*, Reff&, Mat4f& >        GetTransformCallback;

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
   inline double time          () const { return _time; }
   //inline void time          ( double t ) { _time = t;  }

   // Bodies.
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

   MOTION_DLL_API void  findIntersectingBodies( const CollisionShape& shape, const Reff& shapeRef, IntersectionData& intersections );
   MOTION_DLL_API void  raycast( const Vector<Rayf>& rays, uint typeMask, uint existMask, IntersectionData& intersections );
   MOTION_DLL_API void  ccd(
      const CollisionShape& convex,
      const Reff&           from,
      const Reff&           to,
      IntersectionData&     intersections,
      Vec3f&                collisionPos,
      const RigidBody*      nonBody = nullptr
   );

   MOTION_DLL_API void  debugRender( Vector<float>& points, Vector<float>& lines, Vector<float>& triangles );

   // Simulation.
   MOTION_DLL_API bool stepSimulation( double step );

   // Collision callback.
   void collisionBeginCallback( const CollisionCallback& c )   { _collisionBeginCallback = c; }
   void collisionEndCallback( const CollisionCallback& c )     { _collisionEndCallback   = c; }
   void sensationBeginCallback( const CollisionCallback& c )   { _sensationBeginCallback = c; }
   void sensationEndCallback( const CollisionCallback& c )     { _sensationEndCallback   = c; }

   // Bodies transformations.
   static inline  void  getTransformCallback( const GetTransformCallback& c )             { _getTransformCB = c; }
   static inline  void  setTransformCallback( const SetTransformCallback& c )             { _setTransformCB = c; }
   static inline  void  getTransform( const void* ud,       Reff& ref,       Mat4f& mat ) { _getTransformCB(ud, ref, mat); }
   static inline  void  setTransform(       void* ud, const Reff& ref, const Mat4f& mat ) { _setTransformCB(ud, ref, mat); }

   MOTION_DLL_API void defaultGravity( const Vec3f& g );
   MOTION_DLL_API void getGravity( const RigidBody* body, Vec3f& gravityDir, float& gravityNorm ) const;

protected:

   /*----- methods -----*/

   virtual ~MotionWorld();

private:

  /*----- friends -----*/

   friend class RigidBody;
   friend class Constraint;
   friend class ::btWorld;

   /*----- methods -----*/

   void bodyTypeChanged( RigidBody* b, RigidBody::Type newType );

   inline void  handleAttractors();
   inline void  handleConstraintsPreStep( double step );
   inline void  handleConstraintsPostStep( double step );
   inline void  clearForces();

   /*----- static data members -----*/

   MOTION_DLL_API static GetTransformCallback  _getTransformCB;
   MOTION_DLL_API static SetTransformCallback  _setTransformCB;

   /*----- data members -----*/

   btDiscreteDynamicsWorld*     _world;
   btConstraintSolver*          _solver;
   btBroadphaseInterface*       _broadPhase;
   btDispatcher*                _collisionDispatcher;
   btCollisionConfiguration*    _collisionConfiguration;


   double                       _simulationRate;
   double                       _simulationDelta;
   double                       _time;

   Vector< RCP<RigidBody> >     _rigidBodies;
   Vector< RCP<RigidBody> >     _staticBodies;
   Vector< RCP<RigidBody> >     _kinematicBodies;
   Vector< RCP<Attractor> >     _attractors;
   Vector< RCP<Constraint> >    _constraints;

   CollisionCallback            _collisionBeginCallback;
   CollisionCallback            _collisionEndCallback;
   CollisionCallback            _sensationBeginCallback;
   CollisionCallback            _sensationEndCallback;

   Vec3f                        _defaultGravityDir;
   float                        _defaultGravityNorm;
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

/*
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
*/

//------------------------------------------------------------------------------
//!
inline void
MotionWorld::handleAttractors()
{
   // Apply attractors.
   for( uint i = 0; i < _attractors.size(); ++i )
   {
      _attractors[i]->addForce( _rigidBodies );
   }
}

//------------------------------------------------------------------------------
//!
inline void
MotionWorld::handleConstraintsPreStep( double step )
{
   // Constraints pre-step.
   for( uint i = 0; i < _constraints.size(); ++i )
   {
      _constraints[i]->preStep( step );
   }
}

//------------------------------------------------------------------------------
//!
inline void
MotionWorld::handleConstraintsPostStep( double step )
{
   // Constraints post-step.
   for( uint i = 0; i < _constraints.size(); ++i )
   {
      _constraints[i]->postStep( step );
   }
}

//------------------------------------------------------------------------------
//!
inline void
MotionWorld::clearForces()
{
   for( uint i = 0; i < _rigidBodies.size(); ++i )
   {
      _rigidBodies[i]->_totalTorque = Vec3f( 0.0f );
      _rigidBodies[i]->_totalForce  = Vec3f( 0.0f );
   }
   for( uint i = 0; i < _kinematicBodies.size(); ++i )
   {
      _kinematicBodies[i]->_totalTorque = Vec3f( 0.0f );
      _kinematicBodies[i]->_totalForce  = Vec3f( 0.0f );
   }
}


NAMESPACE_END

#endif
