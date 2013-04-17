/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Plasma.h>

#include <Fusion/Resource/Bitmap.h>
#include <Fusion/Resource/BitmapManipulator.h>

#include <CGMath/Math.h>

#include <Base/IO/FileDevice.h>
#include <Base/IO/TextStream.h>

USING_NAMESPACE

#define DIM_X 320
#define DIM_Y 480

/*==============================================================================
  CLASS TouchDataAnalyzer
==============================================================================*/
class TouchDataAnalyzer:
   public PlasmaApp
{
public:

   /*----- methods -----*/

   TouchDataAnalyzer( int argc, char* argv[] );

   virtual ~TouchDataAnalyzer();

   void  clearData();
   void  drawRect( int x, int y, int w, int h, float error );
   void  drawCirc( int x, int y, int r, float error );

   RCP<Bitmap>  dataToBitmap() const;

protected:

   /*----- data members -----*/
   struct Data
   {
      float     _error;
      uint32_t  _count;
   };

   union U96b
   {
      uchar*  _u8;
      float*  _f;
   };

   Data   _data[DIM_Y][DIM_X];
   float  _minError;
   float  _maxError;

   /*----- methods -----*/

   /* methods... */

private:
}; //class TouchDataAnalyzer

//------------------------------------------------------------------------------
//!
TouchDataAnalyzer::TouchDataAnalyzer( int argc, char* argv[] ):
   PlasmaApp( argc, argv )
{
   clearData();
   _minError =  CGConstf::infinity();
   _maxError = -CGConstf::infinity();
   for( int i = 1; i < argc; ++i )
   {
      const char* arg = argv[i];
      TextStream is( new FileDevice(arg, IODevice::MODE_READ) );
      for( TextStream::LineIterator iter = is.lines(); iter.isValid(); ++iter )
      {
         const String& line = *iter;
         //StdErr << "line: " << line << nl;
         if( !line.empty() && isdigit(line[0]) )
         {
            int tx, ty, dx, dy;
            float ep;
            if( sscanf(line.cstr(), "%d%d%d%d%f", &tx, &ty, &dx, &dy, &ep) == 5 )
            {
               //StdErr << ">>>" << tx << "," << ty << "," << dx << "," << dy << "," << ep << nl;
               float v = ep;

               _minError = CGM::min( _minError, v );
               _maxError = CGM::max( _maxError, v );
#if 0
               tx -= dx/2; ty -= dy/2;
               drawRect( tx, ty, dx, dy, ep );
#else
               drawCirc( tx, ty, 10, v );
#endif
            }
         }
      }
   }

   StdErr << "minErr=" << _minError << " maxErr=" << _maxError << nl;
   RCP<Bitmap> bmp = dataToBitmap();
   bmp->saveFile( "jph" );
   exit(0);
}

//------------------------------------------------------------------------------
//!
TouchDataAnalyzer::~TouchDataAnalyzer()
{
}

//------------------------------------------------------------------------------
//!
void
TouchDataAnalyzer::clearData()
{
   StdErr << sizeof(_data) << nl;
   memset( _data, 0, sizeof(_data) );
}

//------------------------------------------------------------------------------
//!
void
TouchDataAnalyzer::drawRect( int x, int y, int w, int h, float error )
{
   int xs = CGM::max( x  ,   0   );
   int xe = CGM::min( x+w, DIM_X );
   int ys = CGM::max( y  ,   0   );
   int ye = CGM::min( y+h, DIM_Y );

   for( y = ys; y < ye; ++y )
   {
      Data* curData = &(_data[y][xs]);
      for( x = xs; x < xe; ++x )
      {
         curData->_error += error;
         ++(curData->_count);
         ++curData;
      }
   }
}

//------------------------------------------------------------------------------
//!
void
TouchDataAnalyzer::drawCirc( int x, int y, int r, float error )
{
   int xs = CGM::max( x-r,   0   );
   int xe = CGM::min( x+r, DIM_X );
   int ys = CGM::max( y-r,   0   );
   int ye = CGM::min( y+r, DIM_Y );

   int r2 = r * r;
   for( int yi = ys; yi < ye; ++yi )
   {
      Data* curData = &(_data[yi][xs]);
      int y2 = (yi - y);
      y2 *= y2;
      for( int xi = xs; xi < xe; ++xi )
      {
         int x2 = (xi - x);
         x2 *= x2;
         if( (x2+y2) < r2 )
         {
            curData->_error += error;
            ++(curData->_count);
         }
         ++curData;
      }
   }
}

//------------------------------------------------------------------------------
//!
void  scalarToColor( float v, uchar* rgb )
{
   rgb[0] = (uchar)(v*255.0f + 0.5f);
   rgb[1] = rgb[0];
   rgb[2] = rgb[0];
}

//------------------------------------------------------------------------------
//!
void  scalarToBlendedColor( const float v, const Vec3f& a, const Vec3f& b, uchar* rgb )
{
   Vec3f tmp = CGM::linear( a, b-a, v );
   rgb[0] = (uchar)(tmp.x*255.0f + 0.5f);
   rgb[1] = (uchar)(tmp.y*255.0f + 0.5f);
   rgb[2] = (uchar)(tmp.z*255.0f + 0.5f);
}

//------------------------------------------------------------------------------
//!
RCP<Bitmap>
TouchDataAnalyzer::dataToBitmap() const
{
   Vec3f a( 0.0f, 0.0f, 1.0f );
   Vec3f b( 1.0f, 0.0f, 0.0f );
   RCP<Bitmap> bmp = new Bitmap( Vec2i(DIM_X, DIM_Y), Bitmap::BYTE, 3 );
   const Data* src = _data[0];
   uchar* dst      = bmp->pixels();
   for( uint32_t y = 0; y < DIM_Y; ++y )
   {
      for( uint32_t x = 0; x < DIM_X; ++x )
      {
         float f = src->_error/(_maxError*2.0f*src->_count) + 0.5f;
         //scalarToColor( src->_error/(_maxError*src->_count), dst );
         scalarToBlendedColor( f, a, b, dst );
         ++src;
         dst += bmp->pixelSize();
      }
   }
   return bmp;
}


//------------------------------------------------------------------------------
//!
int main( int argc, char** argv )
{
   RCP<TouchDataAnalyzer> app = new TouchDataAnalyzer( argc, argv );
   return app->run();
}
