/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Motion/World/RigidBody.h>


NAMESPACE_BEGIN

/*==============================================================================
   CLASS RigidBody
==============================================================================*/

//------------------------------------------------------------------------------
//!
RigidBody::RigidBody( Type type, void* userData ) :
   _type( type ),
   _userData( userData ),
   _world( NULL ),
   _friction( 0.5f ),
   _restitution( 0.5f ),
   _mass( 1.0f ),
   _invMass( 1.0f ),
   _interpolatedRef( Reff::identity() ),
   _linearVelocity( 0.0f, 0.0f, 0.0f ),
   _angularVelocity( 0.0f, 0.0f, 0.0f ),
   _totalForce( 0.0f, 0.0f, 0.0f ),
   _totalTorque( 0.0f, 0.0f, 0.0f ),
   _inertiaTensor( Mat3f::identity() ),
   _invInertiaTensor( Mat3f::identity() ),
   _prevPos( 0.0f ),
   _simRef( Reff::identity() ),
   _flags(RIGID_BODY_FLAGS_ACTIVE),
   _collisionCats( 0x01 ),
   _collisionMask( ~0x0 ),
   _callbackMask( ~0x0 ),
   _attractionCats( ~0x0 )
{
   mass( _mass );
   // FIXME: temp
   //updateRefDerivedData();
}

//------------------------------------------------------------------------------
//!
RigidBody::~RigidBody()
{
}

//------------------------------------------------------------------------------
//!
void 
RigidBody::referential( const Reff& ref )
{
   _prevPos = ref.position();
   _interpolatedRef = ref;
   _simRef = ref;
   updateRefDerivedData();
}

//------------------------------------------------------------------------------
//!
void 
RigidBody::position( const Vec3f& pos )
{
   _prevPos = pos;
   _interpolatedRef.position( pos );
   _simRef.position( pos );
   updateRefDerivedData();
}

//------------------------------------------------------------------------------
//!
void 
RigidBody::orientation( const Quatf& orient )
{
   _interpolatedRef.orientation( orient );
   _simRef.orientation( orient );
   updateRefDerivedData();
}

//------------------------------------------------------------------------------
//!
void 
RigidBody::linearVelocity( const Vec3f& velocity )
{
   _linearVelocity = velocity;
}

//------------------------------------------------------------------------------
//!
void 
RigidBody::angularVelocity( const Vec3f& velocity )
{
   _angularVelocity = velocity;
}

//------------------------------------------------------------------------------
//!
void
RigidBody::applyForces( double step )
{
   if( !active() ) return;
   _linearVelocity  += _totalForce * _invMass * (float)step;
   _angularVelocity +=  _invWorldInertiaTensor * (_totalTorque * (float)step);

   // Clear forces.
   _totalForce  = Vec3f::zero();
   _totalTorque = Vec3f::zero();
}

//------------------------------------------------------------------------------
//!
void
RigidBody::applyVelocities( double step )
{
   if( !active() ) return;

   _prevPos = _simRef.position();
   _prevInvWorldInertiaTensor = _invWorldInertiaTensor;

   Vec3f pos( _simRef.position() + _linearVelocity * (float)step );
   Quatf orient( _simRef.orientation() );

   float avLen = _angularVelocity.length();
   if( avLen > 0.0f )
   {
      Quatf drot = Quatf::axisAngle( _angularVelocity / avLen, avLen*(float)step );
      orient = ( drot * orient ).normalize();
   }

   _simRef = Reff( orient, pos );

   updateRefDerivedData();
}

//------------------------------------------------------------------------------
//!
void
RigidBody::updateRefDerivedData()
{
   // Update inertia tensor.
   Mat3f worldTransform( _simRef.orientation().toMatrix3() );
   _invWorldInertiaTensor = worldTransform * _invInertiaTensor * worldTransform.getTransposed();
}

//------------------------------------------------------------------------------
//!
Mat3f
RigidBody::computeK( const Vec3f& p ) const
{
   if( _type == DYNAMIC )
   {
      Vec3f r = p - _simRef.position();
      float j1 = _invWorldInertiaTensor._00;
      float j2 = _invWorldInertiaTensor._01;
      float j3 = _invWorldInertiaTensor._02;
      float j4 = _invWorldInertiaTensor._10;
      float j5 = _invWorldInertiaTensor._11;
      float j6 = _invWorldInertiaTensor._12;
      float j7 = _invWorldInertiaTensor._20;
      float j8 = _invWorldInertiaTensor._21;
      float j9 = _invWorldInertiaTensor._22;

      float m = 1.0f / _mass;

      float xx = r.x*r.x;
      float xy = r.x*r.y;
      float xz = r.x*r.z;
      float yy = r.y*r.y;
      float yz = r.y*r.z;
      float zz = r.z*r.z;

      return Mat3f(
         zz*j5    - yz*(j6 + j8)  + yy*j9 + m,
         -(zz*j4) + xz*j6 + yz*j7 - xy*j9,
         yz*j4    - xz*j5 - yy*j7 + xy*j8,

         -(zz*j2) + yz*j3 + xz*j8 - xy*j9,
         zz*j1    - xz*(j3 + j7)  + xx*j9 + m,
         -(yz*j1) + xz*j2 + xy*j7 - xx*j8,

         yz*j2    - yy*j3 - xz*j5 + xy*j6,
         -(yz*j1) + xy*j3 + xz*j4 - xx*j6,
         yy*j1    - xy*(j2 + j4)  + xx*j5 + m
      );
   }
   else
   {
      return Mat3f::zero();
   }
}

//------------------------------------------------------------------------------
//!
Mat3f
RigidBody::computeL() const
{
   if( _type == DYNAMIC )
   {
      return _invWorldInertiaTensor;
   }
   else
   {
      return Mat3f::zero();
   }
}

//------------------------------------------------------------------------------
//!
void 
RigidBody::simReferential( const Reff& ref )
{
   _simRef = ref;
   updateRefDerivedData();
}

//------------------------------------------------------------------------------
//!
void 
RigidBody::simPosition( const Vec3f& pos )
{
   _simRef.position( pos );
   updateRefDerivedData();
}

//------------------------------------------------------------------------------
//!
void 
RigidBody::simOrientation( const Quatf& orient )
{
   _simRef.orientation( orient );
   updateRefDerivedData();
}

NAMESPACE_END
