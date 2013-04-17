/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#import <Cocoa/Cocoa.h>

#import <OpenGL/CGLRenderers.h>
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>
#import <OpenGL/OpenGL.h>

@interface GfxOpenGLView : NSOpenGLView
{
   NSPoint             lastMousePoint;             // last place the mouse was 
   bool                leftMouseIsDown;            // was the left mouse button pressed?
   bool                rightMouseIsDown;           // was the right mouse button pressed?
} // GfxOpenGLView

+ (void) _keepAtLinkTime;

@end
