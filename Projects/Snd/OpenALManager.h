/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef SND_OPENAL_MANAGER_H
#define SND_OPENAL_MANAGER_H

#include <Snd/StdDefs.h>

#include <Snd/Listener.h>
#include <Snd/Manager.h>
#include <Snd/Source.h>

#include <CGMath/Ref.h>

#include <Base/ADT/Set.h>

//----------------
// OpenAL headers 
//----------------
#if defined(__APPLE__)
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#elif defined(_WIN32)
#include <al.h>
#include <alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif


NAMESPACE_BEGIN

namespace Snd
{

/*==============================================================================
  CLASS OpenALManager
==============================================================================*/
class OpenALManager:
   public Manager
{
public:

   /*----- methods -----*/

   SND_DLL_API virtual void masterGain( float g );
   SND_DLL_API virtual void masterPitch( float p );

   SND_DLL_API virtual void masterStop();
   SND_DLL_API virtual void masterResume();

   // Listener
   SND_DLL_API virtual RCP<Listener>  createListener();
   SND_DLL_API virtual bool  update( Listener* listener );

   // Source
   SND_DLL_API virtual RCP<Source>  createSource();
   SND_DLL_API virtual bool  update( Source* source );

   SND_DLL_API virtual bool  play( Source* source );
   SND_DLL_API virtual bool  pause( Source* source );
   SND_DLL_API virtual bool  stop( Source* source );
   SND_DLL_API virtual bool  rewind( Source* source );

   SND_DLL_API virtual bool  playing( const Source* source );

   // Sound
   SND_DLL_API virtual RCP<Sound>  createSound();
   SND_DLL_API virtual bool  setData( Sound* sound, const size_t sizeInBytes, const void* data, const SampleType sample, const uint numChannels, const Freq freq );

   SND_DLL_API virtual bool  bind( Source* source, const Sound* sound );

   // Processing (a.k.a. main loop).
   SND_DLL_API virtual bool  process();

protected:

   friend class Manager;


   /*==============================================================================
     CLASS UpdateEntry
   ==============================================================================*/
   class UpdateEntry
   {
   public:
      typedef Set<Source*>  SourceContainer;

      /*----- methods -----*/

      UpdateEntry( Listener* listener );
      virtual ~UpdateEntry();

      Listener*  listener() const { return _listener; }
      void  listener( Listener* listener ) { _listener = listener; }

      void  add( Source* source ) { _sources.add(source); }
      //void  add( const SourceContainer& sources ) { _sources = sources; }

      SourceContainer::Iterator  begin() { return _sources.begin(); }
      SourceContainer::Iterator  end()   { return _sources.end();   }

   protected:
      /*----- data members -----*/
      Listener*        _listener;  //!< The listener, common to all of the source.
      SourceContainer  _sources;   //!< All of the sources to update.
   }; //class UpdateEntry


   /*----- data members -----*/

   ALCdevice*                _pDevice;    //!< Playback device.
   ALCdevice*                _cDevice;    //!< Capture device.
   ALCcontext*               _pContext;   //!< Playback context.
   Vector< RCP<Source> >     _sources;    //!< All of the sources.
   Vector< RCP<Listener> >   _listeners;  //!< All of the listeners.
   Vector< Vector<ALuint> >  _vSources;   //!< Virtual OpenAL source ids for every source (_vSources[S][L]).
   Vector<UpdateEntry>       _updates;    //!< The list of updates to execute.

   /*----- methods -----*/

   OpenALManager();

   virtual ~OpenALManager();

   // Device
   bool  openDevice( const String& deviceName );
   bool  closeDevice();

   // CaptureDevice
   bool  openCaptureDevice( const String& deviceName );
   bool  closeCaptureDevice();

   // Context
   bool  createContext();
   bool  deleteContext();

   virtual void release( Listener* listener );
   virtual void release( Source* listener );

   inline void  updateSource( Listener* listener, Source* source );
   inline void  updateSources( Listener* listener );
   inline UpdateEntry&  findOrCreateUpdateEntry( Listener* listener );

   void  updateAll();

   void  createVirtualSource( const uint lid, const uint sid );
   void  deleteVirtualSource( const uint lid, const uint sid );

   // OpenAL routines.
   void  al_resetListener() const;
   void  al_update( const Listener* listener ) const;
   void  al_update( const Source* source, const ALuint alid ) const;
   void  al_update( const Listener* listener, const Mat4f& mat, const Source* source, const ALuint alid ) const;
   void  al_update( const Listener* listener, const Source* source, const ALuint alid ) const;

private:
}; //class OpenALManager

//------------------------------------------------------------------------------
//! Registers a single source into the update entry associated to the listener.
inline void
OpenALManager::updateSource( Listener* listener, Source* source )
{
   UpdateEntry& ue = findOrCreateUpdateEntry( listener );
   ue.add( source );
}

//------------------------------------------------------------------------------
//! Registers all of the sources of a listener.
inline void
OpenALManager::updateSources( Listener* listener )
{
   UpdateEntry& ue = findOrCreateUpdateEntry( listener );
   for( uint i = 0; i < _sources.size(); ++i )
   {
      const RCP<Source>& source = _sources[i];
      if( source.isValid() )
      {
         ue.add( source.ptr() );
      }
   }
}

//------------------------------------------------------------------------------
//! Retrieves an update entry to fill.  Creates one if it doesn't already exist.
inline OpenALManager::UpdateEntry&
OpenALManager::findOrCreateUpdateEntry( Listener* listener )
{
   for( uint i = 0; i < _updates.size(); ++i )
   {
      if( _updates[i].listener() == listener )
      {
         return _updates[i];
      }
   }
   _updates.pushBack( UpdateEntry(listener) );
   return _updates.back();
}


}  //namespace Snd

NAMESPACE_END


#endif //SND_OPENAL_MANAGER_H
