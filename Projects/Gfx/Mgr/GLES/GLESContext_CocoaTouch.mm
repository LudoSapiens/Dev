/*==============================================================================
   Copyright (c) 2009, Ludo Sapiens Inc.
   All rights reserved.

   These coded instructions, statements, and computer programs contain
   unpublished, proprietary information and are protected by Federal copyright
   law. They may not be disclosed to third parties or copied or duplicated in
   any form, in whole or in part, without prior written consent.
==============================================================================*/
#include <Gfx/Mgr/GLES/GLESContext_CocoaTouch.h>

#include <Base/Dbg/DebugStream.h>

//#import <OpenGLES/EAGL.h>
#import <OpenGLES/EAGLDrawable.h>

#if GFX_GLES_2_SUPPORT
#  import <OpenGLES/ES2/gl.h>
#endif

#if  GFX_GLES_1_SUPPORT
#  import <OpenGLES/ES1/gl.h>
#  import <OpenGLES/ES1/glext.h>
#  if GFX_GLES_2_SUPPORT == 0
#    define GL_COLOR_ATTACHMENT0       GL_COLOR_ATTACHMENT0_OES
#    define GL_DEPTH_ATTACHMENT        GL_DEPTH_ATTACHMENT_OES
#    define GL_DEPTH_COMPONENT16       GL_DEPTH_COMPONENT16_OES
#    define GL_FRAMEBUFFER             GL_FRAMEBUFFER_OES
#    define GL_FRAMEBUFFER_COMPLETE    GL_FRAMEBUFFER_COMPLETE_OES
#    define GL_RENDERBUFFER            GL_RENDERBUFFER_OES
#    define GL_RENDERBUFFER_HEIGHT     GL_RENDERBUFFER_HEIGHT_OES
#    define GL_RENDERBUFFER_WIDTH      GL_RENDERBUFFER_WIDTH_OES
#  endif
#endif

#import <QuartzCore/QuartzCore.h>

#define ZBUFFER 1

USING_NAMESPACE

using namespace Gfx;

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_es, "GLESContext" );

UNNAMESPACE_END


/*==============================================================================
   CLASS GLESContext_CocoaTouch
==============================================================================*/

//------------------------------------------------------------------------------
//!
GLESContext_CocoaTouch::GLESContext_CocoaTouch( CAEAGLLayer* eaglLayer, uint desiredVersion ):
   _layer( eaglLayer ),
   _curW( 0 ), _curH( 0 )
{
   DBG_BLOCK( os_es, "GLESContext_CocoaTouch::GLESContext_CocoaTouch(" << (void*)eaglLayer << ")" );
   DBG_MSG( os_es, "this=" << (void*)this );

   DBG_MSG( os_es, "Setting default CAEAGLLayer properties" );
   eaglLayer.opaque = YES;
   eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                     // Value                        Key
                                     [NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking,
                                     kEAGLColorFormatRGBA8       , kEAGLDrawablePropertyColorFormat,
                                     nil];

#if GFX_GLES_1_SUPPORT && GFX_GLES_2_SUPPORT
   // Try GLES 2 first, and fallback to GLES 1.1 otherwise, unless we force a version 1.1 to begin with.
   if( desiredVersion == 1 )
   {
      _context = NULL; // Force to take GLES1 path below.
   }
   else
   {
      _context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
   }
   if( _context == NULL )
   {
      version( 1 );
      _context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
      _api.glBindFramebuffer            = &glBindFramebufferOES;
      _api.glBindRenderbuffer           = &glBindRenderbufferOES;
      _api.glCheckFramebufferStatus     = &glCheckFramebufferStatusOES;
      _api.glFramebufferRenderbuffer    = &glFramebufferRenderbufferOES;
      _api.glGenFramebuffers            = &glGenFramebuffersOES;
      _api.glGenRenderbuffers           = &glGenRenderbuffersOES;
      _api.glRenderbufferStorage        = &glRenderbufferStorageOES;
   }
   else
   {
      version( 2 );
      _api.glBindFramebuffer            = &glBindFramebuffer;
      _api.glBindRenderbuffer           = &glBindRenderbuffer;
      _api.glCheckFramebufferStatus     = &glCheckFramebufferStatus;
      _api.glFramebufferRenderbuffer    = &glFramebufferRenderbuffer;
      _api.glGenFramebuffers            = &glGenFramebuffers;
      _api.glGenRenderbuffers           = &glGenRenderbuffers;
      _api.glRenderbufferStorage        = &glRenderbufferStorage;
   }
#elif GFX_GLES_1_SUPPORT
   desiredVersion = desiredVersion; // Remove warning about unused parameter.
   version( 1 );
   _context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
   _api.glBindFramebuffer         = &glBindFramebufferOES;
   _api.glBindRenderbuffer        = &glBindRenderbufferOES;
   _api.glCheckFramebufferStatus  = &glCheckFramebufferStatusOES;
   _api.glFramebufferRenderbuffer = &glFramebufferRenderbufferOES;
   _api.glGenFramebuffers         = &glGenFramebuffersOES;
   _api.glGenRenderbuffers        = &glGenRenderbuffersOES;
   _api.glRenderbufferStorage     = &glRenderbufferStorageOES;
#elif GFX_GLES_2_SUPPORT
   desiredVersion = desiredVersion; // Remove warning about unused parameter.
   version( 2 );
   _context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
   _api.glBindFramebuffer         = &glBindFramebuffer;
   _api.glBindRenderbuffer        = &glBindRenderbuffer;
   _api.glCheckFramebufferStatus  = &glCheckFramebufferStatus;
   _api.glFramebufferRenderbuffer = &glFramebufferRenderbuffer;
   _api.glGenFramebuffers         = &glGenFramebuffers;
   _api.glGenRenderbuffers        = &glGenRenderbuffers;
   _api.glRenderbufferStorage     = &glRenderbufferStorage;
#else
#   error GLESContext requires at least some GFX_GLES_*_SUPPORT.
#endif

   DBG_MSG( os_es, "Created context: " << (void*)_context );

   if( !_context || ![EAGLContext setCurrentContext:_context] )
   {
      CHECK( false );
      return;
   }

   CGSize size = _layer.bounds.size;
   DBG_MSG( os_es, "Initial size: " << size.width << "x" << size.height );
   unused( size );

   _api.glGenFramebuffers( 1, &_frameBuffer );
   _api.glBindFramebuffer( GL_FRAMEBUFFER, _frameBuffer );

   _api.glGenRenderbuffers( 1, &_colorBuffer );
   _api.glBindRenderbuffer( GL_RENDERBUFFER, _colorBuffer );
   _api.glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _colorBuffer );

#if ZBUFFER
   _api.glGenRenderbuffers( 1, &_depthBuffer );
   _api.glBindRenderbuffer( GL_RENDERBUFFER, _depthBuffer );
   _api.glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT , GL_RENDERBUFFER, _depthBuffer );
#endif

   //_api.glBindFramebuffer( GL_FRAMEBUFFER, 0 );

   DBG_MSG( os_es, "_frameBuffer=" << _frameBuffer << " _colorBuffer=" << _colorBuffer << " _depthBuffer=" << _depthBuffer );
   //CHECK( _api.glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE );
}

//------------------------------------------------------------------------------
//!
GLESContext_CocoaTouch::~GLESContext_CocoaTouch()
{
   DBG_BLOCK( os_es, "GLESContext_CocoaTouch::~GLESContext_CocoaTouch()" );
   if( [EAGLContext currentContext] == _context )
   {
      DBG_MSG( os_es, "Disconnecting currently active context" );
      [EAGLContext setCurrentContext:nil];
   }
   DBG_MSG( os_es, "Destroying context: " << (void*)_context );
   [_context release];
}

//------------------------------------------------------------------------------
//!
bool
GLESContext_CocoaTouch::vsync() const
{
   // As of 2010-04-05, it sounds like we don't have VSYNC support in CocoaTouch.
   // Ref:
   //   http://stackoverflow.com/questions/2087622/iphone-deactivate-vsync
   // Also, according to:
   //   http://www.khronos.org/opengles/sdk/1.1/docs/man/eglSwapInterval.xml
   // eglSwapInterval is set to 1 by default.
   return true;
}

//------------------------------------------------------------------------------
//!
void
GLESContext_CocoaTouch::vsync( bool /*v*/ )
{
}

//------------------------------------------------------------------------------
//!
bool
GLESContext_CocoaTouch::makeCurrent()
{
   DBG_BLOCK( os_es, "GLESContext_CocoaTouch::makeCurrent()" );
   return [EAGLContext setCurrentContext:_context];
}

//------------------------------------------------------------------------------
//!
void
GLESContext_CocoaTouch::swapBuffers()
{
   DBG_BLOCK( os_es, "GLESContext_CocoaTouch::swapBuffers()" );
   _api.glBindRenderbuffer( GL_RENDERBUFFER, _colorBuffer );
   [_context presentRenderbuffer:GL_RENDERBUFFER];
}

//------------------------------------------------------------------------------
//!
void
GLESContext_CocoaTouch::updateSize()
{
   DBG_BLOCK( os_es, "GLESContext_CocoaTouch::updateSize()" );

   CGSize size = _layer.bounds.size;

   if( _curW == size.width && _curH == size.height )  return;

   _curW = size.width;
   _curH = size.height;

   //_api.glBindFramebuffer( GL_FRAMEBUFFER, _frameBuffer );

   // Allocate color buffer storage.
   _api.glBindRenderbuffer( GL_RENDERBUFFER, _colorBuffer );
   [_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:_layer]; // For presentRenderbuffer:.

#if ZBUFFER
   // Allocate depth buffer storage.
   _api.glBindRenderbuffer( GL_RENDERBUFFER, _depthBuffer );
   _api.glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, _curW, _curH );
#endif

   //_api.glBindFramebuffer( GL_FRAMEBUFFER, 0 );

   CHECK( _api.glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE );
}
