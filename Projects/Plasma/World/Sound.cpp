/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/World/Sound.h>

#include <Plasma/World/Entity.h>
#include <Plasma/Plasma.h>
#include <Plasma/Resource/ResManager.h>

#include <Base/ADT/StringMap.h>
#include <Base/Dbg/DebugStream.h>


USING_NAMESPACE


/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_snd, "Sound" );


//------------------------------------------------------------------------------
//!
enum
{
   ATTRIB_AUTO_DELETE,
   ATTRIB_FREQUENCY,
   ATTRIB_GAIN,
   ATTRIB_GEN_COLLISION,
   ATTRIB_LOOPING,
   ATTRIB_OBSERVE,
   ATTRIB_PAUSE,
   ATTRIB_PITCH,
   ATTRIB_PLAY,
   ATTRIB_PLAYING,
   ATTRIB_POINT_OF_INTEREST,
   ATTRIB_POSITION,
   ATTRIB_RELATIVE,
   ATTRIB_REWIND,
   ATTRIB_SET_BGM,
   ATTRIB_SET_DEFAULTS,
   ATTRIB_SET_SOUND,
   ATTRIB_STOP,
   ATTRIB_STOPPED,
   ATTRIB_UP_VECTOR,
   ATTRIB_UPDATE,
   ATTRIB_VELOCITY,
   NUM_ATTRIBS
};

StringMap _attributes(
   "autoDelete",       ATTRIB_AUTO_DELETE,
   "frequency",        ATTRIB_FREQUENCY,
   "gain",             ATTRIB_GAIN,
   "genCollision",     ATTRIB_GEN_COLLISION,
   "looping",          ATTRIB_LOOPING,
   "observe",          ATTRIB_OBSERVE,
   "pause",            ATTRIB_PAUSE,
   "pitch",            ATTRIB_PITCH,
   "play",             ATTRIB_PLAY,
   "playing",          ATTRIB_PLAYING,
   "pointOfInterest",  ATTRIB_POINT_OF_INTEREST,
   "position",         ATTRIB_POSITION,
   "relative",         ATTRIB_RELATIVE,
   "rewind",           ATTRIB_REWIND,
   "setBGM",           ATTRIB_SET_BGM,
   "setDefaults",      ATTRIB_SET_DEFAULTS,
   "setSound",         ATTRIB_SET_SOUND,
   "stop",             ATTRIB_STOP,
   "stopped",          ATTRIB_STOPPED,
   "update",           ATTRIB_UPDATE,
   "upVector",         ATTRIB_UP_VECTOR,
   "velocity",         ATTRIB_VELOCITY,
   ""
);

#define DECLARE_VM_ROUTINE( funcName, ptrType, ptrFunc ) \
   int funcName( VMState* vm ) \
   { \
      DBG_BLOCK( os_snd, #funcName ); \
      ptrType* ptr = (ptrType*)VM::thisPtr(vm); \
      CHECK( ptr != NULL ); \
      ptr->ptrFunc(); \
      return 0; \
   }

// SoundSource
DECLARE_VM_ROUTINE( source_pauseVM,       SoundSource, pause       )
DECLARE_VM_ROUTINE( source_playVM,        SoundSource, play        )
DECLARE_VM_ROUTINE( source_rewindVM,      SoundSource, rewind      )
DECLARE_VM_ROUTINE( source_setBGMVM,      SoundSource, setBGM      )
DECLARE_VM_ROUTINE( source_setDefaultsVM, SoundSource, setDefaults )
DECLARE_VM_ROUTINE( source_stopVM,        SoundSource, stop        )
DECLARE_VM_ROUTINE( source_updateVM,      SoundSource, update      )

int source_setSoundVM( VMState* vm )
{
   DBG_BLOCK( os_snd, "source_setSoundVM" );
   SoundSource* src = (SoundSource*)VM::thisPtr(vm);
   CHECK( src != NULL );
   RCP<Sound> snd( SoundVM::to(vm, 1) );
   CHECK( snd.isValid() );
   src->setSound( snd.ptr() );
   return 0;
}

int source_observeVM( VMState* vm )
{
   DBG_BLOCK( os_snd, "source_observeVM" );
   SoundSource* src = (SoundSource*)VM::thisPtr(vm);
   CHECK( src != NULL );
   RCP<Entity> ent( EntityVM::to(vm, 1) );
   CHECK( ent.isValid() );
   src->observe( ent.ptr() );
   return 0;
}

// SoundListener
DECLARE_VM_ROUTINE( listener_updateVM,      SoundListener, update      )
DECLARE_VM_ROUTINE( listener_setDefaultsVM, SoundListener, setDefaults )

int listener_observeVM( VMState* vm )
{
   DBG_BLOCK( os_snd, "listener_observeVM" );
   SoundListener* lis = (SoundListener*)VM::thisPtr(vm);
   CHECK( lis != NULL );
   RCP<Entity> ent( EntityVM::to(vm, 1) );
   CHECK( ent.isValid() );
   lis->observe( ent.ptr() );
   return 0;
}

// SoundManager
int manager_genCollisionVM( VMState* vm )
{
   DBG_BLOCK( os_snd, "manager_genCollision" );
   SoundManager* mgr = (SoundManager*)VM::thisPtr(vm);
   CHECK( mgr != NULL );
   Vec3f pos = VM::toVec3f(vm, 1);
   float f = (float)VM::toNumber(vm, 2);
   // matA, matB
   mgr->genCollision( pos, f );
   return 0;
}

#undef DECLARE_VM_ROUTINE



UNNAMESPACE_END


/*==============================================================================
  CLASS Sound
==============================================================================*/

//------------------------------------------------------------------------------
//!
Sound::Sound()
{
   _sound = Plasma::soundManager()->sndMgr()->createSound();
}

//------------------------------------------------------------------------------
//!
Sound::~Sound()
{
}

//------------------------------------------------------------------------------
//!
void
Sound::init( VMState* vm )
{
   DBG_BLOCK( os_snd, "Sound::init" );
   if( VM::isTable(vm, -1) )
   {
      // Start iterating at index 0 (nil, pushed below).
      VM::push(vm);
      while( VM::next(vm, -2) )
      {
         // Let the performSet() routine handle the various assignments.
         performSet(vm);

         // Pop the value, but keep the key.
         VM::pop(vm, 1);
      }

      // Creation-only parameters.
   }
}

//------------------------------------------------------------------------------
//!
bool
Sound::performGet( VMState* vm )
{
   DBG_BLOCK( os_snd, "Sound::performGet" );
   const char* str = VM::toCString( vm, -1 );
   switch( _attributes[str] )
   {
      case ATTRIB_FREQUENCY:
         printf("Freq=%g\n", getDuration());
         VM::push( vm, getDuration() );
         return true;
      default:
         break;
   }

   return false;
}

//------------------------------------------------------------------------------
//!
bool
Sound::performSet( VMState* vm )
{
   DBG_BLOCK( os_snd, "Sound::performSet" );
   const char* str = VM::toCString( vm, -2 );
   switch( _attributes[str] )
   {
      case ATTRIB_FREQUENCY:
         // Read-only.
         return true;
      default:
         break;
   }

   return false;
}


/*==============================================================================
  CLASS SoundSource
==============================================================================*/

//------------------------------------------------------------------------------
//!
SoundSource::SoundSource():
   _entity( NULL )
{
   _source = Plasma::soundManager()->sndMgr()->createSource();
}

//------------------------------------------------------------------------------
//!
SoundSource::~SoundSource()
{
   if( _entity )
   {
      _entity->positionSubject().detach( this );
   }
}

//------------------------------------------------------------------------------
//!
void
SoundSource::init( VMState* vm )
{
   DBG_BLOCK( os_snd, "SoundSource::init" );
   if( VM::isTable(vm, -1) )
   {
      // Start iterating at index 0 (nil, pushed below).
      VM::push(vm);
      while( VM::next(vm, -2) )
      {
         // Let the performSet() routine handle the various assignments.
         performSet(vm);

         // Pop the value, but keep the key.
         VM::pop(vm, 1);
      }

      // Creation-only parameters.
   }
}

//------------------------------------------------------------------------------
//!
bool
SoundSource::performGet( VMState* vm )
{
   DBG_BLOCK( os_snd, "SoundSource::performGet" );
   const char* str = VM::toCString( vm, -1 );
   switch( _attributes[str] )
   {
      case ATTRIB_AUTO_DELETE:
         VM::push( vm, autoDelete() );
         return true;
      case ATTRIB_GAIN:
         VM::push( vm, gain() );
         return true;
      case ATTRIB_LOOPING:
         VM::push( vm, looping() );
         return true;
      case ATTRIB_OBSERVE:
         VM::push( vm, this, source_observeVM );
         return true;
      case ATTRIB_PAUSE:
         VM::push( vm, this, source_pauseVM );
         return true;
      case ATTRIB_PITCH:
         VM::push( vm, pitch() );
         return true;
      case ATTRIB_PLAY:
         VM::push( vm, this, source_playVM );
         return true;
      case ATTRIB_PLAYING:
         VM::push( vm, playing() );
         return true;
      case ATTRIB_POSITION:
         VM::push( vm, position() );
         return true;
      case ATTRIB_RELATIVE:
         VM::push( vm, relative() );
         return true;
      case ATTRIB_REWIND:
         VM::push( vm, this, source_rewindVM );
         return true;
      case ATTRIB_SET_BGM:
         VM::push( vm, this, source_setBGMVM );
         return true;
      case ATTRIB_SET_DEFAULTS:
         VM::push( vm, this, source_setDefaultsVM );
         return true;
      case ATTRIB_SET_SOUND:
         VM::push( vm, this, source_setSoundVM );
         return true;
      case ATTRIB_STOP:
         VM::push( vm, this, source_stopVM );
         return true;
      case ATTRIB_STOPPED:
         VM::push( vm, stopped() );
         return true;
      case ATTRIB_UPDATE:
         VM::push( vm, this, source_updateVM );
         return true;
      case ATTRIB_VELOCITY:
         VM::push( vm, velocity() );
         return true;
      default:
         break;
   }

   return false;
}

//------------------------------------------------------------------------------
//!
bool
SoundSource::performSet( VMState* vm )
{
   DBG_BLOCK( os_snd, "SoundSource::performSet" );
   const char* str = VM::toCString( vm, -2 );
   switch( _attributes[str] )
   {
      case ATTRIB_AUTO_DELETE:
         autoDelete( VM::toBoolean( vm, 1 ) );
         return true;
      case ATTRIB_GAIN:
         gain( (float)VM::toNumber( vm, 1 ) );
         return true;
      case ATTRIB_LOOPING:
         looping( VM::toBoolean( vm, 1 ) );
         return true;
      case ATTRIB_OBSERVE:
         // Read-only.
         return true;
      case ATTRIB_PAUSE:
         // Read-only.
         return true;
      case ATTRIB_PITCH:
         pitch( (float)VM::toNumber( vm, 1 ) );
         return true;
      case ATTRIB_PLAY:
         // Read-only.
         return true;
      case ATTRIB_PLAYING:
         // Read-only.
         return true;
      case ATTRIB_POSITION:
         position( VM::toVec3f( vm, 1 ) );
         return true;
      case ATTRIB_RELATIVE:
         relative( VM::toBoolean( vm, 1 ) );
         return true;
      case ATTRIB_REWIND:
         // Read-only.
         return true;
      case ATTRIB_SET_BGM:
         // Read-only.
         return true;
      case ATTRIB_SET_DEFAULTS:
         // Read-only.
         return true;
      case ATTRIB_SET_SOUND:
         // Read-only.
         return true;
      case ATTRIB_STOP:
         // Read-only.
         return true;
      case ATTRIB_STOPPED:
         // Read-only.
         return true;
      case ATTRIB_UPDATE:
         // Read-only.
         return true;
      case ATTRIB_VELOCITY:
         velocity( VM::toVec3f( vm, 1 ) );
         return true;
      default:
         break;
   }

   return false;
}

//------------------------------------------------------------------------------
//!
void
SoundSource::update()
{
   if( _entity != NULL )
   {
      _source->position( _entity->position() );
      //_source->velocity( _entity->linearVelocity() );
   }
   _source->update();
}

//------------------------------------------------------------------------------
//!
void
SoundSource::observe( Entity* e )
{
   if( _entity != NULL )
   {
      _entity->positionSubject().detach( this );
   }

   _entity = e;

   if( _entity != NULL )
   {
      _entity->positionSubject().attach( this );
   }
}

//------------------------------------------------------------------------------
//!
void
SoundSource::destroy()
{
   _entity = NULL;
}


/*==============================================================================
  CLASS SoundListener
==============================================================================*/

//------------------------------------------------------------------------------
//!
SoundListener::SoundListener():
   _entity( NULL )
{
   _listener = Plasma::soundManager()->sndMgr()->createListener();
}

//------------------------------------------------------------------------------
//!
SoundListener::~SoundListener()
{
   if( _entity )
   {
      _entity->positionSubject().detach( this );
   }
}

//------------------------------------------------------------------------------
//!
void
SoundListener::init( VMState* vm )
{
   DBG_BLOCK( os_snd, "SoundListener::init" );
   if( VM::isTable(vm, -1) )
   {
      // Start iterating at index 0 (nil, pushed below).
      VM::push(vm);
      while( VM::next(vm, -2) )
      {
         // Let the performSet() routine handle the various assignments.
         performSet(vm);

         // Pop the value, but keep the key.
         VM::pop(vm, 1);
      }

      // Creation-only parameters.
   }
}

//------------------------------------------------------------------------------
//!
bool
SoundListener::performGet( VMState* vm )
{
   DBG_BLOCK( os_snd, "SoundListener::performGet" );
   const char* str = VM::toCString( vm, -1 );
   switch( _attributes[str] )
   {
      case ATTRIB_GAIN:
         VM::push( vm, gain() );
         return true;
      case ATTRIB_OBSERVE:
         VM::push( vm, this, listener_observeVM );
         return true;
      case ATTRIB_POINT_OF_INTEREST:
         VM::push( vm, pointOfInterest() );
         return true;
      case ATTRIB_POSITION:
         VM::push( vm, position() );
         return true;
      case ATTRIB_SET_DEFAULTS:
         VM::push( vm, this, listener_setDefaultsVM );
         return true;
      case ATTRIB_UP_VECTOR:
         VM::push( vm, upVector() );
         return true;
      case ATTRIB_UPDATE:
         VM::push( vm, this, listener_updateVM );
         return true;
      case ATTRIB_VELOCITY:
         VM::push( vm, velocity() );
         return true;
      default:
         break;
   }

   return false;
}

//------------------------------------------------------------------------------
//!
bool
SoundListener::performSet( VMState* vm )
{
   DBG_BLOCK( os_snd, "SoundListener::performSet" );
   const char* str = VM::toCString( vm, -2 );
   switch( _attributes[str] )
   {
      case ATTRIB_GAIN:
         gain( (float)VM::toNumber( vm, 1 ) );
         return true;
      case ATTRIB_OBSERVE:
         // Read-only.
         return true;
      case ATTRIB_POINT_OF_INTEREST:
         pointOfInterest( VM::toVec3f( vm, 1 ) );
         return true;
      case ATTRIB_POSITION:
         position( VM::toVec3f( vm, 1 ) );
         return true;
      case ATTRIB_SET_DEFAULTS:
         // Read-only.
         return true;
      case ATTRIB_UP_VECTOR:
         upVector( VM::toVec3f( vm, 1 ) );
         return true;
      case ATTRIB_UPDATE:
         // Read-only.
         return true;
      case ATTRIB_VELOCITY:
         velocity( VM::toVec3f( vm, 1 ) );
         return true;
      default:
         break;
   }

   return false;
}

//------------------------------------------------------------------------------
//!
void
SoundListener::update()
{
   if( _entity != NULL )
   {
      Vec3f y, z;
      _entity->orientation().getAxesYZ( y, z );
      _listener->position( _entity->position() );
      _listener->pointOfInterest( _entity->position() + z );
      _listener->upVector( y );
      //_listener->velocity( _entity->linearVelocity() );
   }
   _listener->update();
}

//------------------------------------------------------------------------------
//!
void
SoundListener::observe( Entity* e )
{
   if( _entity != NULL )
   {
      _entity->positionSubject().detach( this );
   }

   _entity = e;

   if( _entity != NULL )
   {
      _entity->positionSubject().attach( this );
   }
}

//------------------------------------------------------------------------------
//!
void
SoundListener::destroy()
{
   _entity = NULL;
}


/*==============================================================================
  CLASS SoundManager
==============================================================================*/

//------------------------------------------------------------------------------
//!
SoundManager::SoundManager()
{
   DBG_BLOCK( os_snd, "Starting up SoundManager" );
   _sndMgr = Snd::Manager::createManager();
}

//------------------------------------------------------------------------------
//!
SoundManager::~SoundManager()
{
   DBG_BLOCK( os_snd, "Shutting down SoundManager" );
   _sndMgr = NULL;
}

//------------------------------------------------------------------------------
//!
void
SoundManager::init( VMState* vm )
{
   DBG_BLOCK( os_snd, "SoundManager::init" );
   if( VM::isTable(vm, -1) )
   {
      // Start iterating at index 0 (nil, pushed below).
      VM::push(vm);
      while( VM::next(vm, -2) )
      {
         // Let the performSet() routine handle the various assignments.
         performSet(vm);

         // Pop the value, but keep the key.
         VM::pop(vm, 1);
      }

      // Creation-only parameters.
   }
}

//------------------------------------------------------------------------------
//!
bool
SoundManager::performGet( VMState* vm )
{
   DBG_BLOCK( os_snd, "SoundManager::performGet" );
   const char* str = VM::toCString( vm, -1 );
   switch( _attributes[str] )
   {
      case ATTRIB_GEN_COLLISION:
         VM::push( vm, this, manager_genCollisionVM );
         return true;
      default:
         break;
   }

   return false;
}

//------------------------------------------------------------------------------
//!
bool
SoundManager::performSet( VMState* vm )
{
   DBG_BLOCK( os_snd, "SoundManager::performSet" );
   const char* str = VM::toCString( vm, -2 );
   switch( _attributes[str] )
   {
      case ATTRIB_GEN_COLLISION:
         // Read-only.
         return true;
      default:
         break;
   }

   return false;
}

//------------------------------------------------------------------------------
//!
void
SoundManager::genCollision( const Vec3f& position, const float force /* matA, matB*/ )
{
   RCP<Sound> snd;
   if( force == 0.0f )
   {
      snd = ResManager::getSound("sound/step_001");
   }
   else
   {
      snd = ResManager::getSound("sound/step_002");
   }

   RCP<SoundSource> src( new SoundSource() );
   src->autoDelete( true );
   src->setSound( snd.ptr() );
   src->position( position );
   //src->gain( CGM::min( force/10.0f, 1.0f ) );
   src->update();
   src->play();
}
