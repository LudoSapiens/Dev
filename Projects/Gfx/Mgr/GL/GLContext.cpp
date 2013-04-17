/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Gfx/Mgr/GL/GLContext.h>

#if GFX_OGL_SUPPORT

#include <Gfx/Mgr/GL/GLManager.h>

#include <Base/Dbg/DebugStream.h>

USING_NAMESPACE

using namespace Gfx;

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_gl, "GLManager" );

UNNAMESPACE_END


/*==============================================================================
   CLASS GLContext
==============================================================================*/

//------------------------------------------------------------------------------
//!
GLContext::GLContext()
{
   DBG_BLOCK( os_gl, "GLContext::GLContext()" );
}

//------------------------------------------------------------------------------
//!
GLContext::~GLContext()
{
   DBG_BLOCK( os_gl, "GLContext::~GLContext()" );
}

//------------------------------------------------------------------------------
//!
RCP<Manager>
GLContext::createManager()
{
   DBG_BLOCK( os_gl, "GLContext::createManager()" );
   return new GLManager( this );
}

#endif //GFX_OGL_SUPPORT
