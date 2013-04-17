/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <MotionBullet/World/RigidBody.h>
#include <MotionBullet/World/MotionWorld.h>

#include <BulletCollision/CollisionShapes/btCompoundShape.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>

#include <Base/Dbg/Defs.h>

NAMESPACE_BEGIN

inline void convert( const btTransform& src, Mat4f& dst )
{
   const   btVector3& origin = src.getOrigin();
   const btMatrix3x3& basis  = src.getBasis();
   const   btVector3& row0   = basis.getRow(0);
   const   btVector3& row1   = basis.getRow(1);
   const   btVector3& row2   = basis.getRow(2);
   dst = Mat4f(
      row0.x(), row0.y(), row0.z(), origin.x(),
      row1.x(), row1.y(), row1.z(), origin.y(),
      row2.x(), row2.y(), row2.z(), origin.z(),
          0.0f,     0.0f,     0.0f,       1.0f
   );
}

//------------------------------------------------------------------------------
//!
inline void convert( const Mat4f& src, btTransform& dst )
{
   dst.setOrigin( btVector3(src(0, 3), src(1, 3), src(2, 3)) );
   dst.getBasis().setValue(
      src(0, 0), src(0, 1), src(0, 2),
      src(1, 0), src(1, 1), src(1, 2),
      src(2, 0), src(2, 1), src(2, 2)
   );
}

//------------------------------------------------------------------------------
//!
inline void convert( const btTransform& src, Reff& dst )
{
   dst.scale( 1.0f );
   dst.position( src.getOrigin().x(), src.getOrigin().y(), src.getOrigin().z() );
   btQuaternion q( src.getRotation() );
   dst.orientation( Quatf( q.x(), q.y(), q.z(), q.w() ) );
}

//------------------------------------------------------------------------------
//!
inline void convert( const Reff& src, btTransform& dst )
{
   dst.setOrigin( btVector3( src.position().x, src.position().y, src.position().z ) );
   dst.setRotation( btQuaternion(
      src.orientation().x(),
      src.orientation().y(),
      src.orientation().z(),
      src.orientation().w()
   ) );
}

//------------------------------------------------------------------------------
//!
inline btVector3 convert( const Vec3f& v )
{
   return btVector3( v.x, v.y, v.z );
}

//------------------------------------------------------------------------------
//!
void eigenDecomposition( const Mat3f& a, Quatf& rot, Vec3f& diag )
{
   int maxSteps = 24;
   Quatf q      = Quatf::identity();

   int i;
   for( i = 0; i < maxSteps; ++i )
   {
      Mat3f Q = q.toMatrix3();
      //Mat3f D = Q * a * Q.getTransposed();
      Mat3f D = Q.getTransposed() * a * Q;
      Vec3f offd( D(1,2),D(0,2),D(0,1) ); // Elements not on the diagonal.
      int k   = offd.maxComponent();      // Index of largest element of offd.
      int k1  = (k+1)%3;
      int k2  = (k+2)%3;

      // Already a diagonal?
      if( offd(k) == 0.0f ) break;

      float thet = (D(k2,k2)-D(k1,k1))/(2.0f*offd(k));
      float sgn  = CGM::sign( thet );
      thet      *= sgn;
      float t    = sgn / (thet+((thet < 1e6f)?CGM::sqrt(thet*thet+1.0f):thet));
      float c    = 1.0f / CGM::sqrt( t*t+1.0f );

      // Reached machine precision.
      if( c == 1.0f ) break;

      // Jacobi rotation.
      Quatf jr( 0.0f, 0.0f, 0.0f, 0.0f );
      jr.vec()(k)  = sgn*CGM::sqrt( (1.0f-c)/2.0f ); // Using 1/2 angle identity sin(a/2) = sqrt((1-cos(a))/2).
      //jr.vec()(k) *= -1.0f; // Since our quat-to-matrix convention was for v*M instead of M*v.
      jr.w()       = CGM::sqrt( 1.0f-jr.vec()(k)*jr.vec()(k) );

      // Reached limits of floating precision.
      if( jr.w() == 1.0f ) break;

      q = q*jr;
      //q = jr*q;
      q.normalize();
   }
   rot = q;
   Mat3f Q = q.toMatrix3();
   //Mat3f D = Q * a * Q.getTransposed();
   Mat3f D = Q.getTransposed() * a * Q;
   diag.x = D(0,0);
   diag.y = D(1,1);
   diag.z = D(2,2);
}


/*==============================================================================
   CLASS RigidBody::MotionState
==============================================================================*/

class RigidBody::MotionState : public btMotionState
{
public:

   MotionState( void* userRef ) : _userRef( userRef )
   {
      _com.setIdentity();
   }

   virtual void getWorldTransform( btTransform& t ) const
   {
      Reff  ref;
      Mat4f mat;
      MotionWorld::getTransform( _userRef, ref, mat );
      convert( mat, t );
      t = t * _com;
   }

   virtual void setWorldTransform( const btTransform& t )
   {
      Reff  ref;
      Mat4f mat;
      btTransform tcom = t * _com.inverse();
      convert( tcom, ref );
      convert( tcom, mat );
      MotionWorld::setTransform( _userRef, ref, mat );
   }

   void*       _userRef;
   btTransform _com;
};

/*==============================================================================
   Shape properties
==============================================================================*/

//------------------------------------------------------------------------------
//! Base on code from Stan Melax.
void
RigidBody::computeShapeProperties(
   const float*    pos,
   int             stride,
   const uint32_t* indices,
   int             numTriangles,
   Reff&           centerOfMass,
   Vec3f&          tensor
)
{
   // Center of mass computation.
   Vec3f com(0.0f);
   float volume = 0.0f;

   const uint32_t* id = indices;
   for( int i = 0; i < numTriangles; ++i, id += 3 )
   {
      Vec3f v0( pos + id[0]*stride );
      Vec3f v1( pos + id[1]*stride );
      Vec3f v2( pos + id[2]*stride );
      Mat3f m( v0, v1, v2 );

      float det = m.determinant();
      com      += det * (v0+v1+v2);
      volume   += det;
   }

   if( volume > CGM::EqualityThreshold )  com /= volume*4.0f;

   // Compute inertia tensor matrice.
   Vec3f diag(0.0f);    // Accumulate matrix main diagonal integrals [x*x, y*y, z*z].
   Vec3f offd(0.0f);    // accumulate matrix off-diagonal  integrals [y*z, x*z, x*y].

   id = indices;
   volume = 0.0f;
   for( int i = 0; i < numTriangles; ++i, id +=3 )
   {
      Vec3f v0( pos + id[0]*stride );
      Vec3f v1( pos + id[1]*stride );
      Vec3f v2( pos + id[2]*stride );
      Mat3f m( v0-com, v1-com, v2-com );

      float d = m.determinant();
      volume += d;

      for( int j = 0; j < 3; ++j )
      {
         int j1=(j+1)%3;
         int j2=(j+2)%3;
         diag(j) += (m(0,j)*m(1,j) + m(1,j)*m(2,j) + m(2,j)*m(0,j) +
                     m(0,j)*m(0,j) + m(1,j)*m(1,j) + m(2,j)*m(2,j))*d;
         offd(j) += (m(0,j1)*m(1,j2)      + m(1,j1)*m(2,j2)      + m(2,j1)*m(0,j2) +
                     m(0,j1)*m(2,j2)      + m(1,j1)*m(0,j2)      + m(2,j1)*m(1,j2) +
                     m(0,j1)*m(0,j2)*2.0f + m(1,j1)*m(1,j2)*2.0f + m(2,j1)*m(2,j2)*2.0f)*d;
      }
   }
   if( volume > CGM::EqualityThreshold )
   {
      diag /= volume*10.0f;
      offd /= volume*20.0f;
   }

   Mat3f inertiaMat( diag.y+diag.z, -offd.z, -offd.y,
                     -offd.z, diag.x+diag.z, -offd.x,
                     -offd.y, -offd.x, diag.x+diag.y );

   Quatf rot;
   eigenDecomposition( inertiaMat, rot, tensor );
   centerOfMass.position( com );
   centerOfMass.orientation( rot );
}

/*==============================================================================
   CLASS RigidBody
==============================================================================*/

//------------------------------------------------------------------------------
//!
RigidBody::RigidBody( Type type, void* userRef ) :
   _type( type ),
   _world( 0 ),
   _totalForce( 0.0f ),
   _totalTorque( 0.0f ),
   _existsMask( 0x1 ),
   _sensesMask( 0x0 ),
   _attractionCats( ~0x0 )
{
   _motionState = new MotionState( userRef );

   _mass = (type == DYNAMIC) ? 1.0f : 0.0f;
   btRigidBody::btRigidBodyConstructionInfo cInfo(
      _mass,                         // Mass
      _motionState,                  // MotionState
      NULL,                          // Collisionshape
      type == DYNAMIC ? btVector3( 1.0, 1.0, 1.0 ) : btVector3( 0.0, 0.0, 0.0 )// Inertia tensor
   );
   cInfo.m_restitution = 0.5f;
   cInfo.m_friction    = 0.5f;

   _body = new btRigidBody( cInfo );
   _body->setUserPointer( this ); // Allows us to retrieve Motion's RigidBody elements in a Bullet callback.
   _body->setSleepingThresholds( 0.5f, 1.0f ); // Bullet defaults are 0.8f, 1.0f.

   switch( _type )
   {
      case DYNAMIC:
         _body->setCollisionFlags( 0 );
         _body->setActivationState( ACTIVE_TAG );
         break;
      case KINEMATIC:
         _body->setCollisionFlags( btCollisionObject::CF_KINEMATIC_OBJECT );
         _body->setActivationState( DISABLE_DEACTIVATION );
         break;
      case STATIC:
         _body->setCollisionFlags( btCollisionObject::CF_STATIC_OBJECT );
         _body->setActivationState( WANTS_DEACTIVATION );
         break;
   }
}

//------------------------------------------------------------------------------
//!
RigidBody::~RigidBody()
{
   delete _body;
   delete _motionState;
}

//------------------------------------------------------------------------------
//!
void
RigidBody::type( Type v )
{
   if( v == _type ) return;

   if( _world ) _world->bodyTypeChanged( this, v );
   _type = v;

   switch( _type )
   {
      case DYNAMIC:
         _body->setCollisionFlags( 0 );
         _body->setActivationState( ACTIVE_TAG );
         break;
      case KINEMATIC:
         _body->setCollisionFlags( btCollisionObject::CF_KINEMATIC_OBJECT );
         _body->setActivationState( DISABLE_DEACTIVATION );
         break;
      case STATIC:
         _body->setCollisionFlags( btCollisionObject::CF_STATIC_OBJECT );
         _body->setActivationState( WANTS_DEACTIVATION );
         break;
   }

   // Reset velocities.
   linearVelocity( Vec3f(0.0f) );
   angularVelocity( Vec3f(0.0f) );
   // Update body.
   btUpdate( true );
   _body->activate();
}

//------------------------------------------------------------------------------
//!
void
RigidBody::userRef( void* v )
{
   _motionState->_userRef = v;
}

//------------------------------------------------------------------------------
//!
void*
RigidBody::userRef() const
{
   return _motionState->_userRef;
}

//------------------------------------------------------------------------------
//!
void
RigidBody::friction( float friction )
{
   _body->setFriction( friction );
}

//------------------------------------------------------------------------------
//!
void
RigidBody::restitution( float restitution )
{
   _body->setRestitution( restitution );
}

//------------------------------------------------------------------------------
//!
void
RigidBody::mass( float mass )
{
   _mass = mass;
   btUpdateMass();
}

//------------------------------------------------------------------------------
//!
float
RigidBody::friction() const
{
   return _body->getFriction();
}

//------------------------------------------------------------------------------
//!
float
RigidBody::restitution() const
{
   return _body->getRestitution();
}

//------------------------------------------------------------------------------
//!
void
RigidBody::referential( const Reff& ref )
{
   btTransform t;
   convert( ref, t );
   _body->setCenterOfMassTransform( t*_motionState->_com );
}

//------------------------------------------------------------------------------
//!
Reff
RigidBody::referential() const
{
   btTransform t = _body->getCenterOfMassTransform()*_motionState->_com.inverse();
   Reff ref;
   convert( t, ref );
   return ref;
}

//------------------------------------------------------------------------------
//!
const Vec3f&
RigidBody::centerPosition() const
{
   return (const Vec3f&)_body->getCenterOfMassPosition();
}

//------------------------------------------------------------------------------
//!
Quatf
RigidBody::centerOrientation() const
{
   const btTransform& xform = _body->getCenterOfMassTransform();
   btQuaternion q = xform.getRotation();
   return Quatf( q.x(), q.y(), q.z(), q.getW() );
}

//------------------------------------------------------------------------------
//!
Mat4f
RigidBody::transform() const
{
   btTransform xform  = _body->getCenterOfMassTransform() * _motionState->_com.inverse();
   const btMatrix3x3& basis  = xform.getBasis();
   const btVector3&   row0   = basis.getRow( 0 );
   const btVector3&   row1   = basis.getRow( 1 );
   const btVector3&   row2   = basis.getRow( 2 );
   const btVector3&   origin = xform.getOrigin();
   Mat4f mat(
      row0.x(), row0.y(), row0.z(), origin.x(),
      row1.x(), row1.y(), row1.z(), origin.y(),
      row2.x(), row2.y(), row2.z(), origin.z(),
          0.0f,     0.0f,     0.0f,       1.0f
   );
   return mat;
}

//------------------------------------------------------------------------------
//!
void
RigidBody::linearVelocity( const Vec3f& velocity )
{
   _body->setLinearVelocity( convert( velocity ) );
}

//------------------------------------------------------------------------------
//!
void
RigidBody::angularVelocity( const Vec3f& velocity )
{
   _body->setAngularVelocity( convert( velocity ) );
}

//------------------------------------------------------------------------------
//!
const Vec3f&
RigidBody::linearVelocity() const
{
   return (const Vec3f&)_body->getLinearVelocity();
}

//------------------------------------------------------------------------------
//!
const Vec3f&
RigidBody::angularVelocity() const
{
   return (const Vec3f&)_body->getAngularVelocity();
}

//------------------------------------------------------------------------------
//!
void
RigidBody::addForce( const Vec3f& force )
{
   _body->activate();
   _totalForce += force;
   _body->applyCentralForce( convert( force ) );
}

//------------------------------------------------------------------------------
//!
void
RigidBody::addTorque( const Vec3f& torque )
{
   _body->activate();
   _totalTorque += torque;
   _body->applyTorque( convert( torque ) );
}

//------------------------------------------------------------------------------
//!
const Vec3f&
RigidBody::totalForce() const
{
   return _totalForce;
}

//------------------------------------------------------------------------------
//!
const Vec3f&
RigidBody::totalTorque() const
{
   return _totalTorque;
}

//------------------------------------------------------------------------------
//!
void
RigidBody::applyImpulse( const Vec3f& impulse, const Vec3f& relPos )
{
   _body->activate();
   _body->applyImpulse( convert( impulse ), convert( relPos ) );
}

//------------------------------------------------------------------------------
//!
void
RigidBody::applyImpulse( const Vec3f& impulse )
{
   _body->activate();
   _body->applyCentralImpulse( convert( impulse ) );
}

//------------------------------------------------------------------------------
//!
void
RigidBody::applyTorqueImpulse( const Vec3f& impulse )
{
   _body->activate();
   _body->applyTorqueImpulse( convert( impulse ) );
}

//------------------------------------------------------------------------------
//!
void
RigidBody::connect( MotionWorld* w )
{
   _world = w;
   _world->_world->addRigidBody( _body );
}

//------------------------------------------------------------------------------
//!
void
RigidBody::disconnect()
{
   _world->_world->removeRigidBody( _body );
   _world = 0;
}

//------------------------------------------------------------------------------
//!
void
RigidBody::shape( CollisionShape* shape )
{
   _shape = shape;
   if( shape )
   {
      _body->setCollisionShape( shape->bulletCollisionShape() );
   }
   else
   {
      _body->setCollisionShape( 0 );
   }

   btUpdate( true );
}

//------------------------------------------------------------------------------
//!
void
RigidBody::btUpdateMass()
{
   if( _shape.isNull() )
   {
      _body->setMassProps( 0.0f, btVector3( 0.0f, 0.0f, 0.0f ) );
      return;
   }

   if( _type == DYNAMIC )
   {
      btVector3 localInertia;
      _body->getCollisionShape()->calculateLocalInertia( mass(), localInertia );

      if( _shape->type() == CollisionShape::GROUP )
      {
         CollisionGroup* group = (CollisionGroup*)_shape.ptr();
         convert( group->centerOfMass(), _motionState->_com );
         localInertia = convert( group->inertiaTensor() * mass() );
      }
      else
      {
         _motionState->_com.setIdentity();
      }

      // Update rigid body position.
      btTransform t;
      _motionState->getWorldTransform(t);
      _body->setCenterOfMassTransform(t);

      _body->setMassProps( mass(), localInertia );
      return;
   }

   // Static bodies only.
   if( _shape->type() == CollisionShape::GROUP )
   {
      CollisionGroup* group = (CollisionGroup*)_shape.ptr();
      convert( group->centerOfMass(), _motionState->_com );
   }
   else
   {
      _motionState->_com.setIdentity();
   }

   // Update rigid body position.
   btTransform t;
   _motionState->getWorldTransform(t);
   _body->setCenterOfMassTransform(t);

   _body->setMassProps( 0.0f, btVector3( 0.0f, 0.0f, 0.0f ) );
}

//------------------------------------------------------------------------------
//!
void
RigidBody::btUpdate( bool updateMass )
{
   if( updateMass ) btUpdateMass();
   if( _world )
   {
      _world->_world->removeRigidBody( _body );
      _world->_world->addRigidBody( _body );
   }
}

NAMESPACE_END
