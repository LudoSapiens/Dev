/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#import <CoreFoundation/CFDate.h>

#import <Foundation/NSAttributedString.h>
#import <Foundation/NSGeometry.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSObject.h>
#import <Foundation/NSString.h>
#import <Foundation/NSTimer.h>

#import <AppKit/NSNibDeclarations.h>
#import <AppKit/NSOpenGL.h>
#import <AppKit/NSResponder.h>

@class FusionOpenGLView;

@interface FusionController : NSResponder
{
   IBOutlet FusionOpenGLView*  fusionView;  //!< A pointer to the view.


   NSPoint             lastMousePoint;             // last place the mouse was 
   bool                leftMouseIsDown;            // was the left mouse button pressed?
   bool                rightMouseIsDown;           // was the right mouse button pressed?

   // Model
   BOOL isAnimating;
   NSTimer* animationTimer;
   CFAbsoluteTime timeBefore;

   BOOL stayInFullScreenMode;
   NSOpenGLContext* fullScreenContext;

   BOOL        _running;
   NSUInteger  _oldModifiers;

   NSMutableAttributedString*  _markedText;
   bool                        _lastKeyDownRepeat;
   double                      _lastKeyDownTimestamp;
}

+ (void) _keepAtLinkTime;

- (IBAction) goFullScreen:(id)sender;
- (BOOL) isInFullScreenMode;

@end

@interface FusionController (Animation)
- (BOOL) isAnimating;
- (void) startAnimation;
- (void) stopAnimation;
- (void) toggleAnimation;
- (void) startAnimationTimer;
- (void) stopAnimationTimer;
- (void) animationTimerFired:(NSTimer*)timer;
@end //@interface FusionController (Animation)

@interface FusionController (NSOpenGLView)
- (void) reshape:(int)w:(int)h;
- (void) render;
@end //@interface FusionController (NSOpenGLView)

@interface FusionController (NSView)
- (void)keyDown:(NSEvent*)theEvent;
- (void)keyUp:(NSEvent*)theEvent;
- (void)flagsChanged:(NSEvent*)theEvent;
- (void)mouseDown:(NSEvent*)theEvent;
- (void)mouseDragged:(NSEvent*)theEvent;
- (void)mouseMoved:(NSEvent*)theEvent;
- (void)mouseUp:(NSEvent*)theEvent;
- (void)rightMouseDown:(NSEvent*)theEvent;
- (void)rightMouseDragged:(NSEvent*)theEvent;
- (void)rightMouseUp:(NSEvent*)theEvent;
- (void)otherMouseDown:(NSEvent*)theEvent;
- (void)otherMouseDragged:(NSEvent*)theEvent;
- (void)otherMouseUp:(NSEvent*)theEvent;
- (void)scrollWheel:(NSEvent *)theEvent;
@end //@interface FusionController (NSView)

@interface FusionController (NSWindow)
- (void)windowWillClose:(NSNotification *)notification;
@end //@interface FusionController (NSWindow)

@interface FusionController (NSTextInputClient)
- (BOOL)hasMarkedText;
- (NSRange)markedRange;
- (NSRange)selectedRange;
- (void)setMarkedText:(id)aString selectedRange:(NSRange)selectedRange replacementRange:(NSRange)replacementRange;
- (void)unmarkText;
- (NSArray *)validAttributesForMarkedText;
- (NSAttributedString*)attributedSubstringForProposedRange:(NSRange)aRange actualRange:(NSRangePointer)actualRange;
- (void)insertText:(id)aString replacementRange:(NSRange)replacementRange;
- (NSUInteger)characterIndexForPoint:(NSPoint)aPoint;
- (NSRect)firstRectForCharacterRange:(NSRange)aRange actualRange:(NSRangePointer)actualRange;
- (void)doCommandBySelector:(SEL)aSelector;
@end //@interface FusionController (NSTextInputClient)
