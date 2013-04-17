/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef GFX_D3D_CONTEXT_H
#define GFX_D3D_CONTEXT_H

#include <Gfx/StdDefs.h>

#if GFX_D3D_SUPPORT

#include <Gfx/Mgr/Context.h>

#include <Base/Util/windows.h>

NAMESPACE_BEGIN

namespace Gfx
{

class Manager;

/*==============================================================================
  CLASS D3DContext
==============================================================================*/
class D3DContext:
   public Context
{
public:

   /*----- methods -----*/

   GFX_DLL_API D3DContext( HWND window );
   GFX_DLL_API virtual ~D3DContext();

   GFX_DLL_API virtual bool  vsync() const;
   GFX_DLL_API virtual void  vsync( bool v );

   HWND  window() const { return _window; }

protected:

   friend class D3DManager;

   /*----- members -----*/

   HWND   _window;

   /*----- methods -----*/

   GFX_DLL_API virtual RCP<Manager>  createManager();

private:
}; //class D3DContext


}  //namespace Gfx

NAMESPACE_END

#endif //GFX_D3D_SUPPORT

#endif //GFX_D3D_CONTEXT_H
