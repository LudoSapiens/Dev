/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef SND_NULL_MANAGER_H
#define SND_NULL_MANAGER_H

#include <Snd/StdDefs.h>

#include <Snd/Manager.h>

NAMESPACE_BEGIN

namespace Snd
{

/*==============================================================================
  CLASS NullManager
==============================================================================*/
class NullManager:
   public Manager
{
public:

   /*----- methods -----*/

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

   /*----- methods -----*/

   // Processing (a.k.a. main loop).
   SND_DLL_API virtual bool  process();

protected:

   friend class Manager;

   /*----- methods -----*/

   NullManager();

   virtual ~NullManager();

private:
}; //class Manager


}  //namespace Snd

NAMESPACE_END


#endif //SND_NULL_MANAGER_H
