/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef GFX_GL_CONTEXT_H
#define GFX_GL_CONTEXT_H

#include <Gfx/StdDefs.h>

#if GFX_OGL_SUPPORT

#include <Gfx/Mgr/Context.h>

NAMESPACE_BEGIN

namespace Gfx
{


/*==============================================================================
   CLASS GLContext
==============================================================================*/

class GLContext:
   public Context
{
public:

   /*----- methods -----*/

   GFX_DLL_API GLContext();
   GFX_DLL_API virtual ~GLContext();

   GFX_DLL_API virtual bool  makeCurrent() = 0;
   GFX_DLL_API virtual void  swapBuffers() = 0;

protected:
   GFX_DLL_API virtual RCP<Manager>  createManager();
};


}  //namespace Gfx

NAMESPACE_END

#endif //GFX_OGL_SUPPORT

#endif //GFX_GL_CONTEXT_H
