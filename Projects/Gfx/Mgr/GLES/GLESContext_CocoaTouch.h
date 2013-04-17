/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef GFX_GLES_CONTEXT_COCOA_TOUCH_H
#define GFX_GLES_CONTEXT_COCOA_TOUCH_H

#include <Gfx/StdDefs.h>

#include <Gfx/Mgr/GLES/GLESContext.h>

@class CAEAGLLayer;
@class EAGLContext;

NAMESPACE_BEGIN

namespace Gfx
{

/*==============================================================================
   CLASS GLESContext_CocoaTouch
==============================================================================*/

class GLESContext_CocoaTouch:
   public GLESContext
{
public:
   /*----- types -----*/
   struct APICallbacks
   {
      typedef unsigned int  GLenum;
      typedef int           GLsizei;
      typedef unsigned int  GLuint;
      void (*glBindFramebuffer)(GLenum, GLuint);
      void (*glBindRenderbuffer)(GLenum, GLuint);
      GLenum (*glCheckFramebufferStatus)(GLenum);
      void (*glFramebufferRenderbuffer)(GLenum, GLenum, GLenum, GLuint);
      void (*glGenFramebuffers)(GLsizei, GLuint*);
      void (*glGenRenderbuffers)(GLsizei, GLuint*);
      void (*glRenderbufferStorage)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
   };

   /*----- methods -----*/

   GFX_DLL_API GLESContext_CocoaTouch( CAEAGLLayer* layer, uint desiredVersion );

   GFX_DLL_API virtual ~GLESContext_CocoaTouch();

   EAGLContext*  context() const { return _context; }

   GFX_DLL_API virtual bool  vsync() const;
   GFX_DLL_API virtual void  vsync( bool v );

   GFX_DLL_API virtual bool  makeCurrent();
   GFX_DLL_API virtual void  swapBuffers();
   GFX_DLL_API virtual void  updateSize();

protected:
   CAEAGLLayer*  _layer;
   EAGLContext*  _context;
   APICallbacks  _api;
   int           _curW;
   int           _curH;
};


}  //namespace Gfx

NAMESPACE_END


#endif //GFX_GLES_CONTEXT_COCOA_TOUCH_H
