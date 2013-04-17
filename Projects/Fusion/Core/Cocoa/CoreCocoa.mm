/*=============================================================================
   Copyright (c) 2006, Ludo Sapiens Inc.
   All rights reserved.

   These coded instructions, statements, and computer programs contain
   unpublished, proprietary information and are protected by Federal copyright
   law. They may not be disclosed to third parties or copied or duplicated in
   any form, in whole or in part, without prior written consent.
=============================================================================*/
#include <Fusion/Core/Cocoa/CoreCocoa.h>
#include <Fusion/Core/Event.h>
#include <Fusion/Core/EventProfiler.h>
#include <Fusion/Core/Key.h>
#include <Fusion/Widget/FileDialog.h>

#import <Fusion/Core/Cocoa/FusionController.h>
#import <Fusion/Core/Cocoa/FusionOpenGLView.h>

#include <Gfx/Mgr/GL/GLContext_Cocoa.h>

#include <Base/Dbg/DebugStream.h>
#include <Base/IO/FileSystem.h>
#include <Base/IO/Path.h>
#include <Base/Util/Application.h>

#import <AppKit/NSCursor.h>
#import <AppKit/NSNibLoading.h>
#import <AppKit/NSWindow.h>

#import <Foundation/NSNotification.h>
#import <Foundation/NSPathUtilities.h>

#include <mach/mach_time.h>

#define USE_THREE_BUTTON_HACK 1

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

DBG_STREAM( os_cocoa, "CoreCocoa" );

CoreCocoa*  _single;
NSCursor*   _pointer[Pointer::_COUNT];
//bool        _trackingMouseEvent = false;

Timer       _animatorTimer;
//double      _lastAnimatorTime;

//-----------------------------------------------------------------------------
//!
NSString*  convert( const String& str )
{
   return [NSString stringWithUTF8String: str.cstr()];
}

//-----------------------------------------------------------------------------
//!
void applyOptions( const FileDialog& src, NSSavePanel* dst )
{
   if( !src.title().empty()   )  [dst   setTitle: convert(src.title())  ];

   if( !src.prompt().empty()  )  [dst  setPrompt: convert(src.prompt()) ];

   if( !src.message().empty() )  [dst setMessage: convert(src.message())];

   if( !src.startingLocation().empty() )
   {
      FS::Entry entry( src.startingLocation() );
      if( entry.type() == FS::TYPE_DIRECTORY )
      {
         NSString* str = [NSString stringWithUTF8String: entry.path().cstr()];
         NSURL*    url = [NSURL fileURLWithPath: str isDirectory: YES];
         [dst setDirectoryURL: url];
      }
      else
      {
         Path d = entry.path();
         Path f = d.split();
         NSString* str = [NSString stringWithUTF8String: d.cstr()];
         NSURL*    url = [NSURL fileURLWithPath: str isDirectory: YES];
         [dst setDirectoryURL: url];
         str = [NSString stringWithUTF8String: f.cstr()];
         [dst setNameFieldStringValue: str];
      }
   }

   const Vector<String>& types = src.allowedTypes();
   if( !types.empty() )
   {
      NSMutableArray* array = [NSMutableArray arrayWithCapacity: types.size()];
      for( auto cur = types.begin(); cur != types.end(); ++cur )
      {
         [array addObject: convert( *cur )];
      }
      [dst setAllowedFileTypes: array];
   }

   [dst setShowsHiddenFiles: src.showHidden()];

   [dst setCanCreateDirectories: src.canCreateDirectories()];

   if( !src.message().empty() )  [dst setMessage: convert(src.message())];
   [dst setAllowsOtherFileTypes: YES]; // Always allow to override the extension to our choice.
}

//-----------------------------------------------------------------------------
//!
void applyOptions( const FileDialog& src, NSOpenPanel* dst )
{
   NSSavePanel* dstSave = dst;
   applyOptions( src, dstSave );
   [dst    setCanChooseDirectories: src.canSelectDirectories()];
   [dst          setCanChooseFiles: src.canSelectFiles()      ];
   [dst         setResolvesAliases: src.resolveAliases()      ];
   [dst setAllowsMultipleSelection: src.multipleSelect()      ];
}

//-----------------------------------------------------------------------------
//!
void  savePaths( NSSavePanel* panel, FileDialog& dlg )
{
   Path path;
   if( [panel isMemberOfClass:[NSOpenPanel class]] )
   {
      for( id url in [(NSOpenPanel*)panel URLs] )
      {
         path = [[url path] cStringUsingEncoding:NSUTF8StringEncoding];
         dlg.add( path );
      }
   }
   else
   {
      NSURL* url = [panel URL];
      path = [[url path] cStringUsingEncoding:NSUTF8StringEncoding];
      dlg.add( path );
   }
   CHECK( dlg.numPaths() > 0 );
}

UNNAMESPACE_END


NAMESPACE_BEGIN

/*==============================================================================
  CLASS CoreCocoa
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
Core::initialize()
{
   DBG_BLOCK( os_cocoa, "Initializing CoreCocoa" );
   [FusionController _keepAtLinkTime];
   [FusionOpenGLView _keepAtLinkTime];

   _single = new CoreCocoa();
}

//------------------------------------------------------------------------------
//!
void
Core::finalize()
{
   DBG_BLOCK( os_cocoa, "Finalize CoreCocoa" );
   delete _single;
   _single = NULL;
}

//------------------------------------------------------------------------------
//!
void
Core::printInfo( TextStream& os )
{
   os << "Core: Cocoa" << nl;
}

//------------------------------------------------------------------------------
//!
CoreCocoa::CoreCocoa():
   _mainPointerID( 0 )
{
   DBG_BLOCK( os_cocoa, "CoreCocoa::CoreCocoa" );

   //NSLog(@"Allocating pool\n");
   _pool = [[NSAutoreleasePool alloc] init];

   _context = new Gfx::GLContext_Cocoa();

   setPaths();
}

//------------------------------------------------------------------------------
//!
CoreCocoa::~CoreCocoa()
{
   DBG_BLOCK( os_cocoa, "CoreCocoa::~CoreCocoa" );

   //NSLog(@"Releasing pool\n");
   [_pool release];
}

//------------------------------------------------------------------------------
//!
void
CoreCocoa::initializeGUI()
{
   DBG_BLOCK( os_cocoa, "CoreCocoa::initializeGUI" );

   // Note: We must initialize Gfx manager only after we have our first OpenGL context.
   Core::gfx( Gfx::Manager::create( _context.ptr() ) );

   // Needs to be called after manager is set.
   Core::initializeGUI();

   // arrowCursor is only defined after the NSApp is created.
   _pointer[Pointer::DEFAULT]  = [NSCursor arrowCursor];
   _pointer[Pointer::SIZE_ALL] = [NSCursor IBeamCursor];
   _pointer[Pointer::SIZE_T]   = [NSCursor resizeUpDownCursor];
   _pointer[Pointer::SIZE_B]   = [NSCursor resizeUpDownCursor];
   _pointer[Pointer::SIZE_BL]  = [NSCursor crosshairCursor];
   _pointer[Pointer::SIZE_BR]  = [NSCursor crosshairCursor];
   _pointer[Pointer::SIZE_TL]  = [NSCursor crosshairCursor];
   _pointer[Pointer::SIZE_TR]  = [NSCursor crosshairCursor];
   _pointer[Pointer::SIZE_L]   = [NSCursor resizeLeftRightCursor];
   _pointer[Pointer::SIZE_R]   = [NSCursor resizeLeftRightCursor];
   _pointer[Pointer::TEXT]     = [NSCursor IBeamCursor];
   _pointer[Pointer::HAND]     = [NSCursor pointingHandCursor];
   _pointer[Pointer::WAIT]     = [NSCursor arrowCursor];
   _pointer[Pointer::INVALID]  = [NSCursor arrowCursor];

   _mainPointerID = Core::createPointer().id();

   readyToExec();

   // Refresh internal size which was set when the NSApp was created, but the before _desktop was set.
   performResize( size().x, size().y );

   performShow();

   //NSLog(@"BEFORE RUN\n");
   FusionController* con = (FusionController*)[NSApp delegate];
   [con toggleAnimation];
   [con render];

   _startTI = currentSystemTime();
   _timer.restart();
}

//------------------------------------------------------------------------------
//!
void
CoreCocoa::finalizeGUI()
{
   DBG_BLOCK( os_cocoa, "CoreCocoa::finalizeGUI" );
   performHide();
   Core::finalizeGUI();
}

//------------------------------------------------------------------------------
//!
NSTimeInterval
CoreCocoa::currentSystemTime()
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
//! Sets various paths variables in Core depending on the existence of some files/directories.
//!
//! Search for a configuration file in the following directories:
//!   $(HOME)/Library/Application Support/Ludo Sapiens/config.lua
//!   <app_bundle>/Contents/Resources/Data/config.lua
//! and sets the config() location to the first one found.
//!
//! Sets the root() to the application bundle Resources directory:
//!   <app_bundle>/Contents/Resources/Data
//!
//! Sets the userRoot() to the following directory:
//!   $(HOME)/Library/Application Support/Ludo Sapiens/<app_name>
//!
//! Sets the cacheRoot() to the following directory:
//!   $(HOME)/Library/Caches/Ludo Sapiens/<app_name>
//!
//! Ref:
//!   http://developer.apple.com/mac/library/documentation/MacOSX/Conceptual/BPFileSystem/Articles/WhereToPutFiles.html
//!   http://developer.apple.com/mac/library/documentation/MacOSX/Conceptual/BPFileSystem/Articles/LibraryDirectory.html
//! We decided NOT to check for a file under:
//!   $(HOME)/Library/Preferences
//! since we do not use .plist files, and prefer to have everything grouped together.
void
CoreCocoa::setPaths()
{
   DBG_BLOCK( os_cocoa, "CoreCocoa::setPaths()" );
   // Visit "$(HOME)/Library/Application Support/".
   NSArray* paths = NSSearchPathForDirectoriesInDomains( NSApplicationSupportDirectory, NSUserDomainMask, YES );
   if( [paths count] > 0 )
   {
      NSString* ns_path = [paths objectAtIndex:0];
      const char* str_path = [ns_path cStringUsingEncoding:NSUTF8StringEncoding];
      if( str_path )
      {
         // Check for "$(HOME)/Library/Application Support/Ludo Sapiens/".
         Path path = str_path;
         path /= "Ludo Sapiens";
         path /= sApp->name();
         if( userRoot().empty() )
         {
            // Set userRoot() to "$(HOME)/Library/Application Support/Ludo Sapiens/<app_name>/".
            DBG_MSG( os_cocoa, "Cocoa userRoot: " << path.string() );
            userRoot( path );
         }
         // Check for "$(HOME)/Library/Application Support/Ludo Sapiens/<app_name>/config.lua".
         path /= "config.lua";
         if( config().empty() && FS::Entry(path).exists() )
         {
            DBG_MSG( os_cocoa, "Cocoa config: " << path.string() );
            config( path );
         }
      }
      else
      {
         StdErr << "Error trying to convert Application Support path to UTF8 string." << nl;
      }
   }
   else
   {
      StdErr << "Could not determine Application Support directory location." << nl;
   }

   // Visit "<app_bundle>/Contents/Resources/".
   NSString* ns_path = [[NSBundle mainBundle] resourcePath];
   if( ns_path )
   {
      const char* str_path = [ns_path cStringUsingEncoding:NSUTF8StringEncoding];
      if( str_path )
      {
         Path path = str_path;
         if( FS::Entry(path).exists() )
         {
            path /= "Data";
            if( FS::Entry(path).exists() )
            {
               // Set "<app_bundle>/Contents/Resources/Data" as first root.
               DBG_MSG( os_cocoa, "Cocoa Data root: " << path.string() );
               addRoot( path );
            }

            path /= "config.lua";
            if( config().empty() && FS::Entry(path).exists() )
            {
               // Set alternate config() to "<app_bundle>/Contents/Resources/Data/config.lua".
               DBG_MSG( os_cocoa, "Cocoa config: " << path.string() );
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
      // Visit "$(HOME)/Library/Caches/".
      paths = NSSearchPathForDirectoriesInDomains( NSCachesDirectory, NSUserDomainMask, YES );
      if( [paths count] > 0 )
      {
         NSString* ns_path = [paths objectAtIndex:0];
         const char* str_path = [ns_path cStringUsingEncoding:NSUTF8StringEncoding];
         if( str_path )
         {
            // Set cache root to "$(HOME)/Library/Caches/Ludo Sapiens/<app_name>/".
            Path path = str_path;
            path /= "Ludo Sapiens";
            path /= sApp->name();
            DBG_MSG( os_cocoa, "Cocoa cacheRoot: " << path.string() );
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
CoreCocoa::performLoop()
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
CoreCocoa::performExec()
{
   DBG_BLOCK( os_cocoa, "CoreCocoa::performExec" );

#if 1
   // Replacing NSApplicationMain() because it never returns (always calls exit()),
   // which messes up with our termination cleanup code.

   // Inspired by the Overview section from Apple's:
   //   http://developer.apple.com/mac/library/documentation/cocoa/reference/ApplicationKit/Classes/NSApplication_Class/Reference/Reference.html#//apple_ref/doc/uid/20000012-800060
   // which states that NSApplicationMain() is *functionally* equivalent to:
   //   [NSApplication sharedApplication];
   //   [NSBundle loadNibNamed:@"myMain" owner:NSApp];
   //   [NSApp run];
   // as well as the following page:
   //   http://cocoawithlove.com/2009/01/demystifying-nsapplication-by.html
   // which describes how to retrieve principal class and main nib (we don't need our own run loop).

   NSDictionary* infoDictionary = [[NSBundle mainBundle] infoDictionary];
   Class principalClass = NSClassFromString( [infoDictionary objectForKey:@"NSPrincipalClass"] );
   CHECK( [principalClass respondsToSelector:@selector(sharedApplication)] );
   [principalClass sharedApplication];

   NSString* mainNibName = [infoDictionary objectForKey:@"NSMainNibFile"];
   [NSBundle loadNibNamed:mainNibName owner:NSApp];

   DBG_MSG( os_cocoa, "Calling [NSApp run]" );
   [NSApp run];
   DBG_MSG( os_cocoa, "[NSApp run] returned" );
#else
   const Arguments& args = sApp->args();
   DBG_MSG( os_cocoa, "Calling NSApplicationMain" );
   NSApplicationMain( args.argc(), (const char**)args.argv() );
   DBG_MSG( os_cocoa, "NSApplicationMain returned" );
#endif
}

//------------------------------------------------------------------------------
//!
void
CoreCocoa::performExit()
{
   DBG_BLOCK( os_cocoa, "CoreCocoa::performExit()" );
   FusionController* con = (FusionController*)[NSApp delegate];
   [con toggleAnimation];
   [NSApp terminate:nil];
   //[NSApp stop:con];
   // FIXME: For some reason, the run loop keeps running until the next event.
}

//------------------------------------------------------------------------------
//!
void
CoreCocoa::performShow()
{
   DBG_BLOCK( os_cocoa, "CoreCocoa::performShow" );
}

//------------------------------------------------------------------------------
//!
void
CoreCocoa::performHide()
{
   DBG_BLOCK( os_cocoa, "CoreCocoa::performHide" );
}

//------------------------------------------------------------------------------
//!
void
CoreCocoa::performRedraw()
{
   DBG_BLOCK( os_cocoa, "CoreCocoa::performRedraw" );
}

//------------------------------------------------------------------------------
//!
bool
CoreCocoa::performIsKeyPressed( int key )
{
   switch( key )
   {
      case Key::ALT      : return ([NSEvent modifierFlags] & NSAlternateKeyMask) != 0x0;
      case Key::CAPS_LOCK: return ([NSEvent modifierFlags] & NSAlphaShiftKeyMask) != 0x0;
      case Key::CMD      : return ([NSEvent modifierFlags] & NSCommandKeyMask) != 0x0;
      case Key::CTRL     : return ([NSEvent modifierFlags] & NSControlKeyMask) != 0x0;
      case Key::SHIFT    : return ([NSEvent modifierFlags] & NSShiftKeyMask) != 0x0;
   }
   return Core::performIsKeyPressed( key ); // Fallback path.
}

//------------------------------------------------------------------------------
//!
void
CoreCocoa::performGrabPointer( uint )
{
   DBG_BLOCK( os_cocoa, "CoreCocoa::performGrabPointer" );
}

//------------------------------------------------------------------------------
//!
void
CoreCocoa::performReleasePointer( uint )
{
   DBG_BLOCK( os_cocoa, "CoreCocoa::performReleasePointer" );
}

//------------------------------------------------------------------------------
//!
void
CoreCocoa::performSetPointerIcon( uint pointerID, uint iconID )
{
   DBG_BLOCK( os_cocoa, "CoreCocoa::performSetPointerIcon(" << pointerID << ", " << iconID << ")" );
   if( pointerID == _mainPointerID )
   {
      [_pointer[iconID] set];
   }
   else
   {
      DBG_MSG( os_cocoa, "WARNING - Not sure what to do with non-main cursors." );
   }
}

//-----------------------------------------------------------------------------
//!
void
CoreCocoa::performAsk( FileDialog& dlg )
{
   NSSavePanel* panel = nil;
   switch( dlg.type() )
   {
      case FileDialog::OPEN:
      {
         NSOpenPanel* oPanel = [NSOpenPanel openPanel];
         applyOptions( dlg, oPanel );
         panel = oPanel;
      }  break;
      case FileDialog::SAVE:
      {
         NSSavePanel* sPanel = [NSSavePanel savePanel];
         applyOptions( dlg, sPanel );
         panel = sPanel;
      }  break;
   }

   if( panel != nil )
   {

#define PANEL_MODE 0  // 0: Default OSX, sheet sliding out of window.  1: Calling runModal(), separate window.

#if PANEL_MODE == 0
      __block FileDialog& bdlg = dlg;
      [panel beginSheetModalForWindow:[NSApp mainWindow] completionHandler:^(NSInteger returnCode)
      {
         switch( returnCode )
         {
            case NSOKButton:
            {
               savePaths( panel, bdlg );
               bdlg.onConfirm();
            }  break;
            case NSCancelButton:
               bdlg.onCancel();
               break;
            default:
               StdErr << "CoreCocoa::performAsk() - Unknown return code: " << returnCode << "." << nl;
               bdlg.onCancel();
               break;
         }
      }];
#else
      if( [panel runModal] )
      {
         savePaths( panel, dlg );
         dlg.onConfirm();
      }
      else
      {
         dlg.onCancel();
      }
#endif

   }
}


NAMESPACE_END
