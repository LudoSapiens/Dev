/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef SND_MANAGER_H
#define SND_MANAGER_H

#include <Snd/StdDefs.h>

#include <Snd/Sound.h>
#include <Snd/Types.h>

#include <Base/ADT/String.h>
#include <Base/ADT/Vector.h>
#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>


NAMESPACE_BEGIN

namespace Snd
{

class Listener;
class Source;

/*==============================================================================
  CLASS Manager
==============================================================================*/
class Manager:
   public RCObject
{
public:

   /*----- static methods -----*/

   static SND_DLL_API void  printInfo( TextStream& os );

   // Factory constructor
   static SND_DLL_API RCP<Manager>  createManager( const String& apiName = "" );

   // File reading
   static SND_DLL_API bool  readSoundFile( const String& filename, NAMESPACE::Vector<uchar>& dstData, SampleType& sample, uint& numChannels, Freq& freq );

   /*----- methods -----*/

   inline float masterGain() const { return _masterGain; }
   SND_DLL_API virtual void masterGain( float g );

   inline float masterPitch() const { return _masterPitch; }
   SND_DLL_API virtual void masterPitch( float p );

   SND_DLL_API virtual void masterStop();
   SND_DLL_API virtual void masterResume();

   // Listener
   SND_DLL_API virtual RCP<Listener>  createListener() = 0;
   SND_DLL_API virtual bool  update( Listener* listener ) = 0;

   // Source
   SND_DLL_API virtual RCP<Source>  createSource() = 0;
   SND_DLL_API virtual bool  update( Source* source ) = 0;

   SND_DLL_API virtual bool  play( Source* source ) = 0;
   SND_DLL_API virtual bool  pause( Source* source ) = 0;
   SND_DLL_API virtual bool  stop( Source* source ) = 0;
   SND_DLL_API virtual bool  rewind( Source* source ) = 0;

   SND_DLL_API virtual bool  playing( const Source* source ) = 0;
   inline bool  stopped( const Source* source ) { return !playing(source); }

   // Sound
   SND_DLL_API virtual RCP<Sound>  createSound() = 0;
   SND_DLL_API virtual bool  setData( Sound* sound, const size_t sizeInBytes, const void* data, const SampleType sample, const uint numChannels, const Freq freq ) = 0;
   SND_DLL_API RCP<Sound>  makeSoundFromFile( const String& filename );

   SND_DLL_API virtual bool  bind( Source* source, const Sound* sound ) = 0;

   /*----- methods -----*/

   // Processing (a.k.a. main loop).
   SND_DLL_API virtual bool  process() = 0;

   const String&  API() const { return _api; }

protected:

   /*----- data members -----*/

   String  _api;  //!< String representing the API ("OpenAL")
   float   _masterGain;   //!< The master gain, or volume.
   float   _masterPitch;  //!< The master pitch, or speed.

   friend class Listener;
   friend class Source;

   /*----- methods -----*/

   Manager();

   virtual ~Manager();

   void  API( const String& api ) { _api = api; }

   virtual void release( Listener* listener );
   virtual void release( Source* listener );

private:
}; //class Manager


}  //namespace Snd

NAMESPACE_END


#endif //SND_MANAGER_H
