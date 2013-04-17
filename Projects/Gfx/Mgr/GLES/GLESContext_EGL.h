/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef GFX_GLES_CONTEXT__EGL_H
#define GFX_GLES_CONTEXT__EGL_H

#include <Gfx/Mgr/GLES/GLESContext.h>

#if GFX_GLES_EGL_SUPPORT

#include <EGL/egl.h>
#include <GLES2/gl2.h>

NAMESPACE_BEGIN

namespace Gfx
{

/*==============================================================================
   CLASS GLESContext_EGL
==============================================================================*/

class GLESContext_EGL:
   public GLESContext
{

public:

   /*----- methods -----*/

   GFX_DLL_API GLESContext_EGL( EGLNativeDisplayType nativeDisplay, EGLNativeWindowType nativeWindow );

   GFX_DLL_API virtual ~GLESContext_EGL();

   GFX_DLL_API virtual bool  vsync() const;
   GFX_DLL_API virtual void  vsync( bool v );
   GFX_DLL_API virtual bool  makeCurrent();
   GFX_DLL_API virtual void  swapBuffers();

private:

   /*----- data members -----*/
   EGLNativeDisplayType  _nativeDisplay;
   EGLNativeWindowType   _nativeWindow;
   EGLDisplay            _eglDisplay;
   EGLConfig             _eglConfig;
   EGLSurface            _eglSurface;
   EGLContext            _eglContext;
};


}  //namespace Gfx

NAMESPACE_END

#endif //GFX_GLES_EGL_SUPPORT

#endif //GFX_GLES_CONTEXT__EGL_H
