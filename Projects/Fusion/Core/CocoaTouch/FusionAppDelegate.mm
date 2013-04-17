/*==============================================================================
   Copyright (c) 2009, Ludo Sapiens Inc.
   All rights reserved.

   These coded instructions, statements, and computer programs contain
   unpublished, proprietary information and are protected by Federal copyright
   law. They may not be disclosed to third parties or copied or duplicated in
   any form, in whole or in part, without prior written consent.
==============================================================================*/
#import <Fusion/Core/CocoaTouch/FusionAppDelegate.h>

#include <Fusion/Core/EventProfiler.h>
#include <Fusion/Core/Key.h>
#include <Fusion/Core/CocoaTouch/CoreCocoaTouch.h>
//#import <Fusion/Core/CocoaTouch/FusionEAGLView.h>
#import <Fusion/Core/CocoaTouch/FusionEAGLViewController.h>

#include <Base/Dbg/Defs.h>
#include <Base/Dbg/DebugStream.h>
#include <Base/Util/Application.h>

#import <AudioToolbox/AudioToolbox.h>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_ct, "CocoaTouch" );

//------------------------------------------------------------------------------
//!
inline double fix( NSTimeInterval ti )
{
   //return Core::lastTime();
   return CoreCocoaTouch::toCoreTime( ti );
}

inline Vec2i  getPosition( UITouch* touch )
{
   CGPoint loc = [touch locationInView:touch.view]; // Or mainView()?
   loc.y = touch.view.bounds.size.height - loc.y - 1; // Flip Y coordinate.
   return Vec2i( loc.x, loc.y );
}

#if 0
inline const char*  toStr( UITouchPhase phase )
{
   switch( phase )
   {
      case UITouchPhaseBegan:       return "Began";
      case UITouchPhaseMoved:       return "Moved";
      case UITouchPhaseStationary:  return "Stationary";
      case UITouchPhaseEnded:       return "Ended";
      case UITouchPhaseCancelled:   return "Cancelled";
      default:                      return "<Unknown>";
   }
}
#endif

void audioInterrupt( void* /*data*/, UInt32 state )
{
   StdErr << "audioInterrupt()" << nl;
   switch( state )
   {
      case kAudioSessionBeginInterruption:
         Core::snd()->masterStop();
         break;
      case kAudioSessionEndInterruption:
         Core::snd()->masterResume();
         break;
      default:
         StdErr << "audioInterrupt() - ERROR - Unknown state: " << state << nl;
         break;
   }
}

UNNAMESPACE_END

FusionAppDelegate*  _mainDelegate = NULL;

@implementation FusionAppDelegate

@synthesize window;
@synthesize glView;
@synthesize glViewController;
@synthesize keyboardField;
@synthesize animationInterval;

/*
- (void)application:(UIApplication *)application willChangeStatusBarOrientation:(UIInterfaceOrientation)newStatusBarOrientation duration:(NSTimeInterval)duration
{
   StdErr << "willChangeStatusBarOrientation " << (int)newStatusBarOrientation << nl;
}

- (void)application:(UIApplication *)application didChangeStatusBarOrientation:(UIInterfaceOrientation)oldStatusBarOrientation
{
   StdErr << "didChangeStatusBarOrientation " << (int)oldStatusBarOrientation << nl;
}
*/

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
   unused(launchOptions);
   DBG_BLOCK( os_ct, "FusionAppDelegate application:didFinishLaunchingWithOptions" );

   String str;
   str = [[[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleDisplayName"] cStringUsingEncoding:NSUTF8StringEncoding];
   if( str.empty() )
      str = [[[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleExecutable"] cStringUsingEncoding:NSUTF8StringEncoding];
   sApp->name( str );
   str = [[[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleVersion"] cStringUsingEncoding:NSUTF8StringEncoding];
   sApp->version( str );

   [glViewController.view setFrame:[[UIScreen mainScreen] bounds]]; // applicationFrame accounts for the status bar.
   [window addSubview:glViewController.view];
   [window makeKeyAndVisible];

   [self registerKeyboardNotifications];
   //[application setStatusBarHidden:YES animated:YES];
   application.idleTimerDisabled = YES;  // Prevent screen dimming when no input is sent.

   // Set audio options.
   // 1. Register interrupt callback to detect and handle phone calls/reminder alerts.
   OSStatus result = AudioSessionInitialize( NULL, NULL, audioInterrupt, self );
   if( result )
   {
      DBG_MSG( os_ct, "Error initializing audio session." );
      StdErr << "Error initializing audio session." << nl;
   }
#if 0
   // Allow iTunes music to keep playing in the background if it already is playing.
   UInt32 alreadyPlaying = false;
   UInt32 size = sizeof(alreadyPlaying);
   result = AudioSessionGetProperty( kAudioSessionProperty_OtherAudioIsPlaying, &size, &alreadyPlaying );
   if( result )
   {
      DBG_MSG( os_ct, "Error determining current audio state." );
      StdErr << "Error determining current audio state." << nl;
   }

   // Use mixing mode if background music is already playing; otherwise, play solo.
   UInt32 category = alreadyPlaying ? kAudioSessionCategory_AmbientSound : kAudioSessionCategory_SoloAmbientSound;
#else
   // Always allow iTunes music to keep playing in the background.
   UInt32 category = kAudioSessionCategory_AmbientSound;
#endif
   // 2. Set audio session category to potentially allow iTunes to play in the background.
   result = AudioSessionSetProperty( kAudioSessionProperty_AudioCategory, sizeof(category), &category );
   if( result )
   {
      DBG_MSG( os_ct, "Error setting audio session category." );
      StdErr << "Error setting audio session category." << nl;
   }
   // 3. Activate the session.
   result = AudioSessionSetActive( true );
   if( result )
   {
      DBG_MSG( os_ct, "Error activating audio session." );
      StdErr << "Error activating audio session." << nl;
   }

   accelMat = Mat3f::identity();

   CoreCocoaTouch::singleton().initializeGUI();

   // Execute one loop for rendering to prevent screen flickering.
   CoreCocoaTouch::doLoop();

   self.animationInterval = 1.0 / 60.0;
   [self startAnimation];
   return TRUE;
}

- (void)applicationWillTerminate:(UIApplication *)application
{
   unused(application);
   DBG_BLOCK( os_ct, "FusionAppDelegate applicationWillTerminate:" );
   [self stopAnimation];
   CoreCocoaTouch::singleton().finalizeGUI();
   delete sApp;
}

- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application
{
   unused(application);
   DBG_BLOCK( os_ct, "FusionAppDelegate applicationDidReceiveMemoryWarning:" );
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
   unused(application);
   DBG_BLOCK( os_ct, "FusionAppDelegate applicationDidBecomeActive:" );
}

- (void)applicationWillResignActive:(UIApplication *)application
{
   unused(application);
   DBG_BLOCK( os_ct, "FusionAppDelegate applicationWillResignActive:" );
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
   unused(application);
   DBG_BLOCK( os_ct, "FusionAppDelegate applicationDidEnterBackground:" );
   [self stopAnimation];
   Core::goToSleep();
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
   unused(application);
   DBG_BLOCK( os_ct, "FusionAppDelegate applicationWillEnterForeground:" );
   Core::wakeUp();
   [self startAnimation];
}

- (id)init
{
   DBG_BLOCK( os_ct, "FusionAppDelegate init" );
   self = [super init];
   CHECK( _mainDelegate == NULL );
   _mainDelegate = self;
   return self;
}

- (void) printInfo: (TextStream&)os
{
   os << "FusionAppDelegate:";
   os << (animationTimer ? " animating" : " not animating");
   if( self.animationInterval != 0 )  os << " (" << 1.0/self.animationInterval << " Hz)";
   else                               os << " (inf)";
   os << nl;
}

- (void) resize: (CGSize)size
{
   CoreCocoaTouch::resize( size.width, size.height );
}

@end

//===========================================================================
//= Animation Interface
//===========================================================================
@implementation FusionAppDelegate (Animation)

- (BOOL) isAnimating
{
    return animationTimer != nil;
}

- (void) startAnimation
{
   PROFILE_EVENT( EventProfiler::LOOPS_BEGIN );
   animationTimer = [NSTimer scheduledTimerWithTimeInterval:animationInterval target:self selector:@selector(animationTimerFired:) userInfo:nil repeats:YES];
}

- (void) stopAnimation
{
   [animationTimer invalidate];
   animationTimer = nil;
   PROFILE_EVENT( EventProfiler::LOOPS_END );
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

- (void) animationTimerFired:(NSTimer*)timer
{
   unused(timer);
   CoreCocoaTouch::doLoop();
}

@end //@implementation FusionAppDelegate (Animation)


//===========================================================================
//= Events Interface
//===========================================================================
@implementation FusionAppDelegate (Events)

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
   DBG_BLOCK( os_ct, "FusionAppDelegate touchesBegan:" << (void*)touches << " withEvent:" << (void*)event );
   for( UITouch* touch in touches )
   {
      if( touch.phase == UITouchPhaseBegan )
      {
         Pointer& pointer = CoreCocoaTouch::createPointer( touch );
         Vec2i pos = getPosition( touch );
         CoreCocoaTouch::submitEvent(
            Event::PointerPress( fix(event.timestamp), pointer.id(), 1, pos, touch.tapCount )
         );
      }
   }
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
   DBG_BLOCK( os_ct, "FusionAppDelegate touchesCancelled:" << (void*)touches << " withEvent:" << (void*)event );
   for( UITouch* touch in touches )
   {
      if( touch.phase == UITouchPhaseCancelled )
      {
         Pointer& pointer = CoreCocoaTouch::pointer( touch );
         CoreCocoaTouch::submitEvent(
            Event::PointerCancel( fix(event.timestamp), pointer.id() )
         );
         CoreCocoaTouch::deletePointer( touch );
      }
   }
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
   DBG_BLOCK( os_ct, "FusionAppDelegate touchesEnded:" << (void*)touches << " withEvent:" << (void*)event );
   for( UITouch* touch in touches )
   {
      if( touch.phase == UITouchPhaseEnded )
      {
         Pointer& pointer = CoreCocoaTouch::pointer( touch );
         Vec2i pos = getPosition( touch );
         CoreCocoaTouch::submitEvent(
            Event::PointerRelease( fix(event.timestamp), pointer.id(), 1, pos, touch.tapCount )
         );
         //// Need to queue the delete.
         //CoreCocoaTouch::submitEvent(
         //   Event::PointerDelete( fix(event.timestamp), pointer.id() )
         //);
         CoreCocoaTouch::deletePointer( touch );
      }
   }
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
   DBG_BLOCK( os_ct, "FusionAppDelegate touchesMoved:" << (void*)touches << " withEvent:" << (void*)event );
   for( UITouch* touch in touches )
   {
      if( touch.phase == UITouchPhaseMoved )
      {
         Pointer& pointer = CoreCocoaTouch::pointer( touch );
         Vec2i pos = getPosition( touch );
         CoreCocoaTouch::submitEvent(
            Event::PointerMove( fix(event.timestamp), pointer.id(), pos )
         );
      }
   }
}

- (void)motionBegan:(UIEventSubtype)motion withEvent:(UIEvent *)event
{
   unused(motion);
   unused(event);
   DBG_BLOCK( os_ct, "FusionAppDelegate motionBegan:withEvent:" );
}

- (void)motionCancelled:(UIEventSubtype)motion withEvent:(UIEvent *)event
{
   unused(motion);
   unused(event);
   DBG_BLOCK( os_ct, "FusionAppDelegate motionCancelled:withEvent:" );
}

- (void)motionEnded:(UIEventSubtype)motion withEvent:(UIEvent *)event
{
   unused(motion);
   unused(event);
   DBG_BLOCK( os_ct, "FusionAppDelegate motionEnded:withEvent:" );
}

- (void)accelerometer:(UIAccelerometer *)accelerometer didAccelerate:(UIAcceleration *)acceleration
{
   unused(accelerometer);
   DBG_BLOCK( os_ct, "FusionAppDelegate accelerometer:didAccelerate:" );
   Vec3f accel = accelMat * Vec3f( acceleration.x, acceleration.y, acceleration.z );
   CoreCocoaTouch::submitEvent(
      Event::Accelerate( fix(acceleration.timestamp), accel.x, accel.y, accel.z )
   );
}

@end //@implementation FusionAppDelegate (Events)


//===========================================================================
//= Keyboard Interface
//===========================================================================
@implementation FusionAppDelegate (Keyboard)

- (void)registerKeyboardNotifications
{
   [[NSNotificationCenter defaultCenter] addObserver:self
                                            selector:@selector(keyboardWillShow:)
                                                name:UIKeyboardWillShowNotification
                                              object:nil];

   [[NSNotificationCenter defaultCenter] addObserver:self
                                            selector:@selector(keyboardDidShow:)
                                                name:UIKeyboardDidShowNotification
                                              object:nil];

   [[NSNotificationCenter defaultCenter] addObserver:self
                                            selector:@selector(keyboardWillHide:)
                                                name:UIKeyboardWillHideNotification
                                              object:nil];

   [[NSNotificationCenter defaultCenter] addObserver:self
                                            selector:@selector(keyboardDidHide:)
                                                name:UIKeyboardDidHideNotification
                                              object:nil];
}

- (void)keyboardWillShow:(NSNotification*)notification
{
   unused(notification);
   // Set initial text here.
   DBG_BLOCK( os_ct, "Keyboard will show" );
   //StdErr << "Keyboard will show" << nl;
   keyboardField.text = @" ";
}

- (void)keyboardDidShow:(NSNotification*)notification
{
   unused(notification);
   DBG_BLOCK( os_ct, "Keyboard did show" );
   //StdErr << "Keyboard did show" << nl;
}

- (void)keyboardWillHide:(NSNotification*)notification
{
   unused(notification);
   DBG_BLOCK( os_ct, "Keyboard will hide" );
   //StdErr << "Keyboard will hide" << nl;
}

- (void)keyboardDidHide:(NSNotification*)notification
{
   unused(notification);
   DBG_BLOCK( os_ct, "Keyboard did hide" );
   //StdErr << "Keyboard did hide" << nl;
}

- (BOOL)textFieldShouldBeginEditing:(UITextField *)textField
{
   unused(textField);
   DBG_BLOCK( os_ct, "Should begin editing" );
   //StdErr << "Should begin editing" << nl;
   return YES;
}

- (BOOL)textFieldShouldEndEditing:(UITextField *)textField
{
   unused(textField);
   DBG_BLOCK( os_ct, "Should end editing" );
   //StdErr << "Should end editing" << nl;
   return YES;
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField
{
   DBG_BLOCK( os_ct, "Should return" );
   //StdErr << "Should return" << nl;
   [textField resignFirstResponder];
   return YES;
}

- (IBAction)editingChanged:(UITextField*)textField
{
   DBG_BLOCK( os_ct, "Editing changed" );
   //StdErr << "Editing changed" << nl;
   NSString* str = textField.text;
   switch( str.length )
   {
      case 0:
         CoreCocoaTouch::submitEvent( Event::KeyPress  ( Core::lastTime(), Key::BACKSPACE ) );
         CoreCocoaTouch::submitEvent( Event::KeyRelease( Core::lastTime(), Key::BACKSPACE ) );
         CoreCocoaTouch::submitEvent( Event::Char      ( Core::lastTime(), Key::BACKSPACE, 1 ) );
         break;
      case 1:
         //StdErr << "Didn't add anything" << nl;
         return;
      case 2:
      {
         char c = [str characterAtIndex:1];
         CoreCocoaTouch::submitEvent( Event::KeyPress  ( Core::lastTime(), Key::charToKey(c) ) );
         CoreCocoaTouch::submitEvent( Event::KeyRelease( Core::lastTime(), Key::charToKey(c) ) );
         CoreCocoaTouch::submitEvent( Event::Char      ( Core::lastTime(), Key::charToKey(c), 1 ) );
      }  break;
   }
   textField.text = @" ";
}

- (IBAction)editingDidBegin:(UITextField*)textField
{
   unused(textField);
   DBG_BLOCK( os_ct, "Editing did begin" );
   //StdErr << "Editing did begin" << nl;
}


- (IBAction)editingDidEnd:(UITextField*)textField
{
   unused(textField);
   DBG_BLOCK( os_ct, "Editing did end" );
   //StdErr << "Editing did end" << nl;
   CoreCocoaTouch::submitEvent( Event::KeyPress  ( Core::lastTime(), Key::RETURN ) );
   CoreCocoaTouch::submitEvent( Event::KeyRelease( Core::lastTime(), Key::RETURN ) );
   CoreCocoaTouch::submitEvent( Event::Char      ( Core::lastTime(), Key::RETURN, 1 ) );
}


- (IBAction)editingDidEndOnExit:(UITextField*)textField
{
   unused(textField);
   DBG_BLOCK( os_ct, "Editing did end on exit" );
   //StdErr << "Editing did end on exit" << nl;
   CoreCocoaTouch::submitEvent( Event::KeyPress  ( Core::lastTime(), Key::ESCAPE ) );
   CoreCocoaTouch::submitEvent( Event::KeyRelease( Core::lastTime(), Key::ESCAPE ) );
   CoreCocoaTouch::submitEvent( Event::Char      ( Core::lastTime(), Key::ESCAPE, 1 ) );
}


@end //@interface FusionAppDelegate (Keyboard)


//===========================================================================
//= Orientation Interface
//===========================================================================
@implementation FusionAppDelegate (Orientation)

- (void)changeOrientation:(Core::Orientation)newOri
{
   switch( newOri )
   {
      case Core::ORIENTATION_DEFAULT:
         accelMat = Mat4f::identity();
         break;
      case Core::ORIENTATION_CCW_90:
         accelMat = Mat4f::rotationZ( 0.0f, -1.0f );
         break;
      case Core::ORIENTATION_UPSIDE_DOWN:
         accelMat = Mat4f::rotationZ( -1.0f, 0.0f );
         break;
      case Core::ORIENTATION_CW_90:
         accelMat = Mat4f::rotationZ( 0.0f, 1.0f );
         break;
      default:
         CHECK( false );
         accelMat = Mat4f::identity();
         break;
   }
}

@end //@interface FusionAppDelegate (Orientation)


FusionAppDelegate*  mainDelegate()
{
   return _mainDelegate;
}
