/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Snd/NullManager.h>

#include <Snd/Listener.h>
#include <Snd/Source.h>


USING_NAMESPACE

using namespace Snd;


UNNAMESPACE_BEGIN

UNNAMESPACE_END


//------------------------------------------------------------------------------
//!
NullManager::NullManager
( void )
{
   API("NULL");
}

//------------------------------------------------------------------------------
//!
NullManager::~NullManager
( void )
{
}

// Listener
//------------------------------------------------------------------------------
//!
RCP<Listener>
NullManager::createListener()
{
   RCP<Listener> listener( new Listener(this) );
   return listener;
}

//------------------------------------------------------------------------------
//!
bool
NullManager::update( Listener* /*listener*/ )
{
   return true;
}

// Source
//------------------------------------------------------------------------------
//!
RCP<Source>
NullManager::createSource()
{
   RCP<Source> source( new Source(this) );
   return source;
}

//------------------------------------------------------------------------------
//!
bool
NullManager::update( Source* /*source*/ )
{
   return true;
}

//------------------------------------------------------------------------------
//!
bool
NullManager::play( Source* /*source*/ )
{
   return true;
}

//------------------------------------------------------------------------------
//!
bool
NullManager::pause( Source* /*source*/ )
{
   return true;
}

//------------------------------------------------------------------------------
//!
bool
NullManager::stop( Source* /*source*/ )
{
   return true;
}

//------------------------------------------------------------------------------
//!
bool
NullManager::rewind( Source* /*source*/ )
{
   return true;
}

//------------------------------------------------------------------------------
//!
bool
NullManager::playing( const Source* /*source*/ )
{
   return true;
}

// Sound
//------------------------------------------------------------------------------
//!
RCP<Sound>
NullManager::createSound()
{
   return 0;
}

//------------------------------------------------------------------------------
//!
bool
NullManager::setData(
   Sound*            /*sound*/,
   const size_t      /*sizeInBytes*/,
   const void*       /*data*/,
   const SampleType  /*sample*/,
   const uint        /*numChannels*/,
   const Freq        /*freq*/
)
{
   return true;
}

//------------------------------------------------------------------------------
//!
bool
NullManager::bind( Source* /*source*/, const Sound* /*sound*/ )
{
   return true;
}

//------------------------------------------------------------------------------
//!
bool
NullManager::process()
{
   return true;
}
