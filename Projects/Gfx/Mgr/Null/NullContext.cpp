/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Gfx/Mgr/Null/NullContext.h>

#include <Gfx/Mgr/Null/NullManager.h>

#include <Base/Dbg/DebugStream.h>

USING_NAMESPACE

using namespace Gfx;

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_null, "NullContext" );

UNNAMESPACE_END


/*==============================================================================
   CLASS NullContext
==============================================================================*/

//------------------------------------------------------------------------------
//!
NullContext::NullContext
( void )
{
   DBG_BLOCK( os_null, "NullContext::NullContext()" );
}

//------------------------------------------------------------------------------
//!
NullContext::~NullContext
( void )
{
   DBG_BLOCK( os_null, "NullContext::~NullContext()" );
}

//------------------------------------------------------------------------------
//!
bool
NullContext::vsync() const
{
   return false;
}

//------------------------------------------------------------------------------
//!
void
NullContext::vsync( bool /*v*/ )
{
}

//------------------------------------------------------------------------------
//!
RCP<Manager>
NullContext::createManager()
{
   DBG_BLOCK( os_null, "NullContext::createManager()" );
   return new NullManager( this );
}
