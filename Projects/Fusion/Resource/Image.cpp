/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Resource/Image.h>
#include <Fusion/Resource/BitmapManipulator.h>
#include <Fusion/Core/Core.h>


USING_NAMESPACE

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
void
toTexture(
   const RCP<Bitmap>&    bmp,
   Gfx::TextureFormat&   fmt,
   Gfx::TextureChannels& ch
)
{
   if( bmp->pixelType() == Bitmap::BYTE )
   {
      switch( bmp->numChannels() )
      {
         case  1: fmt = Gfx::TEX_FMT_8;       ch = Gfx::TEX_CHANS_L;           break;
         case  2: fmt = Gfx::TEX_FMT_8_8;     ch = Gfx::TEX_CHANS_LA;          break;
         case  4: fmt = Gfx::TEX_FMT_8_8_8_8; ch = Gfx::TEX_CHANS_RGBA;        break;
         default: fmt = Gfx::TEX_FMT_INVALID; ch = Gfx::TEX_CHANS_UNSPECIFIED; break;
      }
   }
   else
   {
      switch( bmp->numChannels() )
      {
         case  1: fmt = Gfx::TEX_FMT_32F;             ch = Gfx::TEX_CHANS_L;           break;
         case  2: fmt = Gfx::TEX_FMT_32F_32F;         ch = Gfx::TEX_CHANS_LA;          break;
         case  4: fmt = Gfx::TEX_FMT_32F_32F_32F_32F; ch = Gfx::TEX_CHANS_RGBA;        break;
         default: fmt = Gfx::TEX_FMT_INVALID;         ch = Gfx::TEX_CHANS_UNSPECIFIED; break;
      }
   }
}

//------------------------------------------------------------------------------
//! Verifies if the specified texture is suitable for the bitmap.
//! Receives extra relevant parameters that a computed in createTexture().
bool suitable( const Gfx::Texture& tex, const Bitmap& bmp, uint w, uint h, uint d, Gfx::TextureFormat fmt, Gfx::TextureChannels ch )
{
   switch( bmp.dimType() )
   {
      case Bitmap::DIM_2D:
         if( tex.width() != w || tex.height() != h )  return false;
         break;
      case Bitmap::DIM_CUBEMAP:
         CHECK( d == 6 );
         if( tex.width() != w || tex.height() != h || tex.depth() != d )  return false;
         break;
      case Bitmap::DIM_2D_ARRAY:
      case Bitmap::DIM_3D:
         if( tex.width() != w || tex.height() != h || tex.depth() != d )  return false;
         break;
      default:
         return false;
   }
   // Require format and channel order to match.
   return tex.format() == fmt && tex.channelOrder() == ch;
}

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   CLASS Image
==============================================================================*/

//------------------------------------------------------------------------------
//!
Image::Image( const String& path ):
   _needUpdate( true ), _sourcePath( path )
{
}
//------------------------------------------------------------------------------
//!
Image::Image( Gfx::Texture* t ):
   _needUpdate( false ), _texture( t )
{
}
//------------------------------------------------------------------------------
//!
Image::Image( Bitmap* b ):
   _needUpdate( true ), _bitmap( b )
{
}

//------------------------------------------------------------------------------
//!
Image::~Image()
{
}

//------------------------------------------------------------------------------
//!
void
Image::init( const Vec2i& dim, const Bitmap::PixelType type, const int numChannels )
{
   CHECK( _bitmap.isNull() );
   _bitmap = new Bitmap( dim, type, numChannels );
}

//------------------------------------------------------------------------------
//!
void
Image::load()
{
   load( _sourcePath );
}

//------------------------------------------------------------------------------
//!
void
Image::load( const String& path )
{
   _bitmap = new Bitmap();
   _bitmap->load( path );
}

//------------------------------------------------------------------------------
//!
void
Image::loadCubemap( const String& path )
{
   _bitmap = new Bitmap();
   _bitmap->loadCubemap( path );
}

//------------------------------------------------------------------------------
//!
Gfx::Texture*
Image::texture()
{
   if( _needUpdate ) createTexture();
   return _texture.ptr();
}

//------------------------------------------------------------------------------
//!
void
Image::bitmap( Bitmap* bitmap )
{
   _bitmap     = bitmap;
   _needUpdate = true;
}

//------------------------------------------------------------------------------
//!
void
Image::createTexture()
{
   if( _bitmap.isNull() ) return;

   RCP<Bitmap> bmp = _bitmap;
   // No support for 3 channels.
   if( bmp->numChannels() == 3 )
   {
      bmp = BitmapManipulator::addAlpha( *bmp, 1.0f );
   }

   uint width  = bmp->dimension().x;
   uint height = bmp->dimension().y;
   uint depth  = bmp->dimension().z;

   // Currently force power-of-2 size.
   uint p2_width  = CGM::nextPow2( width );
   uint p2_height = CGM::nextPow2( height );

   // Convert to texture format.
   Gfx::TextureFormat fmt;
   Gfx::TextureChannels ch;
   toTexture( bmp, fmt, ch );

   // Create texture.
   if( _texture.isNull() || !suitable( *_texture, *bmp, p2_width, p2_height, depth, fmt, ch ) )
   {
      switch( bmp->dimType() )
      {
         case Bitmap::DIM_2D:
            _texture = Core::gfx()->create2DTexture( p2_width, p2_height, fmt, ch, Gfx::TEX_FLAGS_MIPMAPPED );
            break;
         case Bitmap::DIM_CUBEMAP:
            _texture = Core::gfx()->createCubeTexture( p2_width, fmt, ch, Gfx::TEX_FLAGS_MIPMAPPED );
            break;
         case Bitmap::DIM_2D_ARRAY:
            //_texture = Core::gfx()->create2DArrayTexture( p2_width, p2_height, depth, fmt, ch, Gfx::TEX_FLAGS_MIPMAPPED );
            break;
         case Bitmap::DIM_3D:
            _texture = Core::gfx()->create3DTexture( p2_width, p2_height, depth, fmt, ch, Gfx::TEX_FLAGS_MIPMAPPED );
            break;
      }
   }

   // Temporary code to clear around texture.
   if( width < p2_width || height < p2_height )
   {
      size_t tmpSize = CGM::max( width, height ) * bmp->pixelSize();
      Vector<uchar> tmpBuffer( tmpSize, 0 );

      if( height < p2_height && height != _texture->definedWidth() )
         Core::gfx()->setData( _texture, 0, 0, height, width, 1, tmpBuffer.data() );
      if( width < p2_width  && width != _texture->definedHeight() )
         Core::gfx()->setData( _texture, 0, width, 0, 1, height, tmpBuffer.data() );
   }

   _texture->definedRegionX().reset();
   _texture->definedRegionY().reset();

   switch( bmp->dimType() )
   {
      case Bitmap::DIM_2D:
         Core::gfx()->setData( _texture, 0, 0, 0, width, height, bmp->pixels() );
         break;
      case Bitmap::DIM_CUBEMAP:
         Core::gfx()->setData( _texture, 0, Gfx::TEX_SLICE_NEG_X, bmp->pixels(0) );
         Core::gfx()->setData( _texture, 0, Gfx::TEX_SLICE_POS_X, bmp->pixels(1) );
         Core::gfx()->setData( _texture, 0, Gfx::TEX_SLICE_NEG_Y, bmp->pixels(2) );
         Core::gfx()->setData( _texture, 0, Gfx::TEX_SLICE_POS_Y, bmp->pixels(3) );
         Core::gfx()->setData( _texture, 0, Gfx::TEX_SLICE_NEG_Z, bmp->pixels(4) );
         Core::gfx()->setData( _texture, 0, Gfx::TEX_SLICE_POS_Z, bmp->pixels(5) );
         break;
      case Bitmap::DIM_2D_ARRAY:
         for( uint s = 0; s < depth; ++s )
         {
            Core::gfx()->setData( _texture, 0, s, 0, 0, width, height, bmp->pixels(s) );
         }
         break;
      case Bitmap::DIM_3D:
         Core::gfx()->setData( _texture, 0, 0, 0, 0, width, height, depth, bmp->pixels() );
         break;
   }
   Core::gfx()->generateMipmaps( _texture );

   // Should we delete the image?
   _needUpdate = false;
}

//------------------------------------------------------------------------------
//!
void
Image::invalidateTexture()
{
   _needUpdate = true;
}

NAMESPACE_END
