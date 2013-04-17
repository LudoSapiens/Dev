/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef GFX_GLES_CONTEXT_ANDROID_H
#define GFX_GLES_CONTEXT_ANDROID_H

#include <Gfx/StdDefs.h>

#include <Gfx/Mgr/GLES/GLESContext.h>

NAMESPACE_BEGIN

namespace Gfx
{

/*==============================================================================
   CLASS GLESContext_Android
==============================================================================*/

class GLESContext_Android:
   public GLESContext
{
public:
   /*----- types -----*/

   /*----- methods -----*/

   GFX_DLL_API GLESContext_Android( uint desiredVersion );

   GFX_DLL_API virtual ~GLESContext_Android();

   GFX_DLL_API virtual bool  vsync() const;
   GFX_DLL_API virtual void  vsync( bool v );

   GFX_DLL_API virtual bool  makeCurrent();
   GFX_DLL_API virtual void  swapBuffers();
   //GFX_DLL_API virtual void  updateSize();

protected:
};

}  //namespace Gfx

NAMESPACE_END

#endif //GFX_GLES_CONTEXT_ANDROID_H
