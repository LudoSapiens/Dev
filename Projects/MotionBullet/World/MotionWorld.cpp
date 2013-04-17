/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <MotionBullet/World/MotionWorld.h>

#include <MotionBullet/Constraint/CharacterConstraint.h>

#include <Base/Dbg/DebugStream.h>

#include <BulletCollision/BroadphaseCollision/btAxisSweep3.h>
#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h>
#include <BulletCollision/CollisionDispatch/btManifoldResult.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <LinearMath/btIDebugDraw.h>


/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_att, "MotionWorld" );

//-----------------------------------------------------------------------------
//!
inline void  add( Vector<float>& dst, const Vec3f& position, const Vec4f& color )
{
   dst.pushBack( position.x );
   dst.pushBack( position.y );
   dst.pushBack( position.z );
   dst.pushBack( color.x );
   dst.pushBack( color.y );
   dst.pushBack( color.z );
   dst.pushBack( color.w );
}

UNNAMESPACE_END

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

//-----------------------------------------------------------------------------
//!
inline btVector3  toBullet( const Vec3f& v )
{
   return btVector3( v.x, v.y, v.z );
}

//-----------------------------------------------------------------------------
//!
inline Vec3f  toVec3f( const btVector3& v )
{
   return Vec3f( v.x(), v.y(), v.z() );
}

//-----------------------------------------------------------------------------
//!
inline Vec4f  toVec4f( const btVector3& v, float a )
{
   return Vec4f( v.x(), v.y(), v.z(), a );
}

//------------------------------------------------------------------------------
//!
bool BulletContactAdded(
   btManifoldPoint&          cp,
   const btCollisionObject*  /*colObj0*/,
   int                       /*partId0*/,
   int                       /*index0*/,
   const btCollisionObject*  /*colObj1*/,
   int                       /*partId1*/,
   int                       /*index*/
)
{
   StdErr << "BCA:"
          << " " << cp.m_positionWorldOnA.x() << "," << cp.m_positionWorldOnA.y() << "," << cp.m_positionWorldOnA.z()
          << " " << cp.m_positionWorldOnB.x() << "," << cp.m_positionWorldOnB.y() << "," << cp.m_positionWorldOnB.z()
          << nl;
   return true;
}

//------------------------------------------------------------------------------
//!
void BulletNearCallback(
   btBroadphasePair&       /*collisionPair*/,
   btCollisionDispatcher&  /*dispatcher*/,
   btDispatcherInfo&       /*dispatchInf*/
)
{
   StdErr << "BNC" << nl;
}

/*==============================================================================
  CLASS ContactCache
==============================================================================*/
class ContactCache
{
public:
   /*----- types -----*/

   typedef Pair< RigidBody*, RigidBody* >  BodyPair;
   typedef Set< BodyPair >                 ContactContainer;
   typedef ContactContainer::ConstIterator ConstIterator;

   /*----- methods -----*/

   ContactCache() {}

   static inline BodyPair pair( RigidBody* a, RigidBody* b )
   {
      if( a <= b )  return BodyPair( a, b );
      else          return BodyPair( b, a );
   }

   bool  isNew( RigidBody* a, RigidBody* b );

   ConstIterator  outdatedBegin() const { return _previous.begin(); }
   ConstIterator  outdatedEnd() const   { return _previous.end(); }

   void  endFrame();

protected:

   /*----- data members -----*/

   ContactContainer  _current;  //!< The list of contacts for the current frame.
   ContactContainer  _previous; //!< The list of contacts for the previous frame.

};

//-----------------------------------------------------------------------------
//!
bool
ContactCache::isNew( RigidBody* a, RigidBody* b )
{
   BodyPair p = pair( a, b );
   // Remove the pair from _previous in order to only keep outdated pairs.
   if( _previous.removeCheck(p) )
   {
      _current.add(p);
      return false; // Was in previous.
   }
   else
   {
      return _current.addCheck(p);
   }
}

//------------------------------------------------------------------------------
//! Marks the end of a frame.
void
ContactCache::endFrame()
{
   // Clear previous contacts (normally handled by the user).
   _previous.clear();
   // Make the current contacts the previous for the next frame.
   _previous.swap( _current );
}

/*==============================================================================
   CLASS btWorld
==============================================================================*/

class btWorld:
   public btDiscreteDynamicsWorld
{
public:

   /*----- members -----*/

   btWorld(
      MotionWorld*              world,
      btDispatcher*             dispatcher,
      btBroadphaseInterface*    pairCache,
      btConstraintSolver*       constraintSolver,
      btCollisionConfiguration* collisionConfiguration
   ) :
      btDiscreteDynamicsWorld( dispatcher, pairCache, constraintSolver, collisionConfiguration ),
      _world( world )
   {}

   virtual void applyGravity()
   {
      // Overridden (called in each stepSimulation loop below).
   }

   virtual void clearForces()
   {
      // Overridden (called in each stepSimulation loop below).
      // Note: Might not be what we want...
   }

   virtual void internalSingleStepSimulation( btScalar timeStep )
   {
      // Apply attractors.
      btDiscreteDynamicsWorld::applyGravity();
      _world->handleAttractors();

      // Constraints pre-step.
      _world->handleConstraintsPreStep( timeStep );

      // Simulate the step.
      btDiscreteDynamicsWorld::internalSingleStepSimulation( timeStep );

      // Clear forces.
      btDiscreteDynamicsWorld::clearForces();
      //_world->clearForces();

      // Constraints post-step.
      _world->handleConstraintsPostStep( timeStep );

      handleCollisionCallback();
   }

   //virtual void applyGravity()
   //{
   //   btDiscreteDynamicsWorld::applyGravity();
   //
   //   // Apply attractors.
   //   for( uint i = 0; i < _world->_attractors.size(); ++i )
   //   {
   //      _world->_attractors[i]->addForce( _world->_rigidBodies );
   //   }
   //}

#if 0
   virtual void internalSingleStepSimulation( btScalar timeStep )
   {

      // Constraints...
      for( uint i = 0; i < _world->_constraints.size(); ++i )
      {
         _world->_constraints[i]->preStep( timeStep );
      }

      btDiscreteDynamicsWorld::internalSingleStepSimulation( timeStep );

      // Constraints...
      for( uint i = 0; i < _world->_constraints.size(); ++i )
      {
         _world->_constraints[i]->postStep( timeStep );
      }
   }
#endif

   void handleCollisionCallback();

   /*----- members -----*/

   MotionWorld* _world;
   ContactCache _contactCache;
};

//------------------------------------------------------------------------------
//! Iterates over all of the dispatcher's manifolds, and triggers a callback
//! when appropriate.
void
btWorld::handleCollisionCallback()
{
   btDispatcher* dispatcher = _world->_collisionDispatcher;
   int numManifolds         = dispatcher->getNumManifolds();

   // New contacts.
   for( int i = 0; i < numManifolds; ++i )
   {
      btPersistentManifold* manifold = dispatcher->getManifoldByIndexInternal( i );
      btCollisionObject* obj0        = (btCollisionObject*)manifold->getBody0();
      btCollisionObject* obj1        = (btCollisionObject*)manifold->getBody1();
      RigidBody* body0               = (RigidBody*)obj0->getUserPointer();
      RigidBody* body1               = (RigidBody*)obj1->getUserPointer();
      int numContacts                = manifold->getNumContacts();

      if( numContacts > 0 && _contactCache.isNew( body0, body1 ) )
      {
         CollisionPair pair( body0, body1 );
         // Check for collisions.
         if( _world->_collisionBeginCallback && RigidBody::collide( *body0, *body1 ) )
            _world->_collisionBeginCallback( pair );
         // Check for sensations.
         if( _world->_sensationBeginCallback && RigidBody::sense( *body0, *body1 ) )
            _world->_sensationBeginCallback( pair );
      }
   }

   // End contacts.
   for( auto cur = _contactCache.outdatedBegin(); cur != _contactCache.outdatedEnd(); ++cur )
   {
      RigidBody* body0 = cur->first;
      RigidBody* body1 = cur->second;
      CollisionPair pair( body0, body1 );
      // Check for collisions.
      if( _world->_collisionEndCallback && RigidBody::collide( *body0, *body1 ) )
         _world->_collisionEndCallback( pair );
      // Check for sensations.
      if( _world->_sensationEndCallback && RigidBody::sense( *body0, *body1 ) )
         _world->_sensationEndCallback( pair );
   }

   _contactCache.endFrame();
}


/*==============================================================================
  CLASS CustomCollisionDispatcher
==============================================================================*/
class CustomCollisionDispatcher:
   public btCollisionDispatcher
{
public:

   /*----- methods -----*/

   CustomCollisionDispatcher( btCollisionConfiguration* colConf ):
      btCollisionDispatcher( colConf ) {}

   virtual bool needsCollision( btCollisionObject* body0, btCollisionObject* body1 )
   {
      if( !body0->isActive() && !body1->isActive() ) return false;

      RigidBody* motionBody0 = (RigidBody*)body0->getUserPointer();
      RigidBody* motionBody1 = (RigidBody*)body1->getUserPointer();

      return ((motionBody0->exists() & motionBody1->exists()) |      // Needs response.
              (motionBody0->senses() & motionBody1->exists()) |      // body0 senses body1.
              (motionBody0->exists() & motionBody1->senses())) != 0; // body1 senses body0.
   }

   virtual bool needsResponse( btCollisionObject* body0, btCollisionObject* body1 )
   {
      RigidBody* motionBody0 = (RigidBody*)body0->getUserPointer();
      RigidBody* motionBody1 = (RigidBody*)body1->getUserPointer();

      bool hasResponse = RigidBody::collide( *motionBody0, *motionBody1 );
      hasResponse      = hasResponse && (motionBody0->isDynamic() || motionBody1->isDynamic());

      return hasResponse;
   }

};

/*==============================================================================
   CLASS CustomConvexResultCallback
==============================================================================*/
class CustomConvexResultCallback:
   public btCollisionWorld::ConvexResultCallback
{
public:

   /*----- methods -----*/

   CustomConvexResultCallback( const RigidBody* nonBody ): _body(nullptr), _nonBody( nonBody ) {}

   virtual  btScalar addSingleResult( btCollisionWorld::LocalConvexResult& convexResult, bool /*normalInWorldSpace*/ )
   {
      RigidBody* body = (RigidBody*)convexResult.m_hitCollisionObject->getUserPointer();

      if( body == _nonBody ) return CGConstf::infinity();

      _body                = body;
      _pos                 = toVec3f( convexResult.m_hitPointLocal );
      m_closestHitFraction = convexResult.m_hitFraction;

      return convexResult.m_hitFraction;
   }

   RigidBody* body()             { return _body; }
   const Vec3f& position() const { return _pos; }

protected:

   /*----- data members -----*/

   RigidBody*       _body;
   const RigidBody* _nonBody;
   Vec3f            _pos;
};

/*==============================================================================
  CLASS CustomContactResultCallback
==============================================================================*/
class CustomContactResultCallback:
   public btCollisionWorld::ContactResultCallback
{
public:

   /*----- methods -----*/

   CustomContactResultCallback( const btCollisionObject* obj, MotionWorld::IntersectionData& dst ):
      _collidingObject( obj ),
      _intersectionData( dst )
   {
   }

   virtual btScalar  addSingleResult(
      btManifoldPoint& cp,
      const btCollisionObject* colObj0, int /*partId0*/, int /*index0*/,
      const btCollisionObject* colObj1, int /*partId1*/, int /*index1*/
   )
   {
      if( colObj0 != _collidingObject )
      {
         CHECK( colObj1 == _collidingObject );
         RigidBody* body0 = (RigidBody*)colObj0->getUserPointer();
         _intersectionData.add( body0, toVec3f(cp.getPositionWorldOnA()) );
      }
      else
      {
         CHECK( colObj1 != _collidingObject );
         RigidBody* body1 = (RigidBody*)colObj1->getUserPointer();
         _intersectionData.add( body1, toVec3f(cp.getPositionWorldOnB()) );
      }
      return 0;
   }

protected:

   /*----- data members -----*/

   const btCollisionObject*        _collidingObject;
   MotionWorld::IntersectionData&  _intersectionData;

private:
}; //class CustomContactResultCallback


/*==============================================================================
  CLASS CustomRayResultCallback
==============================================================================*/
class CustomRayResultCallback:
   public btCollisionWorld::ClosestRayResultCallback
{
public:

   CustomRayResultCallback( const btVector3& from, const btVector3& to, uint typeMask, uint existMask ):
      ClosestRayResultCallback( from, to ),
      _typeMask( typeMask ),
      _existMask( existMask )
   {
   }

   virtual bool needsCollision( btBroadphaseProxy* p ) const
   {
      btCollisionObject* o = (btCollisionObject*)p->m_clientObject;
      RigidBody* r = (RigidBody*)o->getUserPointer();
      return (r->typeMask() & _typeMask ) != 0x0 &&
             (r->exists()   & _existMask) != 0x0 &&
             ClosestRayResultCallback::needsCollision( p );
   }

protected:

   /*----- data members -----*/

   uint  _typeMask;
   uint  _existMask;
};


/*==============================================================================
  CLASS CustomDrawer
==============================================================================*/
class CustomDrawer:
   public btIDebugDraw
{
public:

   /*----- methods -----*/

   CustomDrawer();
   //virtual ~CustomDrawer();

   void  set( Vector<float>& points, Vector<float>& lines, Vector<float>& triangles );

   virtual void  setDebugMode( int debugMode );
   virtual int   getDebugMode() const;

   virtual void  draw3dText( const btVector3& location, const char* textString );
   virtual void  drawContactPoint( const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color );
   virtual void  drawLine( const btVector3& from, const btVector3& to, const btVector3& color );

   virtual void  reportErrorWarning( const char* warningString );

protected:

   /*----- data members -----*/

   Vector<float>*  _points;
   Vector<float>*  _lines;
   Vector<float>*  _triangles;
   int             _debugMode;

   /**
    * Various debug mode bits:
    *   DBG_NoDebug
    *   DBG_DrawWireframe
    *   DBG_DrawAabb
    *   DBG_DrawFeaturesText
    *   DBG_DrawContactPoints
    *   DBG_NoDeactivation
    *   DBG_NoHelpText
    *   DBG_DrawText
    *   DBG_ProfileTimings
    *   DBG_EnableSatComparison
    *   DBG_DisableBulletLCP
    *   DBG_EnableCCD
    *   DBG_DrawConstraints
    *   DBG_DrawConstraintLimits
    *   DBG_FastWireframe
    */


   /*----- methods -----*/

   /* methods... */

private:
}; //class CustomDrawer

//-----------------------------------------------------------------------------
//!
CustomDrawer::CustomDrawer():
   _points( NULL ),
   _lines( NULL ),
   _triangles( NULL ),
   _debugMode( DBG_DrawWireframe|DBG_DrawContactPoints|DBG_NoDeactivation|DBG_FastWireframe )
{
}

//-----------------------------------------------------------------------------
//!
void
CustomDrawer::set( Vector<float>& points, Vector<float>& lines, Vector<float>& triangles )
{
   _points    = &points;
   _lines     = &lines;
   _triangles = &triangles;
}

//-----------------------------------------------------------------------------
//!
void
CustomDrawer::setDebugMode( int debugMode )
{
   _debugMode = debugMode;
}

//-----------------------------------------------------------------------------
//!
int
CustomDrawer::getDebugMode() const
{
   return _debugMode;
}

//-----------------------------------------------------------------------------
//!
void
CustomDrawer::draw3dText( const btVector3& location, const char* textString )
{
   StdErr << "draw3dText: " << textString << " @ " << toVec3f(location) << nl;
}

//-----------------------------------------------------------------------------
//!
void
CustomDrawer::drawLine( const btVector3& from, const btVector3& to, const btVector3& color )
{
   Vec4f c = toVec4f( color, 1.0f );
   add( *_lines, toVec3f(from), c );
   add( *_lines, toVec3f(to  ), c );
}

//-----------------------------------------------------------------------------
//!
void
CustomDrawer::drawContactPoint( const btVector3& PointOnB, const btVector3& normalOnB, btScalar /*distance*/, int /*lifeTime*/, const btVector3& color )
{
   Vec4f c = toVec4f( color, 1.0f );
   add( *_points, toVec3f(PointOnB), c );
   add( *_lines, toVec3f(PointOnB          ), c );
   add( *_lines, toVec3f(PointOnB+normalOnB), c );
}

//-----------------------------------------------------------------------------
//!
void
CustomDrawer::reportErrorWarning( const char* warningString )
{
   StdErr << "Bullet: " << warningString << nl;
}


NAMESPACE_BEGIN

/*==============================================================================
   CLASS MotionWorld
==============================================================================*/

MotionWorld::GetTransformCallback  MotionWorld::_getTransformCB;
MotionWorld::SetTransformCallback  MotionWorld::_setTransformCB;

//------------------------------------------------------------------------------
//!
void
MotionWorld::printInfo( TextStream& os )
{
   os << "Motion:";
   int v = btGetVersion();
   os << " Bullet(v" << v << ")";
   os << nl;
}

//------------------------------------------------------------------------------
//!
MotionWorld::MotionWorld() :
   _time( 0.0 )
{
   simulationRate( 60.0 );

   _collisionConfiguration = new btDefaultCollisionConfiguration();
   _collisionDispatcher    = new CustomCollisionDispatcher( _collisionConfiguration );

   // Set broadphase.
   // More details here:
   //   http://bulletphysics.org/mediawiki-1.5.8/index.php/Broadphase
   //float range = (float)(1 << 14);
   //btVector3 worldMin( -range, -range, -range );
   //btVector3 worldMax(  range,  range,  range );
   //_broadPhase = new btAxisSweep3( worldMin, worldMax );
   //_broadPhase = new bt32BitAxisSweep3( worldMin, worldMax );  // Crashes in iOS 4.1.
   _broadPhase = new btDbvtBroadphase();

   _solver = new btSequentialImpulseConstraintSolver();
   //_solver = new btParallelConstraintSolver( 2 );
   _world  = new btWorld(
      this,
      _collisionDispatcher,
      _broadPhase,
      _solver,
      _collisionConfiguration
   );

#if 1
   // Tweaking contact solver parameters.
   // More details here:
   //   http://bulletphysics.org/mediawiki-1.5.8/index.php/BtContactSolverInfo
   btContactSolverInfo& solverInfo = _world->getSolverInfo();
   solverInfo.m_solverMode   &= ~SOLVER_RANDMIZE_ORDER; // Disable randomization in Bullet's solver.
   solverInfo.m_splitImpulse  = 1;
   //solverInfo.m_numIterations = 4; // 4 = Fast, 10 = Default, 20 = High Quality.

   // Rigid bodies deactivation parameters.
   gDeactivationTime          = 0.5f;
   //gDisableDeactivation       = true;
#endif

   // TEMP
   //_world->setGravity(btVector3(0,0,0));
   defaultGravity( Vec3f(0.0f, -9.81f, 0.0f) );
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

   delete _world;
   delete _solver;
   delete _broadPhase;
   delete _collisionDispatcher;
   delete _collisionConfiguration;
}

//------------------------------------------------------------------------------
//! TEMP (FIXME)
//! Normally, a DirectionalAttractor should do the trick, but it ain't (block stacking stability).
void
MotionWorld::defaultGravity( const Vec3f& g )
{
   _world->setGravity( btVector3(g.x, g.y, g.z) );
   _defaultGravityNorm = CGM::length( g );
   _defaultGravityDir  = g / _defaultGravityNorm;
}

//-----------------------------------------------------------------------------
//!
void
MotionWorld::getGravity( const RigidBody* /*body*/, Vec3f& gravityDir, float& gravityNorm ) const
{
   // TODO: Honor mulitple gravity regions.
   gravityDir  = _defaultGravityDir;
   gravityNorm = _defaultGravityNorm;
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
      case RigidBody::DYNAMIC:    _rigidBodies.removeSwap( body );     break;
      case RigidBody::STATIC:     _staticBodies.removeSwap( body );    break;
      case RigidBody::KINEMATIC:  _kinematicBodies.removeSwap( body ); break;
   }
}

//------------------------------------------------------------------------------
//!
void
MotionWorld::bodyTypeChanged( RigidBody* body, RigidBody::Type newType )
{
   switch( newType )
   {
      case RigidBody::DYNAMIC:    _rigidBodies.pushBack( body );     break;
      case RigidBody::STATIC:     _staticBodies.pushBack( body );    break;
      case RigidBody::KINEMATIC:  _kinematicBodies.pushBack( body ); break;
   }
   switch( body->type() )
   {
      case RigidBody::DYNAMIC:    _rigidBodies.removeSwap( body );     break;
      case RigidBody::STATIC:     _staticBodies.removeSwap( body );    break;
      case RigidBody::KINEMATIC:  _kinematicBodies.removeSwap( body ); break;
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
   attractor->connect( this );
}

//------------------------------------------------------------------------------
//!
void
MotionWorld::removeAttractor( Attractor* attractor )
{
   attractor->disconnect();
   _attractors.removeSwap( attractor );
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
MotionWorld::createHingeJoint( RigidBody* bodyA, RigidBody* bodyB )
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
   constraint->connect( this );
}

//------------------------------------------------------------------------------
//!
void
MotionWorld::removeConstraint( Constraint* constraint )
{
   _constraints.removeSwap( constraint );
   constraint->disconnect();
}

//------------------------------------------------------------------------------
//!
void
MotionWorld::removeAllConstraints()
{
   _constraints.clear();
   for( uint i = 0; i < _constraints.size(); ++i )
   {
      _constraints[i]->disconnect();
   }
}

//-----------------------------------------------------------------------------
//! Finds all of the rigid bodies intersection the specified shape and returns
//! them in the destination IntersectionData structure.
void
MotionWorld::findIntersectingBodies( const CollisionShape& shape, const Reff& shapeRef, IntersectionData& dst )
{
   btCollisionObject* obj = new btCollisionObject();
   obj->setCollisionShape( const_cast<CollisionShape&>(shape).bulletCollisionShape() );
   btTransform t;
   convert( shapeRef, t );
   obj->setWorldTransform( t );

   CustomContactResultCallback cb( obj, dst );

   _world->contactTest( obj, cb );
}

//-----------------------------------------------------------------------------
//! Finds the closest rigid body hit by the ray, and stores them in the same
//! order in the destination IntersectionData structure.
void
MotionWorld::raycast( const Vector<Rayf>& rays, uint typeMask, uint existMask, IntersectionData& dst )
{
   CHECK( dst.empty() );

   // Raycast in the whole scene.
   for( auto cur = rays.begin(), end = rays.end(); cur != end; ++cur )
   {
      const Rayf& ray = *cur;
      btVector3 from = toBullet( ray.origin() );
      btVector3   to = toBullet( ray.origin() + ray.direction() );
      CustomRayResultCallback cb( from, to, typeMask, existMask );
      _world->rayTest( from, to, cb );
      if( cb.hasHit() )
      {
         btRigidBody* btBody = btRigidBody::upcast( cb.m_collisionObject );
         if( btBody && btBody->hasContactResponse() )
         {
            RigidBody* body = (RigidBody*)btBody->getUserPointer();
            dst.add( body, toVec3f(cb.m_hitPointWorld) );
         }
         else
         {
            StdErr << "MotionWorld::raycast() - No contact response." << nl;
            dst.add( NULL, Vec3f(0.0f) );
         }
      }
      else
      {
         dst.add( NULL, Vec3f(0.0f) );
      }
   }
}

//------------------------------------------------------------------------------
//!
void
MotionWorld::ccd(
   const CollisionShape& convex,
   const Reff&           from,
   const Reff&           to,
   IntersectionData&     intersections,
   Vec3f&                collisionPos,
   const RigidBody*      nonBody
)
{
   btCollisionShape* shape = const_cast<CollisionShape&>(convex).bulletCollisionShape();
   if( !shape->isConvex() ) return;

   btConvexShape* cshape = (btConvexShape*)shape;

   btTransform btFrom;
   btTransform btTo;
   convert( from, btFrom );
   convert( to, btTo );
   CustomConvexResultCallback cb( nonBody );
   _world->convexSweepTest( cshape, btFrom, btTo, cb );
   if( cb.body() )
   {
      intersections.add( cb.body(), cb.position() );
      collisionPos = from.position() + (to.position()-from.position()) * cb.m_closestHitFraction;
      StdErr << "from=" << from.position() << " to=" << to.position() << " fr=" << cb.m_closestHitFraction << nl;
   }
}

//-----------------------------------------------------------------------------
//! Prepares buffers for debug rendering.
//! The vertex buffer format is always XYZRGBA for every vertex.
void
MotionWorld::debugRender( Vector<float>& points, Vector<float>& lines, Vector<float>& triangles )
{
   CustomDrawer* drawer = (CustomDrawer*)_world->getDebugDrawer();
   if( drawer == NULL )
   {
      drawer = new CustomDrawer();
      _world->setDebugDrawer( drawer );
   }
   drawer->set( points, lines, triangles );
   _world->debugDrawWorld();
}

//------------------------------------------------------------------------------
//!
bool
MotionWorld::stepSimulation( double step )
{
   _time += step;

   //handleConstraintsPreStep( step );

   // Physics simulation by Bullet.
   int nbSubSteps = _world->stepSimulation( (btScalar)step, 2, (btScalar)_simulationDelta );

   clearForces();

   //handleConstraintsPostStep( step );

   return nbSubSteps != 0;
}

NAMESPACE_END
