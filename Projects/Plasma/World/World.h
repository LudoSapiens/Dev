/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_WORLD_H
#define PLASMA_WORLD_H

#include <Plasma/StdDefs.h>

#if MOTION_BULLET
#include <MotionBullet/Attractor/BasicAttractors.h>
#include <MotionBullet/Collision/CollisionInfo.h>
#include <MotionBullet/World/MotionWorld.h>
#else
#include <Motion/Attractor/BasicAttractors.h>
#include <Motion/Collision/CollisionInfo.h>
#include <Motion/World/MotionWorld.h>
#endif

#include <Snd/Listener.h>
#include <Snd/Source.h>

#include <Fusion/Core/Animator.h>
#include <Fusion/Core/Core.h>

#include <CGMath/Vec3.h>

#include <Base/ADT/HashTable.h>
#include <Base/ADT/List.h>
#include <Base/ADT/Map.h>
#include <Base/ADT/Vector.h>
#include <Base/MT/Lock.h>
#include <Base/Util/Bits.h>
#include <Base/Util/RCP.h>


NAMESPACE_BEGIN

class Animator;
class Brain;
class Camera;
class Command;
class ContactGroupReceptor;
class ContactReceptor;
class DebugGeometry;
class Entity;
class EntityGroup;
class Light;
class Observer;
class Probe;
class Receptor;
class Renderable;
class RigidEntity;
class Selection;
class SkeletalEntity;
class Stimulus;

/*==============================================================================
  CLASS SafeSet
==============================================================================*/
template< typename T >
class SafeSet
{
public:

   /*----- types -----*/

   typedef Set< T >  Container;

   /*----- methods -----*/

   inline void  add( const T& t );
   inline void  drain( Container& c );
   inline void  clear() { LockGuard g( _lock ); _data.clear(); }

protected:

   /*----- data members -----*/

   Container  _data;
   Lock       _lock;

private:
}; //class SafeSet

//-----------------------------------------------------------------------------
//!
template< typename T> void
SafeSet<T>::add( const T& t )
{
   LockGuard g( _lock );
   _data.add( t );
}

//-----------------------------------------------------------------------------
//!
template< typename T> void
SafeSet<T>::drain( Container& c )
{
   Container().swap(c); // We use this trick to guarantee the memory is deallocated; c.clear() doesn't.
   LockGuard g( _lock );
   c.swap( _data );
}


/*==============================================================================
  CLASS World
==============================================================================*/

//! Main Container for 3D.

class World:
   public RCObject,
   public VMProxy
{

public:

   /*----- types and enumerations ----*/

   typedef Vector< RCP<Entity> >          EntityContainer;
   typedef SafeSet< RCP<Entity> >         EntitySet;
   typedef Vector< RCP<EntityGroup> >     GroupContainer;
   typedef Vector< Light* >               LightContainer;
   typedef Vector< Camera* >              CameraContainer;
   typedef Vector< RCP<Selection> >       SelectionContainer;
   typedef Vector< Command >              CommandContainer;
   typedef Vector< RCP<Renderable> >      RenderableContainer;

   /*==============================================================================
     CLASS PickingData
   ==============================================================================*/
   class PickingData
   {
   public:

      /*----- types -----*/

      struct Entry
      {
         Entry(): _entity(NULL) {}

         inline const   Rayf&       ray() const { return _ray;      }

         inline bool  hasHit() const { return _entity != NULL; }

         inline       Entity*    entity()       { return _entity;   }
         inline const Entity*    entity() const { return _entity;   }
         inline const  Vec3f&  location() const { return _location; }

         Rayf     _ray;
         Entity*  _entity;
         Vec3f    _location;
      };

      /*----- methods -----*/

      PickingData( uint numRays ): _entries( numRays ) {}

      inline size_t  size() const     { return _entries.size();    }
      inline void  resize( size_t n ) { return _entries.resize(n); }

      inline Entry&  entry( uint i ) { return _entries[i]; }
      inline Vector<Entry>::ConstIterator  begin() const { return _entries.begin(); }
      inline Vector<Entry>::ConstIterator    end() const { return _entries.end();   }

      inline void  setRay( uint idx, const Rayf& ray ) { CHECK( idx < _entries.size() ); _entries[idx]._ray = ray; }

      inline void  print() const
      {
         StdErr << size() << " entries:" << nl;
         for( auto c = begin(), e = end(); c != e; ++c )
         {
            StdErr << " ... " << (*c).ray() << " --> " << (void*)(*c).entity() << " at " << (*c).location() << nl;
         }
      }

   protected:

      /*----- data members -----*/

      Vector<Entry>  _entries;

   }; //class PickingData


   /*----- static methods -----*/

   static void initialize();

   /*----- methods -----*/

   PLASMA_DLL_API World();

   PLASMA_DLL_API void clear();

   // Entities.
   PLASMA_DLL_API void addEntity( Entity* e );
   PLASMA_DLL_API void removeEntity( Entity* e );
   PLASMA_DLL_API void removeAllEntities();
   inline uint numEntities() const             { return uint(_entities.size()); }
   inline Entity* entity( uint i ) const       { return _entities[i].ptr(); }
   PLASMA_DLL_API Entity* entity( const ConstString& ) const;

   PLASMA_DLL_API AABBoxf boundingBox() const;

   // Lights.
   inline uint numLights() const               { return uint(_lights.size()); }
   inline Light* light( uint i ) const         { return _lights[i]; }
   PLASMA_DLL_API Light* light( const ConstString& ) const;

   // Cameras.
   inline uint numCameras() const              { return uint(_cameras.size());  }
   inline Camera* camera( uint i ) const       { return _cameras[i]; }
   PLASMA_DLL_API Camera* camera( const ConstString& ) const;

   // Groups.
   PLASMA_DLL_API void addGroup( EntityGroup* );
   PLASMA_DLL_API void removeGroup( EntityGroup* );
   PLASMA_DLL_API void removeAllGroups();
   inline uint numGroups() const               { return uint(_groups.size()); }
   inline EntityGroup* group( uint i ) const   { return _groups[i].ptr(); }

   // Renderable.
   PLASMA_DLL_API void addRenderable( Renderable* );
   PLASMA_DLL_API void removeRenderable( Renderable* );
   PLASMA_DLL_API void removeAllRenderable();
   inline uint numRenderables() const            { return uint(_renderables.size()); }
   inline Renderable* renderable( uint i ) const { return _renderables[i].ptr(); }

   // Selection.
   PLASMA_DLL_API void addSelection( Selection* );
   PLASMA_DLL_API void removeSelection( Selection* );
   PLASMA_DLL_API void removeAllSelections();
   inline uint numSelections() const            { return uint(_selections.size()); }
   inline Selection* selection( uint i ) const  { return _selections[i].ptr(); }

   // Constraint.
   PLASMA_DLL_API BallJoint* createBallJoint( RigidEntity*, RigidEntity* );
   PLASMA_DLL_API HingeJoint* createHingeJoint( RigidEntity*, RigidEntity* );
   PLASMA_DLL_API void addConstraint( Constraint* );
   PLASMA_DLL_API void removeConstraint( Constraint* );
   PLASMA_DLL_API void removeAllConstraints();

   // Physics and simulation.
   PLASMA_DLL_API void startSimulation();
   PLASMA_DLL_API void stopSimulation();

   inline MotionWorld* motionWorld() const { return _world.ptr(); }

   PLASMA_DLL_API void gravity( const Vec3f& );
   PLASMA_DLL_API const Vec3f& gravity() const;
   PLASMA_DLL_API void simulationSpeed( double speed );
   inline double simulationSpeed() const         { return _simulationSpeed; }
   inline double time() const                    { return _world->time(); }

   // Background.
   inline void backgroundColor( const Vec4f& c ) { _bgColor = c; }
   inline const Vec4f& backgroundColor() const   { return _bgColor; }

   inline Table& attributes() const              { return *_attributes; }

   // Animator.
   PLASMA_DLL_API void addAnimator( Animator* a );
   PLASMA_DLL_API void addAnimator( Animator* a, double startTime );
   PLASMA_DLL_API void removeAnimator( Animator* a );

   // Brains.
   PLASMA_DLL_API void scheduleAnalyze( Entity* e );
   PLASMA_DLL_API void registerReceptor( Entity* e, Receptor* r );
   PLASMA_DLL_API void unregisterReceptor( Entity* e, Receptor* r );
   PLASMA_DLL_API void handleCommands( const CommandContainer& cmds );
   PLASMA_DLL_API void markForActionBefore( Entity* e );
   PLASMA_DLL_API void markForActionAfter( Entity* e );

   // Sounds.
   inline void addSoundSource     ( Snd::Source*   src ) { _sndSources[src].read(src); if( simulationSpeed() == 0.0 )  src->pause(); }
   inline void addSoundListener   ( Snd::Listener* lis ) { _sndListeners.add(lis);    }
   inline void removeSoundSource  ( Snd::Source*   src ) { _sndSources.erase(src);    }
   inline void removeSoundListener( Snd::Listener* lis ) { _sndListeners.remove(lis); }
   inline RCP<Snd::Source>    createSoundSource();
   inline RCP<Snd::Listener>  createSoundListener();
   PLASMA_DLL_API void bind  ( RigidEntity* e, Snd::Source*   src );
   PLASMA_DLL_API void bind  ( RigidEntity* e, Snd::Listener* lis );
   PLASMA_DLL_API void unbind( RigidEntity* e, Snd::Source*   src );
   PLASMA_DLL_API void unbind( RigidEntity* e, Snd::Listener* lis );
   static inline Snd::Source*    ALL_SND_SOURCES()   { return (Snd::Source*)NULL;   }
   static inline Snd::Listener*  ALL_SND_LISTENERS() { return (Snd::Listener*)NULL; }

   // Probes.
   PLASMA_DLL_API   void  addProbe( Probe* p );
   PLASMA_DLL_API   void  removeProbe( Probe* p );
   inline           uint  numProbes() const { return uint(_probes.size()); }
   inline         Probe*  probe( uint idx ) const { return _probes[idx].ptr(); }
   PLASMA_DLL_API Probe*  findBestProbe( const Vec3f& pos ) const;

   // State.
   inline bool silent() const   { return getbits(_state, 0, 1) != 0; }
   inline void silent( bool v ) { _state = setbits(_state, 0, 1, v?1:0); }

   // Picking.
   PLASMA_DLL_API void  pick( PickingData& data );

   inline const RCP<DebugGeometry>&  debugGeometry() const { return _debugGeometry; }
   PLASMA_DLL_API void  debugGeometry( DebugGeometry* g );

   // VM.
   virtual const char*  meta() const;

   // Thread safe methods.
   PLASMA_DLL_API void  submit( const Command& cmd );
   //PLASMA_DLL_API void  submit( const Vector< Command >& cmds );

protected:

   /*----- friends -----*/

   friend class WorldAnimator;
   friend class Entity;

   /*----- methods -----*/

   virtual ~World();

   void updateID( Entity*, const ConstString& newID );

   void runActions( double t, double d, bool afterPhysics );
   void runPhysics( double step );
   void runBrains( double step );
   void runAnimators( double time, double delta );
   void runCommandsAfterBrains();
   void runCommandsAfterActions();
   void runCommandsAfterPhysics();

   //void emitSounds( double time, double delta );
   void updateSounds();
   void updateSoundStates( double oldSpeed, double newSpeed );

   void newMotionWorld();
   void sensationBeginCallback( const CollisionPair& cp );
   void sensationEndCallback( const CollisionPair& cp );

   static void getTransform( const void* userData, Reff& dstRef, Mat4f& dstMat );
   static void setTransform( void* userData, const Reff& srcRef, const Mat4f& srcMat );

   /*----- data types -----*/

   // FIXME: Create a container class to hide the EntityReceptor pair class completely.
   // Template that class with only the Receptor class type.
   template< class R >
   struct EntityReceptor
   {
      Entity*    _entity;
      R*         _receptor;
      EntityReceptor( Entity* e, R* r ): _entity(e), _receptor(r) {}
      bool  operator==( const EntityReceptor& er ) { return _entity == er._entity && _receptor == er._receptor; }
   };

   typedef Vector< EntityReceptor<ContactReceptor> >       ContactReceptorList;
   typedef HashTable< void*, ContactReceptorList >         ContactReceptorContainer;
   typedef Vector< EntityReceptor<ContactGroupReceptor> >  ContactGroupReceptorContainer;

   struct SndSourceData
   {
      bool   _wasPlaying;
      float  _oldPitch;
      SndSourceData(){}
      void  read( const Snd::Source* src )
      {
         _wasPlaying = src->playing();
         _oldPitch   = src->pitch();
      }
   };

   typedef Vector< RCP<Animator> >                    AnimatorContainer;
   typedef Map< RCP<Snd::Source>, SndSourceData >     SndSourceContainer;
   typedef Set< RCP<Snd::Listener> >                  SndListenerContainer;
   typedef Map< RigidEntity*, List<Snd::Source*> >    SndEntitySourceMap;
   typedef Map< RigidEntity*, List<Snd::Listener*> >  SndEntityListenerMap;
   typedef Vector< Entity* >                          EntityPtrContainer;
   typedef Map< ConstString, Entity* >                EntityIDContainer;
   typedef Vector< RCP<Probe> >                       ProbeContainer;

   /*----- data members -----*/

   RCP<MotionWorld>          _world;
   RCP<DirectionalAttractor> _gravityAttractor;

   // Descriptions.
   EntityContainer           _entities;
   LightContainer            _lights;
   CameraContainer           _cameras;
   RenderableContainer       _renderables;

   AnimatorContainer         _animators;
   AnimatorContainer         _newAnimators;
   GroupContainer            _groups;
   SelectionContainer        _selections;
   EntitySet                 _entitiesToAnalyze;
   EntityPtrContainer        _entitiesWithActionsBefore;
   EntityPtrContainer        _entitiesWithActionsAfter;
   EntityIDContainer         _entityIDs;
   SndSourceContainer        _sndSources;
   SndListenerContainer      _sndListeners;
   SndEntitySourceMap        _sndBoundSources;
   SndEntityListenerMap      _sndBoundListeners;
   CommandContainer          _commands;
   ProbeContainer            _probes;

   // Receptors.
   ContactReceptorContainer       _contactReceptors;
   ContactGroupReceptorContainer  _contactGroupReceptors;

   bool                      _physicsLooped;

   // Parameters.
   Vec3f                     _gravity;
   Vec4f                     _bgColor;
   RCP<Table>                _attributes;

   // Time.
   double                    _simulationSpeed;

   uint  _state; //!< A series of bits packed together.
   //! silent     (1)  _state[ 0: 0]    //!< Set when world should not emit any sound.

   Lock                      _lock;

   RCP<DebugGeometry>        _debugGeometry;  //!< Some points, lines, and triangles used to debug information.
};

//------------------------------------------------------------------------------
//!
RCP<Snd::Source>
World::createSoundSource()
{
   RCP<Snd::Source> src = Core::snd()->createSource();
   addSoundSource( src.ptr() );
   return src;
}

//------------------------------------------------------------------------------
//!
RCP<Snd::Listener>
World::createSoundListener()
{
   RCP<Snd::Listener> lis = Core::snd()->createListener();
   addSoundListener( lis.ptr() );
   return lis;
}

NAMESPACE_END

#endif
