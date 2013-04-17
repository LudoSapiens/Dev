/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/DataFlow/TextureMosaic.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS TextureMosaic
==============================================================================*/

//------------------------------------------------------------------------------
//!
TextureMosaic::TextureMosaic()
{

}

//------------------------------------------------------------------------------
//!
TextureMosaic::~TextureMosaic()
{
   releaseMemory();
}

//------------------------------------------------------------------------------
//!
void TextureMosaic::releaseMemory()
{
   for( auto cur = _textures.begin(); cur != _textures.end(); ++cur )
   {
      free( cur->_data );
   }
}

//------------------------------------------------------------------------------
//!
uint TextureMosaic::addTexture( const Vec2i& d )
{
   _textures.pushBack( Texture() );
   Texture& t = _textures.back();
   t._dim     = d;
   t._data    = (float*)malloc( sizeof(float)*d.x*d.y );
   return uint(_textures.size())-1;
}

//------------------------------------------------------------------------------
//!
void TextureMosaic::removeTexture( uint i )
{
   _textures.erase( _textures.begin() + i );
}

//------------------------------------------------------------------------------
//!
void TextureMosaic::resizeTexture( uint, const Vec2i& )
{
   // TODO
}

NAMESPACE_END
