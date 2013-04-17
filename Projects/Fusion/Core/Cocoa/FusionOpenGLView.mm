/*==============================================================================
   Copyright (c) 2007, Ludo Sapiens Inc.
   All rights reserved.

   These coded instructions, statements, and computer programs contain
   unpublished, proprietary information and are protected by Federal copyright
   law. They may not be disclosed to third parties or copied or duplicated in
   any form, in whole or in part, without prior written consent.
==============================================================================*/
#import <Fusion/Core/Cocoa/FusionOpenGLView.h>

#include <Fusion/Core/Core.h>

#include <Base/Dbg/DebugStream.h>

#import <OpenGL/CGLRenderers.h>

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_cocoa, "CoreCocoa" );

UNNAMESPACE_END


@implementation FusionOpenGLView

+ (void)_keepAtLinkTime
{
   // Do nothing (this is just to force the class to be kept at link time).
}

- (void)awakeFromNib
{
   DBG_BLOCK( os_cocoa, "FusionOpenGLView awakeFromNib" );
   // Resize to what Core expects.
   Vec2i s = Core::size();
   [self setSize:NSMakeSize(s.x, s.y)];
   [[self window] center];
   // Want to receive mouseMove events.
   [[self window] setAcceptsMouseMovedEvents:YES];
}

- (id)initWithFrame:(NSRect)frameRect
{
   DBG_BLOCK( os_cocoa, "FusionOpenGLView initWithFrame:" );
   // Pixel Format Attributes for the View-based (non-FullScreen) NSOpenGLContext
   NSOpenGLPixelFormatAttribute attrs[] =
   {

       // Specifying "NoRecovery" gives us a context that cannot fall back to the software renderer.
       // This makes the View-based context a compatible with the fullscreen context, enabling us to
       // use the "shareContext" feature to share textures, display lists, and other OpenGL objects
       // between the two.
       NSOpenGLPFANoRecovery,

       // Attributes Common to FullScreen and non-FullScreen
       NSOpenGLPFAColorSize, (NSOpenGLPixelFormatAttribute)24,
       NSOpenGLPFADepthSize, (NSOpenGLPixelFormatAttribute)16,
       NSOpenGLPFADoubleBuffer,

       // GPU-acceleration or software renderer (currently ignored because the NIB creates the context).
       NSOpenGLPFAAccelerated,
       //NSOpenGLPFARendererID, (NSOpenGLPixelFormatAttribute)kCGLRendererAppleSWID,        // Since kCGLRendererGenericID is deprecated.
       //NSOpenGLPFARendererID, (NSOpenGLPixelFormatAttribute)kCGLRendererGenericFloatID, // The floating-point software renderer.

       // Attributes for MSAA
       //NSOpenGLPFASampleBuffers, (NSOpenGLPixelFormatAttribute)4, // 2, 4, 6, 8, more?

       0
   };
   GLint rendererID;

   // Create our non-FullScreen pixel format.
   NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];

   // Just as a diagnostic, report the renderer ID that this pixel format binds to.  CGLRenderers.h contains a list of known renderers and their corresponding RendererID codes.
   [pixelFormat getValues:&rendererID forAttribute:NSOpenGLPFARendererID forVirtualScreen:0];
   NSLog(@"NSOpenGLView pixelFormat RendererID = %08x", (unsigned)rendererID);

   self = [super initWithFrame:frameRect pixelFormat:pixelFormat];
   [pixelFormat release];
   return self;
}

- (void) drawRect:(NSRect)aRect
{
   //DBG_BLOCK( os_cocoa, "FusionOpenGLView drawRect:" );
   //NSLog(@"drawRect");
   [super drawRect:aRect];
   // No region optimization for now, so redraw everything.
   [fusionController render];
}

- (void) reshape
{
   DBG_BLOCK( os_cocoa, "FusionOpenGLView reshape" );
   [super reshape];
   NSRect rect = [self bounds];
   [fusionController reshape: (int)rect.size.width: (int)rect.size.height];
   [self setNeedsDisplay:YES];
}

//-----------------------------------------------------------------------------
//! Resizes the parent window so that the view has the specified resolution.
- (void)setSize:(NSSize)size
{
   NSRect wrect = [[self window] frame];
   NSRect vrect = [self frame];
   NSRect frame;
   frame.origin = wrect.origin;
   frame.size.width  = size.width  + (wrect.size.width  - vrect.size.width );
   frame.size.height = size.height + (wrect.size.height - vrect.size.height);
   [[self window] setFrame:frame display:YES animate:NO];
}

- (BOOL) acceptsFirstResponder
{
   // We want this view to be able to receive key events.
   return YES;
}

// Delegate to our controller object for handling key and mouse events.

- (void)keyDown:(NSEvent*)theEvent
{
   [fusionController keyDown:theEvent];
   [self interpretKeyEvents:[NSArray arrayWithObject:theEvent]];
}

- (void)keyUp:(NSEvent*)theEvent
{
   [fusionController keyUp:theEvent];
}

- (void)mouseDown:(NSEvent*)theEvent
{
   [fusionController mouseDown:theEvent];
}

- (void)mouseDragged:(NSEvent*)theEvent
{
   [fusionController mouseDragged:theEvent];
}

- (void)mouseMoved:(NSEvent*)theEvent
{
   [fusionController mouseMoved:theEvent];
}

- (void)mouseUp:(NSEvent*)theEvent
{
   [fusionController mouseUp:theEvent];
}

- (void)rightMouseDown:(NSEvent*)theEvent
{
   [fusionController rightMouseDown:theEvent];
}

- (void)rightMouseDragged:(NSEvent*)theEvent
{
   [fusionController rightMouseDragged:theEvent];
}

- (void)rightMouseUp:(NSEvent*)theEvent
{
   [fusionController rightMouseUp:theEvent];
}

- (void)otherMouseDown:(NSEvent*)theEvent
{
   [fusionController otherMouseDown:theEvent];
}

- (void)otherMouseDragged:(NSEvent*)theEvent
{
   [fusionController otherMouseDragged:theEvent];
}

- (void)otherMouseUp:(NSEvent*)theEvent
{
   [fusionController otherMouseUp:theEvent];
}

- (void)scrollWheel:(NSEvent*)theEvent
{
   [fusionController scrollWheel:theEvent];
}

- (void)flagsChanged:(NSEvent*)theEvent
{
   [fusionController flagsChanged:theEvent];
}

- (BOOL)performKeyEquivalent:(NSEvent *)theEvent
{
   unused(theEvent);
   [fusionController keyDown:theEvent];
   return TRUE;
}

// @protocol NSTextInputClient

//-----------------------------------------------------------------------------
//!
- (BOOL)hasMarkedText
{
   return [fusionController hasMarkedText];
}

//-----------------------------------------------------------------------------
//!
- (NSRange)markedRange
{
   return [fusionController markedRange];
}

//-----------------------------------------------------------------------------
//!
- (NSRange)selectedRange
{
   return [fusionController selectedRange];
}

//-----------------------------------------------------------------------------
//!
- (void)setMarkedText:(id)aString selectedRange:(NSRange)selectedRange replacementRange:(NSRange)replacementRange
{
   [fusionController setMarkedText:aString selectedRange:selectedRange replacementRange:replacementRange];
}

//-----------------------------------------------------------------------------
//!
- (void)unmarkText
{
   [fusionController unmarkText];
}

//-----------------------------------------------------------------------------
//!
- (NSArray *)validAttributesForMarkedText
{
   return [fusionController validAttributesForMarkedText];
}

//-----------------------------------------------------------------------------
//!
- (NSAttributedString *)attributedSubstringForProposedRange:(NSRange)aRange actualRange:(NSRangePointer)actualRange
{
   return [fusionController attributedSubstringForProposedRange:aRange actualRange:actualRange];
}

//-----------------------------------------------------------------------------
//!
- (void)insertText:(id)aString replacementRange:(NSRange)replacementRange
{
   [fusionController insertText:aString replacementRange:replacementRange];
}

//-----------------------------------------------------------------------------
//!
- (NSUInteger)characterIndexForPoint:(NSPoint)aPoint
{
   return [fusionController characterIndexForPoint:aPoint];
}

//-----------------------------------------------------------------------------
//!
- (NSRect)firstRectForCharacterRange:(NSRange)aRange actualRange:(NSRangePointer)actualRange
{
   return [fusionController firstRectForCharacterRange:aRange actualRange:actualRange];
}

//-----------------------------------------------------------------------------
//!
- (void)doCommandBySelector:(SEL)aSelector
{
   [fusionController doCommandBySelector:aSelector];
}

@end //@implementation FusionOpenGLView
