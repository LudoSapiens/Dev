/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/Dbg/UnitTest.h>

#include <CGMath/Vec2.h>
#include <CGMath/Vec3.h>
#include <CGMath/Vec4.h>
#include <CGMath/Mat2.h>
#include <CGMath/Mat3.h>
#include <CGMath/Mat4.h>
#include <CGMath/Quat.h>

#include <Base/Dbg/DebugStream.h>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_test, "Test" );

UNNAMESPACE_END

inline const Vec3f&  asVec3f( const float* v )
{
   return reinterpret_cast<const Vec3f&>( *v );
}

//------------------------------------------------------------------------------
//!
void cgmath_vectors( Test::Result& res )
{
#if 0
   Vec2f vec1( 10, 20 );
   Vec2f vec2( 30, 13 );
   Vec3f vec3( 2, 24, 5 );
   Vec4f vec4( 10, 23, 11, 1 );

   Vec2f res = vec1 + vec2;

   DBG_MSG( os_test, res );
#endif

   //Vec2f
   {
      Vec2f v1, v2, v3, v4;

      v1 = Vec2f(3, 4);
      TEST_ADD( res, v1(0) == 3 );
      TEST_ADD( res, v1(1) == 4 );
      TEST_ADD( res, v1(1,0) == Vec2f(4, 3) ); // v1.yx
      TEST_ADD( res, v1(1,1) == Vec2f(4, 4) ); // v1.yy
      TEST_ADD( res, length(v1) == 5.0 );

      v2 = v1.getRescaled(10);
      TEST_ADD( res, length(v1) == 5.0 );
      TEST_ADD( res, length(v2) == 10.0 );

      v1.rescale(10);
      TEST_ADD( res, length(v1) == 10.0 );

      v1.normalize();
      TEST_ADD( res, length(v1) == 1.0 );

      v1.rescale(4.99f);
      TEST_ADD( res, !equal(v1, Vec2f(3, 4), 0.0f) );
      TEST_ADD( res,  equal(v1, Vec2f(3, 4), 0.1f) );

      v1 = Vec2f(0, 3);
      v2 = Vec2f(1, 1);
      TEST_ADD( res, CGM::pseudoCross( v1, v2 ) == -3.0f );
      v3 = v2.projectOnto(v1);
      TEST_ADD( res, equal(v3, Vec2f(0, 1)) );
      v1 = Vec2f(3, 5);
      v2 = Vec2f(1, 7);
      v2.decompose(v1, v3, v4);
      TEST_ADD( res, !equal(v1.dot(v3), 0.0f) );
      TEST_ADD( res,  equal(v1.dot(v4), 0.0f) );
      TEST_ADD( res, !equal(v2, v3) );
      TEST_ADD( res, !equal(v2, v4) );
      TEST_ADD( res,  equal(v2, v3+v4) );

      v1 = Vec2f(1, 2);
      TEST_ADD( res, !equal( v1.length(), 1.0f ) );
      v1.maxLength( 1.0f );
      TEST_ADD( res, equal( v1.length(), 1.0f ) );
      TEST_ADD( res, equal( v1, Vec2f(1, 2).normalize() ) );

      v1 = Vec2f(1,  2);
      v2 = Vec2f(3, -1);
      TEST_ADD( res, equal( v1.max(v2), Vec2f(3, 2) ) );
      TEST_ADD( res, equal( v1.min(v2), Vec2f(1,-1) ) );
      v1.clampMax( 1.5f );
      TEST_ADD( res, equal( v1, Vec2f(1.0f, 1.5f) ) );
      v1.clampMin( 1.5f );
      TEST_ADD( res, equal( v1, Vec2f(1.5f, 1.5f) ) );

      TEST_ADD( res, equal( CGM::abs(Vec2f(-1.0f, -2.0f)), Vec2f(1.0f, 2.0f) ) );

      TEST_ADD( res, CGM::floor(Vec2f(-1.2f, 3.99f)) == Vec2f(-2.0f, 3.0f) );
      TEST_ADD( res, CGM::ceil (Vec2f(-1.2f, 3.99f)) == Vec2f(-1.0f, 4.0f) );
      TEST_ADD( res, CGM::round(Vec2f(-1.2f, 3.99f)) == Vec2f(-1.0f, 4.0f) );

      TEST_ADD( res, Vec2f( 3.0f, 2.0f).getAxisAlignedDirection() == Vec2f( 1.0f, 0.0f) );
      TEST_ADD( res, Vec2f( 3.0f,-2.0f).getAxisAlignedDirection() == Vec2f( 1.0f, 0.0f) );
      TEST_ADD( res, Vec2f(-3.0f, 2.0f).getAxisAlignedDirection() == Vec2f(-1.0f, 0.0f) );
      TEST_ADD( res, Vec2f(-3.0f,-2.0f).getAxisAlignedDirection() == Vec2f(-1.0f, 0.0f) );
      TEST_ADD( res, Vec2f( 2.0f, 3.0f).getAxisAlignedDirection() == Vec2f( 0.0f, 1.0f) );
      TEST_ADD( res, Vec2f(-2.0f, 3.0f).getAxisAlignedDirection() == Vec2f( 0.0f, 1.0f) );
      TEST_ADD( res, Vec2f( 2.0f,-3.0f).getAxisAlignedDirection() == Vec2f( 0.0f,-1.0f) );
      TEST_ADD( res, Vec2f(-2.0f,-3.0f).getAxisAlignedDirection() == Vec2f( 0.0f,-1.0f) );

      float val[] = { 1.0f, 2.0f, 3.0f, 4.0f };
      TEST_ADD( res, Vec2f::as(val+0) == Vec2f(1.0f, 2.0f) );
      TEST_ADD( res, Vec2f::as(val+1) == Vec2f(2.0f, 3.0f) );
      TEST_ADD( res, Vec2f::as(val+2) == Vec2f(3.0f, 4.0f) );
   }

   //Vec3f
   {
      Vec3f v1, v2, v3, v4;

      v1 = Vec3f(3, 0, 4);
      TEST_ADD( res, v1(0) == 3 );
      TEST_ADD( res, v1(1) == 0 );
      TEST_ADD( res, v1(2) == 4 );
      TEST_ADD( res, v1(2, 0) == Vec2f(4, 3) );
      TEST_ADD( res, v1(2, 1, 0) == Vec3f(4, 0, 3) );
      TEST_ADD( res, length(v1) == 5.0 );

      v2 = v1.getRescaled(10);
      TEST_ADD( res, length(v1) == 5.0 );
      TEST_ADD( res, length(v2) == 10.0 );

      v1.rescale(10);
      TEST_ADD( res, length(v1) == 10.0 );

      v1.normalize();
      TEST_ADD( res, length(v1) == 1.0 );

      v1.rescale(4.99f);
      TEST_ADD( res, !equal(v1, Vec3f(3, 0, 4), 0.0f) );
      TEST_ADD( res,  equal(v1, Vec3f(3, 0, 4), 0.1f) );

      v1 = Vec3f(0, 3, 0);
      v2 = Vec3f(1, 1, 0);
      v3 = v1.cross( v2 );
      TEST_ADD( res, equal(v3, Vec3f(0, 0, -3)) );
      v3 = v2.projectOnto(v1);
      TEST_ADD( res, equal(v3, Vec3f(0, 1, 0)) );
      v1 = Vec3f(0, 3, 5);
      v2 = Vec3f(1, 1, 5);
      v2.decompose(v1, v3, v4);
      TEST_ADD( res, !equal(v1.dot(v3), 0.0f) );
      TEST_ADD( res,  equal(v1.dot(v4), 0.0f) );
      TEST_ADD( res, !equal(v2, v3) );
      TEST_ADD( res, !equal(v2, v4) );
      TEST_ADD( res,  equal(v2, v3+v4) );

      v1 = Vec3f(1, 2, 3);
      TEST_ADD( res, !equal( v1.length(), 1.0f ) );
      v1.maxLength( 1.0f );
      TEST_ADD( res, equal( v1.length(), 1.0f) );
      TEST_ADD( res, equal( v1, Vec3f(1, 2, 3).normalize() ) );

      v1 = Vec3f(1, 2, 3);
      v2 = Vec3f(3,-1, 5);
      TEST_ADD( res, equal( v1.max(v2), Vec3f(3, 2, 5) ) );
      TEST_ADD( res, equal( v1.min(v2), Vec3f(1,-1, 3) ) );
      v1.clampMax( 1.5f );
      TEST_ADD( res, equal( v1, Vec3f(1.0f, 1.5f, 1.5f) ) );
      v1.clampMin( 1.5f );
      TEST_ADD( res, equal( v1, Vec3f(1.5f, 1.5f, 1.5f) ) );

      TEST_ADD( res, equal( CGM::abs(Vec3f(-1.0f, -2.0f, -3.0f)), Vec3f(1.0f, 2.0f, 3.0f) ) );

      TEST_ADD( res, CGM::floor(Vec3f(-1.2f, 3.99f, 0.5f)) == Vec3f(-2.0f, 3.0f, 0.0f) );
      TEST_ADD( res, CGM::ceil (Vec3f(-1.2f, 3.99f, 0.5f)) == Vec3f(-1.0f, 4.0f, 1.0f) );
      TEST_ADD( res, CGM::round(Vec3f(-1.2f, 3.99f, 0.5f)) == Vec3f(-1.0f, 4.0f, 1.0f) );

      TEST_ADD( res, Vec3f( 4.0f, 3.0f, 3.0f).getAxisAlignedDirection() == Vec3f( 1.0f, 0.0f, 0.0f) );
      TEST_ADD( res, Vec3f( 4.0f, 3.0f,-2.0f).getAxisAlignedDirection() == Vec3f( 1.0f, 0.0f, 0.0f) );
      TEST_ADD( res, Vec3f( 4.0f,-3.0f, 2.0f).getAxisAlignedDirection() == Vec3f( 1.0f, 0.0f, 0.0f) );
      TEST_ADD( res, Vec3f(-4.0f, 3.0f, 3.0f).getAxisAlignedDirection() == Vec3f(-1.0f, 0.0f, 0.0f) );
      TEST_ADD( res, Vec3f(-4.0f, 3.0f,-2.0f).getAxisAlignedDirection() == Vec3f(-1.0f, 0.0f, 0.0f) );
      TEST_ADD( res, Vec3f(-4.0f,-3.0f, 2.0f).getAxisAlignedDirection() == Vec3f(-1.0f, 0.0f, 0.0f) );
      TEST_ADD( res, Vec3f( 3.0f, 4.0f, 3.0f).getAxisAlignedDirection() == Vec3f( 0.0f, 1.0f, 0.0f) );
      TEST_ADD( res, Vec3f( 3.0f, 4.0f,-2.0f).getAxisAlignedDirection() == Vec3f( 0.0f, 1.0f, 0.0f) );
      TEST_ADD( res, Vec3f(-3.0f, 4.0f, 2.0f).getAxisAlignedDirection() == Vec3f( 0.0f, 1.0f, 0.0f) );
      TEST_ADD( res, Vec3f( 3.0f,-4.0f, 3.0f).getAxisAlignedDirection() == Vec3f( 0.0f,-1.0f, 0.0f) );
      TEST_ADD( res, Vec3f( 3.0f,-4.0f,-2.0f).getAxisAlignedDirection() == Vec3f( 0.0f,-1.0f, 0.0f) );
      TEST_ADD( res, Vec3f(-3.0f,-4.0f, 2.0f).getAxisAlignedDirection() == Vec3f( 0.0f,-1.0f, 0.0f) );
      TEST_ADD( res, Vec3f( 3.0f, 3.0f, 4.0f).getAxisAlignedDirection() == Vec3f( 0.0f, 0.0f, 1.0f) );
      TEST_ADD( res, Vec3f( 3.0f,-2.0f, 4.0f).getAxisAlignedDirection() == Vec3f( 0.0f, 0.0f, 1.0f) );
      TEST_ADD( res, Vec3f(-3.0f, 2.0f, 4.0f).getAxisAlignedDirection() == Vec3f( 0.0f, 0.0f, 1.0f) );
      TEST_ADD( res, Vec3f( 3.0f, 3.0f,-4.0f).getAxisAlignedDirection() == Vec3f( 0.0f, 0.0f,-1.0f) );
      TEST_ADD( res, Vec3f( 3.0f,-2.0f,-4.0f).getAxisAlignedDirection() == Vec3f( 0.0f, 0.0f,-1.0f) );
      TEST_ADD( res, Vec3f(-3.0f, 2.0f,-4.0f).getAxisAlignedDirection() == Vec3f( 0.0f, 0.0f,-1.0f) );

      float val[] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f };
      TEST_ADD( res, Vec3f::as(val+0) == Vec3f(1.0f, 2.0f, 3.0f) );
      TEST_ADD( res, Vec3f::as(val+1) == Vec3f(2.0f, 3.0f, 4.0f) );
      TEST_ADD( res, Vec3f::as(val+2) == Vec3f(3.0f, 4.0f, 5.0f) );
      TEST_ADD( res, Vec3f::as(val+3) == Vec3f(4.0f, 5.0f, 6.0f) );
   }

   //Vec4f
   {
      Vec4f v1, v2, v3, v4;

      v1 = Vec4f(0, 4, 3, 0);
      TEST_ADD( res, v1(0) == 0 );
      TEST_ADD( res, v1(1) == 4 );
      TEST_ADD( res, v1(2) == 3 );
      TEST_ADD( res, v1(3) == 0 );
      TEST_ADD( res, v1(2, 1) == Vec2f(3, 4) );
      TEST_ADD( res, v1(1, 2, 1) == Vec3f(4, 3, 4) );
      TEST_ADD( res, v1(2, 3, 0, 1) == Vec4f(3, 0, 0, 4) );
      TEST_ADD( res, length(v1) == 5.0 );

      v2 = v1.getRescaled(10);
      TEST_ADD( res, length(v1) == 5.0 );
      TEST_ADD( res, length(v2) == 10.0 );

      v1.rescale(10);
      TEST_ADD( res, length(v1) == 10.0 );

      v1.normalize();
      TEST_ADD( res, length(v1) == 1.0 );

      v1.rescale(4.99f);
      TEST_ADD( res, !equal(v1, Vec4f(0, 4, 3, 0), 0.0f) );
      TEST_ADD( res,  equal(v1, Vec4f(0, 4, 3, 0), 0.1f) );

      v1 = Vec4f(0, 3, 0, 0);
      v2 = Vec4f(1, 1, 0, 0);
      v3 = v2.projectOnto(v1);
      TEST_ADD( res, equal(v3, Vec4f(0, 1, 0, 0)) );
      v1 = Vec4f(0, 3, 5, 0);
      v2 = Vec4f(1, 1, 5, 0);
      v2.decompose(v1, v3, v4);
      TEST_ADD( res, !equal(v1.dot(v3), 0.0f) );
      TEST_ADD( res,  equal(v1.dot(v4), 0.0f) );
      TEST_ADD( res, !equal(v2, v3) );
      TEST_ADD( res, !equal(v2, v4) );
      TEST_ADD( res,  equal(v2, v3 + v4) );

      v1 = Vec4f(1, 2, 3, 4);
      TEST_ADD( res, !equal(v1.length(), 1.0f) );
      v1.maxLength( 1.0f );
      TEST_ADD( res, equal(v1.length(), 1.0f) );
      TEST_ADD( res, equal(v1, Vec4f(1, 2, 3, 4).normalize()) );

      v1 = Vec4f(1, 2, 3, 4);
      v2 = Vec4f(3,-1, 5, 0);
      TEST_ADD( res, equal( v1.max(v2), Vec4f(3, 2, 5, 4) ) );
      TEST_ADD( res, equal( v1.min(v2), Vec4f(1,-1, 3, 0) ) );
      v1.clampMax( 1.5f );
      TEST_ADD( res, equal( v1, Vec4f(1.0f, 1.5f, 1.5f, 1.5f) ) );
      v1.clampMin( 1.5f );
      TEST_ADD( res, equal( v1, Vec4f(1.5f, 1.5f, 1.5f, 1.5f) ) );

      TEST_ADD( res, equal( CGM::abs(Vec4f(-1.0f, -2.0f, -3.0f, -4.0f)), Vec4f(1.0f, 2.0f, 3.0f, 4.0f) ) );

      TEST_ADD( res, CGM::floor(Vec4f(-1.2f, 3.99f, 0.5f, -0.5f)) == Vec4f(-2.0f, 3.0f, 0.0f, -1.0f) );
      TEST_ADD( res, CGM::ceil (Vec4f(-1.2f, 3.99f, 0.5f, -0.5f)) == Vec4f(-1.0f, 4.0f, 1.0f,  0.0f) );
      TEST_ADD( res, CGM::round(Vec4f(-1.2f, 3.99f, 0.5f, -0.5f)) == Vec4f(-1.0f, 4.0f, 1.0f, -1.0f) );

      float val[] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f };
      TEST_ADD( res, Vec4f::as(val+0) == Vec4f(1.0f, 2.0f, 3.0f, 4.0f) );
      TEST_ADD( res, Vec4f::as(val+1) == Vec4f(2.0f, 3.0f, 4.0f, 5.0f) );
      TEST_ADD( res, Vec4f::as(val+2) == Vec4f(3.0f, 4.0f, 5.0f, 6.0f) );
      TEST_ADD( res, Vec4f::as(val+3) == Vec4f(4.0f, 5.0f, 6.0f, 7.0f) );
      TEST_ADD( res, Vec4f::as(val+4) == Vec4f(5.0f, 6.0f, 7.0f, 8.0f) );
   }
}

//------------------------------------------------------------------------------
//!
void cgmath_matrices( Test::Result& res )
{
   Mat2f mat1 = Mat2f::identity();
   Mat3f mat2 = Mat3f::identity();
   Mat4f mat3 = Mat4f::identity();

   DBG_MSG( os_test, mat3 );
   bool ok;

   ok = true;
   for( uint j = 0; j < 4; ++j )
   {
      for( uint i = 0; i < 4; ++i )
      {
         ok &= (i == j) ? (mat3(i, j) == 1.0f) : (mat3(i, j) == 0.0f);
      }
   }
   TEST_ADD( res, ok );

   ok = true;
   for( uint j = 0; j < 4; ++j )
   {
      for( uint i = 0; i < 4; ++i )
      {
         ok &= (i == j) ? (mat3(j*4 + i) == 1.0f) : (mat3(j*4 + i) == 0.0f);
      }
   }
   TEST_ADD( res, ok );

   Mat3f m1(
      CGM::cos( 2.0f ), -CGM::sin( 2.0f ), 4.0f,
      CGM::sin( 2.0f ),  CGM::cos( 2.0f ), 3.0f,
      0.0f, 0.0f, 1.0f
   );

   Mat3f m2 = m1.inverse();

   Mat3f m3 = m1*m2;

   Mat3f m4 = m1;
   m4.inverse();
   Mat3f m5 = m1*m4;

   DBG_MSG( os_test, m1 );
   DBG_MSG( os_test, m3 );
   DBG_MSG( os_test, m5 );

   // Why isn't m3 identity?

   ok = true;
   for( uint j = 0; j < 3; ++j )
   {
      for( uint i = 0; i < 3; ++i )
      {
         if( i == j )
         {
            ok &= CGM::equal( m5(i, j), 1.0f );
         }
         else
         {
            ok &= CGM::equal( m5(i, j), 0.0f );
         }
      }
   }
   TEST_ADD( res, ok );

   // Mat2
   {
      Mat2f m = Mat2f::identity();
      TEST_ADD( res, m(0,0) == 1.0f );
      TEST_ADD( res, m(0,1) == 0.0f );
      TEST_ADD( res, m(1,0) == 0.0f );
      TEST_ADD( res, m(1,1) == 1.0f );
      m = Mat2f( 1, 2,
                 3, 4 );
      TEST_ADD( res, m(0,0) == 1.0f );
      TEST_ADD( res, m(0,1) == 2.0f );
      TEST_ADD( res, m(1,0) == 3.0f );
      TEST_ADD( res, m(1,1) == 4.0f );
      TEST_ADD( res, m.row(0) == Vec2f(1, 2) );
      TEST_ADD( res, m.row(1) == Vec2f(3, 4) );
      TEST_ADD( res, m.col(0) == Vec2f(1, 3) );
      TEST_ADD( res, m.col(1) == Vec2f(2, 4) );

      m.transpose();
      TEST_ADD( res, m(0,0) == 1.0f );
      TEST_ADD( res, m(0,1) == 3.0f );
      TEST_ADD( res, m(1,0) == 2.0f );
      TEST_ADD( res, m(1,1) == 4.0f );
      TEST_ADD( res, m.row(0) == Vec2f(1, 3) );
      TEST_ADD( res, m.row(1) == Vec2f(2, 4) );
      TEST_ADD( res, m.col(0) == Vec2f(1, 2) );
      TEST_ADD( res, m.col(1) == Vec2f(3, 4) );

      m.row( 1, Vec2f(10,20) );
      m.col( 1, Vec2f(30,40) );
      TEST_ADD( res, m.row(0) == Vec2f( 1,30) );
      TEST_ADD( res, m.row(1) == Vec2f(10,40) );
   }

   // Mat3
   {
      Mat3f m = Mat3f::identity();
      TEST_ADD( res, m(0,0) == 1.0f );
      TEST_ADD( res, m(0,1) == 0.0f );
      TEST_ADD( res, m(0,2) == 0.0f );
      TEST_ADD( res, m(1,0) == 0.0f );
      TEST_ADD( res, m(1,1) == 1.0f );
      TEST_ADD( res, m(1,2) == 0.0f );
      TEST_ADD( res, m(2,0) == 0.0f );
      TEST_ADD( res, m(2,1) == 0.0f );
      TEST_ADD( res, m(2,2) == 1.0f );
      m = Mat3f( 1, 2, 3,
                 4, 5, 6,
                 7, 8, 9 );
      TEST_ADD( res, m(0,0) == 1.0f );
      TEST_ADD( res, m(0,1) == 2.0f );
      TEST_ADD( res, m(0,2) == 3.0f );
      TEST_ADD( res, m(1,0) == 4.0f );
      TEST_ADD( res, m(1,1) == 5.0f );
      TEST_ADD( res, m(1,2) == 6.0f );
      TEST_ADD( res, m(2,0) == 7.0f );
      TEST_ADD( res, m(2,1) == 8.0f );
      TEST_ADD( res, m(2,2) == 9.0f );
      TEST_ADD( res, m.row(0) == Vec3f(1, 2, 3) );
      TEST_ADD( res, m.row(1) == Vec3f(4, 5, 6) );
      TEST_ADD( res, m.row(2) == Vec3f(7, 8, 9) );
      TEST_ADD( res, m.col(0) == Vec3f(1, 4, 7) );
      TEST_ADD( res, m.col(1) == Vec3f(2, 5, 8) );
      TEST_ADD( res, m.col(2) == Vec3f(3, 6, 9) );

      m.transpose();
      TEST_ADD( res, m(0,0) == 1.0f );
      TEST_ADD( res, m(0,1) == 4.0f );
      TEST_ADD( res, m(0,2) == 7.0f );
      TEST_ADD( res, m(1,0) == 2.0f );
      TEST_ADD( res, m(1,1) == 5.0f );
      TEST_ADD( res, m(1,2) == 8.0f );
      TEST_ADD( res, m(2,0) == 3.0f );
      TEST_ADD( res, m(2,1) == 6.0f );
      TEST_ADD( res, m(2,2) == 9.0f );
      TEST_ADD( res, m.row(0) == Vec3f(1, 4, 7) );
      TEST_ADD( res, m.row(1) == Vec3f(2, 5, 8) );
      TEST_ADD( res, m.row(2) == Vec3f(3, 6, 9) );
      TEST_ADD( res, m.col(0) == Vec3f(1, 2, 3) );
      TEST_ADD( res, m.col(1) == Vec3f(4, 5, 6) );
      TEST_ADD( res, m.col(2) == Vec3f(7, 8, 9) );

      m.row( 1, Vec3f(10,20,30) );
      m.col( 2, Vec3f(40,50,60) );
      TEST_ADD( res, m.row(0) == Vec3f( 1, 4,40) );
      TEST_ADD( res, m.row(1) == Vec3f(10,20,50) );
      TEST_ADD( res, m.row(2) == Vec3f( 3, 6,60) );

      m = Mat3f( 2, 0, 4,
                 0, 3, 5,
                 0, 0, 1 );
      TEST_ADD( res, (m*Vec2f(0.0f, 0.0f)) == Vec2f(4.0f, 5.0f) );
      TEST_ADD( res, (m*Vec2f(1.0f, 1.0f)) == Vec2f(6.0f, 8.0f) );
      TEST_ADD( res, (m^Vec2f(1.0f, 1.0f)) == Vec2f(2.0f, 3.0f) );
   }

   // Mat4
   {
      Mat4f m = Mat4f::identity();
      TEST_ADD( res, m(0,0) == 1.0f );
      TEST_ADD( res, m(0,1) == 0.0f );
      TEST_ADD( res, m(0,2) == 0.0f );
      TEST_ADD( res, m(0,3) == 0.0f );
      TEST_ADD( res, m(1,0) == 0.0f );
      TEST_ADD( res, m(1,1) == 1.0f );
      TEST_ADD( res, m(1,2) == 0.0f );
      TEST_ADD( res, m(1,3) == 0.0f );
      TEST_ADD( res, m(2,0) == 0.0f );
      TEST_ADD( res, m(2,1) == 0.0f );
      TEST_ADD( res, m(2,2) == 1.0f );
      TEST_ADD( res, m(2,3) == 0.0f );
      TEST_ADD( res, m(3,0) == 0.0f );
      TEST_ADD( res, m(3,1) == 0.0f );
      TEST_ADD( res, m(3,2) == 0.0f );
      TEST_ADD( res, m(3,3) == 1.0f );
      m = Mat4f( 1, 2, 3, 4,
                 5, 6, 7, 8,
                 9,10,11,12,
                13,14,15,16 );
      TEST_ADD( res, m(0,0) ==  1.0f );
      TEST_ADD( res, m(0,1) ==  2.0f );
      TEST_ADD( res, m(0,2) ==  3.0f );
      TEST_ADD( res, m(0,3) ==  4.0f );
      TEST_ADD( res, m(1,0) ==  5.0f );
      TEST_ADD( res, m(1,1) ==  6.0f );
      TEST_ADD( res, m(1,2) ==  7.0f );
      TEST_ADD( res, m(1,3) ==  8.0f );
      TEST_ADD( res, m(2,0) ==  9.0f );
      TEST_ADD( res, m(2,1) == 10.0f );
      TEST_ADD( res, m(2,2) == 11.0f );
      TEST_ADD( res, m(2,3) == 12.0f );
      TEST_ADD( res, m(3,0) == 13.0f );
      TEST_ADD( res, m(3,1) == 14.0f );
      TEST_ADD( res, m(3,2) == 15.0f );
      TEST_ADD( res, m(3,3) == 16.0f );
      TEST_ADD( res, m.row(0) == Vec4f( 1, 2, 3, 4) );
      TEST_ADD( res, m.row(1) == Vec4f( 5, 6, 7, 8) );
      TEST_ADD( res, m.row(2) == Vec4f( 9,10,11,12) );
      TEST_ADD( res, m.row(3) == Vec4f(13,14,15,16) );
      TEST_ADD( res, m.col(0) == Vec4f( 1, 5, 9,13) );
      TEST_ADD( res, m.col(1) == Vec4f( 2, 6,10,14) );
      TEST_ADD( res, m.col(2) == Vec4f( 3, 7,11,15) );
      TEST_ADD( res, m.col(3) == Vec4f( 4, 8,12,16) );

      m.transpose();
      TEST_ADD( res, m(0,0) ==  1.0f );
      TEST_ADD( res, m(0,1) ==  5.0f );
      TEST_ADD( res, m(0,2) ==  9.0f );
      TEST_ADD( res, m(0,3) == 13.0f );
      TEST_ADD( res, m(1,0) ==  2.0f );
      TEST_ADD( res, m(1,1) ==  6.0f );
      TEST_ADD( res, m(1,2) == 10.0f );
      TEST_ADD( res, m(1,3) == 14.0f );
      TEST_ADD( res, m(2,0) ==  3.0f );
      TEST_ADD( res, m(2,1) ==  7.0f );
      TEST_ADD( res, m(2,2) == 11.0f );
      TEST_ADD( res, m(2,3) == 15.0f );
      TEST_ADD( res, m(3,0) ==  4.0f );
      TEST_ADD( res, m(3,1) ==  8.0f );
      TEST_ADD( res, m(3,2) == 12.0f );
      TEST_ADD( res, m(3,3) == 16.0f );
      TEST_ADD( res, m.row(0) == Vec4f( 1, 5, 9,13) );
      TEST_ADD( res, m.row(1) == Vec4f( 2, 6,10,14) );
      TEST_ADD( res, m.row(2) == Vec4f( 3, 7,11,15) );
      TEST_ADD( res, m.row(3) == Vec4f( 4, 8,12,16) );
      TEST_ADD( res, m.col(0) == Vec4f( 1, 2, 3, 4) );
      TEST_ADD( res, m.col(1) == Vec4f( 5, 6, 7, 8) );
      TEST_ADD( res, m.col(2) == Vec4f( 9,10,11,12) );
      TEST_ADD( res, m.col(3) == Vec4f(13,14,15,16) );

      m.row( 1, Vec4f(10,20,30,40) );
      m.col( 2, Vec4f(50,60,70,80) );
      TEST_ADD( res, m.row(0) == Vec4f( 1, 5,50,13) );
      TEST_ADD( res, m.row(1) == Vec4f(10,20,60,40) );
      TEST_ADD( res, m.row(2) == Vec4f( 3, 7,70,15) );
      TEST_ADD( res, m.row(3) == Vec4f( 4, 8,80,16) );
   }
}

void test( const Quatf& q )
{
   Vec3f euler = q.toEulerXYZ();
   // Snap angles to 0', 90', 180'.
   Vec3f eulerSnapped = CGM::round( euler / CGConstf::pi_2() ) * CGConstf::pi_2();
   Quatf qe = Quatf::eulerXYZ( euler );
   Quatf qs = Quatf::eulerXYZ( eulerSnapped );

   StdErr << "q=" << q << nl;
   StdErr << "q axis: " << q.getAxisX() << " " << q.getAxisY() << " " << q.getAxisZ() << nl;
   StdErr << "euler=" << euler << nl;
   StdErr << "qe=" << qe << nl;
   StdErr << "qe axis: " << qe.getAxisX() << " " << qe.getAxisY() << " " << qe.getAxisZ() << nl;
   StdErr << "eulerSnapped=" << eulerSnapped << nl;
   StdErr << "qs=" << qs << nl;
   StdErr << "qs axis: " << qs.getAxisX() << " " << qs.getAxisY() << " " << qs.getAxisZ() << nl;
   euler = qs.toEulerXYZ();
   StdErr << "eulerf=" << euler << nl;
}

//------------------------------------------------------------------------------
//!
void cgmath_quaternions( Test::Result& res )
{
   Quatf q, qa, q1, q2, q3, q4;
   Vec3f v, x, y, z;
   float f;

   q = Quatf( 11.0f, 12.0f, 13.0f, 14.0f );
   TEST_ADD( res, q.x() == 11.0f );
   TEST_ADD( res, q.y() == 12.0f );
   TEST_ADD( res, q.z() == 13.0f );
   TEST_ADD( res, q.w() == 14.0f );
   TEST_ADD( res, q.ptr()[0] == 11.0f );
   TEST_ADD( res, q.ptr()[1] == 12.0f );
   TEST_ADD( res, q.ptr()[2] == 13.0f );
   TEST_ADD( res, q.ptr()[3] == 14.0f );

   q = Quatf::eulerZYX( Vec3f(0.9f, 0.6f, 1.1f) );
   v = q.toEulerZYX();
   TEST_ADD( res, CGM::equal(v.x, 0.9f) );
   TEST_ADD( res, CGM::equal(v.y, 0.6f) );
   TEST_ADD( res, CGM::equal(v.z, 1.1f) );

   q = Quatf::axisAngle( Vec3f(1.0f, 2.0f, -3.0f), CGConstf::pi() );
   q.toAxisCos( v, f );
   TEST_ADD( res, v.equal( Vec3f(1.0f, 2.0f, -3.0f).normalize() ) );
   TEST_ADD( res, CGM::equal(f, 0.0f) );

   q = Quatf::eulerXYZ( Vec3f(0.9f, 0.6f, 1.1f) ).getAxisAligned();
   v = q.toEulerXYZ();
   TEST_ADD( res, CGM::equal( v.x, 0.0f             ) );
   TEST_ADD( res, CGM::equal( v.y, 0.0f             ) );
   TEST_ADD( res, CGM::equal( v.z, CGConstf::pi_2() ) );

   // TwoVecs.
   q = Quatf::twoVecs( Vec3f(1,0,0), Vec3f(0,1,0) );
   TEST_ADD( res, CGM::equal( q*Vec3f(1,0,0), Vec3f(0,1,0) ) );
   q = Quatf::twoVecs( Vec3f(1,0,0), Vec3f(0,0,1) );
   TEST_ADD( res, CGM::equal( q*Vec3f(1,0,0), Vec3f(0,0,1) ) );
   q = Quatf::twoVecs( Vec3f(1,0,0), Vec3f(1,2,3) );
   TEST_ADD( res, CGM::equal( q*Vec3f(1,0,0), normalize(Vec3f(1,2,3)) ) );
   q = Quatf::twoVecs( Vec3f(1,0,0), Vec3f(1,0,0) );
   TEST_ADD( res, CGM::equal( q*Vec3f(1,0,0), Vec3f(1,0,0) ) );
   q = Quatf::twoVecs( Vec3f(1,0,0), Vec3f(-1,0,0) );
   TEST_ADD( res, CGM::equal( q*Vec3f(1,0,0), Vec3f(-1,0,0) ) );
   q = Quatf::twoVecs( Vec3f(1,2,3), Vec3f(8,3,4) );
   v = q * Vec3f(1,2,3);
   TEST_ADD( res, CGM::equal( normalize(v), normalize(Vec3f(8,3,4)) ) && CGM::equal( sqrLength(v), sqrLength(Vec3f(1,2,3))) );

   // TwoVecsNorm.
   q = Quatf::twoVecsNorm( Vec3f(1,0,0), Vec3f(0,1,0) );
   TEST_ADD( res, CGM::equal( q*Vec3f(1,0,0), Vec3f(0,1,0) ) );
   q = Quatf::twoVecsNorm( Vec3f(1,0,0), Vec3f(0,0,1) );
   TEST_ADD( res, CGM::equal( q*Vec3f(1,0,0), Vec3f(0,0,1) ) );
   q = Quatf::twoVecsNorm( Vec3f(1,0,0), normalize(Vec3f(1,2,3)) );
   TEST_ADD( res, CGM::equal( q*Vec3f(1,0,0), normalize(Vec3f(1,2,3)) ) );
   q = Quatf::twoVecsNorm( Vec3f(1,0,0), Vec3f(1,0,0) );
   TEST_ADD( res, CGM::equal( q*Vec3f(1,0,0), Vec3f(1,0,0) ) );
   q = Quatf::twoVecsNorm( Vec3f(1,0,0), Vec3f(-1,0,0) );
   TEST_ADD( res, CGM::equal( q*Vec3f(1,0,0), Vec3f(-1,0,0) ) );
   q = Quatf::twoVecsNorm( normalize(Vec3f(1,2,3)), normalize(Vec3f(8,3,4)) );
   v = q * Vec3f(1,2,3);
   TEST_ADD( res, CGM::equal( normalize(v), normalize(Vec3f(8,3,4)) ) && CGM::equal( sqrLength(v), sqrLength(Vec3f(1,2,3))) );

   // TwoQuats.
   q1 = Quatf( 1, 2, 3, 4 );
   q2 = Quatf( 21, -5, 2, -1 );
   q3 = q2 * q1;
   q4 = Quatf::twoQuats( q1, q3 );
   TEST_ADD( res, CGM::equal( q2.getNormalized(), q4.getNormalized() ) );

   // Quat::as() conversion.
   float val[] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f };
   TEST_ADD( res, Quatf::as(val+0) == Quatf(1.0f, 2.0f, 3.0f, 4.0f) );
   TEST_ADD( res, Quatf::as(val+1) == Quatf(2.0f, 3.0f, 4.0f, 5.0f) );
   TEST_ADD( res, Quatf::as(val+2) == Quatf(3.0f, 4.0f, 5.0f, 6.0f) );
   TEST_ADD( res, Quatf::as(val+3) == Quatf(4.0f, 5.0f, 6.0f, 7.0f) );
   TEST_ADD( res, Quatf::as(val+4) == Quatf(5.0f, 6.0f, 7.0f, 8.0f) );

   // AxisAligned.
   q  = Quatf( -0.0815561f, 0.785528f, 0.106033f, 0.604195f );
   qa = q.getAxisAligned();
   qa.getAxes( x, y, z );
   TEST_ADD( res, CGM::equal( x, Vec3f(0,0,-1) ) && CGM::equal( y, Vec3f(0,1,0) ) );

   q  = Quatf( -0.0800674f, 0.793889f, 0.107162f, 0.593166f );
   qa = q.getAxisAligned();
   qa.getAxes( x, y, z );
   TEST_ADD( res, CGM::equal( x, Vec3f(0,0,-1) ) && CGM::equal( y, Vec3f(0,1,0) ) );

   // Test Euler conversions.
   float err_q = CGM::pow( 2.0f, -16.0f );

   float rxt[] = { 0.0f, 0.1f, 0.2f, 0.24f, 0.25f, 0.26f, 0.49f, 0.50f, 0.51f, 0.75f, 0.99f, 1.0f, -0.1f, -0.2f, -0.24f, -0.25f, -0.26f, -0.49f, -0.50f, -0.51f, -0.75f, -0.99f, -1.0f };
   float ryt[] = { 0.0f, 0.1f, 0.2f, 0.24f, 0.25f, 0.26f, 0.49f, 0.50f, 0.51f, 0.75f, 0.99f, 1.0f, -0.1f, -0.2f, -0.24f, -0.25f, -0.26f, -0.49f, -0.50f, -0.51f, -0.75f, -0.99f, -1.0f };
   float rzt[] = { 0.0f, 0.1f, 0.2f, 0.24f, 0.25f, 0.26f, 0.49f, 0.50f, 0.51f, 0.75f, 0.99f, 1.0f, -0.1f, -0.2f, -0.24f, -0.25f, -0.26f, -0.49f, -0.50f, -0.51f, -0.75f, -0.99f, -1.0f };
   for( uint zi = 0; zi < sizeof(rzt)/sizeof(rzt[0]); ++zi )
   {
      float rz = rzt[zi];
      Quatf qz = Quatf::axisCir( Vec3f(0,0,1), rz );
      for( uint yi = 0; yi < sizeof(ryt)/sizeof(ryt[0]); ++yi )
      {
         float ry = ryt[yi];
         Quatf qy = Quatf::axisCir( Vec3f(0,1,0), ry );
         for( uint xi = 0; xi < sizeof(rxt)/sizeof(rxt[0]); ++xi )
         {
            float rx = rxt[xi];
            Quatf qx = Quatf::axisCir( Vec3f(1,0,0), rx );

            Vec3f rxyz   = CGM::cirToRad( Vec3f(rx, ry, rz) );
            //StdErr << "rx=" << rx << " ry=" << ry << " rz=" << rz << " " << rxyz << nl;

            Quatf q_xyz  = qx * qy * qz;
            Quatf q_xzy  = qx * qz * qy;
            Quatf q_yxz  = qy * qx * qz;
            Quatf q_yzx  = qy * qz * qx;
            Quatf q_zxy  = qz * qx * qy;
            Quatf q_zyx  = qz * qy * qx;
            Quatf qe_xyz = Quatf::eulerXYZ( rxyz );
            Quatf qe_xzy = Quatf::eulerXZY( rxyz );
            Quatf qe_yxz = Quatf::eulerYXZ( rxyz );
            Quatf qe_yzx = Quatf::eulerYZX( rxyz );
            Quatf qe_zxy = Quatf::eulerZXY( rxyz );
            Quatf qe_zyx = Quatf::eulerZYX( rxyz );
            TEST_ADD( res, CGM::equal( q_xyz, qe_xyz ) );
            TEST_ADD( res, CGM::equal( q_xzy, qe_xzy ) );
            TEST_ADD( res, CGM::equal( q_yxz, qe_yxz ) );
            TEST_ADD( res, CGM::equal( q_yzx, qe_yzx ) );
            TEST_ADD( res, CGM::equal( q_zxy, qe_zxy ) );
            TEST_ADD( res, CGM::equal( q_zyx, qe_zyx ) );
            //StdErr << "XYZ: " << q_xyz << " vs. " << qe_xyz << nl;
            //StdErr << "XZY: " << q_xzy << " vs. " << qe_xzy << nl;
            //StdErr << "YXZ: " << q_yxz << " vs. " << qe_yxz << nl;
            //StdErr << "YZX: " << q_yzx << " vs. " << qe_yzx << nl;
            //StdErr << "ZXY: " << q_zxy << " vs. " << qe_zxy << nl;
            //StdErr << "ZYX: " << q_zyx << " vs. " << qe_zyx << nl;

            Vec3f re_xyz = qe_xyz.toEulerXYZ();
            Vec3f re_xzy = qe_xzy.toEulerXZY();
            Vec3f re_yxz = qe_yxz.toEulerYXZ();
            Vec3f re_yzx = qe_yzx.toEulerYZX();
            Vec3f re_zxy = qe_zxy.toEulerZXY();
            Vec3f re_zyx = qe_zyx.toEulerZYX();
                  qe_xyz = Quatf::eulerXYZ( re_xyz ).normalize();
                  qe_xzy = Quatf::eulerXZY( re_xzy ).normalize();
                  qe_yxz = Quatf::eulerYXZ( re_yxz ).normalize();
                  qe_yzx = Quatf::eulerYZX( re_yzx ).normalize();
                  qe_zxy = Quatf::eulerZXY( re_zxy ).normalize();
                  qe_zyx = Quatf::eulerZYX( re_zyx ).normalize();
            TEST_ADD( res, CGM::equal( qe_xyz, q_xyz, err_q ) );
            TEST_ADD( res, CGM::equal( qe_xzy, q_xzy, err_q ) );
            TEST_ADD( res, CGM::equal( qe_yxz, q_yxz, err_q ) );
            TEST_ADD( res, CGM::equal( qe_yzx, q_yzx, err_q ) );
            TEST_ADD( res, CGM::equal( qe_zxy, q_zxy, err_q ) );
            TEST_ADD( res, CGM::equal( qe_zyx, q_zyx, err_q ) );
            //uint prec = 10;
            //StdErr << CGM::fixedPrec( rxyz, prec ) << nl;
            //StdErr << "toEulerXYZ() " << CGM::fixedPrec( re_xyz, prec ) << " " << CGM::equal( re_xyz, rxyz ) << "      " << CGM::equal( qe_xyz, q_xyz, err_q ) << " " << qe_xyz << " vs. " << q_xyz << nl;
            //StdErr << "toEulerXZY() " << CGM::fixedPrec( re_xzy, prec ) << " " << CGM::equal( re_xzy, rxyz ) << "      " << CGM::equal( qe_xzy, q_xzy, err_q ) << " " << qe_xzy << " vs. " << q_xzy << nl;
            //StdErr << "toEulerYXZ() " << CGM::fixedPrec( re_yxz, prec ) << " " << CGM::equal( re_yxz, rxyz ) << "      " << CGM::equal( qe_yxz, q_yxz, err_q ) << " " << qe_yxz << " vs. " << q_yxz << nl;
            //StdErr << "toEulerYZX() " << CGM::fixedPrec( re_yzx, prec ) << " " << CGM::equal( re_yzx, rxyz ) << "      " << CGM::equal( qe_yzx, q_yzx, err_q ) << " " << qe_yzx << " vs. " << q_yzx << nl;
            //StdErr << "toEulerZXY() " << CGM::fixedPrec( re_zxy, prec ) << " " << CGM::equal( re_zxy, rxyz ) << "      " << CGM::equal( qe_zxy, q_zxy, err_q ) << " " << qe_zxy << " vs. " << q_zxy << nl;
            //StdErr << "toEulerZYX() " << CGM::fixedPrec( re_zyx, prec ) << " " << CGM::equal( re_zyx, rxyz ) << "      " << CGM::equal( qe_zyx, q_zyx, err_q ) << " " << qe_zyx << " vs. " << q_zyx << nl;
         }
      }
   }
}

//------------------------------------------------------------------------------
//!
void cgmath_spherical_coords( Test::Result& res )
{
#define PRINT_V1_TP_V2_ERR() \
   //printf("(%+5.4g, %+5.4g, %+5.4g) --> theta: %5g, phi: %5g  -->  (%+5.4g, %+5.4g, %+5.4g)  (err=%g)\n", v1(0), v1(1), v1(2), theta1, phi1, v2(0), v2(1), v2(2), (v1-v2).length())

   float err = 1e-5f;
   Vec3f v1, v2;
   float theta1, phi1; //, theta2, phi2;
   v1 = Vec3f(1, 0, 0);
   v1.toThetaPhi(theta1, phi1);
   v2.fromThetaPhi(theta1, phi1);
   PRINT_V1_TP_V2_ERR();
   TEST_ADD( res, (v1-v2).length() < err );
   v1 = Vec3f(0, 1, 0);
   v1.toThetaPhi(theta1, phi1);
   v2.fromThetaPhi(theta1, phi1);
   PRINT_V1_TP_V2_ERR();
   TEST_ADD( res, (v1-v2).length() < err );
   v1 = Vec3f(0, 0, 1);
   v1.toThetaPhi(theta1, phi1);
   v2.fromThetaPhi(theta1, phi1);
   PRINT_V1_TP_V2_ERR();
   TEST_ADD( res, (v1-v2).length() < err );
   v1 = Vec3f(-1, 0, 0);
   v1.toThetaPhi(theta1, phi1);
   v2.fromThetaPhi(theta1, phi1);
   PRINT_V1_TP_V2_ERR();
   TEST_ADD( res, (v1-v2).length() < err );
   v1 = Vec3f(0, -1, 0);
   v1.toThetaPhi(theta1, phi1);
   v2.fromThetaPhi(theta1, phi1);
   PRINT_V1_TP_V2_ERR();
   TEST_ADD( res, (v1-v2).length() < err );
   v1 = Vec3f(0, 0, -1);
   v1.toThetaPhi(theta1, phi1);
   v2.fromThetaPhi(theta1, phi1);
   PRINT_V1_TP_V2_ERR();
   TEST_ADD( res, (v1-v2).length() < err );

   v1 = Vec3f(1, 1, 1);
   v1.toThetaPhi(theta1, phi1);
   v2.fromThetaPhi(theta1, phi1, v1.length());
   PRINT_V1_TP_V2_ERR();
   TEST_ADD( res, (v1-v2).length() < err );
}


//------------------------------------------------------------------------------
//!
void  init_vmq()
{
   Test::Collection& std = Test::standard();
   std.add( new Test::Function( "vec",  "Tests Vec2/Vec3/Vec4", cgmath_vectors     ) );
   std.add( new Test::Function( "mat",  "Tests Mat2/Mat3/Mat4", cgmath_matrices    ) );
   std.add( new Test::Function( "quat", "Tests quaternions",    cgmath_quaternions ) );
   std.add( new Test::Function( "spherical_coords", "Tests sphercial coordinate conversions",    cgmath_spherical_coords ) );
}
