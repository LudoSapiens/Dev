/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Gfx/FB/RenderState.h>
#include <Base/Dbg/DebugStream.h>


USING_NAMESPACE

using namespace Gfx;

/*==============================================================================
   CLASS AlphaState
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
AlphaState::setDefaults()
{
   alphaBlending( true );
   setAlphaBlend( ALPHA_BLEND_ONE, ALPHA_BLEND_ONE_MINUS_SRC_ALPHA );
   alphaTesting( false );
   alphaTestRef( 0.0f );
   alphaTestFunc( COMPARE_FUNC_ALWAYS );
}

/*==============================================================================
   CLASS DepthState
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
DepthState::setDefaults()
{
   depthTesting( true );
   depthTestFunc( COMPARE_FUNC_LESSEQUAL );
   depthWriting( true );
}

/*==============================================================================
   CLASS ColorState
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
ColorState::setDefaults()
{
   colorWriting( true );
}

/*==============================================================================
   CLASS CullState
==============================================================================*/

//------------------------------------------------------------------------------
//! 
void
CullState::setDefaults()
{
   _side = FACE_BACK;
}

/*==============================================================================
   CLASS StencilState
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
StencilState::setDefaults
( void )
{
   stencilTesting( false );
   stencilTestRef( 0 );
   stencilTestRefMask( 0xFFFFFFFF );
   stencilTestWriteMask( 0xFFFFFFFF );
   stencilTestFunc( COMPARE_FUNC_ALWAYS );
   setStencilOps( STENCIL_OP_KEEP, STENCIL_OP_KEEP, STENCIL_OP_KEEP );
}

/*==============================================================================
   CLASS OffsetState
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
OffsetState::setDefaults()
{
   factor(0.0f);
   constant(0.0f);
}
