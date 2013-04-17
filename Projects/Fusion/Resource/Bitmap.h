/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_BITMAP_H
#define FUSION_BITMAP_H

#include <Fusion/StdDefs.h>

#include <CGMath/Vec2.h>
#include <CGMath/Vec4.h>

#include <Base/IO/TextStream.h>
#include <Base/Util/RCP.h>
#include <Base/Util/RCObject.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Bitmap
==============================================================================*/

//! Contains a bitmap definition (size, pixel format, ... ) and its buffer.

class Bitmap
   : public RCObject
{

public:

   /*----- types and enumerations ----*/

   enum DimType {
      DIM_2D,
      DIM_CUBEMAP,
      DIM_2D_ARRAY,
      DIM_3D,
   };

   enum PixelType {
      BYTE,
      FLOAT
   };

   /*----- static methods -----*/

   FUSION_DLL_API static void printInfo( TextStream& os );

   /*----- methods -----*/

   FUSION_DLL_API Bitmap();
   FUSION_DLL_API Bitmap( const Vec2i& dim,                      const PixelType type, const int numChannels );
   FUSION_DLL_API Bitmap( const int    dim,                      const PixelType type, const int numChannels );
   FUSION_DLL_API Bitmap( const Vec2i& dim, const int numSlices, const PixelType type, const int numChannels );
   FUSION_DLL_API Bitmap( const Vec3i& dim,                      const PixelType type, const int numChannels );
   FUSION_DLL_API Bitmap( const Bitmap& src,                     const PixelType type, const int numChannels );

   FUSION_DLL_API RCP<Bitmap> clone() const;

   FUSION_DLL_API void init(
      const Vec2i&    dim,
      const PixelType type,
      const int       numChannels
   );
   FUSION_DLL_API void init(
      const int       dim,
      const PixelType type,
      const int       numChannels
   );
   FUSION_DLL_API void init(
      const Vec2i&    dim,
      const int       numSlices,
      const PixelType type,
      const int       numChannels
   );
   FUSION_DLL_API void init(
      const Vec3i&    dim,
      const PixelType type,
      const int       numChannels
   );
   FUSION_DLL_API void init(
      const Bitmap&   src,
      const PixelType type,
      const int       numChannels
   );
   FUSION_DLL_API void init(
      const Vec2i&    dim,
      const int       numSlices,
      const bool      cubemap,
      const PixelType type,
      const int       numChannels
   );

   inline const Vec3i& dimension() const;
   inline int  width() const;
   inline int  height() const;
   inline int  depth() const;
   inline DimType   dimType()   const;
   inline PixelType pixelType() const;

   inline size_t size() const;
   inline size_t pixelSize() const;
   inline size_t lineSize() const;
   inline size_t sliceSize() const;
   inline size_t numPixels() const;
   inline size_t numPixelsPerSlice() const;
   inline int    numChannels() const;

   inline const uchar* pixels() const;
   inline const uchar* pixels( int s ) const;
   inline const uchar* pixel( size_t idx ) const;
   inline const uchar* pixel( const Vec2i& pos ) const;
   inline const uchar* pixel( const Vec2i& pos, const int slice ) const;
   inline const uchar* pixel( const Vec3i& pos ) const;
   inline const uchar* pixelRow( int y ) const;
   inline const uchar* pixelRow( int s, int y ) const;
   inline uchar* pixels();
   inline uchar* pixels( int s );
   inline uchar* pixel( size_t idx );
   inline uchar* pixel( const Vec2i& pos );
   inline uchar* pixel( const Vec2i& pos, const int slice );
   inline uchar* pixel( const Vec3i& pos );
   inline uchar* pixelRow( int y );
   inline uchar* pixelRow( int s, int y );

   FUSION_DLL_API void clearBuffer( void* data = nullptr );

   FUSION_DLL_API bool load( const String& path );
   FUSION_DLL_API bool loadCubemap( const String& nxpath );
   FUSION_DLL_API bool save( const String& path ) const;
   FUSION_DLL_API bool save( const String& path, int slice ) const;

   inline Vec4f  getColor( const Vec2i& pos ) const;
   inline Vec4f  getColor( const Vec2i& pos, const int slice ) const;
   inline Vec4f  getColor( const Vec3i& pos ) const;
   FUSION_DLL_API Vec4f  getColor( const uchar* data ) const;

   FUSION_DLL_API void  print( TextStream& os = StdErr ) const;

protected:

   /*----- methods -----*/

   virtual ~Bitmap();

private:

   /*----- data types -----*/
   typedef Vec2<size_t>  Vec2t;
   typedef Vec3<size_t>  Vec3t;

   /*----- data members -----*/

   Vec3i     _dim;
   DimType   _dimType;
   PixelType _pixType;
   Vec3t     _strides;  // x: pixelSize, y: lineSize, z: sliceSize.
   int       _numChannels;
   uchar*    _pixels;
};

//------------------------------------------------------------------------------
//!
inline const char*  toStr( Bitmap::DimType v )
{
   switch( v )
   {
      case Bitmap::DIM_2D       : return "2D";
      case Bitmap::DIM_CUBEMAP  : return "Cubemap";
      case Bitmap::DIM_2D_ARRAY : return "2Darray";
      case Bitmap::DIM_3D       : return "3D";
      default                   : return "<unknown>";
   }
}

//------------------------------------------------------------------------------
//!
inline const char*  toStr( Bitmap::PixelType v )
{
   switch( v )
   {
      case Bitmap::BYTE : return "Byte";
      case Bitmap::FLOAT: return "Float";
      default           : return "<unknown>";
   }
}

//------------------------------------------------------------------------------
//!
inline const Vec3i&
Bitmap::dimension() const
{
   return _dim;
}

//------------------------------------------------------------------------------
//!
inline int
Bitmap::width() const
{
   return _dim.x;
}

//------------------------------------------------------------------------------
//!
inline int
Bitmap::height() const
{
   return _dim.y;
}

//------------------------------------------------------------------------------
//!
inline int
Bitmap::depth() const
{
   return _dim.z;
}

//------------------------------------------------------------------------------
//!
inline Bitmap::DimType
Bitmap::dimType() const
{
   return _dimType;
}

//------------------------------------------------------------------------------
//!
inline Bitmap::PixelType
Bitmap::pixelType() const
{
   return _pixType;
}

//------------------------------------------------------------------------------
//!
inline size_t
Bitmap::size() const
{
   return _dim.z * _strides.z;
}

//------------------------------------------------------------------------------
//!
inline size_t
Bitmap::pixelSize() const
{
   return _strides.x;
}

//------------------------------------------------------------------------------
//!
inline size_t
Bitmap::lineSize() const
{
   return _strides.y;
}

//------------------------------------------------------------------------------
//!
inline size_t
Bitmap::sliceSize() const
{
   return _strides.z;
}

//------------------------------------------------------------------------------
//!
inline size_t
Bitmap::numPixels() const
{
   return _dim.x*_dim.y*_dim.z;
}

//------------------------------------------------------------------------------
//!
inline size_t
Bitmap::numPixelsPerSlice() const
{
   return _dim.x*_dim.y;
}

//------------------------------------------------------------------------------
//!
inline int
Bitmap::numChannels() const
{
   return _numChannels;
}

//------------------------------------------------------------------------------
//!
inline const uchar*
Bitmap::pixels() const
{
   return _pixels;
}

//------------------------------------------------------------------------------
//! Returns the first pixel of a slice.
inline const uchar*
Bitmap::pixels( int s ) const
{
   return &_pixels[s*sliceSize()];
}

//------------------------------------------------------------------------------
//!
inline const uchar*
Bitmap::pixel( size_t idx ) const
{
   return &_pixels[ idx*_strides.x ];
}

//------------------------------------------------------------------------------
//!
inline const uchar*
Bitmap::pixel( const Vec2i& pos ) const
{
   return &_pixels[ dot( _strides(0,1), Vec2t(pos) ) ];
}

//------------------------------------------------------------------------------
//!
inline const uchar*
Bitmap::pixel( const Vec2i& pos, const int slice ) const
{
   return pixel( Vec3i( pos, slice ) );
}

//------------------------------------------------------------------------------
//!
inline const uchar*
Bitmap::pixel( const Vec3i& pos ) const
{
   return &_pixels[ dot( _strides, Vec3t(pos) ) ];
}

//------------------------------------------------------------------------------
//!
inline const uchar*
Bitmap::pixelRow( int y ) const
{
   return &_pixels[y*lineSize()];
}

//------------------------------------------------------------------------------
//!
inline const uchar*
Bitmap::pixelRow( int s, int y ) const
{
   return &_pixels[s*sliceSize() + y*lineSize()];
}

//------------------------------------------------------------------------------
//!
inline uchar*
Bitmap::pixels()
{
   return _pixels;
}

//------------------------------------------------------------------------------
//! Returns the first pixel of a slice.
inline uchar*
Bitmap::pixels( int s )
{
   return &_pixels[s*sliceSize()];
}

//------------------------------------------------------------------------------
//!
inline uchar*
Bitmap::pixel( size_t idx )
{
   return &_pixels[ idx*_strides.x ];
}

//------------------------------------------------------------------------------
//!
inline uchar*
Bitmap::pixel( const Vec2i& pos )
{
   return &_pixels[ dot( _strides(0,1), Vec2t(pos) ) ];
}

//------------------------------------------------------------------------------
//!
inline uchar*
Bitmap::pixel( const Vec2i& pos, const int slice )
{
   return pixel( Vec3i( pos, slice ) );
}

//------------------------------------------------------------------------------
//!
inline uchar*
Bitmap::pixel( const Vec3i& pos )
{
   return &_pixels[ dot( _strides, Vec3t(pos) ) ];
}

//------------------------------------------------------------------------------
//!
inline uchar*
Bitmap::pixelRow( int y )
{
   return &_pixels[y*lineSize()];
}

//------------------------------------------------------------------------------
//!
inline uchar*
Bitmap::pixelRow( int s, int y )
{
   return &_pixels[s*sliceSize() + y*lineSize()];
}

//------------------------------------------------------------------------------
//!
inline Vec4f
Bitmap::getColor( const Vec2i& pos ) const
{
   return getColor( pixel(pos) );
}

//------------------------------------------------------------------------------
//!
inline Vec4f
Bitmap::getColor( const Vec2i& pos, const int slice ) const
{
   return getColor( pixel(pos, slice) );
}

//------------------------------------------------------------------------------
//!
inline Vec4f
Bitmap::getColor( const Vec3i& pos ) const
{
   return getColor( pixel(pos) );
}

//------------------------------------------------------------------------------
//!
inline TextStream&
operator<<( TextStream& os, const Bitmap& bmp )
{
   return os << "bmp["
             << toStr(bmp.dimType())
             << " "
             << bmp.dimension()
             << " "
             << bmp.numChannels() << " x " << toStr(bmp.pixelType()) << " = " << bmp.size() << " B"
             << "]";
}

NAMESPACE_END

#endif
