/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Resource/BitmapManipulator.h>

USING_NAMESPACE

//------------------------------------------------------------------------------
//!
bool toDimension( const String& str, Vec2i& dim )
{
   String::SizeType pos = str.findFirstOf("Xx");
   if( pos == String::npos )  return false;
   dim.x = str.sub(0, pos).toInt();
   dim.y = str.sub(pos+1, String::npos).toInt();
   return true;
}

//------------------------------------------------------------------------------
//!
bool toDistanceField( const String& srcFile, const String& dstFile, const Vec2i& dstDim, const int kernelSize )
{
   RCP<Bitmap> srcBmp = new Bitmap( srcFile );
   if( srcBmp.isNull() )  return false;

   RCP<Bitmap> dstBmp = BitmapManipulator::toDistanceField( *srcBmp, dstDim, kernelSize );
   if( dstBmp.isNull() )  return false;

   return dstBmp->saveFile( dstFile );
}

//------------------------------------------------------------------------------
//!
int
main( const int argc, const char* argv[] )
{
   bool ok = true;

   String srcFile;
   String dstFile;
   int   kernelSize = 256;
   Vec2i dim = Vec2i( 64, 64 );
   for( int argi = 1; argi < argc; ++argi )
   {
      if( argv[argi][0] == '-' )
      {
         String option = argv[argi] + 1;
         if( option == "dim" )
         {
            ++argi;
            if( argi == argc )
            {
               StdErr << "ERROR - Missing dimension argument." << nl;
               return 2;
            }
            String dimStr = String( argv[argi] );
            if( !toDimension(dimStr, dim) )
            {
               StdErr << "ERROR - Could not convert dimension: " << dimStr << nl;
               return 2;
            }
         }
         else
         if( option == "size" )
         {
            ++argi;
            if( argi == argc )
            {
               StdErr << "ERROR - Missing kernel size argument." << nl;
               return 2;
            }
            kernelSize = String( argv[argi] ).toInt();
         }
         else
         {
            StdErr << "Unsupported option: " << option << nl;
            return 2;
         }
         continue;
      }

      srcFile = argv[argi];
      ++argi;
      if( argi < argc )
      {
         dstFile = argv[argi];
      }
      else
      {
         String ext = srcFile.getExt();
         if( ext.empty() )
         {
            StdErr << "ERROR - Cannot generate new filename without extension." << nl;
            return 2;
         }
         dstFile = srcFile.sub( 0, srcFile.size() - ext.size() - 1 );
         dstFile += "_df.";
         dstFile += ext;
      }
      StdErr << srcFile << " --(kernelSize=" << kernelSize << ")--> " << dstFile << " " << dim << nl;
      ok &= toDistanceField( srcFile, dstFile, dim, kernelSize );
      if( !ok )
      {
         StdErr << "ERROR - Failed to convert the distance field." << nl;
      }
   }

   return (ok ? 0 : 1);
}
