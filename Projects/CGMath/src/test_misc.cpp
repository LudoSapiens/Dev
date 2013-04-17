/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/Dbg/UnitTest.h>

#include <CGMath/CGConst.h>
#include <CGMath/Geom.h>
#include <CGMath/Math.h>
#include <CGMath/Trig.h>
#include <CGMath/Variant.h>
#include <CGMath/Vec4.h>

#include <Base/Dbg/DebugStream.h>
#include <Base/Util/Timer.h>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_test, "Test" );

UNNAMESPACE_END

//------------------------------------------------------------------------------
//! Some atan2 approximations (all bad, the good one is in CGMath.h)
// |error| < 0.005
float fast_atan2f( float y, float x )
{
   if ( x == 0.0f )
   {
      if ( y > 0.0f ) return CGConstf::pi_2();
      if ( y == 0.0f ) return 0.0f;
      return -CGConstf::pi_2();
   }
   float atan;
   float z = y/x;
   if ( fabsf( z ) < 1.0f )
   {
      atan = z/(1.0f + 0.28f*z*z);
      if ( x < 0.0f )
      {
         if ( y < 0.0f ) return atan - CGConstf::pi();
         return atan + CGConstf::pi();
      }
   }
   else
   {
      atan = CGConstf::pi_2() - z/(z*z + 0.28f);
      if ( y < 0.0f ) return atan - CGConstf::pi();
   }
   return atan;
}

// |error| < 0.005
double fast_atan2d( double y, double x )
{
   if ( x == 0.0 )
   {
      if ( y > 0.0 ) return CGConstd::pi_2();
      if ( y == 0.0 ) return 0.0;
      return -CGConstd::pi_2();
   }
   double atan;
   double z = y/x;
   if ( fabs( z ) < 1.0 )
   {
      atan = z/(1.0 + 0.28*z*z);
      if ( x < 0.0 )
      {
         if ( y < 0.0 ) return atan - CGConstd::pi();
         return atan + CGConstd::pi();
      }
   }
   else
   {
      atan = CGConstd::pi_2() - z/(z*z + 0.28);
      if ( y < 0.0 ) return atan - CGConstd::pi();
   }
   return atan;
}


#define RUN_ATAN2_PREC(atan2_func, type) \
   sum = 0; \
   count = 0; \
   max_error = 0; \
   max_rel_error = 0; \
   fflush(NULL); \
   timer.restart(); \
   for( int i = 0; i < 10000; ++i ) \
   { \
      for( double y = -1.0; y <= 1.0; y += 0.05 ) \
      { \
         for( double x = -1.0; x <= 1.0; x += 0.05 ) \
         { \
            double theta = atan2_func((type)y, (type)x); \
            double theta_gold = atan2(y, x); \
            double error = fabs(theta-theta_gold); \
            if( max_error < error )  max_error = error; \
            double rel_error = fabs(error)/CGConstd::pi2(); \
            if( max_rel_error < rel_error )  max_rel_error = rel_error; \
            sum += theta; \
            ++count; \
         } \
      } \
   } \
   elapsed = timer.elapsed(); \
   printf("%15s: SUM=%g  elapsed=%gs  [rate=%.4g calls/s]  max_error=%g (%.2f%%)\n", #atan2_func, sum, elapsed, count/elapsed, max_error, max_rel_error*100.0)

#define RUN_ATAN2(atan2_func, type) \
   sum = 0; \
   count = 0; \
   max_error = 0; \
   fflush(NULL); \
   timer.restart(); \
   for( int i = 0; i < 10000; ++i ) \
   { \
      for( double y = -1.0; y <= 1.0; y += 0.05 ) \
      { \
         for( double x = -1.0; x <= 1.0; x += 0.05 ) \
         { \
            double theta = atan2_func((type)y, (type)x); \
            sum += theta; \
            ++count; \
         } \
      } \
   } \
   elapsed = timer.elapsed(); \
   printf("%15s: SUM=%g  elapsed=%gs  rate=%.4g calls/s\n", #atan2_func, sum, elapsed, count/elapsed)

void cgmath_atan2( Test::Result& res )
{
   printf("Testing atan2 approximations\n");
   printf("Note: optimizations should be turned on for performance tests be relevant\n");

   Timer timer;
   double elapsed;
   double sum;
   double count;
   double max_error;
   double max_rel_error;

   RUN_ATAN2(atan2, double);
   RUN_ATAN2(fast_atan2f, float);
   RUN_ATAN2(fast_atan2d, double);
   RUN_ATAN2(CGM::atan2, double);
   RUN_ATAN2(CGM::atan2, float);
   printf("----\n");
   RUN_ATAN2_PREC(atan2, double);
   RUN_ATAN2_PREC(fast_atan2f, float);
   RUN_ATAN2_PREC(fast_atan2d, double);
   RUN_ATAN2_PREC(CGM::atan2, double);
   RUN_ATAN2_PREC(CGM::atan2, float);
}

//------------------------------------------------------------------------------
//!
void cgmath_angles( Test::Result& res )
{
   float deg = 90;
   float rad = CGM::degToRad( deg );

   TEST_ADD( res, CGM::equal((double)rad, CGConstd::pi_2()) );

   DBG_MSG( os_test, "deg: " << deg << " rad: " << rad << " deg: " << CGM::radToDeg( rad ) );
}

//------------------------------------------------------------------------------
//!
void cgmath_clamp( Test::Result& res )
{
   TEST_ADD( res, CGM::clamp( -1.5, 0.0, 1.0 ) == 0.0 );
   TEST_ADD( res, CGM::clamp(  0.5, 0.0, 1.0 ) == 0.5 );
   TEST_ADD( res, CGM::clamp(  1.5, 0.0, 1.0 ) == 1.0 );

   TEST_ADD( res, CGM::clamp( -1.1, -1.0, 1.0 ) == -1.0 );
   TEST_ADD( res, CGM::clamp( -1.0, -1.0, 1.0 ) == -1.0 );
   TEST_ADD( res, CGM::clamp( -0.9, -1.0, 1.0 ) == -0.9 );
   TEST_ADD( res, CGM::clamp(  0.9, -1.0, 1.0 ) ==  0.9 );
   TEST_ADD( res, CGM::clamp(  1.0, -1.0, 1.0 ) ==  1.0 );
   TEST_ADD( res, CGM::clamp(  1.1, -1.0, 1.0 ) ==  1.0 );

   double v;
   v = -1.0;
   CGM::clampEq( v, 0.0, 1.0 );
   TEST_ADD( res, v == 0.0 );
   v = 0.5;
   CGM::clampEq( v, 0.0, 1.0 );
   TEST_ADD( res, v == 0.5 );
   v = 1.5;
   CGM::clampEq( v, 0.0, 1.0 );
   TEST_ADD( res, v == 1.0 );

   // l: clamps left, v: valid (no clamp), r: clamps right
   TEST_ADD( res, CGM::clamp( Vec2f(0.8f, 0.9f), 1.0f, 2.0f ) == Vec2f(1.0f, 1.0f) ); // l l
   TEST_ADD( res, CGM::clamp( Vec2f(0.9f, 1.1f), 1.0f, 2.0f ) == Vec2f(1.0f, 1.1f) ); // l v
   TEST_ADD( res, CGM::clamp( Vec2f(1.1f, 1.9f), 1.0f, 2.0f ) == Vec2f(1.1f, 1.9f) ); // v v
   TEST_ADD( res, CGM::clamp( Vec2f(1.9f, 2.1f), 1.0f, 2.0f ) == Vec2f(1.9f, 2.0f) ); // v r
   TEST_ADD( res, CGM::clamp( Vec2f(2.1f, 2.2f), 1.0f, 2.0f ) == Vec2f(2.0f, 2.0f) ); // r r

   TEST_ADD( res, CGM::clamp( Vec3f(0.7f, 0.8f, 0.9f), 1.0f, 2.0f ) == Vec3f(1.0f, 1.0f, 1.0f) ); // l l l
   TEST_ADD( res, CGM::clamp( Vec3f(0.8f, 0.9f, 1.1f), 1.0f, 2.0f ) == Vec3f(1.0f, 1.0f, 1.1f) ); // l l v
   TEST_ADD( res, CGM::clamp( Vec3f(0.9f, 1.1f, 1.5f), 1.0f, 2.0f ) == Vec3f(1.0f, 1.1f, 1.5f) ); // l v v
   TEST_ADD( res, CGM::clamp( Vec3f(1.1f, 1.5f, 1.9f), 1.0f, 2.0f ) == Vec3f(1.1f, 1.5f, 1.9f) ); // v v v
   TEST_ADD( res, CGM::clamp( Vec3f(1.5f, 1.9f, 2.1f), 1.0f, 2.0f ) == Vec3f(1.5f, 1.9f, 2.0f) ); // v v r
   TEST_ADD( res, CGM::clamp( Vec3f(1.9f, 2.1f, 2.2f), 1.0f, 2.0f ) == Vec3f(1.9f, 2.0f, 2.0f) ); // v r r
   TEST_ADD( res, CGM::clamp( Vec3f(2.1f, 2.2f, 2.3f), 1.0f, 2.0f ) == Vec3f(2.0f, 2.0f, 2.0f) ); // r r r

   TEST_ADD( res, CGM::clamp( Vec4f(0.6f, 0.7f, 0.8f, 0.9f), 1.0f, 2.0f ) == Vec4f(1.0f, 1.0f, 1.0f, 1.0f) ); // l l l l
   TEST_ADD( res, CGM::clamp( Vec4f(0.7f, 0.8f, 0.9f, 1.1f), 1.0f, 2.0f ) == Vec4f(1.0f, 1.0f, 1.0f, 1.1f) ); // l l l v
   TEST_ADD( res, CGM::clamp( Vec4f(0.8f, 0.9f, 1.1f, 1.4f), 1.0f, 2.0f ) == Vec4f(1.0f, 1.0f, 1.1f, 1.4f) ); // l l v v
   TEST_ADD( res, CGM::clamp( Vec4f(0.9f, 1.1f, 1.4f, 1.6f), 1.0f, 2.0f ) == Vec4f(1.0f, 1.1f, 1.4f, 1.6f) ); // l v v v
   TEST_ADD( res, CGM::clamp( Vec4f(1.1f, 1.4f, 1.6f, 1.9f), 1.0f, 2.0f ) == Vec4f(1.1f, 1.4f, 1.6f, 1.9f) ); // v v v v
   TEST_ADD( res, CGM::clamp( Vec4f(1.4f, 1.6f, 1.9f, 2.1f), 1.0f, 2.0f ) == Vec4f(1.4f, 1.6f, 1.9f, 2.0f) ); // v v v r
   TEST_ADD( res, CGM::clamp( Vec4f(1.6f, 1.9f, 2.1f, 2.2f), 1.0f, 2.0f ) == Vec4f(1.6f, 1.9f, 2.0f, 2.0f) ); // v v r r
   TEST_ADD( res, CGM::clamp( Vec4f(1.9f, 2.1f, 2.2f, 2.3f), 1.0f, 2.0f ) == Vec4f(1.9f, 2.0f, 2.0f, 2.0f) ); // v r r r
   TEST_ADD( res, CGM::clamp( Vec4f(2.1f, 2.2f, 2.3f, 2.4f), 1.0f, 2.0f ) == Vec4f(2.0f, 2.0f, 2.0f, 2.0f) ); // r r r r

   // Invalid range values.
   int i;
   i = 1;
   TEST_ADD( res, CGM::clamp(i, 4, 2) == 2 );
   CGM::clampEq( i, 4, 2 );
   TEST_ADD( res, i == 4 );

   i = 3;
   TEST_ADD( res, CGM::clamp(i, 4, 2) == 2 );
   CGM::clampEq( i, 4, 2 );
   TEST_ADD( res, i == 4 );

   i = 5;
   TEST_ADD( res, CGM::clamp(i, 4, 2) == 2 );
   CGM::clampEq( i, 4, 2 );
   TEST_ADD( res, i == 2 );
}

//------------------------------------------------------------------------------
//!
void cgmath_fixed( Test::Result& res )
{
   TEST_ADD( res, CGM::fixedPrec( 0.00f, 0) ==  0.0f );
   TEST_ADD( res, CGM::fixedPrec( 0.24f, 0) ==  0.0f );
   TEST_ADD( res, CGM::fixedPrec( 0.25f, 0) ==  0.0f );
   TEST_ADD( res, CGM::fixedPrec( 0.26f, 0) ==  0.0f );
   TEST_ADD( res, CGM::fixedPrec( 0.49f, 0) ==  0.0f );
   TEST_ADD( res, CGM::fixedPrec( 0.50f, 0) ==  0.0f );
   TEST_ADD( res, CGM::fixedPrec( 0.51f, 0) ==  1.0f );
   TEST_ADD( res, CGM::fixedPrec( 0.99f, 0) ==  1.0f );
   TEST_ADD( res, CGM::fixedPrec( 1.00f, 0) ==  1.0f );
   TEST_ADD( res, CGM::fixedPrec( 1.01f, 0) ==  1.0f );
   TEST_ADD( res, CGM::fixedPrec( 1.49f, 0) ==  1.0f );
   TEST_ADD( res, CGM::fixedPrec( 1.50f, 0) ==  2.0f );
   TEST_ADD( res, CGM::fixedPrec(-1.26f, 0) == -1.0f );

   TEST_ADD( res, CGM::fixedPrec( 0.00f, 1) ==  0.0f );
   TEST_ADD( res, CGM::fixedPrec( 0.24f, 1) ==  0.0f );
   TEST_ADD( res, CGM::fixedPrec( 0.25f, 1) ==  0.0f );
   TEST_ADD( res, CGM::fixedPrec( 0.26f, 1) ==  0.5f );
   TEST_ADD( res, CGM::fixedPrec( 0.49f, 1) ==  0.5f );
   TEST_ADD( res, CGM::fixedPrec( 0.50f, 1) ==  0.5f );
   TEST_ADD( res, CGM::fixedPrec( 0.51f, 1) ==  0.5f );
   TEST_ADD( res, CGM::fixedPrec( 0.99f, 1) ==  1.0f );
   TEST_ADD( res, CGM::fixedPrec( 1.00f, 1) ==  1.0f );
   TEST_ADD( res, CGM::fixedPrec( 1.01f, 1) ==  1.0f );
   TEST_ADD( res, CGM::fixedPrec( 1.49f, 1) ==  1.5f );
   TEST_ADD( res, CGM::fixedPrec( 1.50f, 1) ==  1.5f );
   TEST_ADD( res, CGM::fixedPrec(-1.26f, 1) == -1.5f );

   TEST_ADD( res, CGM::fixedPrec( 0.00f, 2) ==  0.00f );
   TEST_ADD( res, CGM::fixedPrec( 0.24f, 2) ==  0.25f );
   TEST_ADD( res, CGM::fixedPrec( 0.25f, 2) ==  0.25f );
   TEST_ADD( res, CGM::fixedPrec( 0.26f, 2) ==  0.25f );
   TEST_ADD( res, CGM::fixedPrec( 0.49f, 2) ==  0.50f );
   TEST_ADD( res, CGM::fixedPrec( 0.50f, 2) ==  0.50f );
   TEST_ADD( res, CGM::fixedPrec( 0.51f, 2) ==  0.50f );
   TEST_ADD( res, CGM::fixedPrec( 0.99f, 2) ==  1.00f );
   TEST_ADD( res, CGM::fixedPrec( 1.00f, 2) ==  1.00f );
   TEST_ADD( res, CGM::fixedPrec( 1.01f, 2) ==  1.00f );
   TEST_ADD( res, CGM::fixedPrec( 1.49f, 2) ==  1.50f );
   TEST_ADD( res, CGM::fixedPrec( 1.50f, 2) ==  1.50f );
   TEST_ADD( res, CGM::fixedPrec(-1.26f, 2) == -1.25f );
}

//------------------------------------------------------------------------------
//!
void cgmath_math( Test::Result& res )
{
   float f_f;
   float f_i;
   int   i_i;

   CGM::splitIntFrac( 1.25f, f_i, f_f );
   TEST_ADD( res, f_i == 1.0f && f_f == 0.25f );

   CGM::splitIntFrac( 1.25f, i_i, f_f );
   TEST_ADD( res, i_i == 1 && f_f == 0.25f );

   CGM::splitIntFrac( -1.25f, f_i, f_f );
   TEST_ADD( res, f_i == -2.0f && f_f == 0.75f );

   CGM::splitIntFrac( -1.25f, i_i, f_f );
   TEST_ADD( res, i_i == -2 && f_f == 0.75f );

   TEST_ADD( res, CGM::copySign( 2.0f, 3.0f) ==  2.0f );
   TEST_ADD( res, CGM::copySign( 2.0f,-3.0f) == -2.0f );
   TEST_ADD( res, CGM::copySign(-2.0f, 3.0f) ==  2.0f );
   TEST_ADD( res, CGM::copySign(-2.0f,-3.0f) == -2.0f );

   TEST_ADD( res, CGM::sign(-2.0f) == -1.0f );
   TEST_ADD( res, CGM::sign( 0.0f) ==  0.0f );
   TEST_ADD( res, CGM::sign( 2.0f) ==  1.0f );

   TEST_ADD( res, CGM::copySign( 2, 3) ==  2 );
   TEST_ADD( res, CGM::copySign( 2,-3) == -2 );
   TEST_ADD( res, CGM::copySign(-2, 3) ==  2 );
   TEST_ADD( res, CGM::copySign(-2,-3) == -2 );

   TEST_ADD( res, CGM::sign(-2) == -1 );
   TEST_ADD( res, CGM::sign( 0) ==  0 );
   TEST_ADD( res, CGM::sign( 2) ==  1 );
}

//------------------------------------------------------------------------------
//!
void cgmath_misc( Test::Result& res )
{
   Vec4f v1( 1,  2,  3,  4);
   Vec4f v2(11, 22, 33, 44);

   TEST_ADD( res, v1 == Vec4f( 1,  2,  3,  4) );
   TEST_ADD( res, v2 == Vec4f(11, 22, 33, 44) );
   CGM::swap(v1, v2);
   TEST_ADD( res, v1 == Vec4f(11, 22, 33, 44) );
   TEST_ADD( res, v2 == Vec4f( 1,  2,  3,  4) );
   CGM::swap(v1(1), v2(2));
   TEST_ADD( res, v1 == Vec4f(11,  3, 33, 44) );
   TEST_ADD( res, v2 == Vec4f( 1,  2, 22,  4) );

   float fNaN = CGConstf::NaN();
   TEST_ADD( res, fNaN != fNaN );
   TEST_ADD( res, fNaN != 0.0f );
   TEST_ADD( res, !(fNaN <  0.0f) );
   TEST_ADD( res, !(fNaN >= 0.0f) );
   TEST_ADD( res, fNaN != CGConstf::infinity() );
   TEST_ADD( res, CGConstf::lowest()  == -CGConstf::infinity() );
   TEST_ADD( res, CGConstf::min()      >  0.0f );
   TEST_ADD( res, CGConstf::max()      <  CGConstf::infinity() );
   TEST_ADD( res, CGConstf::highest() ==  CGConstf::infinity() );
   TEST_ADD( res, CGConstf::infinity()+1.0f == CGConstf::infinity() );

   double dNaN = CGConstd::NaN();
   TEST_ADD( res, dNaN != dNaN );
   TEST_ADD( res, dNaN != 0.0 );
   TEST_ADD( res, !(dNaN <  0.0) );
   TEST_ADD( res, !(dNaN >= 0.0) );
   TEST_ADD( res, dNaN != CGConstd::infinity() );
   TEST_ADD( res, CGConstd::lowest()  == -CGConstd::infinity() );
   TEST_ADD( res, CGConstd::min()      >  0.0  );
   TEST_ADD( res, CGConstd::max()      <  CGConstd::infinity() );
   TEST_ADD( res, CGConstd::highest() ==  CGConstd::infinity() );
   TEST_ADD( res, CGConstd::infinity()+1.0  == CGConstd::infinity() );

   TEST_ADD( res, CGConst<int8_t>::lowest()   == -128 );
   TEST_ADD( res, CGConst<int8_t>::min()      == -128 );
   TEST_ADD( res, CGConst<int8_t>::max()      ==  127 );
   TEST_ADD( res, CGConst<int8_t>::highest()  ==  127 );
   TEST_ADD( res, CGConst<int8_t>::infinity() ==  127 );
}

//------------------------------------------------------------------------------
//!
void cgmath_nextPow2( Test::Result& res )
{
   TEST_ADD( res, CGM::nextPow2(  0) ==   0 );
   TEST_ADD( res, CGM::nextPow2(  1) ==   1 );
   TEST_ADD( res, CGM::nextPow2(  2) ==   2 );
   TEST_ADD( res, CGM::nextPow2(  3) ==   4 );
   TEST_ADD( res, CGM::nextPow2(  4) ==   4 );
   TEST_ADD( res, CGM::nextPow2(  5) ==   8 );
   TEST_ADD( res, CGM::nextPow2(  6) ==   8 );
   TEST_ADD( res, CGM::nextPow2(  7) ==   8 );
   TEST_ADD( res, CGM::nextPow2(  8) ==   8 );
   TEST_ADD( res, CGM::nextPow2( 15) ==  16 );
   TEST_ADD( res, CGM::nextPow2( 16) ==  16 );
   TEST_ADD( res, CGM::nextPow2( 17) ==  32 );
   TEST_ADD( res, CGM::nextPow2( 31) ==  32 );
   TEST_ADD( res, CGM::nextPow2( 32) ==  32 );
   TEST_ADD( res, CGM::nextPow2( 33) ==  64 );
   TEST_ADD( res, CGM::nextPow2( 63) ==  64 );
   TEST_ADD( res, CGM::nextPow2( 64) ==  64 );
   TEST_ADD( res, CGM::nextPow2( 65) == 128 );

   TEST_ADD( res, CGM::nextPow2(0x01000000) == 0x01000000 );
   TEST_ADD( res, CGM::nextPow2(0x01000001) == 0x02000000 );
   TEST_ADD( res, CGM::nextPow2(0x01000002) == 0x02000000 );
   TEST_ADD( res, CGM::nextPow2(0x01000004) == 0x02000000 );
   TEST_ADD( res, CGM::nextPow2(0x01000008) == 0x02000000 );
   TEST_ADD( res, CGM::nextPow2(0x40000000) == 0x40000000 );
   TEST_ADD( res, CGM::nextPow2(0x40000001) == 0x80000000 );
   TEST_ADD( res, CGM::nextPow2(0x40000002) == 0x80000000 );
   TEST_ADD( res, CGM::nextPow2(0x40000004) == 0x80000000 );
   TEST_ADD( res, CGM::nextPow2(0x40000008) == 0x80000000 );
   TEST_ADD( res, CGM::nextPow2(0x7FFFFFFF) == 0x80000000 );
   TEST_ADD( res, CGM::nextPow2(0x80000000) == 0x80000000 );
}

//------------------------------------------------------------------------------
//!
void cgmath_perf( Test::Result& res )
{
   //////////////////
   // Speed tests. //

   // Tests for vector insertions.

   Vector< Vec3f > vectors;
   vectors.reserve( 10000 );
   Timer timer;
   for( int i = 0; i < 10000; ++i )
   {
      vectors.pushBack( Vec3f( 10, 4, 4 ) );
   }
   printf( "time: %f\n", timer.elapsed() );

   timer.restart();
   for( uint i = 0; i < vectors.size(); ++i )
   {
      vectors[i] = Vec3f( 10, 4, 4 );
   }
   printf( "time: %f\n", timer.elapsed() );

   Vector< Vec3f >::Iterator it  = vectors.begin();
   Vector< Vec3f >::Iterator end = vectors.end();
   timer.restart();
   for( ; it != end; ++it )
   {
      (*it) = Vec3f( 10, 4, 4 );
   }
   printf( "time: %f\n", timer.elapsed() );

#define TEST_ROUTINE(func, s, e, i) \
   timer.restart(); \
   f = 0.0f; \
   for( float v = (float)s; v <= (float)e; v += (float)i ) \
   { \
      f += CGM::func((float)v); \
   } \
   printf( "%s(float): %fs (f=%g)\n", #func, timer.elapsed(), f ); \
   timer.restart(); \
   d = 0.0; \
   for( double v = s; v <= e; v += i ) \
   { \
      d += CGM::func(v); \
   } \
   printf( "%s(double): %fs (d=%g)\n", #func, timer.elapsed(), d )

   // Test float vs. double speed.
   float f;
   double d;
   TEST_ROUTINE(cos, -CGConstd::pi(), CGConstd::pi(), 0.00001f);
   TEST_ROUTINE(sin, -CGConstd::pi(), CGConstd::pi(), 0.00001f);
   TEST_ROUTINE(tan, -CGConstd::pi(), CGConstd::pi(), 0.00001f);
   TEST_ROUTINE(acos, -1.0f, +1.0f, 0.00001f);
   TEST_ROUTINE(asin, -1.0f, +1.0f, 0.00001f);
   TEST_ROUTINE(atan, -1.0f, +1.0f, 0.00001f);
   TEST_ROUTINE(sqrt, -1000.0f, 1000.0f, 0.0001f);
}

//------------------------------------------------------------------------------
//!
void cgmath_variant( Test::Result& res )
{
   Variant v;

   TEST_ADD( res,  v.type() == Variant::NIL );
   TEST_ADD( res,  v.isNil()     );
   TEST_ADD( res, !v.isBoolean() );
   TEST_ADD( res, !v.isFloat()   );
   TEST_ADD( res, !v.isVec2()    );
   TEST_ADD( res, !v.isVec3()    );
   TEST_ADD( res, !v.isVec4()    );
   TEST_ADD( res, !v.isQuat()    );
   TEST_ADD( res, !v.isString()  );
   TEST_ADD( res, !v.isTable()   );

   // Boolean.
   v = true;
   TEST_ADD( res,  v.type() == Variant::BOOL );
   TEST_ADD( res, !v.isNil()     );
   TEST_ADD( res,  v.isBoolean() );
   TEST_ADD( res, !v.isFloat()   );
   TEST_ADD( res, !v.isVec2()    );
   TEST_ADD( res, !v.isVec3()    );
   TEST_ADD( res, !v.isVec4()    );
   TEST_ADD( res, !v.isQuat()    );
   TEST_ADD( res, !v.isString()  );
   TEST_ADD( res, !v.isTable()   );
   TEST_ADD( res, v.getBoolean() == true  );
   TEST_ADD( res, v.getBoolean() != false );
   v = false;
   TEST_ADD( res, v.getBoolean() == false );
   TEST_ADD( res, v.getBoolean() != true  );

   // Float.
   v = 1.0f;
   TEST_ADD( res,  v.type() == Variant::FLOAT );
   TEST_ADD( res, !v.isNil()     );
   TEST_ADD( res, !v.isBoolean() );
   TEST_ADD( res,  v.isFloat()   );
   TEST_ADD( res, !v.isVec2()    );
   TEST_ADD( res, !v.isVec3()    );
   TEST_ADD( res, !v.isVec4()    );
   TEST_ADD( res, !v.isQuat()    );
   TEST_ADD( res, !v.isString()  );
   TEST_ADD( res, !v.isTable()   );
   TEST_ADD( res, v.getFloat() == 1.0f );
   TEST_ADD( res, v.getFloat() != 2.0f );
   v = 2.0f;
   TEST_ADD( res, v.getFloat() != 1.0f );
   TEST_ADD( res, v.getFloat() == 2.0f );

   // Vec2.
   v = Vec2f( 1.0f, 2.0f );
   TEST_ADD( res,  v.type() == Variant::VEC2 );
   TEST_ADD( res, !v.isNil()     );
   TEST_ADD( res, !v.isBoolean() );
   TEST_ADD( res, !v.isFloat()   );
   TEST_ADD( res,  v.isVec2()    );
   TEST_ADD( res, !v.isVec3()    );
   TEST_ADD( res, !v.isVec4()    );
   TEST_ADD( res, !v.isQuat()    );
   TEST_ADD( res, !v.isString()  );
   TEST_ADD( res, !v.isTable()   );
   TEST_ADD( res, v.getVec2() == Vec2f(1.0f, 2.0f) );
   TEST_ADD( res, v.getVec2() != Vec2f(3.0f, 4.0f) );
   v = Vec2f( 3.0f, 4.0f );
   TEST_ADD( res, v.getVec2() != Vec2f(1.0f, 2.0f) );
   TEST_ADD( res, v.getVec2() == Vec2f(3.0f, 4.0f) );

   // Vec3.
   v = Vec3f( 1.0f, 2.0f, 3.0f );
   TEST_ADD( res,  v.type() == Variant::VEC3 );
   TEST_ADD( res, !v.isNil()     );
   TEST_ADD( res, !v.isBoolean() );
   TEST_ADD( res, !v.isFloat()   );
   TEST_ADD( res, !v.isVec2()    );
   TEST_ADD( res,  v.isVec3()    );
   TEST_ADD( res, !v.isVec4()    );
   TEST_ADD( res, !v.isQuat()    );
   TEST_ADD( res, !v.isString()  );
   TEST_ADD( res, !v.isTable()   );
   TEST_ADD( res, v.getVec3() == Vec3f(1.0f, 2.0f, 3.0f) );
   TEST_ADD( res, v.getVec3() != Vec3f(4.0f, 5.0f, 6.0f) );
   v = Vec3f( 4.0f, 5.0f, 6.0f );
   TEST_ADD( res, v.getVec3() != Vec3f(1.0f, 2.0f, 3.0f) );
   TEST_ADD( res, v.getVec3() == Vec3f(4.0f, 5.0f, 6.0f) );

   // Vec4.
   v = Vec4f( 1.0f, 2.0f, 3.0f, 4.0f );
   TEST_ADD( res,  v.type() == Variant::VEC4 );
   TEST_ADD( res, !v.isNil()     );
   TEST_ADD( res, !v.isBoolean() );
   TEST_ADD( res, !v.isFloat()   );
   TEST_ADD( res, !v.isVec2()    );
   TEST_ADD( res, !v.isVec3()    );
   TEST_ADD( res,  v.isVec4()    );
   TEST_ADD( res, !v.isQuat()    );
   TEST_ADD( res, !v.isString()  );
   TEST_ADD( res, !v.isTable()   );
   TEST_ADD( res, v.getVec4() == Vec4f(1.0f, 2.0f, 3.0f, 4.0f) );
   TEST_ADD( res, v.getVec4() != Vec4f(5.0f, 6.0f, 7.0f, 8.0f) );
   v = Vec4f( 5.0f, 6.0f, 7.0f, 8.0f );
   TEST_ADD( res, v.getVec4() != Vec4f(1.0f, 2.0f, 3.0f, 4.0f) );
   TEST_ADD( res, v.getVec4() == Vec4f(5.0f, 6.0f, 7.0f, 8.0f) );

   // Quat.
   v = Quatf( 1.0f, 2.0f, 3.0f, 4.0f );
   TEST_ADD( res,  v.type() == Variant::QUAT );
   TEST_ADD( res, !v.isNil()     );
   TEST_ADD( res, !v.isBoolean() );
   TEST_ADD( res, !v.isFloat()   );
   TEST_ADD( res, !v.isVec2()    );
   TEST_ADD( res, !v.isVec3()    );
   TEST_ADD( res, !v.isVec4()    );
   TEST_ADD( res,  v.isQuat()    );
   TEST_ADD( res, !v.isString()  );
   TEST_ADD( res, !v.isTable()   );
   TEST_ADD( res, v.getQuat() == Quatf(1.0f, 2.0f, 3.0f, 4.0f) );
   TEST_ADD( res, v.getQuat() != Quatf(5.0f, 6.0f, 7.0f, 8.0f) );
   v = Quatf( 5.0f, 6.0f, 7.0f, 8.0f );
   TEST_ADD( res, v.getQuat() != Quatf(1.0f, 2.0f, 3.0f, 4.0f) );
   TEST_ADD( res, v.getQuat() == Quatf(5.0f, 6.0f, 7.0f, 8.0f) );

   // String.
   v = ConstString( "some string" );
   TEST_ADD( res,  v.type() == Variant::STRING );
   TEST_ADD( res, !v.isNil()     );
   TEST_ADD( res, !v.isBoolean() );
   TEST_ADD( res, !v.isFloat()   );
   TEST_ADD( res, !v.isVec2()    );
   TEST_ADD( res, !v.isVec3()    );
   TEST_ADD( res, !v.isVec4()    );
   TEST_ADD( res, !v.isQuat()    );
   TEST_ADD( res,  v.isString()  );
   TEST_ADD( res, !v.isTable()   );
   TEST_ADD( res, v.getString() == ConstString( "some string" )       );
   TEST_ADD( res, v.getString() != ConstString( "some other string" ) );
   v = ConstString( "some other string" );
   TEST_ADD( res, v.getString() != ConstString( "some string" )       );
   TEST_ADD( res, v.getString() == ConstString( "some other string" ) );


   RCP<Table> tp = new Table();
   Table& t      = *tp;

   TEST_ADD( res, t.size() == 0 );
   TEST_ADD( res, t.mapSize() == 0 );
   TEST_ADD( res, t.arraySize() == 0 );

   t.set( ConstString("one"), 1.0f );
   TEST_ADD( res, t.size() == 1 );
   TEST_ADD( res, t.mapSize() == 1 );
   TEST_ADD( res, t.arraySize() == 0 );
   TEST_ADD( res, t["one"].getFloat() == 1.0f );
   TEST_ADD( res, t["two"].isNil() );

   t.set( ConstString("two"), Vec2f(2.0f) );
   TEST_ADD( res, t.size() == 2 );
   TEST_ADD( res, t.mapSize() == 2 );
   TEST_ADD( res, t.arraySize() == 0 );
   TEST_ADD( res, t["one"].getFloat() == 1.0f );
   TEST_ADD( res, t["two"].getVec2()  == Vec2f(2.0f) );

   t.remove( ConstString("two") );
   TEST_ADD( res, t.size() == 1 );
   TEST_ADD( res, t.mapSize() == 1 );
   TEST_ADD( res, t.arraySize() == 0 );
   TEST_ADD( res, t["one"].getFloat() == 1.0f );
   TEST_ADD( res, t["two"].isNil() );

   t["two"] = Vec2f(2.0f);
   TEST_ADD( res, t.size() == 2 );
   TEST_ADD( res, t.mapSize() == 2 );
   TEST_ADD( res, t.arraySize() == 0 );
   TEST_ADD( res, t["one"].getFloat() == 1.0f );
   TEST_ADD( res, t["two"].getVec2()  == Vec2f(2.0f) );

   Table::ConstIterator first = t.begin();
   TEST_ADD( res, first != t.end() );
   Table::ConstIterator second = (++t.begin());
   TEST_ADD( res, second != t.end() );
   Table::ConstIterator third = (++(++t.begin()));
   TEST_ADD( res, third == t.end() );
   TEST_ADD( res, (first->first == "one" && second->first == "two") || (first->first == "two" && second->first == "one") );
   TEST_ADD( res, (first->first == "one" && first->second.isFloat() && second->second.isVec2()) || (first->second.isVec2() && second->second.isFloat()) );

   t[0] = ConstString("a");
   t.set( 1, ConstString("b") );
   t[2] = ConstString("c");
   TEST_ADD( res, t.size() == 5 );
   TEST_ADD( res, t.mapSize() == 2 );
   TEST_ADD( res, t.arraySize() == 3 );
   TEST_ADD( res, t[0].getString() == "a" );
   TEST_ADD( res, t[1].getString() == "b" );
   TEST_ADD( res, t.get(2).getString() == "c" );
   TEST_ADD( res, t[3].isNil() );

   t[4] = ConstString("e");
   TEST_ADD( res, t.size() == 7 );
   TEST_ADD( res, t.mapSize() == 2 );
   TEST_ADD( res, t.arraySize() == 5 );
   TEST_ADD( res, t[0].getString() == "a" );
   TEST_ADD( res, t[1].getString() == "b" );
   TEST_ADD( res, t[2].getString() == "c" );
   TEST_ADD( res, t[3].isNil() );
   TEST_ADD( res, t[4].getString() == "e" );

   const Table&   tc = t;
   const Variant& vc = tc[5];
   TEST_ADD( res, vc.isNil() );
   TEST_ADD( res, t.arraySize() == 5 ); // Accessing from a const Table& won't grow it.

   t.clear();
   TEST_ADD( res, t.size() == 0 );
   TEST_ADD( res, t.mapSize() == 0 );
   TEST_ADD( res, t.arraySize() == 0 );
   t[1] = Variant("one");
   TEST_ADD( res, t.size() == 2 );
   TEST_ADD( res, t.mapSize() == 0 );
   TEST_ADD( res, t.arraySize() == 2 );
   TEST_ADD( res, t[0].isNil() );
   TEST_ADD( res, t[1].getString() == "one" );
   TEST_ADD( res, t[2].isNil() );

   t.clear();
   t[0] = "a";
   t[1] = "b";
   t["key1"] = 1.0f;
   t["key2"] = "two";
   RCP<Table> t2p = new Table();
   Table& t2 = *t2p;
   t2[0] = "c";
   t2["key1"] = "one";
   t2["key3"] = "three";
   t.extend(t2);
   TEST_ADD( res, t.size() == 6 );
   TEST_ADD( res, t[0].getString() == "a" );
   TEST_ADD( res, t[1].getString() == "b" );
   TEST_ADD( res, t[2].getString() == "c" );
   TEST_ADD( res, t["key1"].getString() == "one" );
   TEST_ADD( res, t["key2"].getString() == "two" );
   TEST_ADD( res, t["key3"].getString() == "three" );
}

//------------------------------------------------------------------------------
//!
void  init_misc()
{
   Test::Collection& std = Test::standard();
   std.add( new Test::Function( "angles",   "Tests angle conversions",                   cgmath_angles   ) );
   std.add( new Test::Function( "clamp",    "Tests the clamp*() routines",               cgmath_clamp    ) );
   std.add( new Test::Function( "fixed",    "Tests the fixedPrec() routine",             cgmath_fixed    ) );
   std.add( new Test::Function( "nextpow2", "Tests next power of 2 routine",             cgmath_nextPow2 ) );
   std.add( new Test::Function( "math",     "Tests various routines from CGMath/Math.h", cgmath_math     ) );
   std.add( new Test::Function( "misc",     "Tests various stuff (swap routine, NaNs)",  cgmath_misc     ) );
   std.add( new Test::Function( "variant",  "Tests the Variant-related clases",          cgmath_variant  ) );

   Test::Collection& spc = Test::special();
   spc.add( new Test::Function( "atan2", "Compares various arctan implementations", cgmath_atan2 ) );
   spc.add( new Test::Function( "perf",  "Some performance tests",                  cgmath_perf  ) );
}
