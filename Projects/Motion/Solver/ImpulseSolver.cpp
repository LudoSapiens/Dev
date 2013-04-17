/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Motion/Solver/ImpulseSolver.h>
#include <Motion/World/MotionWorld.h>


NAMESPACE_BEGIN

/*==============================================================================
   CLASS ImpulseSolver
==============================================================================*/

//------------------------------------------------------------------------------
//!
ImpulseSolver::ImpulseSolver( MotionWorld* world ) :
   _world( world )
{
}

//------------------------------------------------------------------------------
//!
ImpulseSolver::~ImpulseSolver()
{
}

//------------------------------------------------------------------------------
//!
void
ImpulseSolver::solveCollisions()
{
   const float restingThreshold = -0.4f;
   const int maxLoopCount = 10;

   MotionWorld::CollisionConstraintContainer::Iterator it;
   MotionWorld::CollisionConstraintContainer::Iterator end = 
      _world->collisionConstraints().end();

   // Precompute per contact data.
   for( it = _world->collisionConstraints().begin(); it != end; ++it )
   {
      CollisionInfo::Contact& c = *(*it)._contact;
      c._k = (*it)._bodyA->computeK( c.worldPositionA() ) +
             (*it)._bodyB->computeK( c.worldPositionB() );
      c._normK = 1.0f / c.worldNormal().dot( c._k*c.worldNormal() );
      c._p = 0.0f;
   }

   // Solve collisions.
   int nbCollisions;
   int i = 0;
   
   do
   {
      nbCollisions = 0;
      for( it = _world->collisionConstraints().begin(); it != end; ++it )
      {
         CollisionInfo::Contact& c = *(*it)._contact;

         const Vec3f& posA = c.worldPositionA();
         const Vec3f& posB = c.worldPositionB();
         const Vec3f& n    = c.worldNormal();

         // Compute normal relative velocity.
         Vec3f ua = (*it)._bodyA->velocity( posA );
         Vec3f ub = (*it)._bodyB->velocity( posB );
         
         c._nRelVel = ( ua - ub ).dot( n );

         // Do we have a collision?
         if( c._nRelVel <= restingThreshold )
         {
            ++nbCollisions;

            // Compute collision impulse.

            // 1. Normal velocity after collision.
            float nVelAfter = -c._nRelVel * (*it)._bodyA->restitution( *(*it)._bodyB );

            // 2. Impulse.
            float p = (nVelAfter - c._nRelVel) * c._normK;
               
            // 3. Clamp impulse.
            if( -p > c._p )
            {
               p    = -c._p;
               c._p = 0.0f;
            }
            else
            {               
               c._p += p;
            }

            // 5. Apply impulse.
            (*it)._bodyA->applyImpulse( n*p, posA );
            (*it)._bodyB->applyImpulse( n*-p, posB );

            // Compute friction impulse.
            ua = (*it)._bodyA->velocity( posA );
            ub = (*it)._bodyB->velocity( posB );
            c._nRelVel = ( ua - ub ).dot( n );

            // 1. Tangential relative velocity.
            Vec3f tRelVel = ( ua - ub ) - n * c._nRelVel;

            float tvelLen = tRelVel.length();

            if( tvelLen > 0.000001f )
            {
               // 2. Tangent.
               Vec3f tan = tRelVel * ( 1.0f / tvelLen );

               // 3. Friction impulse.
               float tp = -CGM::abs(p) * (*it)._bodyA->friction( *(*it)._bodyB );

               // 4. Max friction impulse.
               float tpMax = -tvelLen / tan.dot( c._k*tan );

               // 5. Apply impulse.
               if( tpMax > tp )
               {
                  tp = tpMax;
               }
               (*it)._bodyA->applyImpulse( tan*tp, posA );
               (*it)._bodyB->applyImpulse( tan*-tp, posB );
            }
         }
      }
   } while( nbCollisions && (++i < maxLoopCount) );


   // Create contact constraints.
   _contacts.clear();
   for( it = _world->collisionConstraints().begin(); it != end; ++it )
   {
      if( (*it)._contact->_nRelVel <= 0.1f )
      //if( (*it)._contact->_nRelVel <= -restingThreshold )
      {
         // Add to constraints list.
         _contacts.pushBack( it );
      }
   }
}

//------------------------------------------------------------------------------
//!
void
ImpulseSolver::solveConstraints( double step )
{
   const int maxLoopCount  = 10;
   const float maxDistance = 0.001f; // 1 mm precision.
   
   // Precompute per contact data.
   Vector< MotionWorld::CollisionConstraint* >::Iterator it;
   Vector< MotionWorld::CollisionConstraint* >::Iterator end = _contacts.end();

   for( it = _contacts.begin(); it != end; ++it )
   {
      (*it)->_contact->_p = 0.0f;
   }

   // Precompute per constraint data.
   MotionWorld::ConstraintContainer::Iterator itC;
   MotionWorld::ConstraintContainer::Iterator endC = _world->constraints().end();

   for( itC = _world->constraints().begin(); itC != endC; ++itC )
   {
      (*itC)->prePositionStep( step );
   }

   // Solve constraints.
   int nbConstraits;
   int i = 0;
   do
   {
      // Contact constraints.
      nbConstraits = 0;

      for( it = _contacts.begin(); it != end; ++it )
      {
         CollisionInfo::Contact& contact = *(*it)->_contact;

         // Compute the predicted distance between contact points.
         const Vec3f& posA = contact.worldPositionA();
         const Vec3f& posB = contact.worldPositionB();
         const Vec3f& n    = contact.worldNormal();

         // 1. Compute future positions.
         Vec3f predA = (*it)->_bodyA->lookAhead( posA, step );
         Vec3f predB = (*it)->_bodyB->lookAhead( posB, step );
         Vec3f d = predA - predB;

         // 2. Compute future contact normal.
         Vec3f predN = contact.worldNormal();

         // 3. Distance.
         float dn = -( d.dot(predN) );

         // Check if the constraint is satisfied.
         if( CGM::abs( dn ) > maxDistance )
         {
            ++nbConstraits;

            // Compute correction impulse.
            float p = contact._normK * dn / (float)step;
               
            // Clamp impulse.
            if( -p > contact._p )
            {
               p = -contact._p;
               contact._p = 0.0f;
            }
            else
            {
               contact._p += p;
            }

            // Apply impulse.
            (*it)->_bodyA->applyImpulse( n*p, posA );
            (*it)->_bodyB->applyImpulse( n*-p, posB );

            // Friction.
            predA = (*it)->_bodyA->lookAhead( posA, step );
            predB = (*it)->_bodyB->lookAhead( posB, step );
            d = predA - predB;
            dn = -( d.dot(predN) );

            // 1. Tangential distance.
            Vec3f dt = d+dn*predN;
            float dtLen = dt.length();

            if( dtLen > 0.000001f )
            {
               // 2. Tangent.
               Vec3f tan = dt * ( 1.0f / dtLen );
#if 0
               // 3. Friction impulse.
               float tp = -CGM::abs(p) * (*it)->_bodyA->friction( *(*it)->_bodyB );

               // 4. Max friction impulse.
               float tpMax = -dtLen / (tan.dot( contact._k*tan ) * (float)step );

               // 5. Apply impulse.
               if( tpMax > tp )
               {
                  tp = tpMax;
               }
#else
               float tp = -dtLen / (tan.dot( contact._k*tan ) * (float)step ) *
                  (*it)->_bodyA->friction( *(*it)->_bodyB );

#endif               
               (*it)->_bodyA->applyImpulse( tan*tp, posA );
               (*it)->_bodyB->applyImpulse( tan*-tp, posB );
            }
         }
      }

      // General constraints.
      for( itC = _world->constraints().begin(); itC != endC; ++itC )
      {
         if( (*itC)->solvePosition( step ) )
         {
            ++nbConstraits;
         }
      }

   } while( nbConstraits && (++i < maxLoopCount) );
}

//------------------------------------------------------------------------------
//!
void
ImpulseSolver::solveVelocities()
{
   const int maxLoopCount = 10;

   // Contact constraints should have relative velocity of zero!

   // Precompute per constraint data.
   MotionWorld::ConstraintContainer::Iterator itC;
   MotionWorld::ConstraintContainer::Iterator endC = _world->constraints().end();

   for( itC = _world->constraints().begin(); itC != endC; ++itC )
   {
      (*itC)->preVelocitiesStep();
   }
   
   // Precompute per contact data.
   Vector< MotionWorld::CollisionConstraint* >::Iterator it;
   Vector< MotionWorld::CollisionConstraint* >::Iterator end = _contacts.end();
   
   // Solve constraints.
   int nbConstraits;
   int i = 0;
   do
   {
      nbConstraits = 0;
      
      for( it = _contacts.begin(); it != end; ++it )
      {
         CollisionInfo::Contact& contact = *(*it)->_contact;

         // Compute the predicted distance between contact points.
         const Vec3f& posA = contact.worldPositionA();
         const Vec3f& posB = contact.worldPositionB();
         const Vec3f& n    = contact.worldNormal();
         
         // Compute normal relative velocity.
         Vec3f ua = (*it)->_bodyA->velocityPrev( posA );
         Vec3f ub = (*it)->_bodyB->velocityPrev( posB );
         float nRelVel = ( ua - ub ).dot( n );
         
         // Do we have a difference in velocity?
         if( CGM::abs( nRelVel ) > 0.001f )
         {
            ++nbConstraits;

            // Compute correction impulse.
            
            // 1. Impulse.
            float p = -nRelVel * contact._normK;

            // 2. Apply impulse.
            (*it)->_bodyA->applyImpulsePrev( n*p, posA );
            (*it)->_bodyB->applyImpulsePrev( n*-p, posB );
         }
      }

      // General constraints.
      for( itC = _world->constraints().begin(); itC != endC; ++itC )
      {
         if( (*itC)->solveVelocities() )
         {
            ++nbConstraits;
         }
      }

   } while( nbConstraits && (++i < maxLoopCount) );
}

NAMESPACE_END
