/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Gfx/Mgr/GLES/GLESContext_Android.h>

#include <Base/Dbg/DebugStream.h>

USING_NAMESPACE

using namespace Gfx;

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_es, "GLESContext" );

UNNAMESPACE_END


/*==============================================================================
   CLASS GLESContext_Android
==============================================================================*/

//------------------------------------------------------------------------------
//!
GLESContext_Android::GLESContext_Android( uint ver )
{
   DBG_BLOCK( os_es, "GLESContext_Android::GLESContext_Android(" << ver << ")" );
   version( ver );
}

//------------------------------------------------------------------------------
//!
GLESContext_Android::~GLESContext_Android()
{
}

//------------------------------------------------------------------------------
//!
bool
GLESContext_Android::vsync() const
{
   return true;
}

//------------------------------------------------------------------------------
//!
void
GLESContext_Android::vsync( bool /*v*/ )
{
}

//------------------------------------------------------------------------------
//!
bool
GLESContext_Android::makeCurrent()
{
   DBG_BLOCK( os_es, "GLESContext_Android::makeCurrent()" );
}

//------------------------------------------------------------------------------
//!
void
GLESContext_Android::swapBuffers()
{
   DBG_BLOCK( os_es, "GLESContext_Android::swapBuffers()" );
}
