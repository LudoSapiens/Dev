/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Motion/Attractor/GravitationalAttractor.h>
#include <Motion/World/RigidBody.h>

#include <Base/Dbg/DebugStream.h>


/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_att, "Attractor" );

UNNAMESPACE_END



NAMESPACE_BEGIN

/*==============================================================================
   CLASS GravitationalAttractor
==============================================================================*/

//------------------------------------------------------------------------------
//!
GravitationalAttractor::GravitationalAttractor() :
   Attractor(), _threshold(0.0f)
{
}

//------------------------------------------------------------------------------
//!
GravitationalAttractor::GravitationalAttractor( const float threshold ) :
   Attractor(), _threshold(threshold)
{
}

//------------------------------------------------------------------------------
//!
GravitationalAttractor::~GravitationalAttractor()
{
}

//------------------------------------------------------------------------------
//!
void 
GravitationalAttractor::addForce( const Vector< RCP<RigidBody> >& bodies )
{
   DBG_BLOCK( os_att, "GravitationalAttractor::addForce" );

   for( uint b = 0; b < bodies.size(); ++b )
   {
      DBG_MSG( os_att, "Body[" << b << "]: mass=" << bodies[b]->mass() << " position=" << bodies[b]->position() );
   }

   DBG_MSG( os_att, "Method 1" );
   // Method 1:
   //   Compute everything as separate pairs, O(n^2).
   // 1. For every pair of bodies, calculate and apply the gravitational force.
   for( uint a = 0; a < bodies.size(); ++a )
   {
      for( uint b = a+1; b < bodies.size(); ++b )
      {
         Vec3f forceAtoB = bodies[b]->position() - bodies[a]->position();
         float rSq = forceAtoB.sqrLength();
         float GMm_rr = Constants::G * bodies[a]->mass() * bodies[b]->mass() / rSq;
         DBG_MSG( os_att, "Ma:" << bodies[a]->mass() << " Pa: " << bodies[a]->position() );
         DBG_MSG( os_att, "Mb:" << bodies[b]->mass() << " Pb: " << bodies[b]->position() );
         DBG_MSG( os_att, "DistSq:" << rSq );
         DBG_MSG( os_att, "GMm_rr: " << GMm_rr );
         if( GMm_rr > threshold() )
         {
            forceAtoB.rescale(GMm_rr);
            bodies[a]->addForce( forceAtoB );
            bodies[b]->addForce( -forceAtoB );
            DBG_MSG( os_att, "[" << a << "]: " << forceAtoB << "  [" << b << "]: " << -forceAtoB );
         }
         else
         {
            DBG_MSG( os_att, "Under threshold: ignored." );
         }
      }
   }
   for( uint b = 0; b < bodies.size(); ++b )
   {
      DBG_MSG( os_att, ">>TotalForce[" << b << "]: " << bodies[b]->totalForce() );
   }
   
#if 0
   DBG_MSG( os_att, "--------" );
   DBG_MSG( os_att, "Method 2" );
   // Method 2:
   //   Consider the rest of the universe as one big point w/mass, O(n).

   // 1. Accumulate mass and position.
   float totalMass = 0.0f;
   Vec3f centerOfMass(0.0f);
   for( uint b = 0; b < bodies.size(); ++b )
   {
      totalMass += bodies[b]->mass();
      centerOfMass += bodies[b]->position() * bodies[b]->mass();
   }
   DBG_MSG( os_att, "TotalMass: " << totalMass << " CenterOfMass: " << centerOfMass << " (" << centerOfMass/totalMass << ")" );
   // Note: the center of mass is actually scaled with the masses.
   //centerOfMass *= (1.0f/totalMass);

   // 2. Apply force of rest of universe over current body.
   for( uint b = 0; b < bodies.size(); ++b )
   {
      Vec3f residualCoM = centerOfMass - bodies[b]->position() * bodies[b]->mass();
      float residualMass = totalMass - bodies[b]->mass();
      residualCoM *= (1.0f/residualMass);
      Vec3f force = residualCoM - bodies[b]->position();
      float rSq = force.sqrLength();
      float GMm_rr = Constants::G * residualMass * bodies[b]->mass() / rSq;
      DBG_MSG( os_att, "Mu:" << residualMass << " Pu: " << residualCoM );
      DBG_MSG( os_att, "Mb:" << bodies[b]->mass() << " Pb: " << bodies[b]->position() );
      DBG_MSG( os_att, "DistSq:" << rSq );
      DBG_MSG( os_att, "GMm_rr: " << GMm_rr );
      if( GMm_rr > threshold() )
      {
         force.rescale(GMm_rr);
         //bodies[b]->addForce( force );
         DBG_MSG( os_att, "[" << b << "]: " << force );
      }
      else
      {
         DBG_MSG( os_att, "Under threshold: ignored." );
      }
   }
#endif
}

NAMESPACE_END
