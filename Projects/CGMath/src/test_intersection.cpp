/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/Dbg/UnitTest.h>

#include <CGMath/Dist.h>
#include <CGMath/Geom.h>
#include <CGMath/Random.h>
#include <CGMath/Ray.h>
#include <CGMath/RayCaster.h>

#include <Base/Util/Bits.h>

USING_NAMESPACE

//DBG_STREAM( os_test, "Test" );

//------------------------------------------------------------------------------
//!
void cgmath_rays( Test::Result& res )
{
   float t;
   Rayf ray( Vec3f(1.0f, 2.0f, 3.0f), Vec3f(2.0f, 4.0f, 6.0f) );
   TEST_ADD( res, ray.point(3.0f).equal( Vec3f(7.0f, 14.0f, 21.0f) ) );
   t = ray.computeT( Vec3f(7.0f, 14.0f, 21.0f) );
   TEST_ADD( res, CGM::equal( t, 3.0f ) );
   t = ray.computeT( Vec3f(1.0f, 2.0f, 3.0f) );
   TEST_ADD( res, CGM::equal( t, 0.0f ) );
   t = ray.computeT( Vec3f(-1.0f, -2.0f, -3.0f) );
   TEST_ADD( res, CGM::equal( t, -1.0f ) );
}

//------------------------------------------------------------------------------
//!
void cgmath_tri_pt_closest( Test::Result& res )
{
   Vec3f  A, B, C;  //triangle vertices
   Vec3f  pt;       //comparison point
   Vec3f  c;        //closest point
   float  d;        //distance (squared)

   //+Z over at 1
   A = Vec3f(-2, -2, +1);
   B = Vec3f(+2, -2, +1);
   C = Vec3f( 0, +2, +1);
   pt = Vec3f(0, 0, 0);
   CGM::triangle_point_closestPoint( A, B, C, pt, c );
   d = (pt-c).sqrLength();
   TEST_ADD( res, c.equal(Vec3f(0, 0, 1)) );
   TEST_ADD( res, CGM::equal(d, 1.0f) );
   pt = Vec3f(-10, -10, -3);
   CGM::triangle_point_closestPoint( A, B, C, pt, c );
   d = (pt-c).sqrLength();
   TEST_ADD( res, c.equal(A) );
   TEST_ADD( res, CGM::equal(d, (pt-A).sqrLength()) );
   pt = Vec3f(0, -10, +3);
   CGM::triangle_point_closestPoint( A, B, C, pt, c );
   d = (pt-c).sqrLength();
   TEST_ADD( res, c.equal(Vec3f(0, -2, +1)) );
   TEST_ADD( res, CGM::equal(d, (pt-Vec3f(0, -2, +1)).sqrLength()) );
   pt = Vec3f(+10, -10, -2);
   CGM::triangle_point_closestPoint( A, B, C, pt, c );
   d = (pt-c).sqrLength();
   TEST_ADD( res, c.equal(B) );
   TEST_ADD( res, CGM::equal(d, (pt-B).sqrLength()) );
   pt = Vec3f(-2, +0.5, +2);
   CGM::triangle_point_closestPoint( A, B, C, pt, c );
   d = (pt-c).sqrLength();
   TEST_ADD( res, c.equal(Vec3f(-1, 0, 1)) );
   TEST_ADD( res, CGM::equal(d, (pt-Vec3f(-1, 0, 1)).sqrLength()) );
   pt = Vec3f(+2, +0.5, +2);
   CGM::triangle_point_closestPoint( A, B, C, pt, c );
   d = (pt-c).sqrLength();
   TEST_ADD( res, c.equal(Vec3f(1, 0, 1)) );
   TEST_ADD( res, CGM::equal(d, (pt-Vec3f(1, 0, 1)).sqrLength()) );

   //45deg, then moved at origin
   A = Vec3f(3, 0, 0);
   B = Vec3f(0, 3, 0);
   C = Vec3f(0, 0, 3);
   pt = Vec3f(0, 0, 0);
   CGM::triangle_point_closestPoint( A, B, C, pt, c );
   d = (pt-c).sqrLength();
   TEST_ADD( res, c.equal(Vec3f(1, 1, 1)) );
   TEST_ADD( res, CGM::equal(d, 3.0f) );
   pt = Vec3f(3, 0, 0) + Vec3f(1, 1, 1).rescale(-2);
   CGM::triangle_point_closestPoint( A, B, C, pt, c );
   d = (pt-c).sqrLength();
   TEST_ADD( res, c.equal(Vec3f(3, 0, 0)) );
   TEST_ADD( res, CGM::equal(d, 4.0f) );
   pt = Vec3f(0, 4, 0);
   CGM::triangle_point_closestPoint( A, B, C, pt, c );
   d = (pt-c).sqrLength();
   TEST_ADD( res, c.equal(Vec3f(0, 3, 0)) );
   TEST_ADD( res, CGM::equal(d, 1.0f) );

   /**
   printf("Tri: (%g, %g, %g)  (%g, %g, %g)  (%g, %g, %g)\n",
          A(0), A(1), A(2),
          B(0), B(1), B(2),
          C(0), C(1), C(2));
   printf("Pt: (%g, %g, %g)\n",
          pt(0), pt(1), pt(2));
   printf("Closest: (%g, %g, %g)  Dist=%g\n",
          c(0), c(1), c(2), d);
   **/
}

//------------------------------------------------------------------------------
//!
void cgmath_ray_caster( Test::Result& res )
{
   Rayf ray;
   RayCaster::Hitf  hit;
   Vec3f a, b, c, d, e, f, g, h, o;
   float small = 0.0001f;
   Vec3f dx = Vec3f( 1.0f, 0.0f, 0.0f );
   Vec3f dy = Vec3f( 0.0f, 1.0f, 0.0f );

#define TEST_CCW_CW( a, b, c, v, t ) \
   hit._t = CGConstf::infinity(); \
   TEST_ADD( res, RayCaster::hit( ray, dx, dy, a, b, c, hit ) == v ); \
   if( v ) TEST_ADD( res, CGM::equal( hit._t, t ) && hit._backFacing == false ); \
   hit._t = CGConstf::infinity(); \
   TEST_ADD( res, RayCaster::hit( ray, dx, dy, b, c, a, hit ) == v ); \
   if( v ) TEST_ADD( res, CGM::equal( hit._t, t ) && hit._backFacing == false ); \
   hit._t = CGConstf::infinity(); \
   TEST_ADD( res, RayCaster::hit( ray, dx, dy, c, a, b, hit ) == v ); \
   if( v ) TEST_ADD( res, CGM::equal( hit._t, t ) && hit._backFacing == false ); \
   hit._t = CGConstf::infinity(); \
   TEST_ADD( res, RayCaster::hit( ray, dx, dy, b, a, c, hit ) == v ); \
   if( v ) TEST_ADD( res, CGM::equal( hit._t, t ) && hit._backFacing == true  ); \
   hit._t = CGConstf::infinity(); \
   TEST_ADD( res, RayCaster::hit( ray, dx, dy, a, c, b, hit ) == v ); \
   if( v ) TEST_ADD( res, CGM::equal( hit._t, t ) && hit._backFacing == true  ); \
   hit._t = CGConstf::infinity(); \
   TEST_ADD( res, RayCaster::hit( ray, dx, dy, c, b, a, hit ) == v ); \
   if( v ) TEST_ADD( res, CGM::equal( hit._t, t ) && hit._backFacing == true  ); \


#if 1
   a = Vec3f( -1.0f, -1.0f, 0.0f );
   b = Vec3f(  1.0f, -1.0f, 0.0f );
   c = Vec3f(  0.0f,  2.0f, 0.0f );

   ray.origin   ( Vec3f( 0.0f, 0.0f, 4.0f ) );
   ray.direction( Vec3f( 0.0f, 0.0f,-1.0f ) );
   TEST_CCW_CW( a, b, c, true, 4.0f );

   ray.direction( a - ray.origin() );
   TEST_CCW_CW( a, b, c, true , 1.0f );
   ray.direction( ray.direction() + Vec3f(-small,   0.0f, 0.0f) );
   TEST_CCW_CW( a, b, c, false, 1.0f );
   ray.direction( ray.direction() + Vec3f(+small, +small, 0.0f) );
   TEST_CCW_CW( a, b, c, false, 1.0f );
   ray.direction( ray.direction() + Vec3f(+small,   0.0f, 0.0f) );
   TEST_CCW_CW( a, b, c, true , 1.0f );

   ray.direction( b - ray.origin() );
   TEST_CCW_CW( a, b, c, false, 1.0f );
   ray.direction( ray.direction() + Vec3f(+small,   0.0f, 0.0f) );
   TEST_CCW_CW( a, b, c, false, 1.0f );
   ray.direction( ray.direction() + Vec3f(-small, +small, 0.0f) );
   TEST_CCW_CW( a, b, c, false, 1.0f );
   ray.direction( ray.direction() + Vec3f(-small,   0.0f, 0.0f) );
   TEST_CCW_CW( a, b, c, true , 1.0f );

   ray.direction( c - ray.origin() );
   TEST_CCW_CW( a, b, c, false, 1.0f );
   ray.direction( ray.direction() + Vec3f(0.0f, +small  , 0.0f) );
   TEST_CCW_CW( a, b, c, false, 1.0f );
   ray.direction( ray.direction() + Vec3f(0.0f, -small*2, 0.0f) );
   TEST_CCW_CW( a, b, c, true , 1.0f );
#endif

   // Some robustness check.
   // Star, CCW.
   //    d    c    b
   //      +---+---+
   //      |\  |  /|
   //      | \ | / |
   //      |  \|/  |
   //    e +---o---+ a
   //      |  /|\  |
   //      | / | \ |
   //      |/  |  \|
   //      +---+---+
   //    f    g    h
   a = Vec3f(  1.0f,  0.0f,  0.0f );
   b = Vec3f(  1.0f,  1.0f,  0.0f );
   c = Vec3f(  0.0f,  1.0f,  0.0f );
   d = Vec3f( -1.0f,  1.0f,  0.0f );
   e = Vec3f( -1.0f,  0.0f,  0.0f );
   f = Vec3f( -1.0f, -1.0f,  0.0f );
   g = Vec3f(  0.0f, -1.0f,  0.0f );
   h = Vec3f(  1.0f, -1.0f,  0.0f );
   o = Vec3f(  0.0f,  0.0f,  0.0f );
   ray.direction( Vec3f(0.0f, 0.0f, -1.0f) );

   // 2. Center of edges.
   // 2.1 Exterior edges.
   //      +-x-+-x-+
   //      |\  |  /|
   //      x \ | / x
   //      |  \|/  |
   //      +---o---+
   //      |  /|\  |
   //      x / | \ x
   //      |/  |  \|
   //      +-x-+-x-+
#if 1
   ray.origin( Vec3f( 1.0f, 0.5f, 4.0f) );
   TEST_CCW_CW( o, a, b, false, 4.0f );
   ray.origin( Vec3f( 0.5f, 1.0f, 4.0f) );
   TEST_CCW_CW( o, b, c, false, 4.0f );
   ray.origin( Vec3f(-0.5f, 1.0f, 4.0f) );
   TEST_CCW_CW( o, c, d, false, 4.0f );
   ray.origin( Vec3f(-1.0f, 0.5f, 4.0f) );
   TEST_CCW_CW( o, d, e, true , 4.0f );
   ray.origin( Vec3f(-1.0f,-0.5f, 4.0f) );
   TEST_CCW_CW( o, e, f, true , 4.0f );
   ray.origin( Vec3f(-0.5f,-1.0f, 4.0f) );
   TEST_CCW_CW( o, f, g, true , 4.0f );
   ray.origin( Vec3f( 0.5f,-1.0f, 4.0f) );
   TEST_CCW_CW( o, g, h, true , 4.0f );
   ray.origin( Vec3f( 1.0f,-0.5f, 4.0f) );
   TEST_CCW_CW( o, h, a, false, 4.0f );
#endif

   // 2.2 Interior edges.
   //      +---+---+
   //      |\  |  /|
   //      | x x x |
   //      |  \|/  |
   //      +-x-o-x-+
   //      |  /|\  |
   //      | x x x |
   //      |/  |  \|
   //      +---+---+
#if 1
   ray.origin( Vec3f( 0.5f, 0.0f, 4.0f) );
   TEST_CCW_CW( o, h, a, false, 4.0f );
   TEST_CCW_CW( o, a, b, true , 4.0f );
   ray.origin( Vec3f( 0.5f, 0.5f, 4.0f) );
   TEST_CCW_CW( o, a, b, true , 4.0f );
   TEST_CCW_CW( o, b, c, false, 4.0f );
   ray.origin( Vec3f( 0.0f, 0.5f, 4.0f) );
   TEST_CCW_CW( o, b, c, true , 4.0f );
   TEST_CCW_CW( o, c, d, false, 4.0f );
   ray.origin( Vec3f(-0.5f, 0.5f, 4.0f) );
   TEST_CCW_CW( o, c, d, true , 4.0f );
   TEST_CCW_CW( o, d, e, false, 4.0f );
   ray.origin( Vec3f(-0.5f, 0.0f, 4.0f) );
   TEST_CCW_CW( o, d, e, true , 4.0f );
   TEST_CCW_CW( o, e, f, false, 4.0f );
   ray.origin( Vec3f(-0.5f,-0.5f, 4.0f) );
   TEST_CCW_CW( o, e, f, false, 4.0f );
   TEST_CCW_CW( o, f, g, true , 4.0f );
   ray.origin( Vec3f( 0.0f,-0.5f, 4.0f) );
   TEST_CCW_CW( o, f, g, false, 4.0f );
   TEST_CCW_CW( o, g, h, true , 4.0f );
   ray.origin( Vec3f( 0.5f,-0.5f, 4.0f) );
   TEST_CCW_CW( o, g, h, false, 4.0f );
   TEST_CCW_CW( o, h, a, true , 4.0f );
#endif

   // 3. Corners.
   // 3.1 Outer corners.
   //      x---x---x
   //      |\  |  /|
   //      | \ | / |
   //      |  \|/  |
   //      x---o---x
   //      |  /|\  |
   //      | / | \ |
   //      |/  |  \|
   //      x---x---x
#if 1
   ray.origin( Vec3f( 1.0f, 0.0f, 4.0f) );
   TEST_CCW_CW( o, h, a, false, 4.0f );
   TEST_CCW_CW( o, a, b, false, 4.0f );
   ray.origin( Vec3f( 1.0f, 1.0f, 4.0f) );
   TEST_CCW_CW( o, a, b, false, 4.0f );
   TEST_CCW_CW( o, b, c, false, 4.0f );
   ray.origin( Vec3f( 0.0f, 1.0f, 4.0f) );
   TEST_CCW_CW( o, b, c, false, 4.0f );
   TEST_CCW_CW( o, c, d, false, 4.0f );
   ray.origin( Vec3f(-1.0f, 1.0f, 4.0f) );
   TEST_CCW_CW( o, c, d, false, 4.0f );
   TEST_CCW_CW( o, d, e, false, 4.0f );
   ray.origin( Vec3f(-1.0f, 0.0f, 4.0f) );
   TEST_CCW_CW( o, d, e, true , 4.0f );
   TEST_CCW_CW( o, e, f, false, 4.0f );
   ray.origin( Vec3f(-1.0f,-1.0f, 4.0f) );
   TEST_CCW_CW( o, e, f, false, 4.0f );
   TEST_CCW_CW( o, f, g, true , 4.0f );
   ray.origin( Vec3f( 0.0f,-1.0f, 4.0f) );
   TEST_CCW_CW( o, f, g, false, 4.0f );
   TEST_CCW_CW( o, g, h, true , 4.0f );
   ray.origin( Vec3f( 1.0f,-1.0f, 4.0f) );
   TEST_CCW_CW( o, g, h, false, 4.0f );
   TEST_CCW_CW( o, h, a, false, 4.0f );
#endif

   // 3.2 Center corner.
   //      +---+---+
   //      |\  |  /|
   //      | \ | / |
   //      |  \|/  |
   //      +---x---+
   //      |  /|\  |
   //      | / | \ |
   //      |/  |  \|
   //      +---+---+
#if 1
   ray.origin( Vec3f( 0.0f, 0.0f, 4.0f) );
   TEST_CCW_CW( o, a, b, true , 4.0f );
   TEST_CCW_CW( o, b, c, false, 4.0f );
   TEST_CCW_CW( o, c, d, false, 4.0f );
   TEST_CCW_CW( o, d, e, false, 4.0f );
   TEST_CCW_CW( o, e, f, false, 4.0f );
   TEST_CCW_CW( o, f, g, false, 4.0f );
   TEST_CCW_CW( o, g, h, false, 4.0f );
   TEST_CCW_CW( o, h, a, false, 4.0f );
#endif

   // Some corner cases found in specific tests.

   // Bug #1
   //   Caused by cross product being different across a shared edge.
   /*
   ori: (-7.0001 0xC0E000D2 (s=1, e=0x81(2), m=0x6000D2), 3.33333 0x40555556 (s=0, e=0x80(1), m=0x555556), -1.375 0xBFB00000 (s=1, e=0x7F(0), m=0x300000))
   dir: (1 0x3F800000 (s=0, e=0x7F(0), m=0x000000), 0 0x00000000 (s=0, e=0x00(-127), m=0x000000), 0 0x00000000 (s=0, e=0x00(-127), m=0x000000))
   A (6 0x40C00000 (s=0, e=0x81(2), m=0x400000), 2.77778 0x4031C71D (s=0, e=0x80(1), m=0x31C71D), -1.5 0xBFC00000 (s=1, e=0x7F(0), m=0x400000))
   B (6 0x40C00000 (s=0, e=0x81(2), m=0x400000), 2.77778 0x4031C71C (s=0, e=0x80(1), m=0x31C71C), -1 0xBF800000 (s=1, e=0x7F(0), m=0x000000))
   C (6 0x40C00000 (s=0, e=0x81(2), m=0x400000), 5 0x40A00000 (s=0, e=0x81(2), m=0x200000), -1 0xBF800000 (s=1, e=0x7F(0), m=0x000000))
   D (6 0x40C00000 (s=0, e=0x81(2), m=0x400000), 5 0x40A00000 (s=0, e=0x81(2), m=0x200000), -1.5 0xBFC00000 (s=1, e=0x7F(0), m=0x400000))
   1- A B C
   2- A C D
   */
   ray.origin( Vec3f( toFloat(0xC0E000D2), toFloat(0x40555556), toFloat(0xBFB00000) ) );
   ray.direction( Vec3f( toFloat(0x3F800000), toFloat(0x00000000), toFloat(0x00000000) ) );
   a = Vec3f( toFloat(0x40C00000), toFloat(0x4031C71D), toFloat(0xBFC00000) );
   b = Vec3f( toFloat(0x40C00000), toFloat(0x4031C71C), toFloat(0xBF800000) );
   c = Vec3f( toFloat(0x40C00000), toFloat(0x40A00000), toFloat(0xBF800000) );
   d = Vec3f( toFloat(0x40C00000), toFloat(0x40A00000), toFloat(0xBFC00000) );
   TEST_CCW_CW( a, b, c, false, 13.0001f );
   TEST_CCW_CW( a, c, d, true , 13.0001f );

   // Bug #2
   //    Caused by callapsing cases due to an "orthographic" projection.
   /*
   abc != def  1,1 i=0
   ori: (42.1096 0x42287047 (s=0, e=0x84(5), m=0x287047), -48.613 0xC24273AD (s=1, e=0x84(5), m=0x4273AD), 3 0x40400000 (s=0, e=0x80(1), m=0x400000))
   dir: (-40.1096 0xC2207047 (s=1, e=0x84(5), m=0x207047), 50.8932 0x424B92A0 (s=0, e=0x84(5), m=0x4B92A0), -1 0xBF800000 (s=1, e=0x7F(0), m=0x000000))
   hit: (2 0x40000000 (s=0, e=0x80(1), m=0x000000), 2.28022 0x4011EF30 (s=0, e=0x80(1), m=0x11EF30), 2 0x40000000 (s=0, e=0x80(1), m=0x000000))
   a: (2 0x40000000 (s=0, e=0x80(1), m=0x000000), 2 0x40000000 (s=0, e=0x80(1), m=0x000000), 2 0x40000000 (s=0, e=0x80(1), m=0x000000))
   b: (3 0x40400000 (s=0, e=0x80(1), m=0x400000), 3 0x40400000 (s=0, e=0x80(1), m=0x400000), 2 0x40000000 (s=0, e=0x80(1), m=0x000000))
   c: (2 0x40000000 (s=0, e=0x80(1), m=0x000000), 3 0x40400000 (s=0, e=0x80(1), m=0x400000), 2 0x40000000 (s=0, e=0x80(1), m=0x000000))
   d: (1 0x3F800000 (s=0, e=0x7F(0), m=0x000000), 3 0x40400000 (s=0, e=0x80(1), m=0x400000), 2 0x40000000 (s=0, e=0x80(1), m=0x000000))
   1- A B C
   2- A C D
   */
   ray.origin( Vec3f( toFloat(0x42287047), toFloat(0xC24273AD), toFloat(0x40400000) ) );
   ray.direction( Vec3f( toFloat(0xC2207047), toFloat(0x424B92A0), toFloat(0xBF800000) ) );
   a = Vec3f( toFloat(0x40000000), toFloat(0x40000000), toFloat(0x40000000) );
   b = Vec3f( toFloat(0x40400000), toFloat(0x40400000), toFloat(0x40000000) );
   c = Vec3f( toFloat(0x40000000), toFloat(0x40400000), toFloat(0x40000000) );
   d = Vec3f( toFloat(0x3F800000), toFloat(0x40400000), toFloat(0x40000000) );
   TEST_CCW_CW( a, b, c, true , 1.0f );
   TEST_CCW_CW( a, c, d, false, 1.0f );

//#undef TEST_CCW_CW

}

//------------------------------------------------------------------------------
//! Tries to stress ULP differences.
void cgmath_ray_caster_random( Test::Result& res )
{
   Rayf ray;
   RayCaster::Hitf  hit;
   Vec3f a, b, c, d, e, f, g, h, o;
   Vec3f dx = Vec3f( 1.0f, 0.0f, 0.0f );
   Vec3f dy = Vec3f( 0.0f, 1.0f, 0.0f );
   RNG_WELL  rng;

// Creates code to intersect 2 triangles:
//   [a b c] and [d e f]
// and verifies that only one of the 2 is hit.
#define COMPARE_CCW_CW( a, b, c, d, e, f ) \
   { \
      /* CCW */ \
      hit._t = CGConstf::infinity(); \
      bool h_abc = RayCaster::hit( ray, dx, dy, a, b, c, hit ); \
      hit._t = CGConstf::infinity(); \
      bool h_bca = RayCaster::hit( ray, dx, dy, b, c, a, hit ); \
      hit._t = CGConstf::infinity(); \
      bool h_cab = RayCaster::hit( ray, dx, dy, c, a, b, hit ); \
      TEST_ADD( res, (h_abc == h_bca) && (h_bca == h_cab) ); \
      hit._t = CGConstf::infinity(); \
      bool h_def = RayCaster::hit( ray, dx, dy, d, e, f, hit ); \
      hit._t = CGConstf::infinity(); \
      bool h_efd = RayCaster::hit( ray, dx, dy, e, f, d, hit ); \
      hit._t = CGConstf::infinity(); \
      bool h_fde = RayCaster::hit( ray, dx, dy, f, d, e, hit ); \
      TEST_ADD( res, (h_def == h_efd) && (h_efd == h_fde) ); \
      TEST_ADD( res, h_abc != h_def ); \
      if( h_abc == h_def ) \
      { \
         StdErr << "abc != def  " << h_abc << "," << h_def << " i=" << i << nl; \
         StdErr << "ori: " << toStr(ray.origin()) << nl; \
         StdErr << "dir: " << toStr(ray.direction()) << nl; \
         StdErr << "hit: " << toStr(ray.origin() + ray.direction()) << nl; \
         StdErr << "a: " << toStr(a) << nl; \
         StdErr << "b: " << toStr(b) << nl; \
         StdErr << "c: " << toStr(c) << nl; \
         StdErr << "d: " << toStr(d) << nl; \
         StdErr << "e: " << toStr(e) << nl; \
         StdErr << "f: " << toStr(f) << nl; \
         CHECK( false ); \
      } \
   }

// Prepares a randomized value along the specified edge.
#define PREPARE_EDGE( a, b ) \
   pt = CGM::linear( a, b-a, t ); \
   ray.origin( pt - dir ); \
   ray.direction( dir ); \

   // We use a range >1.0f to avoid denormalized numbers (which have more than 24b effective mantissa because of leading zeroes).
   // We throw random rays at edges and make sure only one of the triangles is hit.
   //      +---+---+
   //      |\  |  /|
   //      | x x x |
   //      |  \|/  |
   //      +-x-o-x-+
   //      |  /|\  |
   //      | x x x |
   //      |/  |  \|
   //      +---+---+
   a = Vec3f(  3.0f,  2.0f,  2.0f );
   b = Vec3f(  3.0f,  3.0f,  2.0f );
   c = Vec3f(  2.0f,  3.0f,  2.0f );
   d = Vec3f(  1.0f,  3.0f,  2.0f );
   e = Vec3f(  1.0f,  2.0f,  2.0f );
   f = Vec3f(  1.0f,  1.0f,  2.0f );
   g = Vec3f(  2.0f,  1.0f,  2.0f );
   h = Vec3f(  3.0f,  1.0f,  2.0f );
   o = Vec3f(  2.0f,  2.0f,  2.0f );
   const float t_range   = 0.95f;
   const float t_offset  = (1.0f - t_range) * 0.5f;
   const float xy_range  = 128.0f;
   const float xy_offset = xy_range * -0.5f;
   float t;
   Vec3f dir;
   Vec3f pt;
   for( uint i = 0; i < 1024; ++i )
   {
      t   = (float)rng.getDouble() * t_range + t_offset;
      dir = Vec3f( (float)rng.getDouble() * xy_range + xy_offset,
                   (float)rng.getDouble() * xy_range + xy_offset,
                   -1.0f );
      PREPARE_EDGE( o, a );
      COMPARE_CCW_CW( o, h, a, o, a, b );
      PREPARE_EDGE( o, b );
      COMPARE_CCW_CW( o, a, b, o, b, c );
      PREPARE_EDGE( o, c );
      COMPARE_CCW_CW( o, b, c, o, c, d );
      PREPARE_EDGE( o, d );
      COMPARE_CCW_CW( o, c, d, o, d, e );
      PREPARE_EDGE( o, e );
      COMPARE_CCW_CW( o, d, e, o, e, f );
      PREPARE_EDGE( o, f );
      COMPARE_CCW_CW( o, e, f, o, f, g );
      PREPARE_EDGE( o, g );
      COMPARE_CCW_CW( o, f, g, o, g, h );
      PREPARE_EDGE( o, h );
      COMPARE_CCW_CW( o, g, h, o, h, a );
   }


#undef COMPARE_CCW_CW

}


//------------------------------------------------------------------------------
//!
void  init_intersection()
{
   Test::Collection& std = Test::standard();
   std.add( new Test::Function( "rays",              "Tests ray class",                   cgmath_rays              ) );
   std.add( new Test::Function( "tri_pt_closest",    "Tests closest point to a triangle", cgmath_tri_pt_closest    ) );
   std.add( new Test::Function( "ray_caster",        "Tests ray casting",                 cgmath_ray_caster        ) );
   std.add( new Test::Function( "ray_caster_random", "Tests ray casting precision",       cgmath_ray_caster_random ) );
}
