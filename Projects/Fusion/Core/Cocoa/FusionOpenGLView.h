/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#import <Cocoa/Cocoa.h>

#import <AppKit/NSOpenGLView.h>
#import <AppKit/NSResponder.h>
#import <AppKit/NSTextInputClient.h>

#include <Fusion/Core/Cocoa/FusionController.h>

@class FusionController;

@interface FusionOpenGLView : NSOpenGLView<NSTextInputClient>
{
   // Controller
   IBOutlet FusionController*  fusionController; //!< A pointer to the controller.

} // FusionOpenGLView

+ (void) _keepAtLinkTime;

// @protocol NSTextInputClient
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
- (void)setSize:(NSSize)size;

@end
