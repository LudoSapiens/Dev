/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/Dbg/UnitTest.h>

#include <CGMath/AABBox.h>
#include <CGMath/AARect.h>
#include <CGMath/BIH.h>
#include <CGMath/Frustum.h>
#include <CGMath/Range.h>

USING_NAMESPACE

//DBG_STREAM( os_test, "Test" );

//------------------------------------------------------------------------------
//!
void cgmath_AABBox( Test::Result& res )
{
   AABBoxf box = AABBoxf::empty();
   Vec3f n(-1, -2, -3);
   Vec3f z = Vec3f::zero();
   Vec3f p( 1,  2,  3);
   Vec3f o(p(0), n(1), p(2));

   TEST_ADD( res, !box.isInside(p) );
   box |= p;
   TEST_ADD( res, box.min(0) == 1 );
   TEST_ADD( res, box.max(0) == 1 );
   TEST_ADD( res, box.min(1) == 2 );
   TEST_ADD( res, box.max(1) == 2 );
   TEST_ADD( res, box.min(2) == 3 );
   TEST_ADD( res, box.max(2) == 3 );
   TEST_ADD( res, !box.isInside(n) );
   TEST_ADD( res, !box.isInside(z) );
   TEST_ADD( res,  box.isInside(p) );
   TEST_ADD( res, !box.isInside(o) );
   box |= z;
   TEST_ADD( res, box.min(0) == 0 );
   TEST_ADD( res, box.max(0) == 1 );
   TEST_ADD( res, box.min(1) == 0 );
   TEST_ADD( res, box.max(1) == 2 );
   TEST_ADD( res, box.min(2) == 0 );
   TEST_ADD( res, box.max(2) == 3 );
   TEST_ADD( res, !box.isInside(n) );
   TEST_ADD( res,  box.isInside(z) );
   TEST_ADD( res,  box.isInside(p) );
   TEST_ADD( res, !box.isInside(o) );
   box |= n;
   TEST_ADD( res, box.min(0) ==-1 );
   TEST_ADD( res, box.max(0) == 1 );
   TEST_ADD( res, box.min(1) ==-2 );
   TEST_ADD( res, box.max(1) == 2 );
   TEST_ADD( res, box.min(2) ==-3 );
   TEST_ADD( res, box.max(2) == 3 );
   TEST_ADD( res,  box.isInside(n) );
   TEST_ADD( res,  box.isInside(z) );
   TEST_ADD( res,  box.isInside(p) );
   TEST_ADD( res,  box.isInside(o) );
   TEST_ADD( res,  box.isInsideCO(n) );
   TEST_ADD( res,  box.isInsideCO(z) );
   TEST_ADD( res, !box.isInsideCO(p) );
   TEST_ADD( res, !box.isInsideCO(o) );
   TEST_ADD( res, !box.isInsideOO(n) );
   TEST_ADD( res,  box.isInsideOO(z) );
   TEST_ADD( res, !box.isInsideOO(p) );
   TEST_ADD( res, !box.isInsideOO(o) );
}

//------------------------------------------------------------------------------
//!
void cgmath_AARect( Test::Result& res )
{
   AARecti ri( Vec2i(1, 2), Vec2i(10, 20) );

   TEST_ADD( res, ri.position() ==  Vec2i( 1,  2) );
   TEST_ADD( res, ri.size()     ==  Vec2i(10, 20) );
   TEST_ADD( res, ri.left()     ==  1 );
   TEST_ADD( res, ri.right()    == 11 );
   TEST_ADD( res, ri.bottom()   ==  2 );
   TEST_ADD( res, ri.top()      == 22 );

   ri.left( -1 );  // Grows width by 2.
   TEST_ADD( res, (ri.position() == Vec2i(-1,  2)) && (ri.size() == Vec2i(12, 20)) );
   ri.right( 5 );
   TEST_ADD( res, (ri.position() == Vec2i(-1,  2)) && (ri.size() == Vec2i( 6, 20)) );
   ri.bottom( -2 ); // Grows height by 4.
   TEST_ADD( res, (ri.position() == Vec2i(-1, -2)) && (ri.size() == Vec2i( 6, 24)) );
   ri.top( 10 );
   TEST_ADD( res, (ri.position() == Vec2i(-1, -2)) && (ri.size() == Vec2i( 6, 12)) );

   AARectf rf = AARectf::empty();
   Vec2f   pfA = Vec2f( 1, 1 );
   Vec2f   pfB = Vec2f( 2, 1 );
   TEST_ADD( res, !rf.isInside(pfA) );
   TEST_ADD( res, !rf.isInside(pfB) );
   rf |= Vec2f( 1, 1 );
   TEST_ADD( res,  rf.isInside(pfA) );
   TEST_ADD( res, !rf.isInside(pfB) );
   rf |= Vec2f( 2, 1 );
   TEST_ADD( res,  rf.isInside(pfA) );
   TEST_ADD( res,  rf.isInside(pfB) );
}

//------------------------------------------------------------------------------
//!
void cgmath_BIH( Test::Result& res )
{
   Vector< Vec3f >     centers;
   Vector< AABBoxf >   boxes;
   Vector< AABBoxf* >  boxPtrs;

   centers.pushBack( Vec3f( 1.0f, 0.0f, 0.0f) );
   centers.pushBack( Vec3f( 3.0f, 0.0f, 0.0f) );
   centers.pushBack( Vec3f( 5.0f, 0.0f, 0.0f) );
   centers.pushBack( Vec3f(10.0f, 0.0f, 0.0f) );

   boxes.resize( centers.size() );
   boxPtrs.resize( centers.size() );
   for( uint i = 0; i < centers.size(); ++i )
   {
      AABBoxf& box = boxes[i];
      box.set( centers[i] );
      box.grow( 0.1f );
      boxPtrs[i] = &box;
   }

   BIH bih;
   TEST_ADD( res, bih.isEmpty() );

   bih.create( boxPtrs, centers, NULL );
   TEST_ADD( res, !bih.isEmpty() );

   AABBoxf box( Vec3f(2.0f, 0.0, 0.0), Vec3f(7.0f, 0.0f, 0.0f) );
   Vector< uint >  elems;
   bih.findElementsInside( box, elems );
   TEST_ADD( res, elems.size() == 2 );
   TEST_ADD( res, elems[0] == 1 );
   TEST_ADD( res, elems[1] == 2 );

   elems.clear();
   box.slabX() = Vec2f(4.0f, 7.0f); // Only 5.0f (aka idx=2) will match
   bih.findElementsInside( box, elems );
   TEST_ADD( res, elems.size() == 1 );
   TEST_ADD( res, elems[0] == 2 );

   elems.clear();
   box.slabX() = Vec2f(1.1f, 1.1f); // Only the first element
   bih.findElementsInside( box, elems );
   TEST_ADD( res, elems.size() == 1 );
   TEST_ADD( res, elems[0] == 0 );
}

//------------------------------------------------------------------------------
//!
void cgmath_frustum( Test::Result& res )
{
   const float s = 1.0f / 64.0f;
   //const float m = 1.0f / 8.0f;
   Frustumf frustum;
   AABBoxf  box;
   frustum = Frustumf(
      Reff::identity(),
      1.0f, 3.0f, // Front and back.
      1.0f, 1.0f, // FOY in X and Y.
      0.0f, 0.0f  // Shear in X and Y.
   );

   box.set( Vec3f(0.0f, 0.0f, 0.0f) );
   TEST_ADD( res, frustum.intersect(box) == Frustumf::OUTSIDE );
   box.set( Vec3f(0.0f, 0.0f, 1.0f) );
   TEST_ADD( res, frustum.intersect(box) == Frustumf::INSIDE  );
   box.grow( 5.0f );
   TEST_ADD( res, frustum.intersect(box) == Frustumf::INTERSECT );
   box.set( Vec3f(0.0f, 0.0f, 3.0f) );
   TEST_ADD( res, frustum.intersect(box) == Frustumf::INSIDE  );
   box.set( Vec3f(0.0f, 0.0f, 3.1f) );
   TEST_ADD( res, frustum.intersect(box) == Frustumf::OUTSIDE  );

   // Frustum pointing +X axis.
   frustum = Frustumf(
      Reff( Quatf::axisCir( Vec3f(0.0f, 1.0f, 0.0f), 0.25f ), Vec3f(1.0f, 2.0f, 3.0f) ),
      1.0f, 3.0f, // Front and back.
      CGM::degToRad( 45.0f ), CGM::degToRad( 30.0f )
   );
   box.set( Vec3f(0.0f, 0.0f, 0.0f) );
   TEST_ADD( res, frustum.intersect(box) == Frustumf::OUTSIDE );
   box.set( Vec3f(1.0f, 2.0f, 3.0f) );
   TEST_ADD( res, frustum.intersect(box) == Frustumf::OUTSIDE );
   box.set( Vec3f(1.0f, 2.0f, 3.0f) + Vec3f(1.0f, 0.0f, 0.0f) );
   TEST_ADD( res, frustum.intersect(box) == Frustumf::INSIDE  );
   box.set( Vec3f(1.0f, 2.0f, 3.0f) + Vec3f(2.99f, 0.0f, 0.0f) ); // 3.0 is OUTSIDE?!?
   TEST_ADD( res, frustum.intersect(box) == Frustumf::INSIDE  );
   box.set( Vec3f(1.0f, 2.0f, 3.0f) + Vec3f(3.1f, 0.0f, 0.0f) );
   TEST_ADD( res, frustum.intersect(box) == Frustumf::OUTSIDE );
   box.set( Vec3f(1.0f, 2.0f, 3.0f) + Vec3f(1.0f, -0.5f, -1.0f) ); // Bottom left near corner
   box.grow( s );
   TEST_ADD( res, frustum.intersect(box) == Frustumf::INTERSECT );
   box.set( Vec3f(1.0f, 2.0f, 3.0f) + Vec3f(3.0f, 1.5f, 3.0f) ); // Top right far corner
   box.grow( s );
   TEST_ADD( res, frustum.intersect(box) == Frustumf::INTERSECT );
   box.set( Vec3f(1.0f, 2.0f, 3.0f) + Vec3f(3.0f, 1.5f, 3.0f) + Vec3f(s, s, s) ); // Too far right, top, and away.
   TEST_ADD( res, frustum.intersect(box) == Frustumf::OUTSIDE );
   box.set( Vec3f(1.0f, 2.0f, 3.0f) + Vec3f(3.0f, 1.5f, 3.0f) + Vec3f(s, s, 0) ); // Too far right and top.
   TEST_ADD( res, frustum.intersect(box) == Frustumf::OUTSIDE );
   box.set( Vec3f(1.0f, 2.0f, 3.0f) + Vec3f(3.0f, 1.5f, 3.0f) + Vec3f(s, 0, s) ); // Too far right and away.
   TEST_ADD( res, frustum.intersect(box) == Frustumf::OUTSIDE );
   box.set( Vec3f(1.0f, 2.0f, 3.0f) + Vec3f(3.0f, 1.5f, 3.0f) + Vec3f(0, s, s) ); // Too far top and away.
   TEST_ADD( res, frustum.intersect(box) == Frustumf::OUTSIDE );
   box.set( Vec3f(1.0f, 2.0f, 3.0f) + Vec3f(3.0f, 1.5f, 3.0f) + Vec3f(s, 0, 0) ); // Too far right.
   TEST_ADD( res, frustum.intersect(box) == Frustumf::OUTSIDE );
   box.set( Vec3f(1.0f, 2.0f, 3.0f) + Vec3f(3.0f, 1.5f, 3.0f) + Vec3f(0, 0, s) ); // Too far away.
   TEST_ADD( res, frustum.intersect(box) == Frustumf::OUTSIDE );
   box.set( Vec3f(1.0f, 2.0f, 3.0f) + Vec3f(3.0f, 1.5f, 3.0f) + Vec3f(-s, -s, -s) ); // Barely top, right, and away.
   TEST_ADD( res, frustum.intersect(box) == Frustumf::INSIDE );
   box.set( Vec3f(1.0f, 2.0f, 3.0f) + Vec3f(3.0f, 1.5f, 3.0f) + Vec3f(-s, -s,  0) ); // Barely top and right.
   //TEST_ADD( res, frustum.intersect(box) == Frustumf::INSIDE );
   box.set( Vec3f(1.0f, 2.0f, 3.0f) + Vec3f(3.0f, 1.5f, 3.0f) + Vec3f(-s,  0, -s) ); // Barely top and away.
   TEST_ADD( res, frustum.intersect(box) == Frustumf::INSIDE );
   box.set( Vec3f(1.0f, 2.0f, 3.0f) + Vec3f(3.0f, 1.5f, 3.0f) + Vec3f( 0, -s, -s) ); // Barely top and away.
   //TEST_ADD( res, frustum.intersect(box) == Frustumf::INSIDE );
   box.set( Vec3f(1.0f, 2.0f, 3.0f) + Vec3f(3.0f, 1.5f, 3.0f) + Vec3f(-s,  0,  0) ); // Barely right.
   //TEST_ADD( res, frustum.intersect(box) == Frustumf::INSIDE );
   box.set( Vec3f(1.0f, 2.0f, 3.0f) + Vec3f(3.0f, 1.5f, 3.0f) + Vec3f( 0,  0, -s) ); // Barely away.
   //TEST_ADD( res, frustum.intersect(box) == Frustumf::INSIDE );

   // Frustum pointing +X axis, with zero shearing specified.
   frustum = Frustumf(
      Reff( Quatf::axisCir( Vec3f(0.0f, 1.0f, 0.0f), 0.25f ), Vec3f(1.0f, 2.0f, 3.0f) ),
      1.0f, 3.0f, // Front and back.
      CGM::degToRad( 45.0f ), CGM::degToRad( 30.0f ),
      0.0f, 0.0f
   );
   box.set( Vec3f(0.0f, 0.0f, 0.0f) );
   TEST_ADD( res, frustum.intersect(box) == Frustumf::OUTSIDE );
   box.set( Vec3f(1.0f, 2.0f, 3.0f) );
   TEST_ADD( res, frustum.intersect(box) == Frustumf::OUTSIDE );
   box.set( Vec3f(1.0f, 2.0f, 3.0f) + Vec3f(1.0f, 0.0f, 0.0f) );
   TEST_ADD( res, frustum.intersect(box) == Frustumf::INSIDE  );
   box.set( Vec3f(1.0f, 2.0f, 3.0f) + Vec3f(2.99f, 0.0f, 0.0f) ); // 3.0 is OUTSIDE?!?
   TEST_ADD( res, frustum.intersect(box) == Frustumf::INSIDE  );
   box.set( Vec3f(1.0f, 2.0f, 3.0f) + Vec3f(3.1f, 0.0f, 0.0f) );
   TEST_ADD( res, frustum.intersect(box) == Frustumf::OUTSIDE );
   box.set( Vec3f(1.0f, 2.0f, 3.0f) + Vec3f(1.0f, -0.5f, -1.0f) ); // Bottom left near corner
   box.grow( s );
   TEST_ADD( res, frustum.intersect(box) == Frustumf::INTERSECT );
   box.set( Vec3f(1.0f, 2.0f, 3.0f) + Vec3f(3.0f, 1.5f, 3.0f) ); // Top right far corner
   box.grow( s );
   TEST_ADD( res, frustum.intersect(box) == Frustumf::INTERSECT );
   box.set( Vec3f(1.0f, 2.0f, 3.0f) + Vec3f(3.0f, 1.5f, 3.0f) + Vec3f(s, s, s) ); // Too far right, top, and away.
   TEST_ADD( res, frustum.intersect(box) == Frustumf::OUTSIDE );
   box.set( Vec3f(1.0f, 2.0f, 3.0f) + Vec3f(3.0f, 1.5f, 3.0f) + Vec3f(s, s, 0) ); // Too far right and top.
   TEST_ADD( res, frustum.intersect(box) == Frustumf::OUTSIDE );
   box.set( Vec3f(1.0f, 2.0f, 3.0f) + Vec3f(3.0f, 1.5f, 3.0f) + Vec3f(s, 0, s) ); // Too far right and away.
   TEST_ADD( res, frustum.intersect(box) == Frustumf::OUTSIDE );
   box.set( Vec3f(1.0f, 2.0f, 3.0f) + Vec3f(3.0f, 1.5f, 3.0f) + Vec3f(0, s, s) ); // Too far top and away.
   TEST_ADD( res, frustum.intersect(box) == Frustumf::OUTSIDE );
   box.set( Vec3f(1.0f, 2.0f, 3.0f) + Vec3f(3.0f, 1.5f, 3.0f) + Vec3f(s, 0, 0) ); // Too far right.
   TEST_ADD( res, frustum.intersect(box) == Frustumf::OUTSIDE );
   box.set( Vec3f(1.0f, 2.0f, 3.0f) + Vec3f(3.0f, 1.5f, 3.0f) + Vec3f(0, 0, s) ); // Too far away.
   TEST_ADD( res, frustum.intersect(box) == Frustumf::OUTSIDE );
   box.set( Vec3f(1.0f, 2.0f, 3.0f) + Vec3f(3.0f, 1.5f, 3.0f) + Vec3f(-s, -s, -s) ); // Barely top, right, and away.
   TEST_ADD( res, frustum.intersect(box) == Frustumf::INSIDE );
   box.set( Vec3f(1.0f, 2.0f, 3.0f) + Vec3f(3.0f, 1.5f, 3.0f) + Vec3f(-s, -s,  0) ); // Barely top and right.
   //TEST_ADD( res, frustum.intersect(box) == Frustumf::INSIDE );
   box.set( Vec3f(1.0f, 2.0f, 3.0f) + Vec3f(3.0f, 1.5f, 3.0f) + Vec3f(-s,  0, -s) ); // Barely top and away.
   TEST_ADD( res, frustum.intersect(box) == Frustumf::INSIDE );
   box.set( Vec3f(1.0f, 2.0f, 3.0f) + Vec3f(3.0f, 1.5f, 3.0f) + Vec3f( 0, -s, -s) ); // Barely top and away.
   //TEST_ADD( res, frustum.intersect(box) == Frustumf::INSIDE );
   box.set( Vec3f(1.0f, 2.0f, 3.0f) + Vec3f(3.0f, 1.5f, 3.0f) + Vec3f(-s,  0,  0) ); // Barely right.
   //TEST_ADD( res, frustum.intersect(box) == Frustumf::INSIDE );
   box.set( Vec3f(1.0f, 2.0f, 3.0f) + Vec3f(3.0f, 1.5f, 3.0f) + Vec3f( 0,  0, -s) ); // Barely away.
   //TEST_ADD( res, frustum.intersect(box) == Frustumf::INSIDE );
}

//------------------------------------------------------------------------------
//!
void cgmath_Range( Test::Result& res )
{
   Rangei ri( 3 );

   TEST_ADD( res, ri.min()    == 3 );
   TEST_ADD( res, ri.max()    == 3 );
   TEST_ADD( res, ri.size()   == 0 );
   TEST_ADD( res, ri.center() == 3 );

   ri.min( 1 );
   TEST_ADD( res, ri.min()    == 1 );
   TEST_ADD( res, ri.max()    == 3 );
   TEST_ADD( res, ri.size()   == 2 );
   TEST_ADD( res, ri.center() == 2 );

   ri.max( 7 );
   TEST_ADD( res, ri.min()    == 1 );
   TEST_ADD( res, ri.max()    == 7 );
   TEST_ADD( res, ri.size()   == 6 );
   TEST_ADD( res, ri.center() == 4 );

   Rangef rf = Rangef::empty();
   TEST_ADD( res, !rf.isInside(1) );
   TEST_ADD( res, !rf.isInside(2) );
   rf |= 1.0f;
   TEST_ADD( res,  rf.isInside(1) );
   TEST_ADD( res, !rf.isInside(2) );
   rf |= 2.0f;
   TEST_ADD( res,  rf.isInside(1) );
   TEST_ADD( res,  rf.isInside(2) );
   TEST_ADD( res, rf.min()    == 1.0f );
   TEST_ADD( res, rf.max()    == 2.0f );
   TEST_ADD( res, rf.size()   == 1.0f );
   TEST_ADD( res, rf.center() == 1.5f );
}


//------------------------------------------------------------------------------
//!
void  init_boxes()
{
   Test::Collection& std = Test::standard();
   std.add( new Test::Function( "aabbox",  "Tests axis-aligned bounding boxes", cgmath_AABBox  ) );
   std.add( new Test::Function( "aarect",  "Tests 2D rectangles",               cgmath_AARect  ) );
   std.add( new Test::Function( "frustum", "Tests frustum class",               cgmath_frustum ) );
   std.add( new Test::Function( "range",   "Tests 1D ranges",                   cgmath_Range   ) );

   Test::Collection& spc = Test::special();
   spc.add( new Test::Function( "bih",     "Tests BIH class",                   cgmath_BIH     ) );
}
