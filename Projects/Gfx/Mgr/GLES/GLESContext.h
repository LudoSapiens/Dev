/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef GFX_GLES_CONTEXT_H
#define GFX_GLES_CONTEXT_H

#include <Gfx/StdDefs.h>

#if GFX_GLES_1_SUPPORT | GFX_GLES_2_SUPPORT

#include <Gfx/Mgr/Context.h>

//#include <Base/Util/RCObject.h>

NAMESPACE_BEGIN

namespace Gfx
{

/*==============================================================================
   CLASS GLESContext
==============================================================================*/

class GLESContext:
   public Context
{
public:

   /*----- methods -----*/

   GFX_DLL_API GLESContext();

   GFX_DLL_API virtual ~GLESContext();

   GFX_DLL_API virtual bool  makeCurrent() = 0;
   GFX_DLL_API virtual void  swapBuffers() = 0;
   GFX_DLL_API virtual void  updateSize();

   inline uint  frameBuffer() const { return _frameBuffer; }
   inline uint  colorBuffer() const { return _colorBuffer; }
   inline uint  depthBuffer() const { return _depthBuffer; }

   inline uint  version() const { return _version; }

protected:
   uint  _frameBuffer;  //!< Framebuffer ID of the backbuffer.
   uint  _colorBuffer;  //!< Renderbuffer ID of the color backbuffer.
   uint  _depthBuffer;  //!< Renderbuffer ID of the depth backbuffer.
   uint  _version;

   inline void  version( uint version ) { _version = version; }

   GFX_DLL_API virtual RCP<Manager>  createManager();
};


}  //namespace Gfx

NAMESPACE_END

#endif //GFX_GLES_1_SUPPORT | GFX_GLES_2_SUPPORT

#endif //GFX_GLES_CONTEXT_H
