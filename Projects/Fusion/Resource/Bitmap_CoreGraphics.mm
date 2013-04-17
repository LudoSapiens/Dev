/*==============================================================================
   Copyright (c) 2009, Ludo Sapiens Inc.
   All rights reserved.

   These coded instructions, statements, and computer programs contain
   unpublished, proprietary information and are protected by Federal copyright
   law. They may not be disclosed to third parties or copied or duplicated in
   any form, in whole or in part, without prior written consent.
==============================================================================*/
#include <Fusion/Resource/Bitmap.h>

#include <Fusion/Resource/BitmapManipulator.h>

#include <Base/Dbg/DebugStream.h>
#include <Base/Dbg/Defs.h>
#include <Base/Util/Platform.h>

#if PLAT_APPLE

#if PLAT_IPHONE
#import <CoreFoundation/CoreFoundation.h>
#import <UIKit/UIKit.h>
#else
#include <Quartz/Quartz.h>
#endif

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_bmp, "Bitmap" );

UNNAMESPACE_END


NAMESPACE_BEGIN

#if PLAT_IPHONE

//------------------------------------------------------------------------------
//!
bool loadSlice_cg( const String& name, Bitmap& /*dst*/, int /*curSlice*/, int /*numSlices*/, bool /*cubemap*/, bool /*allocate*/ )
{
   DBG_BLOCK( os_bmp, "loadSlice_cg(" << name << ") UNIMPLEMENTED!" );
   unused( name );
   //NSString* path = [NSString stringWithUTF8String:name.cstr()];
   //UIImage* img = [UIImage imageNamed:path];
   return false;
}

//------------------------------------------------------------------------------
//!
bool saveSlice_cg( const String& name, const Bitmap& src, int slice )
{
   DBG_BLOCK( os_bmp, "saveFile_cg(" << name << ")" );
   unused( name );
   RCP<Bitmap> srcP = BitmapManipulator::extractSlice( src, slice, true );

   CGColorSpaceRef space;
   CGBitmapInfo    bitmapInfo;
   switch( srcP->numChannels() )
   {
      case 1:
         space = CGColorSpaceCreateDeviceGray();
         bitmapInfo = kCGBitmapByteOrderDefault | kCGImageAlphaNone;
         break;
      case 2:
         //srcP = ImageManipulator::luminanceToRGB( *srcP );
         space = CGColorSpaceCreateDeviceGray();
         bitmapInfo = kCGBitmapByteOrder32Big | kCGImageAlphaPremultipliedLast;
         break;
      case 3:
         //srcP = ImageManipulator::addAlpha( *srcP );
         space = CGColorSpaceCreateDeviceRGB();
         bitmapInfo = kCGBitmapByteOrder32Big | kCGImageAlphaNone;
         break;
      case 4:
         space = CGColorSpaceCreateDeviceRGB();
         bitmapInfo = kCGBitmapByteOrder32Big | kCGImageAlphaPremultipliedLast;
         break;
      default:
         CHECK( false );
         return false;
         break;
   }
   if( space == NULL )
   {
      StdErr << "Could not allocated color space device." << nl;
      return false;
   }

   CGDataProviderRef provider = CGDataProviderCreateWithData( NULL, srcP->pixels(), srcP->size(), NULL );
   if( provider == NULL )
   {
      StdErr << "CGDataProviderCreateWithData() failed." << nl;
      CGColorSpaceRelease( space );
      return false;
   }

   size_t w = srcP->dimension().x;
   size_t h = srcP->dimension().y;
   //StdErr << w << "x" << h << " " << srcP->lineSize() << nl;
   CGImageRef srcImg = CGImageCreate(
      w, h,
      8, srcP->pixelSize()*8, srcP->lineSize(),
      space, bitmapInfo,
      provider,
      NULL,  // No need to scale and bias.
      false, // No need to interpolate.
      kCGRenderingIntentDefault
   );
   if( srcImg == NULL )
   {
      StdErr << "CGImageCreate() failed." << nl;
      CGDataProviderRelease( provider );
      CGColorSpaceRelease( space );
      return false;
   }

   UIImage* image = [UIImage imageWithCGImage:srcImg];
   if( image == NULL )
   {
      StdErr << "[UIImage imageWithCGImage:] failed." << nl;
   }

   UIImageWriteToSavedPhotosAlbum( image, nil, nil, nil );

   return true;
}

#else

//------------------------------------------------------------------------------
//!
bool loadSlice_cg( const String& name, Bitmap& dst, int curSlice, int numSlices, bool cubemap, bool allocate )
{
   DBG_BLOCK( os_bmp, "loadSlice_cg(" << name << ")" );
   CFURLRef url = CFURLCreateFromFileSystemRepresentation( NULL, (const UInt8*)name.cstr(), name.size(), false );
   if( url == NULL )
   {
      StdErr << "CFURLCreateFromFileSystemRepresentation() failed." << nl;
      return false;
   }

   CGImageSourceRef imgSrc = CGImageSourceCreateWithURL( url, NULL );
   if( imgSrc == NULL )
   {
      StdErr << "CGImageSourceCreateWithURL() failed." << nl;
      CFRelease( url );
      return false;
   }

   CGImageRef img = CGImageSourceCreateImageAtIndex( imgSrc, 0, NULL );
   if( img == NULL )
   {
      StdErr << "CGImageSourceCreateImageAtIndex() failed." << nl;
      CFRelease( imgSrc );
      CFRelease( url );
      return false;
   }

   CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
   if( space == NULL )
   {
      StdErr << "CGColorSpaceCreateDeviceRGB() failed." << nl;
      CGImageRelease( img );
      CFRelease( imgSrc );
      CFRelease( url );
      return false;
   }

   size_t w    = CGImageGetWidth( img );
   size_t h    = CGImageGetHeight( img );
   CGRect rect = { { 0, 0 }, { CGFloat(w), CGFloat(h) } };

   if( allocate )  dst.init( Vec2i(w, h), numSlices, cubemap, Bitmap::BYTE, 4 );

   CGContextRef context = CGBitmapContextCreate(
      dst.pixels(curSlice),
      w, h,
      8, w*4,
      space,
      kCGBitmapByteOrder32Host | kCGImageAlphaPremultipliedFirst
   );
   if( context == NULL )
   {
      StdErr << "CGBitmapContextCreate() failed." << nl;
      CGColorSpaceRelease( space );
      CGImageRelease( img );
      CFRelease( imgSrc );
      CFRelease( url );
      return false;
   }

   // Clear the context.
   //memset( dst.pixelSlice(curSlice), 0, dst.sliceSize() );
   CGContextClearRect( context, rect );

   // Draw the image flipped.
   CGContextScaleCTM( context, 1.0f, -1.0f );
   rect.size.height = -rect.size.height;
   CGContextDrawImage( context, rect, img );

   CGContextRelease( context );
   CGColorSpaceRelease( space );
   CGImageRelease( img );
   CFRelease( imgSrc );
   CFRelease( url );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
saveSlice_cg( const String& name, const Bitmap& src, int slice )
{
   DBG_BLOCK( os_bmp, "saveSlice_cg(" << name << ")" );
   RCP<Bitmap> srcP = BitmapManipulator::extractSlice( src, slice, true );


   CGColorSpaceRef space;
   CGBitmapInfo    bitmapInfo;
   switch( srcP->numChannels() )
   {
      case 1:
         space = CGColorSpaceCreateDeviceGray();
         bitmapInfo = kCGBitmapByteOrderDefault | kCGImageAlphaNone;
         break;
      case 2:
         //srcP = ImageManipulator::luminanceToRGB( *srcP );
         space = CGColorSpaceCreateDeviceGray();
         bitmapInfo = kCGBitmapByteOrder32Big | kCGImageAlphaPremultipliedLast;
         break;
      case 3:
         //srcP = ImageManipulator::addAlpha( *srcP );
         space = CGColorSpaceCreateDeviceRGB();
         bitmapInfo = kCGBitmapByteOrder32Big | kCGImageAlphaNone;
         break;
      case 4:
         space = CGColorSpaceCreateDeviceRGB();
         bitmapInfo = kCGBitmapByteOrder32Big | kCGImageAlphaPremultipliedLast;
         break;
      default:
         CHECK( false );
         return false;
         break;
   }
   if( space == NULL )
   {
      StdErr << "Could not allocated color space device." << nl;
      return false;
   }

   CGDataProviderRef provider = CGDataProviderCreateWithData( NULL, srcP->pixels(), srcP->size(), NULL );
   if( provider == NULL )
   {
      StdErr << "CGDataProviderCreateWithData() failed." << nl;
      CGColorSpaceRelease( space );
      return false;
   }

   size_t w = srcP->dimension().x;
   size_t h = srcP->dimension().y;
   //StdErr << w << "x" << h << " " << srcP->lineSize() << nl;
   CGImageRef srcImg = CGImageCreate(
      w, h,
      8, srcP->pixelSize()*8, srcP->lineSize(),
      space, bitmapInfo,
      provider,
      NULL,  // No need to scale and bias.
      false, // No need to interpolate.
      kCGRenderingIntentDefault
   );
   if( srcImg == NULL )
   {
      StdErr << "CGImageCreate() failed." << nl;
      CGDataProviderRelease( provider );
      CGColorSpaceRelease( space );
      return false;
   }

   CFURLRef url = CFURLCreateFromFileSystemRepresentation( NULL, (const UInt8*)name.cstr(), name.size(), false );
   if( url == NULL )
   {
      StdErr << "CFURLCreateFromFileSystemRepresentation( NULL, " << name.cstr() << ", " << name.size() << ", false ) failed." << nl;
      CGImageRelease( srcImg );
      CGDataProviderRelease( provider );
      CGColorSpaceRelease( space );
      return false;
   }

   String ext = name.getExt().lower();
   CFStringRef type;
   if     ( ext == "png" )   type = CFSTR("public.png");
   else if( ext == "bmp" )   type = CFSTR("public.bmp");
   else if( ext == "jpg" ||
            ext == "jpeg" )  type = CFSTR("public.jpeg");
   else if( ext == "tif" ||
            ext == "tiff" )  type = CFSTR("public.tiff");
   else                      type = NULL;
   CGImageDestinationRef imgDst = CGImageDestinationCreateWithURL( url, type, 1, NULL );
   if( imgDst == NULL )
   {
      StdErr << "CGImageDestinationCreateWithURL() failed." << nl;
      CFRelease( url );
      CGImageRelease( srcImg );
      CGDataProviderRelease( provider );
      CGColorSpaceRelease( space );
      return false;
   }

   CGImageDestinationAddImage( imgDst, srcImg, NULL );
   bool ok = CGImageDestinationFinalize( imgDst );

   CFRelease( imgDst );
   CFRelease( url );
   CGImageRelease( srcImg );
   CGDataProviderRelease( provider );
   CGColorSpaceRelease( space );

   return ok;
}

#endif

NAMESPACE_END

#endif
