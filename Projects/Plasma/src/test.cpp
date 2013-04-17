/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/Dbg/UnitTest.h>

#include <Plasma/Procedural/Tex.h>

#include <CGMath/Vec3.h>
#include <CGMath/CGMath.h>

#include <cmath>

USING_NAMESPACE


/*==============================================================================
   BOX-TRIANGLE INTERSECTION
==============================================================================*/
//! Based on a paper and code by Akenine-Moller.

//------------------------------------------------------------------------------
//!
inline bool axisTestX(
   const Vec3f& e,
   const Vec3f& ae,
   const Vec3f& v0,
   const Vec3f& v1,
   const Vec3f& boxHalfsize
)
{
   float p0 = e.z*v0.y - e.y*v0.z;
   float p1 = e.z*v1.y - e.y*v1.z;
   float min, max;

   if( p0 < p1 )
   {
      min = p0;
      max = p1;
   }
   else
   {
      min = p1;
      max = p0;
   }

   float rad = ae.z*boxHalfsize.y + ae.y*boxHalfsize.z;
   if( min > rad || max < -rad ) return true;
   return false;
}
//------------------------------------------------------------------------------
//!
inline bool axisTestY(
   const Vec3f& e,
   const Vec3f& ae,
   const Vec3f& v0,
   const Vec3f& v1,
   const Vec3f& boxHalfsize
)
{
   float p0 = -e.z*v0.x + e.x*v0.z;
   float p1 = -e.z*v1.x + e.x*v1.z;
   float min, max;

   if( p0 < p1 )
   {
      min = p0;
      max = p1;
   }
   else
   {
      min = p1;
      max = p0;
   }

   float rad = ae.z*boxHalfsize.x + ae.x*boxHalfsize.z;
   if( min > rad || max < -rad ) return true;
   return false;
}

//------------------------------------------------------------------------------
//!
inline bool axisTestZ(
   const Vec3f& e,
   const Vec3f& ae,
   const Vec3f& v0,
   const Vec3f& v1,
   const Vec3f& boxHalfsize
)
{
   float p0 = e.y*v0.x - e.x*v0.y;
   float p1 = e.y*v1.x - e.x*v1.y;
   float min, max;

   if( p0 < p1 )
   {
      min = p0;
      max = p1;
   }
   else
   {
      min = p1;
      max = p0;
   }

   float rad = ae.y*boxHalfsize.x + ae.x*boxHalfsize.y;
   if( min > rad || max < -rad ) return true;
   return false;
}

//------------------------------------------------------------------------------
//!
inline bool planeBoxOverlap( const Vec3f& n, const Vec3f& v, const Vec3f& boxHalfsize )
{
   Vec3f vmin;
   Vec3f vmax;

   for( uint a = 0; a < 3; ++a )
   {
      if( n(a) > 0.0f )
      {
         vmin(a) = -boxHalfsize(a) - v(a);
         vmax(a) =  boxHalfsize(a) - v(a);
      }
      else
      {
         vmin(a) =  boxHalfsize(a) - v(a);
         vmax(a) = -boxHalfsize(a) - v(a);
      }
   }

   float min = n.dot( vmin );
   if( min > 0.0f )
   {
      return false;
   }
   float max = n.dot( vmax );
   if( max >= 0.0f )
   {
      return true;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
inline void findMinMax( float x0, float x1, float x2, float& min, float& max )
{
   min = max = x0;
   if( x1 < min ) min = x1;
   if( x1 > max ) max = x1;
   if( x2 < min ) min = x2;
   if( x2 > max ) max = x2;
}

//------------------------------------------------------------------------------
//!
bool triBoxOverlap(
   const Vec3f& boxCenter,
   const Vec3f& boxHalfsize,
   const Vec3f& p0,
   const Vec3f& p1,
   const Vec3f& p2
)
{
   // Move triangle in box space.
   Vec3f v0 = p0 - boxCenter;
   Vec3f v1 = p1 - boxCenter;
   Vec3f v2 = p2 - boxCenter;

   // Edges.
   Vec3f e0 = v1-v0;
   Vec3f e1 = v2-v1;
   Vec3f e2 = v0-v2;

   // Testing triangles edges vs box edges.
   // 9 cases.
   Vec3f ae( CGM::abs( e0.x ), CGM::abs( e0.y ), CGM::abs( e0.z ) );
   if( axisTestX( e0, ae, v0, v2, boxHalfsize ) ) return false;
   if( axisTestY( e0, ae, v0, v2, boxHalfsize ) ) return false;
   if( axisTestZ( e0, ae, v1, v2, boxHalfsize ) ) return false;

   ae = Vec3f( CGM::abs( e1.x ), CGM::abs( e1.y ), CGM::abs( e1.z ) );
   if( axisTestX( e1, ae, v0, v2, boxHalfsize ) ) return false;
   if( axisTestY( e1, ae, v0, v2, boxHalfsize ) ) return false;
   if( axisTestZ( e1, ae, v0, v1, boxHalfsize ) ) return false;

   ae = Vec3f( CGM::abs( e2.x ), CGM::abs( e2.y ), CGM::abs( e2.z ) );
   if( axisTestX( e2, ae, v0, v1, boxHalfsize ) ) return false;
   if( axisTestY( e2, ae, v0, v1, boxHalfsize ) ) return false;
   if( axisTestZ( e2, ae, v1, v2, boxHalfsize ) ) return false;

   // Testing box faces (X,Y,Z).
   float min;
   float max;

   findMinMax( v0.x, v1.x, v2.x, min, max );
   if( min >= boxHalfsize.x || max < -boxHalfsize.x ) return false;

   findMinMax( v0.y, v1.y, v2.y, min, max );
   if( min >= boxHalfsize.y || max < -boxHalfsize.y ) return false;

   findMinMax( v0.z, v1.z, v2.z, min, max );
   if( min >= boxHalfsize.z || max < -boxHalfsize.z ) return false;

   // Testing triangle.
   Vec3f n = e0.cross( e1 );
   if( !planeBoxOverlap( n, v0, boxHalfsize ) ) return false;

   return true;
}

//------------------------------------------------------------------------------
//!
void testIntersection( Test::Result& )
{
   StdErr << '\n';
   Vec3f p0;
   Vec3f p1;
   Vec3f p2;

   p0 = Vec3f( 1.0f, 1.0f, -4.0f );
   p1 = Vec3f( 1.0f, 1.0f,  4.0f );
   p2 = Vec3f( -1.0f, -1.0f,  0.0f );

   Vec3f hsize( 2.0f, 2.0f, 2.0f );
   Vec3f c( -2.0f, -2.0f, -2.0f );

   //Vec3f epsilon( CGM::pow( 2.0f, -4.0f ), 0.0f, 0.0f );
   Vec3f epsilon( 0.0f );
   p0 += epsilon;
   p1 += epsilon;
   p2 += epsilon;

   StdErr << "Testing triangle: " << p0 << p1 << p2 << "\n";

   for( uint x = 0; x < 2; ++x )
   {
      for( uint y = 0; y < 2; ++y )
      {
         for( uint z = 0; z < 2; ++z )
         {
            Vec3f center = c + Vec3f( (x*2)*hsize.x, (y*2)*hsize.y, (z*2)*hsize.z );
            bool overlap = triBoxOverlap( center, hsize, p0, p1, p2 );

            StdErr << "box with center at " << center << " is " << (overlap ? "OVERLAPPING" : "NOT OVERLAPPING") << "\n";
         }
      }
   }
}

/*==============================================================================

==============================================================================*/

//------------------------------------------------------------------------------
//!
void testRounding( Test::Result& )
{
   printf("\n");
   for( float g = -5.0f; g < 5.0f; g += 1.0f )
   {
      int r = -10;
      float f;
      f = g + 0.0f;
      printf("f=%5.5f(%X) --> %5.5f, %5.12f(%X)\n", f, (uint)toBits(f), roundTo(f, 0), roundTo(f, r), (uint)toBits(roundTo(f, r)));
      f = g + 0.001f;
      printf("f=%5.5f(%X) --> %5.5f, %5.12f(%X)\n", f, (uint)toBits(f), roundTo(f, 0), roundTo(f, r), (uint)toBits(roundTo(f, r)));
      f = g + 0.0024f;
      printf("f=%5.5f(%X) --> %5.5f, %5.12f(%X)\n", f, (uint)toBits(f), roundTo(f, 0), roundTo(f, r), (uint)toBits(roundTo(f, r)));
      f = g + 0.25f;
      printf("f=%5.5f(%X) --> %5.5f, %5.12f(%X)\n", f, (uint)toBits(f), roundTo(f, 0), roundTo(f, r), (uint)toBits(roundTo(f, r)));
      f = g + 0.26f;
      printf("f=%5.5f(%X) --> %5.5f, %5.12f(%X)\n", f, (uint)toBits(f), roundTo(f, 0), roundTo(f, r), (uint)toBits(roundTo(f, r)));
      f = g + 0.49f;
      printf("f=%5.5f(%X) --> %5.5f, %5.12f(%X)\n", f, (uint)toBits(f), roundTo(f, 0), roundTo(f, r), (uint)toBits(roundTo(f, r)));
      f = g + 0.50f;
      printf("f=%5.5f(%X) --> %5.5f, %5.12f(%X)\n", f, (uint)toBits(f), roundTo(f, 0), roundTo(f, r), (uint)toBits(roundTo(f, r)));
      f = g + 0.51f;
      printf("f=%5.5f(%X) --> %5.5f, %5.12f(%X)\n", f, (uint)toBits(f), roundTo(f, 0), roundTo(f, r), (uint)toBits(roundTo(f, r)));
      f = g + 0.74f;
      printf("f=%5.5f(%X) --> %5.5f, %5.12f(%X)\n", f, (uint)toBits(f), roundTo(f, 0), roundTo(f, r), (uint)toBits(roundTo(f, r)));
      f = g + 0.75f;
      printf("f=%5.5f(%X) --> %5.5f, %5.12f(%X)\n", f, (uint)toBits(f), roundTo(f, 0), roundTo(f, r), (uint)toBits(roundTo(f, r)));
      f = g + 0.76f;
      printf("f=%5.5f(%X) --> %5.5f, %5.12f(%X)\n", f, (uint)toBits(f), roundTo(f, 0), roundTo(f, r), (uint)toBits(roundTo(f, r)));
      f = g + 0.9f;
      printf("f=%5.5f(%X) --> %5.5f, %5.12f(%X)\n", f, (uint)toBits(f), roundTo(f, 0), roundTo(f, r), (uint)toBits(roundTo(f, r)));
   }
}

/*==============================================================================

==============================================================================*/

Vec3i _dim[8] = {
   Vec3i(0,0,0),
   Vec3i(1,0,0),
   Vec3i(0,1,0),
   Vec3i(1,1,0),
   Vec3i(0,0,1),
   Vec3i(1,0,1),
   Vec3i(0,1,1),
   Vec3i(1,1,1)
};

int _pattern[8][28] = {
   {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,-1},
   {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,-1},
   {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,-1},
   {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,-1},
   {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,-1},
   {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,-1},
   {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,-1},
   {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,-1}
};

int _pattern2[8][14] = {
   {14,15,16,17,18,19,20,21,22,23,24,25,26,-1},
   {14,15,16,17,18,19,20,21,22,23,24,25,26,-1},
   {14,15,16,17,18,19,20,21,22,23,24,25,26,-1},
   {14,15,16,17,18,19,20,21,22,23,24,25,26,-1},
   {14,15,16,17,18,19,20,21,22,23,24,25,26,-1},
   {14,15,16,17,18,19,20,21,22,23,24,25,26,-1},
   {14,15,16,17,18,19,20,21,22,23,24,25,26,-1},
   {14,15,16,17,18,19,20,21,22,23,24,25,26,-1}
};

int _pattern3[8][14] = {
   {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
   {14,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
   {15,16,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
   {14,15,16,17,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
   {18,19,21,22,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
   {14,18,19,20,21,22,23,-1,-1,-1,-1,-1,-1,-1},
   {15,16,18,19,21,22,24,25,-1,-1,-1,-1,-1,-1},
   {14,15,16,17,18,19,20,21,22,23,24,25,26,-1}
};


Vec3i _offset[27] =
{
   Vec3i(-1,-1,-1),
   Vec3i( 0,-1,-1),
   Vec3i( 1,-1,-1),
   Vec3i(-1, 0,-1),
   Vec3i( 0, 0,-1),
   Vec3i( 1, 0,-1),
   Vec3i(-1, 1,-1),
   Vec3i( 0, 1,-1),
   Vec3i( 1, 1,-1),

   Vec3i(-1,-1, 0),
   Vec3i( 0,-1, 0),
   Vec3i( 1,-1, 0),
   Vec3i(-1, 0, 0),
   Vec3i( 0, 0, 0),
   Vec3i( 1, 0, 0),
   Vec3i(-1, 1, 0),
   Vec3i( 0, 1, 0),
   Vec3i( 1, 1, 0),

   Vec3i(-1,-1, 1),
   Vec3i( 0,-1, 1),
   Vec3i( 1,-1, 1),
   Vec3i(-1, 0, 1),
   Vec3i( 0, 0, 1),
   Vec3i( 1, 0, 1),
   Vec3i(-1, 1, 1),
   Vec3i( 0, 1, 1),
   Vec3i( 1, 1, 1)
};

//------------------------------------------------------------------------------
//!
uint collisiond( const Vec3i& p0, const Vec3i& s0, const Vec3i& p1, const Vec3i& s1 )
{
   Vec3i p00 = p0 + s0;
   Vec3i p11 = p1 + s1;

   if( p00.x < p1.x || p0.x > p11.x ) return 0;
   if( p00.y < p1.y || p0.y > p11.y ) return 0;
   if( p00.z < p1.z || p0.z > p11.z ) return 0;
   return 1;
}

//------------------------------------------------------------------------------
//!
uint collision1( const Vec3i& p0, const Vec3i& s0, const Vec3i& p1, const Vec3i& s1 )
{
   uint pID = s0.x | (s0.y<<1) | (s0.z<<2);
   int* p   = _pattern[pID];

   for( ; *p != -1; ++p )
   {
      if( (p0+_offset[*p]) == p1 )
      {
         return collisiond( p0, s0, p1, s1 );
      }
   }
   return 0;
}

//------------------------------------------------------------------------------
//!
int numCell = 0;
uint collision2( const Vec3i& p0, const Vec3i& s0, const Vec3i& p1, const Vec3i& s1 )
{
   uint numCol = 0;

   if( p0 == p1 ) ++numCol;
   ++numCell;

   uint pID;
   int* p;

   // p0
   pID = s0.x | (s0.y<<1) | (s0.z<<2);
   p   = _pattern3[pID];
   for( ; *p != -1; ++p )
   {
      ++numCell;
      if( (p0+_offset[*p]) == p1 ) numCol += collisiond( p0, s0, p1, s1 );
   }

   // p1
   pID = s1.x | (s1.y<<1) | (s1.z<<2);
   p   = _pattern3[pID];
   for( ; *p != -1; ++p )
   {
      ++numCell;
      if( (p1+_offset[*p]) == p0 ) numCol += collisiond( p0, s0, p1, s1 );
   }

   return numCol;
}

//------------------------------------------------------------------------------
//!
void testCollision( Test::Result& )
{
   StdErr << '\n';
   uint total     = 0;
   uint totalCol  = 0;
   uint totalCol1 = 0;
   uint totalCol2 = 0;
   for( uint s0 = 0; s0 < 8; ++s0 )
   {
      for( int x = -1; x <= 1; ++x )
      {
         for( int y = -1; y <= 1; ++y )
         {
            for( int z = -1; z <= 1; ++z )
            {
               for( uint s1 = 0; s1 < 8; ++s1, ++total )
               {
                  Vec3i p0 = Vec3i( 0, 0, 0 );
                  Vec3i p1 = Vec3i( x, y, z );
                  uint numCol  = collisiond( p0, _dim[s0], p1, _dim[s1] );
                  uint numCol1 = collision1( p0, _dim[s0], p1, _dim[s1] );
                  uint numCol2 = collision2( p0, _dim[s0], p1, _dim[s1] );
                  totalCol  += numCol;
                  totalCol1 += numCol1;
                  totalCol2 += numCol2;
               }
            }
         }
      }
   }
   StdErr << "Collisions: " << totalCol  << " on " << total << "\n";
   StdErr << "Collisions: " << totalCol1 << " on " << total << "\n";
   StdErr << "Collisions: " << totalCol2 << " on " << total << "\n";
   StdErr << "Visited cells: " << numCell << "\n";
}

//------------------------------------------------------------------------------
//!
void testTex( Test::Result& )
{
   StdErr << '\n';

   RCP<Bitmap> src = new Bitmap( "../../Data/texture/test/RGBw.png" );

   Vec2i size( 32, 32 );
   RCP<Bitmap> dst = new Bitmap( size, Bitmap::BYTE, 3 );

   Vec2i pos;
   Vec2f st;
   uchar* dstRGB = dst->pixels();
   for( pos.y = 0; pos.y < size.y; ++pos.y )
   {
      st.y  = pos.y + 0.5f;
      st.y /= size.y;
      for( pos.x = 0; pos.x < size.x; ++pos.x )
      {
         st.x      = pos.x + 0.5f;
         st.x     /= size.x;
         Vec4f col = Tex::bilinear( *src, st );
         col      *= 255.0f;
         col      += 0.5f;
         *dstRGB++ = (uchar)col.x;
         *dstRGB++ = (uchar)col.y;
         *dstRGB++ = (uchar)col.z;
      }
   }

   dst->saveFile( "tex" );
}

//------------------------------------------------------------------------------
//!
void initTests()
{
   Test::Collection& spc = Test::special();
   spc.add( new Test::Function( "collision"   , "Collision"   , testCollision    ) );
   spc.add( new Test::Function( "intersection", "Intersection", testIntersection ) );
   spc.add( new Test::Function( "rounding"    , "Rounding"    , testRounding     ) );
   spc.add( new Test::Function( "tex"         , "Tex"         , testTex          ) );
}

//------------------------------------------------------------------------------
//!
int main( int argc, char** argv )
{
   initTests();
   return Test::main( argc, argv );
}
