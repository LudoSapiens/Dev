/*==============================================================================
   Copyright (c) 2009, Ludo Sapiens Inc.
   All rights reserved.

   These coded instructions, statements, and computer programs contain
   unpublished, proprietary information and are protected by Federal copyright
   law. They may not be disclosed to third parties or copied or duplicated in
   any form, in whole or in part, without prior written consent.
==============================================================================*/
#include <Fusion/Core/CocoaTouch/CoreCocoaTouch.h>
#include <Fusion/Core/Event.h>
#include <Fusion/Core/EventProfiler.h>
#include <Fusion/Core/Key.h>

#import <Fusion/Core/CocoaTouch/FusionAppDelegate.h>
#import <Fusion/Core/CocoaTouch/FusionEAGLView.h>
#import <Fusion/Core/CocoaTouch/FusionEAGLViewController.h>
#import <Fusion/Core/CocoaTouch/FusionUIApplication.h>

#import <Gfx/Mgr/GLES/GLESContext_CocoaTouch.h>

#include <Base/Dbg/DebugStream.h>
#include <Base/IO/FileSystem.h>
#include <Base/Util/Application.h>

//#import <AppKit/NSCursor.h>
//#import <AppKit/NSNibLoading.h>
//#import <AppKit/NSWindow.h>
#import <QuartzCore/QuartzCore.h>

#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSNotification.h>

#include <mach/mach_time.h>

#define USE_THREE_BUTTON_HACK 1


/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

DBG_STREAM( os_ct, "CoreCocoaTouch" );

CoreCocoaTouch*  _single;
//FIXME NSCursor*   _pointer[Pointer::_COUNT];

Timer       _animatorTimer;


UNNAMESPACE_END


NAMESPACE_BEGIN

/*==============================================================================
  CLASS CoreCocoaTouch
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
Core::initialize()
{
   //printf("Initializing CoreCocoaTouch\n");
   DBG_BLOCK( os_ct, "Initializing CoreCocoaTouch" );
   _single = new CoreCocoaTouch();
}

//------------------------------------------------------------------------------
//!
void
Core::finalize()
{
   delete _single;
   _single = NULL;
}

//------------------------------------------------------------------------------
//!
void
Core::printInfo( TextStream& os )
{
   os << "Core: CocoaTouch" << nl;
   [mainApp() printInfo:os];
   [mainDelegate() printInfo:os];
   [mainView() printInfo:os];
   [mainViewController() printInfo:os];
}

//------------------------------------------------------------------------------
//!
CoreCocoaTouch::CoreCocoaTouch()
{
   DBG_BLOCK( os_ct, "CoreCocoaTouch::CoreCocoaTouch" );

   //NSLog(@"Allocating pool\n");
   _pool = [[NSAutoreleasePool alloc] init];

   setPaths();
}

//------------------------------------------------------------------------------
//!
CoreCocoaTouch::~CoreCocoaTouch()
{
   DBG_BLOCK( os_ct, "CoreCocoaTouch::~CoreCocoaTouch" );
   //NSLog(@"Releasing pool\n");
   //[_pool release];
}

//------------------------------------------------------------------------------
//! Sets various paths variables in Core depending on the existence of some files/directories.
//!
//! Search for a configuration file in the following directories:
//!   <Application_Home>/Documents/config.lua
//!   <Application_Home>/<app_bundle>/Contents/Resources/Data/config.lua
//! and sets the config() location to the first one found.
//!
//! Sets the root() to Data director stored in the application bundle's Resources directory:
//!   <Application_Home>/<app_bundle>/<resource>/Data/
//!
//! Sets the userRoot() to the following directory:
//!   <Application_Home>/Documents/
//!
//! Sets the cacheRoot() to the following directory:
//!   <Application_Home>/Library/Caches/
//!
//! Ref:
//!   https://developer.apple.com/iphone/library/documentation/iPhone/Conceptual/iPhoneOSProgrammingGuide/FilesandNetworking/FilesandNetworking.html
//!
//! We decided NOT to check for a file under:
//!   <Application_Home>/Library/Preferences
//! since we do not use .plist files, and prefer to have everything grouped together.
//!
//! Finally, we kept the Ludo Sapiens directory just for consistency with Cocoa.
void
CoreCocoaTouch::setPaths()
{
   DBG_BLOCK( os_ct, "CoreCocoaTouch::setPaths()" );

   // Visit "<Application_Home>/Documents/".
   NSArray* paths = NSSearchPathForDirectoriesInDomains( NSDocumentDirectory, NSUserDomainMask, YES );
   if( [paths count] > 0 )
   {
      NSString* ns_path = [paths objectAtIndex:0];
      const char* str_path = [ns_path cStringUsingEncoding:NSUTF8StringEncoding];
      if( str_path )
      {
         // Check for "<Application_Home>/Documents/".
         Path path = str_path;
         if( userRoot().empty() )
         {
            DBG_MSG( os_ct, "CocoaTouch userRoot: " << path.string() );
            userRoot( path );
         }
         // Check for "<Application_Home>/Documents/config.lua".
         path /= "config.lua";
         if( config().empty() && FS::Entry(path).exists() )
         {
            DBG_MSG( os_ct, "CocoaTouch config: " << path.string() );
            config( path );
         }
      }
      else
      {
         StdErr << "Error trying to convert Documents path to UTF8 string." << nl;
      }
   }
   else
   {
      StdErr << "Could not determine Documents directory location." << nl;
   }

   // Visit "<Application_Home>/<app_bundle>/<resource>/".
   NSString* ns_path = [[NSBundle mainBundle] resourcePath];
   if( ns_path )
   {
      const char* str_path = [ns_path cStringUsingEncoding:NSUTF8StringEncoding];
      if( str_path )
      {
         // Check for "<Application_Home>/<app_bundle>/<resource>/".
         Path path = str_path;
         if( FS::Entry(path).exists() )
         {
            path /= "Data";
            if( FS::Entry(path).exists() )
            {
               // Set "<Application_Home>/<app_bundle>/<resource>/Data" as first root.
               DBG_MSG( os_ct, "CocoaTouch Data root: " << path.string() );
               addRoot( path );
            }

            path /= "config.lua";
            if( config().empty() && FS::Entry(path).exists() )
            {
               // Set alternate config() to "<Application_Home>/<app_bundle>/<resource>/Data/config.lua".
               DBG_MSG( os_ct, "CocoaTouch config: " << path.string() );
               config( path );
            }
         }
      }
      else
      {
         StdErr << "Could not convert Resources path to UTF8 string." << nl;
      }
   }
   else
   {
      StdErr << "Could not determine application's Resources path." << nl;
   }

   if( cacheRoot().empty() )
   {
      // Visit "<Application_Home>/Library/Caches/".
      paths = NSSearchPathForDirectoriesInDomains( NSCachesDirectory, NSUserDomainMask, YES );
      if( [paths count] > 0 )
      {
         NSString* ns_path = [paths objectAtIndex:0];
         const char* str_path = [ns_path cStringUsingEncoding:NSUTF8StringEncoding];
         if( str_path )
         {
            // Check for "<Application_Home>/Library/Caches/".
            Path path = str_path;
            // Set cacheRoot() to "<Application_Home>/Library/Caches/".
            DBG_MSG( os_ct, "CocoaTouch cacheRoot: " << path.string() );
            cacheRoot( path );
         }
         else
         {
            StdErr << "Could not convert Caches path to UTF8 string." << nl;
         }
      }
      else
      {
         StdErr << "Could not determine application's Caches path." << nl;
      }
   }
}

//------------------------------------------------------------------------------
//!
void
CoreCocoaTouch::initializeGUI()
{
   DBG_BLOCK( os_ct, "CoreCocoaTouch::initializeGUI" );

   // Note: We must initialize Gfx manager only after we have our first OpenGL context.
   //Core::gfx( Gfx::Manager::createManager( NULL, "null", _context.ptr() ) );
   CAEAGLLayer* layer = (CAEAGLLayer*)[mainView() layer];
   RCP<Gfx::GLESContext> context = new Gfx::GLESContext_CocoaTouch( layer, Core::gfxVersion() );
   Core::gfx( Gfx::Manager::create( context.ptr() ) );
   // Update Core's size with the layer's size.
   CGRect rect = layer.bounds;
   performResize( rect.size.width, rect.size.height );

   Core::initializeGUI();

   readyToExec();

   _startTI = currentSystemTime();
   _timer.restart();
   _lastTime = 0.0;

   performShow();
}

//------------------------------------------------------------------------------
//!
void
CoreCocoaTouch::finalizeGUI()
{
   DBG_BLOCK( os_ct, "CoreCocoaTouch::finalizeGUI" );

   performHide();

   Core::finalizeGUI();
}

//------------------------------------------------------------------------------
//!
NSTimeInterval
CoreCocoaTouch::currentSystemTime()
{
   mach_timebase_info_data_t info;
   mach_timebase_info( &info );
   uint64_t machTime = mach_absolute_time();
   machTime *= info.numer;
   NSTimeInterval ti = (double)machTime;
   ti /= (double)(info.denom*1000000000);
   return ti;
}

//------------------------------------------------------------------------------
//!
void
CoreCocoaTouch::performLoop()
{
   PROFILE_EVENT( EventProfiler::LOOP_BEGIN );

   processEvents();

   executeAnimators( _timer.elapsed() );

   render();

   PROFILE_EVENT( EventProfiler::LOOP_END );
}

//------------------------------------------------------------------------------
//!
void
CoreCocoaTouch::performExec()
{
   DBG_BLOCK( os_ct, "CoreCocoaTouch::performExec" );
   const Arguments& args = sApp->args();
   DBG_MSG( os_ct, "Calling UIApplicationMain" );
   UIApplicationMain( args.argc(), args.argv(), @"FusionUIApplication", nil );
   DBG_MSG( os_ct, "UIApplicationMain returned" ); // Should never happen.
}

//------------------------------------------------------------------------------
//!
void
CoreCocoaTouch::performExit()
{
   DBG_BLOCK( os_ct, "CoreCocoaTouch::performExit()" );
   [mainDelegate() applicationWillTerminate: mainApp()];
   ::exit(0);
}

//------------------------------------------------------------------------------
//!
void
CoreCocoaTouch::performShow()
{
   DBG_BLOCK( os_ct, "CoreCocoaTouch::performShow" );
}

//------------------------------------------------------------------------------
//!
void
CoreCocoaTouch::performHide()
{
   DBG_BLOCK( os_ct, "CoreCocoaTouch::performHide" );
}

//------------------------------------------------------------------------------
//!
void
CoreCocoaTouch::performRedraw()
{
   DBG_BLOCK( os_ct, "CoreCocoaTouch::performRedraw" );
}

//------------------------------------------------------------------------------
//!
void
CoreCocoaTouch::performGrabPointer( uint )
{
   DBG_BLOCK( os_ct, "CoreCocoaTouch::performGrabPointer" );
}

//------------------------------------------------------------------------------
//!
void
CoreCocoaTouch::performReleasePointer( uint )
{
   DBG_BLOCK( os_ct, "CoreCocoaTouch::performReleasePointer" );
}

//------------------------------------------------------------------------------
//!
void
CoreCocoaTouch::performSetPointerIcon( uint /*pointerID*/, uint /*iconID*/ )
{
   //DBG_BLOCK( os_ct, "CoreCocoaTouch::performSetPointerIcon(" << pointerID << ", " << iconID << ")" );
}

//------------------------------------------------------------------------------
//!
void
CoreCocoaTouch::performGrabKeyboard()
{
   DBG_BLOCK( os_ct, "CoreCocoaTouch::performGrabKeyboard" );
   [mainDelegate().keyboardField becomeFirstResponder];
}

//------------------------------------------------------------------------------
//!
void
CoreCocoaTouch::performReleaseKeyboard()
{
   DBG_BLOCK( os_ct, "CoreCocoaTouch::performReleaseKeyboard" );
   [mainDelegate().keyboardField resignFirstResponder];
}

//------------------------------------------------------------------------------
//!
void
CoreCocoaTouch::performSetAccelerometerFrequency( float hz )
{
   DBG_BLOCK( os_ct, "CoreCocoaTouch::performSetAccelerometerFrequency(" << hz << ")" );
   [UIAccelerometer sharedAccelerometer].updateInterval = 1.0 / hz;
}

//------------------------------------------------------------------------------
//!
void
CoreCocoaTouch::startAccelerometer()
{
   DBG_BLOCK( os_ct, "CoreCocoaTouch::startAccelerometer()" );
   [UIAccelerometer sharedAccelerometer].delegate = mainDelegate();
}

//------------------------------------------------------------------------------
//!
void
CoreCocoaTouch::stopAccelerometer()
{
   DBG_BLOCK( os_ct, "CoreCocoaTouch::stopAccelerometer()" );
   [UIAccelerometer sharedAccelerometer].delegate = nil;
}

//------------------------------------------------------------------------------
//!
void
CoreCocoaTouch::performChangeOrientation( Orientation /*oldOri*/, Orientation newOri )
{
   [mainDelegate() changeOrientation:newOri];
}

//------------------------------------------------------------------------------
//!
bool
CoreCocoaTouch::performOpen( const String& url )
{
   DBG_BLOCK( os_ct, "CoreCocoaTouch::performOpen(" << url << ")" );
   NSString* str = [NSString stringWithCString:url.cstr() encoding:NSUTF8StringEncoding];
   NSURL*  nsurl = [NSURL URLWithString:str];
   return [[UIApplication sharedApplication] openURL:nsurl];
}

NAMESPACE_END
