/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Motion/World/MotionWorld.h>

#include <Motion/Constraint/CharacterConstraint.h>
#include <Motion/Solver/SequentialImpulseSolver.h>
#include <Motion/Solver/ImpulseSolver.h>
#include <Motion/Solver/NextSolver.h>

#include <Base/Dbg/DebugStream.h>

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_att, "Attractor" );

UNNAMESPACE_END


NAMESPACE_BEGIN

/*==============================================================================
   CLASS MotionWorld
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
MotionWorld::printInfo( TextStream& os )
{
   os << "Motion: custom" << nl;
}

//------------------------------------------------------------------------------
//!
MotionWorld::MotionWorld() :
   _time( 0.0 ),
   _simulationTime( 0.0 )
{
   simulationRate( 60.0 );
   //_solver = new SequentialImpulseSolver( this );
   _solver = new ImpulseSolver( this );
   //_solver = new NextSolver( this );
}

//------------------------------------------------------------------------------
//!
MotionWorld::~MotionWorld()
{
   // Disconnect all connected elements.
   for( uint i = 0; i < _rigidBodies.size(); ++i )
   {
      _rigidBodies[i]->disconnect();
   }
   for( uint i = 0; i < _staticBodies.size(); ++i )
   {
      _staticBodies[i]->disconnect();
   }
   for( uint i = 0; i < _kinematicBodies.size(); ++i )
   {
      _kinematicBodies[i]->disconnect();
   }
   for( uint i = 0; i < _attractors.size(); ++i )
   {
      _attractors[i]->disconnect();
   }
   for( uint i = 0; i < _constraints.size(); ++i )
   {
      _constraints[i]->disconnect();
   }
}

//------------------------------------------------------------------------------
//!
RCP<RigidBody>
MotionWorld::createRigidBody()
{
   RCP<RigidBody> body( new RigidBody( RigidBody::DYNAMIC ) );
   return body;
}

//------------------------------------------------------------------------------
//!
RCP<RigidBody>
MotionWorld::createStaticBody()
{
   RCP<RigidBody> body( new RigidBody( RigidBody::STATIC ) );
   return body;
}

//------------------------------------------------------------------------------
//!
RCP<RigidBody>
MotionWorld::createKinematicBody()
{
   RCP<RigidBody> body( new RigidBody( RigidBody::KINEMATIC ) );
   return body;
}

//------------------------------------------------------------------------------
//!
void
MotionWorld::addBody( RigidBody* body )
{
   switch( body->type() )
   {
      case RigidBody::DYNAMIC:    _rigidBodies.pushBack( body );     break;
      case RigidBody::STATIC:     _staticBodies.pushBack( body );    break;
      case RigidBody::KINEMATIC:  _kinematicBodies.pushBack( body ); break;
   }
   body->connect( this );
}

//------------------------------------------------------------------------------
//!
void
MotionWorld::removeBody( RigidBody* body )
{
   body->disconnect();
   switch( body->type() )
   {
      case RigidBody::DYNAMIC:    _rigidBodies.remove( body );     break;
      case RigidBody::STATIC:     _staticBodies.remove( body );    break;
      case RigidBody::KINEMATIC:  _kinematicBodies.remove( body ); break;
   }
}

//------------------------------------------------------------------------------
//!
void
MotionWorld::removeAllBodies()
{
   _rigidBodies.clear();
   _staticBodies.clear();
   _kinematicBodies.clear();
}

//------------------------------------------------------------------------------
//!
void
MotionWorld::addAttractor( Attractor* attractor )
{
   if( attractor->_world != NULL )
   {
      if( attractor->_world != this )
      {
         DBG_MSG( os_att, "Attractor moving to another world" );
         attractor->_world->removeAttractor( attractor );
      }
      else
      {
         DBG_MSG( os_att, "Attractor already connected to this world" );
         return;
      }
   }
   _attractors.pushBack( attractor );
   attractor->_world = this;
}

//------------------------------------------------------------------------------
//!
void
MotionWorld::removeAttractor( Attractor* attractor )
{
   attractor->disconnect();
   _attractors.remove( attractor );
}

//------------------------------------------------------------------------------
//!
void
MotionWorld::removeAllAttractors()
{
   for( uint i = 0; i < _attractors.size(); ++i )
   {
      _attractors[i]->disconnect();
   }
   _attractors.clear();
}

//------------------------------------------------------------------------------
//!
BallJoint*
MotionWorld::createBallJoint( RigidBody* bodyA, RigidBody* bodyB )
{
   BallJoint* joint = new BallJoint( bodyA, bodyB );
   addConstraint( joint );
   return joint;
}

//------------------------------------------------------------------------------
//!
HingeJoint*
MotionWorld::createHingeJoint( RigidBody* bodyA, RigidBody* bodyB)
{
   HingeJoint* joint = new HingeJoint( bodyA, bodyB );
   addConstraint( joint );
   return joint;
}

//------------------------------------------------------------------------------
//!
CharacterConstraint*
MotionWorld::createCharacterConstraint( RigidBody* body )
{
   CharacterConstraint* con = new CharacterConstraint( body );
   addConstraint( con );
   return con;
}

//------------------------------------------------------------------------------
//!
void
MotionWorld::addConstraint( Constraint* constraint )
{
   _constraints.pushBack( constraint );
}

//------------------------------------------------------------------------------
//!
void
MotionWorld::removeConstraint( Constraint* constraint )
{
   _constraints.remove( constraint );
}

//------------------------------------------------------------------------------
//!
void
MotionWorld::removeAllConstraints()
{
   _constraints.clear();
}

//------------------------------------------------------------------------------
//!
bool
MotionWorld::stepSimulation( double step )
{
   bool ran = false;
   // For now the simulation step is fixed.
   if( step > 0 )
   {
      double previousTime = _time;
      _time += step;

      // Physics simulation.
      while( _simulationTime <= _time )
      {
         singleStepSimulation( _simulationDelta );
         ran = true;
      }

      // Interpolate the new position of the objects.
      double t = step / ( _simulationTime - previousTime );

      Vector< RCP<RigidBody> >::Iterator it  = _rigidBodies.begin();
      Vector< RCP<RigidBody> >::Iterator end = _rigidBodies.end();

      for( ; it != end; ++it )
      {
         (*it)->update( (float)t );
      }
   }
   return ran;
}

//------------------------------------------------------------------------------
//!
void
MotionWorld::singleStepSimulation( double step )
{
   // Collision.
   collisionDetection();

   // Apply external forces.
   for( uint i = 0; i < _attractors.size(); ++i )
   {
      _attractors[i]->addForce( _rigidBodies );
   }

   // Update velocities by integrating forces.
   for( uint i = 0; i < _rigidBodies.size(); ++i )
   {
      _rigidBodies[i]->applyForces( step );
   }

   // Collisions solving.
   _solver->solveCollisions();

   // Contacts + Joints.
   _solver->solveConstraints( step );

   // Update bodies positions.
   for( uint i = 0; i < _rigidBodies.size(); ++i )
   {
      _rigidBodies[i]->applyVelocities( step );
   }

   // Fix velocities.
   _solver->solveVelocities();

   // Increase simulation time.
   _simulationTime += step;
}

//------------------------------------------------------------------------------
//!
void
MotionWorld::collisionDetection()
{
   static uint frame = 0;
   // 3. handle multiple contact in collision primitive (add/remove)
   // 4. handle multiple pair in solver...


   // Broadphase.
   // For now test all pairs of CollisionShape.

   // 1. Update collision pairs.
   for( uint a = 0; a < _rigidBodies.size(); ++a )
   {
      const RCP<RigidBody>& bodyA = _rigidBodies[a];
      if( bodyA->shape() == NULL )  continue;
      for( uint b = 0; b < _staticBodies.size(); ++b )
      {
         const RCP<RigidBody>& bodyB = _staticBodies[b];
         if( bodyB->shape() == NULL )  continue;
         // Narrow phase.
         if( bodyA->boundingBox().isOverlapping( bodyB->boundingBox() ) )
         {
            CollisionPair pair( bodyA, bodyB );
            _collisions.add( pair ).frame( frame );
         }
      }

      for( uint b = 0; b < _kinematicBodies.size(); ++b )
      {
         const RCP<RigidBody>& bodyB = _kinematicBodies[b];
         if( bodyB->shape() == NULL )  continue;
         // Narrow phase.
         if( bodyA->boundingBox().isOverlapping( bodyB->boundingBox() ) )
         {
            CollisionPair pair( bodyA, bodyB );
            _collisions.add( pair ).frame( frame );
         }
      }

      for( uint b = a+1; b < _rigidBodies.size(); ++b )
      {
         const RCP<RigidBody>& bodyB = _rigidBodies[b];
         if( bodyB->shape() == NULL )  continue;
         // Narrow phase.
         if( bodyA->boundingBox().isOverlapping( bodyB->boundingBox() ) )
         {
            CollisionPair pair( bodyA, bodyB );
            _collisions.add( pair ).frame( frame );
         }
      }
   }

   // 2. Handle collision pairs and create collision constraints.
   _collisionConstraints.clear();
   CollisionContainer::Iterator it = _collisions.begin();
   for( ; it != _collisions.end(); )
   {
      // Remove unused pairs.
      if( (*it).frame() != frame )
      {
         it = _collisions.remove( it );
      }
      else
      {
         CollisionShape::collide( (*it) );

         // Update contact positions in world space.
         (*it).info()->updatePositions(
            (*it).bodyA()->simTransform(),
            (*it).bodyB()->simTransform()
         );

         // Remove invalid contacts.
         (*it).info()->removeInvalids();

         // Check if objects can collide with one another.
         if( RigidBody::canCollide( *(*it).bodyA(), *(*it).bodyB() ) )
         {
            for( uint c = 0; c < (*it).info()->numContacts(); ++c )
            {
               _collisionConstraints.pushBack(
                  CollisionConstraint( (*it).bodyA(), (*it).bodyB(), &(*it).info()->contact(c) )
               );
            }
         }

         // Check if collision triggers a callback.
         if( !_collisionCallback.empty()    &&
             (*it).info()->numContacts() > 0 &&
             RigidBody::triggersCallback( *(*it).bodyA(), *(*it).bodyB() ) )
         {
            _collisionCallback( (*it) );
         }

         ++it;
      }
   }

   ++frame;
}

NAMESPACE_END
