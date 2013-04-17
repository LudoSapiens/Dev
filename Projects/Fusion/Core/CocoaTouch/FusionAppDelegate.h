/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#import <UIKit/UIKit.h>

#include <Base/IO/TextStream.h>

#include <CGMath/Mat4.h>

#include <Fusion/Core/Core.h>

USING_NAMESPACE

@class FusionEAGLView;
@class FusionEAGLViewController;

@interface FusionAppDelegate : NSObject <UIApplicationDelegate,UIAccelerometerDelegate,UITextFieldDelegate>
{
    UIWindow*                  window;
    FusionEAGLView*            glView;
    FusionEAGLViewController*  glViewController;
    UITextField*               keyboardField;

@private
    // Animation
    NSTimer*        animationTimer;
    NSTimeInterval  animationInterval;

    // Accelerometer
    Mat4f           accelMat;

}

@property (nonatomic, retain) IBOutlet UIWindow*                  window;
@property (nonatomic, retain) IBOutlet FusionEAGLView*            glView;
@property (nonatomic, retain) IBOutlet FusionEAGLViewController*  glViewController;
@property (nonatomic, retain) IBOutlet UITextField*               keyboardField;
@property (nonatomic)                  NSTimeInterval             animationInterval;

- (void) applicationWillTerminate: (UIApplication*)application;
- (void) printInfo: (TextStream&)os;
- (void) resize: (CGSize)size;

@end //@interface FusionAppDelegate

@interface FusionAppDelegate (Animation)
- (BOOL) isAnimating;
- (void) startAnimation;
- (void) stopAnimation;
- (void) toggleAnimation;
- (void) animationTimerFired:(NSTimer*)timer;
@end //@interface FusionAppDelegate (Animation)

@interface FusionAppDelegate (Events)
- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event;
- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event;
- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event;
- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event;
- (void)motionBegan:(UIEventSubtype)motion withEvent:(UIEvent *)event;
- (void)motionCancelled:(UIEventSubtype)motion withEvent:(UIEvent *)event;
- (void)motionEnded:(UIEventSubtype)motion withEvent:(UIEvent *)event;
- (void)accelerometer:(UIAccelerometer *)accelerometer didAccelerate:(UIAcceleration *)acceleration;
@end //@interface FusionAppDelegate (Events)

@interface FusionAppDelegate (Keyboard)
- (void)registerKeyboardNotifications;
- (void)keyboardWillShow:(NSNotification*)notification;
- (void)keyboardDidShow:(NSNotification*)notification;
- (void)keyboardWillHide:(NSNotification*)notification;
- (void)keyboardDidHide:(NSNotification*)notification;
- (IBAction)editingChanged:(UITextField*)textField;
- (IBAction)editingDidBegin:(UITextField*)textField;
- (IBAction)editingDidEnd:(UITextField*)textField;
- (IBAction)editingDidEndOnExit:(UITextField*)textField;
@end //@interface FusionAppDelegate (Keyboard)

@interface FusionAppDelegate (Orientation)
- (void)changeOrientation:(Core::Orientation)newOri;
@end //@interface FusionAppDelegate (Orientation)

FusionAppDelegate*  mainDelegate();
