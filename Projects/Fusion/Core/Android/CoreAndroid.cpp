/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Core/Android/CoreAndroid.h>
#include <Fusion/Core/EventProfiler.h>

#include <Gfx/Mgr/GLES/GLESContext_Android.h>

#include <Base/Dbg/DebugStream.h>
#include <Base/IO/FileSystem.h>

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_an, "CoreAndroid" );

CoreAndroid*  _single = NULL;

UNNAMESPACE_END

//------------------------------------------------------------------------------
//!
void
Core::initialize()
{
   DBG_BLOCK( os_an, "CoreAndroid::initialize()" );
   _single = new CoreAndroid();
}

//------------------------------------------------------------------------------
//!
void
Core::finalize()
{
   DBG_BLOCK( os_an, "CoreAndroid::finalize()" );
   delete _single;
   _single = NULL;
}

//------------------------------------------------------------------------------
//!
void
Core::printInfo( TextStream& os )
{
   os << "Core: Android" << nl;
}

//------------------------------------------------------------------------------
//!
CoreAndroid::CoreAndroid()
{
   DBG_BLOCK( os_an, "CoreAndroid::CoreAndroid()" );
}

//------------------------------------------------------------------------------
//!
CoreAndroid::~CoreAndroid()
{
   DBG_BLOCK( os_an, "CoreAndroid::~CoreAndroid()" );
}

//------------------------------------------------------------------------------
//!
void
CoreAndroid::startTime( uint64_t ms )
{
   singleton()._startTimeMS = ms;
   singleton()._timer.restart();
   singleton()._lastTime = 0.0;
}

//------------------------------------------------------------------------------
//!
double
CoreAndroid::millisecondsToSeconds( uint64_t ms )
{
   double tmp = ms - singleton()._startTimeMS;
   tmp *= (1.0/1000.0); // Convert milliseconds to seconds.
   return tmp;
}

//-----------------------------------------------------------------------------
//! Sets standard paths.
//! Here is what I currently get:
//!   /data/data/com.ludosapiens.fusion/files
//!   /mnt/sdcard/Android/data/com.ludosapiens.fusion/files
//!   /data/data/com.ludosapiens.fusion/cache
//!   /data/data/com.ludosapiens.fusion/app_Data
void
CoreAndroid::setPaths( const char* files, const char* ext, const char* cache, const char* data )
{
   DBG_BLOCK( os_an, "CoreAndroid::setPaths(...)" );

   Path path;

   path = ext;
   if( userRoot().empty() )
   {
      DBG_MSG( os_an, "userRoot: " << path.string() );
      userRoot( path );
   }

   //path = ext; // TEMP: the apk keeps a *compressed* assets directory, and I'm not sure I like the res/raw approach.
   //path /= "Data";
   path = "/mnt/sdcard/Data";
   if( FS::Entry(path).exists() )
   {
      DBG_MSG( os_an, "Data root: " << path.string() );
      addRoot( path );
   }

   if( cacheRoot().empty() )
   {
      path = cache;
      DBG_MSG( os_an, "cache: " << path.string() );
      cacheRoot( path );
   }

#define TRY_CONFIG( dir_func ) \
   if( config().empty() ) \
   { \
      const Path& dir = dir_func(); \
      if( !dir.empty() ) \
      { \
         path = dir; \
         path /= "config.lua"; \
         if( FS::Entry( path ).exists() ) \
         { \
            DBG_MSG( os_an, "config: " << path.string() ); \
            config( path ); \
         } \
      } \
   }

   TRY_CONFIG( userRoot )
   TRY_CONFIG( root )

}

//-----------------------------------------------------------------------------
//!
void
CoreAndroid::initGfx()
{
   DBG_BLOCK( os_an, "CoreAndroid::initGfx()" );

   CoreAndroid& core = singleton();

   // Create Gfx manager and context.
   RCP<Gfx::GLESContext_Android> cntx = new Gfx::GLESContext_Android( 1 );
   core.gfx( Gfx::Manager::create( cntx.ptr() ) );
}

//------------------------------------------------------------------------------
//!
void
CoreAndroid::performLoop()
{
   PROFILE_EVENT( EventProfiler::LOOP_BEGIN );

   processEvents();

   executeAnimators( _timer.elapsed() );

   performRender();

   PROFILE_EVENT( EventProfiler::LOOP_END );
}

//-----------------------------------------------------------------------------
//!
void
CoreAndroid::performExec()
{
   DBG_BLOCK( os_an, "CoreAndroid::performExec()" );

   CoreAndroid& core = singleton();

   core.initializeGUI();
   core.readyToExec();
   core.performShow();

   // Relegate the looping code to the FusionGLView.Renderer.
}

//-----------------------------------------------------------------------------
//!
void
CoreAndroid::performExit()
{
   DBG_BLOCK( os_an, "CoreAndroid::performExit()" );
}

//-----------------------------------------------------------------------------
//!
void
CoreAndroid::performShow()
{
   DBG_BLOCK( os_an, "CoreAndroid::performShow()" );
}

//-----------------------------------------------------------------------------
//!
void
CoreAndroid::performHide()
{
   DBG_BLOCK( os_an, "CoreAndroid::performHide()" );
}

//-----------------------------------------------------------------------------
//!
void
CoreAndroid::performSetPointerIcon( uint id, uint icon )
{
   DBG_BLOCK( os_an, "CoreAndroid::performSetPointerIcon(" << id << "," << icon << ")" );
}
