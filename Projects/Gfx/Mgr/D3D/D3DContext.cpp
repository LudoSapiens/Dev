/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Gfx/Mgr/D3D/D3DContext.h>

#if GFX_D3D_SUPPORT

#include <Gfx/Mgr/D3D/D3DManager.h>

#include <Base/Dbg/DebugStream.h>

USING_NAMESPACE

using namespace Gfx;

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_d3d, "D3DManager" );

UNNAMESPACE_END


/*==============================================================================
   CLASS D3DContext
==============================================================================*/

//------------------------------------------------------------------------------
//!
D3DContext::D3DContext( HWND window ):
   _window( window )
{
   DBG_BLOCK( os_d3d, "D3DContext::D3DContext()" );
}

//------------------------------------------------------------------------------
//!
D3DContext::~D3DContext()
{
   DBG_BLOCK( os_d3d, "D3DContext::~D3DContext()" );
}

//------------------------------------------------------------------------------
//!
bool
D3DContext::vsync() const
{
   return false;
}

//------------------------------------------------------------------------------
//!
void
D3DContext::vsync( bool /*v*/ )
{
}

//------------------------------------------------------------------------------
//!
RCP<Manager>
D3DContext::createManager()
{
   DBG_BLOCK( os_d3d, "D3DContext::createManager()" );
   return new D3DManager( this );
}

#endif //GFX_D3D_SUPPORT
