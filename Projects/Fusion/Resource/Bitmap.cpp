/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Resource/Bitmap.h>

#include <Fusion/Resource/BitmapManipulator.h>

#include <Gfx/Tex/Texture.h>

#include <Base/Dbg/DebugStream.h>
#include <Base/Util/Memory.h>
#include <Base/Util/Platform.h>

#ifndef FUSION_USE_CORE_GRAPHICS
#if PLAT_APPLE
#define FUSION_USE_CORE_GRAPHICS 1
#else
#define FUSION_USE_CORE_GRAPHICS 0
#endif
#endif

#ifndef FUSION_USE_LIBPNG
#define FUSION_USE_LIBPNG 0
#endif

#ifndef FUSION_USE_STB_IMAGE
#define FUSION_USE_STB_IMAGE 1
#endif


#if FUSION_USE_CORE_GRAPHICS
NAMESPACE_BEGIN
bool loadSlice_cg( const String& name, Bitmap& dst, int curSlice, int numSlices, bool cubemap, bool allocate );
bool saveSlice_cg( const String& name, const Bitmap& src, int slice );
NAMESPACE_END
#endif // FUSION_USE_CORE_GRAPHICS


/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

DBG_STREAM( os_bmp, "Bitmap" );


#if FUSION_USE_STB_IMAGE

#if defined(__clang__)
// Squash warnings in clang.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-value"
#endif

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <Fusion/Resource/stb_image_write.h>

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

typedef unsigned char stbi_uc;
extern "C"
{
   //extern int    stbi_write_bmp       (char const *filename, int x, int y, int comp, const void *data);
   //extern int    stbi_write_tga       (char const *filename, int x, int y, int comp, const void *data);
   //extern int    stbi_write_png       (char const *filename, int x, int y, int comp, const void *data, int stride_bytes)

   extern stbi_uc *stbi_load            (char *filename, int *x, int *y, int *comp, int req_comp);
   extern char    *stbi_failure_reason  (void);
   extern void     stbi_image_free      (stbi_uc *retval_from_stbi_load);
}

//------------------------------------------------------------------------------
//! A routine which loads a bitmap using stb_image.
bool loadSlice_stbi( const String& name, Bitmap& dst, int curSlice, int numSlices, bool cubemap, bool allocate )
{
   DBG_BLOCK( os_bmp, "loadSlice_stbi(" << name << ")" );
   int x, y, n;
   unsigned char* data = stbi_load( (char*)name.cstr(), &x, &y, &n, 0 );
   if( data != NULL )
   {
      if( allocate )  dst.init( Vec2i(x, y), numSlices, cubemap, Bitmap::BYTE, n );
      for( int j = 0; j < y; ++j )
      {
         memcpy( dst.pixelRow(curSlice, j), data + (y-j-1)*dst.lineSize(), dst.lineSize() );
      }
      stbi_image_free( data );
      size_t from = curSlice * dst.numPixelsPerSlice();
      BitmapManipulator::scaleColorByAlpha( dst, from, dst.numPixelsPerSlice() );
      return true;
   }
   else
   {
      printf("ERROR - stbi_load failed: %s\n", stbi_failure_reason());
      return false;
   }
}

//------------------------------------------------------------------------------
//! A routine which saves a bitmap using stb_image.
bool saveSlice_stbi( const String& name, const Bitmap& src, int slice )
{
   DBG_BLOCK( os_bmp, "saveFile_stbi(" << name << ")" );
   String ext       = name.getExt().lower();
   const Vec2i& dim = src.dimension()(0,1);
   char* filename   = (char*)name.cstr();
   void* data       = (void*)src.pixels(slice);
   if( ext == "bmp" )
   {
      if( stbi_write_bmp(filename, dim.x, -dim.y, src.numChannels(), data) != 0 )
      {
         return true;
      }
      else
      {
         printf("ERROR - stbi_write_bmp failed to write '%s'\n", name.cstr());
         return false;
      }
   }
   else
   if( ext == "png" )
   {
      if( stbi_write_png(filename, dim.x, -dim.y, src.numChannels(), data, 0) != 0 )
      {
         return true;
      }
      else
      {
         printf("ERROR - stbi_write_bmp failed to write '%s'\n", name.cstr());
         return false;
      }
   }
   else
   if( ext == "tga" )
   {
      if( stbi_write_tga(filename, dim.x, -dim.y, src.numChannels(), data) != 0 )
      {
         return true;
      }
      else
      {
         printf("ERROR - stbi_write_bmp failed to write '%s'\n", name.cstr());
         return false;
      }
   }
   if( ext.empty() )
   {
      // Fallback to bmp dumper if no extension is given.
      String newName = name + ".bmp";
      if( stbi_write_bmp(filename, dim.x, dim.y, src.numChannels(), data) != 0 )
      {
         return true;
      }
      else
      {
         printf("ERROR - stbi_write_bmp failed to write '%s'\n", name.cstr());
         return false;
      }
   }
   else
   {
      printf("ERROR - unknown bitmap format: %s\n", stbi_failure_reason());
   }
   return true;
}

#endif //FUSION_USE_STB_IMAGE

#if FUSION_USE_LIBPNG

#include <png.h>

int _pngChannels[4] = {
   PNG_COLOR_TYPE_GRAY,
   PNG_COLOR_TYPE_GRAY_ALPHA,
   PNG_COLOR_TYPE_RGB,
   PNG_COLOR_TYPE_RGB_ALPHA
};

//------------------------------------------------------------------------------
//! A routine which loads a bitmap using LibPNG.
bool loadSlice_libpng( const String& name, Bitmap& dst, int curSlice, int numSlices, bool cubemap, bool allocate )
{
   DBG_BLOCK( os_bmp, "loadSlice_libpng(" << name << ")" );

   FILE*        file;
   png_structp  read;
   png_infop    info;
   png_infop    endInfo;
   png_bytepp   rowBuf;
   int          bitDepth;
   int          colorType;

   if( ( file = fopen( name.cstr(), "rb" ) ) == 0 )
   {
      DBG_MSG( os_bmp, "Could not find input file " << name.cstr() );
      return false;
   }

   // Creates structure.
   read = png_create_read_struct(
      PNG_LIBPNG_VER_STRING, (png_voidp)0,
      (png_error_ptr)0, (png_error_ptr)0
   );

   info    = png_create_info_struct( read );
   endInfo = png_create_info_struct( read );

   // Verify for errors.
   if( setjmp( png_jmpbuf( read ) ) )
   {
      DBG_MSG( os_bmp, "libpng read error" );
      png_destroy_read_struct( &read, &info, &endInfo );
      fclose( file );
      return false;
   }

   // Read header.
   png_init_io( read, file );
   png_read_png( read, info, PNG_TRANSFORM_IDENTITY, 0 );

   Vec2i dim;
   dim.x     = png_get_image_width( read, info );
   dim.y     = png_get_image_height( read, info );
   bitDepth  = png_get_bit_depth( read, info );
   colorType = png_get_color_type( read, info );

   int numChannels = 0;
   switch( colorType )
   {
      case PNG_COLOR_TYPE_GRAY:       numChannels = 1; break;
      case PNG_COLOR_TYPE_GRAY_ALPHA: numChannels = 2; break;
      case PNG_COLOR_TYPE_RGB:        numChannels = 3; break;
      case PNG_COLOR_TYPE_RGB_ALPHA:  numChannels = 4; break;
      default: printf( "error: %d\n", colorType );// error
   }

   // Init bitmap.
   if( allocate )  dst.init( dim, numSlices, cubemap, Bitmap::BYTE, numChannels );

   // clear bitmap.
   memset( dst.pixelSlice(sliceID), 0, dst.sliceSize() );

   // Read bitmap.
   rowBuf = png_get_rows( read, info );

   for( int i = 0; i < dim.y; ++i )
   {
      memcpy( dst.pixelRow(sliceID, i), rowBuf[dim.y-i-1], dst.lineSize() );
   }

   // Clean up after the read, and free any memory allocated - REQUIRED
   png_destroy_read_struct( &read, &info, &endInfo );

   fclose( file );

   size_t from = curSlice * dst.numPixelsPerSlice();
   BitmapManipulator::scaleColorByAlpha( dst, from, dst.numPixelsPerSlice() );

   return true;
}

//------------------------------------------------------------------------------
//!
bool saveSlice_libpng( const String& name, const Bitmap& src, int slice )
{
   DBG_BLOCK( os_bmp, "saveFile_libpng(" << name << ")" );

   FILE*        file;
   png_structp  write;
   png_infop    info;
   png_bytepp   rowBuf;

   if( ( file = fopen( name.cstr(), "wb" ) ) == 0 )
   {
      DBG_MSG( os_bmp, "Could not create output file " << name.cstr() );
      return false;
   }

   // Creates structure.
   write = png_create_write_struct(
      PNG_LIBPNG_VER_STRING, (png_voidp)0,
      (png_error_ptr)0, (png_error_ptr)0
   );

   info = png_create_info_struct( write );

   // Verify for errors.
   if( setjmp( png_jmpbuf( write ) ) )
   {
      DBG_MSG( os_bmp, "libpng write error" );
      png_destroy_write_struct( &write, &info );
      fclose( file );
      return false;
   }

   // Write header.
   png_init_io( write, file );

   const Vec2i dim = src.dimension()(0,1);
   png_set_IHDR(
      write,
      info,
      dim.x,
      dim.y,
      8,
      _pngChannels[src.numChannels()-1],
      PNG_INTERLACE_NONE,
      PNG_COMPRESSION_TYPE_BASE,
      PNG_FILTER_TYPE_BASE
   );

   png_write_info( write, info );

   // Write bitmap.
   rowBuf = new png_bytep[dim.y];

   const uchar* buffer = src.pixels(slice);
   for( int i = 0; i < dim.y; ++i )
   {
      rowBuf[dim.y-i-1] = (png_bytep)&buffer[src.lineSize()*i];
   }

   png_write_image( write, rowBuf );

   // Clean up after the write, and free any memory allocated - REQUIRED
   png_write_end( write, info );
   png_destroy_write_struct( &write, &info );

   delete[] rowBuf;
   fclose( file );

   return true;
}

#endif //FUSION_USE_LIBPNG


//------------------------------------------------------------------------------
//! Dispatches the proper load routine based on the extension.
bool  loadSlice( const String& path, const String& ext, Bitmap& dst, int curSlice, int numSlices, bool cubemap, bool allocate )
{
   if( ext.empty() )
   {
      DBG_MSG( os_bmp, "Unknown bitmap type for: " << path );
      return false;
   }
#if FUSION_USE_LIBPNG
   else
   if( ext == "png" )
   {
      return loadSlice_libpng( path, dst, curSlice, numSlices, cubemap, allocate );
   }
#endif
#if FUSION_USE_STB_IMAGE
   else
   if( ext == "bmp" ||
       ext == "jpg" || ext == "jpeg" ||
       ext == "png" ||
       ext == "tga" ||
       ext == "psd" ||
       ext == "gif" ||
       ext == "hdr" ||
       ext == "pic" )
   {
      bool ok = loadSlice_stbi( path, dst, curSlice, numSlices, cubemap, allocate );
      return ok;
   }
#endif
#if FUSION_USE_CORE_GRAPHICS
   else
   if( ext == "bmp" ||
       ext == "jpg" || ext == "jpeg" ||
       ext == "png" ||
       ext == "tga" ||
       ext == "tif" || ext == "tiff" ||
       ext == "psd" )
   {
      return loadSlice_cg( path, dst, curSlice, numSlices, cubemap, allocate );
   }
#endif
   else
   {
      DBG_MSG( os_bmp, "Unsupported bitmap type '" << ext << "' for " << path );
      return false;
   }
}

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Bitmap
==============================================================================*/

size_t pixelTypeSize[2] = { sizeof(char), sizeof(float) };

//------------------------------------------------------------------------------
//!
void Bitmap::printInfo( TextStream& os )
{
   os << "Bitmap:";

#if FUSION_USE_LIBPNG
   os << " libpng(" << PNG_LIBPNG_VER_STRING << ")";
#endif

#if FUSION_USE_STB_IMAGE
   os << " STB_Image(>=1.18)";
#endif

   os << nl;
}

//------------------------------------------------------------------------------
//!
Bitmap::Bitmap():
   _dim(0),
   _dimType(DIM_2D),
   _pixType(BYTE),
   _strides(size_t(0)),
   _numChannels(0),
   _pixels( NULL )
{
}

//------------------------------------------------------------------------------
//!
Bitmap::Bitmap( const Vec2i& dim, const PixelType type, const int numChannels ):
   _pixels( nullptr )
{
   init( dim, type, numChannels );
}

//------------------------------------------------------------------------------
//!
Bitmap::Bitmap( const int dim, const PixelType type, const int numChannels ):
   _pixels( nullptr )
{
   init( dim, type, numChannels );
}

//------------------------------------------------------------------------------
//!
Bitmap::Bitmap( const Vec2i& dim, const int numSlices, const PixelType type, const int numChannels ):
   _pixels( nullptr )
{
   init( dim, numSlices, type, numChannels );
}

//------------------------------------------------------------------------------
//!
Bitmap::Bitmap( const Vec3i& dim, const PixelType type, const int numChannels ):
   _pixels( nullptr )
{
   init( dim, type, numChannels );
}

//------------------------------------------------------------------------------
//!
Bitmap::Bitmap( const Bitmap& src, const PixelType type, const int numChannels ):
   _pixels( nullptr )
{
   init( src, type, numChannels );
}

//------------------------------------------------------------------------------
//!
Bitmap::~Bitmap()
{
   delete [] _pixels;
}

//------------------------------------------------------------------------------
//!
RCP<Bitmap>
Bitmap::clone() const
{
   RCP<Bitmap> bmp = new Bitmap( dimension(), pixelType(), numChannels() );
   CHECK( bmp->_pixels != NULL );
   memcpy( bmp->_pixels, _pixels, size() );
   return bmp;
}

//------------------------------------------------------------------------------
//!
void
Bitmap::init( const Vec2i& dim, const PixelType type, const int numChannels )
{
   if( _pixels ) delete [] _pixels;

   _dim         = Vec3i( dim, 1 );
   _dimType     = DIM_2D;
   _pixType     = type;
   _numChannels = numChannels;
   _strides.x   = pixelTypeSize[type] * _numChannels;
   _strides.y   = _strides.x * _dim.x; // no padding for now
   _strides.z   = _strides.y * _dim.y;

   _pixels      = new uchar[ size() ];
}

//------------------------------------------------------------------------------
//!
void
Bitmap::init( const int dim, const PixelType type, const int numChannels )
{
   if( _pixels ) delete [] _pixels;

   _dim         = Vec3i( dim, dim, 6 );
   _dimType     = DIM_CUBEMAP;
   _pixType     = type;
   _numChannels = numChannels;
   _strides.x   = pixelTypeSize[type] * _numChannels;
   _strides.y   = _strides.x * _dim.x; // no padding for now
   _strides.z   = _strides.y * _dim.y;

   _pixels      = new uchar[ size() ];
}

//------------------------------------------------------------------------------
//!
void
Bitmap::init( const Vec2i& dim, const int numSlices, const PixelType type, const int numChannels )
{
   if( _pixels ) delete [] _pixels;

   _dim         = Vec3i( dim, numSlices );
   _dimType     = DIM_2D_ARRAY;
   _pixType     = type;
   _numChannels = numChannels;
   _strides.x   = pixelTypeSize[type] * _numChannels;
   _strides.y   = _strides.x * _dim.x; // no padding for now
   _strides.z   = _strides.y * _dim.y;

   _pixels      = new uchar[ size() ];
}

//------------------------------------------------------------------------------
//!
void
Bitmap::init( const Vec3i& dim, const PixelType type, const int numChannels )
{
   if( _pixels ) delete [] _pixels;

   _dim         = dim;
   _dimType     = DIM_3D;
   _pixType     = type;
   _numChannels = numChannels;
   _strides.x   = pixelTypeSize[type] * _numChannels;
   _strides.y   = _strides.x * _dim.x; // no padding for now
   _strides.z   = _strides.y * _dim.y;

   _pixels      = new uchar[ size() ];
}

//------------------------------------------------------------------------------
//!
void
Bitmap::init( const Bitmap& src, const PixelType type, const int numChannels )
{
   if( _pixels ) delete [] _pixels;

   _dim         = src._dim;
   _dimType     = src._dimType;
   _pixType     = type;
   _numChannels = numChannels;
   _strides.x   = pixelTypeSize[type] * _numChannels;
   _strides.y   = _strides.x * _dim.x; // no padding for now
   _strides.z   = _strides.y * _dim.y;

   _pixels      = new uchar[ size() ];
}

//------------------------------------------------------------------------------
//!
void
Bitmap::init(
   const Vec2i&    dim,
   const int       numSlices,
   const bool      cubemap,
   const PixelType type,
   const int       numChannels
)
{
   if( _pixels ) delete [] _pixels;

   if( cubemap )
   {
      CHECK( dim.x == dim.y );
      _dim      = Vec3i( dim.x, dim.x, 6 );
      _dimType  = DIM_CUBEMAP;
   }
   else
   if( numSlices > 1 )
   {
      _dim      = Vec3i( dim, numSlices );
      _dimType  = DIM_2D_ARRAY;
   }
   else
   {
      _dim      = Vec3i( dim, 1 );
      _dimType  = DIM_2D;
   }
   _pixType     = type;
   _numChannels = numChannels;
   _strides.x   = pixelTypeSize[type] * _numChannels;
   _strides.y   = _strides.x * _dim.x; // no padding for now
   _strides.z   = _strides.y * _dim.y;

   _pixels      = new uchar[ size() ];
   _pixels[0] = 0xDD;
}

//------------------------------------------------------------------------------
//!
void
Bitmap::clearBuffer( void* data )
{
   MemsetFunc memFunc = getMemset( pixelSize() );
   if( memFunc == nullptr )  return;
   memFunc( _pixels, (data != nullptr) ? data : Vec4f(0.0f).ptr(), size() );
}

//------------------------------------------------------------------------------
//!
bool
Bitmap::load( const String& path )
{
   DBG_BLOCK( os_bmp, "Bitmap::load" );

   String ext = path.getExt().lower();

   return loadSlice( path, ext, *this, 0, 1, false, true );
}

//------------------------------------------------------------------------------
//!
bool
Bitmap::loadCubemap( const String& path )
{
   DBG_BLOCK( os_bmp, "Bitmap::loadCubemap" );

   // Extract the extension only once.
   String ext = path.getExt().lower();

   // Copy the path in order to work in-place when generating new paths.
   String pathTmp = path;

   // Point pos to 'n' in "*_nx.ext".
   String::SizeType pos = pathTmp.size() - ext.size() - 3;

   bool ok = true;

   // Slices: 0=nx, 1=px, 2=ny, 3=py, 4=nz, 5=pz.
   ok &= loadSlice( pathTmp, ext, *this, 0, 6, true, true  );
   pathTmp[pos] = 'p'; pathTmp[pos+1] = 'x';
   ok &= loadSlice( pathTmp, ext, *this, 1, 6, true, false );
   pathTmp[pos] = 'n'; pathTmp[pos+1] = 'y';
   ok &= loadSlice( pathTmp, ext, *this, 2, 6, true, false );
   pathTmp[pos] = 'p'; pathTmp[pos+1] = 'y';
   ok &= loadSlice( pathTmp, ext, *this, 3, 6, true, false );
   pathTmp[pos] = 'n'; pathTmp[pos+1] = 'z';
   ok &= loadSlice( pathTmp, ext, *this, 4, 6, true, false );
   pathTmp[pos] = 'p'; pathTmp[pos+1] = 'z';
   ok &= loadSlice( pathTmp, ext, *this, 5, 6, true, false );

   return ok;
}

//------------------------------------------------------------------------------
//!
bool
Bitmap::save( const String& path ) const
{
   switch( dimType() )
   {
      case DIM_2D:
      {
         return save( path, 0 );
      }  break;
      case DIM_CUBEMAP:
      {
         // Assume 'file*.png' format, which would
         // yield: 'file_nx.png', 'file_px.png', etc.
         String::SizeType pos = path.find( '*' );
         String::SizeType siz = 1;
         if( pos == String::npos )
         {
            // Assume an implicit '*' before the extension.
            siz = 0;
            pos = path.rfind( '.' );
            if( pos == String::npos )
            {
               // Assume an implicit '*' at the end.
               pos = path.size();
            }
         }
         bool ok = true;
         String pathTmp = path;
         pathTmp.replace( pos, siz, "_nx" );
         ok &= save( pathTmp, Gfx::TEX_SLICE_NEG_X );
         ++pos;
         pathTmp[pos]   = 'p';
         ok &= save( pathTmp, Gfx::TEX_SLICE_POS_X );
         pathTmp[pos]   = 'n';
         pathTmp[pos+1] = 'y';
         ok &= save( pathTmp, Gfx::TEX_SLICE_NEG_Y );
         pathTmp[pos]   = 'p';
         ok &= save( pathTmp, Gfx::TEX_SLICE_POS_Y );
         pathTmp[pos]   = 'n';
         pathTmp[pos+1] = 'z';
         ok &= save( pathTmp, Gfx::TEX_SLICE_NEG_Z );
         pathTmp[pos]   = 'p';
         ok &= save( pathTmp, Gfx::TEX_SLICE_POS_Z );
         return ok;
      }  break;
      case DIM_2D_ARRAY:
      {
         int numDigits = 1;
         int numSlices = depth();
         for( int i = numSlices; i >= 10; i /= 10 )
         {
            ++numDigits;
         }
         // 0-9: '_%01d', 10-99: '_%02d', 100-999: '_%03d', etc.
         String fmt = String().format("_%%0%dd", numDigits);

         // Assume 'file*.png' format, which would
         // yield: 'file_nx.png', 'file_px.png', etc.
         String::SizeType pos = path.find( '*' );
         String::SizeType siz = 1;
         if( pos == String::npos )
         {
            // Assume an implicit '*' before the extension.
            siz = 0;
            pos = path.rfind( '.' );
            if( pos == String::npos )
            {
               // Assume an implicit '*' at the end.
               pos = path.size();
            }
         }
         String pathTmpl = path;
         String pathTmp;
         pathTmpl.replace( pos, siz, fmt );
         bool ok = true;
         for( int s = 0; s < numSlices; ++s )
         {
            pathTmp.format( pathTmpl.cstr(), s );
            ok &= save( pathTmp, s );
         }
         return ok;

      }  break;
      case DIM_3D:
      {
         // TODO.
      }  break;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
bool
Bitmap::save( const String& path, int slice ) const
{
   // FIXME: We should only convert if necessary.
   // Convert to 8Bits format.
   RCP<const Bitmap> img = this;
   if( pixelType() == FLOAT )
   {
      if( depth() > 1 )  img = BitmapManipulator::extractSlice( *img, slice );
      img = BitmapManipulator::convert( *img, BYTE );
   }

   String ext = path.getExt().lower();
   if( ext.empty() )
   {
      DBG_MSG( os_bmp, "Unknown bitmap type for: " << path );
#if   FUSION_USE_LIBPNG
      DBG_MSG( os_bmp, "Using PNG format" );
      return saveSlice_libpng( path + ".png", *img, slice );
#elif FUSION_USE_CORE_GRAPHICS
      DBG_MSG( os_bmp, "Using PNG format" );
      return saveSlice_cg( path + ".png", *img, slice );
#elif FUSION_USE_STB_IMAGE
      DBG_MSG( os_bmp, "Using PNG format" );
      return saveSlice_stbi( path + ".png", *img, slice );
#else
      StdErr << "Cannot save '" << path << "' - No default bitmap type supported." << nl;
      return false;
#endif
   }
#if FUSION_USE_LIBPNG
   else
   if( ext == "png" )
   {
      return saveSlice_libpng( path, *img, slice );
   }
#endif
#if FUSION_USE_STB_IMAGE
   else
   if( ext == "bmp" || ext == "png" || ext == "tga" )
   {
      return saveSlice_stbi( path, *img, slice );
   }
#endif
#if FUSION_USE_CORE_GRAPHICS
   else
   if( ext == "png" ||
       ext == "bmp" ||
       ext == "tga" ||
       ext == "tif" || ext == "tiff" ||
       ext == "jpg" || ext == "jpeg" ||
       ext == "rgb" ||
       ext == "pdf" ||
       ext == "gif" ||
       ext == "psd" )
   {
      return saveSlice_cg( path, *img, slice );
   }
#endif
   else
   {
      StdErr << "Cannot save '" << path << "' - Unsupported bitmap type (" << ext << ")." << nl;
      return false;
   }
}

//------------------------------------------------------------------------------
//!
Vec4f
Bitmap::getColor( const uchar* data ) const
{
   switch( (_pixType<<2) | (_numChannels-1) )
   {
      case (BYTE<<2) | (1-1):
      {
         Vec4f color;
         BitmapManipulator::convert_1x8_1x32f( data, color.ptr() );
         color.y = 0.0f;
         color.z = 0.0f;
         color.w = 1.0f;
         return color;
      }  break;
      case (BYTE<<2) | (2-1):
      {
         Vec4f color;
         BitmapManipulator::convert_2x8_2x32f( data, color.ptr() );
         color.z = 0.0f;
         color.w = 1.0f;
         return color;
      }  break;
      case (BYTE<<2) | (3-1):
      {
         Vec4f color;
         BitmapManipulator::convert_3x8_3x32f( data, color.ptr() );
         color.w = 1.0f;
         return color;
      }  break;
      case (BYTE<<2) | (4-1):
      {
         Vec4f color;
         BitmapManipulator::convert_4x8_4x32f( data, color.ptr() );
         return color;
      }  break;
      case (FLOAT<<2) | (1-1):
      {
         const float* pixel = (const float*)data;
         return Vec4f( pixel[0], 0.0f, 0.0f, 1.0f );
      }  break;
      case (FLOAT<<2) | (2-1):
      {
         const float* pixel = (const float*)data;
         return Vec4f( pixel[0], pixel[1], 0.0f, 1.0f );
      }  break;
      case (FLOAT<<2) | (3-1):
      {
         const float* pixel = (const float*)data;
         return Vec4f( pixel[0], pixel[1], pixel[2], 1.0f );
      }  break;
      case (FLOAT<<2) | (4-1):
      {
         const float* pixel = (const float*)data;
         return Vec4f( pixel[0], pixel[1], pixel[2], pixel[3] );
      }  break;
      default:
      {
         StdErr << "Bitmap::getColor() - Invalid channel/type code." << nl;
         return Vec4f( 0.0f, 0.0f, 0.0f, 1.0f );
      }
   }
}

//-----------------------------------------------------------------------------
//!
void
Bitmap::print( TextStream& os ) const
{
   switch( pixelType() )
   {
      case BYTE:
      {
         os << "Bitmap: " << width() << "x" << height() << "x" << depth() << " " << numChannels() << "-BYTE" << nl;
         const uchar* ptr = pixels();
         for( int z = 0; z < depth(); ++z )
         {
            StdErr << "Z: " << z << nl;
            for( int y = 0; y < height(); ++y )
            {
               for( int x = 0; x < width(); ++x )
               {
                  os << "\t";
                  for( int c = 0; c < numChannels(); ++c, ++ptr )
                  {
                     os << " " << toHex(*ptr) << ",";
                  }
               }
               os << nl;
            }
         }
      }  break;
      case FLOAT:
      {
         os << "Bitmap: " << width() << "x" << height() << "x" << depth() << " " << numChannels() << "-FLOAT" << nl;
         const float* ptr = (const float*)pixels();
         for( int z = 0; z < depth(); ++z )
         {
            StdErr << "Z: " << z << nl;
            for( int y = 0; y < height(); ++y )
            {
               for( int x = 0; x < width(); ++x )
               {
                  os << " ";
                  for( int c = 0; c < numChannels(); ++c, ++ptr )
                  {
                     os << " " << (*ptr) << ",";
                  }
               }
               os << nl;
            }
         }
      }  break;
      default:
         os << "Bitmap: " << width() << "x" << height() << "x" << depth() << " " << numChannels() << "-<UNKNOWN>" << nl;
         break;
   }
}


NAMESPACE_END
