/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Motion/Solver/NextSolver.h>
#include <Motion/World/MotionWorld.h>


float eps = 0.00001f;

NAMESPACE_BEGIN

/*==============================================================================
   CLASS NextSolver
==============================================================================*/

//------------------------------------------------------------------------------
//!
NextSolver::NextSolver( MotionWorld* world ) :
   _world( world )
{
}

//------------------------------------------------------------------------------
//!
NextSolver::~NextSolver()
{
}

//------------------------------------------------------------------------------
//!
bool
NextSolver::collisionResponse()
{
   const float maxVelDiff       = 0.001f;
   const float restingThreshold = 0.8f;
   const uint maxLoop           = 10;
   
   Vector<bool> sticking;
   Vector<MotionWorld::CollisionConstraint*> cc;
   MotionWorld::CollisionConstraintContainer::Iterator it;
   MotionWorld::CollisionConstraintContainer::Iterator end = _world->collisionConstraints().end();

   // Precompute per contact data.
   for( it = _world->collisionConstraints().begin(); it != end; ++it )
   {
      CollisionInfo::Contact& c = *(*it)._contact;
      
      const Vec3f& posA = c.worldPositionA();
      const Vec3f& posB = c.worldPositionB();
      const Vec3f& n    = c.worldNormal();

      // Compute normal relative velocity.
      Vec3f ua    = (*it)._bodyA->velocity( posA );
      Vec3f ub    = (*it)._bodyB->velocity( posB );
      Vec3f urel  = ua-ub;
      float ureln = urel.dot( n );
      Vec3f urelt = urel-n*ureln;
      
      if( ureln > 0.0f )
      {
         // Nothing!
      }
      else if( ureln*ureln < restingThreshold )
      {
         // Nothing!
      }
      else
      {
         c._nRelVel = -ureln*(*it)._bodyA->restitution( *(*it)._bodyB );
         cc.pushBack( it );
         
         if( urelt.sqrLength() <= eps )
         {
            sticking.pushBack( true );
         }
         else
         {
            sticking.pushBack( false );
         }
      }
      
      c._k     = (*it)._bodyA->computeK( posA ) + (*it)._bodyB->computeK( posB );
      c._normK = 1.0f / n.dot( c._k*n );
      c._p     = 0.0f;
      c._fp    = 0.0f;
   }
   
   if( cc.empty() )
   {
      return false;
   }
   
   
   bool fix = true;
   uint loop = 0;
   
   while( fix && loop < maxLoop )
   {
      ++loop;
      fix = false;
      
      for( uint k = 0; k < cc.size(); ++k )
      {
         CollisionInfo::Contact& c = *cc[k]->_contact;
         
         const Vec3f& posA = c.worldPositionA();
         const Vec3f& posB = c.worldPositionB();
         const Vec3f& n    = c.worldNormal();
         
         Vec3f ua    = cc[k]->_bodyA->velocity( posA );
         Vec3f ub    = cc[k]->_bodyB->velocity( posB );
         float du    = c._nRelVel - (ua-ub).dot( n );
         
         float p = 0.0f;
      
         // Collision.
         if( du > maxVelDiff )
         {
            float d = n.dot( c._k*n );
            p       = du;
            if( CGM::abs(d) > eps )
            {
               p = p/d;
            }
            
            // Apply impulse.
            cc[k]->_bodyA->applyImpulse( n*p, posA );
            cc[k]->_bodyB->applyImpulse( n*-p, posB );

            // Accumulate impulse.
            c._p += p;
            
            fix = true;
         }
         else if( (du < -maxVelDiff) && (c._p > eps) )
         {
            float d = n.dot( c._k*n );
            p       = du;
            if( CGM::abs(d) > eps )
            {
               p = p/d;
            }
            
            // Clamp impulse.
            if( -p > c._p )
            {
               p = -c._p;
            }
            
            // Apply impulse.
            cc[k]->_bodyA->applyImpulse( n*p, posA );
            cc[k]->_bodyB->applyImpulse( n*-p, posB );

            // Accumulate impulse.
            c._p += p;
            
            fix = true;
         }
         
         // Friction.
         ua          = cc[k]->_bodyA->velocity( posA );
         ub          = cc[k]->_bodyB->velocity( posB );
         Vec3f urel  = ua-ub;
         float ureln = urel.dot( n );
         Vec3f urelt = urel-n*ureln;
         
         float friction = cc[k]->_bodyA->friction( *cc[k]->_bodyB );
         if( sticking[k] &&  (c._fp*c._fp <= friction*friction*c._p*c._p) )
         {
            sticking[k] = true;
         }
         else
         {
            sticking[k] = false;
         }
         
         if( sticking[k] && friction > 0.0f )
         {
            if( urelt.sqrLength() > maxVelDiff*maxVelDiff )
            {
               Vec3f tan = urelt.getNormalized();
               float d   = tan.dot( c._k*tan );
               
               Vec3f fp = -urelt;
               if( CGM::abs(d) > eps )
               {
                  fp = fp*(1.0f/d);
               }
               
               // Apply impulse.
               cc[k]->_bodyA->applyImpulse( fp, posA );
               cc[k]->_bodyB->applyImpulse( -fp, posB );
               
               fix = true;
            }
         }
         else if( fix && friction > 0.0f )
         {
            Vec3f tan   = urelt;
            float tl2   = tan.sqrLength();
               
            if( tl2 > eps )
            {
               tan /= CGM::sqrt(tl2);
                  
               float fp = -friction*p;
                  
               float d     = tan.dot(c._k*tan);
               float fpMax = -tan.dot(urelt);
               if( CGM::abs(d) > eps )
               {
                  fpMax = fpMax/d;
               }
                  
               if( fpMax > fp )
               {
                  sticking[k] = true;
                  fp = fpMax;
               }
                  
               // Accumulate impulse.
               c._fp += fp;
                  
               // Apply impulse.
               cc[k]->_bodyA->applyImpulse( tan*fp, posA );
               cc[k]->_bodyB->applyImpulse( tan*-fp, posB );
            }
            else
            {
               sticking[k] = true;
            }
         }
      }
      
      // TODO
      // joints
   }

   return true;
}

//------------------------------------------------------------------------------
//!
void
NextSolver::solveCollisions()
{
   const uint maxLoop = 10;
   const float restingThreshold = 0.8f;
   
   bool fix = true;
   uint loop = 0;
   while( fix && loop < maxLoop )
   {
      ++loop;
      fix = false;
      
      if( collisionResponse() )
      {
         fix = true;
      }
   }
   
   
   // Create contact constraints.
   _contacts.clear();
   MotionWorld::CollisionConstraintContainer::Iterator it;
   MotionWorld::CollisionConstraintContainer::Iterator end = _world->collisionConstraints().end();

   for( it = _world->collisionConstraints().begin(); it != end; ++it )
   {
      CollisionInfo::Contact& c = *(*it)._contact;
      
      const Vec3f& posA = c.worldPositionA();
      const Vec3f& posB = c.worldPositionB();
      const Vec3f& n    = c.worldNormal();

      // Compute normal relative velocity.
      Vec3f ua    = (*it)._bodyA->velocity( posA );
      Vec3f ub    = (*it)._bodyB->velocity( posB );
      Vec3f urel  = ua-ub;
      float ureln = urel.dot( n );
      
      if( (ureln > 0.0f) && !(ureln*ureln < restingThreshold) )
      {
         // Nothing!
      }
      else
      {
         _contacts.pushBack( it );
      }
   }
   
#if 0   
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
#endif   
}

//------------------------------------------------------------------------------
//!
void
NextSolver::solveConstraints( double step )
{   
   const uint maxLoop        = 10;
   const uint maxJointLoop   = 10;
   const uint maxContactLoop = 10;
   
   const float maxDistance = 0.001f; // 1 mm precision.
   
   // Precompute per contact data.
   Vector< MotionWorld::CollisionConstraint* >::Iterator it;
   Vector< MotionWorld::CollisionConstraint* >::Iterator end = _contacts.end();

   Vector<bool> sticking(_contacts.size());
   uint i;
   for( i = 0, it = _contacts.begin(); it != end; ++it, ++i )
   {
      (*it)->_contact->_p  = 0.0f;
      (*it)->_contact->_fp = 0.0f;
      
      CollisionInfo::Contact& contact = *(*it)->_contact;
      const Vec3f& posA = contact.worldPositionA();
      const Vec3f& posB = contact.worldPositionB();
      const Vec3f& n    = contact.worldNormal();
      
      // Do we have friction?
      Vec3f ua    = (*it)->_bodyA->velocity( posA );
      Vec3f ub    = (*it)->_bodyB->velocity( posB );
      Vec3f urel  = ua-ub;
      Vec3f ureln = n*( urel ).dot( n );
      Vec3f urelt = urel - ureln;

      if( urelt.sqrLength() <= eps )
      {
         sticking[i] = true;
      }
      else
      {
         sticking[i] = false;
      }
   }

   // Precompute per constraint data.
   MotionWorld::ConstraintContainer::Iterator itC;
   MotionWorld::ConstraintContainer::Iterator endC = _world->constraints().end();

   for( itC = _world->constraints().begin(); itC != endC; ++itC )
   {
      (*itC)->prePositionStep( step );
   }
   
#if 1
   bool fix = true;
   uint loop = 0;
   while( fix && loop < maxLoop )
   {
      ++loop;
      fix = false;
      
      // General constraints.
      if( loop <= maxJointLoop )
      {
         for( itC = _world->constraints().begin(); itC != endC; ++itC )
         {
            if( (*itC)->solvePosition( step ) )
            {
               fix = true;
            }
         }
      }
      // Contact constraints.
      if( loop <= maxContactLoop )
      {
         for( i = 0, it = _contacts.begin(); it != end; ++it, ++i )
         {
            bool cfix = false;
            CollisionInfo::Contact& contact = *(*it)->_contact;
            
            // Compute the predicted distance between contact points.
            const Vec3f& posA = contact.worldPositionA();
            const Vec3f& posB = contact.worldPositionB();
            const Vec3f& n    = contact.worldNormal();

            // 1. Compute future positions.
            Vec3f predA = (*it)->_bodyA->lookAhead( posA, step );
            Vec3f predB = (*it)->_bodyB->lookAhead( posB, step );
            Vec3f d     = predB - predA;

            // 2. Compute future contact normal.
            // fixme!
            Vec3f predN = contact.worldNormal();
            
            // 3. Distance.
            float dn = -( d.dot(predN) );
            float l  = 0.00001f - dn;
            float p  = 0.0f;

            if( l > maxDistance )
            {
               // Compute correction impulse.
               p = contact._normK * l / (float)step;
               
               // Accumulate impulse.
               contact._p += p;
               
               // Apply impulse.
               (*it)->_bodyA->applyImpulse( n*p, posA );
               (*it)->_bodyB->applyImpulse( n*-p, posB );
               cfix = true;
            }
            else if( (l < -maxDistance) && (contact._p > eps) )
            {
               // Compute correction impulse.
               p = contact._normK * l / (float)step;
               
               // Clamp impulse.
               if( -p > contact._p )
               {
                  p -= contact._p;
               }
               
               // Accumulate impulse.
               contact._p += p;
               
               // Apply impulse.
               (*it)->_bodyA->applyImpulse( n*p, posA );
               (*it)->_bodyB->applyImpulse( n*-p, posB );
               cfix = true;
            }
            
            // Friction.
            float friction = (*it)->_bodyA->friction( *(*it)->_bodyB );
            if( sticking[i] &&  (contact._fp*contact._fp <= friction*friction*contact._p*contact._p) )
            {
               sticking[i] = true;
            }
            else
            {
               sticking[i] = false;
            }
            
            if( sticking[i] )
            {
               predA = (*it)->_bodyA->lookAhead( posA, step );
               predB = (*it)->_bodyB->lookAhead( posB, step );
               predN = contact.worldNormal(); // FIXME
               d     = predB-predA;
            
               float deltaN = d.dot(n);
               Vec3f deltaT = d - (n*deltaN);
               float l2     = deltaT.sqrLength();
               if( l2 > maxDistance )
               {
                  Vec3f tan = deltaT;
                  float l = CGM::sqrt(l2);
                  tan /= l;

                  float d  = tan.dot(contact._k * tan);
                  float tp = (float)(l/step);
                  if( CGM::abs(d) > eps )
                     tp = tp/d;

                  // Apply impulse.
                  (*it)->_bodyA->applyImpulse( tan*tp, posA );
                  (*it)->_bodyB->applyImpulse( tan*-tp, posB );
                  cfix = true;
               }
            }
            else if( cfix )
            {
               Vec3f ua    = (*it)->_bodyA->velocity( posA );
               Vec3f ub    = (*it)->_bodyB->velocity( posB );
               Vec3f urel  = ua-ub;
               Vec3f urelt = urel - n*( urel ).dot( n );
               Vec3f tan   = urelt;
               float tl2   = tan.sqrLength();
               
               if( tl2 > eps )
               {
                  tan /= CGM::sqrt(tl2);
                  
                  float fp = -friction*p;
                  
                  float d     = tan.dot(contact._k*tan);
                  float fpMax = 0.0f;
                  if( CGM::abs(d) > eps )
                  {
                     fpMax = -tan.dot(urelt)/d;
                  }
                  
                  if( fpMax > fp )
                  {
                     sticking[i] = true;
                     fp = fpMax;
                  }
                  
                  // Accumulate impulse.
                  contact._fp += fp;
                  
                  // Apply impulse.
                  (*it)->_bodyA->applyImpulse( tan*fp, posA );
                  (*it)->_bodyB->applyImpulse( tan*-fp, posB );
               }
               else
               {
                  sticking[i] = true;
               }
            }
            
            fix |= cfix;
         }
      }
   }
   
#endif   
   
#if 0   
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
#if 0
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
#endif            
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

   } while( nbConstraits && (++i < maxLoop) );
   
#endif   
}

//------------------------------------------------------------------------------
//!
void
NextSolver::solveVelocities()
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
#if 1    
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
#endif
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
