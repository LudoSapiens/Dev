/*==============================================================================
   Copyright (c) 2008, Ludo Sapiens Inc.
   All rights reserved.

   These coded instructions, statements, and computer programs contain
   unpublished, proprietary information and are protected by Federal copyright
   law. They may not be disclosed to third parties or copied or duplicated in
   any form, in whole or in part, without prior written consent.
==============================================================================*/
#import <Fusion/Core/Cocoa/FusionController.h>

#import <AppKit/NSApplication.h>
#import <AppKit/NSEvent.h>

#include <Fusion/Core/Cocoa/CoreCocoa.h>
#include <Fusion/Core/Cocoa/FusionOpenGLView.h>
#include <Fusion/Core/EventProfiler.h>
#include <Fusion/Core/Key.h>

#include <Base/ADT/StringMap.h>
#include <Base/Dbg/DebugStream.h>
#include <Base/Util/Bits.h>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_cocoa, "CoreCocoa" );

uint _count_ch = 0;

#if 0
//-----------------------------------------------------------------------------
//!
inline TextStream& operator<<( TextStream& os, const NSRange& range )
{
   return os << "r(" << range.location << ", " << range.length << ")";
}
#endif

//------------------------------------------------------------------------------
//!
inline double fix( NSTimeInterval ti )
{
   //return Core::lastTime();
   return CoreCocoa::toCoreTime( ti );
}

//------------------------------------------------------------------------------
//! Converts the character represented in the event into a Fusion keycode.
//! http://developer.apple.com/documentation/Cocoa/Reference/ApplicationKit/Classes/NSEvent_Class/Reference/Reference.html#//apple_ref/doc/uid/20000016-DontLinkElementID_8
inline int  eventToKey( NSEvent* event )
{
   NSString* str = [event charactersIgnoringModifiers];
   if( [str length] == 0 )  return 0;
   int c = [str characterAtIndex:0];
   switch( c )
   {
      case Key::a:
      case Key::b:
      case Key::c:
      case Key::d:
      case Key::e:
      case Key::f:
      case Key::g:
      case Key::h:
      case Key::i:
      case Key::j:
      case Key::k:
      case Key::l:
      case Key::m:
      case Key::n:
      case Key::o:
      case Key::p:
      case Key::q:
      case Key::r:
      case Key::s:
      case Key::t:
      case Key::v:
      case Key::w:
      case Key::x:
      case Key::y:
      case Key::z:
         c += Key::A - Key::a;
         break;
      case NSUpArrowFunctionKey:
         c = Key::UP_ARROW;
         break;
      case NSDownArrowFunctionKey:
         c = Key::DOWN_ARROW;
         break;
      case NSLeftArrowFunctionKey:
         c = Key::LEFT_ARROW;
         break;
      case NSRightArrowFunctionKey:
         c = Key::RIGHT_ARROW;
         break;
      case NSDeleteCharacter:
         c = Key::BACKSPACE;
         break;
      case NSDeleteFunctionKey:
         c = Key::DELETE;
         break;
      case NSHomeFunctionKey:
         c = Key::HOME;
         break;
      case NSEndFunctionKey:
         c = Key::END;
         break;
      case NSPageUpFunctionKey:
         c = Key::PAGE_UP;
         break;
      case NSPageDownFunctionKey:
         c = Key::PAGE_DOWN;
         break;
   }
   return c;
}

StringMap _SELToKey(
   "cancelOperation:"            ,  Key::ESCAPE,
   "deleteBackward:"             ,  Key::BACKSPACE,
   "deleteForward:"              ,  Key::DELETE,
   "insertNewline:"              ,  Key::RETURN,
   "insertTab:"                  ,  Key::TAB,
   "moveDown:"                   ,  Key::DOWN_ARROW,
   "moveLeft:"                   ,  Key::LEFT_ARROW,
   "moveRight:"                  ,  Key::RIGHT_ARROW,
   "moveUp:"                     ,  Key::UP_ARROW,
   "scrollPageDown:"             ,  Key::PAGE_DOWN,
   "scrollPageUp:"               ,  Key::PAGE_UP,
   "scrollToBeginningOfDocument:",  Key::HOME,
   "scrollToEndOfDocument:"      ,  Key::END,
   ""
);

UNNAMESPACE_END

@interface FusionController (NSApplication)
- (void)awakeFromNib;
- (void)applicationDidFinishLaunching:(NSNotification *)aNotification;
- (void)applicationWillTerminate:(NSNotification *)aNotification;
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender;
@end //@interface FusionController (NSApplication)


//===========================================================================
//= Animation Interface
//===========================================================================
@implementation FusionController (Animation)

- (BOOL) isAnimating
{
    return isAnimating;
}

- (void) startAnimation
{
   if (!isAnimating)
   {
      isAnimating = YES;
      if( ![self isInFullScreenMode] )
      {
         [self startAnimationTimer];
      }
   }
}

- (void) stopAnimation
{
   if( isAnimating )
   {
      if( animationTimer != nil )
      {
         [self stopAnimationTimer];
      }
      isAnimating = NO;
   }
}

- (void) toggleAnimation
{
   if( [self isAnimating] )
   {
      [self stopAnimation];
   }
   else
   {
      [self startAnimation];
   }
}

- (void) startAnimationTimer
{
   if( animationTimer == nil )
   {
      PROFILE_EVENT( EventProfiler::LOOPS_BEGIN );
      animationTimer = [[NSTimer scheduledTimerWithTimeInterval:0.017 target:self selector:@selector(animationTimerFired:) userInfo:nil repeats:YES] retain];
   }
}

- (void) stopAnimationTimer
{
   if( animationTimer != nil )
   {
      [animationTimer invalidate];
      [animationTimer release];
      animationTimer = nil;
      PROFILE_EVENT( EventProfiler::LOOPS_END );
   }
}

- (void) animationTimerFired:(NSTimer*)timer
{
   unused(timer);
   if( isAnimating )
   {
      CoreCocoa::doLoop();
      [fusionView setNeedsDisplay:YES];
   }
}

@end //@implementation FusionController (Animation)

//===========================================================================
//= NSApplication Interface
//===========================================================================
@implementation FusionController (NSApplication)

#pragma mark -- NSApplication Interface --

//------------------------------------------------------------------------------
//!
- (void)awakeFromNib
{
   DBG_BLOCK( os_cocoa, "FusionController awakeFromNib" );

   _running = YES;
   _oldModifiers = 0x0;
   _markedText = [[NSMutableAttributedString alloc] init];

   CoreCocoa::context( [fusionView openGLContext] );
   NSArray* windows = [NSApp windows];
   for( NSUInteger idx = 0; idx < windows.count; ++idx )
   {
      NSWindow* window = (NSWindow*)[windows objectAtIndex:idx];
      if( [[window title] isEqualToString:@"Fusion"] )
      {
         [window center];
      }
   }
}

//-----------------------------------------------------------------------------
//!
- (void)dealloc
{
  [_markedText release];
  [super dealloc];
}

//------------------------------------------------------------------------------
//!
- (void) applicationDidFinishLaunching:(NSNotification *)aNotification
{
   unused(aNotification);
   DBG_BLOCK( os_cocoa, "FusionController applicationDidFinishLaunching:" );
   CoreCocoa::singleton().initializeGUI();
}

//------------------------------------------------------------------------------
//!
- (void) applicationWillFinishLaunching:(NSNotification *)aNotification
{
   unused(aNotification);
#define NSDEF [NSUserDefaults standardUserDefaults]
   if( [NSDEF objectForKey: @"ApplePersistenceIgnoreState"] == nil )
      [NSDEF setBool: YES forKey:@"ApplePersistenceIgnoreState"];
}

//------------------------------------------------------------------------------
//!
- (void)applicationWillTerminate:(NSNotification *)aNotification
{
   unused(aNotification);
   DBG_BLOCK( os_cocoa, "FusionController applicationWillTerminate:" );
   [self stopAnimation];
   CoreCocoa::singleton().finalizeGUI();
}

//------------------------------------------------------------------------------
//!
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender
{
   unused(sender);
   DBG_BLOCK( os_cocoa, "FusionController applicationShouldTerminate:" );

   [self stopAnimation];
   CoreCocoa::singleton().finalizeGUI();

   if( _running )
   {
      DBG_MSG( os_cocoa, "Stopping only." );
      _running = NO;
      [NSApp stop:nil];
      // Trick from: http://www.cocoabuilder.com/archive/cocoa/219842-nsapp-stop.html
      // Post a bogus event to make sure the [NSApp stop:nil] is handled.
      NSEvent* event = [NSEvent otherEventWithType: NSApplicationDefined
                                          location: NSMakePoint(0,0)
                                     modifierFlags: 0
                                         timestamp: 0.0
                                      windowNumber: 0
                                           context: nil
                                           subtype: 0
                                             data1: 0
                                             data2: 0];
      [NSApp postEvent: event atStart: true];
      //CFRunLoopWakeUp( CFRunLoopGetMain() );
      return NSTerminateCancel;
   }
   else
   {
      DBG_MSG( os_cocoa, "Sending terminate signal." );
      return NSTerminateNow;
   }
}

@end //@implementation FusionController (NSApplication)


//===========================================================================
//= NSOpenGLView Interface
//===========================================================================
@implementation FusionController (NSOpenGLView)

#pragma mark -- NSOpenGLView Interface --

//------------------------------------------------------------------------------
//!
- (void) reshape:(int)w:(int)h
{
   DBG_BLOCK( os_cocoa, "FusionController reshape: " << w << " " << h );
   // Update Gfx
   CoreCocoa::resize( w, h );
}

//------------------------------------------------------------------------------
//!
- (void) render
{
   CoreCocoa::render();
}

@end //@implementation FusionController (NSOpenGLView)


//===========================================================================
//= NSView Interface
//===========================================================================
@implementation FusionController (NSView)

#pragma mark -- NSView Interface --

//------------------------------------------------------------------------------
//!
- (void)keyDown:(NSEvent*)theEvent
{
   //DBG_BLOCK( os_cocoa, "FusionController keyDown:" );
   double timestamp = fix(theEvent.timestamp);
   if( !_lastKeyDownRepeat )
   {
      // Avoid repeating keyPress events.
      // Repeat is really for onChar.
      CoreCocoa::submitEvent(
         Event::KeyPress( timestamp, eventToKey(theEvent) )
      );
   }

   //[super keyDown:theEvent];
}

//------------------------------------------------------------------------------
//!
- (void)keyUp:(NSEvent*)theEvent
{
   //DBG_BLOCK( os_cocoa, "FusionController keyUp:" );
   CoreCocoa::submitEvent(
      Event::KeyRelease( fix(theEvent.timestamp), eventToKey(theEvent) )
   );

   //[super keyUp:theEvent];
}

//------------------------------------------------------------------------------
//!
- (void)flagsChanged:(NSEvent*)theEvent
{
   //DBG_BLOCK( os_cocoa, "FusionController flagsChanged:" );
   NSUInteger newModifiers = [theEvent modifierFlags];
   NSUInteger diff = (_oldModifiers ^ newModifiers) & NSDeviceIndependentModifierFlagsMask;
   uchar n = (uchar)(15 - nlz( (uint32_t)diff ));
   //StdErr << "N=" << (uint)n << nl;
   int theKey = 0;
   switch( n )
   {
      case 0:
         CHECK( diff & NSAlphaShiftKeyMask );
         theKey = Key::CAPS_LOCK;
         break;
      case 1:
         CHECK( diff & NSShiftKeyMask );
         theKey = Key::SHIFT;
         break;
      case 2:
         CHECK( diff & NSControlKeyMask );
         theKey = Key::CTRL;
         break;
      case 3:
         CHECK( diff & NSAlternateKeyMask );
         theKey = Key::ALT;
         break;
      case 4:
         CHECK( diff & NSCommandKeyMask );
         theKey = Key::CMD;
         break;
      case 5:
         CHECK( diff & NSNumericPadKeyMask );
         theKey = Key::NUM_LOCK;
         break;
      case 6:
         CHECK( diff & NSHelpKeyMask );
         //theKey = Key::HELP;
         break;
      case 7:
         CHECK( diff & NSFunctionKeyMask );
         theKey = Key::MENU;
         break;
      default:
         break;
   }

#if 1
   if( theKey != 0 )
   {
      if( newModifiers > _oldModifiers )
      {
         Core::submitEvent(
            Event::KeyPress( fix(theEvent.timestamp), theKey )
         );
      }
      else
      {
         Core::submitEvent(
            Event::KeyRelease( fix(theEvent.timestamp), theKey )
         );
      }
   }
#else
   StdErr << "theKey=" << theKey << ":" << repeat << nl;
#endif

   _oldModifiers = newModifiers;

   //[super flagsChanged:theEvent];
}

//------------------------------------------------------------------------------
//!
- (void)mouseDown:(NSEvent*)theEvent
{
   //DBG_BLOCK( os_cocoa, "FusionController mouseDown:" );
   NSPoint loc = [theEvent locationInWindow];
   if( !stayInFullScreenMode )
   {
      loc = [fusionView convertPoint:loc fromView:nil];
   }
   CoreCocoa::submitEvent(
      Event::PointerPress( fix(theEvent.timestamp), CoreCocoa::mainPointerID(), 1, Vec2f(loc.x, loc.y), [theEvent clickCount] )
   );

   [super mouseDown:theEvent];
} // mouseDown

//------------------------------------------------------------------------------
//!
- (void)mouseDragged:(NSEvent*)theEvent
{
   //DBG_BLOCK( os_cocoa, "FusionController mouseDragged:" );
   NSPoint loc = [theEvent locationInWindow];
   if( !stayInFullScreenMode )
   {
      loc = [fusionView convertPoint:loc fromView:nil];
   }
   CoreCocoa::submitEvent(
      Event::PointerMove( fix(theEvent.timestamp), CoreCocoa::mainPointerID(), Vec2f(loc.x, loc.y) )
   );

   [super mouseDragged:theEvent];
} // mouseDragged

//------------------------------------------------------------------------------
//!
- (void)mouseMoved:(NSEvent*)theEvent
{
   //DBG_BLOCK( os_cocoa, "FusionController mouseMoved:" );
   NSPoint loc = [theEvent locationInWindow];
   if( !stayInFullScreenMode )
   {
      loc = [fusionView convertPoint:loc fromView:nil];
   }
   //StdErr << "Moved: " << loc.x << "," << loc.y << nl;
   CoreCocoa::submitEvent(
      Event::PointerMove( fix(theEvent.timestamp), CoreCocoa::mainPointerID(), Vec2f(loc.x, loc.y) )
   );

   [super mouseMoved:theEvent];
}

//------------------------------------------------------------------------------
//!
- (void)mouseUp:(NSEvent*)theEvent
{
   //DBG_BLOCK( os_cocoa, "FusionController mouseUp:" );
   NSPoint loc = [theEvent locationInWindow];
   if( !stayInFullScreenMode )
   {
      loc = [fusionView convertPoint:loc fromView:nil];
   }
   CoreCocoa::submitEvent(
      Event::PointerRelease( fix(theEvent.timestamp), CoreCocoa::mainPointerID(), 1, Vec2f(loc.x, loc.y), [theEvent clickCount] )
   );

   [super mouseUp:theEvent];
} // mouseUp

//------------------------------------------------------------------------------
//!
- (void)rightMouseDown:(NSEvent*)theEvent
{
   //DBG_BLOCK( os_cocoa, "FusionController rightMouseDown:" );
   NSPoint loc = [theEvent locationInWindow];
   if( !stayInFullScreenMode )
   {
      loc = [fusionView convertPoint:loc fromView:nil];
   }
   CoreCocoa::submitEvent(
      Event::PointerPress( fix(theEvent.timestamp), CoreCocoa::mainPointerID(), 3, Vec2f(loc.x, loc.y), [theEvent clickCount] )
   );

   [super rightMouseDown:theEvent];
} // rightMouseDown

//------------------------------------------------------------------------------
//!
- (void)rightMouseDragged:(NSEvent*)theEvent
{
   //DBG_BLOCK( os_cocoa, "FusionController rightMouseDragged:" );
   NSPoint loc = [theEvent locationInWindow];
   if( !stayInFullScreenMode )
   {
      loc = [fusionView convertPoint:loc fromView:nil];
   }
   CoreCocoa::submitEvent(
      Event::PointerMove( fix(theEvent.timestamp), CoreCocoa::mainPointerID(), Vec2f(loc.x, loc.y) )
   );

   [super rightMouseDragged:theEvent];
} // rightMouseDragged

//------------------------------------------------------------------------------
//!
- (void)rightMouseUp:(NSEvent*)theEvent
{
   //DBG_BLOCK( os_cocoa, "FusionController rightMouseUp:" );
   NSPoint loc = [theEvent locationInWindow];
   if( !stayInFullScreenMode )
   {
      loc = [fusionView convertPoint:loc fromView:nil];
   }
   CoreCocoa::submitEvent(
      Event::PointerRelease( fix(theEvent.timestamp), CoreCocoa::mainPointerID(), 3, Vec2f(loc.x, loc.y), [theEvent clickCount] )
   );

   [super rightMouseUp:theEvent];
} // rightMouseUp

//------------------------------------------------------------------------------
//!
- (void)otherMouseDown:(NSEvent*)theEvent
{
   //DBG_BLOCK( os_cocoa, "FusionController otherMouseDown:" );
   NSPoint loc = [theEvent locationInWindow];
   if( !stayInFullScreenMode )
   {
      loc = [fusionView convertPoint:loc fromView:nil];
   }
   CoreCocoa::submitEvent(
      Event::PointerPress( fix(theEvent.timestamp), CoreCocoa::mainPointerID(), 2, Vec2f(loc.x, loc.y), [theEvent clickCount] )
   );

   [super otherMouseDown:theEvent];
} // otherMouseDown

//------------------------------------------------------------------------------
//!
- (void)otherMouseDragged:(NSEvent*)theEvent
{
   //DBG_BLOCK( os_cocoa, "FusionController otherMouseDragged:" );
   NSPoint loc = [theEvent locationInWindow];
   if( !stayInFullScreenMode )
   {
      loc = [fusionView convertPoint:loc fromView:nil];
   }
   CoreCocoa::submitEvent(
      Event::PointerMove( fix(theEvent.timestamp), CoreCocoa::mainPointerID(), Vec2f(loc.x, loc.y) )
   );

   [super otherMouseDragged:theEvent];
} // otherMouseDragged

//------------------------------------------------------------------------------
//!
- (void)otherMouseUp:(NSEvent*)theEvent
{
   //DBG_BLOCK( os_cocoa, "FusionController otherMouseUp:" );
   NSPoint loc = [theEvent locationInWindow];
   if( !stayInFullScreenMode )
   {
      loc = [fusionView convertPoint:loc fromView:nil];
   }
   CoreCocoa::submitEvent(
      Event::PointerRelease( fix(theEvent.timestamp), CoreCocoa::mainPointerID(), 2, Vec2f(loc.x, loc.y), [theEvent clickCount] )
   );

   [super otherMouseUp:theEvent];
} // otherMouseUp

//------------------------------------------------------------------------------
//!
- (void)scrollWheel:(NSEvent *)theEvent
{
   //DBG_BLOCK( os_cocoa, "FusionController scrollWheel:" );
   NSPoint loc = [theEvent locationInWindow];
   if( !stayInFullScreenMode )
   {
      loc = [fusionView convertPoint:loc fromView:nil];
   }
   Vec2f scrollValue( -theEvent.deltaX, theEvent.deltaY );
   CoreCocoa::submitEvent(
      Event::PointerScroll( fix(theEvent.timestamp), CoreCocoa::mainPointerID(), scrollValue, Vec2f(loc.x, loc.y) )
   );

   [super scrollWheel:theEvent];
} // scrollWheel

@end //@implementation FusionController (NSView)

@implementation FusionController (NSWindow)

//-----------------------------------------------------------------------------
//!
- (void)windowWillClose:(NSNotification *)notification
{
   unused(notification);
   DBG_BLOCK( os_cocoa, "FusionController windowWillClose:" );
   //[self stopAnimationTimer]; // Make sure we don't try to draw in a non-existent window.
   CoreCocoa::exit();
}

@end //@implementation FusionController (NSWindow)


@implementation FusionController (NSTextInputClient)

//-----------------------------------------------------------------------------
//!
- (BOOL)hasMarkedText
{
   return _markedText.length > 0;
}

//-----------------------------------------------------------------------------
//!
- (NSRange)markedRange
{
   if( _markedText.length == 0 )
   {
      return NSMakeRange( NSNotFound, 0 );
   }
   else
   {
      return NSMakeRange( 0, _markedText.length-1 );
   }
}

//-----------------------------------------------------------------------------
//!
- (NSRange)selectedRange
{
   return NSMakeRange( NSNotFound, 0 );
}

//-----------------------------------------------------------------------------
//!
- (void)setMarkedText:(id)aString selectedRange:(NSRange)selectedRange replacementRange:(NSRange)replacementRange
{
   unused(selectedRange); unused(replacementRange);

   if( [aString isKindOfClass: [NSAttributedString class]] )
   {
      //[_text replaceCharactersInRange: NSMakeRange( 0, _text.length-1 )
      //           withAttributedString: aString];
      [_markedText initWithAttributedString: aString];
   }
   else
   {
      //[_text replaceCharactersInRange: NSMakeRange( 0, _text.length-1 )
      //                     withString: aString];
      [_markedText initWithString: aString];
   }
}

//-----------------------------------------------------------------------------
//!
- (void)unmarkText
{
   [[_markedText mutableString] setString:@""];
}

//-----------------------------------------------------------------------------
//!
- (NSArray*)validAttributesForMarkedText
{
   return [NSArray array];
}

//-----------------------------------------------------------------------------
//!
- (NSAttributedString*)attributedSubstringForProposedRange:(NSRange)aRange actualRange:(NSRangePointer)actualRange
{
   unused(aRange); unused(actualRange);
   return NULL;
}

//-----------------------------------------------------------------------------
//!
- (void)insertText:(id)aString replacementRange:(NSRange)replacementRange
{
   unused(aString); unused(replacementRange);
   NSString* string;
   if( [aString isKindOfClass: [NSAttributedString class]] )
   {
      string = [aString string];
   }
   else
   {
      string = (NSString*)aString;
   }
   const char* cur = string.UTF8String;
   char32_t codepoint;
   nextUTF8( cur, codepoint );
   NSEvent* theEvent = [NSApp currentEvent];
   double  timestamp = fix(theEvent.timestamp);
   _count_ch = theEvent.isARepeat ? _count_ch+1 : 1;
   CoreCocoa::submitEvent(
      Event::Char( timestamp, codepoint, _count_ch )
   );
}

//-----------------------------------------------------------------------------
//!
- (NSUInteger)characterIndexForPoint:(NSPoint)aPoint
{
   unused(aPoint);
   return 0;
}

//-----------------------------------------------------------------------------
//!
- (NSRect)firstRectForCharacterRange:(NSRange)aRange actualRange:(NSRangePointer)actualRange
{
   unused(aRange); unused(actualRange);
   return NSMakeRect( 0, 0, 0, 0 );
}

//-----------------------------------------------------------------------------
//!
- (void)doCommandBySelector:(SEL)aSelector
{
   const char* str = NSStringFromSelector(aSelector).UTF8String;
   int code = _SELToKey[str];
   if( code != StringMap::INVALID )
   {
      NSEvent* theEvent = [NSApp currentEvent];
      double  timestamp = fix(theEvent.timestamp);
      _count_ch = theEvent.isARepeat ? _count_ch+1 : 1;
      CoreCocoa::submitEvent(
         Event::Char( timestamp, code, _count_ch )
      );
   }
   else
   if( String("noop:") != str )
   {
      DBG( StdErr << "FusionController - Unknown selector: '" << str << "'" << nl );
      //[super doCommandBySelector:aSelector]; // Or NSApp
   }
}

@end //@implementation FusionController (NSTextInputClient)


@implementation FusionController

//===========================================================================
//= Utility Routines
//===========================================================================

+ (void) _keepAtLinkTime
{
   // Do nothing (this is just to force the class to be kept at link time).
}

// Action method wired up to fire when the user clicks the "Go FullScreen" button.
// We remain in this method until the user exits FullScreen mode.
- (IBAction) goFullScreen:(id)sender
{
   unused(sender);
   //DBG_BLOCK( os_cocoa, "FusionController goFullScreen:" );
   //return;
   CFAbsoluteTime timeNow;
   CGLContextObj cglContext;
   CGDisplayErr err;
   GLint oldSwapInterval;
   GLint newSwapInterval;

    // Pixel Format Attributes for the FullScreen NSOpenGLContext
    NSOpenGLPixelFormatAttribute attrs[] =
    {
        // Specify that we want a full-screen OpenGL context.
        NSOpenGLPFAFullScreen,

        // We may be on a multi-display system (and each screen may be driven
        // by a different renderer), so we need to specify which screen we want
        // to take over.  For this demo, we'll specify the main screen.
        NSOpenGLPFAScreenMask, CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay),

        // Attributes Common to FullScreen and non-FullScreen
        NSOpenGLPFAColorSize, 32,
        NSOpenGLPFADepthSize, 24,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAAccelerated,
        0
    };
    GLint rendererID;

    // Create the FullScreen NSOpenGLContext with the attributes listed above.
    NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];

    // Just as a diagnostic, report the renderer ID that this pixel format binds to.  CGLRenderers.h contains a list of known renderers and their corresponding RendererID codes.
    [pixelFormat getValues:&rendererID forAttribute:NSOpenGLPFARendererID forVirtualScreen:0];
    //NSLog(@"FullScreen pixelFormat RendererID = %08x", (unsigned)rendererID);

    // Create an NSOpenGLContext with the FullScreen pixel format.  By specifying the non-FullScreen context as our "shareContext", we automatically inherit all of the textures, display lists, and other OpenGL objects it has defined.
    fullScreenContext = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:[fusionView openGLContext]];
    [pixelFormat release];
    pixelFormat = nil;

    if (fullScreenContext == nil) {
        NSLog(@"Failed to create fullScreenContext");
        return;
    }

    // Pause animation in the OpenGL view.  While we're in full-screen mode, we'll drive the animation actively instead of using a timer callback.
    if ([self isAnimating]) {
        [self stopAnimationTimer];
    }

    // Take control of the display where we're about to go FullScreen.
    err = CGCaptureAllDisplays();
    if (err != CGDisplayNoErr) {
        [fullScreenContext release];
        fullScreenContext = nil;
        return;
    }

    // Enter FullScreen mode and make our FullScreen context the active context for OpenGL commands.
    [fullScreenContext setFullScreen];
    [fullScreenContext makeCurrentContext];
    CoreCocoa::context( fullScreenContext );

    glEnable( GL_SCISSOR_TEST );
    glEnable( GL_BLEND );
    glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    // Save the current swap interval so we can restore it later, and then set the new swap interval to lock us to the display's refresh rate.
    cglContext = CGLGetCurrentContext();
    CGLGetParameter(cglContext, kCGLCPSwapInterval, &oldSwapInterval);
    newSwapInterval = 1;
    CGLSetParameter(cglContext, kCGLCPSwapInterval, &newSwapInterval);

    // Tell the scene the dimensions of the area it's going to render to, so it can set up an appropriate viewport and viewing transformation.
    //[scene setViewportRect:NSMakeRect(0, 0, CGDisplayPixelsWide(kCGDirectMainDisplay), CGDisplayPixelsHigh(kCGDirectMainDisplay))];
    CoreCocoa::resize( CGDisplayPixelsWide(kCGDirectMainDisplay), CGDisplayPixelsHigh(kCGDirectMainDisplay) );

    // Now that we've got the screen, we enter a loop in which we alternately process input events and computer and render the next frame of our animation.  The shift here is from a model in which we passively receive events handed to us by the AppKit to one in which we are actively driving event processing.
    timeBefore = CFAbsoluteTimeGetCurrent();
    stayInFullScreenMode = YES;
    while (stayInFullScreenMode)
    {
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

        // Check for and process input events.
        NSUInteger mask = NSMouseMoved |
                          NSLeftMouseDownMask  | NSLeftMouseUpMask  | NSLeftMouseDraggedMask  |
                          NSRightMouseDownMask | NSRightMouseUpMask | NSRightMouseDraggedMask |
                          NSOtherMouseDownMask | NSOtherMouseUpMask | NSOtherMouseDraggedMask |
                          NSKeyDownMask | NSKeyUpMask | NSFlagsChangedMask;
        //mask = NSAnyEventMask;
        NSEvent* event;
        while( (event = [NSApp nextEventMatchingMask:mask untilDate:[NSDate distantPast] inMode:NSDefaultRunLoopMode dequeue:YES]) )
        {
            //StdErr << "evt" << [event type] << nl;
            switch ([event type])
            {
               case NSLeftMouseDown:
                  [self mouseDown:event];
                  break;
               case NSLeftMouseUp:
                  [self mouseUp:event];
                  break;
               case NSRightMouseDown:
                  [self rightMouseDown:event];
                  break;
               case NSRightMouseUp:
                  [self rightMouseUp:event];
                  break;
               case NSMouseMoved:
                  [self mouseMoved:event];
                  break;
               case NSLeftMouseDragged:
                  [self mouseDragged:event];
                  break;
               case NSRightMouseDragged:
                  [self rightMouseDragged:event];
                  break;
               case NSKeyDown:
                  [self keyDown:event];
                  if( eventToKey( event ) == Key::ESC )
                  {
                     // Exit fullscreen mode.
                     stayInFullScreenMode = NO;
                  }
                  break;
               case NSKeyUp:
                  [self keyUp:event];
                  break;
               case NSFlagsChanged:
                  [self flagsChanged:event];
                  break;
               case NSOtherMouseDown:
                  [self otherMouseDown:event];
                  break;
               case NSOtherMouseUp:
                  [self otherMouseUp:event];
                  break;
               case NSOtherMouseDragged:
                  [self otherMouseDragged:event];
                  break;
               default:
                  break;
            }
        }

        // Update our animation.
        timeNow = CFAbsoluteTimeGetCurrent();
        if ([self isAnimating]) {
            //[scene advanceTimeBy:(timeNow - timeBefore)];
        }
        timeBefore = timeNow;

        // Render a frame, and swap the front and back buffers.
        //[scene render];
        //[self render];
        //[fullScreenContext flushBuffer];
        CoreCocoa::doLoop();

        // Clean up any autoreleased objects that were created this time through the loop.
        [pool release];
    }

    // Clear the front and back framebuffers before switching out of FullScreen mode.  (This is not strictly necessary, but avoids an untidy flash of garbage.)
    //glClearColor(0.0, 0.0, 0.0, 0.0);
    //glClear(GL_COLOR_BUFFER_BIT);
    //[fullScreenContext flushBuffer];
    //glClear(GL_COLOR_BUFFER_BIT);
    //[fullScreenContext flushBuffer];

    // Restore the previously set swap interval.
    CGLSetParameter(cglContext, kCGLCPSwapInterval, &oldSwapInterval);

    // Exit fullscreen mode and release our FullScreen NSOpenGLContext.
    [NSOpenGLContext clearCurrentContext];
    [fullScreenContext clearDrawable];
    [fullScreenContext release];
    fullScreenContext = nil;

    NSOpenGLContext* context = [fusionView openGLContext];
    CoreCocoa::context( context );
    // Clear both front and back buffers before switching out of full scree.
    glClearColor(0.0, 0.0, 0.0, 0.0);
    NSRect bounds = [fusionView bounds];
    CoreCocoa::resize( bounds.size.width, bounds.size.height );
    glClear(GL_COLOR_BUFFER_BIT);
    [context flushBuffer];
    glClear(GL_COLOR_BUFFER_BIT);
    [context flushBuffer];

    // Release control of the display.
    CGReleaseAllDisplays();

    // Mark our view as needing drawing.  (The animation has advanced while we were in FullScreen mode, so its current contents are stale.)
    [fusionView setNeedsDisplay:YES];

    // Resume animation timer firings.
    if( [self isAnimating] )
    {
        [self startAnimationTimer];
    }
}

- (BOOL) isInFullScreenMode
{
    return fullScreenContext != nil;
}


@end //@implementation FusionController
