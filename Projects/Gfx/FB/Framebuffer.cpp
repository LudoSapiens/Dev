/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Gfx/FB/Framebuffer.h>

USING_NAMESPACE

using namespace Gfx;

//! Verifies that all of the specified buffers have the same dimension
bool
Framebuffer::checkConsistency
( void )
const
{
   if( _colorBuffer.isValid() )
   {
      if( _colorBuffer->type() == TEX_TYPE_2D )
      {
         if( _depthBuffer.isValid() )
         {
            return  _colorBuffer->levelWidth(_colorBufferLevel) == _depthBuffer->levelWidth(_depthBufferLevel) &&
                    _colorBuffer->levelHeight(_colorBufferLevel) == _depthBuffer->levelHeight(_depthBufferLevel) &&
                    _colorBuffer->type() == _depthBuffer->type();
         }
         else
         {
            // No depth buffer set, we assume sizes are equal
            return true;
         }
      }
      else
      {
         // Only 2D textures are supported
         return false;
      }
   }

   // Ignore depth/stencil inconsistencies for now
   return true;
}

uint
Framebuffer::width
( void )
const
{
   if( _colorBuffer.isValid()   )  return _colorBuffer->levelWidth(_colorBufferLevel);
   else
   if( _depthBuffer.isValid()   )  return _depthBuffer->levelWidth(_depthBufferLevel);
   else
   if( _stencilBuffer.isValid() )  return _stencilBuffer->levelWidth(_stencilBufferLevel);
   else
                                 return 0;
}

uint
Framebuffer::height
( void )
const
{
   if( _colorBuffer.isValid()   )  return _colorBuffer->levelHeight(_colorBufferLevel);
   else
   if( _depthBuffer.isValid()   )  return _depthBuffer->levelHeight(_depthBufferLevel);
   else
   if( _stencilBuffer.isValid() )  return _stencilBuffer->levelHeight(_stencilBufferLevel);
   else
                                 return 0;
}
