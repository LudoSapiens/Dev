/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Resource/BitmapManipulator.h>

USING_NAMESPACE

//------------------------------------------------------------------------------
//!
float
velvet( float cos )
{
   float c  = 0.25f;
   float c2 = cos*cos;
   float ep = c2/(1-c2);
   return c*(1.0f + 4.0f*exp(-ep));
}

//------------------------------------------------------------------------------
//!
float phongEp = 10.0;

float
phong( float cos )
{
   return CGM::pow( CGM::max( 0.0f, cos ), phongEp ) + 0.2;
}

//------------------------------------------------------------------------------
//!
bool 
toDBRDF( float (*dbrdf)( float ), const String& dstFile, uint size )
{
   RCP<Bitmap> dbrdfBmp = BitmapManipulator::toDBRDF( size, dbrdf );
   StdErr << "Saving to file: " << dstFile << "\n";
   dbrdfBmp->saveFile( dstFile );
   return true;
}

//------------------------------------------------------------------------------
//!
bool 
toDBRDF( const String& srcFile, const String& dstFile, uint size )
{
   StdErr << "Building brdf from " << srcFile << " of size " << size << "\n";
   RCP<Bitmap> dbrdf = BitmapManipulator::toDBRDF( size, srcFile );
   StdErr << "Saving to file: " << dstFile << "\n";
   dbrdf->saveFile( dstFile );
   return true;
}

//------------------------------------------------------------------------------
//!
bool 
toDBRDF( const String& anglesFile, const String& srcFile, const String& dstFile, uint size )
{
   StdErr << "Building brdf from " << srcFile << " of size " << size << "\n";
   RCP<Bitmap> dbrdf = BitmapManipulator::toDBRDF( size, anglesFile, srcFile );
   StdErr << "Saving to file: " << dstFile << "\n";
   dbrdf->saveFile( dstFile );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
toDBRDFs( const String& dstFile, uint size )
{
   RCP<Bitmap> dbrdfs = new Bitmap( Vec2i( size, 128 ), Bitmap::BYTE, 1 );
   
   uint y = 0;
   
   RCP<Bitmap> dbrdf;
   
   
   dbrdf = BitmapManipulator::toDBRDF( size, velvet );
   BitmapManipulator::copy( *dbrdf, *dbrdfs, Vec2i(0,y++) );
   
   for( uint i = 0; i < 10; ++i )
   {
      phongEp = i*10+1.0f;
      dbrdf   = BitmapManipulator::toDBRDF( size, phong );
      BitmapManipulator::copy( *dbrdf, *dbrdfs, Vec2i(0,y++) );
   }
   
   const char* names[] = {
      "c:\\resources\\brdf\\acrylic-blue.brdf",
      "c:\\resources\\brdf\\alum-bronze.binary",
      "c:\\resources\\brdf\\aluminium.binary",
      "c:\\resources\\brdf\\black-oxidized-steel.binary",
      "c:\\resources\\brdf\\blue-metallic-paint.binary",
      "c:\\resources\\brdf\\blue-rubber.brdf",
      "c:\\resources\\brdf\\concrete.brdf",
      "c:\\resources\\brdf\\metallic-blue.binary",
      "c:\\resources\\brdf\\metallic-gold.binary",
      "c:\\resources\\brdf\\nickel.binary",
      "c:\\resources\\brdf\\plastic.brdf",
      "c:\\resources\\brdf\\red-bball.brdf",
      "c:\\resources\\brdf\\red-oak-215.brdf",
      "c:\\resources\\brdf\\sugar.brdf",
      "c:\\resources\\brdf\\tungsten-carbide.brdf",
      ""
   };
   
   for( uint i = 0; names[i][0] > 0; ++i )
   {
      dbrdf = BitmapManipulator::toDBRDF( size, names[i] );
      BitmapManipulator::copy( *dbrdf, *dbrdfs, Vec2i(0,y++) );
   }
   
   
   dbrdfs->saveFile( dstFile );
}

//------------------------------------------------------------------------------
//!
int
main( const int argc, const char* argv[] )
{
   bool ok = true;

   String srcFile;
   String srcFile2;
   String dstFile;
   int size = 64;
   
   
   uint type = 0;
   
   
   for( int argi = 1; argi < argc; ++argi )
   {
      if( argv[argi][0] == '-' )
      {
         String option = argv[argi] + 1;
         
         if( option == "size" )
         {
            ++argi;
            if( argi == argc )
            {
               StdErr << "ERROR - Missing size argument." << nl;
               return 2;
            }
            size = String( argv[argi] ).toInt();
         }
         else if( option == "dst" )
         {
            ++argi;
            if( argi == argc )
            {
               StdErr << "ERROR - Missing dst argument." << nl;
               return 2;
            }
            dstFile = argv[argi];
         }
         else if( option == "matusik" )
         {
            ++argi;
            if( argi == argc )
            {
               StdErr << "ERROR - Missing src argument." << nl;
               return 2;
            }
            srcFile = argv[argi];
            type = 1;
         }
         else if( option == "velvet" )
         {
            type = 2;
         }
         else if( option == "phong" )
         {
            ++argi;
            if( argi == argc )
            {
               StdErr << "ERROR - Missing exponent argument." << nl;
               return 2;
            }
            phongEp = String( argv[argi] ).toFloat();
            type = 3;
         }
         else if( option == "dbrdf" )
         {
            type = 4;
         }
         else if( option == "curret" )
         {
            ++argi;
            if( argi == argc )
            {
               StdErr << "ERROR - Missing data argument." << nl;
               return 2;
            }
            srcFile = argv[argi];
            ++argi;
            if( argi == argc )
            {
               StdErr << "ERROR - Missing dbrdf argument." << nl;
               return 2;
            }
            srcFile2 = argv[argi];
            type = 5;
         }
         else
         {
            StdErr << "Unsupported option: " << option << nl;
            return 2;
         }
      }
   }
   
   StdErr << "TYPE: " << type << "\n";
      
   switch( type )
   {
      case 0:
      {
         // Nothing.
      } break;
      case 1:
      {
         ok &= toDBRDF( srcFile, dstFile, size );
      } break;
      case 2:
      {
         ok &= toDBRDF( velvet, dstFile, size );
      } break;
      case 3:
      {
         ok &= toDBRDF( phong, dstFile, size );
      } break;
      case 4:
      {
         ok &= toDBRDFs( dstFile, size );
      } break;
      case 5:
      {
         ok &= toDBRDF( srcFile, srcFile2, dstFile, size );
      } break;
   }
   
   
   if( !ok )
   {
      StdErr << "ERROR - Failed to convert to dbrdf." << nl;
   }

   return (ok ? 0 : 1);
}
