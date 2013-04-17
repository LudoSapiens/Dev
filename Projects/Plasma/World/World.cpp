/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/World/World.h>

#include <Plasma/Plasma.h>
#include <Plasma/Action/Command.h>
#include <Plasma/Render/DebugGeometry.h>
#include <Plasma/Renderable/Renderable.h>
#include <Plasma/Resource/ResManager.h>
#include <Plasma/Stimulus/EventStimuli.h>
#include <Plasma/World/EntityGroup.h>
#include <Plasma/World/Light.h>
#include <Plasma/World/Probe.h>
#include <Plasma/World/RigidEntity.h>
#include <Plasma/World/SkeletalEntity.h>
#include <Plasma/World/Selection.h>
#include <Plasma/World/WorldVM.h>

#if MOTION_BULLET
#include <MotionBullet/Collision/BasicShapes.h>
#else
#include <Motion/Collision/BasicShapes.h>
#endif

#include <Fusion/Core/Animator.h>
#include <Fusion/Core/Core.h>
#include <Fusion/Core/EventProfiler.h>
#include <Fusion/VM/VMRegistry.h>

#include <Base/ADT/Pair.h>
#include <Base/Dbg/DebugStream.h>
#include <Base/Msg/Observer.h>
#include <Base/MT/TaskQueue.h>
#include <Base/MT/Thread.h>
#include <Base/Util/Bits.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

DBG_STREAM( os_w, "World" );

const char* _world_str = "world";

Vector<World*> _worlds;

//------------------------------------------------------------------------------
//!
bool runAnimators(
   double time,
   double delta,
   Vector< RCP<Animator> >& animators,
   size_t startIndex
)
{
   bool removed = false;

   auto cur = animators.begin() + startIndex;
   auto end = animators.end();

   for( ; cur != end; ++cur )
   {
      Animator* anim = (*cur).ptr();
      if( anim != NULL )
      {
         if( anim->nextTime() <= time )
         {
            // Compute the delta since the last call.
            double animDelta = CGM::min( delta, anim->period() + time - anim->nextTime() );

            // Compute the next time before calling exec().
            anim->nextTime( anim->nextTime() + anim->period() ); // Respect animator's frequency.
            DBG_BEGIN();
            // Some sanity check in debug only.
            if( anim->nextTime() < time && anim->period() > 0.0 )
            {
               double n = (time - anim->nextTime()) / anim->period();
               if( n > 3 )
               {
                  StdErr << "WARNING: A world animator is running more than 3 periods late: "
                         << " time=" << time << " delta=" << delta
                         << " animDelta=" << animDelta << " anim_next=" << anim->nextTime() << " anim_period=" << anim->period() << nl;
               }
            }
            DBG_END();

            // Call the exec() routine; animators could delete themselves from the queue.
            if( anim->exec( time, animDelta ) )
            {
               (*cur)  = NULL; // Using iterator, which could point to NULL already if the exec did a remove.
               removed = true;
            }
         }
      }
      else
      {
         removed = true;
      }
   }

   return removed;
}

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
  CLASS WorldAnimator
==============================================================================*/

//!

class WorldAnimator
   : public Animator
{

public:

   /*----- methods -----*/

   WorldAnimator()
   {}

   virtual bool exec( double /*time*/, double delta )
   {
      for( auto cur = _worlds.begin(), end = _worlds.end(); cur != end; ++cur )
      {
         EventProfiler& profiler = Core::profiler();
         profiler.add( EventProfiler::WORLD_BEGIN );
         World* world = *cur;
         if( world->simulationSpeed() != 0.0 )
         {
            double worldDelta = delta * world->simulationSpeed();
            double worldTime  = world->time() + worldDelta;
            world->runAnimators( worldTime, worldDelta );
            world->runBrains( worldDelta );
            world->runCommandsAfterBrains();
            world->runActions( worldTime, worldDelta, false );
            world->runCommandsAfterActions();
            world->runPhysics( worldDelta );
            world->runActions( worldTime, worldDelta, true );
            world->runCommandsAfterPhysics();
            //Thread::sleep(1.0/4.0);
         }
         profiler.add( EventProfiler::WORLD_END );
      }

      return false;
   }


protected:

   /*----- methods -----*/

   virtual ~WorldAnimator() {}

};

/*==============================================================================
  CLASS World
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
World::initialize()
{
   Core::addAnimator( new WorldAnimator() );
   VMRegistry::add( _world_str, NULL, world_get, world_set, VM_CAT_APP );
   MotionWorld::getTransformCallback( MotionWorld::GetTransformCallback(&World::getTransform) );
   MotionWorld::setTransformCallback( MotionWorld::SetTransformCallback(&World::setTransform) );
}

//------------------------------------------------------------------------------
//!
void
World::getTransform( const void* userData, Reff& dstRef, Mat4f& dstMat )
{
   const Entity* e = static_cast<const Entity*>( userData );
   dstRef = e->referential();
   dstMat = e->transform();
}

//------------------------------------------------------------------------------
//!
void
World::setTransform( void* userData, const Reff& srcRef, const Mat4f& srcMat )
{
   Entity* e = static_cast<Entity*>( userData );
   e->update( srcRef, srcMat );
}

//------------------------------------------------------------------------------
//!
World::World()
   : _physicsLooped( false ),
     _gravity( 0, -9.80665f, 0 ),
     _bgColor( 0.5f, 0.5f, 0.5f, 1.0f ),
     _simulationSpeed( 1 ),
     _state( 0x0 )
{
   _attributes       = new Table();
   _gravityAttractor = new DirectionalAttractor(_gravity);
   newMotionWorld();
   _debugGeometry = new DebugGeometry();
}

//------------------------------------------------------------------------------
//!
World::~World()
{
   stopSimulation();

   // Disconnect all connected elements.
   removeAllEntities();
   removeAllRenderable();

   for( auto cur = _sndSources.begin(); cur != _sndSources.end(); ++cur )
   {
      Snd::Source& src  = *(cur->first);
      if( src.valid() && src.playing() )  src.stop();
   }
}

//------------------------------------------------------------------------------
//!
void World::startSimulation()
{
   _worlds.pushBack( this );
}
//------------------------------------------------------------------------------
//!
void World::stopSimulation()
{
   _worlds.remove( this );
}

//------------------------------------------------------------------------------
//!
void World::clear()
{
   // Clearing entities.
   removeAllEntities();

   // Others.
   _renderables.clear();
   _selections.clear();

   // Receptors.
   _contactReceptors.clear();
   _contactGroupReceptors.clear();

   // New physics world.
   newMotionWorld();

   // Time.
   _simulationSpeed = 1.0f;
   _state           = 0;
}

//------------------------------------------------------------------------------
//!
void World::addEntity( Entity* entity )
{
   DBG_BLOCK( os_w, "World::addEntity" );
   if( entity->world() == this ) return;

   _entities.pushBack( entity );

   switch( entity->type() )
   {
      case Entity::LIGHT:
         _lights.pushBack( (Light*)entity );
         break;
      case Entity::CAMERA:
         _cameras.pushBack( (Camera*)entity );
         break;
      default: break;
   }

   // ID registering.
   if( !entity->id().isNull() )
   {
      auto it = _entityIDs.find( entity->id() );
      if( it != _entityIDs.end() )
      {
         StdErr << "ERROR: The ID " << entity->id() << " of the entity " << entity << " is not unique.\n";
      }
      else
         _entityIDs[entity->id()] = entity;
   }

   // Brain registering.
   if( entity->brain() )
   {
      Brain::ReceptorContainer& receptors = entity->brain()->receptors();
      for( auto cur = receptors.begin(); cur != receptors.end(); ++cur )
      {
         registerReceptor( entity, (*cur).ptr() );
      }
      if( entity->brain()->hasStimuli() )  scheduleAnalyze( entity );
      if( entity->brain()->hasActions() )  markForActionBefore( entity );  // After should still work.
   }

   entity->connect( this );
}

//------------------------------------------------------------------------------
//!
void World::removeEntity( Entity* entity )
{
   DBG_BLOCK( os_w, "World::removeEntity" );

   if( entity->world() == this )
   {
      // Unregister brain.
      if( entity->brain() )
      {
         Brain::ReceptorContainer& receptors = entity->brain()->receptors();
         for( auto cur = receptors.begin(); cur != receptors.end(); ++cur )
         {
            unregisterReceptor( entity, (*cur).ptr() );
         }
      }

      // Unregister from analyze and actions.
      // TODO!!!

      // Unregister ID.
      if( !entity->id().isNull() )
      {
         auto it = _entityIDs.find( entity->id() );
         if( it != _entityIDs.end() )
         {
            if( it->second == entity ) _entityIDs.erase( it );
         }
      }

      // Removing entity.
      entity->disconnect();
      _entities.removeSwap( entity );

      switch( entity->type() )
      {
         case Entity::LIGHT:
            _lights.removeSwap( (Light*)entity );
            break;
         case Entity::CAMERA:
            _cameras.removeSwap( (Camera*)entity );
            break;
         default: break;
      }
   }
#if DBG_MSG
   else if( entity->world().isValid() )
   {
      DBG_MSG( os_w, "ERROR - Attempt to remove an entity not of this world." );
   }
#endif
}

//------------------------------------------------------------------------------
//!
void World::removeAllEntities()
{
   DBG_BLOCK( os_w, "World::removeAllEntities" );

   for( uint i = 0; i < _entities.size(); ++i )
   {
      Entity* e = _entities[i].ptr();
      // Unregister brain.
      if( e->brain() )
      {
         auto& receptors = e->brain()->receptors();
         for( auto cur = receptors.begin(); cur != receptors.end(); ++cur )
         {
            unregisterReceptor( e, (*cur).ptr() );
         }
      }
      // Disconnecting signal.
      e->disconnect();
   }
   _entities.clear();
   _entityIDs.clear();
   _lights.clear();
   _cameras.clear();

   _entitiesWithActionsBefore.clear();
   _entitiesWithActionsAfter.clear();
   _entitiesToAnalyze.clear();

   // Clearing receptors.
   _contactReceptors.clear();
   _contactGroupReceptors.clear();
}

//------------------------------------------------------------------------------
//!
AABBoxf World::boundingBox() const
{
   // FIXME: We should always update the bounding box automaticaly when
   // entities move and we should keep an acceleration structure for it.
   AABBoxf box = AABBoxf::empty();
   for( auto cur = _entities.begin(); cur != _entities.end(); ++cur )
   {
      Geometry* geom = (*cur)->geometry();
      if( geom ) box |= geom->boundingBox();
   }
   return box;
}

//------------------------------------------------------------------------------
//!
Entity* World::entity( const ConstString& id ) const
{
   auto it = _entityIDs.find( id );
   if( it != _entityIDs.end() ) return it->second;
   return nullptr;
}

//------------------------------------------------------------------------------
//!
Light* World::light( const ConstString& id ) const
{
   Entity* e = entity( id );
   if( e && e->type() == Entity::LIGHT ) return (Light*)e;
   return nullptr;
}
//------------------------------------------------------------------------------
//!
Camera* World::camera( const ConstString& id ) const
{
   Entity* e = entity( id );
   if( e && e->type() == Entity::CAMERA ) return (Camera*)e;
   return nullptr;
}

//------------------------------------------------------------------------------
//!
void World::updateID( Entity* e, const ConstString& newID )
{
   // Remove ID.
   if( !e->id().isNull() )
   {
      auto it = _entityIDs.find( e->id() );
      if( it != _entityIDs.end() )
      {
         if( it->second == e ) _entityIDs.erase( it );
      }
   }

   // Add ID.
   if( newID.isNull() ) return;

   auto it = _entityIDs.find( newID );
   if( it != _entityIDs.end() )
   {
      StdErr << "ERROR: The ID " << newID << " of the entity " << e << " is not unique.\n";
   }
   else
      _entityIDs[newID] = e;
}

//------------------------------------------------------------------------------
//!
void World::addGroup( EntityGroup* g )
{
   _groups.pushBack( g );
}

//------------------------------------------------------------------------------
//!
void World::removeGroup( EntityGroup* g )
{
   _groups.removeSwap( g );
}

//------------------------------------------------------------------------------
//!
void World::removeAllGroups()
{
   _groups.clear();
}

//------------------------------------------------------------------------------
//!
void World::addRenderable( Renderable* r )
{
   _renderables.pushBack( r );
}

//------------------------------------------------------------------------------
//!
void World::removeRenderable( Renderable* r )
{
   _renderables.removeSwap( r );
}

//------------------------------------------------------------------------------
//!
void World::removeAllRenderable()
{
   _renderables.clear();
}

//------------------------------------------------------------------------------
//!
BallJoint*
World::createBallJoint( RigidEntity* entityA, RigidEntity* entityB )
{
   return motionWorld()->createBallJoint( entityA->_body.ptr(), entityB->_body.ptr() );
}

//------------------------------------------------------------------------------
//!
HingeJoint*
World::createHingeJoint( RigidEntity* entityA, RigidEntity* entityB )
{
   return motionWorld()->createHingeJoint( entityA->_body.ptr(), entityB->_body.ptr() );
}

//------------------------------------------------------------------------------
//!
void
World::addConstraint( Constraint* constraint )
{
   motionWorld()->addConstraint( constraint );
}

//------------------------------------------------------------------------------
//!
void
World::removeConstraint( Constraint* constraint )
{
   motionWorld()->removeConstraint( constraint );
}

//------------------------------------------------------------------------------
//!
void
World::removeAllConstraints()
{
   motionWorld()->removeAllConstraints();
}

//------------------------------------------------------------------------------
//!
void
World::addSelection( Selection* s )
{
   _selections.pushBack( s );
}

//------------------------------------------------------------------------------
//!
void
World::removeSelection( Selection* s )
{
   _selections.removeSwap( s );
}

//------------------------------------------------------------------------------
//!
void
World::removeAllSelections()
{
   _selections.clear();
}

//------------------------------------------------------------------------------
//!
void
World::bind( RigidEntity* e, Snd::Source*   src )
{
   List<Snd::Source*>& list = _sndBoundSources[e];
   list.pushBack( src );
}

//------------------------------------------------------------------------------
//!
void
World::bind( RigidEntity* e, Snd::Listener* lis )
{
   List<Snd::Listener*>& list = _sndBoundListeners[e];
   list.pushBack( lis );
}

//------------------------------------------------------------------------------
//!
void
World::unbind( RigidEntity* e, Snd::Source* src )
{
   if( src )
   {
      List<Snd::Source*>& list = _sndBoundSources[e];
      list.remove( src );
      if( list.empty() )  _sndBoundSources.erase( e );
   }
   else
   {
      _sndBoundSources.erase( e );
   }
}

//------------------------------------------------------------------------------
//!
void
World::unbind( RigidEntity* e, Snd::Listener* lis )
{
   if( lis )
   {
      List<Snd::Listener*>& list = _sndBoundListeners[e];
      list.remove( lis );
      if( list.empty() )  _sndBoundListeners.erase( e );
   }
   else
   {
      _sndBoundListeners.erase( e );
   }
}

//------------------------------------------------------------------------------
//!
void
World::addProbe( Probe* p )
{
   _probes.pushBack( p );
}

//------------------------------------------------------------------------------
//!
void
World::removeProbe( Probe* p )
{
   _probes.remove( p );
}

//------------------------------------------------------------------------------
//! Returns the best probe amongst the one present in the world.
//! Note: Currently does NOT consider visibilty (only distance).
Probe*
World::findBestProbe( const Vec3f& pos ) const
{
   float bestDistSq = CGConstf::max();
   Probe* bestProbe = nullptr;
   for( auto cur = _probes.begin(); cur != _probes.end(); ++cur )
   {
      Probe* p = (*cur).ptr();
      float  d = sqrLength( p->position() - pos ); // TODO: Condisder visibility.
      if( d < bestDistSq )
      {
         bestDistSq = d;
         bestProbe  = p;
      }
   }
   return bestProbe;
}

//------------------------------------------------------------------------------
//!
void
World::gravity( const Vec3f& val )
{
   _gravity = val;
   _gravityAttractor->acceleration( val );
#if MOTION_BULLET
   _world->defaultGravity( val );
#endif
}

//------------------------------------------------------------------------------
//!
const Vec3f&
World::gravity() const
{
   return _gravity;
}

//------------------------------------------------------------------------------
//!
void
World::runPhysics( double step )
{
   PROFILE_EVENT( EventProfiler::WORLD_BEGIN_PHYSICS );
   double realStep = step * _simulationSpeed;
   _physicsLooped  = _world->stepSimulation( realStep );

   // Generate the sounds.
   //emitSounds( worldTime, worldDelta );

   if( _debugGeometry.isValid() && Plasma::debugGeometry() )
   {
      _debugGeometry->clear();
      _world->debugRender( _debugGeometry->points(), _debugGeometry->lines(), _debugGeometry->triangles() );
      _debugGeometry->computeRenderableGeometry();
   }
}

//------------------------------------------------------------------------------
//!
void
World::simulationSpeed( double speed )
{
   // What should be the maximum allowed speed?
   speed = CGM::clamp( speed, 0.0, 4.0 );
   updateSoundStates( _simulationSpeed, speed );
   _simulationSpeed = speed;
}

//-----------------------------------------------------------------------------
//!
void
World::scheduleAnalyze( Entity* e )
{
   _entitiesToAnalyze.add(e);
}

//------------------------------------------------------------------------------
//!
void
World::registerReceptor( Entity* e, Receptor* r )
{
   //FIXME: One vector per type of receptor, bubbleInsert, etc.
   switch( r->type() )
   {
      case Receptor::TYPE_CONTACT:
      {
         ContactReceptor* cr = static_cast<ContactReceptor*>(r);
         _contactReceptors[cr->receivingEntity()].pushBack( EntityReceptor<ContactReceptor>(e, cr) );
      }  break;
      case Receptor::TYPE_CONTACT_GROUP:
      {
         ContactGroupReceptor* cgr = static_cast<ContactGroupReceptor*>(r);
         _contactGroupReceptors.pushBack( EntityReceptor<ContactGroupReceptor>(e, cgr) );
         //addCollisionGroups( cgr->receivingGroups() );
      }  break;
      default:
         printf("ERROR - World::registerReceptor does not know about receptor type: %d\n", r->type());
         break;
   }
}

//------------------------------------------------------------------------------
//!
void
World::unregisterReceptor( Entity* e, Receptor* r )
{
   //FIXME: One vector per type of receptor, bubbleRemove, etc.
   switch( r->type() )
   {
      case Receptor::TYPE_CONTACT:
      {
         ContactReceptor* cr = static_cast<ContactReceptor*>(r);
         auto it = _contactReceptors.find( cr->receivingEntity() );
         CHECK( it != _contactReceptors.end() );
         ContactReceptorList& list = it.data();
         list.remove( EntityReceptor<ContactReceptor>(e, cr) );
         if( list.empty() )  _contactReceptors.erase( it );
      }  break;
      case Receptor::TYPE_CONTACT_GROUP:
      {
         ContactGroupReceptor* cgr = static_cast<ContactGroupReceptor*>(r);
         _contactGroupReceptors.remove( EntityReceptor<ContactGroupReceptor>(e, cgr) );
         //removeCollisionGroups( cgr->receivingGroups() );
      }  break;
      default:
         printf("ERROR - World::unregisterReceptor does not know about receptor type: %d\n", r->type());
         break;
   }
}

//------------------------------------------------------------------------------
//!
void
World::runBrains( double /*step*/ )
{
   EntitySet::Container entities;
   _entitiesToAnalyze.drain( entities );
   PROFILE_EVENT_C( EventProfiler::WORLD_BEGIN_BRAINS, entities.size() );
   if( !entities.empty() )
   {
      TaskQueue* queue = Plasma::dispatchQueue();
      // TODO: bundle brains by type to reduce the code of bytecode loading time
      // compared to its execution time.
      for( auto cur = entities.begin(); cur != entities.end(); ++cur )
      {
         Entity* e       = (*cur).ptr();
         BrainTask* task = new BrainTask( e );
         queue->post( task );
      }
      queue->waitForAll();

      Brain::handlePostedCommands( this );
      Brain::markEntitiesForAction( this );
   }
}

//------------------------------------------------------------------------------
//!
void
World::addAnimator( Animator* a )
{
   a->nextTime( time() );
   _newAnimators.pushBack(a);
}

//------------------------------------------------------------------------------
//!
void
World::addAnimator( Animator* a, double startTime )
{
   a->nextTime( startTime );
   _newAnimators.pushBack(a);
}

//------------------------------------------------------------------------------
//!
void
World::removeAnimator( Animator* a )
{
   if( !_animators.replace( a, NULL ) )
   {
      // Didn't find it in _animators, so falling back in _newAnimators.
      if( !_newAnimators.replace( a, NULL ) )
      {
         //StdErr << "ERROR - World::removeAnimator() could not find animator: " << (void*)a << nl;
      }
   }
}

//------------------------------------------------------------------------------
//!
void
World::runAnimators( double time, double delta )
{
   PROFILE_EVENT_C( EventProfiler::WORLD_BEGIN_ANIMATORS, _animators.size() );
   bool removed = ::runAnimators( time, delta, _animators, 0 );
   while( !_newAnimators.empty() )
   {
      size_t oldSize = _animators.size();
      _animators.append( _newAnimators );
      _newAnimators.clear();
      removed |= ::runAnimators( time, delta, _animators, oldSize );
   }

   if( removed )
   {
      _animators.removeAll( NULL );
   }
}

//------------------------------------------------------------------------------
//! Handles all of the specified commands.
void
World::handleCommands( const CommandContainer& cmds )
{
   for( auto cur = cmds.begin(), end = cmds.end(); cur != end; ++cur )
   {
      const Command& cmd = (*cur);
      switch( cmd.type() )
      {
         case Command::TYPE_DELEGATE0:
         {
            const CmdDelegate0& ev = CmdDelegate0::as( cmd );
            ev.delegate()();
         }  break;
         case Command::TYPE_DELEGATE1:
         {
            const CmdDelegate1& ev = CmdDelegate1::as( cmd );
            ev.delegate()( ev.ptr0() );
         }  break;
         case Command::TYPE_MATERIAL_COLOR:
         {
            const CmdMaterialColor& ev = CmdMaterialColor::as( cmd );
            Entity* e = ev.entity();
            uint id = e->geometry()->patchInfo(0).materialID();
            Material* mat = e->materialSet()->material( id );
            if( mat->type() == Material::BASE )
            {
               BaseMaterial* bmat = (BaseMaterial*)mat;
               bmat->color( ev.color() );
            }
            else
            {
               StdErr << "CmdMaterialColor only works on BaseMaterial on the first patch." << nl;
            }
         }  break;

         case Command::TYPE_RIGID_BODY_TYPE:
         {
            const CmdRigidBodyType& ev = CmdRigidBodyType::as( cmd );
            RigidEntity* e = ev.entity();
            RigidBody::Type type = (RigidBody::Type)ev.type();
            switch( type )
            {
               case RigidBody::STATIC   : e->makeStatic(); break;
               case RigidBody::DYNAMIC  : e->makeDynamic(); break;
               case RigidBody::KINEMATIC: e->makeKinematic(); break;
               default:
                  StdErr << "Invalid RigidBody type: " << type << "." << nl;
                  CHECK( false );
                  break;
            }
         }  break;

         case Command::TYPE_STIMULATE:
         {
            const CmdStimulate& ev = CmdStimulate::as( cmd );
            ev.entity()->stimulate( ev.stimulus() );
         }  break;

         default:
            StdErr << "ERROR - World::handleCommands() doesn't know about type " << cmd.type() << "." << nl;
            break;
      }
   }
}

//------------------------------------------------------------------------------
//!
void
World::runCommandsAfterBrains()
{
   PROFILE_EVENT_C( EventProfiler::WORLD_BEGIN_COMMANDS_AFTER_BRAINS, _commands.size() );
   handleCommands( _commands );
   _commands.clear();
}

//------------------------------------------------------------------------------
//! Analyzes all of the commands.
void
World::runCommandsAfterActions()
{
   PROFILE_EVENT_C( EventProfiler::WORLD_BEGIN_COMMANDS_AFTER_ACTIONS, _commands.size() );
   handleCommands( _commands );
   _commands.clear();
}

//------------------------------------------------------------------------------
//! Analyzes all of the commands.
void
World::runCommandsAfterPhysics()
{
   PROFILE_EVENT_C( EventProfiler::WORLD_BEGIN_COMMANDS_AFTER_PHYSICS, _commands.size() );
   handleCommands( _commands );
   _commands.clear();
}

//------------------------------------------------------------------------------
//!
void
World::submit( const Command& cmd )
{
   LockGuard g( _lock );
   _commands.pushBack( cmd );
}

//-----------------------------------------------------------------------------
//!
void
World::runActions( double t, double d, bool afterPhysics )
{
   EntityPtrContainer& entities = afterPhysics ? _entitiesWithActionsAfter : _entitiesWithActionsBefore;
#if PROFILE_EVENTS
   if( afterPhysics )
   {
      PROFILE_EVENT_C( EventProfiler::WORLD_BEGIN_ACTIONS_AFTER_PHYSICS, entities.size() );
   }
   else
   {
      PROFILE_EVENT_C( EventProfiler::WORLD_BEGIN_ACTIONS, entities.size() );
   }
#endif
   if( !entities.empty() )
   {
      TaskQueue* queue = Plasma::dispatchQueue();

      // Post one task for each entity that need to run actions.
      for( auto cur = entities.begin(); cur != entities.end(); ++cur )
      {
         Entity* e = (*cur);
         ActionTask* task = new ActionTask( e, t, d, afterPhysics );
         queue->post( task );
      }
      queue->waitForAll();

      // Clean up entities that no longer have actions.
      for( auto cur = entities.begin(); cur != entities.end(); ++cur )
      {
         Entity*& e = (*cur);
         bool runningActions = afterPhysics ? e->runningActionsAfter() : e->runningActionsBefore();
         if( !runningActions )  e = NULL;
      }
      entities.removeAllSwap( NULL );
   }
}

//-----------------------------------------------------------------------------
//!
void
World::markForActionBefore( Entity* e )
{
   if( !e->runningActionsBefore() )
   {
      _entitiesWithActionsBefore.pushBack( e );
      e->runningActionsBefore( true );
   }
}

//-----------------------------------------------------------------------------
//!
void
World::markForActionAfter( Entity* e )
{
   if( !e->runningActionsAfter() )
   {
      _entitiesWithActionsAfter.pushBack( e );
      e->runningActionsAfter( true );
   }
}

#if 0
//------------------------------------------------------------------------------
//! Analyzes all of the events to find sound-emitting ones.
//! Eventually, this one will handle distance of objects, etc.
void
World::emitSounds( double /*time*/, double /*delta*/ )
{
   if( silent() )  return;

   WorldEventContainer::Iterator cur = _events.begin();
   WorldEventContainer::Iterator end = _events.end();
   for( ; cur != end; ++cur )
   {
      const WorldEvent& event = (*cur);
      switch( event.type() )
      {
         case WorldEvent::TYPE_COLLISION:
         {
            const WECollision& ev = WECollision::as( event );
            if( ev->force() > 0.0f )
            {
               Plasma::soundManager()->genCollision( ev->location(), ev->force() );
            }
         }  break;
      }
   }
}
#endif

//------------------------------------------------------------------------------
//!
void
World::updateSounds()
{
   // Update bound sources.
   for( auto cur = _sndBoundSources.begin(); cur != _sndBoundSources.end(); ++cur )
   {
      RigidEntity* re          = cur->first;
      List<Snd::Source*>& list = cur->second;

      for( auto curSrc = list.begin(); curSrc != list.end(); ++curSrc )
      {
         Snd::Source& src = *(*curSrc);
         src.position( re->position() );
         src.velocity( re->linearVelocity() );
         src.update();
      }
   }

   // Update bound listeners.
   for( auto cur = _sndBoundListeners.begin(); cur != _sndBoundListeners.end(); ++cur )
   {
      RigidEntity* re            = cur->first;
      List<Snd::Listener*>& list = cur->second;

      for( auto curLis = list.begin(); curLis != list.end(); ++curLis )
      {
         Snd::Listener& lis = *(*curLis);
         const Reff& ref = re->referential();
         Vec3f yAxis, zAxis;
         ref.orientation().getAxesYZ( yAxis, zAxis );
         lis.position( ref.position() );
         lis.pointOfInterest( ref.position() - zAxis );
         lis.upVector( yAxis );
         lis.velocity( re->linearVelocity() );
         lis.update();
      }
   }
}

//------------------------------------------------------------------------------
//!
void
World::updateSoundStates( double oldSpeed, double newSpeed )
{
   if( oldSpeed == 1.0 )
   {
      // Register the old pitch for every source.
      for( auto cur = _sndSources.begin(); cur != _sndSources.end(); ++cur )
      {
         Snd::Source&   src  = *(cur->first);
         SndSourceData& data = cur->second;
         data._oldPitch      = src.pitch();
      }
   }

   if( oldSpeed == newSpeed )  return;

   if( oldSpeed == 0.0 )
   {
      // Resume all previously playing sounds.
      for( auto cur = _sndSources.begin(); cur != _sndSources.end(); ++cur )
      {
         Snd::Source&   src  = *(cur->first);
         SndSourceData& data = cur->second;
         if( data._wasPlaying )
         {
            src.play();
         }
      }
   }

   if( newSpeed == 0.0 )
   {
      // Pause all of the sounds.
      for( auto cur = _sndSources.begin(); cur != _sndSources.end(); ++cur )
      {
         Snd::Source&   src  = *(cur->first);
         SndSourceData& data = cur->second;
         data._wasPlaying    = src.playing();
         //data._oldPitch    = src.pitch();
         src.pause();
      }
   }
   else
   {
      // Set a pitch proportional to the simulation speed.
      // FIXME: Doesn't play well with user-controller pitches.
      for( auto cur = _sndSources.begin(); cur != _sndSources.end(); ++cur )
      {
         Snd::Source&   src  = *(cur->first);
         SndSourceData& data = cur->second;
         src.pitch( float(data._oldPitch * newSpeed) );
      }
   }
}

//------------------------------------------------------------------------------
//!
void
World::newMotionWorld()
{
   _world = new MotionWorld();
   _world->simulationRate( Plasma::physicsFPS() );
   _world->addAttractor( _gravityAttractor.ptr() );

   _world->sensationBeginCallback( makeDelegate( this, &World::sensationBeginCallback ) );
   _world->sensationEndCallback( makeDelegate( this, &World::sensationEndCallback ) );
}

//------------------------------------------------------------------------------
//!
void
World::sensationBeginCallback( const CollisionPair& cp )
{
   RigidEntity* entA = (RigidEntity*)cp.bodyA()->userRef();
   RigidEntity* entB = (RigidEntity*)cp.bodyB()->userRef();

   // Filter contact receptors.
#if 0
   for( uint i = 0; i < _contactReceptors.size(); ++i )
   {
      auto& er = _contactReceptors[i];
      if( er._receptor->interestedInA( entA, entB ) )
      {
         er._entity->stimulate( new ContactBeginStimulus( entA, entB, cp.worldPosition(), 0.0f ) );
      }
      else
      if( er._receptor->interestedInB( entA, entB ) )
      {
         er._entity->stimulate( new ContactBeginStimulus( entB, entA, cp.worldPosition(), 0.0f ) );
      }
   }
#else
   if( entA->canSense( *entB ) )
   {
      auto it = _contactReceptors.find( entA );
      if( it != _contactReceptors.end() )
      {
         RCP<Stimulus> stim = new ContactBeginStimulus( entA, entB, cp.worldPosition(), 0.0f );
         ContactReceptorList& list = it.data();
         for( auto cur = list.begin(); cur != list.end(); ++cur )
         {
            (*cur)._entity->stimulate( stim.ptr() );
         }
      }
   }
   if( entB->canSense( *entA ) )
   {
      auto it = _contactReceptors.find( entB );
      if( it != _contactReceptors.end() )
      {
         RCP<Stimulus> stim = new ContactBeginStimulus( entB, entA, cp.worldPosition(), 0.0f );
         ContactReceptorList& list = it.data();
         for( auto cur = list.begin(); cur != list.end(); ++cur )
         {
            (*cur)._entity->stimulate( stim.ptr() );
         }
      }
   }
#endif

   // Filter contact group receptors.
   // for( uint i = 0; i < _contactGroupReceptors.size(); ++i )
   // {
   //    EntityReceptor<ContactGroupReceptor>& er = _contactGroupReceptors[i];
   //    if( er._receptor->interestedInA( entA, entB ) )
   //    {
   //       er._entity->stimulate( new ContactBeginStimulus( entA, entB, cp.worldPosition(), 0.0f ) );
   //    }
   //    else
   //    if( er._receptor->interestedInB( entA, entB ) )
   //    {
   //       er._entity->stimulate( new ContactBeginStimulus( entB, entA, cp.worldPosition(), 0.0f ) );
   //    }
   // }
}

//------------------------------------------------------------------------------
//!
void
World::sensationEndCallback( const CollisionPair& cp )
{
   RigidEntity* entA = (RigidEntity*)cp.bodyA()->userRef();
   RigidEntity* entB = (RigidEntity*)cp.bodyB()->userRef();

   // Filter contact receptors.
#if 0
   for( uint i = 0; i < _contactReceptors.size(); ++i )
   {
      auto& er = _contactReceptors[i];
      if( er._receptor->interestedInA( entA, entB ) )
      {
         er._entity->stimulate( new ContactEndStimulus( entA, entB ) );
      }
      else
      if( er._receptor->interestedInB( entA, entB ) )
      {
         er._entity->stimulate( new ContactEndStimulus( entB, entA ) );
      }
   }
#else
   if( entA->canSense( *entB ) )
   {
      auto it = _contactReceptors.find( entA );
      if( it != _contactReceptors.end() )
      {
         RCP<Stimulus> stim = new ContactEndStimulus( entA, entB );
         ContactReceptorList& list = it.data();
         for( auto cur = list.begin(); cur != list.end(); ++cur )
         {
            (*cur)._entity->stimulate( stim.ptr() );
         }
      }
   }
   if( entB->canSense( *entA ) )
   {
      auto it = _contactReceptors.find( entB );
      if( it != _contactReceptors.end() )
      {
         RCP<Stimulus> stim = new ContactEndStimulus( entB, entA );
         ContactReceptorList& list = it.data();
         for( auto cur = list.begin(); cur != list.end(); ++cur )
         {
            (*cur)._entity->stimulate( stim.ptr() );
         }
      }
   }
#endif

   // Filter contact group receptors.
   // for( uint i = 0; i < _contactGroupReceptors.size(); ++i )
   // {
   //    EntityReceptor<ContactGroupReceptor>& er = _contactGroupReceptors[i];
   //    if( er._receptor->interestedInA( entA, entB ) )
   //    {
   //       er._entity->stimulate( new ContactEndStimulus( entA, entB ) );
   //    }
   //    else
   //    if( er._receptor->interestedInB( entA, entB ) )
   //    {
   //       er._entity->stimulate( new ContactEndStimulus( entB, entA ) );
   //    }
   // }
}

//-----------------------------------------------------------------------------
//!
void
World::pick( PickingData& data )
{
   Vector<Rayf>  rays;
   MotionWorld::IntersectionData  dst;
   // Convert the rays for MotionWorld.
   for( auto cur = data.begin(), end = data.end(); cur != end; ++cur )
   {
      rays.pushBack( (*cur).ray() );
   }
   _world->raycast( rays, RigidBody::ANY_MASK, 0x1, dst );
   // Convert MotionWorld back to World.
   CHECK( dst.size() == data.size() );
   uint n = uint(data.size());
   for( uint i = 0; i < n; ++i )
   {
      auto& entry = data.entry(i);
      RigidBody* body = dst.body( i );
      if( body )
      {
         entry._entity   = (Entity*)body->userRef();
         entry._location = dst.location( i );
      }
      else
      {
         entry._entity   = NULL;
      }
   }
}

//-----------------------------------------------------------------------------
//!
void
World::debugGeometry( DebugGeometry* g )
{
   _debugGeometry = g;
}

//------------------------------------------------------------------------------
//!
const char*
World::meta() const
{
   return _world_str;
}

NAMESPACE_END
