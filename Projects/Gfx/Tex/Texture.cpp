/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Gfx/Tex/Texture.h>

#include <Base/Util/Bits.h>


USING_NAMESPACE

using namespace Gfx;

UNNAMESPACE_BEGIN

static const TextureFormatInfo sTextureFormatOracle[] = {
   //                format                  nCh    b   B    X  Y  Z  W  FLT
   TextureFormatInfo(TEX_FMT_INVALID        , 0,    0,  0,   0, 0, 0, 0,  0 ),
   TextureFormatInfo(TEX_FMT_8              , 1,    8,  1,   8, 0, 0, 0,  0 ),
   TextureFormatInfo(TEX_FMT_8_8            , 2,   16,  2,   8, 8, 0, 0,  0 ),
   //TextureFormatInfo(TEX_FMT_8_8_8          , 3,   24,  3,   8, 8, 8, 0,  0 ),
   TextureFormatInfo(TEX_FMT_8_8_8_8        , 4,   32,  4,   8, 8, 8, 8,  0 ),
   TextureFormatInfo(TEX_FMT_16             , 1,   16,  2,  16, 0, 0, 0,  0 ),
   TextureFormatInfo(TEX_FMT_16_16          , 2,   32,  4,  16,16, 0, 0,  0 ),
   //TextureFormatInfo(TEX_FMT_16_16_16       , 3,   48,  6,  16,16,16, 0,  0 ),
   TextureFormatInfo(TEX_FMT_16_16_16_16    , 4,   64,  8,  16,16,16,16,  0 ),
   TextureFormatInfo(TEX_FMT_16F            , 1,   16,  2,  16, 0, 0, 0,  1 ),
   TextureFormatInfo(TEX_FMT_16F_16F        , 2,   32,  4,  16,16, 0, 0,  1 ),
   //TextureFormatInfo(TEX_FMT_16F_16F_16F    , 3,   48,  6,  16,16,16, 0,  1 ),
   TextureFormatInfo(TEX_FMT_16F_16F_16F_16F, 4,   64,  8,  16,16,16,16,  1 ),
   TextureFormatInfo(TEX_FMT_32             , 1,   32,  4,  32, 0, 0, 0,  0 ),
   TextureFormatInfo(TEX_FMT_32_32          , 2,   64,  8,  32,32, 0, 0,  0 ),
   //TextureFormatInfo(TEX_FMT_32_32_32       , 3,   96, 12,  32,32,32, 0,  0 ),
   TextureFormatInfo(TEX_FMT_32_32_32_32    , 4,  128, 16,  32,32,32,32,  0 ),
   TextureFormatInfo(TEX_FMT_32F            , 1,   32,  4,  32, 0, 0, 0,  1 ),
   TextureFormatInfo(TEX_FMT_32F_32F        , 2,   64,  8,  32,32, 0, 0,  1 ),
   //TextureFormatInfo(TEX_FMT_32F_32F_32F    , 3,   96, 12,  32,32,32, 0,  1 ),
   TextureFormatInfo(TEX_FMT_32F_32F_32F_32F, 4,  128, 16,  32,32,32,32,  1 ),
   TextureFormatInfo(TEX_FMT_24_8           , 4,   32,  4,  24, 8, 0, 0,  1 ),
   TextureFormatInfo(TEX_FMT_8_24           , 4,   32,  4,   8,24, 0, 0,  1 )
};

UNNAMESPACE_END

//------------------------------------------------------------------------------
//! Retrieves information about a specified format in O(1) (actually, a simple table access)
const TextureFormatInfo&  Gfx::ask( const TextureFormat format )
{
   return  sTextureFormatOracle[format];
}


//------------------------------------------------------------------------------
//!
Texture&
Texture::set1D
( uint width )
{
   _width = width;
   _height = 0;
   _depth = 0;
   _type = TEX_TYPE_1D;
   return *this;
}

//------------------------------------------------------------------------------
//!
Texture&
Texture::set1DArray
( uint width, uint depth )
{
   _width = width;
   _height = 0;
   _depth = depth;
   _type = TEX_TYPE_1D_ARRAY;
   return *this;
}



//------------------------------------------------------------------------------
//!
Texture&
Texture::set2D
( uint width, uint height )
{
   _width = width;
   _height = height;
   _depth = 0;
   _type = TEX_TYPE_2D;
   return *this;
}


//------------------------------------------------------------------------------
//!
Texture&
Texture::set2DArray
( uint width, uint height, uint depth )
{
   _width = width;
   _height = height;
   _depth = depth;
   _type = TEX_TYPE_2D_ARRAY;
   return *this;
}



//------------------------------------------------------------------------------
//!
Texture&
Texture::set3D
( uint width, uint height, uint depth )
{
   _width = width;
   _height = height;
   _depth = depth;
   _type = TEX_TYPE_3D;
   return *this;
}



//------------------------------------------------------------------------------
//!
Texture&
Texture::setCubemap
( uint edgeLength )
{
   _width = edgeLength;
   _height = edgeLength;
   _depth = 6;
   _type = TEX_TYPE_CUBEMAP;
   return *this;
}


//------------------------------------------------------------------------------
//!
uint
Texture::getNumLevelsFromSize
( uint w, uint h, uint d )
{
   if( w == 0 ) return 0;
   uint max = (w > h) ? w : h;
   if( d > max ) max = d;
   uint numLevels = 1;
   while( max > 1 )
   {
      ++numLevels;
      max = sizeAtLevel(max, 1);
   }
   return numLevels;
}

//------------------------------------------------------------------------------
//!
uint
Texture::sizeAtLevel
( const uint baseSize, const uint level )
{
   uint sizeAtLevel = baseSize >> level;
   if( sizeAtLevel > 0 )
   {
      return sizeAtLevel;
   }
   else
   {
      return (baseSize > 0)?(1):(0);
   }
}


//------------------------------------------------------------------------------
//! Reorders the channel size and start values to match the following indices:
//!   dst*[0] = Red channel (or Z, or Luminance)
//!   dst*[1] = Green channel (or Stencil)
//!   dst*[2] = Blue channel
//!   dst*[3] = Alpha channel
bool
reorderChannels
( const TextureChannels tc,
  const uint* srcSize, const uint* srcStart,
  uint* dstSize, uint* dstStart )
{
   switch(tc)
   {
   case TEX_CHANS_UNSPECIFIED:
      return false;
   case TEX_CHANS_R:
      dstSize[0] = srcSize[0];  //X
      dstSize[1] = 0;
      dstSize[2] = 0;
      dstSize[3] = 0;
      dstStart[0] = srcStart[0];  //X
      dstStart[1] = 0;
      dstStart[2] = 0;
      dstStart[3] = 0;
      break;
   case TEX_CHANS_G:
      dstSize[0] = 0;
      dstSize[1] = srcSize[0];  //X
      dstSize[2] = 0;
      dstSize[3] = 0;
      dstStart[0] = 0;
      dstStart[1] = srcStart[0];  //X
      dstStart[2] = 0;
      dstStart[3] = 0;
      break;
   case TEX_CHANS_B:
      dstSize[0] = 0;
      dstSize[1] = 0;
      dstSize[2] = srcSize[0];  //X
      dstSize[3] = 0;
      dstStart[0] = 0;
      dstStart[1] = 0;
      dstStart[2] = srcStart[0];  //X
      dstStart[3] = 0;
      break;
   case TEX_CHANS_A:
      dstSize[0] = 0;
      dstSize[1] = 0;
      dstSize[2] = 0;
      dstSize[3] = srcSize[0];  //X
      dstStart[0] = 0;
      dstStart[1] = 0;
      dstStart[2] = 0;
      dstStart[3] = srcStart[0];  //X
      break;
   case TEX_CHANS_L:
      dstSize[0] = srcSize[0];  //X
      dstSize[1] = 0;
      dstSize[2] = 0;
      dstSize[3] = 0;
      dstStart[0] = srcStart[0];  //X
      dstStart[1] = 0;
      dstStart[2] = 0;
      dstStart[3] = 0;
      break;
   case TEX_CHANS_Z:
      dstSize[0] = srcSize[0];  //X
      dstSize[1] = 0;
      dstSize[2] = 0;
      dstSize[3] = 0;
      dstStart[0] = srcStart[0];  //X
      dstStart[1] = 0;
      dstStart[2] = 0;
      dstStart[3] = 0;
      break;
   case TEX_CHANS_S:
      dstSize[0] = srcSize[0];  //X
      dstSize[1] = 0;
      dstSize[2] = 0;
      dstSize[3] = 0;
      dstStart[0] = srcStart[0];  //X
      dstStart[1] = 0;
      dstStart[2] = 0;
      dstStart[3] = 0;
      break;
   case TEX_CHANS_RG:
      dstSize[0] = srcSize[0];  //X
      dstSize[1] = srcSize[1];  //Y
      dstSize[2] = 0;
      dstSize[3] = 0;
      dstStart[0] = srcStart[0];  //X
      dstStart[1] = srcStart[1];  //Y
      dstStart[2] = 0;
      dstStart[3] = 0;
      break;
   case TEX_CHANS_GR:
      dstSize[0] = srcSize[1];  //Y
      dstSize[1] = srcSize[0];  //X
      dstSize[2] = 0;
      dstSize[3] = 0;
      dstStart[0] = srcStart[1];  //Y
      dstStart[1] = srcStart[0];  //X
      dstStart[2] = 0;
      dstStart[3] = 0;
      break;
   case TEX_CHANS_LA:
      dstSize[0] = srcSize[0];  //X
      dstSize[1] = 0;
      dstSize[2] = 0;
      dstSize[3] = srcSize[1];  //Y
      dstStart[0] = srcStart[0];  //X
      dstStart[1] = 0;
      dstStart[2] = 0;
      dstStart[3] = srcStart[1];  //Y
      break;
   case TEX_CHANS_AL:
      dstSize[0] = srcSize[1];  //Y
      dstSize[1] = 0;
      dstSize[2] = 0;
      dstSize[3] = srcSize[0];  //X
      dstStart[0] = srcStart[1];  //Y
      dstStart[1] = 0;
      dstStart[2] = 0;
      dstStart[3] = srcStart[0];  //X
      break;
   case TEX_CHANS_ZS:
      dstSize[0] = srcSize[0];  //X
      dstSize[1] = srcSize[1];  //Y
      dstSize[2] = 0;
      dstSize[3] = 0;
      dstStart[0] = srcStart[0];  //X
      dstStart[1] = srcStart[1];  //Y
      dstStart[2] = 0;
      dstStart[3] = 0;
      break;
   case TEX_CHANS_SZ:
      dstSize[0] = srcSize[1];  //Y
      dstSize[1] = srcSize[0];  //X
      dstSize[2] = 0;
      dstSize[3] = 0;
      dstStart[0] = srcStart[1];  //Y
      dstStart[1] = srcStart[0];  //X
      dstStart[2] = 0;
      dstStart[3] = 0;
      break;
   case TEX_CHANS_RGB:
      dstSize[0] = srcSize[0];  //X
      dstSize[1] = srcSize[1];  //Y
      dstSize[2] = srcSize[2];  //Z
      dstSize[3] = 0;
      dstStart[0] = srcStart[0];  //X
      dstStart[1] = srcStart[1];  //Y
      dstStart[2] = srcStart[2];  //Z
      dstStart[3] = 0;
      break;
   case TEX_CHANS_BGR:
      dstSize[0] = srcSize[2];  //Z
      dstSize[1] = srcSize[1];  //Y
      dstSize[2] = srcSize[0];  //X
      dstSize[3] = 0;
      dstStart[0] = srcStart[2];  //Z
      dstStart[1] = srcStart[1];  //Y
      dstStart[2] = srcStart[0];  //X
      dstStart[3] = 0;
      break;
   case TEX_CHANS_RGBA:
      dstSize[0] = srcSize[0];  //X
      dstSize[1] = srcSize[1];  //Y
      dstSize[2] = srcSize[2];  //Z
      dstSize[3] = srcSize[3];  //W
      dstStart[0] = srcStart[0];  //X
      dstStart[1] = srcStart[1];  //Y
      dstStart[2] = srcStart[2];  //Z
      dstStart[3] = srcStart[3];  //W
      break;
   case TEX_CHANS_BGRA:
      dstSize[0] = srcSize[2];  //Z
      dstSize[1] = srcSize[1];  //Y
      dstSize[2] = srcSize[0];  //X
      dstSize[3] = srcSize[3];  //W
      dstStart[0] = srcStart[2];  //Z
      dstStart[1] = srcStart[1];  //Y
      dstStart[2] = srcStart[0];  //X
      dstStart[3] = srcStart[3];  //W
      break;
   case TEX_CHANS_ARGB:
      dstSize[0] = srcSize[1];  //Y
      dstSize[1] = srcSize[2];  //Z
      dstSize[2] = srcSize[3];  //W
      dstSize[3] = srcSize[0];  //X
      dstStart[0] = srcStart[1];  //Y
      dstStart[1] = srcStart[2];  //Z
      dstStart[2] = srcStart[3];  //W
      dstStart[3] = srcStart[0];  //X
      break;
   case TEX_CHANS_ABGR:
      dstSize[0] = srcSize[3];  //W
      dstSize[1] = srcSize[2];  //Z
      dstSize[2] = srcSize[1];  //Y
      dstSize[3] = srcSize[0];  //X
      dstStart[0] = srcStart[3];  //W
      dstStart[1] = srcStart[2];  //Z
      dstStart[2] = srcStart[1];  //Y
      dstStart[3] = srcStart[0];  //X
      break;
   default:
      return false;
   }

   return true;
}

//------------------------------------------------------------------------------
//! Translates all of the texels from one format to another
bool
Gfx::convert
( const uint numTexels,
  const TextureFormat srcFmt, const TextureChannels srcChans, const uchar* srcData,
  const TextureFormat dstFmt, const TextureChannels dstChans, uchar* dstData )
{
   if( srcFmt == dstFmt && srcChans == dstChans )
   {
      memcpy(dstData, srcData, numTexels * ask(srcFmt).sizeInBytes * sizeof(uchar));
      return true;
   }

   //Note: this is not the most efficient code, but it is fast enough for now, and clear to understand.
   //Besides, there are very few branches in the code
   const TextureFormatInfo& srcFmtInfo = ask(srcFmt);
   const TextureFormatInfo& dstFmtInfo = ask(dstFmt);

   //Source and destination size and start, all reordered as 0=R, 1=G, 2=B, 3=A
   uint srcSize[4];
   uint srcStart[4];
   if( !reorderChannels(srcChans,
                        srcFmtInfo.channels.array.size, srcFmtInfo.channels.array.start,
                        srcSize, srcStart) )
   {
      return false;
   }
   uint dstSize[4];
   uint dstStart[4];
   if( !reorderChannels(dstChans,
                        dstFmtInfo.channels.array.size, dstFmtInfo.channels.array.start,
                        dstSize, dstStart) )
   {
      return false;
   }

   uint chanBits[4];
   for( uint tidx = 0; tidx < numTexels; ++tidx )
   {
      //Extract every channel from source
      chanBits[0] = getbits(srcData, srcStart[0], srcSize[0]);
      chanBits[1] = getbits(srcData, srcStart[1], srcSize[1]);
      chanBits[2] = getbits(srcData, srcStart[2], srcSize[2]);
      chanBits[3] = getbits(srcData, srcStart[3], srcSize[3]);

      //Insert every channels into destination
      setbits( dstData, dstStart[0], dstSize[0], chanBits[0] );
      setbits( dstData, dstStart[1], dstSize[1], chanBits[1] );
      setbits( dstData, dstStart[2], dstSize[2], chanBits[2] );
      setbits( dstData, dstStart[3], dstSize[3], chanBits[3] );

      //Moves pointers to next texels
      srcData += srcFmtInfo.sizeInBytes;
      dstData += dstFmtInfo.sizeInBytes;
   }

   return true;
}


#define ENUM_TO_STRING(e) \
   case e:  return #e; break

//------------------------------------------------------------------------------
//!
const char*  Gfx::toString( const TextureFormat tfmt )
{
   switch(tfmt)
   {
      ENUM_TO_STRING(TEX_FMT_INVALID);
      ENUM_TO_STRING(TEX_FMT_8);
      ENUM_TO_STRING(TEX_FMT_8_8);
      //ENUM_TO_STRING(TEX_FMT_8_8_8);     //Probably not supported
      ENUM_TO_STRING(TEX_FMT_8_8_8_8);
      ENUM_TO_STRING(TEX_FMT_16);
      ENUM_TO_STRING(TEX_FMT_16_16);
      //ENUM_TO_STRING(TEX_FMT_16_16_16);  //Probably not supported
      ENUM_TO_STRING(TEX_FMT_16_16_16_16);
      ENUM_TO_STRING(TEX_FMT_16F);
      ENUM_TO_STRING(TEX_FMT_16F_16F);
      //ENUM_TO_STRING(TEX_FMT_16F_16F_16F);  //Probably not supported
      ENUM_TO_STRING(TEX_FMT_16F_16F_16F_16F);
      ENUM_TO_STRING(TEX_FMT_32);
      ENUM_TO_STRING(TEX_FMT_32_32);
      //ENUM_TO_STRING(TEX_FMT_32_32_32);
      ENUM_TO_STRING(TEX_FMT_32_32_32_32);
      ENUM_TO_STRING(TEX_FMT_32F);
      ENUM_TO_STRING(TEX_FMT_32F_32F);
      //ENUM_TO_STRING(TEX_FMT_32F_32F_32F);
      ENUM_TO_STRING(TEX_FMT_32F_32F_32F_32F);
      ENUM_TO_STRING(TEX_FMT_24_8);
      ENUM_TO_STRING(TEX_FMT_8_24);
      default:  return "<UNKNWON>";
   }
}

//------------------------------------------------------------------------------
//!
const char*  Gfx::toString( const TextureChannels tChans )
{
   switch(tChans)
   {
      ENUM_TO_STRING(TEX_CHANS_UNSPECIFIED);
      ENUM_TO_STRING(TEX_CHANS_R);
      ENUM_TO_STRING(TEX_CHANS_G);
      ENUM_TO_STRING(TEX_CHANS_B);
      ENUM_TO_STRING(TEX_CHANS_A);
      ENUM_TO_STRING(TEX_CHANS_L);
      ENUM_TO_STRING(TEX_CHANS_Z);
      ENUM_TO_STRING(TEX_CHANS_S);
      ENUM_TO_STRING(TEX_CHANS_RG);
      ENUM_TO_STRING(TEX_CHANS_GR);
      ENUM_TO_STRING(TEX_CHANS_LA);
      ENUM_TO_STRING(TEX_CHANS_AL);
      ENUM_TO_STRING(TEX_CHANS_ZS);
      ENUM_TO_STRING(TEX_CHANS_SZ);
      ENUM_TO_STRING(TEX_CHANS_RGB);
      ENUM_TO_STRING(TEX_CHANS_BGR);
      ENUM_TO_STRING(TEX_CHANS_RGBA);
      ENUM_TO_STRING(TEX_CHANS_BGRA);
      ENUM_TO_STRING(TEX_CHANS_ARGB);
      ENUM_TO_STRING(TEX_CHANS_ABGR);
      default:  return "<UNKNWON>";
   }
}

//------------------------------------------------------------------------------
//!
const char*  Gfx::toString( const TextureType tType )
{
   switch(tType)
   {
      ENUM_TO_STRING(TEX_TYPE_UNSPECIFIED);
      ENUM_TO_STRING(TEX_TYPE_1D);
      ENUM_TO_STRING(TEX_TYPE_1D_ARRAY);
      ENUM_TO_STRING(TEX_TYPE_2D);
      ENUM_TO_STRING(TEX_TYPE_2D_ARRAY);
      ENUM_TO_STRING(TEX_TYPE_3D);
      ENUM_TO_STRING(TEX_TYPE_CUBEMAP);
      default:  return "<UNKNWON>";
   }
}

//------------------------------------------------------------------------------
//!
const char*  Gfx::toString( const TextureFlags tFlags )
{
   switch(tFlags)
   {
      ENUM_TO_STRING(TEX_FLAGS_NONE);
      ENUM_TO_STRING(TEX_FLAGS_MIPMAPPED);
      ENUM_TO_STRING(TEX_FLAGS_RENDERABLE);
      default:  return "<UNKNWON>";
   }
}
