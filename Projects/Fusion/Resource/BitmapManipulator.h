/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_BITMAP_MANIPULATOR_H
#define FUSION_BITMAP_MANIPULATOR_H

#include <Fusion/StdDefs.h>

#include <Fusion/Resource/Bitmap.h>

#include <CGMath/Mat4.h>
#include <CGMath/Vec2.h>
#include <CGMath/Vec3.h>
#include <CGMath/Vec4.h>

#include <Base/Dbg/Defs.h>
#include <Base/Msg/Delegate.h>

#include <climits>

NAMESPACE_BEGIN

namespace BitmapManipulator
{

   //============
   // Generators
   //============
   //!< Routines creating Bitmap out of a few parameters.

   // BRDF bitmap.
   FUSION_DLL_API RCP<Bitmap> toDBRDF( uint size, float (*dbrdf)( float ) );
   FUSION_DLL_API RCP<Bitmap> toDBRDF( uint size, const String& name );
   FUSION_DLL_API RCP<Bitmap> toDBRDF(
      uint          size,
      const String& angles,
      const String& brdfs
   );


   //==============
   // Manipulators
   //==============
   //!< Routines taking a Bitmap as input and returning a manipulated result.

   // Channel manipulation.
   FUSION_DLL_API RCP<Bitmap>  addAlpha( const Bitmap& src, float value = 1.0f );
   FUSION_DLL_API RCP<Bitmap>  convert( const Bitmap& src, const Bitmap::PixelType& type );
   FUSION_DLL_API RCP<Bitmap>  luminanceToRGB( const Bitmap& src );
   FUSION_DLL_API RCP<Bitmap>  mulAdd( const Bitmap& src, const Vec4f& mul, const Vec4f& add );

   // Pixel manipulation.
   FUSION_DLL_API RCP<Bitmap>  crop( const Bitmap& src, const Vec2i& pos, const Vec2i& size );
   FUSION_DLL_API RCP<Bitmap>  extractSlice( const Bitmap& src, int slice, bool flip = false );
   FUSION_DLL_API RCP<Bitmap>  flipVertical( const Bitmap& src );

   // Filters and operators.
   FUSION_DLL_API RCP<Bitmap>  downsample( const Bitmap& src );
   FUSION_DLL_API RCP<Bitmap>  edgeDetectFreiChen( const Bitmap& src, bool skipEdges = true );
   //FUSION_DLL_API RCP<Bitmap>  edgeDetectSobel( const Bitmap& src );
   FUSION_DLL_API RCP<Bitmap>  grayscaleToDistanceField( const Bitmap& src, int maxDist = INT_MAX );
   FUSION_DLL_API RCP<Bitmap>  grayscaleToDistanceField( const Bitmap& src, const Vec2i& pos, const Vec2i& size, int maxDist = INT_MAX );
   FUSION_DLL_API RCP<Bitmap>  grayscaleToDistanceFieldFullSearch( const Bitmap& src, const Vec2i& pos, const Vec2i& size, int maxDist = INT_MAX );
   FUSION_DLL_API RCP<Bitmap>  grayscaleToDistanceFieldSpans( const Bitmap& src, const Vec2i& pos, const Vec2i& size, int maxDist = INT_MAX );


   //===========
   // Modifiers
   //===========
   //!< Routines modifying Bitmap data directly (i.e. in-place operations).

   inline void  apply( Bitmap& bmp, const Delegate1<uchar*>& );
   inline void  apply( const Bitmap& src, Bitmap& dst, const Delegate2<const uchar*, uchar*>& func );

   // Copying.
   FUSION_DLL_API bool  copy( const Bitmap& src, Bitmap& dst, const Vec2i& pos );

   FUSION_DLL_API bool  copyRow( const Bitmap& srcBmp, const Vec2i& srcPos, const int srcLength,
                                       Bitmap& dstBmp, const Vec2i& dstPos, const int dstLength );

   FUSION_DLL_API bool  copyRowToCol( const Bitmap& srcBmp, const Vec2i& srcPos, const int srcLength,
                                            Bitmap& dstBmp, const Vec2i& dstPos, const int dstHeight );

   FUSION_DLL_API bool  copyCol( const Bitmap& srcBmp, const Vec2i& srcPos, const int srcHeight,
                                       Bitmap& dstBmp, const Vec2i& dstPos, const int dstHeight );

   FUSION_DLL_API bool  copyColToRow( const Bitmap& srcBmp, const Vec2i& srcPos, const int srcHeight,
                                            Bitmap& dstBmp, const Vec2i& dstPos, const int dstLength );

   // Drawing.
   FUSION_DLL_API void  fillRect( Bitmap& dst, const Vec2i& pos, const Vec2i& size, const void* data );
   FUSION_DLL_API void  fillCircle( Bitmap& dst, const Vec2f& pos, float radius, const void* data );

   // Alpha.
   FUSION_DLL_API bool  scaleColorByAlpha( Bitmap& bmp );
   FUSION_DLL_API bool  scaleColorByAlpha( Bitmap& bmp, size_t firstPixelIdx, size_t numPixels );
   FUSION_DLL_API bool  unscaleColorByAlpha( Bitmap& bmp );
   FUSION_DLL_API bool  unscaleColorByAlpha( Bitmap& bmp, size_t firstPixelIdx, size_t numPixels );

   // Transform.
   FUSION_DLL_API void  transform( Bitmap& dst, const Vec2i& pos, const Vec2i& size, const Mat4f& mat, const Vec4f& off );

   //===========
   // Utilities
   //===========

   // Filtering routines.
   FUSION_DLL_API void  linearL8( const Bitmap& bmp, const Vec2f& uv, const int axis, uchar* dst );
   FUSION_DLL_API void  linearLA8( const Bitmap& bmp, const Vec2f& uv, const int axis, uchar* dst );
   FUSION_DLL_API void  linearRGB8( const Bitmap& bmp, const Vec2f& uv, const int axis, uchar* dst );
   FUSION_DLL_API void  linearRGBA8( const Bitmap& bmp, const Vec2f& uv, const int axis, uchar* dst );
   FUSION_DLL_API void  linearL32f( const Bitmap& bmp, const Vec2f& uv, const int axis, uchar* dst );
   FUSION_DLL_API void  linearLA32f( const Bitmap& bmp, const Vec2f& uv, const int axis, uchar* dst );
   FUSION_DLL_API void  linearRGB32f( const Bitmap& bmp, const Vec2f& uv, const int axis, uchar* dst );
   FUSION_DLL_API void  linearRGBA32f( const Bitmap& bmp, const Vec2f& uv, const int axis, uchar* dst );
   FUSION_DLL_API void  linearH( const Bitmap& bmp, const float x, const uint y, uchar* dst );
   FUSION_DLL_API void  linearV( const Bitmap& bmp, const uint x, const float y, uchar* dst );

   FUSION_DLL_API Vec4f  nearest( const Bitmap& bmp, const Vec2f& st );
   FUSION_DLL_API Vec4f  bilinear( const Bitmap& bmp, const Vec2f& st );
   //FUSION_DLL_API Vec4f  trilinear( const Bitmap& bmp, const Vec2f& st, const float lod );

   //!< Various utility routines that can work on arbitrary data buffers.
   // Conversion routines.
   inline void  convert_1x32f_1x8( const void* src, void* dst );
   inline void  convert_2x32f_2x8( const void* src, void* dst );
   inline void  convert_3x32f_3x8( const void* src, void* dst );
   inline void  convert_4x32f_4x8( const void* src, void* dst );

   inline void  convert_1x8_1x32f( const void* src, void* dst );
   inline void  convert_2x8_2x32f( const void* src, void* dst );
   inline void  convert_3x8_3x32f( const void* src, void* dst );
   inline void  convert_4x8_4x32f( const void* src, void* dst );

   // Distance field routines.
   FUSION_DLL_API float  distance( const Vec2i& srcIdx, float srcVal, const Vec2i& dstIdx, float dstVal, const Vec2f& dstGrd );

   // Others.
   inline bool  isGray( float v );

}

//-----------------------------------------------------------------------------
//! Calls the specified transformation function func on every pixel.
inline void
BitmapManipulator::apply( Bitmap& dst, const Delegate1<uchar*>& func )
{
   uchar* bgn = dst.pixels();
   uchar* end = dst.pixels() + dst.size();
   size_t pix = dst.pixelSize();
   for( uchar* cur = bgn; cur != end; cur += pix )
   {
      func( cur );
   }
}

//-----------------------------------------------------------------------------
//! Calls the specified transformation function func on every pixel.
inline void
BitmapManipulator::apply( const Bitmap& src, Bitmap& dst, const Delegate2<const uchar*, uchar*>& fgFunc )
{
   CHECK( src.dimension() == dst.dimension() );
   size_t n = src.numPixels();
   const uchar* srcP = src.pixels();
         uchar* dstP = dst.pixels();
   size_t srcS = src.pixelSize();
   size_t dstS = dst.pixelSize();
   for( size_t p = 0; p < n; ++p, srcP += srcS, dstP += dstS )
   {
      fgFunc( srcP, dstP );
   }
}

//------------------------------------------------------------------------------
//! Converts a float (range [0.0f, 1.0f]) to a byte (range [0, 255]).
inline void
BitmapManipulator::convert_1x32f_1x8( const void* src, void* dst )
{
   float*   src32f = (float*)src;
   uint8_t* dst8   = (uint8_t*)dst;
   float tmp = src32f[0];
   tmp = CGM::clamp( tmp, 0.0f, 1.0f );
   tmp *= 255.0f;
   tmp += 0.5f;
   dst8[0] = (uint8_t)tmp;
}

//------------------------------------------------------------------------------
//! Converts 2 floats (range [0.0f, 1.0f]) to 2 bytes (range [0, 255]).
inline void
BitmapManipulator::convert_2x32f_2x8( const void* src, void* dst )
{
   float*   src32f = (float*)src;
   uint8_t* dst8   = (uint8_t*)dst;
   Vec2f tmp = Vec2f( src32f[0], src32f[1] );
   tmp = CGM::clamp( tmp, 0.0f, 1.0f );
   tmp *= 255.0f;
   tmp += 0.5f;
   dst8[0] = (uint8_t)tmp.x;
   dst8[1] = (uint8_t)tmp.y;
}

//------------------------------------------------------------------------------
//! Converts 3 floats (range [0.0f, 1.0f]) to 3 bytes (range [0, 255]).
inline void
BitmapManipulator::convert_3x32f_3x8( const void* src, void* dst )
{
   float*   src32f = (float*)src;
   uint8_t* dst8   = (uint8_t*)dst;
   Vec3f tmp = Vec3f( src32f[0], src32f[1], src32f[2] );
   tmp = CGM::clamp( tmp, 0.0f, 1.0f );
   tmp *= 255.0f;
   tmp += 0.5f;
   dst8[0] = (uint8_t)tmp.x;
   dst8[1] = (uint8_t)tmp.y;
   dst8[2] = (uint8_t)tmp.z;
}

//------------------------------------------------------------------------------
//! Converts 4 floats (range [0.0f, 1.0f]) to 4 bytes (range [0, 255]).
inline void
BitmapManipulator::convert_4x32f_4x8( const void* src, void* dst )
{
   float*   src32f = (float*)src;
   uint8_t* dst8   = (uint8_t*)dst;
   Vec4f tmp = Vec4f( src32f[0], src32f[1], src32f[2], src32f[3] );
   tmp = CGM::clamp( tmp, 0.0f, 1.0f );
   tmp *= 255.0f;
   tmp += 0.5f;
   dst8[0] = (uint8_t)tmp.x;
   dst8[1] = (uint8_t)tmp.y;
   dst8[2] = (uint8_t)tmp.z;
   dst8[3] = (uint8_t)tmp.w;
}


//------------------------------------------------------------------------------
//! Converts a byte (range [0, 255]) to a float (range [0.0f, 1.0f]).
inline void
BitmapManipulator::convert_1x8_1x32f( const void* src, void* dst )
{
   uint8_t* src8 = (uint8_t*)src;
   float&   tmp  = *(float*)dst;
   tmp  = src8[0];
   tmp *= (1.0f/255.0f);
}

//------------------------------------------------------------------------------
//! Converts 2 bytes (range [0, 255]) to 2 floats (range [0.0f, 1.0f]).
inline void
BitmapManipulator::convert_2x8_2x32f( const void* src, void* dst )
{
   uint8_t* src8 = (uint8_t*)src;
   Vec2f&   tmp  = *(Vec2f*)dst;
   tmp.x = src8[0];
   tmp.y = src8[1];
   tmp *= (1.0f/255.0f);
}

//------------------------------------------------------------------------------
//! Converts 3 bytes (range [0, 255]) to 3 floats (range [0.0f, 1.0f]).
inline void
BitmapManipulator::convert_3x8_3x32f( const void* src, void* dst )
{
   uint8_t* src8 = (uint8_t*)src;
   Vec3f&   tmp  = *(Vec3f*)dst;
   tmp.x = src8[0];
   tmp.y = src8[1];
   tmp.z = src8[2];
   tmp *= (1.0f/255.0f);
}

//------------------------------------------------------------------------------
//! Converts 4 bytes (range [0, 255]) to 4 floats (range [0.0f, 1.0f]).
inline void
BitmapManipulator::convert_4x8_4x32f( const void* src, void* dst )
{
   uint8_t* src8 = (uint8_t*)src;
   Vec4f&   tmp  = *(Vec4f*)dst;
   tmp.x = src8[0];
   tmp.y = src8[1];
   tmp.z = src8[2];
   tmp.w = src8[3];
   tmp *= (1.0f/255.0f);
}

//-----------------------------------------------------------------------------
//!
inline bool
BitmapManipulator::isGray( float v )
{
   //return CGM::abs( 0.5f - v ) < 0.5;
   return (v*v) != v; // Different than 0.0f or 1.0f.
}


NAMESPACE_END

#endif //FUSION_BITMAP_MANIPULATOR_H
