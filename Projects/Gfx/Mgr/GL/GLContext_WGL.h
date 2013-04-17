/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef GFX_GL_CONTEXT__WGL_H
#define GFX_GL_CONTEXT__WGL_H

#include <Gfx/StdDefs.h>

#if GFX_OGL_SUPPORT

#include <Gfx/Mgr/GL/GLContext.h>

#include <Base/Util/windows.h>

NAMESPACE_BEGIN

namespace Gfx
{

/*==============================================================================
   CLASS GLContext_WGL
==============================================================================*/

class GLContext_WGL:
   public GLContext
{

public:

   /*----- methods -----*/

   GFX_DLL_API GLContext_WGL( HWND window );

   GFX_DLL_API virtual ~GLContext_WGL();

   GFX_DLL_API virtual bool  vsync() const;
   GFX_DLL_API virtual void  vsync( bool v );
   GFX_DLL_API virtual bool  makeCurrent();
   GFX_DLL_API virtual void  swapBuffers();

private:

   bool  init( HWND window );

   /*----- data members -----*/

   HGLRC   _glContext;
   HWND    _winHandle;
};


}  //namespace Gfx

NAMESPACE_END

#endif //GFX_OGL_SUPPORT

#endif //GFX_GL_CONTEXT__WGL_H
