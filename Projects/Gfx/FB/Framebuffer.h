/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef GFX_FRAMEBUFFER_H
#define GFX_FRAMEBUFFER_H

#include <Gfx/StdDefs.h>
#include <Gfx/Tex/Texture.h>

#include <Base/Util/RCP.h>
#include <Base/Util/Bits.h>

NAMESPACE_BEGIN

namespace Gfx
{


class Framebuffer:
   public RCObject
{

public:

   /*----- methods -----*/

   virtual ~Framebuffer();

   inline void  setColorBuffer( const RCP<Texture>& cb, const uint level = 0 );
   inline void  setDepthBuffer( const RCP<Texture>& db, const uint level = 0 );
   inline void  setStencilBuffer( const RCP<Texture>& sb, const uint level = 0 );

   //! Verifies that all of the specified buffers have the same dimension
   GFX_DLL_API bool  checkConsistency() const;

   GFX_DLL_API uint  width() const;
   GFX_DLL_API uint  height() const;

   inline bool  hasColorBuffer()   const { return _colorBuffer.isValid();   }
   inline bool  hasDepthBuffer()   const { return _depthBuffer.isValid();   }
   inline bool  hasStencilBuffer() const { return _stencilBuffer.isValid(); }

protected:
   /*----- data members -----*/

   typedef enum
   {
      FB_BUFFER_BIT_NONE    = 0x00,
      FB_BUFFER_BIT_COLOR   = 0x01,
      FB_BUFFER_BIT_DEPTH   = 0x02,
      FB_BUFFER_BIT_STENCIL = 0x04,
      FB_BUFFER_BIT_ALL     = 0x07
   } BufferBit;

   mutable uchar  _dirty;  //!< One dirty bit per buffer (_dirty[0] = color, _dirty[1] = depth, _dirty[2] = stencil)

   // If NULL, indicates we don't have that buffer
   RCP<Texture>  _colorBuffer;
   uint          _colorBufferLevel;
   RCP<Texture>  _depthBuffer;
   uint          _depthBufferLevel;
   RCP<Texture>  _stencilBuffer;
   uint          _stencilBufferLevel;

   // Only GfxManagers can create this object
   Framebuffer();

   // Some utility routine
   bool  isDirty( BufferBit bb ) const { return (_dirty & bb) != 0; }
   void  setDirty( BufferBit bb ) const { _dirty |= bb; }
   void  unsetDirty( BufferBit bb ) const { _dirty &= ~bb; }


private:
   GFX_MAKE_MANAGERS_FRIENDS();

};

//------------------------------------------------------------------------------
//!
inline
Framebuffer::Framebuffer
( void ):
   _dirty(0),
   _colorBuffer(NULL),
   _colorBufferLevel(0),
   _depthBuffer(NULL),
   _depthBufferLevel(0),
   _stencilBuffer(NULL),
   _stencilBufferLevel(0)
{
}

//------------------------------------------------------------------------------
//!
inline
Framebuffer::~Framebuffer
( void )
{
}

//------------------------------------------------------------------------------
//!
inline
void
Framebuffer::setColorBuffer
( const RCP<Texture>& cb, const uint level )
{
   _colorBuffer = cb;
   _colorBufferLevel = level;
   setDirty(FB_BUFFER_BIT_COLOR);
}

//------------------------------------------------------------------------------
//!
inline
void
Framebuffer::setDepthBuffer
( const RCP<Texture>& db, const uint level )
{
   _depthBuffer = db;
   _depthBufferLevel = level;
   setDirty(FB_BUFFER_BIT_DEPTH);
}

//------------------------------------------------------------------------------
//!
inline
void
Framebuffer::setStencilBuffer
( const RCP<Texture>& sb, const uint level )
{
   _stencilBuffer = sb;
   _stencilBufferLevel = level;
   setDirty(FB_BUFFER_BIT_STENCIL);
}


}  //namespace Gfx

NAMESPACE_END


#endif //GFX_FRAMEBUFFER_H
