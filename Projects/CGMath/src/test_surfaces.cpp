/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/Dbg/UnitTest.h>

#include <CGMath/HEALPix.h>
#include <CGMath/Plane.h>
#include <CGMath/Polygon.h>
#include <CGMath/Quadric.h>
#include <CGMath/Quat.h>
#include <CGMath/Sphere.h>

USING_NAMESPACE

//DBG_STREAM( os_test, "Test" );

//------------------------------------------------------------------------------
//!
void cgmath_HEALPix( Test::Result& res )
{
   int los = 8;
   
   Vec3d sum( Vec3d::zero() );
   
   int nside = HEALPix::faceLength( los );
   
   for( int f = 0; f < 12; ++f )
   {
      for( int x = 0; x < nside; ++x )
      {
         for( int y = 0; y < nside; ++y )
         {
            sum += Vec3d( HEALPix::pix2vec( los, f, x, y ) );
         }
      }
   }

   TEST_ADD( res, CGM::equal( sum.length(), 0.0, 0.0001 ) );
}

//------------------------------------------------------------------------------
//!
void cgmath_planes( Test::Result& res )
{
   Planef plane;
   Vec3f  p;
   Vec3f  c;
   float  d;

   //+Z over at 1
   plane = Planef( Vec3f(0, 0, 1), Vec3f(2, 0, 1), Vec3f(0, 2, 1) );
   TEST_ADD( res, plane.direction() == Vec3f(0, 0, 4) );
   TEST_ADD( res, plane.d() == -4 );
   TEST_ADD( res, !plane.contains(Vec3f(0,0,0)) );
   TEST_ADD( res, !plane.inFront(Vec3f(0,0,0)) );
   TEST_ADD( res, plane.contains(Vec3f(0,0,1)) );
   TEST_ADD( res, plane.contains(Vec3f(1,1,1)) );
   TEST_ADD( res, !plane.contains(Vec3f(2,2,2)) );
   TEST_ADD( res, plane.inFront(Vec3f(2,2,2)) );

   //45deg, then moved at origin
   plane = Planef( Vec3f(1,0,0), Vec3f(0,1,0), Vec3f(0,0,1) );
   TEST_ADD( res, plane.direction() == Vec3f(1, 1, 1) );
   TEST_ADD( res, plane.d() == -1 );
   TEST_ADD( res, !plane.contains(Vec3f(0,0,0)) );
   TEST_ADD( res, !plane.inFront(Vec3f(0,0,0)) );
   plane.d(0);
   TEST_ADD( res, plane.direction() == Vec3f(1, 1, 1) );
   TEST_ADD( res, plane.d() == 0 );
   TEST_ADD( res, plane.d() == 0 );
   TEST_ADD( res, plane.contains(Vec3f(0,0,0)) );

   //Checking normalization
   plane = Planef( Vec3f(1,0,0), Vec3f(0,1,0), Vec3f(0,0,1) );
   TEST_ADD( res, plane.direction() == Vec3f(1, 1, 1) );
   TEST_ADD( res, plane.d() == -1 );
   TEST_ADD( res, !plane.contains(Vec3f(0,0,0), 1e-6f) );
   TEST_ADD( res, !plane.inFront(Vec3f(0,0,0)) );
   TEST_ADD( res, plane.contains(Vec3f(1,0,0)) );
   TEST_ADD( res, plane.contains(Vec3f(0,1,0)) );
   TEST_ADD( res, plane.contains(Vec3f(0,0,1)) );
   TEST_ADD( res, !plane.contains(Vec3f(1,1,1), 1e-6f) );
   TEST_ADD( res, plane.inFront(Vec3f(1,1,1)) );
   plane.normalize();
   TEST_ADD( res, CGM::equal(plane.direction().length(), 1.0f, 1e-6f) );
   TEST_ADD( res, plane.direction()(0) == plane.direction()(1) );
   TEST_ADD( res, plane.direction()(1) == plane.direction()(2) );
   TEST_ADD( res, plane.direction()(2) == plane.direction()(0) );
   TEST_ADD( res, CGM::equal(plane.d(), -CGM::sqrt(1.0f/3.0f), 1e-6f) );
   TEST_ADD( res, !plane.contains(Vec3f(0,0,0)) );
   TEST_ADD( res, !plane.inFront(Vec3f(0,0,0)) );
   TEST_ADD( res, plane.contains(Vec3f(1,0,0), 1e-6f) );
   TEST_ADD( res, plane.contains(Vec3f(0,1,0), 1e-6f) );
   TEST_ADD( res, plane.contains(Vec3f(0,0,1), 1e-6f) );
   TEST_ADD( res, !plane.contains(Vec3f(1,1,1), 1e-6f) );
   TEST_ADD( res, plane.inFront(Vec3f(1,1,1)) );

   //Check distance to plane routines
   plane = Planef( Vec3f(0, 0, 1), Vec3f(2, 0, 1), Vec3f(0, 2, 1) );
   plane.normalize();  //need to call this prior to closest(Vec3f) and distance(Vec3f)
   p = Vec3f::zero();
   c = plane.closest(p);
   d = plane.distance(p);
   TEST_ADD( res, c.equal(Vec3f(0, 0, 1)) );
   TEST_ADD( res, CGM::equal(d, -1.0f) );
   p = Vec3f(1, 0, 0);
   c = plane.closest(p);
   d = plane.distance(p);
   TEST_ADD( res, c.equal(Vec3f(1, 0, 1)) );
   TEST_ADD( res, CGM::equal(d, -1.0f) );
   p = Vec3f(1, 1, 1);
   c = plane.closest(p);
   d = plane.distance(p);
   TEST_ADD( res, c.equal(Vec3f(1, 1, 1)) );
   TEST_ADD( res, CGM::equal(d, 0.0f) );
   p = Vec3f(1, 1, -1.5);
   c = plane.closest(p);
   d = plane.distance(p);
   TEST_ADD( res, c.equal(Vec3f(1, 1, 1)) );
   TEST_ADD( res, CGM::equal(d, -2.5f) );
   p = Vec3f(1, 1, +1.5);
   c = plane.closest(p);
   d = plane.distance(p);
   TEST_ADD( res, c.equal(Vec3f(1, 1, 1)) );
   TEST_ADD( res, CGM::equal(d, 0.5f) );
   //Different plane
   plane = Planef( Vec3f(3, 0, 0), Vec3f(0, 3, 0), Vec3f(0, 0, 3) );
   plane.normalize();  //need to call this prior to closest(Vec3f) and distance(Vec3f)
   p = Vec3f(0, 0, 0);
   c = plane.closest(p);
   d = plane.distance(p);
   TEST_ADD( res, c.equal(Vec3f(1, 1, 1)) );
   TEST_ADD( res, CGM::equal(d, -CGM::sqrt(3.0f)) );
   TEST_ADD( res, plane.contains(c, 1e-6f) );
   p = Vec3f(0, 3, 0) + Vec3f(1, 1, 1).rescale(-2.0f);
   c = plane.closest(p);
   d = plane.distance(p);
   TEST_ADD( res, c.equal(Vec3f(0, 3, 0)) );
   TEST_ADD( res, CGM::equal(d, -2.0f) );
   TEST_ADD( res, plane.contains(c, 1e-6f) );

   //Simply make sure this routine is the same as calling both separately
   plane.closestAndDistance(p, c, d);
   TEST_ADD( res, c == plane.closest(p) );
   TEST_ADD( res, CGM::equal(d, plane.distance(p), 0.0f) );
   //printf("c: %g, %g, %g\n", c(0), c(1), c(2));
   //printf("d: %g\n", d);

   /**
   printf("D=%g\n", plane.d());
   printf("Len=%g\n", plane.direction().length());
   printf("Ev=%g\n", plane.evaluate(Vec3f(0,0,0)));
   printf("Dir=%g %g %g\n",
          plane.direction()(0),
          plane.direction()(1),
          plane.direction()(2));
   **/
}


//------------------------------------------------------------------------------
//!
void cgmath_quadrics( Test::Result& res )
{
   Quadricd q;
   Vec3d    p;
   bool     ok;

   q = Quadricd( Planed( Vec3d(0, 1, 0), Vec3d(0, 0, 0) ), 1.0 );
   //Vec3d p( 1, 1, 0 );
   //double v = q.evaluate( p );
   //StdErr << "eval(" << p << ") = " << v << Endl;
   //StdErr << q.b() << Endl;
   TEST_ADD( res, CGM::equal(q.evaluate(0.0, 0.0, 0.0), 0.0) );
   TEST_ADD( res, CGM::equal(q.evaluate(0.0, 1.0, 0.0), 1.0) );
   TEST_ADD( res, CGM::equal(q.evaluate(1.0, 1.0, 0.0), 1.0) );
   TEST_ADD( res, CGM::equal(q.evaluate(1.0, 1.0, 1.0), 1.0) );
   TEST_ADD( res, q.b().equal( Vec3d(0.0, 0.0, 0.0) ) );

   // Corner of 3 planes, at (1, 2, 3).
   q  = Quadricd( Planed( Vec3d(1.0,0.0,0.0), Vec3d(1.0,0.0,0.0) ), 1.0 );
   q += Quadricd( Planed( Vec3d(0.0,1.0,0.0), Vec3d(0.0,2.0,0.0) ), 1.0 );
   q += Quadricd( Planed( Vec3d(0.0,0.0,1.0), Vec3d(0.0,0.0,3.0) ), 1.0 );
   ok = q.optimize( p );
   TEST_ADD( res, ok && p.equal( Vec3d(1.0, 2.0, 3.0) ) );

   // Edge of 2 planes, Y=1.
   q  = Quadricd( Planed( Vec3d(-1.0,1.0,0.0), Vec3d(0.0,1.0,0.0) ), 1.0 );
   q += Quadricd( Planed( Vec3d( 1.0,1.0,0.0), Vec3d(0.0,1.0,0.0) ), 1.0 );
   ok = q.optimize( p );
   TEST_ADD( res, !ok );
   // Just add some clamping quadrics in Z (very small area, yet enough).
   q += Quadricd( Planed( Vec3d( 0.0,0.0, 1.0), Vec3d(0.0,1.0, 1.0) ), 1e-5 );
   q += Quadricd( Planed( Vec3d( 0.0,0.0,-1.0), Vec3d(0.0,1.0,-1.0) ), 1e-5 );
   ok = q.optimize( p );
   TEST_ADD( res, ok && p.equal( Vec3d(0.0,1.0,0.0) ) );

   // Middle of a cell.
   q  = Quadricd( Planed( Vec3d(-1.0,0.0,0.0), Vec3d(0.0,0.0,0.0) ), 1.0 );
   q += Quadricd( Planed( Vec3d( 1.0,0.0,0.0), Vec3d(1.0,0.0,0.0) ), 1.0 );
   q += Quadricd( Planed( Vec3d(0.0,-1.0,0.0), Vec3d(0.0,1.0,0.0) ), 1.0 );
   q += Quadricd( Planed( Vec3d(0.0, 1.0,0.0), Vec3d(0.0,2.0,0.0) ), 1.0 );
   q += Quadricd( Planed( Vec3d(0.0,0.0,-1.0), Vec3d(0.0,0.0,2.0) ), 1.0 );
   q += Quadricd( Planed( Vec3d(0.0,0.0, 1.0), Vec3d(0.0,0.0,3.0) ), 1.0 );
   ok = q.optimize( p );
   TEST_ADD( res, ok && p.equal( Vec3d(0.5,1.5,2.5) ) );

   // Check impact of putting a cell around a feature point.
   q = Quadricd::zero();
   q += Quadricd( Planed( Vec3d(-1.0, 0.0, 0.0), Vec3d(0.1,0.2,0.3) ), 1.0 );
   q += Quadricd( Planed( Vec3d( 0.0,-1.0, 0.0), Vec3d(0.1,0.2,0.3) ), 1.0 );
   q += Quadricd( Planed( Vec3d( 0.0, 0.0,-1.0), Vec3d(0.1,0.2,0.3) ), 1.0 );
   q += Quadricd( Vec3d( 0.0, 0.1, 0.2 ), Vec3d( 0.2, 0.3, 0.4 ), 0.5 );
   ok = q.optimize( p );
   TEST_ADD( res, ok && p.equal( Vec3d(0.1, 0.2, 0.3) ) );
}

//------------------------------------------------------------------------------
//!
void cgmath_polygons( Test::Result& res )
{
   RCP<Polygonf> poly = new Polygonf();

   poly->addVertex( Vec3f(0.0f, 0.0f, 3.0f) );
   poly->addVertex( Vec3f(5.0f, 0.0f, 3.0f) );
   poly->addVertex( Vec3f(7.0f, 1.0f, 3.0f) );
   poly->addVertex( Vec3f(5.0f, 4.0f, 3.0f) );
   poly->addVertex( Vec3f(0.0f, 4.0f, 3.0f) );
   TEST_ADD( res, poly->numVertices() == 5 );
   poly->computeDerivedData();
   TEST_ADD( res, poly->normal().equal( Vec3f(0.0f, 0.0f, 1.0f) ) );
   TEST_ADD( res, poly->plane().d() == -3.0f );
   TEST_ADD( res, !poly->inFront ( Vec3f(0.0f, 0.0f, 0.0f) ) );
   TEST_ADD( res,  poly->inBack  ( Vec3f(0.0f, 0.0f, 0.0f) ) );
   TEST_ADD( res,  poly->isInside( Vec3f(0.0f, 0.0f, 0.0f) ) );
   TEST_ADD( res, !poly->isInside( Vec3f(9.0f, 0.0f, 0.0f) ) );
   TEST_ADD( res, !poly->inFront ( Vec3f(0.0f, 0.0f, 3.0f) ) );
   TEST_ADD( res,  poly->inBack  ( Vec3f(0.0f, 0.0f, 3.0f) ) );
   TEST_ADD( res,  poly->isInside( Vec3f(0.0f, 0.0f, 3.0f) ) );
   TEST_ADD( res, !poly->isInside( Vec3f(9.0f, 0.0f, 3.0f) ) );
   TEST_ADD( res,  poly->inFront ( Vec3f(0.0f, 0.0f, 4.0f) ) );
   TEST_ADD( res, !poly->inBack  ( Vec3f(0.0f, 0.0f, 4.0f) ) );
   TEST_ADD( res,  poly->isInside( Vec3f(0.0f, 0.0f, 4.0f) ) );
   TEST_ADD( res, !poly->isInside( Vec3f(9.0f, 0.0f, 4.0f) ) );
   // 5x4 (rect) + 2x4/2 (tri)
   TEST_ADD( res, poly->area() == 24.0f );

   poly->addVertex( Vec3f(-2.0f, 3.0f, 3.0f) );
   poly->computeDerivedData(); // Updates internal values.
   TEST_ADD( res, poly->computeCentroid().equal( Vec3f(2.5f, 2.0f, 3.0f) ) );
   // same as above, plus another 2x4/2 tri
   TEST_ADD( res, poly->area() == 28.0f );
   poly->addVertex( Vec3f( 0.0f, 2.0f, 3.0f) );
   poly->addVertex( Vec3f( 0.0f, 1.0f, 3.0f) );
   poly->addVertex( Vec3f(-1.0f, 0.0f, 3.0f) );
   poly->computeDerivedData(); // Updates internal values.
   // Previous - 2 (smaller tri) + 0.5
   TEST_ADD( res, poly->area() == 26.5f );

   poly->removeAllVertices();
   TEST_ADD( res, poly->numVertices() == 0 );

   // Same case as the first, with arbitrary rotation.
   Quatf q = Quatf::axisAngle( Vec3f(1.13f, -42.3f, 18.16f).normalize(), 1.3741f );
   poly->addVertex( q * Vec3f(0.0f, 0.0f, 3.0f) );
   poly->addVertex( q * Vec3f(5.0f, 0.0f, 3.0f) );
   poly->addVertex( q * Vec3f(7.0f, 1.0f, 3.0f) );
   poly->addVertex( q * Vec3f(5.0f, 4.0f, 3.0f) );
   poly->addVertex( q * Vec3f(0.0f, 4.0f, 3.0f) );
   poly->computeDerivedData();
   TEST_ADD( res, CGM::equal(poly->area(), 24.0f, 1e-5f) );
}

//------------------------------------------------------------------------------
//!
void cgmath_spheres( Test::Result& res )
{
   Spheref sph( Vec3f(1.0f, 2.0f, 3.0f), 2.0f );

   TEST_ADD( res, sph.center().equal( Vec3f(1.0, 2.0f, 3.0f) ) );
   TEST_ADD( res, sph.radius() == 2.0f );
   TEST_ADD( res, sph.isInside( sph.center() ) );
   TEST_ADD( res, !sph.isInside( Vec3f(0.0f, 0.0f, 0.0f) ) );
   TEST_ADD( res, sph.isInside( Vec3f(1.0f, 0.0f, 3.0f) ) );

   // Sphere-sphere intersection.
   Spheref sph2( Vec3f(-2.0f, 5.0f, 0.5f), 10.0f );
   TEST_ADD( res, sph.isOverlapping( sph2 ) );
   float d = (sph.center() - sph2.center()).length();
   sph2.radius( d - sph.radius() );
   TEST_ADD( res, sph.isOverlapping( sph2 ) );
   sph2.radius( sph2.radius() - 0.01f );
   TEST_ADD( res, !sph.isOverlapping( sph2 ) );
}

//------------------------------------------------------------------------------
//!
void  init_surfaces()
{
   Test::Collection& std = Test::standard();
   std.add( new Test::Function( "planes",   "Tests planes",   cgmath_planes   ) );
   std.add( new Test::Function( "polygons", "Tests polygons", cgmath_polygons ) );
   std.add( new Test::Function( "quadrics", "Tests quadrics", cgmath_quadrics ) );
   std.add( new Test::Function( "spheres",  "Tests spheres",  cgmath_spheres  ) );

   Test::Collection& spc = Test::special();
   spc.add( new Test::Function( "healpix", "Tests sanity on HEALPix class", cgmath_HEALPix ) );
}
