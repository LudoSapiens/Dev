/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Snd/OpenALManager.h>
#include <Snd/Listener.h>
#include <Snd/Source.h>

#include <Base/Dbg/DebugStream.h>

#include <cassert>
#include <cstdio>


/*==============================================================================
   GLOBAL NAMESPACE
==============================================================================*/


USING_NAMESPACE
using namespace Snd;


UNNAMESPACE_BEGIN

DBG_STREAM( os_al, "OpenALManager" );


/*==============================================================================
   Utilities
==============================================================================*/

//------------------------------------------------------------------------------
//!
void displayALError(const char* fmt, ALenum err)
{
   switch(err)
   {
      case AL_NO_ERROR:          printf(fmt, "NO ERROR");          break;
      case AL_INVALID_NAME:      printf(fmt, "INVALID NAME");      break;
      case AL_INVALID_ENUM:      printf(fmt, "INVALID ENUM");      break;
      case AL_INVALID_VALUE:     printf(fmt, "INVALID VALUE");     break;
      case AL_INVALID_OPERATION: printf(fmt, "INVALID OPERATION"); break;
      case AL_OUT_OF_MEMORY:     printf(fmt, "OUT OF MEMORY");     break;
      default:                   printf(fmt, "<UNKNOWN>");         break;
   }
}

//------------------------------------------------------------------------------
//!
bool  checkForErrors(const char* fmt)
{
   ALenum err = alGetError();

   if( err == AL_NO_ERROR )
   {
      return false;
   }
   else
   {
      while( err != AL_NO_ERROR )
      {
         displayALError(fmt, err);
         err = alGetError();
      }
      return true;
   }
}

#if 0
//------------------------------------------------------------------------------
//!
void printALInfo()
{
   const ALchar* str = alGetString( AL_VENDOR );
   printf( "VENDOR: %s\n", str );

   str = alGetString( AL_RENDERER );
   printf( "RENDERER: %s\n", str );

   str = alGetString( AL_VERSION );
   printf( "VERSION: %s\n", str );

   str = alGetString( AL_EXTENSIONS );
   printf( "EXTENSIONS: %s\n", str );
}

//------------------------------------------------------------------------------
//!
void printALErrors()
{
   ALenum err = alGetError();

   if( err == AL_NO_ERROR )
   {
      printf("AL_ERROR: none\n");
   }
   else
   {
      printf("AL_ERROR:\n");
      printf("---------\n");
      while( err != AL_NO_ERROR )
      {
         displayALError("AL Error: %s\n", err);
         err = alGetError();
      }
      printf("---------\n");
   }
}
#endif

//------------------------------------------------------------------------------
//!
ALenum
toALFormat
( const SampleType sample, const uint numChannels )
{
   switch( sample )
   {
      case SND_SAMPLE_8:
      {
         switch( numChannels )
         {
            case 1:  return AL_FORMAT_MONO8;
            case 2:  return AL_FORMAT_STEREO8;
            default: return (ALenum)-1;
         }
      } break;

      case SND_SAMPLE_16:
      {
         switch( numChannels )
         {
            case 1:  return AL_FORMAT_MONO16;
            case 2:  return AL_FORMAT_STEREO16;
            default: return (ALenum)-1;
         }
      } break;

      case SND_SAMPLE_24:
      {
         return (ALenum)-1;
      } break;

      case SND_SAMPLE_24F:
      {
         return (ALenum)-1;
      } break;

      case SND_SAMPLE_32:
      {
         return (ALenum)-1;
      } break;

      case SND_SAMPLE_32F:
      {
         return (ALenum)-1;
      } break;

      default:
      {
         return (ALenum)-1;
      }
   }

   return (ALenum)-1;
}

// =================================
// =================================
// OpenAL-specific subclasses of Snd
// =================================
// =================================

/*==============================================================================
   CLASS OpenALSound
==============================================================================*/
class OpenALSound:
   public Sound
{
public:

   /*----- methods -----*/
   OpenALSound();

   virtual ~OpenALSound();

   /*----- data members -----*/
   ALuint  _bufferID;  //!< The buffer ID.

};  //class OpenALSound

//------------------------------------------------------------------------------
//!
OpenALSound::OpenALSound():
   _bufferID(0)
{
   alGenBuffers(1, &_bufferID);
   DBG_MSG( os_al, "Creating OpenALSound #" << _bufferID );
   checkForErrors("ERROR OpenALSound::OpenALSound - %s\n");
}

//------------------------------------------------------------------------------
//!
OpenALSound::~OpenALSound()
{
   DBG_MSG( os_al, "Deleting OpenALSound #" << _bufferID );
   alDeleteBuffers(1, &_bufferID);
   checkForErrors("ERROR OpenALSound::~OpenALSound - %s\n");
}


UNNAMESPACE_END


/*==============================================================================
   CLASS OpenALManager
==============================================================================*/

//------------------------------------------------------------------------------
//!
OpenALManager::OpenALManager
( void ):
   _pDevice(NULL),
   _cDevice(NULL),
   _pContext(NULL)
{
   DBG_BLOCK( os_al, "OpenALManager::OpenALManager" );
   API("OpenAL");
   //printALInfo();

   openDevice(String());
   createContext();
   openCaptureDevice(String());
}

//------------------------------------------------------------------------------
//!
OpenALManager::~OpenALManager
( void )
{
   DBG_BLOCK( os_al, "OpenALManager::~OpenALManager" );

   // Deallocate in reverse order since deleting #0 before #1 will recompact #1 back into #0,
   // which will give weird results.

   DBG_MSG( os_al, "Clearing all " << _listeners.size() << " listener(s)." );
   for( Vector< RCP<Listener> >::ReverseIterator cur = _listeners.rbegin();
        cur != _listeners.rend();
        ++cur )
   {
      DBG_MSG( os_al, ">>> lid=" << (*cur)->id() );
      //CHECK( (*cur).isUnique() );  // Bad usage: should assign dangling RCPs to NULL.
      (*cur)->terminate();
      //*cur = NULL;
   }

   DBG_MSG( os_al, "Clearing all " << _sources.size() << " source(s)." );
   for( Vector< RCP<Source> >::ReverseIterator cur = _sources.rbegin();
        cur != _sources.rend();
        ++cur )
   {
      DBG_MSG( os_al, ">>> sid=" << (*cur)->id() );
      //CHECK( (*cur).isUnique() );  // Bad usage: should assign dangling RCPs to NULL.
      (*cur)->terminate();
      //*cur = NULL;
   }

#ifdef CHECK
   DBG_MSG( os_al, "Performing some debug check." );
   CHECK( _listeners.empty() );
   CHECK( _sources.empty() );
   // Perform a sanity check on the values left.
   for( uint s = 0; s < _vSources.size(); ++s )
   {
      Vector<ALuint>& vs = _vSources[s];
      for( uint l = 0; l < vs.size(); ++l )
      {
         CHECK( vs[l] == 0 );
      }
   }
#endif

   closeCaptureDevice();
   deleteContext();
   closeDevice();
}

//------------------------------------------------------------------------------
//!
bool
OpenALManager::openDevice
( const String& deviceName )
{
   if( deviceName.empty() )
   {
      _pDevice = alcOpenDevice(NULL);
      checkForErrors("ERROR OpenALManager::openDevice(NULL) - %s\n");
   }
   else
   {
      _pDevice = alcOpenDevice(deviceName.cstr());
      checkForErrors("ERROR OpenALManager::openDevice - %s\n");
   }

   if( _pDevice == NULL )
   {
      checkForErrors("OpenALManager::openDevice - %s\n");
      return false;
   }

   return true;
}

//------------------------------------------------------------------------------
//!
bool
OpenALManager::closeDevice
( void )
{
   if( alcCloseDevice(_pDevice) )
   {
      return true;
   }
   else
   {
      checkForErrors("OpenALManager::closeDevice - %s\n");
   }
   return false;
}

//------------------------------------------------------------------------------
//!
bool
OpenALManager::openCaptureDevice
( const String& deviceName )
{
   ALuint freq = 44100;
   ALCenum format = AL_FORMAT_MONO16;
   ALCsizei size = freq * 5;  //5 seconds worth for now...
   if( deviceName.empty() )
   {
      _cDevice = alcCaptureOpenDevice(NULL, freq, format, size);
   }
   else
   {
      _cDevice = alcCaptureOpenDevice(deviceName.cstr(), freq, format, size);
   }

   if( _cDevice == NULL )
   {
      checkForErrors("OpenALManager::closeDevice - %s\n");
      return false;
   }

   return true;
}

//------------------------------------------------------------------------------
//!
bool
OpenALManager::closeCaptureDevice
( void )
{
   if( alcCloseDevice(_cDevice) )
   {
      return true;
   }
   else
   {
      checkForErrors("OpenALManager::closeCaptureDevice - %s\n");
   }
   return false;
}

//------------------------------------------------------------------------------
//!
bool
OpenALManager::createContext
( void )
{
   _pContext = alcCreateContext(_pDevice, NULL);
   if( _pContext == NULL )
   {
      checkForErrors("OpenALManager::createContext (creation) - %s\n");
      return false;
   }

   if( !alcMakeContextCurrent(_pContext) )
   {
      checkForErrors("OpenALManager::createContext (activation) - %s\n");
      return false;
   }

   return true;
}

//------------------------------------------------------------------------------
//!
bool
OpenALManager::deleteContext
( void )
{
   alcDestroyContext( _pContext );
   _pContext = NULL;
   return !checkForErrors("OpenALManager::deleteContext - %s\n");
}

//------------------------------------------------------------------------------
//!
void
OpenALManager::masterGain( float g )
{
   Manager::masterGain( g );
   updateAll();
}

//------------------------------------------------------------------------------
//!
void
OpenALManager::masterPitch( float p )
{
   Manager::masterPitch( p );
   updateAll();
}

//------------------------------------------------------------------------------
//!
void
OpenALManager::masterStop()
{
   alcSuspendContext( _pContext );
}

//------------------------------------------------------------------------------
//!
void
OpenALManager::masterResume()
{
   alcProcessContext( _pContext );
}

//------------------------------------------------------------------------------
//!
void
OpenALManager::updateAll()
{
   // Need to update all sources and listeners.
   for( uint sid = 0; sid < _sources.size(); ++sid )
   {
      update( _sources[sid].ptr() );
   }
}

// Listener
//------------------------------------------------------------------------------
//!
RCP<Listener>
OpenALManager::createListener
( void )
{
   DBG_BLOCK( os_al, "OpenALManager::createListener" );

   RCP<Listener> listener( new Listener(this) );
   const uint lid = (uint)_listeners.size();
   DBG_MSG( os_al, "lid = " << lid );
   listener->id( lid );
   _listeners.pushBack( listener );

   const uint ns = uint(_sources.size());

   if( lid != 0 )
   {
      DBG_MSG( os_al, "Creating virtual sources" );
      // Create new virtual sources, and bind them to the Source's sound (if there is one).
      for( uint sid = 0; sid < ns; ++sid )
      {
         const RCP<Source>& source = _sources[sid];
         if( source.isValid() )
         {
            createVirtualSource( lid, sid );
            const OpenALSound* al_sound = (const OpenALSound*)source->_sound.ptr();
            if( al_sound != NULL )
            {
               DBG_MSG( os_al, "alSourcei(" << _vSources[sid][lid] << ", AL_BUFFER, " << al_sound->_bufferID << ")" );
               alSourcei( _vSources[sid][lid], AL_BUFFER, al_sound->_bufferID );
               checkForErrors("ERROR OpenALManager::createListener (alSourcei) - %s\n");
            }
         }
      }
   }

   if( lid != 0 )
   {
      // Stop using the optimized path, so reset the AL listener state back to 0.
      al_resetListener();
      // ... and update all Sources too.
      for( uint sid = 0; sid < ns; ++sid )
      {
         const RCP<Source>& source = _sources[sid];
         if( source.isValid() )
         {
            update( source.ptr() );
         }
      }
   }

   return listener;
}

//------------------------------------------------------------------------------
//!
void
OpenALManager::release
( Listener* listener )
{
   DBG_BLOCK( os_al, "OpenALManager::release(Listener* id=" << (listener != NULL ? (int)listener->id() : -1) << ")" );

   CHECK( !_listeners.empty() );

   const uint lid  = listener->id();
   listener->id( INVALID_ID );

   // Put last listener in its place (maybe itself onto itself).
   _listeners[lid] = _listeners.back();
   // Remove last listener.
   _listeners.popBack();

   // Do the same for the virtual sources, but keep at least one (no listener mode).
   if( !_listeners.empty() )
   {
      DBG_MSG( os_al, "Deallocating all associated virtual sources" );
      const uint ns = uint(_sources.size());
      for( uint sid = 0 ; sid < ns; ++sid )
      {
         CHECK( _sources[sid] != NULL );
         deleteVirtualSource( lid, sid );
         Vector<ALuint>& vs = _vSources[sid];
         CHECK( !vs.empty() );
         vs[lid] = vs.back();
         vs.popBack();
      }
   }
}

//------------------------------------------------------------------------------
//!
bool
OpenALManager::update
( Listener* listener )
{
   DBG_BLOCK( os_al, "OpenALManager::update(Listener* id=" << (listener != NULL ? (int)listener->id() : -1) << ")" );
   if( _listeners.size() == 1 )
   {
      // Single-listener mode: use an optimized path.
      al_update( listener );
   }
   else
   {
      // Multi-listener mode: update all of the associated sources.
      const uint lid = listener->id();
      Mat4f mat = Reff::lookAt( listener->position(), listener->pointOfInterest(), listener->upVector() ).globalToLocal();

      const uint ns = uint(_sources.size());
      for( uint sid = 0; sid < ns; ++sid )
      {
         const RCP<Source>& source = _sources[sid];
         if( source.isValid() )
         {
            al_update( listener, mat, source.ptr(), _vSources[sid][lid] );
         }
      }
   }

   return true;
}


// Source
//------------------------------------------------------------------------------
//!
RCP<Source>
OpenALManager::createSource
( void )
{
   DBG_BLOCK( os_al, "OpenALManager::createSource" );
   RCP<Source> source( new Source(this) );
   const uint sid = (uint)_sources.size();
   DBG_MSG( os_al, "sid = " << sid );
   source->id( sid );
   _sources.pushBack( source );

   // Create one virtual source per listener.
   if( _listeners.empty() )
   {
      // Special case where no listener is created, yet we still want a virtual source.
      createVirtualSource( 0, sid );
   }
   else
   {
      // Create a source for every valid listener.
      const uint ls = uint(_listeners.size());
      for( uint lid = 0; lid < ls; ++lid )
      {
         if( _listeners[lid] != NULL )
         {
            createVirtualSource( lid, sid );
         }
      }
   }

   // Sources being created cannot have sounds bound, yet, so no need to update state.

   return source;
}

//------------------------------------------------------------------------------
//!
void
OpenALManager::release
( Source* source )
{
   DBG_BLOCK( os_al, "OpenALManager::release(Source* id=" << (source != NULL ? (int)source->id() : -1) << ")" );

   CHECK( !_sources.empty() );

   const uint sid = source->id();
   source->id( INVALID_ID );

   // Put last source in its place (maybe itself onto itself).
   _sources[sid] = _sources.back();
   // Remove last source.
   _sources.popBack();

   // Handle virtual sources.
   if( _listeners.empty() )
   {
      if( _sources.empty() )
      {
         // Special case where we might still have a virtual source allocated.
         DBG_MSG( os_al, "Deallocating last virtual source" );
         const uint ns = uint(_vSources.size());
         for( uint sid = 0; sid < ns; ++sid )
         {
            deleteVirtualSource( 0, sid );
         }
      }
      else
      {
         DBG_MSG( os_al, "Deallocating virtual source" );
         deleteVirtualSource( 0, sid );
         _vSources[sid][0] = _vSources.back()[0];
         _vSources.popBack();
      }
   }
   else
   {
      // Deallocate all of the associated virtual sources.
      DBG_MSG( os_al, "Deallocating all associated virtual sources" );
      const uint nl = uint(_listeners.size());
      for( uint lid = 0 ; lid < nl; ++lid )
      {
         if( _listeners[lid] != NULL )
         {
            deleteVirtualSource( lid, sid );
         }
      }
   }
}

//------------------------------------------------------------------------------
//!
bool
OpenALManager::update
( Source* source )
{
   DBG_BLOCK( os_al, "OpenALManager::update(Source* id=" << (source != NULL ? (int)source->id() : -1) << ")" );

   const Vector<ALuint>& vs = _vSources[source->id()];

   switch( _listeners.size() )
   {
      case 0:
      {
         DBG_MSG( os_al, "Updating with no listener." );
         CHECK( !vs.empty() );
         CHECK( vs[0] != 0 ); // Should always have a valid virtual source.
         al_update( source, vs[0] );
      }  break;
      case 1:
      {
         const uint ls = uint(_listeners.size());
         for( uint lid = 0; lid < ls; ++lid )
         {
            ALuint alid = vs[lid];
            if( alid != 0 )
            {
               // Single-listener doesn't specify listener.
               al_update( source, alid );
               return true;
            }
         }
      }  break;
      default:
      {
         const uint ls = uint(_listeners.size());
         for( uint lid = 0; lid < ls; ++lid )
         {
            DBG_MSG( os_al, "Updating listener id=" << lid );
            ALuint alid = vs[lid];
            if( alid != 0 )
            {
               al_update( _listeners[lid].ptr(), source, alid );
            }
         }
      }  break;
   }

   return true;
}


// Sound
//------------------------------------------------------------------------------
//!
RCP<Sound>
OpenALManager::createSound
( void )
{
   OpenALSound* al_sound = new OpenALSound();
   RCP<Sound> rcp(al_sound);
   return rcp;
}

#define DEFINE_COMMAND( name, alFunc ) \
   bool \
   OpenALManager::name( Source* source ) \
   { \
      DBG_BLOCK( os_al, "OpenALManager::" #name "(Source* id=" << (source != NULL ? (int)source->id() : -1) << ")" ); \
      bool err; \
      const uint sid = source->id(); \
      if( _listeners.size() <= 1 ) \
      { \
         /* Optimized path. */ \
         const uint lid = (_listeners.empty()) ? 0 : _listeners[0]->id(); \
         /*printf("%sing: sid=%d lid=%d, alsid=%d\n", #name, sid, lid, _vSources[sid][lid]);*/ \
         DBG_MSG( os_al, "ALSID=" << _vSources[sid][lid] ); \
         alFunc( _vSources[sid][lid] ); \
         err = checkForErrors("ERROR OpenALSource::" #name " (" #alFunc ") - %s\n"); \
      } \
      else \
      { \
         err = false; \
         /* Using alFuncv to guarantee simultaneous playback. */ \
         /* 1. Collect only valid IDs. */ \
         Vector<ALuint> vvs; \
         const Vector<ALuint> vs = _vSources[sid]; \
         const uint vsn = uint(vs.size()); \
         for( uint lid = 0; lid < vsn; ++lid ) \
         { \
            if( vs[lid] != 0 ) \
            { \
               DBG_MSG( os_al, "ALSID=" << _vSources[sid][lid] ); \
               vvs.pushBack(vs[lid]); \
            } \
         } \
         alFunc##v( ALsizei(vvs.size()), vvs.data() ); \
         err = checkForErrors("ERROR OpenALSource::" #name " (" #alFunc ") - %s\n"); \
      } \
      return err; \
   }

DEFINE_COMMAND( play,   alSourcePlay   )
DEFINE_COMMAND( pause,  alSourcePause  )
DEFINE_COMMAND( stop,   alSourceStop   )
DEFINE_COMMAND( rewind, alSourceRewind )

//------------------------------------------------------------------------------
//!
bool
OpenALManager::playing( const Source* source )
{
   DBG_BLOCK( os_al, "OpenALManager::playing(Source* id=" << (source != NULL ? (int)source->id() : -1) << ")" );
   // Only probe the first virtual source.
   const uint sid = source->id();
   const Vector<ALuint> vs = _vSources[sid];
   const uint nl = uint(vs.size());
   for( uint lid = 0; lid < nl; ++lid )
   {
      if( vs[lid] != 0 )
      {
         DBG_MSG( os_al, "Found alid " << vs[lid] << " for lid=" << lid );
         ALint state;
         alGetSourcei( vs[lid], AL_SOURCE_STATE, &state );
         checkForErrors("ERROR OpenALSource::playing (alGetSourcei) - %s\n");
         return (state == AL_PLAYING);
      }
   }
   return false;
}


#undef DEFINE_COMMAND

//------------------------------------------------------------------------------
//!
bool
OpenALManager::setData
( Sound* sound,
  const size_t sizeInBytes, const void* data,
  const SampleType sample, const uint numChannels, const Freq freq )
{
   OpenALSound* al_sound = (OpenALSound*)sound;
   ALenum al_format = toALFormat(sample, numChannels);
   alBufferData(al_sound->_bufferID, al_format, data, ALsizei(sizeInBytes), freq);
   sound->sizeInBytes(sizeInBytes);
   sound->sampleType(sample);
   sound->numChannels(numChannels);
   sound->freq(freq);
   return !checkForErrors("OpenALManager::setData - %s\n");
}

//------------------------------------------------------------------------------
//!
bool
OpenALManager::bind( Source* source, const Sound* sound )
{
   DBG_BLOCK( os_al, "OpenALManager::bind("
                     << " Source* id=" << (source != NULL ? (int)source->id() : -1)
                     << " Sound* #" << (sound != NULL ? (int)((OpenALSound*)sound)->_bufferID : -1)
                     << ")" );

   ALuint bufferID = 0;
   if( sound != NULL )
   {
      const OpenALSound* al_sound = (const OpenALSound*)sound;
      bufferID = al_sound->_bufferID;
   }

   CHECK( source->id() < _vSources.size() );
   const Vector<ALuint>& vs = _vSources[source->id()];
   DBG_MSG( os_al, "Source has " << vs.size() << " virtual source(s)" );
   for( uint i = 0; i < vs.size(); ++i )
   {
      if( vs[i] != 0 )
      {
         DBG_MSG( os_al, "alSourcei(" << vs[i] << ", AL_BUFFER, " << bufferID << ")" );
         alSourcei( vs[i], AL_BUFFER, bufferID );
      }
      // else it could have been already deallocated
   }
   bool err = checkForErrors("ERROR OpenALManager::bind (alSourcei) - %s\n");
   if( err )
   {
      printf("ERROR OpenALManager::bind - sid=%d #vs=%d bufferID=%d alsid=%d\n",
             source->id(), (uint)vs.size(), bufferID, vs[0]);
   }

   // Register new sound into the source (after having assigned the new bufferID).
   source->_sound = sound;

   return !err;
}

//------------------------------------------------------------------------------
//!
bool
OpenALManager::process()
{
   // Dispatch all of the updates.
   if( !_updates.empty() )
   {
      for( uint i = 0; i < _updates.size(); ++i )
      {
         UpdateEntry& ue = _updates[i];
         Listener* l = ue.listener();
         if( l != NULL )
         {
            Mat4f mat = Reff::lookAt( l->position(), l->pointOfInterest(), l->upVector() ).globalToLocal();
            UpdateEntry::SourceContainer::Iterator curS = ue.begin();
            UpdateEntry::SourceContainer::Iterator endS = ue.end();
            for( ; curS != endS; ++curS )
            {
               const Source* s = (*curS);
               const uint lid = l->id();
               const uint sid = s->id();
               al_update( l, mat, s, _vSources[sid][lid] );
            }
         }
      }
      _updates.clear(); //FIXME: MT issue
   }

   // Expire sounds that are done playing.
   for( uint sid = 0; sid < _sources.size(); ++sid )
   {
      RCP<Source>& source = _sources[sid];
      if( source.isValid() && source->autoDelete() && !source->playing() )
      {
         DBG_MSG( os_al, "Auto-deleting source " << source->id() );
         if( !source.isUnique() )
         {
            printf("WARNING OpenALManager::process - sid=%d auto-deleting delayed due to non-unique RCP.\n",
                   source->id());
         }
         source = NULL;
      }
   }

   return true;
}


//------------------------------------------------------------------------------
//! Creates a new virtual source for the specified listener and source combination.
//! Grows vectors as required.
void
OpenALManager::createVirtualSource( const uint lid, const uint sid )
{
   DBG_BLOCK( os_al, "OpenALManager::createVirtualSource(" << lid << ", " << sid << ")" );
   // Guarantee space for the source id.
   const uint ns = sid + 1;
   if( _vSources.size() < ns )
   {
      DBG_MSG( os_al, "Resizing for source from " << _vSources.size() << " to " << ns );
      _vSources.resize( ns );
   }

   // Guarantee space for the listener id.
   Vector<ALuint>& vs = _vSources[sid];
   const uint nl = lid + 1;
   if( vs.size() < nl )
   {
      DBG_MSG( os_al, "Resizing for listener from " << _vSources.size() << " to " << ns );
      vs.resize( nl, 0 );
   }

   // Allocate a new virtual source id.
   CHECK( vs[lid] == 0 );
   alGenSources( 1, vs.data() + lid );
   checkForErrors("OpenALManager::createVirtualSource (alGenSources) - %s\n");
   DBG_MSG( os_al, "alGenSources: #" << vs[lid] );
}

//------------------------------------------------------------------------------
//! Deletes a virtual source for the specified listener and source combination.
void
OpenALManager::deleteVirtualSource( const uint lid, const uint sid )
{
   DBG_BLOCK( os_al, "OpenALManager::deleteVirtualSource(" << lid << ", " << sid << ")" );
   CHECK( sid < _vSources.size() );
   CHECK( lid < _vSources[sid].size() );
   CHECK( _vSources[sid][lid] != 0 );
   DBG_MSG( os_al, "Deleting virtual source #" << _vSources[sid][lid] );
   ALuint& alid = _vSources[sid][lid];
   alDeleteSources( 1, &alid );
   alid = 0;
   checkForErrors("OpenALManager::deleteVirtualSource (glDeleteSources) - %s\n");
}

//------------------------------------------------------------------------------
//! Resets the listener (when switching to multi-listener support).
void
OpenALManager::al_resetListener() const
{
   DBG_BLOCK( os_al, "al_resetListener()" );
   float pos_vel[] = {
      0.0f, 0.0f, 0.0f, //pos, vel
   };
   float at_up[] = {
      0.0f, 0.0f,-1.0f, //at
      0.0f, 1.0f, 0.0f  //up (after at)
   };
   alListenerf( AL_GAIN, 1.0f );
   alListenerfv( AL_POSITION, pos_vel );
   alListenerfv( AL_VELOCITY, pos_vel );
   alListenerfv( AL_ORIENTATION, at_up );
   checkForErrors("ERROR al_resetListener() - %s\n");
}

//------------------------------------------------------------------------------
//! Updates the listener (single-listener path).
void
OpenALManager::al_update( const Listener* listener ) const
{
   DBG_BLOCK( os_al, "al_update(lid=" << listener->id() << ")" );
   DBG_MSG( os_al, "id=" << listener->id()
                << " G=" << listener->gain()
                //<< " p=" << listener->pitch()
                << " P=" << listener->position()
                << " V=" << listener->velocity()
                << " O=" << listener->pointOfInterest() << "," << listener->upVector() );
   alListenerf( AL_GAIN, Manager::masterGain() * listener->gain() );
   //alListenerf( AL_PITCH, Manager::masterPitch() * listener->pitch() );
   alListenerfv( AL_POSITION, listener->position().ptr() );
   alListenerfv( AL_VELOCITY, listener->velocity().ptr() );
   alListenerfv( AL_ORIENTATION, listener->pointOfInterest().ptr() );  //works because order is POI then UP
   checkForErrors("ERROR al_update(Listener*) - %s\n");
}

//------------------------------------------------------------------------------
//! Updates the source (single-listener path).
void
OpenALManager::al_update( const Source* source, const ALuint alid ) const
{
   DBG_BLOCK( os_al, "al_update(sid=" << source->id() << ", alid=" << alid << ")" );
   DBG_MSG( os_al, "id=" << source->id()
                << " G=" << source->gain()
                << " p=" << source->pitch()
                << " P=" << source->position()
                << " V=" << source->velocity()
                << " p=" << source->pitch()
                << (source->looping() ? " L" : " ")
                << (source->relative() ? " R" : " ") );
   alSourcef( alid, AL_GAIN, Manager::masterGain() * source->gain() );
   alSourcef( alid, AL_PITCH, Manager::masterPitch() * source->pitch() );
   alSourcefv( alid, AL_POSITION, source->position().ptr() );
   alSourcefv( alid, AL_VELOCITY, source->velocity().ptr() );
   alSourcei( alid, AL_LOOPING, (source->looping()?AL_TRUE:AL_FALSE) );
   alSourcei( alid, AL_SOURCE_RELATIVE, (source->relative()?AL_TRUE:AL_FALSE) );
   checkForErrors("ERROR al_update(Source*, ALuint) - %s\n");
}

//------------------------------------------------------------------------------
//! Updates a source relative to a listener (multi-listener path).
//! The trick here is to move everything in the listener's space.
void
OpenALManager::al_update( const Listener* listener, const Mat4f& mat, const Source* source, const ALuint alid ) const
{
   DBG_BLOCK( os_al, "al_update(lid=" << listener->id() << ", [mat], sid=" << listener->id() << ", alid=" << alid << ")" );
   DBG_MSG( os_al, "id=" << listener->id()
                << " G=" << listener->gain()
                //<< " p=" << listener->pitch()
                << " P=" << listener->position()
                << " V=" << listener->velocity()
                << " O=" << listener->pointOfInterest() << "," << listener->upVector() );
   DBG_MSG( os_al, "id=" << source->id()
                << " G=" << source->gain()
                << " p=" << source->pitch()
                << " P=" << source->position()
                << " V=" << source->velocity()
                << " p=" << source->pitch()
                << (source->looping() ? " L" : " ")
                << (source->relative() ? " R" : " "));
   alSourcei( alid, AL_LOOPING, (source->looping()?AL_TRUE:AL_FALSE) );
   alSourcef( alid, AL_GAIN, Manager::masterGain() * source->gain() * listener->gain() );
   alSourcef( alid, AL_PITCH, Manager::masterPitch() * source->pitch() /** listener->pitch()*/ );
   if( !source->relative() )
   {
      alSourcefv( alid, AL_POSITION, (mat * source->position()).ptr() );
      alSourcefv( alid, AL_VELOCITY, (source->velocity() + listener->velocity()).ptr() );
      alSourcei( alid, AL_SOURCE_RELATIVE, AL_FALSE );
   }
   else
   {
      alSourcefv( alid, AL_POSITION, source->position().ptr() );
      alSourcefv( alid, AL_VELOCITY, source->velocity().ptr() );
      alSourcei( alid, AL_SOURCE_RELATIVE, AL_TRUE );
   }
   checkForErrors("ERROR al_update(Listener*, Mat4f, Source*, ALuint) - %s\n");
}

//------------------------------------------------------------------------------
//! Updates a source relative to a listener (multi-listener path).
//! The trick here is to move everything in the listener's space.
void
OpenALManager::al_update( const Listener* listener, const Source* source, const ALuint alid ) const
{
   DBG_BLOCK( os_al, "al_update(lid=" << listener->id() << ", sid=" << source->id() << ", alid=" << alid << ")" );
   DBG_MSG( os_al, "id=" << listener->id()
                << " G=" << listener->gain()
                //<< " p=" << listener->pitch()
                << " P=" << listener->position()
                << " V=" << listener->velocity()
                << " O=" << listener->pointOfInterest() << "," << listener->upVector() );
   DBG_MSG( os_al, "id=" << source->id()
                << " G=" << source->gain()
                << " p=" << source->pitch()
                << " P=" << source->position()
                << " V=" << source->velocity()
                << " p=" << source->pitch()
                << (source->looping() ? " L" : " ")
                << (source->relative() ? " R" : " ") );
   alSourcei( alid, AL_LOOPING, (source->looping()?AL_TRUE:AL_FALSE) );
   alSourcef( alid, AL_GAIN, Manager::masterGain() * source->gain() * listener->gain() );
   alSourcef( alid, AL_PITCH, Manager::masterPitch() * source->pitch() /** listener->pitch()*/ );
   if( !source->relative() )
   {
      Mat4f mat = Reff::lookAt( listener->position(), listener->pointOfInterest(), listener->upVector() ).globalToLocal();
      alSourcefv( alid, AL_POSITION, (mat * source->position()).ptr() );
      alSourcefv( alid, AL_VELOCITY, (source->velocity() + listener->velocity()).ptr() );
      alSourcei( alid, AL_SOURCE_RELATIVE, AL_FALSE );
   }
   else
   {
      alSourcefv( alid, AL_POSITION, source->position().ptr() );
      alSourcefv( alid, AL_VELOCITY, source->velocity().ptr() );
      alSourcei( alid, AL_SOURCE_RELATIVE, AL_TRUE );
   }
   checkForErrors("ERROR al_update(Listener*, Source*, ALuint) - %s\n");
}
