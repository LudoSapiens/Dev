/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Gfx/Pass/Pass.h>

#include <Gfx/Pass/RenderNode.h>


USING_NAMESPACE

using namespace Gfx;

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

UNNAMESPACE_END

/*==============================================================================
   CLASS Pass
==============================================================================*/
const float Pass::_identityMatrix[16] = 
{ 
   1.0f, 0.0f, 0.0f, 0.0f,
   0.0f, 1.0f, 0.0f, 0.0f,
   0.0f, 0.0f, 1.0f, 0.0f,
   0.0f, 0.0f, 0.0f, 1.0f
};

//------------------------------------------------------------------------------
//!
Pass::Pass()
{
   clear();
}

//------------------------------------------------------------------------------
//!
Pass::~Pass()
{
}

//------------------------------------------------------------------------------
//!
void
Pass::clear()
{
   _size[0]  = 0;
   _size[1]  = 0;
   _size[2]  = 10000;
   _size[3]  = 10000;
   _scissor  = _size;
   _viewport = _size;

   _camPosition = NULL;
   _projMatrix  = _identityMatrix;
   _viewMatrix  = _identityMatrix;
   _worldMatrix = _identityMatrix;

   _color[0] = 0.0f;
   _color[1] = 0.0f;
   _color[2] = 0.0f;
   _color[3] = 1.0f;
   _z        = 1.0f;
   _stencil  = 0;

   _clearBuffers      = CLEAR_ALL;
   _fbGenerateMipmaps = false;

   _commands.clear();
   _ranges.clear();
   _rects.clear();
   _matrices.clear();
   _positions.clear();
}

//------------------------------------------------------------------------------
//!
void
Pass::render( const RCP<const RenderNode>& rn )
{
   Command cmd;
   cmd._id = PASS_CMD_RENDER;
   cmd._data = rn;
   _commands.pushBack(cmd);
}

//------------------------------------------------------------------------------
//!
void 
Pass::setClearColor( float r, float g, float b, float a )
{
   _color[0] = r;
   _color[1] = g;
   _color[2] = b;
   _color[3] = a;
}

//------------------------------------------------------------------------------
//!
void 
Pass::setClearDepth( float z )
{
   _z = z;
}

//------------------------------------------------------------------------------
//!
void 
Pass::setClearStencil( int s )
{
   _stencil = s;
}

//------------------------------------------------------------------------------
//!
void 
Pass::setClear( ClearType buffer )
{
   _clearBuffers = buffer;
}
