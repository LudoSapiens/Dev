/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <CGMath/Dist.h>

#include <CGMath/Plane.h>

#include <Base/Dbg/DebugStream.h>

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_cgmath, "CGMath" );

UNNAMESPACE_END

NAMESPACE_BEGIN

namespace CGM
{


//------------------------------------------------------------------------------
//!
void
triangle_point_closestPoint
( const Vec3f& triA, const Vec3f& triB, const Vec3f& triC,
  const Vec3f& pt,
  Vec3f& closestPt )
{
   DBG_BLOCK( os_cgmath, "triangle_point_closestPointAndDistanceSq" );
   //printf("A: (%g, %g, %g)  B: (%g, %g, %g)  C: (%g, %g, %g)\n",
   //       triA(0), triA(1), triA(2),
   //       triB(0), triB(1), triB(2),
   //       triC(0), triC(1), triC(2));

   // Start by finding the supporting plane for the triangle
   Planef triPlane(triA, triB, triC);
   triPlane.normalize();
   //printf("Plane: dir(%g, %g, %g)  d=%g\n", triPlane.direction()(0), triPlane.direction()(1), triPlane.direction()(2), triPlane.d());

   // Find the closest point on the triangle plane
   Vec3f triClosest;
   triClosest = triPlane.closest(pt);
   //printf("TriC: %g %g %g\n", triClosest(0), triClosest(1), triClosest(2));

#if 0
   //Calculate barycentric coordinates
   /**
    * From "http://en.wikipedia.org/wiki/Barycentric_coordinates_(mathematics)"
    *
    * x = i*x1 + j*x2 + k*x3
    * y = i*y1 + j*y2 + k*y3
    * z = i*z1 + j*z2 + k*z3
    *
    * i + j + k = 1
    *
    * i = B(F+I) - C(E+H)
    *     ---------------
    *     A(E+H) - B(D+G)
    *
    * j = A(F+I) - C(D+G)
    *     ---------------
    *     B(D+G) - A(E+H)
    *
    * A = x1 - x3
    * B = x2 - x3
    * C = x3 - x
    * D = y1 - y3
    * E = y2 - y3
    * F = y3 - y
    * G = z1 - z3
    * H = z2 - z3
    * I = z3 - z
    *
    * The above breaks when A == B == 0; if so, switch A<->D, B<->E, C<->F.
    *
    * And in the code below, we have:
    *    c_a = (A, D, G)
    *    c_b = (B, E, H)
    *    p_c = (C, F, I)
    */

   Vec3f c_a = triA - triC;
   Vec3f c_b = triB - triC;
   Vec3f p_c = triC - pt;
   float i, i_num, i_denom;
   float j, j_num, j_denom;
   float k;
   if( CGM::equal(c_a(0), 0.0f) && CGM::equal(c_b(0), 0.0f) )
   {
      //Avoid divide by 0 by swapping A<->D, B<->E, C<->F
      float DpG = c_a(0) + c_a(2);
      float EpH = c_b(0) + c_b(2);
      float FpI = p_c(0) + p_c(2);
      i_num   = c_b(1)*FpI - p_c(1)*EpH;
      i_denom = c_a(1)*EpH - c_b(1)*DpG;
      j_num   = c_a(1)*FpI - p_c(1)*DpG;
      j_denom = c_b(1)*DpG - c_a(1)*EpH;
   }
   else
   {
      float DpG = c_a(1) + c_a(2);
      float EpH = c_b(1) + c_b(2);
      float FpI = p_c(1) + p_c(2);
      i_num   = c_b(0)*FpI - p_c(0)*EpH;
      i_denom = c_a(0)*EpH - c_b(0)*DpG;
      j_num   = c_a(0)*FpI - p_c(0)*DpG;
      j_denom = c_b(0)*DpG - c_a(0)*EpH;
   }
   i = i_num/i_denom;
   j = j_num/j_denom;
   k = 1.0f - i - j;
   printf("I: %g  J: %g  K: %g\n", i, j, k);
   if( i < 0.0f )
   {
      if( j < 0.0f )
      {
         // Closer to C
         closestPt = triC;
      }
      else
      {
         // Closer to BC
      }
   }
   else
   if( j < 0.0f )
   {
      if( k < 0.0f )
      {
         // Closer to A
         closestPt = triA;
      }
      else
      {
         // Closer to CA
      }
   }
   else
   if( k < 0.0f )
   {
      // Closer to AB
   }
   else
   {
      // Else, the point on the triangle is actually the closest
      DBG_MSG( os_cgmath, "Already on triangle" );
      closestPt = triClosest;
   }
#else
   // Create a plane for every edge pointing towards triClosest
   Vec3f b_c = triC - triB;
   Planef aPlane( b_c.cross(triPlane.direction()), triB );
   aPlane.normalize();

   Vec3f c_a = triA - triC;
   Planef bPlane( c_a.cross(triPlane.direction()), triC );
   bPlane.normalize();

   Vec3f a_b = triB - triA;
   Planef cPlane( a_b.cross(triPlane.direction()), triA );
   cPlane.normalize();


   Vec3f abPt,   bcPt,   caPt;
   float abDist, bcDist, caDist;

   // Check if we are beyond the AB edge
   cPlane.closestAndDistance(triClosest, abPt, abDist);
   if( abDist > 0.0f )
   {
      DBG_MSG( os_cgmath, "Beyond AB edge" );
      // Check if point projected on AB is beyond CA
      bPlane.closestAndDistance(abPt, caPt, caDist);
      if( caDist > 0.0f )
      {
         DBG_MSG( os_cgmath, "Closer to A" );
         //We're closer to A
         closestPt = triA;
         return;
      }

      // Check if point projected on AB is beyond BC
      aPlane.closestAndDistance(abPt, bcPt, bcDist);
      if( bcDist > 0.0f )
      {
         DBG_MSG( os_cgmath, "Closer to B" );
         //We're closer to B
         closestPt = triB;
         return;
      }

      // Else, the point projected on AB is the closest
      DBG_MSG( os_cgmath, "Closer to AB" );
      closestPt = abPt;
      return;
   }

   // Check if we are beyond the BC edge
   aPlane.closestAndDistance(triClosest, bcPt, bcDist);
   if( bcDist > 0.0f )
   {
      DBG_MSG( os_cgmath, "Beyond BC edge" );
      // Check if point projected on BC is beyond AB
      cPlane.closestAndDistance(bcPt, abPt, abDist);
      if( abDist > 0.0f )
      {
         DBG_MSG( os_cgmath, "Closer to B" );
         //We're closer to B
         closestPt = triB;
         return;
      }

      // Check if point projected on BC is beyond CA
      bPlane.closestAndDistance(bcPt, caPt, caDist);
      if( caDist > 0.0f )
      {
         DBG_MSG( os_cgmath, "Closer to C" );
         //We're closer to C
         closestPt = triC;
         return;
      }

      // Else, the point projected on BC is the closest
      DBG_MSG( os_cgmath, "Closer to BC" );
      closestPt = bcPt;
      return;
   }

   // Check if we are beyond the CA edge
   bPlane.closestAndDistance(triClosest, caPt, caDist);
   if( caDist > 0.0f )
   {
      DBG_MSG( os_cgmath, "Beyond CA edge" );
      // Check if point projected on CA is beyond BC
      aPlane.closestAndDistance(caPt, abPt, abDist);
      if( abDist > 0.0f )
      {
         DBG_MSG( os_cgmath, "Closer to C" );
         //We're closer to C
         closestPt = triC;
         return;
      }

      // Check if point projected on CA is beyond AB
      cPlane.closestAndDistance(caPt, abPt, abDist);
      if( abDist > 0.0f )
      {
         DBG_MSG( os_cgmath, "Closer to A" );
         //We're closer to A
         closestPt = triA;
         return;
      }

      // Else, the point projected on CA is the closest
      DBG_MSG( os_cgmath, "Closer to CA" );
      closestPt = caPt;
      return;
   }

   // Else, the point on the triangle is actually the closest
   DBG_MSG( os_cgmath, "Already on triangle" );
   closestPt = triClosest;
#endif //0
}

//-----------------------------------------------------------------------------
//! Computes the distance of a gradient front inside a pixel.
//! Equivalent to equation 4 of Gustavson et al.
//! "Anti-aliased Euclidean distance transform".
//! (a.k.a. edgeDF() function).
float  grayscaleDistance( float srcVal, const Vec2f& srcGrd )
{
   if( srcGrd.x*srcGrd.y == 0.0f )  return 0.5f - srcVal;

   // Normalize the gradient (just need the direction).
   //Vec2f gn = srcGrd.getNormalized();

   // Collapse all cases to +X,+Y quadrant.
   Vec2f ga = CGM::abs( srcGrd );
   if( ga.x < ga.y )  ga = ga(1, 0); // Swap X and Y.

   // Perform the calculation.
   float a1 = 0.5f * ga.y / ga.x;
   //StdErr << "ga=" << ga << " srcVal=" << srcVal << " a1=" << a1 << nl;
   if( srcVal < a1 )
   {
      // 0 <= a < a1
      //StdErr << "A" << nl;
      return 0.5f*(ga.x + ga.y) - CGM::sqrt( 2.0f * ga.x * ga.y * srcVal );
   }
   else
   if( srcVal < (1.0f-a1) )
   {
      // a1 <= srcVal <= 1-a1
      //StdErr << "B" << nl;
      return (0.5f - srcVal) * ga.x;
   }
   else
   {
      // 1-a1 < srcVal <= 1
      //StdErr << "C" << nl;
      return -0.5f*(ga.x + ga.y) + CGM::sqrt( 2.0f * ga.x * ga.y * (1.0f-srcVal) );
   }
}

//-----------------------------------------------------------------------------
//! Computes the distance between 2 pixels in a grayscale image (of floats).
//! It always computes the distance from the center of the sourse pixel to the
//! closest point on a transition front (i.e. edge) in the destination pixel.
//! The source value is assumed to be either 0.0f or 1.0f (handle gray before).
//! The destination value can be anything.
//! If both the source and destination are identical, we assume there is no
//! front separating the pixels, and we return +INF.
//! Otherwise, we determine the closest point on the front's line, and clamp it
//! to be inside the destination pixel bounding box.
//! If srcVal is 1.0f, we are inside the object and return a negative distance.
//! For the distance calculation, we use equation 6 of Gustavson et al.
//! "Anti-aliased Euclidean distance transform".
float  grayscaleDistance( const Vec2i& srcIdx, float srcVal, const Vec2i& dstIdx, float dstVal, const Vec2f& dstGrd )
{
   if( srcVal == dstVal )  return CGConstf::infinity();
#define GOOD_APPROX 1
#if GOOD_APPROX
   Vec2f  o = Vec2f(dstIdx - srcIdx);
   float df = CGM::grayscaleDistance( dstVal, dstGrd );
   Vec2f uv = dstGrd*df;
   float  d = length( o + uv );
   return (srcVal == 0.0f) ? d : -d;
#else
   Vec2f   o = Vec2f(dstIdx - srcIdx);
   float   t = CGM::pseudoCross( dstGrd, o );
   float  df = CGM::grayscaleDistance( dstVal, dstGrd );
   Vec2f  fr = Vec2f( dstGrd.y, -dstGrd.x ); // Perpendicular to the gradient (rot -90'), i.e. in the direction of the front.
   Vec2f  uv = dstGrd*df + fr*t; // Vec2f( -df*dstGrd.x + t*dstGrd.y, -df*dstGrd.y - t*dstGrd.x );
   float   d;
   if( CGM::abs(uv).max() <= 0.5f )
   {
      // Hit inside pixel.
      //StdErr << "    distance(" << srcIdx << "," << srcVal << "," << dstIdx << "," << dstVal << "," << dstGrd << ") t=" << t << " df=" << df << " fr=" << fr << " o=" << o << " uv=" << uv << " inside  >> " << length( uv + o ) << nl;
      d = length( o + uv );
   }
   else
   {
      // Hit outside pixel.
      //StdErr << "    distance(" << srcIdx << "," << srcVal << "," << dstIdx << "," << dstVal << "," << dstGrd << ") t=" << t << " df=" << df << " fr=" << fr << " o=" << o << " uv=" << uv << " outside >> " << length( o ) + df << nl;
      // Clamp uv to bounding box of pixel.
      // Move over fr until uv's largest component (worse dimension) hits -0.5 or 0.5 (the closest).
      uint mc = uv.maxComponent();
      float c = CGM::copySign( 0.5f, uv(mc) ); // The U or V value, clamped to 0.5, with the proper sign.
      float f = (uv(mc)-c)/fr(mc);
      uv -= fr*f;               // We offset by whichever amount moves mc to +/-0.5.
      //uv(  mc) = c;           // We clamp to 0.5 explicitly, so just assign it.
      //uv(1-mc) -= fr(1-mc)*f; // We only compute the offset of the other component.
      //StdErr << "mc=" << mc << " c=" << c << " uv=" << uv << nl;

      if( CGM::abs(uv(1-mc)) > 0.5f )
      {
         // Sometimes, because of fr's slope, the other component still isn't inside the pixel.
         mc  = 1-mc;
         c   = CGM::copySign( 0.5f, uv(mc) );
         f   = (uv(mc)-c)/fr(mc);
         uv -= fr*f;
      }
      CHECK( CGM::abs(uv.x) <= 0.501f && CGM::abs(uv.y) <= 0.501f );

      d = length( o + uv );
      // Use Gustavson's approximation.
      //d = length( o ) + df;
   }

   return (srcVal == 0.0f) ? d : -d;
#endif
}


} //namespace CGM

NAMESPACE_END
