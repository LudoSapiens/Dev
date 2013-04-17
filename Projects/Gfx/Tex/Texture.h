/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef GFX_TEXTURE_H
#define GFX_TEXTURE_H

#include <Gfx/StdDefs.h>

#include <Base/Util/RCObject.h>
#include <Base/ADT/Vector.h>

#include <cassert>
#include <climits>


NAMESPACE_BEGIN

namespace Gfx
{

/*----- types -----*/

//------------------------------------------------------------------------------
//! The fields are order from LSB to MSB, therefore TEX_FMT_6_5_5 has
//!    bits[ 4: 0] = rightmost 5b channel
//!    bits[ 9: 5] = middle 5b channel
//!    bits[15:10] = the 6b channel
typedef enum
{
   TEX_FMT_INVALID,
   TEX_FMT_8,
   TEX_FMT_8_8,
   //TEX_FMT_8_8_8,     //Probably not supported
   TEX_FMT_8_8_8_8,
   TEX_FMT_16,
   TEX_FMT_16_16,
   //TEX_FMT_16_16_16,  //Probably not supported
   TEX_FMT_16_16_16_16,
   TEX_FMT_16F,
   TEX_FMT_16F_16F,
   //TEX_FMT_16F_16F_16F,  //Probably not supported
   TEX_FMT_16F_16F_16F_16F,
   TEX_FMT_32,
   TEX_FMT_32_32,
   //TEX_FMT_32_32_32,
   TEX_FMT_32_32_32_32,
   TEX_FMT_32F,
   TEX_FMT_32F_32F,
   //TEX_FMT_32F_32F_32F,
   TEX_FMT_32F_32F_32F_32F,
   TEX_FMT_24_8,
   TEX_FMT_8_24
} TextureFormat;

//------------------------------------------------------------------------------
//! The fields are order from LSB to MSB, in order to match the TextureFormat.
//! Therefore, if you have TEX_FMT_5_5_6 and TEX_CHANS_RGB, then you'll get:
//!    bits[ 5: 0] = red (6b)
//!    bits[10: 6] = green (5b)
//!    bits[15:11] = blue (5b)
typedef enum
{
   TEX_CHANS_UNSPECIFIED,
   TEX_CHANS_R,
   TEX_CHANS_G,
   TEX_CHANS_B,
   TEX_CHANS_A,
   TEX_CHANS_L,
   TEX_CHANS_Z,
   TEX_CHANS_S,
   TEX_CHANS_RG,
   TEX_CHANS_GR,
   TEX_CHANS_LA,
   TEX_CHANS_AL,
   TEX_CHANS_ZS,
   TEX_CHANS_SZ,
   TEX_CHANS_RGB,
   TEX_CHANS_BGR,
   TEX_CHANS_RGBA,
   TEX_CHANS_BGRA,
   TEX_CHANS_ARGB,
   TEX_CHANS_ABGR
} TextureChannels;

typedef enum
{
   TEX_TYPE_UNSPECIFIED,
   TEX_TYPE_1D,
   TEX_TYPE_1D_ARRAY,
   TEX_TYPE_2D,
   TEX_TYPE_2D_ARRAY,
   TEX_TYPE_3D,
   TEX_TYPE_CUBEMAP
} TextureType;

enum
{
   TEX_SLICE_NEG_X,
   TEX_SLICE_POS_X,
   TEX_SLICE_NEG_Y,
   TEX_SLICE_POS_Y,
   TEX_SLICE_NEG_Z,
   TEX_SLICE_POS_Z,
};

typedef enum
{
   TEX_FLAGS_NONE          = 0x00,
   TEX_FLAGS_MIPMAPPED     = 0x01,
   TEX_FLAGS_RENDERABLE    = 0x02
} TextureFlags;

inline TextureFlags  operator| ( TextureFlags f1, TextureFlags f2 )
{ return (TextureFlags)((int)f1 | (int)f2); }

inline bool  isMipmapped( const TextureFlags fl )
{ return (fl & TEX_FLAGS_MIPMAPPED) != 0; }

inline bool  isRenderable( const TextureFlags fl )
{ return (fl & TEX_FLAGS_RENDERABLE) != 0; }


/*==============================================================================
   CLASS TextureFormatInfo
==============================================================================*/
class TextureFormatInfo
{
public:
   TextureFormat format;   //!< The texture format
   uint numberOfChannels;  //!< The total number of channels in the format
   uint sizeInBits;        //!< The total number of bits
   uint sizeInBytes;       //!< The total size in bytes
   union {
      struct
      {
         uint size[4];     //!< The number of bits for every channel (index 0 is rightmost channel, in the LSBs)
         uint start[4];    //!< The start of bits for every channel (index 0 is rightmost channel, in the LSBs)
      } array;
      struct
      {
         uint sizeX;
         uint sizeY;
         uint sizeZ;
         uint sizeW;
         uint startX;
         uint startY;
         uint startZ;
         uint startW;
      } field;
   } channels;
   bool  isFloat;          //!< A bit indicating whether or not the format has a floating-point channel

   TextureFormatInfo( const TextureFormat format,
                      const uint numberOfChannels,
                      const uint sizeInBits, const uint sizeInBytes,
                      const uint chanSizeX, const uint chanSizeY, const uint chanSizeZ, const uint chanSizeW,
                      const uint isFloat ):
      format(format), numberOfChannels(numberOfChannels), sizeInBits(sizeInBits), sizeInBytes(sizeInBytes)
   {
      channels.field.sizeX = chanSizeX;
      channels.field.sizeY = chanSizeY;
      channels.field.sizeZ = chanSizeZ;
      channels.field.sizeW = chanSizeW;
      channels.field.startX = 0;
      channels.field.startY = channels.field.startX + chanSizeX;
      channels.field.startZ = channels.field.startY + chanSizeY;
      channels.field.startW = channels.field.startZ + chanSizeZ;
      assert( channels.field.sizeX == channels.array.size[0] );
      assert( channels.field.sizeY == channels.array.size[1] );
      assert( channels.field.sizeZ == channels.array.size[2] );
      assert( channels.field.sizeW == channels.array.size[3] );
      assert( channels.field.startX == channels.array.start[0] );
      assert( channels.field.startY == channels.array.start[1] );
      assert( channels.field.startZ == channels.array.start[2] );
      assert( channels.field.startW == channels.array.start[3] );
      this->isFloat = (isFloat & 0x01);
   }
};

//------------------------------------------------------------------------------
//! Retrieves information about a specified format in O(1) (actually, a simple table access)
GFX_DLL_API const TextureFormatInfo&  ask( const TextureFormat format );

/*==============================================================================
   CLASS Texture
==============================================================================*/
class Texture:
   public RCObject
{

public:
   /*----- structures -----*/

   /*==============================================================================
      CLASS Texture::Region
   ==============================================================================*/
   class Region
   {
   public:
      Region()
      {
         reset();
      }

      void  reset() { _min = UINT_MAX; _max = 0; }

      void  update( const uint v )
      {
         if( v < _min )  _min = v;
         if( _max < v )  _max = v;
      }

      void  setMin( const uint min ) { _min = min; }
      void  setMax( const uint max ) { _max = max; }
      void  setRange( const uint min, const uint max ) { _min = min; _max = max; }

      uint  getMin() const { return _min; }
      uint  getMax() const { return _max; }
      uint  getRange() const { return  _max - _min; }
      void  getRange( uint& min, uint& max ) const { min = _min; max = _max; }
      uint  getOffset() const { return _min; }

   protected:
      /*----- members -----*/
      uint  _min;
      uint  _max;
   };

   /*----- methods -----*/

   virtual ~Texture();

   TextureFormat  format() const { return _format; }

   void  channelOrder( const TextureChannels chOrder ) { _chOrder = chOrder; }
   TextureChannels  channelOrder() const { return _chOrder; }

   TextureType  type() const { return _type; }

   TextureFlags  flags() const { return _flags; }
   bool  isMipmapped() const { return Gfx::isMipmapped(_flags); }
   bool  isRenderable() const { return Gfx::isRenderable(_flags); }

   uint  width() const  { return _width;  }
   uint  height() const { return _height; }
   uint  depth() const  { return _depth;  }

   uint  levelWidth( const uint level ) const;
   uint  levelHeight( const uint level ) const;
   uint  levelDepth( const uint level ) const;

         Region&  definedRegionX()       { return _definedRegionX; }
   const Region&  definedRegionX() const { return _definedRegionX; }
         Region&  definedRegionY()       { return _definedRegionY; }
   const Region&  definedRegionY() const { return _definedRegionY; }
         Region&  definedRegionZ()       { return _definedRegionZ; }
   const Region&  definedRegionZ() const { return _definedRegionZ; }

   uint  definedWidth() const { return _definedRegionX.getRange(); }
   uint  definedHeight() const { return _definedRegionY.getRange(); }
   uint  definedDepth() const { return _definedRegionZ.getRange(); }
   uint  definedOffsetX() const { return _definedRegionX.getMin(); }
   uint  definedOffsetY() const { return _definedRegionY.getMin(); }
   uint  definedOffsetZ() const { return _definedRegionZ.getMin(); }

   //setDefinedWidth(w) would be problematic if we call setDefinedXOffset *after*
   void  definedOffsetX( const uint offset ) { _definedRegionX.setMin(offset); }
   void  definedOffsetY( const uint offset ) { _definedRegionY.setMin(offset); }
   void  definedOffsetZ( const uint offset ) { _definedRegionZ.setMin(offset); }
   void  updateDefinedRange( const uint x ) { _definedRegionX.update(x); }
   void  updateDefinedRange( const uint x, const uint y )
   { _definedRegionX.update(x); _definedRegionY.update(y); }
   void  updateDefinedRange( const uint x, const uint y, const uint z )
   { _definedRegionX.update(x); _definedRegionY.update(y); _definedRegionZ.update(z); }

   uint  getNumLevelsFromSize() const { return getNumLevelsFromSize(_width, _height, _depth); }

   /*----- static routines -----*/
   static GFX_DLL_API uint  getNumLevelsFromSize( uint width, uint height = 0, uint depth = 0 );
   static GFX_DLL_API uint  sizeAtLevel( const uint size, const uint level );

protected:

   /*----- data members -----*/

   TextureFormat    _format;   //!< The format of the data contained in the buffer
   TextureChannels  _chOrder;  //!< The channel order of the texture
   TextureType      _type;     //!< The type of data contained in the buffer
   TextureFlags     _flags;    //!< Some flags giving further information

   uint  _width;
   uint  _height;
   uint  _depth;  //also used for arrays

   Region  _definedRegionX;
   Region  _definedRegionY;
   Region  _definedRegionZ;

   Texture();

private:

   GFX_MAKE_MANAGERS_FRIENDS();

   //------------------------------------------------------------------------------
   //!
   void  format( const TextureFormat format ) { _format = format; }

   //------------------------------------------------------------------------------
   //!
   void  type( const TextureType type ) { _type = type; }

   //------------------------------------------------------------------------------
   //!
   void  flags( const TextureFlags flags ) { _flags = flags; }

   Texture&  set1D( uint width );
   Texture&  set1DArray( uint width, uint depth );

   Texture&  set2D( uint width, uint height );
   Texture&  set2DArray( uint width, uint height, uint depth );

   Texture&  set3D( uint width, uint height, uint depth );

   Texture&  setCubemap( uint edgeLength );
};

//------------------------------------------------------------------------------
//!
inline
Texture::Texture
( void ):
   _type(TEX_TYPE_UNSPECIFIED),
   _flags(TEX_FLAGS_NONE)
{
}

//------------------------------------------------------------------------------
//!
inline
Texture::~Texture
( void )
{
}

//------------------------------------------------------------------------------
//!
inline
uint
Texture::levelWidth
( const uint level )
const
{
   return sizeAtLevel(_width, level);
}

//------------------------------------------------------------------------------
//!
inline
uint
Texture::levelHeight
( const uint level )
const
{
   return sizeAtLevel(_height, level);
}

//------------------------------------------------------------------------------
//!
inline
uint
Texture::levelDepth
( const uint level )
const
{
   return sizeAtLevel(_depth, level);
}

//------------------------------------------------------------------------------
//!
inline
uint
toBytes
( const TextureFormat tfmt )
{
   return ask(tfmt).sizeInBytes;
}

//------------------------------------------------------------------------------
//! Translates all of the texels from one format to another
GFX_DLL_API bool
convert
( const uint numTexels,
  const TextureFormat srcFmt, const TextureChannels srcChans, const uchar* srcData,
  const TextureFormat dstFmt, const TextureChannels dstChans, uchar* dstData );

//------------------------------------------------------------------------------
//!
GFX_DLL_API const char*  toString( const TextureFormat tfmt );

//------------------------------------------------------------------------------
//!
GFX_DLL_API const char*  toString( const TextureChannels tChans );

//------------------------------------------------------------------------------
//!
GFX_DLL_API const char*  toString( const TextureType tType );

//------------------------------------------------------------------------------
//!
GFX_DLL_API const char*  toString( const TextureFlags tFlags );


} //namespace Gfx

NAMESPACE_END


#endif //GFX_TEXTURE_H
