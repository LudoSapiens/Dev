/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef GFX_GL_CONTEXT__GLX_H
#define GFX_GL_CONTEXT__GLX_H

#include <Gfx/StdDefs.h>
#include <Gfx/Mgr/GL/GLContext.h>

#define GLX_GLXEXT_PROTOTYPES 1

#include <X11/Xlib.h>
#include <GL/glx.h>
#include <GL/glxext.h>

NAMESPACE_BEGIN

namespace Gfx
{

/*==============================================================================
   CLASS GLContext_GLX
==============================================================================*/
   
class GFX_DLL_API GLContext_GLX:
   public GLContext
{
public:

   /*----- types -----*/

   struct WindowStructure
   {
      Display*  _display;
      Window    _window;
   };

   /*----- methods -----*/

   GLContext_GLX( WindowStructure* ws );
   GLContext_GLX( Display* display, Window window );

   virtual ~GLContext_GLX();

   virtual bool  vsync() const;
   virtual void  vsync( bool v );

   virtual bool  makeCurrent();
   virtual void  swapBuffers();

   bool  init( Display* display, Window window );

protected:
   
   /*----- data members -----*/

   Display*      _display;
   Window        _window;
   XVisualInfo*  _vinfo;
   GLXContext    _context;

private:

}; //class GLContext_GLX


}  //namespace Gfx

NAMESPACE_END


#endif //GFX_GL_CONTEXT__GLX_H
