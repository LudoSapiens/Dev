/*==============================================================================
   Copyright (c) 2007, Ludo Sapiens Inc.
   All rights reserved.

   These coded instructions, statements, and computer programs contain
   unpublished, proprietary information and are protected by Federal copyright
   law. They may not be disclosed to third parties or copied or duplicated in
   any form, in whole or in part, without prior written consent.
==============================================================================*/

#import "GfxOpenGLView.h"

//------------------------------------------------------------------------

static inline GLboolean CheckForExtension(const char *extensionName, const GLubyte *extensions)
{
   GLboolean  bExtensionAvailable = gluCheckExtension((GLubyte *)extensionName, extensions);

   return bExtensionAvailable;
} // CheckForExtension

//------------------------------------------------------------------------

static inline void CheckForAndLogExtensionAvailable(const GLboolean extensionAvailable, const char *extensionName)
{
   if (extensionAvailable == GL_FALSE)
   {
      NSLog(@">> WARNING: \"%s\" extension is not available!\n", extensionName);
   } // if
} // CheckForExtensions

//------------------------------------------------------------------------

static void CheckForShaders(NSOpenGLPixelFormatAttribute *thePixelAttributes)
{
   const GLubyte *extensions = glGetString(GL_EXTENSIONS);

   GLboolean  bShaderObjectAvailable      = CheckForExtension("GL_ARB_shader_objects", extensions);
   GLboolean  bShaderLanguage100Available = CheckForExtension("GL_ARB_shading_language_100", extensions);
   GLboolean  bVertexShaderAvailable      = CheckForExtension("GL_ARB_vertex_shader", extensions);
   GLboolean  bFragmentShaderAvailable    = CheckForExtension("GL_ARB_fragment_shader", extensions);

   GLboolean  bForceSoftwareRendering =      (bShaderObjectAvailable      == GL_FALSE)
                                          || (bShaderLanguage100Available == GL_FALSE)
                                          || (bVertexShaderAvailable      == GL_FALSE)
                                          || (bFragmentShaderAvailable    == GL_FALSE);

   if (bForceSoftwareRendering)
   {
      // Force software rendering, so fragment shaders will execute

      CheckForAndLogExtensionAvailable(bShaderObjectAvailable, "GL_ARB_shader_objects");
      CheckForAndLogExtensionAvailable(bShaderLanguage100Available, "GL_ARB_shading_language_100");
      CheckForAndLogExtensionAvailable(bVertexShaderAvailable, "GL_ARB_vertex_shader");
      CheckForAndLogExtensionAvailable(bFragmentShaderAvailable, "GL_ARB_fragment_shader");

      thePixelAttributes [3] = NSOpenGLPFARendererID;
      thePixelAttributes [4] = (NSOpenGLPixelFormatAttribute)kCGLRendererGenericFloatID;
   } // if
} // CheckForShaders

//------------------------------------------------------------------------

static void CheckForGLSLHardwareSupport(NSOpenGLPixelFormatAttribute *thePixelAttributes)
{
   // Create a pre-flight context to check for GLSL hardware support

   NSOpenGLPixelFormat  *pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes: thePixelAttributes];

   if (pixelFormat != nil)
   {
      NSOpenGLContext *preflight = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];

      if (preflight != nil)
      {
         [preflight makeCurrentContext];

            CheckForShaders(thePixelAttributes);

         [preflight   release];
      } // if

      [pixelFormat release];
   } // if
} // CheckForGLSLHardwareSupport

//------------------------------------------------------------------------

//------------------------------------------------------------------------

@implementation GfxOpenGLView

+ (void) _keepAtLinkTime
{
   // Do nothing (this is just to force the class to be kept at link time).
}

//------------------------------------------------------------------------

- (GLcharARB *) getShaderSourceFromResource:(NSString *)theShaderResourceName extension:(NSString *)theExtension
{
   NSBundle  *appBundle         = [NSBundle mainBundle];
   NSString  *shaderTempSource  = [appBundle pathForResource:theShaderResourceName ofType:theExtension];
   GLcharARB *shaderSource      = NULL;

   shaderTempSource = [NSString stringWithContentsOfFile:shaderTempSource encoding:NSUTF8StringEncoding error:NULL];
   shaderSource     = (GLcharARB *)[shaderTempSource cStringUsingEncoding:NSASCIIStringEncoding];

   return  shaderSource;
} // getShaderSourceFromResource

//------------------------------------------------------------------------

-(void) setupOpenGL
{
   //-----------------------------------------------------------------
   //
   // For some OpenGL implementations, texture coordinates generated
   // during rasterization aren't perspective correct. However, you
   // can usually make them perspective correct by calling the API
   // glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST).  Colors
   // generated at the rasterization stage aren't perspective correct
   // in almost every OpenGL implementation, / and can't be made so.
   // For this reason, you're more likely to encounter this problem
   // with colors than texture coordinates.
   //
   //-----------------------------------------------------------------

   glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);

   // Set up the projection

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glFrustum(-0.3, 0.3, 0.0, 0.6, 1.0, 8.0);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslatef(0.0, 0.0, -2.0);

   // Turn on depth test

    glEnable(GL_DEPTH_TEST);

   // front- or back-facing facets can be culled

    glEnable(GL_CULL_FACE);
} // setupOpenGL

//------------------------------------------------------------------------

//------------------------------------------------------------------------

- (id) initWithFrame:(NSRect)theFrame pixelFormat:(NSOpenGLPixelFormat*)thePixelFormat;
{
   // Create a GL Context to use - i.e. init the superclass

   if(thePixelFormat == nil)
   {
      NSOpenGLPixelFormatAttribute thePixelAttributes []
                                 =  {
                                       NSOpenGLPFADoubleBuffer,
                                       NSOpenGLPFADepthSize,   (NSOpenGLPixelFormatAttribute)24,
                                       NSOpenGLPFAStencilSize, (NSOpenGLPixelFormatAttribute)8,
                                       (NSOpenGLPixelFormatAttribute)0
                                    };

      CheckForGLSLHardwareSupport(thePixelAttributes);

      thePixelFormat = [[[NSOpenGLPixelFormat alloc] initWithAttributes:thePixelAttributes] autorelease];
   } //  if

   if ( (self = [super initWithFrame:theFrame pixelFormat:thePixelFormat]) )
   {
      // Basic GL initializations

      [[self openGLContext] makeCurrentContext];

      [self setupOpenGL];

      // Did the frame change?

      [self setPostsFrameChangedNotifications:YES];

      // Basic initializations
      printf("--> sf\n"); fflush(NULL);

      [self setFrame:theFrame];


   } // if

   return self;
} // initWithFrame

//------------------------------------------------------------------------

- (id) initWithFrame: (NSRect)theFrame
{
   return [self initWithFrame:theFrame pixelFormat:nil];
} // initWithFrame

//------------------------------------------------------------------------

- (void) setViewport
{
   NSRect   theBounds = [self bounds];
   GLfloat  theWidth  = NSWidth(theBounds);
   GLfloat  theHeight = NSHeight(theBounds);

   glViewport(0, 0, theWidth/2, theHeight);
} // setViewport

//------------------------------------------------------------------------

- (void) reshape
{
   // Update Gfx
   [self setViewport];
}

//------------------------------------------------------------------------

- (void) drawRect: (NSRect) theRect
{
   [[self openGLContext] makeCurrentContext];

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
   glClearColor(1.0f, 0.0f, 0.0f, 1.0f);

   glBegin(GL_TRIANGLE_FAN);
      glColor4d(0.0, 0.0, 1.0, 1.0);
      glVertex3d(0.0, 0.0, 0.0);
      glVertex3d(1.0, 0.0, 0.0);
      glVertex3d(1.0, 1.0, 0.0);
   glEnd();

   [[self openGLContext] flushBuffer];
} // drawRect

//------------------------------------------------------------------------

#pragma mark -- Handling mouse events --

//------------------------------------------------------------------------

//------------------------------------------------------------------------

- (void)mouseDown:(NSEvent *)theEvent
{
   printf("mouseDown: %g, %g\n", [theEvent locationInWindow].x, [theEvent locationInWindow].y);
   lastMousePoint  = [self convertPoint:[theEvent locationInWindow] fromView:nil];
   leftMouseIsDown = YES;
} // mouseDown

//------------------------------------------------------------------------

- (void)rightMouseDown:(NSEvent *)theEvent
{
   lastMousePoint   = [self convertPoint:[theEvent locationInWindow] fromView:nil];
   rightMouseIsDown = YES;
} // rightMouseDown

//------------------------------------------------------------------------

- (void)mouseUp:(NSEvent *)theEvent
{
   leftMouseIsDown = NO;
} // mouseUp

//------------------------------------------------------------------------

- (void)rightMouseUp:(NSEvent *)theEvent
{
   rightMouseIsDown = NO;
} // rightMouseUp

//------------------------------------------------------------------------

- (void)mouseDragged:(NSEvent *)theEvent
{
   if ([theEvent modifierFlags] & NSRightMouseDown)
   {
      [self rightMouseDragged:theEvent];
   } // if
   else
   {
      NSPoint mouse = [self convertPoint:[theEvent locationInWindow] fromView:nil];
      // Send to Core
      [self setNeedsDisplay:YES];
   } // else
} // mouseDragged

//------------------------------------------------------------------------

- (void)rightMouseDragged:(NSEvent *)theEvent
{
   NSPoint mouse = [self convertPoint:[theEvent locationInWindow] fromView:nil];

   [self setNeedsDisplay:YES];
} // rightMouseDragged

//------------------------------------------------------------------------

@end

//------------------------------------------------------------------------

//------------------------------------------------------------------------
