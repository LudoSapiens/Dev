/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/Dbg/UnitTest.h>

#include <CGMath/AABBox.h>
#include <CGMath/Distributions.h>
#include <CGMath/Noise.h>
#include <CGMath/Random.h>
#include <CGMath/Range.h>

#include <Base/Util/Timer.h>

USING_NAMESPACE

//DBG_STREAM( os_test, "Test" );

//------------------------------------------------------------------------------
//!
void cgmath_randoms( Test::Result& res )
{
   RNG_WELL well;
   for( uint i = 0 ; i < 10; ++i )
   {
      double v = well( UniformReal<>(4.0, 5.0) );
      TEST_ADD( res, CGM::inRange( v, 4.0, 5.0 ) );
   }

   UniformHemisphere<>  hemi(0, 1, 0);
   for( uint i = 0 ; i < 10; ++i )
   {
      Vec3d v = well( hemi );
      TEST_ADD( res,
                CGM::inRange(v.x, -1.0, +1.0) &&
                v.y >= 0.0                       &&
                CGM::inRange(v.z, -1.0, +1.0) );
   }
}

//------------------------------------------------------------------------------
//!
void cgmath_randoms_performance( Test::Result& res )
{
   printf("\n");
   Timer timer;
   uint bins[256];
   uint n = 1 << 25;
   double t;

   double sum;
   double exp_sum = n * 0.5;
   double sum_error;
   double sum_error_threshold = 0.015/100.0;

   RNG_MT mt;
   memset( bins, 0, sizeof(bins) );
   sum = 0.0;
   timer.restart();
   for( uint i = 0; i < n; ++i )
   {
      sum += mt();
      //++bins[u & (0xFF<<0)];
   }
   t = timer.elapsed();
   sum_error = sum/exp_sum - 1.0;
   printf("Mersenne Twister: %fs (%g gens/s)  (sum=%g, err=%+g%%)\n", t, n/t, sum, sum_error*100.0 );
   TEST_ADD( res, CGM::equal(sum_error, 0.0, sum_error_threshold) );
   /**
   // The numbers below should be about the same.
   for( uint i = 0; i < 256; ++i )
   {
      printf("[%3d]: %d\n", i, bins[i]);
   }
   **/

   RNG_WELL well;
   memset( bins, 0, sizeof(bins) );
   sum = 0.0;
   timer.restart();
   for( uint i = 0; i < n; ++i )
   {
      sum += well();
      /**
      int idx = (uint)( d * 254 ) + 1;
      if( d <= 0.0 ) idx = 0;
      else
      if( d >= 1.0 ) idx = 255;
      ++bins[idx];
      **/
   }
   t = timer.elapsed();
   sum_error = sum/exp_sum - 1.0;
   printf("WELL: %fs (%g gens/s)  (sum=%g, err=%+g%%)\n", t, n/t, sum, sum_error*100.0 );
   TEST_ADD( res, CGM::equal(sum_error, 0.0, sum_error_threshold) );
   /**
   // The numbers below should be about the same.
   for( uint i = 0; i < 256; ++i )
   {
      printf("[%3d]: %d\n", i, bins[i]);
   }
   **/
}

//------------------------------------------------------------------------------
//!
void cgmath_distributions( Test::Result& res )
{
   Uniform01<>  u01;  //cannot put u01() else g++ give a 'too many arguments to function' error
   TEST_ADD( res, u01(0.0 ) == 0.0 );
   TEST_ADD( res, u01(0.25) == 0.25 );
   TEST_ADD( res, u01(1.0 ) == 1.0 );

   UniformReal<>  ud( 2.0, 4.0 );
   TEST_ADD( res, ud(0.0 ) == 2.0 );
   TEST_ADD( res, ud(0.25) == 2.5 );
   TEST_ADD( res, ud(1.0 ) == 4.0 );

   RNG_WELL  gen;
   UniformSphere<> sphDist;
   Vec3d n;
   n = gen( sphDist );
   TEST_ADD( res, CGM::equal(n.length(), 1.0) );
   for( uint i = 0; i < 10; ++i )
   {
      // Check sphere.
      n = gen( sphDist );
      //printf("%g, %g, %g (%g)\n", n.x, n.y, n.z, n.length());
      TEST_ADD( res, CGM::equal(n.length(), 1.0) );

      // Check hemisphere.
      n = gen( sphDist, Vec3d(0.0, 1.0, 0.0) );
      //printf("%g, %g, %g (%g)\n", n.x, n.y, n.z, n.length());
      TEST_ADD( res, CGM::equal(n.length(), 1.0) );
      TEST_ADD( res, n.y >= 0.0 );
   }

   UniformSphere<float>  sphDistF;
   gen.seed( 1 );
   Vec3f nf;
   nf = gen( sphDistF );
   TEST_ADD( res, CGM::equal(nf.length(), 1.0f) );
   for( uint i = 0; i < 10; ++i )
   {
      // Check sphere.
      nf = gen( sphDistF );
      //printf("%g, %g, %g (%g)\n", nf.x, nf.y, nf.z, nf.length());
      TEST_ADD( res, CGM::equal(nf.length(), 1.0f) );

      // Check hemisphere.
      nf = gen( sphDistF, Vec3f(0.0, 0.0, -1.0) );
      //printf("%g, %g, %g (%g)\n", nf.x, nf.y, nf.z, nf.length());
      TEST_ADD( res, CGM::equal(nf.length(), 1.0f) );
      TEST_ADD( res, nf.z <= 0.0 );
   }

   Vec3f p( -1.0f, -2.0f, -3.0f );
   Vec3f d(  3.0f,  2.0f,  1.0f );
   UniformSegment<float> segDistF( p, p+d );
   gen.seed( 1 );
   Vec3f r, pToR;
   float dLenSq = d.sqrLength();
   d.normalize();
   for( uint i = 0; i < 10; ++i )
   {
      r = gen( segDistF );
      pToR = r - p;
      // Generated vector is parallel to d.
      TEST_ADD( res, CGM::equal( d.dot(pToR.getNormalized()), 1.0f ) );
      // Generated vector is not bigger.
      TEST_ADD( res, pToR.sqrLength() <= dLenSq );
   }

   Vec3f p0( 1.0f,   1.0f, -1.0f );
   Vec3f p1( 2.0f, 100.0f,  1.0f );
   UniformBox<float> boxDistF( p0, p1 );
   p = boxDistF( 0.0f, 0.0f, 0.0f );
   TEST_ADD( res, p == Vec3f(p0.x, p0.y, p0.z) );
   p = boxDistF( 1.0f, 1.0f, 1.0f );
   TEST_ADD( res, p == Vec3f(p1.x, p1.y, p1.z) );
   d = Vec3f( 0.25f, 0.50f, 0.75f );
   p = boxDistF( d ); // Need a typecast because there is a templated routine with a Generator.
   TEST_ADD( res, p == (p0*(Vec3f(1.0f)-d) + p1*d) );
   AABBoxf box( p0, p1 );
   for( uint i = 0; i < 10; ++i )
   {
      p = gen( boxDistF );
      TEST_ADD( res, box.isInside(p) );
   }

   UniformAvgVar<float> avgVarDist( -1.0f, 2.0f );
   TEST_ADD( res, avgVarDist.average() == -1.0f );
   TEST_ADD( res, avgVarDist.variance() ==  2.0f );
   TEST_ADD( res, CGM::equal(avgVarDist(0.00f), -3.0f) );
   TEST_ADD( res, CGM::equal(avgVarDist(0.25f), -2.0f) );
   TEST_ADD( res, CGM::equal(avgVarDist(0.50f), -1.0f) );
   TEST_ADD( res, CGM::equal(avgVarDist(0.75f),  0.0f) );
   TEST_ADD( res, CGM::equal(avgVarDist(1.00f),  1.0f) );
}

//------------------------------------------------------------------------------
//!
void cgmath_perlin( Test::Result& /*res*/ )
{
   StdErr << nl;
   RNG_WELL  rng;
   Vec3d rngScale = Vec3d(  2.0f );
   Vec3d rngBias  = Vec3d( -0.5f );
   Vec3d rngVec3d;
   Vec3d rngVec3f;
   uint n = (1 << 12);

   {
      //StdErr << "perlinNoise1" << nl;
      Rangef        range = Rangef::empty();
      Range<Vec3f> coords = Range<Vec3f>::empty();
      for( uint i = 0; i < n; ++i )
      {
         rngVec3d = Vec3d( rng(), rng(), rng() ) * rngScale + rngBias;
         rngVec3f = rngVec3d;
         float noise = CGM::perlinNoise1( rngVec3f );
         coords |= rngVec3f;
         range  |= noise;
         //StdErr << rngVec3f << " " << noise << nl;
      }
      StdErr << "perlinNoise1: " << range.min() << ", " << range.max()
             << " (coord " << coords.min() << ", " << coords.max() << ")" << nl;
   }

   {
      //StdErr << "perlinNoise2" << nl;
      Range<Vec2f> range  = Range<Vec2f>::empty();
      Range<Vec3f> coords = Range<Vec3f>::empty();
      for( uint i = 0; i < n; ++i )
      {
         rngVec3d = Vec3d( rng(), rng(), rng() ) * rngScale + rngBias;
         rngVec3f = rngVec3d;
         Vec2f noise = CGM::perlinNoise2( rngVec3f );
         coords |= rngVec3f;
         range  |= noise;
         //StdErr << v << " " << noise << nl;
      }
      StdErr << "perlinNoise2: " << range.min() << ", " << range.max()
             << " (coord " << coords.min() << ", " << coords.max() << ")" << nl;
   }

   {
      //StdErr << "perlinNoise3" << nl;
      Range<Vec3f> coords = Range<Vec3f>::empty();
      Range<Vec3f> range  = Range<Vec3f>::empty();
      for( uint i = 0; i < n; ++i )
      {
         rngVec3d = Vec3d( rng(), rng(), rng() ) * rngScale + rngBias;
         rngVec3f = rngVec3d;
         Vec3f noise = CGM::perlinNoise3( rngVec3f );
         coords |= rngVec3f;
         range  |= noise;
         //StdErr << v << " " << noise << nl;
      }
      StdErr << "perlinNoise3: " << range.min() << ", " << range.max()
             << " (coord " << coords.min() << ", " << coords.max() << ")" << nl;
   }
}

//------------------------------------------------------------------------------
//!
void  init_random()
{
   Test::Collection& std = Test::standard();
   std.add( new Test::Function( "randoms",       "Tests that randomized values get generated in range", cgmath_randoms       ) );
   std.add( new Test::Function( "distributions", "Tests random number distribution code",               cgmath_distributions ) );

   Test::Collection& spc = Test::special();
   spc.add( new Test::Function( "randoms_performance", "Testbench for random number generators",  cgmath_randoms_performance ) );
   spc.add( new Test::Function( "perlin"             , "Testbench for Perlin noise"            ,  cgmath_perlin              ) );
}
