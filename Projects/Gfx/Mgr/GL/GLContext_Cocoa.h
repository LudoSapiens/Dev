/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef GFX_GL_CONTEXT_COCOA_H
#define GFX_GL_CONTEXT_COCOA_H

#include <Fusion/StdDefs.h>

#include <Gfx/Mgr/GL/GLContext.h>

@class NSOpenGLContext;

NAMESPACE_BEGIN

namespace Gfx
{


/*==============================================================================
  CLASS CocoaContext
==============================================================================*/
class GLContext_Cocoa:
   public GLContext
{
public:

   /*----- methods -----*/

   GLContext_Cocoa();
   virtual ~GLContext_Cocoa();

   virtual bool vsync() const;
   virtual void vsync( bool v );

   NSOpenGLContext*  context() { return _context; }
   void  context( NSOpenGLContext* c ) { _context = c; makeCurrent(); }

   virtual bool  makeCurrent();
   virtual void  swapBuffers();

protected:

   /*----- data members -----*/

   NSOpenGLContext*  _context;

   /*----- methods -----*/

   /* methods... */

private:
}; //class GLContext_Cocoa


} //namespace Gfx

NAMESPACE_END

#endif //GFX_GL_CONTEXT_COCOA_H
