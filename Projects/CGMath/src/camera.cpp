/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/Dbg/UnitTest.h>

#include <CGMath/CGMath.h>
#include <CGMath/Ref.h>

USING_NAMESPACE

inline String toStr( const Mat4f& mat )
{
   return String().format("[ %7.3f %7.3f %7.3f %7.3f ]\n"
                          "[ %7.3f %7.3f %7.3f %7.3f ]\n"
                          "[ %7.3f %7.3f %7.3f %7.3f ]\n"
                          "[ %7.3f %7.3f %7.3f %7.3f ]",
                          mat(0, 0), mat(0, 1), mat(0, 2), mat(0, 3),
                          mat(1, 0), mat(1, 1), mat(1, 2), mat(1, 3),
                          mat(2, 0), mat(2, 1), mat(2, 2), mat(2, 3),
                          mat(3, 0), mat(3, 1), mat(3, 2), mat(3, 3));
}

inline String toStr( const Vec4f& vec )
{
   return String().format("[ %7.3f %7.3f %7.3f %7.3f ]",
                          vec(0), vec(1), vec(2), vec(3));
}

/*==============================================================================
  CLASS Camera
==============================================================================*/
//! Reproduces a camera similar to Plasma's in order to test parallax shearing.
class Camera
{
public:

   /*----- methods -----*/
   Camera();

   Mat4f projectionMatrix() const;

   // From Camera.
   Reff           _referential; // From Entity.
   float          _focalLength;
   float          _fovy;
   float          _back;
   float          _front;
   //float          _scale;
   float          _shearX;
   float          _shearY;
   // From Viewport.
   float          _aspect;
   Vec2f          _size;

private:
}; //class Camera

//------------------------------------------------------------------------------
//!
Camera::Camera():
   // Camera.
   _referential( Reff::identity() ),
   _focalLength( 2.0f ),
   _fovy( 90.0f ),
   _back( 11.0f ),
   _front( 1.0f ),
   //_scale( 10.0f ),
   _shearX( 0.0f ),
   _shearY( 0.0f ),
   // Viewport.
   _aspect( 1.0f ),
   _size(2.0f)
{
}

//------------------------------------------------------------------------------
//!
Mat4f
Camera::projectionMatrix() const
{
   float b  = _back;
   float f  = _front;
   float bf = b - f;

   //if( _camera->projection() == Camera::PERSPECTIVE )
   {
      float sy  = 1.0f / tan( CGM::degToRad( _fovy * 0.5f ) );
      float sx  = _aspect * _size.y * sy / _size.x;
      float sz  = -( b + f ) / bf;
      float tz  = -2.0f * b * f / bf;
      float shx = _shearX;
      float shy = _shearY;

      Mat4f proj = Mat4f(
         sx,   0.0f,  0.0f,  0.0f,
         0.0f,   sy,  0.0f,  0.0f,
         0.0f, 0.0f,    sz,    tz,
         0.0f, 0.0f, -1.0f,  0.0f
      );
      Mat4f shear = Mat4f(
         1,   0.0f,   shx,  0.0f,
         0.0f,   1,   shy,  0.0f,
         0.0f, 0.0f,    1,  0.0f,
         0.0f, 0.0f, 0.0f,   1
      );
      Mat4f full = Mat4f(
         sx,   0.0f,   shx,  0.0f,
         0.0f,   sy,   shy,  0.0f,
         0.0f, 0.0f,    sz,    tz,
         0.0f, 0.0f, -1.0f,  0.0f
      );

      StdErr << "bf=" << bf << " sx=" << sx << " sy=" << sy << " sz=" << sz << " tz=" << tz << nl;
      //StdErr << toStr(proj) << nl;
      //StdErr << " * " << nl;
      //StdErr << toStr(shear) << nl;
      //StdErr << " = " << nl;
      //StdErr << toStr(proj * shear) << nl;
      //StdErr << " versus " << nl;
      //StdErr << toStr(full) << nl;

      return full;
   }
}

//------------------------------------------------------------------------------
//!
void  cgmath_shear( Test::Result& res )
{
   StdErr << nl;

   Camera cam;

   float shx[] = { 0.0f, 1.0f };
   uint shxn = sizeof(shx)/sizeof(shx[0]);

   for( size_t shxi = 0; shxi < shxn; ++shxi )
   {
      cam._shearX = shx[shxi];
      StdErr << String("-") * 20 << nl;
      StdErr << "ShearX = " << cam._shearX << nl;
      StdErr << String("-") * 80 << nl;

      Mat4f mat = cam.projectionMatrix();
      StdErr << toStr(mat) << nl;

      float x[] = { -1.0f, 0.0f, 1.0f, 2.0f, 11.0f };
      float y[] = {  0.0f };
      float z[] = { -1.0f, -2.0f, -11.0f };
      Vec4f pos;
      Vec4f posProj;
      uint xn = sizeof(x)/sizeof(x[0]);
      uint yn = sizeof(y)/sizeof(y[0]);
      uint zn = sizeof(z)/sizeof(z[0]);
      pos.w = 1.0f;
      for( size_t zi = 0; zi < zn; ++zi )
      {
         pos.z = z[zi];
         StdErr << nl << "Z = " << pos.z << nl;
         for( size_t yi = 0; yi < yn; ++yi )
         {
            pos.y = y[yi];
            for( size_t xi = 0; xi < xn; ++xi )
            {
               pos.x = x[xi];
               posProj = mat * pos;
               StdErr << toStr(pos) << " --> " << toStr(posProj) << " --> " << toStr(posProj/posProj.w) << nl;
            }
         }
      }

      StdErr << String("-") * 80 << nl;
      StdErr << nl;
   }
}

//------------------------------------------------------------------------------
//!
void  init()
{
   Test::Collection& std = Test::standard();
   std.add( new Test::Function( "shear", "Tests shearing", cgmath_shear ) );

   //Test::Collection& spc = Test::special();
}

//------------------------------------------------------------------------------
//!
int
main( int argc, char** argv )
{
   init();
   return Test::main( argc, argv );
}
