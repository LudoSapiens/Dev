/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_TEXTURE_MOSAIC_H
#define PLASMA_TEXTURE_MOSAIC_H

#include <Plasma/StdDefs.h>

#include <CGMath/Vec2.h>

#include <Base/Util/RCObject.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS TextureMosaic
==============================================================================*/

class TextureMosaic:
   public RCObject
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API TextureMosaic();

   // Creation.
   uint addTexture( const Vec2i& );
   void removeTexture( uint );
   void resizeTexture( uint, const Vec2i& );

   // Attributes.
   inline const Vec2i& dimension( uint i ) const { return _textures[i]._dim; }

   // Access.
   inline float* buffer( uint );
   inline float* buffer( uint, int y );
   inline float* buffer( uint, const Vec2i& pos );

protected:

   /*----- methods -----*/

   ~TextureMosaic();
   void releaseMemory();

   /*----- structures -----*/

   struct Texture
   {
      Vec2i  _dim;
      float* _data;
   };

   /*----- data members -----*/

   Vector<Texture> _textures;
};

//------------------------------------------------------------------------------
//!
inline float*
TextureMosaic::buffer( uint i )
{
   return _textures[i]._data;
}

//------------------------------------------------------------------------------
//!
inline float*
TextureMosaic::buffer( uint i, int y )
{
   Texture& t = _textures[i];
   return &t._data[y*t._dim.x];
}

//------------------------------------------------------------------------------
//!
inline float*
TextureMosaic::buffer( uint i, const Vec2i& pos )
{
   Texture& t = _textures[i];
   return &t._data[pos.x + pos.y*t._dim.x];
}

NAMESPACE_END

#endif
