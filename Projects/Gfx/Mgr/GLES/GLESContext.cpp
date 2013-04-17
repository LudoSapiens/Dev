/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Gfx/Mgr/GLES/GLESContext.h>

#if GFX_GLES_1_SUPPORT | GFX_GLES_2_SUPPORT

#include <Gfx/Mgr/Manager.h>

#if GFX_GLES_1_SUPPORT
#  include <Gfx/Mgr/GLES/GLES1Manager.h>
#endif

#if GFX_GLES_2_SUPPORT
#  include <Gfx/Mgr/GLES/GLES2Manager.h>
#endif

#include <Base/Dbg/DebugStream.h>

USING_NAMESPACE

using namespace Gfx;

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_gles, "GLESContext" );

UNNAMESPACE_END


/*==============================================================================
   CLASS GLESContext
==============================================================================*/

//------------------------------------------------------------------------------
//!
GLESContext::GLESContext():
   _frameBuffer( 0 ),
   _colorBuffer( 0 ),
   _depthBuffer( 0 ),
   _version( 0 )
{
   DBG_BLOCK( os_gles, "GLESContext::GLESContext()" );
}

//------------------------------------------------------------------------------
//!
GLESContext::~GLESContext()
{
   DBG_BLOCK( os_gles, "GLESContext::~GLESContext()" );
}

//------------------------------------------------------------------------------
//!
void
GLESContext::updateSize()
{
   DBG_BLOCK( os_gles, "GLESContext::updateSize()" );
}

//------------------------------------------------------------------------------
//!
RCP<Manager>
GLESContext::createManager()
{
   DBG_BLOCK( os_gles, "GLESContext::createManager()" );
   switch( version() )
   {
#if GFX_GLES_1_SUPPORT
      case 1:  return new GLES1Manager( this );
#endif
#if GFX_GLES_2_SUPPORT
      case 2:  return new GLES2Manager( this );
#endif
      default: CHECK( false );  return NULL;
   }
}

#endif //GFX_GLES_1_SUPPORT | GFX_GLES_2_SUPPORT
