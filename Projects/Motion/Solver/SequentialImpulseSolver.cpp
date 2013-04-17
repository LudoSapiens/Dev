/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Motion/Solver/SequentialImpulseSolver.h>
#include <Motion/World/MotionWorld.h>


NAMESPACE_BEGIN

/*==============================================================================
   CLASS SequentialImpulseSolver
==============================================================================*/

//------------------------------------------------------------------------------
//!
SequentialImpulseSolver::SequentialImpulseSolver( MotionWorld* world ) :
   _world( world )
{
}

//------------------------------------------------------------------------------
//!
SequentialImpulseSolver::~SequentialImpulseSolver()
{
}

//------------------------------------------------------------------------------
//!
void
SequentialImpulseSolver::solveCollisions()
{
   // NOTHING TODO.
}

//------------------------------------------------------------------------------
//!
void
SequentialImpulseSolver::solveConstraints( double step )
{
   // Precompute per constraint data.
   MotionWorld::ConstraintContainer::Iterator itC;
   MotionWorld::ConstraintContainer::Iterator endC = _world->constraints().end();

   for( itC = _world->constraints().begin(); itC != endC; ++itC )
   {
      (*itC)->prePositionStep( step );
   }
   
   // Precompute per contact data.
   MotionWorld::CollisionContainer::Iterator it;
   MotionWorld::CollisionContainer::Iterator end = _world->collisions().end();

   for( it = _world->collisions().begin(); it != end; ++it )
   {
      CollisionInfo* info = (*it).info();
            
      for( uint c = 0; c < info->numContacts(); ++c )
      {
         CollisionInfo::Contact& contact = info->contact(c);
         if( contact.depth() >= 0.0f )
         {
            contact._k = (*it).bodyA()->computeK( contact.worldPositionA() ) +
                         (*it).bodyB()->computeK( contact.worldPositionB() );
            contact._normK = 1.0f / contact.worldNormal().dot( contact._k*contact.worldNormal() );
            
            // Restitution.
            const Vec3f& posA = contact.worldPositionA();
            const Vec3f& posB = contact.worldPositionB();
            const Vec3f& n    = contact.worldNormal();

            // 1. Compute normal relative velocity.
            Vec3f va = (*it).bodyA()->velocity( posA );
            Vec3f vb = (*it).bodyB()->velocity( posB );
            float vn = ( va - vb ).dot( n );
            
            // 2. Compute restitution factor.
            contact._restitution = -vn * (*it).bodyA()->restitution( *(*it).bodyB() );
            if( contact._restitution < 0.0f )
            {
               contact._restitution = 0.0f;
            }
            
            // Warm starting by applying the previous impulse.
            (*it).bodyA()->applyImpulse( n*contact._p, posA );
            (*it).bodyB()->applyImpulse( -n*contact._p, posB );
            
            contact._fp = 0.0f;
         }
      }
   }
   
   
   // Solve contact constraints.
   int loop = 0;
   int maxLoopCount = 10;
   int numConstraints = 0;
   do
   {
      
      for( it = _world->collisions().begin(); it != end; ++it )
      {
         CollisionInfo* info = (*it).info();
         
         for( uint c = 0; c < info->numContacts(); ++c )
         {
            CollisionInfo::Contact& contact = info->contact(c);
            if( contact.depth() >= 0.0f )
            {
               const Vec3f& posA = contact.worldPositionA();
               const Vec3f& posB = contact.worldPositionB();
               const Vec3f& n    = contact.worldNormal();

               // Collision impulse.
               
               // 1. Compute normal relative velocity.
               Vec3f va = (*it).bodyA()->velocity( posA );
               Vec3f vb = (*it).bodyB()->velocity( posB );
               float vn = ( va - vb ).dot( n );
               
               // 2. Compute impulse.
               float depth = contact.depth();
               if( contact._restitution*(float)step > depth )
               {
                  depth = 0.0f;
               }
               
               float depthCorrection = 0.3f * depth / (float)step;
               float velCorrection = contact._restitution - vn;
               float nImpulse = contact._normK * ( velCorrection + depthCorrection );
               
               // 3. Clamp impulse.
               float prevImpulse = contact._p;
               contact._p = CGM::max( prevImpulse + nImpulse, 0.0f );
               nImpulse = contact._p - prevImpulse;
               
               // 4. Apply impulse.
               (*it).bodyA()->applyImpulse( n*nImpulse, posA );
               (*it).bodyB()->applyImpulse( n*-nImpulse, posB );
               
               // Friction impulse.
               
               // 1. Tangential relative velocity.
               va = (*it).bodyA()->velocity( posA );
               vb = (*it).bodyB()->velocity( posB );
               vn = ( va - vb ).dot( n );
               
               Vec3f tRelVel = ( va - vb ) - n*vn;
               float tvelLen = tRelVel.length();

               if( tvelLen > 0.0000001 && contact._p > 0.0f )
               {
                  // 2. Tangent.
                  Vec3f tan = tRelVel * ( 1.0f / tvelLen );
                  
                  // 3. Friction impulse.
                  float fp = -tvelLen / tan.dot( contact._k*tan );

                  // 4. Max friction impulse.
                  float maxFriction = contact._p * (*it).bodyA()->friction( *(*it).bodyB() );
                  
                  // 5. Clamp impulse.
                  float prevFrictionImpulse = contact._fp;
                  contact._fp += fp;
                  contact._fp  = CGM::clamp( contact._fp, -maxFriction, maxFriction );
                  fp = contact._fp - prevFrictionImpulse;
                  
                  // 6. Apply impulse.
                  (*it).bodyA()->applyImpulse( tan*fp, posA );
                  (*it).bodyB()->applyImpulse( tan*-fp, posB );
               }
            }
         }
      }
      
      for( itC = _world->constraints().begin(); itC != endC; ++itC )
      {
         if( (*itC)->solvePosition( step ) )
         {
            ++numConstraints;
         }
      }
   
   } while( numConstraints && (++loop < maxLoopCount) );
}

//------------------------------------------------------------------------------
//!
void
SequentialImpulseSolver::solveVelocities()
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

   // Solve constraints.
   int nbConstraits;
   int i = 0;
   do
   {
      nbConstraits = 0;

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
