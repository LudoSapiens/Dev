/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Render/DeferredRenderer.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN


UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
  CLASS DeferredRenderer
==============================================================================*/

//------------------------------------------------------------------------------
//!
DeferredRenderer::DeferredRenderer()
{
}

//------------------------------------------------------------------------------
//!
DeferredRenderer::~DeferredRenderer()
{
}

//------------------------------------------------------------------------------
//!
void
DeferredRenderer::render(
   const RCP<Gfx::RenderNode>& /*rn*/,
   World*                      /*world*/,
   Viewport*                   /*vp*/
)
{
}

//------------------------------------------------------------------------------
//! 
void
DeferredRenderer::performResize()
{
}

//------------------------------------------------------------------------------
//! 
void
DeferredRenderer::performBeginFrame()
{
}

//------------------------------------------------------------------------------
//! 
void
DeferredRenderer::performEndFrame()
{
}


NAMESPACE_END
