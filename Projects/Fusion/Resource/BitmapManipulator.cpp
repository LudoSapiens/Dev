/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Resource/BitmapManipulator.h>

#include <CGMath/CGConst.h>
#include <CGMath/CGMath.h>
#include <CGMath/Dist.h>
#include <CGMath/Vec4.h>

#include <Base/Dbg/DebugStream.h>
#include <Base/Util/Memory.h>

#include <algorithm>


/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_bmp, "Bitmap" );

#if _MSC_VER
#pragma warning( push )
// C4521: Multiple copy constructors.
#pragma warning( disable: 4521 )
#endif

template<typename T>
class ArrayGuard
{
public:
   ArrayGuard( T* data = NULL ): _data( data ) {}
   ArrayGuard( size_t numEntries ): _data( new T[numEntries] ) {}
   ArrayGuard( ArrayGuard& ag ): _data( ag._data ) { ag._data = NULL; }
   ArrayGuard( const ArrayGuard& ag ); // Disallow those.
   ~ArrayGuard() { delete [] _data; }
   ArrayGuard&  operator=( T* data ) { _data = data; }
   operator       T*()       { return _data; }
   operator const T*() const { return _data; }
         T*  data()       { return _data; }
   const T*  data() const { return _data; }

protected:
   T*  _data;
};

template<typename T>
class ArrayGuardCheck
{
public:
   ArrayGuardCheck(): _data(NULL) {}
   ArrayGuardCheck( size_t numEntries ): _data( new T[numEntries+2] ), _n( numEntries ) { _data[0] = _data[_n+1] = T(131313); }
   ArrayGuardCheck( ArrayGuardCheck& ag ): _data( ag._data ), _n( ag._n ) { ag._data = NULL; ag._n = 0; }
   ArrayGuardCheck( const ArrayGuardCheck& ag ); // Disallow those.
   ~ArrayGuardCheck() { check(); delete [] _data; }
   ArrayGuardCheck&  operator=( T* data ) { _data = data; }
   operator       T*()       { return _data + 1; }
   operator const T*() const { return _data + 1; }
         T*  data()       { return _data + 1; }
   const T*  data() const { return _data + 1; }
   inline bool check() const
   {
      bool ok = (_data == NULL) || ((_data[0] == T(131313)) && _data[_n+1] == T(131313));
      CHECK( ok );
      return ok;
   }
protected:
   T*      _data;
   size_t  _n;
};

#if _MSC_VER
#pragma warning( pop )
#endif

//------------------------------------------------------------------------------
//!
inline void thetaHalfIdxs( float thetaHalf, int dim, int& idx0, int& idx1, float& t )
{
   if( thetaHalf <= 0.0f )
   {
      idx0 = 0;
      idx1 = 0;
      t    = 0;
      return;
   }
   //int idx = int( sqrt( (thetaHalf / CGM::Pi_2)*dim*dim ) );
   const double magic2 = 14.731765046470095;
   float fidx = (float)(log(thetaHalf / CGConstd::pi_2() * 450.0) * magic2);
   int idx    = (int)fidx;
   t          = fidx-idx;
   idx0       = idx < 0 ? 0 : idx < dim ? idx : dim-1;
   ++idx;
   idx1       = idx < 0 ? 0 : idx < dim ? idx : dim-1;
}

#if 0
//------------------------------------------------------------------------------
//!
bool
angleCmp( const Vec2f& a0, const Vec2f& a1 )
{
   return a0.y < a1.y;
}
#endif

union TexelData
{
   const uchar*  b;
   const float*  f;
};

//------------------------------------------------------------------------------
//! Retrieves the L texel of an L bitmap (stored in bytes) as a float.
//! The range of the returning channels are [0.0, 255.0].
inline float getL8( const Bitmap& bmp, const Vec2i& texelCoord )
{
   const uchar* texel = bmp.pixel(texelCoord);
   return (float)texel[0];
}

//------------------------------------------------------------------------------
//! Retrieves the LA texel of an LA bitmap (stored in bytes) as a Vec2f.
//! The range of the returning channels are [0.0, 255.0].
inline Vec2f getLA8( const Bitmap& bmp, const Vec2i& texelCoord )
{
   const uchar* texel = bmp.pixel(texelCoord);
   return Vec2f( texel[0], texel[1] );
}

//------------------------------------------------------------------------------
//! Retrieves the RGB texel of an RGB bitmap (stored in bytes)as a Vec3f.
//! The range of the returning channels are [0.0, 255.0].
inline Vec3f getRGB8( const Bitmap& bmp, const Vec2i& texelCoord )
{
   const uchar* texel = bmp.pixel(texelCoord);
   return Vec3f( texel[0], texel[1], texel[2] );
}

//------------------------------------------------------------------------------
//! Retrieves the RGBA texel of an RGBA bitmap (stored in bytes) as a Vec4f.
//! The range of the returning channels are [0.0, 255.0].
inline Vec4f getRGBA8( const Bitmap& bmp, const Vec2i& texelCoord )
{
   const uchar* texel = bmp.pixel(texelCoord);
   return Vec4f( texel[0], texel[1], texel[2], texel[3] );
}

//------------------------------------------------------------------------------
//! Retrieves the L texel of an L bitmap (stored in floats) as a float.
inline float getL32f( const Bitmap& bmp, const Vec2i& texelCoord )
{
   TexelData td;
   td.b = bmp.pixel(texelCoord);
   return td.f[0];
}

//------------------------------------------------------------------------------
//! Retrieves the LA texel of an LA bitmap (stored in floats) as a Vec2f.
inline Vec2f getLA32f( const Bitmap& bmp, const Vec2i& texelCoord )
{
   TexelData td;
   td.b = bmp.pixel(texelCoord);
   return Vec2f( td.f[0], td.f[1] );
}

//------------------------------------------------------------------------------
//! Retrieves the RGB texel of an RGB bitmap (stored in floats) as a Vec3f.
inline Vec3f getRGB32f( const Bitmap& bmp, const Vec2i& texelCoord )
{
   TexelData td;
   td.b = bmp.pixel(texelCoord);
   return Vec3f( td.f[0], td.f[1], td.f[2] );
}

//------------------------------------------------------------------------------
//! Retrieves the RGB texel of an RGBA bitmap (stored in floats) as a Vec4f.
inline Vec4f getRGBA32f( const Bitmap& bmp, const Vec2i& texelCoord )
{
   TexelData td;
   td.b = bmp.pixel(texelCoord);
   return Vec4f( td.f[0], td.f[1], td.f[2], td.f[3] );
}

//------------------------------------------------------------------------------
//!
template< typename T >
inline void  copyPixel( const void* src, void* dst )
{
   *(T*)dst = *(const T*)src;
}

inline void  normGrad( Vec2f& grad )
{
   // Normalize the gradients.
   float sqLen = CGM::sqrLength( grad );
   if( sqLen > 0.0f )
   {
      grad *= CGM::invSqrt( sqLen );
   }
}

//-----------------------------------------------------------------------------
//!
inline float  closest( float sDistA, float sDistB )
{
   return ( CGM::abs(sDistA) <= CGM::abs(sDistB) ) ? sDistA : sDistB;
}

#if 0
//-----------------------------------------------------------------------------
//!
inline String  spanToStr( int v )
{
   if( v == -INT_MAX )  return "-";
   else
   if( v ==  INT_MAX )  return "+";
   else
                        return String(v);
}

//-----------------------------------------------------------------------------
//!
inline String  spanToStr( const Vec2i& r )
{
   return spanToStr(r.x) + "," + spanToStr(r.y);
}
#endif

UNNAMESPACE_END


NAMESPACE_BEGIN

namespace BitmapManipulator
{

//============
// Generators
//============

//------------------------------------------------------------------------------
//!
RCP<Bitmap> toDBRDF( uint size, float (*dbrdf)( float ) )
{
   Vec2i dstDim( size, 1 );
   RCP<Bitmap> dstBmp = new Bitmap( dstDim, Bitmap::BYTE, 1 );

   uchar* p = dstBmp->pixels();
   for( uint x = 0; x < size; ++x, ++p )
   {
      float n_dot_h = x/float(size-1);
      float c       = dbrdf( n_dot_h ) * 255.0f + 0.5f;
      c = CGM::clamp( c, 0.0f, 255.0f );
      *p = (uchar)c;
   }

   return dstBmp;
}

//------------------------------------------------------------------------------
//!
RCP<Bitmap> toDBRDF( uint size, const String& name )
{
   StdErr << "Doing " << name << "\n";

   // Read brdf data.
   Vector<float> brdf;
   Vec3i dim;
   String ext = name.getExt();

#if 1
   StdErr << "ERROR - Need input support (and binary) for toBRDF() to work" << nl;
   return NULL;
#else
   IFStream stream;

   if( ext == "brdf" )
   {
      // Open and read the brdf dimension.
      stream.open( name.cstr(), IFStream::BIN );
      stream.read( &dim, sizeof(dim) );

      // Read the brdf data.
      uint size = dim.x*dim.y*dim.z;
      Vector<Vec4f> data(size);
      stream.read( data.data(), sizeof(Vec4f)*size );

      // Convert to monochromatic brdf.
      brdf.resize(size);
      for( uint i = 0; i < size; ++i )
      {
         brdf[i] = data[i].x*0.299f + data[i].y*0.587f + data[i].z*0.114f;
      }
   }
   else if( ext == "binary" )
   {
      // Open and read the brdf dimension.
      stream.open( name.cstr(), IFStream::BIN );
      stream.read( &dim, sizeof(dim) );

      // Read the brdf data.
      uint size = dim.x*dim.y*dim.z;
      Vector<Vec3d> data(size);
      stream.read( data.data(), sizeof(Vec3d)*size );

      // Convert to monochromatic brdf.
      brdf.resize(size);
      for( uint i = 0; i < size; ++i )
      {
         brdf[i] = float(data[i].x*0.34 + data[i].y*0.33 + data[i].z*0.33);
      }
   }
   else
   {
      return NULL;
   }
#endif

   // Compute the dbrdf.
   Vector<float> dbrdf(size, 0.0f);

   for( uint i = 0; i < size; ++i )
   {
      float n_dot_h = i/float(size-1);

      float thetaHalf = acos( n_dot_h );
      //float thetaDiff = 0.0f;
      //float phiDiff   = 0.0; // any angle.

      int pd = 0;   // phiDiff idx.
      int td = 0;   // thetaDiff idx.
      int th0, th1; // thetaHalf idxs.
      float t;
      thetaHalfIdxs( thetaHalf, dim.x, th0, th1, t );
      dbrdf[i] = (1-t)*brdf[pd + dim(2)*(td + th0*dim(1))] + t*brdf[pd + dim(2)*(td + th1*dim(1))];
      dbrdf[i] *= (2.0f*n_dot_h-n_dot_h*n_dot_h)/100.0f;
   }

   // Convert into bitmap format.
   Vec2i dstDim( size, 1 );
   RCP<Bitmap> dstBmp = new Bitmap( dstDim, Bitmap::BYTE, 1 );

   uchar* p = dstBmp->pixels();
   for( uint x = 0; x < size; ++x, ++p )
   {
      float c = dbrdf[x] * 255.0f + 0.5f;
      c = CGM::clamp( c, 0.0f, 255.0f );
      *p = (uchar)c;
   }


   return dstBmp;
}

//------------------------------------------------------------------------------
//!
RCP<Bitmap> toDBRDF( uint size, const String& angles, const String& brdfs )
{
#if 1
   StdErr << "ERROR - Need input support (and binary) for toBRDF() to work" << nl;
   unused(size);
   unused(angles);
   unused(brdfs);
   return NULL;
#else
   IFStream stream;

   // Open and read the samples angles.
   stream.open( angles.cstr(), IFStream::TEXT );

   Vector<Vec2f> cosAngles;
   Vector<Vec3f> ins;
   Vector<Vec3f> outs;
   while( !stream.eof() )
   {
      uint num;
      float thetaOut;
      float phiOut;
      float thetaIn;
      float phiIn;
      stream >> num >> thetaOut >> phiOut >> thetaIn >> phiIn;
      thetaIn  = CGConstf::pi_2()-thetaIn;
      thetaOut = CGConstf::pi_2()-thetaOut;

      // Compute in vector.
      float projIn = sin( thetaIn );
      Vec3f in( projIn*cos( phiIn ), projIn*sin( phiIn ), cos( thetaIn ) );

      // Compute out vector.
      float projOut = sin( thetaOut );
      Vec3f out( projOut*cos( phiOut ), projOut*sin( phiOut ), cos( thetaOut ) );

      Vec3f h = (in+out).getNormalized();

      ins.pushBack( Vec3f(in.y,in.z,in.x).getNormalized() );
      outs.pushBack( Vec3f(out.y,out.z,out.x).getNormalized() );

      cosAngles.pushBack( Vec2f((float)(num-1), h.z) );

      StdErr << num << " " << ins.back() << " " << outs.back() << "\n";
   }
   stream.close();

   // Sort angles.
   std::sort( cosAngles.begin(), cosAngles.end(), angleCmp );

   // Ramapping table.
   Vector<uint> remap( cosAngles.size() );
   for( uint i = 0; i < remap.size(); ++i )
   {
      remap[uint(cosAngles[i].x+0.5f)] = i;
   }

   // Count number of samples.
   stream.open( brdfs.cstr(), IFStream::TEXT );

   uint numSamples = 0;
   while( 1 )
   {
      uint num;
      stream >> num;

      if( stream.eof() )
      {
         break;
      }

      for( uint i = 0; i < cosAngles.size(); ++i )
      {
         float v;
         stream >> v;
      }
      ++numSamples;
   }
   stream.close();

   // Create bitmap.
   Vec2i dstDim( size, 128 );
   //Vec2i dstDim( size, numSamples );
   RCP<Bitmap> dstBmp = new Bitmap( dstDim, Bitmap::BYTE, 1 );


   // Read brdfs.
   Vector<float> dbrdf(size, 0.0f);
   Vector<float> brdf(cosAngles.size());

   stream.open( brdfs.cstr(), IFStream::TEXT );
   for( uint y = 0; y < numSamples; ++y )
   //for( uint y = 10; y < 11; ++y )
   {
      //uint num;
      if( y < 11 )
      {
         for( uint i = 0; i < brdf.size(); ++i )
         {
            stream >> brdf[remap[i]];
         }

         // Reajust brdf.
         for( uint i = 0; i < brdf.size(); ++i )
         {
            float i_dot_n = ins[i].y;
            float o_dot_n = outs[i].y;
            //brdf[remap[i]] *= 1.0/o_dot_n;
            brdf[remap[i]] *= (i_dot_n+o_dot_n - (i_dot_n*o_dot_n));

            //brdf[remap[i]] *= (i_dot_n+o_dot_n - (i_dot_n*o_dot_n));
            //StdErr << i+1 << " " << brdf[i] << "\n";
         }
      }


      // Compute the dbrdf.
      uint c = 0;
      for( uint i = 0; i < size; ++i )
      {
         float n_dot_h = i/float(size-1);

         // Interpolate brdf data.
         while( (c < cosAngles.size()) && (cosAngles[c].y < n_dot_h) )
         {
            ++c;
         }
         if( (n_dot_h > cosAngles[c].y) || c == 0 )
         {
            dbrdf[i] = brdf[c];
         }
         else
         {
            float t = (n_dot_h-cosAngles[c-1].y)/(cosAngles[c].y-cosAngles[c-1].y);
            dbrdf[i] = (1.0f-t)*brdf[c-1] + t*brdf[c];
         }

         // Reajust to dbrdf.
         //dbrdf[i] *= (2.0f*n_dot_h-n_dot_h*n_dot_h);
      }

      // Copy to result bitmap.
      uchar* p = dstBmp->pixelRow(y);
      for( uint x = 0; x < size; ++x, ++p )
      {
         float c = dbrdf[x] * 255.0f + 0.5f;
         c = CGM::clamp( c, 0.0f, 255.0f );
         *p = (uchar)c;
      }
   }

   return dstBmp;
#endif
}


//==============
// Manipulators
//==============

//-----------------------------------------------------------------------------
// Channel manipulation.
//-----------------------------------------------------------------------------

//------------------------------------------------------------------------------
//!
RCP<Bitmap> addAlpha( const Bitmap& bmp, float value )
{
   if( bmp.numChannels() != 3 )
   {
      // TODO: Add support for L --> LA
      return NULL;
   }

   if( bmp.pixelType() == Bitmap::BYTE )
   {
      value = CGM::clamp( value, 0.0f, 1.0f );
      uchar alpha = (uchar)(value * 255.0f + 0.5f);

      RCP<Bitmap> tmp = new Bitmap( bmp, Bitmap::BYTE, 4 );
      const uchar* src = bmp.pixels();
            uchar* dst = tmp->pixels();
      size_t n = bmp.numPixels();
      while( n-- )
      {
         *(dst++) = *(src++);
         *(dst++) = *(src++);
         *(dst++) = *(src++);
         *(dst++) = alpha;
      }
      return tmp;
   }
   else
   {
      RCP<Bitmap> tmp = new Bitmap( bmp, Bitmap::FLOAT, 4 );
      float* src = (float*)bmp.pixels();
      float* dst = (float*)tmp->pixels();
      size_t n = bmp.numPixels();
      while( n-- )
      {
         *(dst++) = *(src++);
         *(dst++) = *(src++);
         *(dst++) = *(src++);
         *(dst++) = value;
      }
      return tmp;
   }

   return NULL;
}

//------------------------------------------------------------------------------
//!
RCP<Bitmap>  convert( const Bitmap& src, const Bitmap::PixelType& type )
{
   uint code = (src.numChannels() - 1) & 0x03;
   code <<= 1;
   code |= (type & 0x01);
   code <<= 1;
   code |= (src.pixelType() & 0x01);

   switch( code )
   {
      //   numChannels       dstType              srcType

      // Single channel formats.
      case ((1-1)<<2) | ((Bitmap::BYTE)<<1) | (Bitmap::BYTE):
      {
         // Same format, nothing to do.
         return src.clone();
      }  break;
      case ((1-1)<<2) | ((Bitmap::BYTE)<<1) | (Bitmap::FLOAT):
      {
         // Convert floats to bytes.
         RCP<Bitmap> dst = new Bitmap( src, type, src.numChannels() );
         const uchar* srcP = src.pixels();
         const uchar* endP = srcP + src.size();
               uchar* dstP = dst->pixels();
         size_t srcOffset = src.pixelSize();
         size_t dstOffset = dst->pixelSize();
         while( srcP < endP )
         {
            convert_1x32f_1x8( srcP, dstP );
            srcP += srcOffset;
            dstP += dstOffset;
         }
         return dst;
      }  break;
      case ((1-1)<<2) | ((Bitmap::FLOAT)<<1) | (Bitmap::BYTE):
      {
         // Convert floats to bytes.
         RCP<Bitmap> dst = new Bitmap( src, type, src.numChannels() );
         const uchar* srcP = src.pixels();
         const uchar* endP = srcP + src.size();
               uchar* dstP = dst->pixels();
         size_t srcOffset = src.pixelSize();
         size_t dstOffset = dst->pixelSize();
         while( srcP < endP )
         {
            convert_1x8_1x32f( srcP, dstP );
            srcP += srcOffset;
            dstP += dstOffset;
         }
         return dst;
      }  break;
      case ((1-1)<<2) | ((Bitmap::FLOAT)<<1) | (Bitmap::FLOAT):
      {
         // Same format, nothing to do.
         return src.clone();
      }  break;

      // 2-channel formats.
      case ((2-1)<<2) | ((Bitmap::BYTE)<<1) | (Bitmap::BYTE):
      {
         // Same format, nothing to do.
         return src.clone();
      }  break;
      case ((2-1)<<2) | ((Bitmap::BYTE)<<1) | (Bitmap::FLOAT):
      {
         // Convert floats to bytes.
         RCP<Bitmap> dst = new Bitmap( src, type, src.numChannels() );
         const uchar* srcP = src.pixels();
         const uchar* endP = srcP + src.size();
               uchar* dstP = dst->pixels();
         size_t srcOffset = src.pixelSize();
         size_t dstOffset = dst->pixelSize();
         while( srcP < endP )
         {
            convert_2x32f_2x8( srcP, dstP );
            srcP += srcOffset;
            dstP += dstOffset;
         }
         return dst;
      }  break;
      case ((2-1)<<2) | ((Bitmap::FLOAT)<<1) | (Bitmap::BYTE):
      {
         // Convert floats to bytes.
         RCP<Bitmap> dst = new Bitmap( src, type, src.numChannels() );
         const uchar* srcP = src.pixels();
         const uchar* endP = srcP + src.size();
               uchar* dstP = dst->pixels();
         size_t srcOffset = src.pixelSize();
         size_t dstOffset = dst->pixelSize();
         while( srcP < endP )
         {
            convert_2x8_2x32f( srcP, dstP );
            srcP += srcOffset;
            dstP += dstOffset;
         }
         return dst;
      }  break;
      case ((2-1)<<2) | ((Bitmap::FLOAT)<<1) | (Bitmap::FLOAT):
      {
         // Same format, nothing to do.
         return src.clone();
      }  break;

      // 3-channel formats.
      case ((3-1)<<2) | ((Bitmap::BYTE)<<1) | (Bitmap::BYTE):
      {
         // Same format, nothing to do.
         return src.clone();
      }  break;
      case ((3-1)<<2) | ((Bitmap::BYTE)<<1) | (Bitmap::FLOAT):
      {
         // Convert floats to bytes.
         RCP<Bitmap> dst = new Bitmap( src, type, src.numChannels() );
         const uchar* srcP = src.pixels();
         const uchar* endP = srcP + src.size();
               uchar* dstP = dst->pixels();
         size_t srcOffset = src.pixelSize();
         size_t dstOffset = dst->pixelSize();
         while( srcP < endP )
         {
            convert_3x32f_3x8( srcP, dstP );
            srcP += srcOffset;
            dstP += dstOffset;
         }
         return dst;
      }  break;
      case ((3-1)<<2) | ((Bitmap::FLOAT)<<1) | (Bitmap::BYTE):
      {
         // Convert floats to bytes.
         RCP<Bitmap> dst = new Bitmap( src, type, src.numChannels() );
         const uchar* srcP = src.pixels();
         const uchar* endP = srcP + src.size();
               uchar* dstP = dst->pixels();
         size_t srcOffset = src.pixelSize();
         size_t dstOffset = dst->pixelSize();
         while( srcP < endP )
         {
            convert_3x8_3x32f( srcP, dstP );
            srcP += srcOffset;
            dstP += dstOffset;
         }
         return dst;
      }  break;
      case ((3-1)<<2) | ((Bitmap::FLOAT)<<1) | (Bitmap::FLOAT):
      {
         // Same format, nothing to do.
         return src.clone();
      }  break;

      // 4-channel formats.
      case ((4-1)<<2) | ((Bitmap::BYTE)<<1) | (Bitmap::BYTE):
      {
         // Same format, nothing to do.
         return src.clone();
      }  break;
      case ((4-1)<<2) | ((Bitmap::BYTE)<<1) | (Bitmap::FLOAT):
      {
         // Convert floats to bytes.
         RCP<Bitmap> dst = new Bitmap( src, type, src.numChannels() );
         const uchar* srcP = src.pixels();
         const uchar* endP = srcP + src.size();
               uchar* dstP = dst->pixels();
         size_t srcOffset = src.pixelSize();
         size_t dstOffset = dst->pixelSize();
         while( srcP < endP )
         {
            convert_4x32f_4x8( srcP, dstP );
            srcP += srcOffset;
            dstP += dstOffset;
         }
         return dst;
      }  break;
      case ((4-1)<<2) | ((Bitmap::FLOAT)<<1) | (Bitmap::BYTE):
      {
         // Convert floats to bytes.
         RCP<Bitmap> dst = new Bitmap( src, type, src.numChannels() );
         const uchar* srcP = src.pixels();
         const uchar* endP = srcP + src.size();
               uchar* dstP = dst->pixels();
         size_t srcOffset = src.pixelSize();
         size_t dstOffset = dst->pixelSize();
         while( srcP < endP )
         {
            convert_4x8_4x32f( srcP, dstP );
            srcP += srcOffset;
            dstP += dstOffset;
         }
         return dst;
      }  break;
      case ((4-1)<<2) | ((Bitmap::FLOAT)<<1) | (Bitmap::FLOAT):
      {
         // Same format, nothing to do.
         return src.clone();
      }  break;
   }

   return NULL;
}

//------------------------------------------------------------------------------
//!
RCP<Bitmap> luminanceToRGB( const Bitmap& bmp )
{
   if( bmp.numChannels() != 1 ) return NULL;

   if( bmp.pixelType() == Bitmap::BYTE )
   {
      RCP<Bitmap> tmp = new Bitmap( bmp, Bitmap::BYTE, 3 );
      const uchar* src = bmp.pixels();
            uchar* dst = tmp->pixels();
      size_t n = bmp.numPixels();
      while( n-- )
      {
         *(dst++) = *(src);
         *(dst++) = *(src);
         *(dst++) = *(src++);
      }
      return tmp;
   }
   else
   {
      RCP<Bitmap> tmp = new Bitmap( bmp, Bitmap::FLOAT, 3 );
      float* src = (float*)bmp.pixels();
      float* dst = (float*)tmp->pixels();
      size_t n = bmp.numPixels();
      while( n-- )
      {
         *(dst++) = *(src);
         *(dst++) = *(src);
         *(dst++) = *(src++);
      }
      return tmp;
   }

   return NULL;
}

//-----------------------------------------------------------------------------
//!
RCP<Bitmap> mulAdd( const Bitmap& src, const Vec4f& mul, const Vec4f& add )
{
   if( src.pixelType() != Bitmap::FLOAT )
   {
      StdErr << "BitmapManipulator::mulAdd() only support floating-point textures for now." << nl;
      return NULL;
   }
   RCP<Bitmap> dst = new Bitmap( src, Bitmap::FLOAT, src.numChannels() );
   size_t n = src.numPixels();
   switch( src.numChannels() )
   {
      case 1:
      {
         const float* srcP = (const float*)src.pixels();
         float* dstP = (float*)dst->pixels();
         float m = mul(0);
         float a = add(0);
         for( size_t i = 0; i < n; ++i, ++srcP, ++dstP )
         {
            *dstP = (*srcP)*m + a;
         }
      }  break;
      case 2:
      {
         const Vec2f* srcP = (const Vec2f*)src.pixels();
         Vec2f* dstP = (Vec2f*)dst->pixels();
         Vec2f m = mul(0,1);
         Vec2f a = add(0,1);
         for( size_t i = 0; i < n; ++i, ++srcP, ++dstP )
         {
            *dstP = (*srcP)*m + a;
         }
      }  break;
      case 3:
      {
         const Vec3f* srcP = (const Vec3f*)src.pixels();
         Vec3f* dstP = (Vec3f*)dst->pixels();
         Vec3f m = mul(0,1,2);
         Vec3f a = add(0,1,2);
         for( size_t i = 0; i < n; ++i, ++srcP, ++dstP )
         {
            *dstP = (*srcP)*m + a;
         }
      }  break;
      case 4:
      {
         const Vec4f* srcP = (const Vec4f*)src.pixels();
         Vec4f* dstP = (Vec4f*)dst->pixels();
         Vec4f m = mul;
         Vec4f a = add;
         for( size_t i = 0; i < n; ++i, ++srcP, ++dstP )
         {
            *dstP = (*srcP)*m + a;
         }
      }  break;
   }
   return dst;
}


//-----------------------------------------------------------------------------
// Pixel manipulation.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//!
RCP<Bitmap> crop( const Bitmap& src, const Vec2i& pos, const Vec2i& size )
{
   CHECK( src.dimension().x*src.dimension().y > 0 );
   Vec2i last = src.dimension()(0,1) - 1;
   Vec2i bl = CGM::clamp( pos       , Vec2i(0), last );
   Vec2i tr = CGM::clamp( pos+size-1, Vec2i(0), last );
   Vec2i s  = tr - bl + 1;
   if( s.x*s.y <= 0 )  return NULL;

   RCP<Bitmap> dst = new Bitmap( s, src.pixelType(), src.numChannels() );
   const uchar* srcP = src.pixel( bl );
   uchar* dstP       = dst->pixels();
   for( int y = bl.y; y <= tr.y; ++y, srcP += src.lineSize(), dstP += dst->lineSize() )
   {
      memcpy( dstP, srcP, dst->lineSize() );
   }
   return dst;
}
//-----------------------------------------------------------------------------
//!
RCP<Bitmap> extractSlice( const Bitmap& src, int slice, bool flip )
{
   CHECK( src.dimension().x*src.dimension().y > 0 );

   RCP<Bitmap> dst = new Bitmap( src.dimension()(0,1), src.pixelType(), src.numChannels() );

   if( flip )
   {
      const int      sy = src.height();
      const size_t   ls = src.lineSize();
      const uchar* srcP = src.pixelRow( slice, 0 );
            uchar* dstP = dst->pixelRow( slice, sy-1 );
      for( int y = 0; y < sy; ++y, srcP += ls, dstP -= ls )
      {
         memcpy( dstP, srcP, ls );
      }
   }
   else
   {
      const uchar* srcP = src.pixels( slice );
            uchar* dstP = dst->pixels();
      memcpy( dstP, srcP, dst->sliceSize() );
   }

   return dst;
}

//------------------------------------------------------------------------------
//!
RCP<Bitmap> flipVertical( const Bitmap& bmp )
{
   RCP<Bitmap> tmp = new Bitmap( bmp.dimension()(0,1), bmp.pixelType(), bmp.numChannels() );
   const uint    sy = bmp.dimension().y;
   const size_t  ls = bmp.lineSize();
   const uchar* src = bmp.pixelRow( 0 );
         uchar* dst = tmp->pixelRow( sy-1 );
   for( uint y = 0; y < sy; ++y )
   {
      memcpy( dst, src, ls );
      src += ls;
      dst -= ls;
   }
   return tmp;
}


//-----------------------------------------------------------------------------
// Filters and operators.
//-----------------------------------------------------------------------------

//------------------------------------------------------------------------------
//! Creates a downsampled version of the source image of half the size in each dimension.
//! Uses bilinear filtering.
RCP<Bitmap> downsample( const Bitmap& src )
{
   Vec2i dim = src.dimension()(0,1);

   // Some sizes, in channels (as opposed to bytes, as in the Bitmap routines).
   const int pixelStride = src.numChannels();
   const int lineStride  = pixelStride * dim.x;

   // Adjust dim for downsampled size.
   if( dim.x > 1 ) dim.x >>= 1;
   if( dim.y > 1 ) dim.y >>= 1;

   RCP<Bitmap> dst = new Bitmap( dim, src.pixelType(), src.numChannels() );

   if( src.pixelType() == Bitmap::BYTE )
   {
      const uchar* srcTL = src.pixels();
            uchar* dstP  = dst->pixels();
      for( int y = 0; y < dim.y; ++y )
      {
         for( int x = 0; x < dim.x; ++x )
         {
            const uchar* srcTR = srcTL + pixelStride;
            const uchar* srcBL = srcTL + lineStride;
            const uchar* srcBR = srcTR + lineStride;
            for( int c = 0; c < src.numChannels(); ++c )
            {
               *dstP = (uchar)( ((int)(*srcTL) + (int)(*srcTR) + (int)(*srcBL) + (int)(*srcBR)) >> 2 );
               ++dstP;
               ++srcTL; ++srcTR; ++srcBL; ++srcBR;
            }
            // Skip right pixel.
            srcTL += pixelStride;
         }
         // Skip bottom row.
         srcTL += lineStride;
      }
   }
   else
   {
      const float* srcTL = (const float*)src.pixels();
            float* dstP  = (float*)dst->pixels();
      for( int y = 0; y < dim.y; ++y )
      {
         for( int x = 0; x < dim.x; ++x )
         {
            const float* srcTR = srcTL + pixelStride;
            const float* srcBL = srcTL + lineStride;
            const float* srcBR = srcTR + lineStride;
            for( int c = 0; c < src.numChannels(); ++c )
            {
               *dstP = ((*srcTL) + (*srcTR) + (*srcBL) + (*srcBR)) * 0.25f;
               ++dstP;
               ++srcTL; ++srcTR; ++srcBL; ++srcBR;
            }
            srcTL += pixelStride;
         }
         srcTL += lineStride;
      }
   }

   return dst;
}

//-----------------------------------------------------------------------------
//! Performs a Frei-Chen edge detection filter on the image, and returns the result.
//! The Frei-Chen filter consists in the following kernel:
//!   H: [    -1     0     +1    ]    V: [    +1  +sqrt(2) +1    ]
//!      [ -sqrt(2)  0  +sqrt(2) ]       [     0     0      0    ]
//!      [    -1     0     +1    ]       [    -1  -sqrt(2) -1    ]
//! which has equal response in horizontal/vertical/diagonal directions (better than Sobel).
RCP<Bitmap>  edgeDetectFreiChen( const Bitmap& src, bool skipEdges )
{
   if( (src.pixelType() != Bitmap::FLOAT) || (src.numChannels() > 1) )
   {
      StdErr << "ERROR - BitmapManipulator::edgeDetectFreiChen() only takes floating-point grayscale textures as input (" << src.pixelType() << ", " << src.numChannels() << ")." << nl;
      return NULL;
   }
   RCP<Bitmap> dst = new Bitmap( src.dimension()(0,1), Bitmap::FLOAT, 2 );
   memset( dst->pixels(), 0, dst->size() );  // Initialize with zeroes.
   int w_1 = src.width() - 1;
   int h_1 = src.height() - 1;
   size_t ls = src.lineSize() / sizeof(float);
   size_t ps = src.pixelSize() / sizeof(float);
   const float* in = (const float*)src.pixels();
   // Loop normal cases.
   int idx = 0;
   for( int y = 1; y < h_1; ++y )
   {
      idx = y*src.width() + 1; // Beginning of the row below.
      for( int x = 1; x < w_1; ++x, ++idx )
      {
         //if( CGM::abs(in[idx]-0.5f) < 0.5f )
         {
            Vec2f& grad = Vec2f::as( (float*)dst->pixel( Vec2i(x, y) ) );
            grad.x =    in[idx-ls+ps] - in[idx-ls-ps]
                     + (in[idx   +ps] - in[idx   -ps])*CGConstf::sqrt2()
                     +  in[idx+ls+ps] - in[idx+ls-ps];
            grad.y =    in[idx+ls-ps] - in[idx-ls-ps]
                     + (in[idx+ls   ] - in[idx-ls   ])*CGConstf::sqrt2()
                     +  in[idx+ls+ps] - in[idx-ls+ps];
            normGrad( grad );
         }
      }
   }
   // Handle borders (with replication).
   if( !skipEdges )
   {
      // 1. Left and right vertical edges.
      for( int y = 1; y < h_1; ++y )
      {
         {
            // 1.1. Leftmost pixel (replicate center into left column).
            idx = y*src.width();
            Vec2f& grad = Vec2f::as( (float*)dst->pixel( Vec2i(0, y) ) );
            grad.x =    in[idx-ls+ps] - in[idx-ls- 0]
                     + (in[idx   +ps] - in[idx   - 0])*CGConstf::sqrt2()
                     +  in[idx+ls+ps] - in[idx+ls- 0];
            grad.y =    in[idx+ls- 0] - in[idx-ls- 0]
                     + (in[idx+ls   ] - in[idx-ls   ])*CGConstf::sqrt2()
                     +  in[idx+ls+ps] - in[idx-ls+ps];
            normGrad( grad );
         }
         {
            // 1.2. Rightmost pixel (replicate center into right column).
            idx = y*src.width() + w_1;
            Vec2f& grad = Vec2f::as( (float*)dst->pixel( Vec2i(w_1, y) ) );
            grad.x =    in[idx-ls+ 0] - in[idx-ls-ps]
                     + (in[idx   + 0] - in[idx   -ps])*CGConstf::sqrt2()
                     +  in[idx+ls+ 0] - in[idx+ls-ps];
            grad.y =    in[idx+ls-ps] - in[idx-ls-ps]
                     + (in[idx+ls   ] - in[idx-ls   ])*CGConstf::sqrt2()
                     +  in[idx+ls+ 0] - in[idx-ls+ 0];
            normGrad( grad );
         }
      }

      // 2. Bottom and top horizontal edges.
      {
         // 2.1. Bottommost pixel (replicate center into bottom).
         int y = 0;
         idx = y*src.width() + 1; // Beginning of the row below.
         for( int x = 1; x < w_1; ++x, ++idx )
         {
            //if( CGM::abs(in[idx]-0.5f) < 0.5f )
            {
               Vec2f& grad = Vec2f::as( (float*)dst->pixel( Vec2i(x, y) ) );
               grad.x =    in[idx- 0+ps] - in[idx- 0-ps]
                        + (in[idx   +ps] - in[idx   -ps])*CGConstf::sqrt2()
                        +  in[idx+ls+ps] - in[idx+ls-ps];
               grad.y =    in[idx+ls-ps] - in[idx- 0-ps]
                        + (in[idx+ls   ] - in[idx- 0   ])*CGConstf::sqrt2()
                        +  in[idx+ls+ps] - in[idx- 0+ps];
               normGrad( grad );
            }
         }
         // 2.2. Topmost pixel (replicate center into top).
         y = h_1;
         idx = y*src.width() + 1; // Beginning of the row below.
         for( int x = 1; x < w_1; ++x, ++idx )
         {
            //if( CGM::abs(in[idx]-0.5f) < 0.5f )
            {
               Vec2f& grad = Vec2f::as( (float*)dst->pixel( Vec2i(x, y) ) );
               grad.x =    in[idx-ls+ps] - in[idx-ls-ps]
                        + (in[idx   +ps] - in[idx   -ps])*CGConstf::sqrt2()
                        +  in[idx+ 0+ps] - in[idx+ 0-ps];
               grad.y =    in[idx+ 0-ps] - in[idx-ls-ps]
                        + (in[idx+ 0   ] - in[idx-ls   ])*CGConstf::sqrt2()
                        +  in[idx+ 0+ps] - in[idx-ls+ps];
               normGrad( grad );
            }
         }
      }

      // 3. Four corners.
      {
         {
            // 3.1. Bottom left (replicate center into left, bottom-left, and bottom).
            idx = 0*src.width() + 0;
            Vec2f& grad = Vec2f::as( (float*)dst->pixel( Vec2i(0, 0) ) );
            grad.x =    in[idx- 0+ps] - in[idx- 0- 0]
                     + (in[idx   +ps] - in[idx   - 0])*CGConstf::sqrt2()
                     +  in[idx+ls+ps] - in[idx+ls- 0];
            grad.y =    in[idx+ls- 0] - in[idx- 0- 0]
                     + (in[idx+ls   ] - in[idx- 0   ])*CGConstf::sqrt2()
                     +  in[idx+ls+ps] - in[idx- 0+ps];
            normGrad( grad );
         }
         {
            // 3.2. Bottom right (replicate center into right, bottom-right, and bottom).
            idx = 0*src.width() + w_1;
            Vec2f& grad = Vec2f::as( (float*)dst->pixel( Vec2i(w_1, 0) ) );
            grad.x =    in[idx- 0+ 0] - in[idx- 0-ps]
                     + (in[idx   + 0] - in[idx   -ps])*CGConstf::sqrt2()
                     +  in[idx+ls+ 0] - in[idx+ls-ps];
            grad.y =    in[idx+ls-ps] - in[idx- 0-ps]
                     + (in[idx+ls   ] - in[idx- 0   ])*CGConstf::sqrt2()
                     +  in[idx+ls+ 0] - in[idx- 0+ 0];
            normGrad( grad );
         }
         {
            // 3.3. Top left (replicate center into left, top-left, and top).
            idx = h_1*src.width() + 0;
            Vec2f& grad = Vec2f::as( (float*)dst->pixel( Vec2i(0, h_1) ) );
            grad.x =    in[idx-ls+ps] - in[idx-ls- 0]
                     + (in[idx   +ps] - in[idx   - 0])*CGConstf::sqrt2()
                     +  in[idx+ 0+ps] - in[idx+ 0- 0];
            grad.y =    in[idx+ 0- 0] - in[idx-ls- 0]
                     + (in[idx+ 0   ] - in[idx-ls   ])*CGConstf::sqrt2()
                     +  in[idx+ 0+ps] - in[idx-ls+ps];
            normGrad( grad );
         }
         {
            // 3.4. Top right (replicate center into right, top-right, and top).
            idx = h_1*src.width() + w_1;
            Vec2f& grad = Vec2f::as( (float*)dst->pixel( Vec2i(w_1, h_1) ) );
            grad.x =    in[idx-ls+ 0] - in[idx-ls-ps]
                     + (in[idx   + 0] - in[idx   -ps])*CGConstf::sqrt2()
                     +  in[idx+ 0+ 0] - in[idx+ 0-ps];
            grad.y =    in[idx+ 0-ps] - in[idx-ls-ps]
                     + (in[idx+ 0   ] - in[idx-ls   ])*CGConstf::sqrt2()
                     +  in[idx+ 0+ 0] - in[idx-ls+ 0];
            normGrad( grad );
         }
      }
   }
   return dst;
}

//-----------------------------------------------------------------------------
//! For every pixel, computes the 2 closest candidates (one left and one right,
//! or one bottom and one top) for each direction (horizontal, or vertical).
//! This effectively limits to 2 (or 4?) the number of candidates we have to
//! consider in each dimension.
void  computeSpans( const float* data, int sx, int sy, Vec2i* hCandidates, Vec2i* vCandidates )
{
   // Horizontal candidates.
   for( int y = 0; y < sy; ++y )
   {
      const float* src = data + y*sx;
      Vec2i*       dst = hCandidates + y*sx;
      int      closest = -INT_MAX;
      float       prev = *src;
      for( int x = 0; x < sx; prev = *src, ++x, ++src, ++dst )
      {
         if( isGray(*src) )  closest = x;
         else
         if( *src != prev )  closest = x - 1;
         dst->x = closest;
      }
      closest = INT_MAX;
      --src; --dst;
      prev = *src;
      for( int x = sx-1; x >= 0; prev = *src, --x, --src, --dst )
      {
         if( isGray(*src) )  closest = x;
         else
         if( *src != prev )  closest = x + 1;
         dst->y = closest;
      }
   }

   // Vertical candidates.
   for( int x = 0; x < sx; ++x )
   {
      const float* src = data + x;
      Vec2i*       dst = vCandidates + x;
      int      closest = -INT_MAX;
      float       prev = *src;
      for( int y = 0; y < sy; prev = *src, ++y, src += sx, dst += sx )
      {
         if( isGray(*src) )  closest = y;
         else
         if( *src != prev )  closest = y - 1;
         dst->x = closest;
      }
      closest = INT_MAX;
      src -= sx; dst -= sx;
      prev = *src;
      for( int y = sy-1; y >= 0; prev = *src, --y, src -= sx, dst -= sx )
      {
         if( isGray(*src) )  closest = y;
         else
         if( *src != prev )  closest = y + 1;
         dst->y = closest;
      }
   }

#if 0
   StdErr << "Horizontal spans:" << nl;
   for( int y = 0; y < sy; ++y )
   {
      for( int x = 0; x < sx; ++x )
      {
         int idx = y*sx + x;
         StdErr << "\t" << spanToStr(hCandidates[idx]);
      }
      StdErr << nl;
   }
   StdErr << "Vertical spans:" << nl;
   for( int y = 0; y < sy; ++y )
   {
      for( int x = 0; x < sx; ++x )
      {
         int idx = y*sx + x;
         StdErr << "\t" << spanToStr(vCandidates[idx]);
      }
      StdErr << nl;
   }
#endif
}

//-----------------------------------------------------------------------------
//! Returns the closest distance between a source pixel and the specified span.
float  closestInSpan(
   const float* src, const Vec2f* grad,
   const Vec2i& srcIdx, const float srcVal,
   const Vec2i& dstIdxXSpan, const int dstIdxY,
   const int sx, const int /*maxDist*/ )
{
   //StdErr << "  closestInSpan() srcIdx=" << srcIdx << " srcVal=" << srcVal << " dstX=" << spanToStr(dstIdxXSpan) << " dstY=" << dstIdxY << " sx=" << sx << " maxDist=" << maxDist;

   float dist = CGConstf::infinity();

   Vec2i dstIdx;
   dstIdx.y = dstIdxY;

   // Check left.
   if( dstIdxXSpan.x != -INT_MAX )
   {
      dstIdx.x = dstIdxXSpan.x;
      // TODO: Check with maxDist.
      int dstI = dstIdx.y*sx + dstIdx.x;
      dist = CGM::grayscaleDistance( srcIdx, srcVal, dstIdx, src[dstI], grad[dstI] ); // Skip closest() for this first assignment.
#if 0
      // Check x-1 as well.
      --dstIdx.x;
      if( 0 <= dstIdx.x )
      {
         --dstI;
         dist = closest( dist, CGM::grayscaleDistance( srcIdx, srcVal, dstIdx, src[dstI], grad[dstI] ) );
      }
#endif
   }

   // Check right.
   if( dstIdxXSpan.y != INT_MAX )
   {
      dstIdx.x = dstIdxXSpan.y;
      // TODO: Check with maxDist.
      int dstI = dstIdx.y*sx + dstIdx.x;
      dist = closest( dist, CGM::grayscaleDistance( srcIdx, srcVal, dstIdx, src[dstI], grad[dstI] ) );
#if 0
      // Check x-1 as well.
      ++dstIdx.x;
      if( dstIdx.x < sx )
      {
         ++dstI;
         dist = closest( dist, CGM::grayscaleDistance( srcIdx, srcVal, dstIdx, src[dstI], grad[dstI] ) );
      }
#endif
   }

   //StdErr << " >> " << dist << nl;
   return dist;
}

//-----------------------------------------------------------------------------
//!
void  computeDistanceWithSpans( const float* src, const Vec2f* grad, int sx, int sy, int maxDist, float* dst )
{
   int nPixels = sx * sy;

   // Compute closest front distance in each direction.
   ArrayGuard<Vec2i>  hSpans( nPixels );
   ArrayGuard<Vec2i>  vSpans( nPixels );
   computeSpans( src, sx, sy, hSpans.data(), vSpans.data() );

   // Iterate over all of the pixels, and do a search over the spans computed above.
   Vec2i srcIdx;
   Vec2i dstIdx;
   for( srcIdx.y = 0; srcIdx.y < sy; ++srcIdx.y )
   {
      for( srcIdx.x = 0; srcIdx.x < sx; ++srcIdx.x )
      {
         int     srcI = srcIdx.y*sx + srcIdx.x;
         float srcVal = src[srcI];
         float&  dist = dst[srcI];
         //StdErr << "srcIdx: " << srcIdx << " srcVal=" << srcVal << nl;

         Vec2i ySpan = vSpans[srcI];
         ySpan.x = CGM::max( 0, ySpan.x );
         ySpan.y = CGM::min( ySpan.y, sy-1 );
         //StdErr << "ySpan=" << ySpan << nl;
         if( isGray(srcVal) )
         {
            CHECK( ySpan.x == ySpan.y );
            dist = CGM::grayscaleDistance( srcVal, grad[srcI] );
            //StdErr << "Already gray: " << dist << nl;
         }
         else
         {
            int   dstRow;
            int   dstI;
            float dstVal;

            // Middle row.
            float best = closestInSpan( src, grad, srcIdx, srcVal, hSpans[srcI], srcIdx.y, sx, maxDist );
            //StdErr << "Middle row: " << best << nl;

            // Loop bottom and top rows.
            int maxY = Vec2i( srcIdx.y-ySpan.x, ySpan.y-srcIdx.y ).max(); // + 1; // Compute largest distance from center pixel.
            if( maxY > maxDist )  maxY = maxDist;
            for( int dy = 1; dy <= maxY; ++dy )
            {
               // Early-out test.
               if( float(dy - 0.5f) >= CGM::abs(best) )  break;

               //StdErr << "Y +/-" << dy << ":" << nl;

               // Bottom row.
               dstRow = srcIdx.y - dy;
               if( ySpan.x <= dstRow )
               {
                  dstI   = srcI - dy*sx;
                  dstVal = src[dstI];
                  if( srcVal != dstVal )
                  {
                     // Means we crossed a front.
                     best = closest( best, CGM::grayscaleDistance(srcIdx, srcVal, Vec2i(srcIdx.x, dstRow), dstVal, grad[dstI]) );
                     //StdErr << "Crossed front: " << " srcVal(" << srcI << ")=" << srcVal << " dstVal(" << dstI << ")=" << dstVal << " best=" << best << nl;
                  }
                  else
                  {
                     // Need to check the whole span.
                     best = closest( best, closestInSpan(src, grad, srcIdx, srcVal, hSpans[dstI], dstRow, sx, maxDist) );
                     //StdErr << "Whole span: " << " srcVal(" << srcI << ")=" << srcVal << " dstVal(" << dstI << ")=" << dstVal << " best=" << best << nl;
                  }
               }
               //else StdErr << "  ... skip -Y (" << dstRow << ")" << nl;

               // Top row.
               dstRow = srcIdx.y + dy;
               if( dstRow <= ySpan.y )
               {
                  dstI   = srcI + dy*sx;
                  dstVal = src[dstI];
                  if( srcVal != dstVal )
                  {
                     // Means we crossed a front.
                     best = closest( best, CGM::grayscaleDistance(srcIdx, srcVal, Vec2i(srcIdx.x, dstRow), dstVal, grad[dstI]) );
                     //StdErr << "Crossed front: " << " srcVal(" << srcI << ")=" << srcVal << " dstVal(" << dstI << ")=" << dstVal << " best=" << best << nl;
                  }
                  else
                  {
                     // Need to check the whole span.
                     best = closest( best, closestInSpan(src, grad, srcIdx, srcVal, hSpans[dstI], dstRow, sx, maxDist) );
                     //StdErr << "Whole span: " << " srcVal(" << srcI << ")=" << srcVal << " dstVal(" << dstI << ")=" << dstVal << " best=" << best << nl;
                  }
               }
               //else StdErr << "  ... skip +Y (" << dstRow << ")" << nl;
            }

            dist = best;
            //StdErr << "Best = " << dist << nl;
            //StdErr << "---" << nl;
         }
      }
      //StdErr << "--------------------------------------------------------------------------------" << nl;
   }
}

//-----------------------------------------------------------------------------
//! Does an exhaustive search of all of the pixels to find the closest candidate.
void  computeDistanceWithFullSearch( const float* src, const Vec2f* grad, int sx, int sy, int maxDist, float* dst )
{
   Vec2i srcIdx;
   //float srcVal;
   Vec2i dstIdx;
   //float dstVal;
   if( maxDist == INT_MAX )
   {
      for( srcIdx.y = 0; srcIdx.y < sy; ++srcIdx.y )
      {
         for( srcIdx.x = 0; srcIdx.x < sx; ++srcIdx.x )
         {
            int     srcI = srcIdx.y*sx + srcIdx.x;
            float srcVal = src[srcI];
            float&  dist = dst[srcI];

            if( isGray(srcVal) )
            {
               dist = CGM::grayscaleDistance( srcVal, grad[srcI] );
            }
            else
            {
               dist = CGConstf::infinity();

               // Loop every other pixel.
               for( dstIdx.y = 0; dstIdx.y < sy; ++dstIdx.y )
               {
                  for( dstIdx.x = 0; dstIdx.x < sx; ++dstIdx.x )
                  {
                     int            dstI = dstIdx.y*sx + dstIdx.x;
                     float        dstVal = src[dstI];
                     const Vec2f& dstGrd = grad[dstI];

                     float tmp = CGM::grayscaleDistance(srcIdx, srcVal, dstIdx, dstVal, dstGrd);
                     dist = closest( dist, tmp );
                     //StdErr << srcIdx << " " << dstIdx << " " << tmp << " >> " << dist << nl;
                  }
               }
            }
         }
         //StdErr << "---" << nl;
      }
   }
   else
   {
      for( srcIdx.y = 0; srcIdx.y < sy; ++srcIdx.y )
      {
         Vec2i ySpan = Vec2i(
            CGM::max( srcIdx.y - maxDist,   0  ),
            CGM::min( srcIdx.y + maxDist, sy-1 )
         );
         //ySpan = Vec2i( 0, sy-1 );  // Disable optimization.
         for( srcIdx.x = 0; srcIdx.x < sx; ++srcIdx.x )
         {
            Vec2i xSpan = Vec2i(
               CGM::max( srcIdx.x - maxDist,   0  ),
               CGM::min( srcIdx.x + maxDist, sx-1 )
            );
            //xSpan = Vec2i( 0, sx-1 );  // Disable optimization.
            int     srcI = srcIdx.y*sx + srcIdx.x;
            float srcVal = src[srcI];
            float&  dist = dst[srcI];

            if( isGray(srcVal) )
            {
               dist = CGM::grayscaleDistance( srcVal, grad[srcI] );
            }
            else
            {
               dist = CGConstf::infinity();

               // Loop every other pixel.
               for( dstIdx.y = ySpan.x; dstIdx.y <= ySpan.y; ++dstIdx.y )
               {
                  for( dstIdx.x = xSpan.x; dstIdx.x <= xSpan.y; ++dstIdx.x )
                  {
                     int            dstI = dstIdx.y*sx + dstIdx.x;
                     float        dstVal = src[dstI];
                     const Vec2f& dstGrd = grad[dstI];

                     float tmp = CGM::grayscaleDistance(srcIdx, srcVal, dstIdx, dstVal, dstGrd);
                     dist = closest( dist, tmp );
                     //StdErr << srcIdx << " " << dstIdx << " " << tmp << " >> " << dist << nl;
                  }
               }
            }
         }
         //StdErr << "---" << nl;
      }
   }
}

//-----------------------------------------------------------------------------
//!
RCP<Bitmap>  grayscaleToDistanceField( const Bitmap& src, int maxDist )
{
   return grayscaleToDistanceField( src, Vec2i(0), src.dimension()(0,1), maxDist );
}

//-----------------------------------------------------------------------------
//! Computes the distance field of a grayscale image using our own algorithm.
RCP<Bitmap>  grayscaleToDistanceFieldSpans( const Bitmap& src, const Vec2i& pos, const Vec2i& size, int maxDist )
{
   CHECK( src.pixelType() == Bitmap::BYTE );

   // Accurate version.
   // Ref: Gustavson et al. (Anti-Aliased Euclidean Distance Transform).
   // 1. Convert byte texture to floating-point (otherwise, 9 x INT->FLT typecast in gradient code).
   RCP<Bitmap> flt = new Bitmap( size, Bitmap::FLOAT, 1 );
   float* fltP = (float*)flt->pixels();
   for( int y = 0; y < size.y; ++y )
   {
      const uchar* cur = src.pixel( Vec2i(pos.x, pos.y+y) ) + src.pixelSize() - 1;
      for( int x = 0; x < size.x; ++x, cur += src.pixelSize(), ++fltP )
      {
         *fltP = float( *cur ) / 255.0f;
      }
   }
   //StdErr << "Flt:" << nl; flt->print();
   // 2. Compute the gradients.
   RCP<Bitmap> grad = BitmapManipulator::edgeDetectFreiChen( *flt, false );
   //StdErr << "Gradients:" << nl; grad->print();

   //RCP<Bitmap> bytes = convert( *flt, Bitmap::BYTE );
   //bytes->print();

   RCP<Bitmap> dst = new Bitmap( size, Bitmap::FLOAT, 1 );
   computeDistanceWithSpans( (const float*)flt->pixels(), (const Vec2f*)grad->pixels(), size.x, size.y, maxDist, (float*)dst->pixels() );
   //StdErr << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << nl;
   return dst;
}

//-----------------------------------------------------------------------------
//! Computes the distance field of a grayscale image using an exhaustive search.
RCP<Bitmap>  grayscaleToDistanceFieldFullSearch( const Bitmap& src, const Vec2i& pos, const Vec2i& size, int maxDist )
{
   CHECK( src.pixelType() == Bitmap::BYTE );

   // 1. Convert byte texture to floating-point (otherwise, 9 x INT->FLT typecast in gradient code).
   RCP<Bitmap> flt = new Bitmap( size, Bitmap::FLOAT, 1 );
   float* fltP = (float*)flt->pixels();
   for( int y = 0; y < size.y; ++y )
   {
      const uchar* cur = src.pixel( Vec2i(pos.x, pos.y+y) ) + src.pixelSize() - 1;
      for( int x = 0; x < size.x; ++x, cur += src.pixelSize(), ++fltP )
      {
         *fltP = float( *cur ) / 255.0f;
      }
   }
   //StdErr << "Flt:" << nl; flt->print();

   // 2. Compute the gradients.
   RCP<Bitmap> grad = BitmapManipulator::edgeDetectFreiChen( *flt, false );
   //StdErr << "Gradients:" << nl; grad->print();

   //RCP<Bitmap> bytes = convert( *flt, Bitmap::BYTE );
   //bytes->print();

   RCP<Bitmap> dst = new Bitmap( size, Bitmap::FLOAT, 1 );
   computeDistanceWithFullSearch( (const float*)flt->pixels(), (const Vec2f*)grad->pixels(), size.x, size.y, maxDist, (float*)dst->pixels() );
   //StdErr << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << nl;

   return dst;
}

//-----------------------------------------------------------------------------
//! Uses the alpha channel of the source bitmap and returns its corresponding
//! distance field (single-channel float bitmap).
//! Grayscale inputs are used to hint of a subpixel distance value.
//! This routine simply selects one of the variants.
RCP<Bitmap>  grayscaleToDistanceField( const Bitmap& src, const Vec2i& pos, const Vec2i& size, int maxDist )
{
   //return grayscaleToDistanceFieldFullSearch( src, pos, size, maxDist );
   return grayscaleToDistanceFieldSpans( src, pos, size, maxDist );
}


//===========
// Modifiers
//===========

//------------------------------------------------------------------------------
//!
bool copy( const Bitmap& src, Bitmap& dst, const Vec2i& pos )
{
   if( (src.numChannels() != dst.numChannels()) || (src.pixelType() != dst.pixelType()) )
   {
      return false;
   }

   if( pos.x >= dst.dimension().x ) return false;

   uint ymax = CGM::min( dst.dimension().y, pos.y + src.dimension().y );
   uint xmax = CGM::min( dst.dimension().x, pos.x + src.dimension().x );
   uint size = xmax - pos.x;

   for( uint y = pos.y; y < ymax; ++y )
   {
      memcpy( dst.pixel(Vec2i(pos.x,y)), src.pixelRow(y-pos.y), size*src.pixelSize() );
   }

   return true;
}

//------------------------------------------------------------------------------
//! Copies the row of pixels of srcBmp starting at srcPos (incl.)
//! and ending at srcPos + Vec2i(srcLength, 0) (excl.)
//! into the row of pixels of dstBmp starting at dstPos (incl.)
//! and ending at dstPos + Vec2i(dstLength, 0) (excl.).
bool copyRow( const Bitmap& srcBmp, const Vec2i& srcPos, const int srcLength,
                    Bitmap& dstBmp, const Vec2i& dstPos, const int dstLength )
{
   if( (srcBmp.numChannels() != dstBmp.numChannels()) || (srcBmp.pixelType() != dstBmp.pixelType()) )
   {
      return false;
   }

   if( CGM::abs(srcLength) != CGM::abs(dstLength) )
   {
      // Scaled copy.
      // TODO.
      return false;
#if 0
      Vec2f src = srcPos;
      float len = srcLen;

      // 1. Iterate every destination pixel.
      int dstDelta = (dstLength < 0) ? -1 : 1;
      float dstFactor = 1.0f / (dstLength * 2);
      uchar* dstPix = dstBmp->pixel( dstPos );
      int n = CGM::abs( dstLength );
      for( int i = 0; i < n; ++i )
      {
         float t = (2*i + 1) * dstFactor; // t = (2*i + 1)/2n = { 1/6, 3/6, 5/6 } for n = 3.
      }
#endif
   }
   else
   {
      // Simple copy.
      const uchar* src = srcBmp.pixel( srcPos );
            uchar* dst = dstBmp.pixel( dstPos );
      int n = CGM::abs( srcLength );
      memcpy( dst, src, n*srcBmp.pixelSize() );
   }

   return true;
}

//------------------------------------------------------------------------------
//! Copies the row of pixels of srcBmp starting at srcPos (incl.)
//! and ending at srcPos + Vec2i(srcLength, 0) (excl.)
//! into the column of pixels of dstBmp starting at dstPos (incl.)
//! and ending at dstPos + Vec2i(0, dstHeight) (excl.).
bool copyRowToCol( const Bitmap& srcBmp, const Vec2i& srcPos, const int srcLength,
                         Bitmap& dstBmp, const Vec2i& dstPos, const int dstHeight )
{
   if( (srcBmp.numChannels() != dstBmp.numChannels()) || (srcBmp.pixelType() != dstBmp.pixelType()) )
   {
      return false;
   }

   if( CGM::abs(srcLength) != CGM::abs(dstHeight) )
   {
      // Scaled copy.
      // TODO.
      return false;
   }
   else
   {
      // Simple copy.
      int n = CGM::abs( srcLength );
      switch( (srcBmp.numChannels()-1) | (srcBmp.pixelType() << 2) )
      {
         case 0x00: // 1 byte
         {
            const uchar* src = srcBmp.pixel( srcPos );
                  uchar* dst = dstBmp.pixel( dstPos );
            for( int i = 0; i < n; ++i )
            {
               copyPixel<uchar>( src, dst );
               src += srcBmp.pixelSize();
               dst += dstBmp.lineSize();
            }
         }  break;
         case 0x01: // 2 bytes
         {
            const uchar* src = srcBmp.pixel( srcPos );
                  uchar* dst = dstBmp.pixel( dstPos );
            for( int i = 0; i < n; ++i )
            {
               copyPixel<uint16_t>( src, dst );
               src += srcBmp.pixelSize();
               dst += dstBmp.lineSize();
            }
         }  break;
         case 0x02: // 3 bytes
         {
            const uchar* src = srcBmp.pixel( srcPos );
                  uchar* dst = dstBmp.pixel( dstPos );
            for( int i = 0; i < n; ++i )
            {
               copyPixel<uchar>( src  , dst   );
               copyPixel<uchar>( src+1, dst+1 );
               copyPixel<uchar>( src+2, dst+2 );
               src += srcBmp.pixelSize();
               dst += dstBmp.lineSize();
            }
         }  break;
         case 0x03: // 4 bytes
         {
            const uchar* src = srcBmp.pixel( srcPos );
                  uchar* dst = dstBmp.pixel( dstPos );
            for( int i = 0; i < n; ++i )
            {
               copyPixel<uint32_t>( src, dst );
               src += srcBmp.pixelSize();
               dst += dstBmp.lineSize();
            }
         }  break;
         case 0x04: // 1 float
         {
            const uchar* src = srcBmp.pixel( srcPos );
                  uchar* dst = dstBmp.pixel( dstPos );
            for( int i = 0; i < n; ++i )
            {
               copyPixel<float>( src, dst );
               src += srcBmp.pixelSize();
               dst += dstBmp.lineSize();
            }
         }  break;
         case 0x05: // 2 floats
         {
            const uchar* src = srcBmp.pixel( srcPos );
                  uchar* dst = dstBmp.pixel( dstPos );
            for( int i = 0; i < n; ++i )
            {
               copyPixel<Vec2f>( src, dst );
               src += srcBmp.pixelSize();
               dst += dstBmp.lineSize();
            }
         }  break;
         case 0x06: // 3 floats
         {
            const uchar* src = srcBmp.pixel( srcPos );
                  uchar* dst = dstBmp.pixel( dstPos );
            for( int i = 0; i < n; ++i )
            {
               copyPixel<Vec3f>( src, dst );
               src += srcBmp.pixelSize();
               dst += dstBmp.lineSize();
            }
         }  break;
         case 0x07: // 4 floats
         {
            const uchar* src = srcBmp.pixel( srcPos );
                  uchar* dst = dstBmp.pixel( dstPos );
            for( int i = 0; i < n; ++i )
            {
               copyPixel<Vec4f>( src, dst );
               src += srcBmp.pixelSize();
               dst += dstBmp.lineSize();
            }
         }  break;
      }
   }

   return true;
}

//------------------------------------------------------------------------------
//! Copies the column of pixels of srcBmp starting at srcPos (incl.)
//! and ending at srcPos + Vec2i(0, srcHeight) (excl.)
//! into the column of pixels of dstBmp starting at dstPos (incl.)
//! and ending at dstPos + Vec2i(0, dstHeight) (excl.).
bool copyCol( const Bitmap& srcBmp, const Vec2i& srcPos, const int srcHeight,
                    Bitmap& dstBmp, const Vec2i& dstPos, const int dstHeight )
{
   if( (srcBmp.numChannels() != dstBmp.numChannels()) || (srcBmp.pixelType() != dstBmp.pixelType()) )
   {
      return false;
   }

   if( CGM::abs(srcHeight) != CGM::abs(dstHeight) )
   {
      // Scaled copy.
      // TODO.
      return false;
   }
   else
   {
      // Simple copy.
      int n = CGM::abs( srcHeight );
      switch( (srcBmp.numChannels()-1) | (srcBmp.pixelType() << 2) )
      {
         case 0x00: // 1 byte
         {
            const uchar* src = srcBmp.pixel( srcPos );
                  uchar* dst = dstBmp.pixel( dstPos );
            for( int i = 0; i < n; ++i )
            {
               copyPixel<uchar>( src, dst );
               src += srcBmp.lineSize();
               dst += dstBmp.lineSize();
            }
         }  break;
         case 0x01: // 2 bytes
         {
            const uchar* src = srcBmp.pixel( srcPos );
                  uchar* dst = dstBmp.pixel( dstPos );
            for( int i = 0; i < n; ++i )
            {
               copyPixel<uint16_t>( src, dst );
               src += srcBmp.lineSize();
               dst += dstBmp.lineSize();
            }
         }  break;
         case 0x02: // 3 bytes
         {
            const uchar* src = srcBmp.pixel( srcPos );
                  uchar* dst = dstBmp.pixel( dstPos );
            for( int i = 0; i < n; ++i )
            {
               copyPixel<uchar>( src  , dst   );
               copyPixel<uchar>( src+1, dst+1 );
               copyPixel<uchar>( src+2, dst+2 );
               src += srcBmp.lineSize();
               dst += dstBmp.lineSize();
            }
         }  break;
         case 0x03: // 4 bytes
         {
            const uchar* src = srcBmp.pixel( srcPos );
                  uchar* dst = dstBmp.pixel( dstPos );
            for( int i = 0; i < n; ++i )
            {
               copyPixel<uint32_t>( src, dst );
               src += srcBmp.lineSize();
               dst += dstBmp.lineSize();
            }
         }  break;
         case 0x04: // 1 float
         {
            const uchar* src = srcBmp.pixel( srcPos );
                  uchar* dst = dstBmp.pixel( dstPos );
            for( int i = 0; i < n; ++i )
            {
               copyPixel<float>( src, dst );
               src += srcBmp.lineSize();
               dst += dstBmp.lineSize();
            }
         }  break;
         case 0x05: // 2 floats
         {
            const uchar* src = srcBmp.pixel( srcPos );
                  uchar* dst = dstBmp.pixel( dstPos );
            for( int i = 0; i < n; ++i )
            {
               copyPixel<Vec2f>( src, dst );
               src += srcBmp.lineSize();
               dst += dstBmp.lineSize();
            }
         }  break;
         case 0x06: // 3 floats
         {
            const uchar* src = srcBmp.pixel( srcPos );
                  uchar* dst = dstBmp.pixel( dstPos );
            for( int i = 0; i < n; ++i )
            {
               copyPixel<Vec3f>( src, dst );
               src += srcBmp.lineSize();
               dst += dstBmp.lineSize();
            }
         }  break;
         case 0x07: // 4 floats
         {
            const uchar* src = srcBmp.pixel( srcPos );
                  uchar* dst = dstBmp.pixel( dstPos );
            for( int i = 0; i < n; ++i )
            {
               copyPixel<Vec4f>( src, dst );
               src += srcBmp.lineSize();
               dst += dstBmp.lineSize();
            }
         }  break;
      }
   }

   return true;
}

//------------------------------------------------------------------------------
//! Copies the column of pixels of srcBmp starting at srcPos (incl.)
//! and ending at srcPos + Vec2i(0, srcHeight) (excl.)
//! into the row of pixels of dstBmp starting at dstPos (incl.)
//! and ending at dstPos + Vec2i(dstLength, 0) (excl.).
bool copyColToRow( const Bitmap& srcBmp, const Vec2i& srcPos, const int srcHeight,
                         Bitmap& dstBmp, const Vec2i& dstPos, const int dstLength )
{
   if( (srcBmp.numChannels() != dstBmp.numChannels()) || (srcBmp.pixelType() != dstBmp.pixelType()) )
   {
      return false;
   }

   if( CGM::abs(srcHeight) != CGM::abs(dstLength) )
   {
      // Scaled copy.
      // TODO.
      return false;
   }
   else
   {
      // Simple copy.
      int n = CGM::abs( srcHeight );
      switch( (srcBmp.numChannels()-1) | (srcBmp.pixelType() << 2) )
      {
         case 0x00: // 1 byte
         {
            const uchar* src = srcBmp.pixel( srcPos );
                  uchar* dst = dstBmp.pixel( dstPos );
            for( int i = 0; i < n; ++i )
            {
               copyPixel<uchar>( src, dst );
               src += srcBmp.lineSize();
               dst += dstBmp.pixelSize();
            }
         }  break;
         case 0x01: // 2 bytes
         {
            const uchar* src = srcBmp.pixel( srcPos );
                  uchar* dst = dstBmp.pixel( dstPos );
            for( int i = 0; i < n; ++i )
            {
               copyPixel<uint16_t>( src, dst );
               src += srcBmp.lineSize();
               dst += dstBmp.pixelSize();
            }
         }  break;
         case 0x02: // 3 bytes
         {
            const uchar* src = srcBmp.pixel( srcPos );
                  uchar* dst = dstBmp.pixel( dstPos );
            for( int i = 0; i < n; ++i )
            {
               copyPixel<uchar>( src  , dst   );
               copyPixel<uchar>( src+1, dst+1 );
               copyPixel<uchar>( src+2, dst+2 );
               src += srcBmp.lineSize();
               dst += dstBmp.pixelSize();
            }
         }  break;
         case 0x03: // 4 bytes
         {
            const uchar* src = srcBmp.pixel( srcPos );
                  uchar* dst = dstBmp.pixel( dstPos );
            for( int i = 0; i < n; ++i )
            {
               copyPixel<uint32_t>( src, dst );
               src += srcBmp.lineSize();
               dst += dstBmp.pixelSize();
            }
         }  break;
         case 0x04: // 1 float
         {
            const uchar* src = srcBmp.pixel( srcPos );
                  uchar* dst = dstBmp.pixel( dstPos );
            for( int i = 0; i < n; ++i )
            {
               copyPixel<float>( src, dst );
               src += srcBmp.lineSize();
               dst += dstBmp.pixelSize();
            }
         }  break;
         case 0x05: // 2 floats
         {
            const uchar* src = srcBmp.pixel( srcPos );
                  uchar* dst = dstBmp.pixel( dstPos );
            for( int i = 0; i < n; ++i )
            {
               copyPixel<Vec2f>( src, dst );
               src += srcBmp.lineSize();
               dst += dstBmp.pixelSize();
            }
         }  break;
         case 0x06: // 3 floats
         {
            const uchar* src = srcBmp.pixel( srcPos );
                  uchar* dst = dstBmp.pixel( dstPos );
            for( int i = 0; i < n; ++i )
            {
               copyPixel<Vec3f>( src, dst );
               src += srcBmp.lineSize();
               dst += dstBmp.pixelSize();
            }
         }  break;
         case 0x07: // 4 floats
         {
            const uchar* src = srcBmp.pixel( srcPos );
                  uchar* dst = dstBmp.pixel( dstPos );
            for( int i = 0; i < n; ++i )
            {
               copyPixel<Vec4f>( src, dst );
               src += srcBmp.lineSize();
               dst += dstBmp.pixelSize();
            }
         }  break;
      }
   }

   return true;
}

// Drawing.

//------------------------------------------------------------------------------
//!
void  fillRect( Bitmap& dst, const Vec2i& pos, const Vec2i& size, const void* data )
{
   // Clamp region to the Bitmap's size.
   Vec2i last = dst.dimension()(0,1) - 1;
   Vec2i sPos = CGM::clamp( pos, Vec2i(0), last );
   Vec2i ePos = pos + size;
   ePos -= 1;
   ePos = CGM::clamp( ePos, Vec2i(0), last );

   if( ePos.x < sPos.x || ePos.y < sPos.y )  return;

   uchar* s = dst.pixel( sPos );
   uchar* e = dst.pixel( ePos ) + dst.pixelSize();
   if( (ePos.x - sPos.x + 1) == dst.dimension().x )
   {
      // Lines are contiguous, fast path.
      switch( dst.pixelSize() )
      {
         case  1: // 1x8
         {
            memset_8b( s, data, e-s );
         }  break;
         case  2: // 2x8, 1x16
         {
            memset_16b( s, data, e-s );
         }  break;
         case  3: // 3x8
         {
            // Slow path.
            StdErr << "BitmapManipulator::fillRect() - 24b pixels not yet supported." << nl;
         }  break;
         case  4: // 4x8, 2x16, 1x32
         {
            memset_32b( s, data, e-s );
         }  break;
         //case  6: //      3x16
         //{
         //
         //}  break;
         case  8: //      4x16, 2x32, 1x64
         {
            memset_64b( s, data, e-s );
         }  break;
         case 12: //            3x32
         {
            // Slow path.
            StdErr << "BitmapManipulator::fillRect() - 96b pixels not yet supported." << nl;
         }  break;
         case 16: //            4x32, 2x64
         {
            memset_128b( s, data, e-s );
         }  break;
         //case 24: //                  3x64
         //{
         //
         //}  break;
         //case 32: //                  4x64
         //{
         //
         //}  break;
      }
   }
   else
   {
      // Loop over every line, one at a time.
      size_t lineLen = (ePos.x - sPos.x + 1)*dst.pixelSize();
      switch( dst.pixelSize() )
      {
         case  1: // 1x8
         {
            while( s < e )
            {
               memset_8b( s, data, lineLen );
               s += dst.lineSize();
            }
         }  break;
         case  2: // 2x8, 1x16
         {
            while( s < e )
            {
               memset_16b( s, data, lineLen );
               s += dst.lineSize();
            }
         }  break;
         case  3: // 3x8
         {
            // Slow path.
            StdErr << "BitmapManipulator::fillRect() - 24b pixels not yet supported." << nl;
         }  break;
         case  4: // 4x8, 2x16, 1x32
         {
            while( s < e )
            {
               memset_32b( s, data, lineLen );
               s += dst.lineSize();
            }
         }  break;
         //case  6: //      3x16
         //{
         //
         //}  break;
         case  8: //      4x16, 2x32, 1x64
         {
            while( s < e )
            {
               memset_64b( s, data, lineLen );
               s += dst.lineSize();
            }
         }  break;
         case 12: //            3x32
         {
            // Slow path.
            StdErr << "BitmapManipulator::fillRect() - 96b pixels not yet supported." << nl;
         }  break;
         case 16: //            4x32, 2x64
         {
            while( s < e )
            {
               memset_128b( s, data, lineLen );
               s += dst.lineSize();
            }
         }  break;
         //case 24: //                  3x64
         //{
         //
         //}  break;
         //case 32: //                  4x64
         //{
         //
         //}  break;
      }
   }
}

//------------------------------------------------------------------------------
//!
void  fillCircle( Bitmap& dst, const Vec2f& pos, float radius, const void* data )
{
   // Clamp region to the Bitmap's size.
   Vec2i last = dst.dimension()(0,1) - 1;
   Vec2i yRange = Vec2i( int(pos.y-radius), int(pos.y+radius) );
   yRange = CGM::clamp( yRange, 0, last.y );

   MemsetFunc memFunc = getMemset( dst.pixelSize() );
   if( memFunc == nullptr )  return;

   float rr = radius * radius;
   for( int y = yRange.x; y < yRange.y; ++y )
   {
      float dy = CGM::abs( pos.y - float(y) );
      float dx = CGM::sqrt( rr - dy*dy );
      Vec2i xRange = Vec2i( int(pos.x - dx + 0.5f), int(pos.x + dx + 0.5f) );
      xRange = CGM::clamp( xRange, 0, last.x );

      uchar* scanLine = dst.pixel( Vec2i(xRange.x, y) );
      memFunc( scanLine, data, (xRange.y - xRange.x)*dst.pixelSize() );
   }
}

//------------------------------------------------------------------------------
//!
bool scaleColorByAlpha( Bitmap& bmp )
{
   return scaleColorByAlpha( bmp, 0, bmp.numPixels() );
}

//------------------------------------------------------------------------------
//!
bool scaleColorByAlpha( Bitmap& bmp, size_t firstPixelIdx, size_t numPixels )
{
   DBG_BLOCK( os_bmp, "Bitmap::scaleColorByAlpha" );
   DBG_MSG( os_bmp, "Size is: " << bmp.dimension() );
   if( bmp.numChannels() == 2 )
   {
      // L A
      if( bmp.pixelType() == Bitmap::BYTE )
      {
         DBG_MSG( os_bmp, "L8_A8" );
         uchar* cur = bmp.pixel( firstPixelIdx );
         for( size_t i = 0; i < numPixels; ++i, cur += bmp.pixelSize() )
         {
            cur[0] = cur[0] * cur[1] / 255;
         }
      }
      else
      if( bmp.pixelType() == Bitmap::FLOAT )
      {
         DBG_MSG( os_bmp, "L32F_A32F" );
         uchar* cur = bmp.pixel( firstPixelIdx );
         for( size_t i = 0; i < numPixels; ++i, cur += bmp.pixelSize() )
         {
            float* curF = (float*)cur;
            curF[0] *= curF[1];
         }
      }
      else
      {
         DBG_MSG( os_bmp, "Unknown type: " << bmp.pixelType() );
         CHECK(false);
         return false;
      }
   }
   else
   if( bmp.numChannels() == 4 )
   {
      // R G B A
      if( bmp.pixelType() == Bitmap::BYTE )
      {
         DBG_MSG( os_bmp, "R8_G8_B8_A8" );
         uchar* cur = bmp.pixel( firstPixelIdx );
         for( size_t i = 0; i < numPixels; ++i, cur += bmp.pixelSize() )
         {
            cur[0] = (int)cur[0] * (int)cur[3] / 255;
            cur[1] = (int)cur[1] * (int)cur[3] / 255;
            cur[2] = (int)cur[2] * (int)cur[3] / 255;
         }
      }
      else
      if( bmp.pixelType() == Bitmap::FLOAT )
      {
         DBG_MSG( os_bmp, "R32_G32_B32_A32" );
         uchar* cur = bmp.pixel( firstPixelIdx );
         for( size_t i = 0; i < numPixels; ++i, cur += bmp.pixelSize() )
         {
            float* curF = (float*)cur;
            curF[0] *= curF[3];
            curF[1] *= curF[3];
            curF[2] *= curF[3];
         }
      }
      else
      {
         DBG_MSG( os_bmp, "Unknown type: " << bmp.pixelType() );
         CHECK(false);
         return false;
      }
   }
   return true;
}

//------------------------------------------------------------------------------
//!
bool unscaleColorByAlpha( Bitmap& bmp )
{
   return unscaleColorByAlpha( bmp, 0, bmp.numPixels() );
}

//------------------------------------------------------------------------------
//!
bool unscaleColorByAlpha( Bitmap& bmp, size_t firstPixelIdx, size_t numPixels )
{
   DBG_BLOCK( os_bmp, "Bitmap::unscaleColorByAlpha" );
   DBG_MSG( os_bmp, "Size is: " << bmp.dimension() );
   if( bmp.numChannels() == 2 )
   {
      // L A
      if( bmp.pixelType() == Bitmap::BYTE )
      {
         DBG_MSG( os_bmp, "L8_A8" );
         uchar* cur = bmp.pixel( firstPixelIdx );
         for( size_t i = 0; i < numPixels; ++i, cur += bmp.pixelSize() )
         {
            cur[0] = (cur[1] == 0) ? cur[0] : 255 * cur[0] / cur[1];
         }
      }
      else
      if( bmp.pixelType() == Bitmap::FLOAT )
      {
         DBG_MSG( os_bmp, "L32F_A32F" );
         uchar* cur = bmp.pixel( firstPixelIdx );
         for( size_t i = 0; i < numPixels; ++i, cur += bmp.pixelSize() )
         {
            float* curF = (float*)cur;
            if( curF[1] != 0.0f ) curF[0] /= curF[1];
         }
      }
      else
      {
         DBG_MSG( os_bmp, "Unknown type: " << bmp.pixelType() );
         CHECK(false);
         return false;
      }
   }
   else
   if( bmp.numChannels() == 4 )
   {
      // R G B A
      if( bmp.pixelType() == Bitmap::BYTE )
      {
         DBG_MSG( os_bmp, "R8_G8_B8_A8" );
         uchar* cur = bmp.pixel( firstPixelIdx );
         for( size_t i = 0; i < numPixels; ++i, cur += bmp.pixelSize() )
         {
            if( cur[3] != 0 )
            {
               cur[0] = 255 * (int)cur[0] / (int)cur[3];
               cur[1] = 255 * (int)cur[1] / (int)cur[3];
               cur[2] = 255 * (int)cur[2] / (int)cur[3];
            }
         }
      }
      else
      if( bmp.pixelType() == Bitmap::FLOAT )
      {
         DBG_MSG( os_bmp, "R32_G32_B32_A32" );
         uchar* cur = bmp.pixel( firstPixelIdx );
         for( size_t i = 0; i < numPixels; ++i, cur += bmp.pixelSize() )
         {
            float* curF = (float*)cur;
            if( curF[3] != 0.0f )
            {
               curF[0] /= curF[3];
               curF[1] /= curF[3];
               curF[2] /= curF[3];
            }
         }
      }
      else
      {
         DBG_MSG( os_bmp, "Unknown type: " << bmp.pixelType() );
         CHECK(false);
         return false;
      }
   }
   return true;
}

//------------------------------------------------------------------------------
//!
void  transform( Bitmap& dst, const Vec2i& pos, const Vec2i& size, const Mat4f& mat, const Vec4f& off )
{
   if( (dst.pixelType() != Bitmap::FLOAT) || (dst.numChannels() != 4) )
   {
      StdErr << "ERROR - BitmapManipulator::transform() only takes 4-channel floating-point textures as input (" << dst.pixelType() << ", " << dst.numChannels() << ")." << nl;
      return;
   }

   // Clamp region to the Bitmap's size.
   Vec2i last = dst.dimension()(0,1) - 1;
   Vec2i sPos = CGM::clamp( pos, Vec2i(0), last );
   Vec2i ePos = pos + size;
   ePos -= 1;
   ePos = CGM::clamp( ePos, Vec2i(0), last );

   for( int y = sPos.y; y <= ePos.y; ++y )
   {
      Vec4f* dstP = (Vec4f*)dst.pixel( Vec2i(sPos.x, y) );
      for( int x = sPos.x; x <= ePos.x; ++x, ++dstP )
      {
         Vec4f& color = *dstP;
         color = mat*color + off;
      }
   }
}


//===========
// Utilities
//===========
//------------------------------------------------------------------------------
//! Performs a linear filtering between 2 values of bmp.
//! If axis=0, the filtering is horizontal, i.e. between (floor(uv.x), uv.y) and (floor(uv.x)+1, uv.y),
//! otherwise, it is vertical, i.e. between (uv.x, floor(uv.y)) and (uv.y, floor(uv.y)+1).
void linearL8( const Bitmap& bmp, const Vec2f& uv, const int axis, uchar* dst )
{
   int   i;
   float f;
   CGM::splitIntFrac( uv(axis), i, f );

   if( i < 0 )
   {
      i = 0;
      f = 0.0f;
   }
   else if( i >= bmp.dimension()(axis)-1 )
   {
      i = bmp.dimension()(axis)-1;
      f = 0.0f;
   }

   Vec2i pos;
   pos(axis)   = i;
   pos(1-axis) = (int)uv(1-axis);
   const uchar* first = bmp.pixel( pos );

   if( f != 0.0f )
   {
      const size_t offset[2] = { bmp.pixelSize(), bmp.lineSize() };
      const uchar* second = first + offset[axis];
      int fi = (int)(f * (1 << 16));
      int c;
      c  = first[0] << 16;
      c += (second[0] - first[0]) * fi;
      c += (1 << (16-1)); // Rounding.
      dst[0] = (c >> 16);
   }
   else
   {
      copyPixel<uchar>( first, dst );
   }
}

//------------------------------------------------------------------------------
//! Performs a linear filtering between 2 values of bmp.
//! If axis=0, the filtering is horizontal, i.e. between (floor(uv.x), uv.y) and (floor(uv.x)+1, uv.y),
//! otherwise, it is vertical, i.e. between (uv.x, floor(uv.y)) and (uv.y, floor(uv.y)+1).
void linearLA8( const Bitmap& bmp, const Vec2f& uv, const int axis, uchar* dst )
{
   int   i;
   float f;
   CGM::splitIntFrac( uv(axis), i, f );

   if( i < 0 )
   {
      i = 0;
      f = 0.0f;
   }
   else if( i >= bmp.dimension()(axis)-1 )
   {
      i = bmp.dimension()(axis)-1;
      f = 0.0f;
   }

   Vec2i pos;
   pos(axis)   = i;
   pos(1-axis) = (int)uv(1-axis);
   const uchar* first = bmp.pixel( pos );

   if( f != 0.0f )
   {
      const size_t offset[2] = { bmp.pixelSize(), bmp.lineSize() };
      const uchar* second = first + offset[axis];
      int fi = (int)(f * (1 << 16));
      int c;
      c  = first[0] << 16;
      c += (second[0] - first[0]) * fi;
      c += (1 << (16-1)); // Rounding.
      dst[0] = (c >> 16);
      c  = first[1] << 16;
      c += (second[1] - first[1]) * fi;
      c += (1 << (16-1)); // Rounding.
      dst[1] = (c >> 16);
   }
   else
   {
      copyPixel<uint16_t>( first, dst );
   }
}

//------------------------------------------------------------------------------
//! Performs a linear filtering between 2 values of bmp.
//! If axis=0, the filtering is horizontal, i.e. between (floor(uv.x), uv.y) and (floor(uv.x)+1, uv.y),
//! otherwise, it is vertical, i.e. between (uv.x, floor(uv.y)) and (uv.y, floor(uv.y)+1).
void linearRGB8( const Bitmap& bmp, const Vec2f& uv, const int axis, uchar* dst )
{
   int   i;
   float f;
   CGM::splitIntFrac( uv(axis), i, f );

   if( i < 0 )
   {
      i = 0;
      f = 0.0f;
   }
   else if( i >= bmp.dimension()(axis)-1 )
   {
      i = bmp.dimension()(axis)-1;
      f = 0.0f;
   }

   Vec2i pos;
   pos(axis)   = i;
   pos(1-axis) = (int)uv(1-axis);
   const uchar* first = bmp.pixel( pos );

   if( f != 0.0f )
   {
      const size_t offset[2] = { bmp.pixelSize(), bmp.lineSize() };
      const uchar* second = first + offset[axis];
      int fi = (int)(f * (1 << 16));
      int c;
      c  = first[0] << 16;
      c += (second[0] - first[0]) * fi;
      c += (1 << (16-1)); // Rounding.
      dst[0] = (c >> 16);
      c  = first[1] << 16;
      c += (second[1] - first[1]) * fi;
      c += (1 << (16-1)); // Rounding.
      dst[1] = (c >> 16);
      c  = first[2] << 16;
      c += (second[2] - first[2]) * fi;
      c += (1 << (16-1)); // Rounding.
      dst[2] = (c >> 16);
   }
   else
   {
      copyPixel<uchar>( first  , dst   );
      copyPixel<uchar>( first+1, dst+1 );
      copyPixel<uchar>( first+2, dst+2 );
   }
}

//------------------------------------------------------------------------------
//! Performs a linear filtering between 2 values of bmp.
//! If axis=0, the filtering is horizontal, i.e. between (floor(uv.x), uv.y) and (floor(uv.x)+1, uv.y),
//! otherwise, it is vertical, i.e. between (uv.x, floor(uv.y)) and (uv.y, floor(uv.y)+1).
void linearRGBA8( const Bitmap& bmp, const Vec2f& uv, const int axis, uchar* dst )
{
   int   i;
   float f;
   CGM::splitIntFrac( uv(axis), i, f );

   if( i < 0 )
   {
      i = 0;
      f = 0.0f;
   }
   else if( i >= bmp.dimension()(axis)-1 )
   {
      i = bmp.dimension()(axis)-1;
      f = 0.0f;
   }

   Vec2i pos;
   pos(axis)   = i;
   pos(1-axis) = (int)uv(1-axis);
   const uchar* first = bmp.pixel( pos );

   if( f != 0.0f )
   {
      const size_t offset[2] = { bmp.pixelSize(), bmp.lineSize() };
      const uchar* second = first + offset[axis];
      int fi = (int)(f * (1 << 16));
      int c;
      c  = first[0] << 16;
      c += (second[0] - first[0]) * fi;
      c += (1 << (16-1)); // Rounding.
      dst[0] = (c >> 16);
      c  = first[1] << 16;
      c += (second[1] - first[1]) * fi;
      c += (1 << (16-1)); // Rounding.
      dst[1] = (c >> 16);
      c  = first[2] << 16;
      c += (second[2] - first[2]) * fi;
      c += (1 << (16-1)); // Rounding.
      dst[2] = (c >> 16);
      c  = first[3] << 16;
      c += (second[3] - first[3]) * fi;
      c += (1 << (16-1)); // Rounding.
      dst[3] = (c >> 16);
   }
   else
   {
      copyPixel<uint32_t>( first, dst );
   }
}

//------------------------------------------------------------------------------
//! Performs a linear filtering between 2 values of bmp.
//! If axis=0, the filtering is horizontal, i.e. between (floor(uv.x), uv.y) and (floor(uv.x)+1, uv.y),
//! otherwise, it is vertical, i.e. between (uv.x, floor(uv.y)) and (uv.y, floor(uv.y)+1).
void linearL32f( const Bitmap& bmp, const Vec2f& uv, const int axis, uchar* dst )
{
   int   i;
   float f;
   CGM::splitIntFrac( uv(axis), i, f );

   if( i < 0 )
   {
      i = 0;
      f = 0.0f;
   }
   else if( i >= bmp.dimension()(axis)-1 )
   {
      i = bmp.dimension()(axis)-1;
      f = 0.0f;
   }

   Vec2i pos;
   pos(axis)   = i;
   pos(1-axis) = (int)uv(1-axis);
   const uchar* first = bmp.pixel( pos );

   if( f != 0.0f )
   {
      const size_t offset[2] = { bmp.pixelSize(), bmp.lineSize() };
      const uchar* second    = first + offset[axis];
      const float& f_first   = *(const float*)first;
      const float& f_second  = *(const float*)second;
      float& f_dst = *(float*)dst;
      f_dst = CGM::linear( f_first, f_second - f_first, f );
   }
   else
   {
      copyPixel<float>( first, dst );
   }
}

//------------------------------------------------------------------------------
//! Performs a linear filtering between 2 values of bmp.
//! If axis=0, the filtering is horizontal, i.e. between (floor(uv.x), uv.y) and (floor(uv.x)+1, uv.y),
//! otherwise, it is vertical, i.e. between (uv.x, floor(uv.y)) and (uv.y, floor(uv.y)+1).
void linearLA32f( const Bitmap& bmp, const Vec2f& uv, const int axis, uchar* dst )
{
   int   i;
   float f;
   CGM::splitIntFrac( uv(axis), i, f );

   if( i < 0 )
   {
      i = 0;
      f = 0.0f;
   }
   else if( i >= bmp.dimension()(axis)-1 )
   {
      i = bmp.dimension()(axis)-1;
      f = 0.0f;
   }

   Vec2i pos;
   pos(axis)   = i;
   pos(1-axis) = (int)uv(1-axis);
   const uchar* first = bmp.pixel( pos );

   if( f != 0.0f )
   {
      const size_t offset[2] = { bmp.pixelSize(), bmp.lineSize() };
      const uchar* second    = first + offset[axis];
      const Vec2f& f_first   = *(const Vec2f*)first;
      const Vec2f& f_second  = *(const Vec2f*)second;
      Vec2f& f_dst = *(Vec2f*)dst;
      f_dst = CGM::linear( f_first, f_second - f_first, f );
   }
   else
   {
      copyPixel<Vec2f>( first, dst );
   }
}

//------------------------------------------------------------------------------
//! Performs a linear filtering between 2 values of bmp.
//! If axis=0, the filtering is horizontal, i.e. between (floor(uv.x), uv.y) and (floor(uv.x)+1, uv.y),
//! otherwise, it is vertical, i.e. between (uv.x, floor(uv.y)) and (uv.y, floor(uv.y)+1).
void linearRGB32f( const Bitmap& bmp, const Vec2f& uv, const int axis, uchar* dst )
{
   int   i;
   float f;
   CGM::splitIntFrac( uv(axis), i, f );

   if( i < 0 )
   {
      i = 0;
      f = 0.0f;
   }
   else if( i >= bmp.dimension()(axis)-1 )
   {
      i = bmp.dimension()(axis)-1;
      f = 0.0f;
   }

   Vec2i pos;
   pos(axis)   = i;
   pos(1-axis) = (int)uv(1-axis);
   const uchar* first = bmp.pixel( pos );

   if( f != 0.0f )
   {
      const size_t offset[2] = { bmp.pixelSize(), bmp.lineSize() };
      const uchar* second    = first + offset[axis];
      const Vec3f& f_first   = *(const Vec3f*)first;
      const Vec3f& f_second  = *(const Vec3f*)second;
      Vec3f& f_dst = *(Vec3f*)dst;
      f_dst = CGM::linear( f_first, f_second - f_first, f );
   }
   else
   {
      copyPixel<Vec3f>( first, dst );
   }
}

//------------------------------------------------------------------------------
//! Performs a linear filtering between 2 values of bmp.
//! If axis=0, the filtering is horizontal, i.e. between (floor(uv.x), uv.y) and (floor(uv.x)+1, uv.y),
//! otherwise, it is vertical, i.e. between (uv.x, floor(uv.y)) and (uv.y, floor(uv.y)+1).
void linearRGBA32f( const Bitmap& bmp, const Vec2f& uv, const int axis, uchar* dst )
{
   int   i;
   float f;
   CGM::splitIntFrac( uv(axis), i, f );

   if( i < 0 )
   {
      i = 0;
      f = 0.0f;
   }
   else if( i >= bmp.dimension()(axis)-1 )
   {
      i = bmp.dimension()(axis)-1;
      f = 0.0f;
   }

   Vec2i pos;
   pos(axis)   = i;
   pos(1-axis) = (int)uv(1-axis);
   const uchar* first = bmp.pixel( pos );

   if( f != 0.0f )
   {
      const size_t offset[2] = { bmp.pixelSize(), bmp.lineSize() };
      const uchar* second    = first + offset[axis];
      const Vec4f& f_first   = *(const Vec4f*)first;
      const Vec4f& f_second  = *(const Vec4f*)second;
      Vec4f& f_dst = *(Vec4f*)dst;
      f_dst = CGM::linear( f_first, f_second - f_first, f );
   }
   else
   {
      copyPixel<Vec4f>( first, dst );
   }
}

//------------------------------------------------------------------------------
//! Does a point-sampled texture fetch in the specified bitmap.
//! This routine assumes the texture has normalized coordinates (range [0, 1] over
//! the whole texture) and only supports wrapping behavior.
//! It also converts the texels in bmp into floating point values so that the
//! math operations are all done in floating-point arithmetic.
Vec4f nearest( const Bitmap& bmp, const Vec2f& st )
{
   // FIXME: Only wrap behavior for now...

   // Clamp coordinate to be within [0, 1] range.
   Vec2f texUV( CGM::fmod(st.x, 1.0f), CGM::fmod(st.y, 1.0f) );

   // Range [0, w], [0, h].
   texUV *= Vec2f( bmp.dimension()(0,1) );

   // Retrieve the integer part.
   Vec2i txl;
   txl.x = CGM::floori( texUV.x );
   txl.y = CGM::floori( texUV.y );

   //StdErr << nl;
   //StdErr << "st=" << st << nl;
   //StdErr << "txl=" << txl << nl;
   //StdErr << "dim=" << bmp.dimension() << nl;

   txl.x = CGM::modulo( txl.x, bmp.dimension().x ); // Necessary?
   txl.y = CGM::modulo( txl.y, bmp.dimension().y );
   //StdErr << "txl=" << txl << nl;

   switch( bmp.numChannels() )
   {
      case 1:
      {
         if( bmp.pixelType() == Bitmap::BYTE )
         {
            float final = getL8( bmp, txl );
            final *= 1.0f/255.0f; // Rescale from [0.0, 255.0] to [0.0, 1.0].
            return Vec4f( final, final, final, 1.0f ); // Return LLL1.
         }
         else
         {
            float final = getL32f( bmp, txl );
            return Vec4f( final, final, final, 1.0f ); // Return LLL1.
         }
      }
      case 2:
      {
         if( bmp.pixelType() == Bitmap::BYTE )
         {
            Vec2f final = getLA8( bmp, txl );
            final *= 1.0f/255.0f; // Rescale from [0.0, 255.0] to [0.0, 1.0].
            return Vec4f( final.x, final.x, final.x, final.y ); // Return LLLA.
         }
         else
         {
            Vec2f final = getLA32f( bmp, txl );
            return Vec4f( final.x, final.x, final.x, final.y ); // Return LLLA.
         }
      }
      case 3:
      {
         if( bmp.pixelType() == Bitmap::BYTE )
         {
            Vec3f final = getRGB8( bmp, txl );
            final *= 1.0f/255.0f; // Rescale from [0.0, 255.0] to [0.0, 1.0].
            return Vec4f( final, 1.0f );
         }
         else
         {
            Vec3f final = getRGB32f( bmp, txl );
            return Vec4f( final, 1.0f );
         }
      }
      case 4:
      {
         if( bmp.pixelType() == Bitmap::BYTE )
         {
            Vec4f final = getRGBA8( bmp, txl );
            final *= 1.0f/255.0f; // Rescale from [0.0, 255.0] to [0.0, 1.0].
            return final;
         }
         else
         {
            Vec4f final = getRGBA32f( bmp, txl );
            return final;
         }
      }
   }

   CHECK( false );
   return Vec4f::zero();
}

//------------------------------------------------------------------------------
//! Does a bilinear texture fetch in the specified bitmap.
//! This routine assumes the texture has normalized coordinates (range [0, 1] over
//! the whole texture) and only supports wrapping behavior.
//! It also converts the texels in bmp into floating point values so that the
//! math operations are all done in floating-point arithmetic.
Vec4f bilinear( const Bitmap& bmp, const Vec2f& st )
{
   // FIXME: Only wrap behavior for now...

   // Clamp coordinate to be within [0, 1] range.
   Vec2f texUV( CGM::fmod(st.x, 1.0f), CGM::fmod(st.y, 1.0f) );

   // Range [0, w], [0, h].
   texUV *= Vec2f( bmp.dimension()(0,1) );

   // Bilinear offset.
   texUV -= Vec2f( 0.5f );

   // Extract integer (pixel position) and fractional (weight) parts.
   int   xi, yi;
   float xf, yf;
   CGM::splitIntFrac( texUV.x, xi, xf );
   CGM::splitIntFrac( texUV.y, yi, yf );

   xi = CGM::modulo( xi, bmp.dimension().x ); // Necessary?
   yi = CGM::modulo( yi, bmp.dimension().y );
   int xip1 = CGM::modulo( xi + 1, bmp.dimension().x );
   int yip1 = CGM::modulo( yi + 1, bmp.dimension().y );
   const bool disOpt = true; // To disable the fetch optimization.

   switch( bmp.numChannels() )
   {
      case 1:
      {
         if( bmp.pixelType() == Bitmap::BYTE )
         {
            float bl =                             getL8( bmp, Vec2i(xi  , yi  ) );
            float br = (disOpt && xf    != 0.0f) ? getL8( bmp, Vec2i(xip1, yi  ) ) : 0.0f;
            float tl = (disOpt && yf    != 0.0f) ? getL8( bmp, Vec2i(xi  , yip1) ) : 0.0f;
            float tr = (disOpt && xf*yf != 0.0f) ? getL8( bmp, Vec2i(xip1, yip1) ) : 0.0f;

            float final = CGM::bilinear(
               bl, br-bl,
               tl, tr-tl,
               xf, yf
            );

            // Rescale in [0.0, 1.0] range, since getL8() returned something in [0.0, 255.0].
            final *= 1.0f/255.0f;

            return Vec4f( final, final, final, 1.0f ); // Return LLL1.
         }
         else
         {
            float bl =                             getL32f( bmp, Vec2i(xi  , yi  ) );
            float br = (disOpt && xf    != 0.0f) ? getL32f( bmp, Vec2i(xip1, yi  ) ) : 0.0f;
            float tl = (disOpt && yf    != 0.0f) ? getL32f( bmp, Vec2i(xi  , yip1) ) : 0.0f;
            float tr = (disOpt && xf*yf != 0.0f) ? getL32f( bmp, Vec2i(xip1, yip1) ) : 0.0f;

            float final = CGM::bilinear(
               bl, br-bl,
               tl, tr-tl,
               xf, yf
            );

            return Vec4f( final, final, final, 1.0f ); // Return LLL1.
         }
      }
      case 2:
      {
         if( bmp.pixelType() == Bitmap::BYTE )
         {
            Vec2f bl =                             getLA8( bmp, Vec2i(xi  , yi  ) );
            Vec2f br = (disOpt && xf    != 0.0f) ? getLA8( bmp, Vec2i(xip1, yi  ) ) : Vec2f::zero();
            Vec2f tl = (disOpt && yf    != 0.0f) ? getLA8( bmp, Vec2i(xi  , yip1) ) : Vec2f::zero();
            Vec2f tr = (disOpt && xf*yf != 0.0f) ? getLA8( bmp, Vec2i(xip1, yip1) ) : Vec2f::zero();

            Vec2f final = CGM::bilinear(
               bl, br-bl,
               tl, tr-tl,
               xf, yf
            );

            // Rescale in [0.0, 1.0] range, since getLA8() returned something in [0.0, 255.0].
            final *= 1.0f/255.0f;

            return Vec4f( final.x, final.x, final.x, final.y ); // Return LLLA.
         }
         else
         {
            Vec2f bl =                             getLA32f( bmp, Vec2i(xi  , yi  ) );
            Vec2f br = (disOpt && xf    != 0.0f) ? getLA32f( bmp, Vec2i(xip1, yi  ) ) : Vec2f::zero();
            Vec2f tl = (disOpt && yf    != 0.0f) ? getLA32f( bmp, Vec2i(xi  , yip1) ) : Vec2f::zero();
            Vec2f tr = (disOpt && xf*yf != 0.0f) ? getLA32f( bmp, Vec2i(xip1, yip1) ) : Vec2f::zero();

            Vec2f final = CGM::bilinear(
               bl, br-bl,
               tl, tr-tl,
               xf, yf
            );

            return Vec4f( final.x, final.x, final.x, final.y ); // Return LLLA.
         }
      }
      case 3:
      {
         if( bmp.pixelType() == Bitmap::BYTE )
         {
            Vec3f bl =                             getRGB8( bmp, Vec2i(xi  , yi  ) );
            Vec3f br = (disOpt && xf    != 0.0f) ? getRGB8( bmp, Vec2i(xip1, yi  ) ) : Vec3f::zero();
            Vec3f tl = (disOpt && yf    != 0.0f) ? getRGB8( bmp, Vec2i(xi  , yip1) ) : Vec3f::zero();
            Vec3f tr = (disOpt && xf*yf != 0.0f) ? getRGB8( bmp, Vec2i(xip1, yip1) ) : Vec3f::zero();

            Vec3f final = CGM::bilinear(
               bl, br-bl,
               tl, tr-tl,
               xf, yf
            );

            // Rescale in [0.0, 1.0] range, since getRGB8() returned something in [0.0, 255.0].
            final *= 1.0f/255.0f;

            return Vec4f( final, 1.0f );
         }
         else
         {
            Vec3f bl =                             getRGB32f( bmp, Vec2i(xi  , yi  ) );
            Vec3f br = (disOpt && xf    != 0.0f) ? getRGB32f( bmp, Vec2i(xip1, yi  ) ) : Vec3f::zero();
            Vec3f tl = (disOpt && yf    != 0.0f) ? getRGB32f( bmp, Vec2i(xi  , yip1) ) : Vec3f::zero();
            Vec3f tr = (disOpt && xf*yf != 0.0f) ? getRGB32f( bmp, Vec2i(xip1, yip1) ) : Vec3f::zero();

            Vec3f final = CGM::bilinear(
               bl, br-bl,
               tl, tr-tl,
               xf, yf
            );

            return Vec4f( final, 1.0f );
         }
      }
      case 4:
      {
         if( bmp.pixelType() == Bitmap::BYTE )
         {
            Vec4f bl =                             getRGBA8( bmp, Vec2i(xi  , yi  ) );
            Vec4f br = (disOpt && xf    != 0.0f) ? getRGBA8( bmp, Vec2i(xip1, yi  ) ) : Vec4f::zero();
            Vec4f tl = (disOpt && yf    != 0.0f) ? getRGBA8( bmp, Vec2i(xi  , yip1) ) : Vec4f::zero();
            Vec4f tr = (disOpt && xf*yf != 0.0f) ? getRGBA8( bmp, Vec2i(xip1, yip1) ) : Vec4f::zero();

            Vec4f final = CGM::bilinear(
               bl, br-bl,
               tl, tr-tl,
               xf, yf
            );

            // Rescale in [0.0, 1.0] range, since getRGBA8() returned something in [0.0, 255.0].
            final *= 1.0f/255.0f;

            return final;
         }
         else
         {
            Vec4f bl =                             getRGBA32f( bmp, Vec2i(xi  , yi  ) );
            Vec4f br = (disOpt && xf    != 0.0f) ? getRGBA32f( bmp, Vec2i(xip1, yi  ) ) : Vec4f::zero();
            Vec4f tl = (disOpt && yf    != 0.0f) ? getRGBA32f( bmp, Vec2i(xi  , yip1) ) : Vec4f::zero();
            Vec4f tr = (disOpt && xf*yf != 0.0f) ? getRGBA32f( bmp, Vec2i(xip1, yip1) ) : Vec4f::zero();

            Vec4f final = CGM::bilinear(
               bl, br-bl,
               tl, tr-tl,
               xf, yf
            );

            return final;
         }
      }
   }

   CHECK( false );
   return Vec4f::zero();
}

//------------------------------------------------------------------------------
//! Performs a linear blend between the pixel located at (floor(x), y) and
//! (floor(x)+1, y) using fract(x) as a blend factor.
//! The result is stored in dst in the same format as the source bitmap.
void linearH( const Bitmap& bmp, const float x, const uint y, uchar* dst )
{
   int   xi;
   float xf;
   CGM::splitIntFrac( x, xi, xf );
   const uchar* left = bmp.pixel( Vec2i(xi, y) );

   if( xf != 0.0f )
   {
      // We are using:
      //   A*(1-f) + Bf = A - Af + Bf = A + (B-A)f.
      // Also, when filtering bytes, we use 8.16 fixed point values.
      switch( (bmp.pixelType() << 2) | (bmp.numChannels()-1) )
      {
         case 0x00: // 1 byte
         {
            const uchar* b_l = left;
            const uchar* b_r = left + bmp.pixelSize();
            int xfi = (int)(xf * (1 << 16));
            int c;
            c  = b_l[0] << 16;
            c += (b_r[0] - b_l[0]) * xfi;
            c += (1 << (16-1)); // Rounding.
            dst[0] = (c >> 16);
         }  break;
         case 0x01: // 2 bytes
         {
            const uchar* b_l = left;
            const uchar* b_r = left + bmp.pixelSize();
            int xfi = (int)(xf * (1 << 16));
            int c;
            c  = b_l[0] << 16;
            c += (b_r[0] - b_l[0]) * xfi;
            c += (1 << (16-1)); // Rounding.
            dst[0] = (c >> 16);
            c  = b_l[1] << 16;
            c += (b_r[1] - b_l[1]) * xfi;
            c += (1 << (16-1)); // Rounding.
            dst[1] = (c >> 16);
         }  break;
         case 0x02: // 3 bytes
         {
            const uchar* b_l = left;
            const uchar* b_r = left + bmp.pixelSize();
            int xfi = (int)(xf * (1 << 16));
            int c;
            c  = b_l[0] << 16;
            c += (b_r[0] - b_l[0]) * xfi;
            c += (1 << (16-1)); // Rounding.
            dst[0] = (c >> 16);
            c  = b_l[1] << 16;
            c += (b_r[1] - b_l[1]) * xfi;
            c += (1 << (16-1)); // Rounding.
            dst[1] = (c >> 16);
            c  = b_l[2] << 16;
            c += (b_r[2] - b_l[2]) * xfi;
            c += (1 << (16-1)); // Rounding.
            dst[2] = (c >> 16);
         }  break;
         case 0x03: // 4 bytes
         {
            const uchar* b_l = left;
            const uchar* b_r = left + bmp.pixelSize();
            int xfi = (int)(xf * (1 << 16));
            int c;
            c  = b_l[0] << 16;
            c += (b_r[0] - b_l[0]) * xfi;
            c += (1 << (16-1)); // Rounding.
            dst[0] = (c >> 16);
            c  = b_l[1] << 16;
            c += (b_r[1] - b_l[1]) * xfi;
            c += (1 << (16-1)); // Rounding.
            dst[1] = (c >> 16);
            c  = b_l[2] << 16;
            c += (b_r[2] - b_l[2]) * xfi;
            c += (1 << (16-1)); // Rounding.
            dst[2] = (c >> 16);
            c  = b_l[3] << 16;
            c += (b_r[3] - b_l[3]) * xfi;
            c += (1 << (16-1)); // Rounding.
            dst[3] = (c >> 16);
         }  break;
         case 0x04: // 1 float
         {
            const float& f_l = *(const float*)(left);
            const float& f_r = *(const float*)(left + bmp.pixelSize());
            float& f_dst = *(float*)(dst);
            f_dst = CGM::linear( f_l, f_r - f_l, xf );
         }  break;
         case 0x05: // 2 floats
         {
            const Vec2f& f_l = *(const Vec2f*)(left);
            const Vec2f& f_r = *(const Vec2f*)(left + bmp.pixelSize());
            Vec2f& f_dst = *(Vec2f*)(dst);
            f_dst = CGM::linear( f_l, f_r - f_l, xf );
         }  break;
         case 0x06: // 3 floats
         {
            const Vec3f& f_l = *(const Vec3f*)(left);
            const Vec3f& f_r = *(const Vec3f*)(left + bmp.pixelSize());
            Vec3f& f_dst = *(Vec3f*)(dst);
            f_dst = CGM::linear( f_l, f_r - f_l, xf );
         }  break;
         case 0x07: // 4 floats
         {
            const Vec4f& f_l = *(const Vec4f*)(left);
            const Vec4f& f_r = *(const Vec4f*)(left + bmp.pixelSize());
            Vec4f& f_dst = *(Vec4f*)(dst);
            f_dst = CGM::linear( f_l, f_r - f_l, xf );
         }  break;
      }
   }
   else
   {
      // Simple copy (no blending).
      switch( (bmp.pixelType() << 2) | (bmp.numChannels()-1) )
      {
         case 0x00: // 1 byte
            copyPixel<uchar>( left, dst );
            break;
         case 0x01: // 2 bytes
            copyPixel<uint16_t>( left, dst );
            break;
         case 0x02: // 3 bytes
            copyPixel<uchar>( left  , dst   );
            copyPixel<uchar>( left+1, dst+1 );
            copyPixel<uchar>( left+2, dst+2 );
            break;
         case 0x03: // 4 bytes
            copyPixel<uint32_t>( left, dst );
            break;
         case 0x04: // 1 float
            copyPixel<float>( left, dst );
            break;
         case 0x05: // 2 floats
            copyPixel<Vec2f>( left, dst );
            break;
         case 0x06: // 3 floats
            copyPixel<Vec3f>( left, dst );
            break;
         case 0x07: // 4 floats
            copyPixel<Vec4f>( left, dst );
            break;
      }
   }
}

//------------------------------------------------------------------------------
//! Performs a linear blend between the pixel located at (x, floor(y)) and
//! (x, floor(y)+1) using fract(y) as a blend factor.
//! The result is stored in dst in the same format as the source bitmap.
void linearV( const Bitmap& bmp, const uint x, const float y, uchar* dst )
{
   int   yi;
   float yf;
   CGM::splitIntFrac( y, yi, yf );
   const uchar* bottom = bmp.pixel( Vec2i(x, yi) );

   if( yf != 0.0f )
   {
      // We are using:
      //   A*(1-f) + Bf = A - Af + Bf = A + (B-A)f.
      // Also, when filtering bytes, we use 8.16 fixed point values.
      switch( (bmp.pixelType() << 2) | (bmp.numChannels()-1) )
      {
         case 0x00: // 1 byte
         {
            const uchar* b_b = bottom;
            const uchar* b_t = bottom + bmp.lineSize();
            int yfi = (int)(yf * (1 << 16));
            int c;
            c  = b_b[0] << 16;
            c += (b_t[0] - b_b[0]) * yfi;
            c += (1 << (16-1)); // Rounding.
            dst[0] = (c >> 16);
         }  break;
         case 0x01: // 2 bytes
         {
            const uchar* b_b = bottom;
            const uchar* b_t = bottom + bmp.lineSize();
            int yfi = (int)(yf * (1 << 16));
            int c;
            c  = b_b[0] << 16;
            c += (b_t[0] - b_b[0]) * yfi;
            c += (1 << (16-1)); // Rounding.
            dst[0] = (c >> 16);
            c  = b_b[1] << 16;
            c += (b_t[1] - b_b[1]) * yfi;
            c += (1 << (16-1)); // Rounding.
            dst[1] = (c >> 16);
         }  break;
         case 0x02: // 3 bytes
         {
            const uchar* b_b = bottom;
            const uchar* b_t = bottom + bmp.lineSize();
            int yfi = (int)(yf * (1 << 16));
            int c;
            c  = b_b[0] << 16;
            c += (b_t[0] - b_b[0]) * yfi;
            c += (1 << (16-1)); // Rounding.
            dst[0] = (c >> 16);
            c  = b_b[1] << 16;
            c += (b_t[1] - b_b[1]) * yfi;
            c += (1 << (16-1)); // Rounding.
            dst[1] = (c >> 16);
            c  = b_b[2] << 16;
            c += (b_t[2] - b_b[2]) * yfi;
            c += (1 << (16-1)); // Rounding.
            dst[2] = (c >> 16);
         }  break;
         case 0x03: // 4 bytes
         {
            const uchar* b_b = bottom;
            const uchar* b_t = bottom + bmp.lineSize();
            int yfi = (int)(yf * (1 << 16));
            int c;
            c  = b_b[0] << 16;
            c += (b_t[0] - b_b[0]) * yfi;
            c += (1 << (16-1)); // Rounding.
            dst[0] = (c >> 16);
            c  = b_b[1] << 16;
            c += (b_t[1] - b_b[1]) * yfi;
            c += (1 << (16-1)); // Rounding.
            dst[1] = (c >> 16);
            c  = b_b[2] << 16;
            c += (b_t[2] - b_b[2]) * yfi;
            c += (1 << (16-1)); // Rounding.
            dst[2] = (c >> 16);
            c  = b_b[3] << 16;
            c += (b_t[3] - b_b[3]) * yfi;
            c += (1 << (16-1)); // Rounding.
            dst[3] = (c >> 16);
         }  break;
         case 0x04: // 1 float
         {
            const float& f_b = *(const float*)(bottom);
            const float& f_t = *(const float*)(bottom + bmp.lineSize());
            float& f_dst = *(float*)(dst);
            f_dst = CGM::linear( f_b, f_t - f_b, yf );
         }  break;
         case 0x05: // 2 floats
         {
            const Vec2f& f_b = *(const Vec2f*)(bottom);
            const Vec2f& f_t = *(const Vec2f*)(bottom + bmp.lineSize());
            Vec2f& f_dst = *(Vec2f*)(dst);
            f_dst = CGM::linear( f_b, f_t - f_b, yf );
         }  break;
         case 0x06: // 3 floats
         {
            const Vec3f& f_b = *(const Vec3f*)(bottom);
            const Vec3f& f_t = *(const Vec3f*)(bottom + bmp.lineSize());
            Vec3f& f_dst = *(Vec3f*)(dst);
            f_dst = CGM::linear( f_b, f_t - f_b, yf );
         }  break;
         case 0x07: // 4 floats
         {
            const Vec4f& f_b = *(const Vec4f*)(bottom);
            const Vec4f& f_t = *(const Vec4f*)(bottom + bmp.lineSize());
            Vec4f& f_dst = *(Vec4f*)(dst);
            f_dst = CGM::linear( f_b, f_t - f_b, yf );
         }  break;
      }
   }
   else
   {
      // Simple copy (no blending).
      switch( (bmp.pixelType() << 2) | (bmp.numChannels()-1) )
      {
         case 0x00: // 1 byte
            copyPixel<uchar>( bottom, dst );
            break;
         case 0x01: // 2 bytes
            copyPixel<uint16_t>( bottom, dst );
            break;
         case 0x02: // 3 bytes
            copyPixel<uchar>( bottom  , dst   );
            copyPixel<uchar>( bottom+1, dst+1 );
            copyPixel<uchar>( bottom+2, dst+2 );
            break;
         case 0x03: // 4 bytes
            copyPixel<uint32_t>( bottom, dst );
            break;
         case 0x04: // 1 float
            copyPixel<float>( bottom, dst );
            break;
         case 0x05: // 2 floats
            copyPixel<Vec2f>( bottom, dst );
            break;
         case 0x06: // 3 floats
            copyPixel<Vec3f>( bottom, dst );
            break;
         case 0x07: // 4 floats
            copyPixel<Vec4f>( bottom, dst );
            break;
      }
   }
}


} // namespace BitmapManipulator

NAMESPACE_END
