/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Geometry/MetaSurface.h>
#include <Plasma/Geometry/ParametricPatch.h>
#include <Plasma/Geometry/MaterialSet.h>
#include <Plasma/Geometry/MetaGeometry.h>

#include <Fusion/VM/VMRegistry.h>

#include <CGMath/Plane.h>
#include <CGMath/Grid.h>

#include <Base/ADT/Cache.h>
#include <Base/ADT/StringMap.h>

#include <algorithm>

#define DBG_DETAILS       0
#define DBG_DETAILS_LOOP  0
#define DBG_DETAILS_ID    31137
#define DBG_DETAILS_UV    Vec2f(0.0f,0.0f)

//#define CONTROL_MESH 1
//#define PER_SUBPATCH_MAPPING

/*==============================================================================
   UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

uint  _debugLevel = 0;

enum {
   ATTRIB_ID,
   ATTRIB_FACE,
   ATTRIB_FACE_N,
   ATTRIB_N,
   ATTRIB_POS,
   ATTRIB_UV,
};

StringMap _attributes(
   "id",    ATTRIB_ID,
   "f",     ATTRIB_FACE,
   "fn",    ATTRIB_FACE_N,
   "n",     ATTRIB_N,
   "pos",   ATTRIB_POS,
   "uv",    ATTRIB_UV,
   ""
);

//------------------------------------------------------------------------------
//! 

enum { INVALID = 0xffffffff };
float _errormax = (1.0f/4096.0f);

#if 0
//------------------------------------------------------------------------------
//! 
inline int
startID( int a, int b )
{
   return ((a+1)%4) == b ? a : b;
}
#endif

//------------------------------------------------------------------------------
//! 
inline MetaSurface::Vertex* 
prevVertex( MetaSurface::Vertex* vs, MetaSurface::Vertex* ve, uint axis, float val )
{
   while( (vs->_uv(axis) < val) && (vs != ve) )
   {
      vs = vs->_next[axis+1];
   }
   return vs->_next[(axis+3)%4];
}

//------------------------------------------------------------------------------
//! 
inline float triArea( const Vec2f& a, const Vec2f& b, const Vec2f& c )
{
   Vec2f u = b-a;
   Vec2f v = c-a;
   return u.y*v.x - u.x*v.y;
}

//------------------------------------------------------------------------------
//! 
inline bool ccw( const Vec2f& a, const Vec2f& b, const Vec2f& c, float& area )
{
   area = triArea( a, b, c );
   return area < 0.0f;
}

//------------------------------------------------------------------------------
//! 
inline bool ccw( const Vec2f& a, const Vec2f& b, const Vec2f& c )
{
   return triArea( a, b, c ) < 0.0f;
}

//------------------------------------------------------------------------------
//! 
bool pointInTriangle( const Vec2f& p, const Vec2f& a, const Vec2f& b, const Vec2f& c )
{
   if( ccw( a, p, b ) ) return false;
   if( ccw( b, p, c ) ) return false;
   if( ccw( c, p, a ) ) return false;
   return true;
}

/*==============================================================================
   EAR CLIPPING
==============================================================================*/

float triangleCost(
   const Vector<MetaSurface::Vertex*>& vptrs, 
   const Vector<int>&                  prev,
   const Vector<int>&                  next,
   uint                                x,
   uint                                y,
   uint                                i
)
{
   int pi  = prev[i];
   int ni  = next[i];
   Vec2f a = vptrs[pi]->_pos(x,y);
   Vec2f b = vptrs[i]->_pos(x,y);
   Vec2f c = vptrs[ni]->_pos(x,y);

   // Is triangle convex (CCW)?
   float area;
   if( !ccw( a, b, c, area ) ) return CGConstf::infinity();

   // Is it an ear?
   int j = next[ni];
   do {
      // Is current vertex in triangle?
      if( pointInTriangle( vptrs[j]->_pos(x,y), a, b, c ) ) 
      {
         if( (vptrs[j] != vptrs[pi]) && (vptrs[j] != vptrs[i]) && (vptrs[j] != vptrs[ni]) )
            return CGConstf::infinity();
      }
      j = next[j];
   } while( j != pi );

   // Compute cost.
   if( area > -1e-7f ) return -1.0f/area;
   return CGM::abs(dot(b-a,c-a))+CGM::abs(dot(c-b,a-b))+CGM::abs(dot(b-c,a-c));
   //return CGM::abs(dot(c-b,a-b));
   //return (c-a).sqrLength()+area;
   //return area;
   //return CGM::dot(c-b,a-b)*area;
   //return CGM::abs( CGM::dot( (c-b).normalize(),(a-b).normalize() ) );
}

//------------------------------------------------------------------------------
//! 
float triangleCost2(
   const Vector<MetaSurface::Vertex*>& vptrs, 
   const Vector<int>&                  prev,
   const Vector<int>&                  next,
   uint                                x,
   uint                                y,
   uint                                i
)
{
   int pi  = prev[i];
   int ni  = next[i];
   Vec2f a = vptrs[pi]->_pos(x,y);
   Vec2f b = vptrs[i]->_pos(x,y);
   Vec2f c = vptrs[ni]->_pos(x,y);

   // Is triangle convex (CCW)?
   float area;
   if( !ccw( a, b, c, area ) ) return CGConstf::infinity();
   
   // Is it an ear?
   int j = next[ni];
   do {
      // Is current vertex in triangle?
      if( pointInTriangle( vptrs[j]->_pos(x,y), a, b, c ) ) 
      {
         if( (vptrs[j] != vptrs[pi]) && (vptrs[j] != vptrs[i]) && (vptrs[j] != vptrs[ni]) )
            return CGConstf::infinity();
      }
      j = next[j];
   } while( j != pi );
   // Compute cost.
   return area;
}

//------------------------------------------------------------------------------
//! 
void triangulate( 
   const Vector<MetaSurface::Vertex*>& vptrs,
   Vector<uint>&                       indices,
   bool                                flip,
   uint                                x,
   uint                                y
)
{
   uint n = uint(vptrs.size());

   if( n == 3 )
   {
      if( !flip )
      {     
         indices.pushBack( vptrs[0]->_id );
         indices.pushBack( vptrs[1]->_id );
         indices.pushBack( vptrs[2]->_id );
      }
      else
      {
         indices.pushBack( vptrs[0]->_id );
         indices.pushBack( vptrs[2]->_id );
         indices.pushBack( vptrs[1]->_id );
      }
      return;
   }

   if( n < 3 )
   {
      StdErr << "ERROR: loop with " << n << " vertices.\n";
      return;
   }

   // Create double link list of vertices.
   Vector<int> prev(n);
   Vector<int> next(n);
   for( uint i = 0; i < n; ++i )
   {
      prev[i] = i-1;
      next[i] = i+1;
   }
   prev[0]   = n-1;
   next[n-1] = 0;

   // Compute which vertices are ears.
   Vector<float> cost(n);
   for( uint i = 0; i < n; ++i )
   {
      cost[i] = triangleCost( vptrs, prev, next, x, y, i );
   }
   
   uint i = 0;
   while( 1 )
   {
      // Find the best ear.
      uint besti     = i;
      float bestCost = cost[i];
      uint j         = next[i];
      do {
         if( cost[j] < bestCost )
         {
            besti    = j;
            bestCost = cost[j];
         }
         j = next[j];
      } while( j != i );
      i = besti;

      // Create triangle.
      indices.pushBack( vptrs[prev[i]]->_id );
      if( !flip )
      {
         indices.pushBack( vptrs[i]->_id );
         indices.pushBack( vptrs[next[i]]->_id );
      }
      else
      {
         indices.pushBack( vptrs[next[i]]->_id );
         indices.pushBack( vptrs[i]->_id );
      }


      // Remove vertices.
      next[prev[i]] = next[i];
      prev[next[i]] = prev[i];
      --n;
      i = prev[i];

      // Update neighbor cost.
      if( n == 3 ) break;
      cost[i]       = triangleCost( vptrs, prev, next, x, y, i );
      cost[next[i]] = triangleCost( vptrs, prev, next, x, y, next[i] );
   }

   // Add last triangle.
   indices.pushBack( vptrs[prev[i]]->_id );
   if( !flip )
   {
      indices.pushBack( vptrs[i]->_id );
      indices.pushBack( vptrs[next[i]]->_id );
   }
   else
   {
      indices.pushBack( vptrs[next[i]]->_id );
      indices.pushBack( vptrs[i]->_id );
   }
}

//------------------------------------------------------------------------------
//! 
Vec3f computePoint( 
   const Vector<MetaSurface::Vertex*>& vptrs,
   uint                                x,
   uint                                y
)
{
   uint n = uint(vptrs.size());
   if( n == 3 ) return (vptrs[0]->_pos*0.25f + vptrs[1]->_pos*0.25f + vptrs[2]->_pos*0.5f);

   if( n < 3 )
   {
      StdErr << "ERROR: loop with " << n << " vertices.\n";
      return vptrs[0]->_pos;
   }

   // Create double link list of vertices.
   Vector<int> prev(n);
   Vector<int> next(n);
   for( uint i = 0; i < n; ++i )
   {
      prev[i] = i-1;
      next[i] = i+1;
   }
   prev[0]   = n-1;
   next[n-1] = 0;

   // Compute which vertices are ears.
   uint besti     = 0;
   float bestCost = CGConstf::infinity();
   Vector<float> cost(n);
   for( uint i = 0; i < n; ++i )
   {
      cost[i] = triangleCost2( vptrs, prev, next, x, y, i );
      if( cost[i] < bestCost )
      {
         besti    = i;
         bestCost = cost[i];
      }
   }
   Vec3f point = (vptrs[prev[besti]]->_pos*0.25f + vptrs[besti]->_pos*0.25f + vptrs[next[besti]]->_pos*0.5f);

   if( bestCost < -1e-5f || n <= 4 ) return point;

   // Find a better triangle.

   // Remove vertices.
   next[prev[besti]] = next[besti];
   prev[next[besti]] = prev[besti];
   --n;
   uint i = prev[besti];

   while( n > 3 )
   {
      // Update neighbot cost.
      cost[i]       = triangleCost2( vptrs, prev, next, x, y, i );
      cost[next[i]] = triangleCost2( vptrs, prev, next, x, y, next[i] );

      // Find the best ear;
      uint cbesti     = i;
      float cbestCost = cost[i];
      uint j          = next[i];
       do {
         if( cost[j] < cbestCost )
         {
            cbesti    = j;
            cbestCost = cost[j];
         }
         j = next[j];
      } while( j != i );
      i = cbesti;

      // Keep best point.
      if( cbestCost < bestCost )
      {
         bestCost = cbestCost;
         point    = (vptrs[prev[i]]->_pos + vptrs[i]->_pos + vptrs[next[i]]->_pos)/3.0f;
      }

      // Remove vertices.
      next[prev[i]] = next[i];
      prev[next[i]] = prev[i];
      --n;
      i = prev[i];
   }
   return point;
}

/*==============================================================================
   TRIANGULATION
==============================================================================*/

void triangulate( 
   const Vector<MetaSurface::Vertex*>& vptrs, 
   uint                                nbVertices[4],
   bool                                flip,
   Vector<uint>&                       indices 
)
{
   float area;
   uint startIndex = uint(indices.size());

#define ADD_TRI( a, b, c ) \
   if( ccw( vptrs[a]->_pos(x,y), vptrs[b]->_pos(x,y), vptrs[c]->_pos(x,y), area ) )               \
   {                                                                                              \
      indices.pushBack( vptrs[a]->_id );                                                          \
      if( !flip )                                                                                 \
      {                                                                                           \
         indices.pushBack( vptrs[b]->_id );                                                       \
         indices.pushBack( vptrs[c]->_id );                                                       \
      }                                                                                           \
      else                                                                                        \
      {                                                                                           \
         indices.pushBack( vptrs[c]->_id );                                                       \
         indices.pushBack( vptrs[b]->_id );                                                       \
      }                                                                                           \
   }                                                                                              \
   else if( area > CGConstf::epsilon() )                                                          \
   {                                                                                              \
      indices.resize( startIndex );                                                               \
      triangulate( vptrs, indices, flip, x, y );                                                  \
      return;                                                                                     \
   }

#define COMPUTE_EDGE_COST( a, b ) \
   (vptrs[a]->_pos - vptrs[b]->_pos).sqrLength()
   
   uint cornerStart[4];
   cornerStart[0] = 0;
   cornerStart[1] = /*cnerStart[0]*/ nbVertices[0];
   cornerStart[2] = cornerStart[1] + nbVertices[1];
   cornerStart[3] = cornerStart[2] + nbVertices[2];
   int n = cornerStart[3] + nbVertices[3];

   // Projection in 2D.
   int x, y;
   Vec3f ab = vptrs[cornerStart[1]]->_pos - vptrs[0]->_pos;
   Vec3f ac = vptrs[cornerStart[2]]->_pos - vptrs[0]->_pos;
   Vec3f ad = vptrs[cornerStart[3]]->_pos - vptrs[0]->_pos;
   Vec3f n0 = cross(ab,ac);
   Vec3f n1 = cross(ac,ad);
   if( dot(n0,n1) > 0.0f )
   {
      (n0+n1).secAxes( x, y );
   }
   else
   {
      cross(ab,ad).secAxes( x, y );
   }
      
   uint sc = 0;                // Start corner, range [0, 4).
   uint lc = (sc+3) % 4;       // Next corner that we will reach when travelling left, range [0, 4).
   uint rc = (sc+1) % 4;       // Next corner that we will reach when travelling right, range [0, 4).

   uint ci = cornerStart[sc];  // The start vertex index.
   uint li = (ci-1+n) % n;     // Current vertex index towards the left.
   uint ri = (ci+1  ) % n;     // Current vertex index towards the right.

   uint lj = (li-1+n) % n;     // Next vertex candidate towards the left.
   uint rj = (ri+1  ) % n;     // Next vertex candidate towards the right.

   // Add first corner triangle.
   ADD_TRI( ci, ri, li );

   if( lj != rj )
   {
      float lv; // The cost if we advance towards the left.
      float rv; // The cost if we advance towards the right.

      // Handle cases where front starts at corners.
      if( li == cornerStart[lc] ) lc = (lc+3) % 4;
      if( ri == cornerStart[rc] ) rc = (rc+1) % 4;

      lv = (lj == cornerStart[rc]) ? CGConstf::infinity() : COMPUTE_EDGE_COST( ri, lj );
      rv = (rj == cornerStart[lc]) ? CGConstf::infinity() : COMPUTE_EDGE_COST( li, rj );

      do {
         // Decide which of the 2 candidates to move forward.
         if( lv < rv )
         {
            // Progress on the left.
            ADD_TRI( li, ri, lj );
            li = lj;
            lj = (li-1+n) % n;

            // Advance corner.
            if( li == cornerStart[lc] ) lc = (lc+3) % 4;

            // Check if our current position lies on same edge as other's corner.
            lv = (lj == cornerStart[rc]) ? CGConstf::infinity() : COMPUTE_EDGE_COST( ri, lj );
         }
         else
         {
            // Progress on the right.
            ADD_TRI( ri, rj, li );
            ri = rj;
            rj = (ri+1) % n;

            // Advance corner.
            if( ri == cornerStart[rc] ) rc = (rc+1) % 4;

            // Check if our current position lies on same edge as other's corner.
            rv = (rj == cornerStart[lc]) ? CGConstf::infinity(): COMPUTE_EDGE_COST( li, rj );
         }
      } while( lj != rj );
   }

   // Add last triangle.
   ADD_TRI( li, ri, lj );  // lj == rj 

#undef COMPUTE_EDGE_COST
#undef ADD_TRI
}

//------------------------------------------------------------------------------
//! 
void
subpatchesOverlap( void* data, void* objA, void* objB )
{
   MetaSurface::Subpatch* spA = (MetaSurface::Subpatch*)objA;
   MetaSurface::Subpatch* spB = (MetaSurface::Subpatch*)objB;
   MetaSurface* surface       = (MetaSurface*)data;
   surface->trim( *spA, *spB );
}

//------------------------------------------------------------------------------
//! 
bool leq( const Vec2f& a, const Vec2f& b )
{
   return (a.x < b.x) || ( (a.x == b.x) && (a.y <= b.y) );
}

/*==============================================================================
   STRUCT TrimEdge
==============================================================================*/

struct TrimEdge
{
   ushort _v0;
   ushort _v1;
   int    _group;
};

/*==============================================================================
   STRUCT CompVert
==============================================================================*/

struct CompVert
{
   CompVert( const Vec2f& v, int x, int y ): _v(v), _x(x), _y(y) {}

   bool operator()( const MetaSurface::Vertex* a, const MetaSurface::Vertex* b ) const
   {
      float dista = (_v-a->_pos(_x,_y)).sqrLength();
      float distb = (_v-b->_pos(_x,_y)).sqrLength();
      return dista < distb;
   }

   Vec2f _v;
   int   _x;
   int   _y;
};

UNNAMESPACE_END


NAMESPACE_BEGIN

/*==============================================================================
   CLASS TrimmingData
==============================================================================*/

class TrimmingData
{
public:

   /*----- typedefs -----*/

   typedef MetaSurface::Loop     Loop;
   typedef MetaSurface::LoopLink LoopLink;
   typedef MetaSurface::Trimming Trimming;
   typedef MetaSurface::Edge     Edge;
   typedef MetaSurface::Vertex   Vertex;

   /*----- methods -----*/

   TrimmingData() {}
   ~TrimmingData() {}

   //---------------------------------------------------------------------------
   //! 
   Trimming* trim( MetaSurface::Subpatch* sp )
   {
      // Find trimming subpatch info.
      Trimming* trim = &(_trimming)[sp];

      // Reset if new.
      if( !MetaSurface::isTrimmed(*sp) )
      {
         trim->_vertices = 0;
         trim->_edges    = 0;
         trim->_loops    = 0;

         // Subpatch has trimming info..
         sp->_flags         = 1;
         sp->_patch->_flags = 1;

         // Compute triangulation pattern.
         // 0: triangles (0,1,2),(0,2,3).
         // 1: triangles (0,1,3),(1,2,3).
         Vertex** c = sp->_corners;
         Vec3f ab  = c[1]->_pos - c[0]->_pos;
         Vec3f ac  = c[2]->_pos - c[0]->_pos;
         Vec3f ad  = c[3]->_pos - c[0]->_pos;
         Vec3f n0  = cross(ab,ac);
         Vec3f n1  = cross(ac,ad);

         if( dot(n0,n1) > 0.0f )
         {
            trim->_flags  = 0;
            trim->_plane0 = Planef( n0, c[0]->_pos ).normalize();
            trim->_plane1 = Planef( n1, c[0]->_pos ).normalize();
         }
         else
         {
            trim->_flags  = 1;
            trim->_plane0 = Planef( cross(ab,ad), c[0]->_pos ).normalize();
            trim->_plane1 = Planef( c[1]->_pos, c[2]->_pos, c[3]->_pos ).normalize();
         }

         //if( !CGM::equal( trim->_plane0.d(), trim->_plane1.d(), 0.2f ) ) { DBG_HALT(); }

         // Compute projection axes.
         uint maxc0 = trim->_plane0.direction().maxComponent();
         uint maxc1 = trim->_plane1.direction().maxComponent();

         if( trim->_plane0.direction()(maxc0) > trim->_plane1.direction()(maxc1) )
         {
            trim->_z = maxc0;
            trim->_x = (maxc0+1)%3;
            trim->_y = (maxc0+2)%3;
            if( trim->_plane0.direction()(maxc0) < 0.0f ) CGM::swap( trim->_x, trim->_y );
         }
         else
         {
            trim->_z = maxc1;
            trim->_x = (maxc1+1)%3;
            trim->_y = (maxc1+2)%3;
            if( trim->_plane1.direction()(maxc1) < 0.0f ) CGM::swap( trim->_x, trim->_y );
         }
      }
      return trim;
   }

   //------------------------------------------------------------------------------
   //! 
   bool validateEdge( Trimming* trim, Vertex* v0, Vertex* v1 )
   {
      Edge* e = trim->_edges;
      return validateEdge( trim, v0, v1, e );
   }

   //------------------------------------------------------------------------------
   //! 
   bool validateEdge( Trimming* trim, Vertex* v0, Vertex* v1, Edge*& pe )
   {
      // Is it an edge?
      if( v0 == v1 ) return false;
      // FIXME: find a better test.
      // Is the edge a valid one?
      if( v0->_uv.x == v1->_uv.x )
      {
         if( (v0->_next[0] && v0->_next[2] && v1->_next[0] && v1->_next[2]) ) return false;
      }
      if( v0->_uv.y == v1->_uv.y )
      {
         if( (v0->_next[1] && v0->_next[3] && v1->_next[1] && v1->_next[3]) ) return false;
      }

      // Test if edge already there and return the insertion position.
      int x   = trim->_x;
      int y   = trim->_y;
      Edge* e = pe ? pe->_next : trim->_edges;
      for( ; e && leq( e->_v0->_pos(x,y), v0->_pos(x,y) ); pe = e, e = e->_next )
      {
         if( e->_v0 == v0 && e->_v1 == v1 ) return false;
      }
      return true;
   }

   //---------------------------------------------------------------------------
   //! 
   void addEdge( Trimming* trim, Vertex* v0, Vertex* v1, Edge* se = 0 )
   {
      // Sort edge.
      int x = trim->_x;
      int y = trim->_y;
      if( leq( v1->_pos(x,y), v0->_pos(x,y) ) ) CGM::swap( v0, v1 );
      // Add edge only if valid.
      if( validateEdge( trim, v0, v1, se ) )
      {
         // Add edge.
         Edge* edge   = _edges.alloc();
         edge->_v0    = v0;
         edge->_v1    = v1;
         // Insert edge at the beginning.
         if( !se )
         {
            edge->_next  = trim->_edges;
            trim->_edges = edge;
         }
         else
         {
            edge->_next = se->_next;
            se->_next   = edge;
         }
      }
   }

   //------------------------------------------------------------------------------
   //! 
   void fixVertex( Trimming* trim, Vertex* nv, const Vec3f& pos )
   {
      int x    = trim->_x;
      int y    = trim->_y;
      int lx   = int(nv->_pos(x));
      nv->_pos = pos;
      Edge* pe = 0;
      for( Edge* e = trim->_edges; e && ( e->_v0->_pos(x) <= lx ); )
      {
         if( e->_v0 == nv )
         {
            // 1.Re-sort vertices.
            if( leq( e->_v1->_pos(x,y), e->_v0->_pos(x,y) ) ) CGM::swap( e->_v0, e->_v1 );
            // 2.Re-sort edge.
            // 2.1 Find insertion position.
            Edge* ne = e;
            while( ne->_next && leq( ne->_next->_v0->_pos(x,y), pos(x,y) ) ) ne = ne->_next;
            // 2.2 Insert is different then current position.
            if( ne != e )
            {
               Edge* next = e->_next;
               if( pe ) pe->_next = e->_next;
               else trim->_edges = e->_next;
               e->_next  = ne->_next;
               ne->_next = e;
               e = next;
               continue;
            }
         }
         pe = e;
         e  = e->_next;
      }
   }

   //------------------------------------------------------------------------------
   //! 
   void removeIntersections( MetaSurface* surface, MetaSurface::Subpatch* sp, Trimming* trim )
   {
      int x = trim->_x;
      int y = trim->_y;

      // Solve edges intersections and overlapping.
      for( Edge* e0 = trim->_edges; e0; e0 = e0->_next )
      {
         // Skip invalid edge.
         if( e0->_v0 == e0->_v1 ) continue;
         for( Edge* e1 = e0->_next; e1; e1 = e1->_next )
         {
            // Skip invalid edge.
            if( e1->_v0 == e1->_v1 ) continue;
            // No more intersection since edges are sorted by X.
            if( e0->_v1->_pos(x) + _errormax < e1->_v0->_pos(x) ) break;

            Vertex* v0p = e0->_v0;
            Vertex* v1p = e0->_v1;
            Vertex* v2p = e1->_v0;
            Vertex* v3p = e1->_v1;

            // Compute intersection.
            Vec2f v0 = v0p->_pos( x, y );
            Vec2f v1 = v1p->_pos( x, y );
            Vec2f v2 = v2p->_pos( x, y );
            Vec2f v3 = v3p->_pos( x, y );

            Vec2f d0 = v1-v0;
            Vec2f d1 = v3-v2;
            Vec2f d2 = v0-v2;

            float den  = (d1.y*d0.x)-(d1.x*d0.y);
            float num0 = (d1.x*d2.y)-(d1.y*d2.x);
            float num1 = (d0.x*d2.y)-(d0.y*d2.x);

            // Edges are parallel?
            if( CGM::equal( den*den/(sqrLength(d0)*sqrLength(d1)), 0.0f ) )
            {
               // Edges are colinear?
               if( CGM::equal( num0, 0.0f, 1e-5f ) && CGM::equal( num1, 0.0f, 1e-5f ) )
               {
                  // Represent e1 in e0's parametric space.
                  // v0-v1 is in [0, t1] pseudo-parametric range.
                  float t1 = dot(  d0,   d0 );
                  float t2 = dot( v2-v0, d0 );
                  float t3 = dot( v3-v0, d0 );
                  // Order interval [v2,v3]
                  if( t3 < t2 )
                  {
                     CGM::swap( t2, t3 );
                     CGM::swap( v2p, v3p );
                  }
                  // Non-intersecting case.
                  if( t1 <= t2 || t3 <= 0.0f ) continue;
                  // We have an intersection!!!
                  // Compute overlap.
                  float st = CGM::max( 0.0f, t2 );
                  float et = CGM::min( t1, t3 );
                  // Compute e0 segments.
                  if( st == 0.0f )
                  {
                     // Overlap [t0,t3].
                     if( et < t1 )
                     {
                        e0->_v1 = e0->_v0; // delete edge.
                        addEdge( trim, v0p, v3p );
                        addEdge( trim, v3p, v1p, e0 );
                     }
                  }
                  else
                  {
                     // Overlap [t2,t3].
                     if( et < t1 )
                     {
                        e0->_v1 = e0->_v0; // delete edge.
                        addEdge( trim, v0p, v2p );
                        addEdge( trim, v3p, v1p, e0 );
                     }
                     // Overlap [t2,t1].
                     else
                     {
                        e0->_v1 = e0->_v0; // delete edge.
                        addEdge( trim, v0p, v2p );
                        addEdge( trim, v2p, v1p, e0 );
                     }
                  }

                  // Compute e1 segments.
                  if( st == t2 )
                  {
                     // Overlap [t2,t1].
                     if( et < t3 )
                     {
                        e1->_v1 = e1->_v0; // delete edge.
                        addEdge( trim, v2p, v1p );
                        addEdge( trim, v1p, v3p );
                     }
                  }
                  else
                  {
                     // Overlap [t0,t1].
                     if( et < t3 )
                     {
                        e1->_v1 = e1->_v0; // delete edge.
                        addEdge( trim, v2p, v0p );
                        addEdge( trim, v1p, v3p );
                     }
                     // Overlap [t0,t3]
                     else
                     {
                        e1->_v1 = e1->_v0; // delete edge.
                        addEdge( trim, v2p, v0p );
                        addEdge( trim, v0p, v3p );
                     }
                  }
               }
            }
            else
            {
               // 1. compute intersection.
               float t0  = num0/den;
               float t1  = num1/den;
               Vec3f pos = v2p->_pos*(1.0f-t1) + v3p->_pos*t1; // Use t1 and not t0 to make sure pos.x >= v2.x.
               Vec2f p   = pos(x,y);

               // 2. test v0
               if( equal( v0, p, _errormax ) )
               {
                  // 2.1 test v2
                  if( equal( v2, p, _errormax ) ) continue;
                  // 2.2 test v3
                  if( equal( v3, p, _errormax ) ) continue;
                  // 2.3 test t1
                  if( t1 < 0.0f || t1 > 1.0f ) continue;
                  // Intersection v0 and e1.
                  // nv can be < e1->_v0! So, invalidate edge and add a new one.
                  e1->_v1 = e1->_v0;
                  addEdge( trim, v2p, v0p );
                  addEdge( trim, v0p, v3p, e0 );
               }
               // 3. test v1
               else if( equal( v1, p, _errormax ) )
               {
                  // 3.1 test v2
                  if( equal( v2, p, _errormax ) ) continue;
                  // 3.2 test v3
                  if( equal( v3, p, _errormax ) ) continue;
                  // 3.3 test t1
                  if( t1 < 0.0f || t1 > 1.0f ) continue;
                  // Intersection v1 and e1.
                  // nv can be < e1->_v0! So, invalidate edge and add a new one.
                  e1->_v1 = e1->_v0;
                  addEdge( trim, v2p, v1p );
                  addEdge( trim, v1p, v3p, e0 );
               }
               // 4. test t0.
               else if( 0.0f < t0 && t0 < 1.0f )
               {
                  // 5. test v2
                  if( equal( v2, p, _errormax ) )
                  {
                     // Intersection e0 and v2.
                     e0->_v1 = v0p;
                     addEdge( trim, v0p, v2p );
                     addEdge( trim, v2p, v1p, e0 );
                  }
                  // 6. test v3
                  else if( equal( v3, p, _errormax ) )
                  {
                     // Intersection e0 and v3.
                     e0->_v1 = v0p;
                     addEdge( trim, v0p, v3p );
                     addEdge( trim, v3p, v1p, e0 );
                  }
                  // 7. test t1
                  else if ( 0.0f < t1 && t1 < 1.0f )
                  {
                     // Intersection e0 and e2.
                     MetaSurface::Vertex* nv = surface->insertVertex( *sp, trim, pos );
                     // nv.x should be >= v0.x else we need to fix it.
                     if( nv->_pos(x) < v2.x ) fixVertex( trim, nv, pos );
                     e0->_v1 = v0p;
                     addEdge( trim, v0p, nv );
                     addEdge( trim, nv, v1p, e0 );
                     e1->_v1 = v2p;
                     addEdge( trim, v2p, nv, e0 );
                     addEdge( trim, nv, v3p, e0 );
                  }
               }
            }
         }
      }
      // Remove invalid edge.
      Edge* pe = 0;
      for( Edge* e = trim->_edges; e; )
      {
         // Invalid edge?
         if( e->_v0 == e->_v1 )
         {
            Edge* ce = e;
            e = e->_next;
            _edges.free(ce);
            if( pe ) pe->_next = e;
            else trim->_edges = e;
         }
         else
         {
            pe = e;
            e  = e->_next;
         }
      }
   }

   //---------------------------------------------------------------------------
   //! 
   int findNextEdge( int e, int x, int y )
   {
      int v0   = _trimEdges[e]._v0;
      int v1   = _trimEdges[e]._v1;
      int re   = 0xffff;
      float ra = CGConstf::infinity();
      e        = _starsFirst[v1];

      Vec2f p0 = _vertices[v0]->_pos(x,y);
      Vec2f p1 = _vertices[v1]->_pos(x,y);
      Vec2f d0 = (p0-p1).getNormalized();

      while( e != 0xffff )
      {
         float angle = 2.0f;
         int v2 = _trimEdges[e]._v1;
         if( v2 != v0 )
         {
            Vec2f p2  = _vertices[v2]->_pos(x,y);
            Vec2f d1  = (p2-p1).getNormalized();
            angle     = dot(d0,d1);
            if( ccw( p0, p1, p2 ) ) angle = -2.0f-angle;
         }
         if( angle < ra )
         {
            ra = angle;
            re = e;
         }
         e = _starsNext[e];
      }
      if( re != 0xffff && _trimEdges[re]._group < 0 ) re = 0xffff;
      return re;
   }

   //---------------------------------------------------------------------------
   //! 
   void tagLoop( int e, int group, int x, int y )
   {
      Vertex* minVertex = _vertices[_trimEdges[e]._v0];
      Vec2f v           = minVertex->_pos(x,y);
      while( 1 )
      {
         // Tag edge.
         _trimEdges[e]._group = group;
         // Keep min loop's vertex.
         Vertex* vertex  = _vertices[_trimEdges[e]._v1];
         Vec2f cv        = vertex->_pos(x,y);
         if( (cv.x < v.x) || (cv.x == v.x && cv.y < v.y) )
         {
            v         = cv;
            minVertex = vertex;
         }
         // Find next edges.
         int ne = _starsFirst[_trimEdges[e]._v1];
         while( ne != 0xffff )
         {
            if( _trimEdges[ne]._group == 0 ) _stack.pushBack( ne );
            ne = _starsNext[ne];
         }
         // Finish tagging?
         if( _stack.empty() ) 
         {
            _minVertices.pushBack( minVertex );
            return;
         }
         // Unstack next edge.
         e = _stack.back();
         _stack.popBack();
      }
   }

   //---------------------------------------------------------------------------
   //!
   bool testEdges( Vertex* v0p, const Vec2f& v0, const Vec2f& v1, int x, int y )
   {
      for( uint e = 0; e < _trimEdges.size(); ++e )
      {
         Vertex* v2p = _vertices[_trimEdges[e]._v0];
         Vertex* v3p = _vertices[_trimEdges[e]._v1];
         if( v2p->_pos(x) > v3p->_pos(x) ) CGM::swap( v2p, v3p );

         if( v2p->_pos(x) < v1.x && v3p->_pos(x) > v0.x )
         {
            if( v2p == v0p || v3p == v0p ) continue;
            Vec2f v2 = v2p->_pos( x, y );
            Vec2f v3 = v3p->_pos( x, y );

            Vec2f d0 = v1-v0;
            Vec2f d1 = v3-v2;
            Vec2f d2 = v0-v2;

            float den  = (d1.y*d0.x)-(d1.x*d0.y);
            float num0 = (d1.x*d2.y)-(d1.y*d2.x);
            float num1 = (d0.x*d2.y)-(d0.y*d2.x);

            // Edges are parallel?
            if( CGM::equal( den, 0.0f ) )
            {
               if( CGM::equal( num0, 0.0f ) && CGM::equal( num1, 0.0f ) ) return false;
            }
            else
            {
               float t0 = num0/den;
               float t1 = num1/den;
               if( t0 >= 0.0f && t0 <= 1.0f && t1 >= 0.0f && t1 <= 1.0f ) return false;
            }
         }
      }
      return true;
   }

   //---------------------------------------------------------------------------
   //! 
   void fixLoop( int x, int y )
   {
      _minVertices.clear();
      int group = 1;
      // Find loops.
      for( uint i = 0; i < _trimEdges.size(); ++i )
      {
         if( _trimEdges[i]._group > 0 ) continue;
         tagLoop( i, group, x, y );
         ++group;
      }
      // Connect loop.
      if( group > 2 )
      {
         for( uint g = 1; g < _minVertices.size(); ++g )
         {
            _conVertices.clear();
            Vec2f v1 = _minVertices[g]->_pos(x,y);
            // Create a list of potential vertices to connect to( < x ).
            for( uint v = 0; v < _vertices.size(); ++v )
            {
               if( _vertices[v]->_pos(x) < v1.x ) _conVertices.pushBack( _vertices[v] );
            }
            // Sort list by distance.
            std::sort( _conVertices.begin(), _conVertices.end(), CompVert( v1, x, y ) );

            // Find witch vertex to connect to and create edges.
            for( uint v = 0; v < _conVertices.size(); ++v )
            {
               Vec2f v0 = _conVertices[v]->_pos(x,y);
               // Test against edges.
               if( testEdges( _conVertices[v], v0, v1, x, y ) )
               {
                  // Add edges.
                  TrimEdge edge;
                  edge._group = g+1;
                  edge._v0 = _minVertices[g]->_id;
                  edge._v1 = _conVertices[v]->_id;
                  _trimEdges.pushBack( edge );
                  _starsNext.pushBack( _starsFirst[edge._v0] );
                  _starsFirst[edge._v0] = uint16_t(_trimEdges.size())-1;

                  edge._v0 = _conVertices[v]->_id;
                  edge._v1 = _minVertices[g]->_id;
                  _trimEdges.pushBack( edge );
                  _starsNext.pushBack( _starsFirst[edge._v0] );
                  _starsFirst[edge._v0] = uint16_t(_trimEdges.size())-1;
                  break;
               }
            }
         }
      }
   }

   //---------------------------------------------------------------------------
   //! 
   void computeLoops( MetaSurface* surface, MetaSurface::Subpatch* sp, Trimming* trim )
   {
#if DBG_DETAILS_LOOP
      //bool dbg = (sp->_patch->_detailsID == 15) && (sp->_corners[0]->_uv == Vec2f(0.0f,0.3125f));
      bool dbg = (sp->_patch->_detailsID == DBG_DETAILS_ID) && (sp->_corners[0]->_uv == DBG_DETAILS_UV);
      if( dbg )
      {
         StdErr << "BEFORE:\n";
         surface->dump( *sp );
         surface->dump( *sp, *trim );
      }
#endif //DBG_DETAILS_LOOP

      // 1. Solve edges intersections.
      removeIntersections( surface, sp, trim );

      // 2. Creating vertices info and edges for subpatch main loop.
      _vertices.clear();
      _starsFirst.clear();
      _trimEdges.clear();
      _starsNext.clear();

      TrimEdge edge;
      edge._group = 0;

      MetaSurface::Vertex* v = sp->_corners[0];
      uint i = 0;
      for( uint c = 0; c < 4; ++c )
      {
         for( ; v != sp->_corners[(c+1)%4]; v = v->_next[(c+1)%4], ++i )
         {
            // Vertex info.
            v->_id = i;
            _vertices.pushBack(v);
            _starsFirst.pushBack(i);
            // Edge info.
            edge._v0 = i;
            edge._v1 = i+1;
            _trimEdges.pushBack( edge );
            _starsNext.pushBack(0xffff);
         }
      }
      _trimEdges.back()._v1 = 0;

      // 3. Create vertices info and edges for interior edges.
      // Interior vertices.
      for( v = trim->_vertices; v; v = v->_next[0], ++i )
      {
         v->_id = i;
         _vertices.pushBack(v);
         _starsFirst.pushBack(0xffff);
      }
      // Interior edges (count for 2 half edges).
      i = uint(_trimEdges.size());
      for( Edge* e = trim->_edges; e != 0; e = e->_next )
      {
         edge._v0 = e->_v0->_id;
         edge._v1 = e->_v1->_id;
         _trimEdges.pushBack( edge );
         _starsNext.pushBack( _starsFirst[edge._v0] );
         _starsFirst[edge._v0] = i++;

         edge._v0 = e->_v1->_id;
         edge._v1 = e->_v0->_id;
         _trimEdges.pushBack( edge );
         _starsNext.pushBack( _starsFirst[edge._v0] );
         _starsFirst[edge._v0] = i++;
      }

      // 4. Find and connect disjoint loops.
      fixLoop( trim->_x, trim->_y );

      // 5. Create loops.
      for( i = 0; i < _trimEdges.size(); ++i )
      {
         // Edge already in a loop?
         if( _trimEdges[i]._group < 0 ) continue;
         
         // New loop.
         Loop* loop   = _loops.alloc();
         loop->_flags = 0;
         loop->_next  = trim->_loops;
         trim->_loops = loop;

         // First vertex.
         LoopLink* link = _links.alloc();
         link->_vertex  = _vertices[_trimEdges[i]._v0];
         link->_next    = 0;
         loop->_link    = link;
         // Set edge as done.
         _trimEdges[i]._group = -1;

         int j = i;
         while( true )
         {
            // Find next edge
            j = findNextEdge(j, trim->_x, trim->_y );
            // No more edge?
            if( j == 0xffff ) break;

            // Add vertex.
            LoopLink* clink = _links.alloc();
            clink->_vertex  = _vertices[_trimEdges[j]._v0];
            clink->_next    = 0;
            link->_next     = clink;
            link            = clink;
            // Set edge as done.
            _trimEdges[j]._group = -1;
         }
      }

      // Clear vertices.
      for( i = 0; i < _vertices.size(); ++i )
      {
         _vertices[i]->_id = INVALID;
      }

#if DBG_DETAILS_LOOP
      if( dbg )
      {
         StdErr << "AFTER:\n";
         surface->dump( *sp, *trim );
      }
#endif
   }

protected:

   /*----- data members -----*/

   HashTable<void*,Trimming> _trimming;
   MemoryPool<Edge>          _edges;
   MemoryPool<Loop>          _loops;
   MemoryPool<LoopLink>      _links;

   // Members for computing loops.
   Vector<ushort>            _stack;
   Vector<Vertex*>           _conVertices;
   Vector<Vertex*>           _minVertices;
   Vector<Vertex*>           _vertices;
   Vector<TrimEdge>          _trimEdges;
   Vector<ushort>            _starsFirst;
   Vector<ushort>            _starsNext;
};


/*==============================================================================
   CLASS MetaSurface::CacheData
==============================================================================*/

class MetaSurface::CacheData:
   public RCObject
{
public:

   /*----- structures -----*/

   struct Context{
      int   _id;
      int   _fid;
      Vec2f _uv;
      Vec3f _pos;
      Vec3f _nor;
      Vec3f _fnor;
   };

   /*----- static methods -----*/

   static int context_get( VMState* vm )
   {
      Context* context = *(Context**)VM::toPtr( vm, 1 );
      switch( _attributes[ VM::toCString( vm, 2 ) ] )
      {
         case ATTRIB_ID:     VM::push( vm, context->_id+1 ); return 1;
         case ATTRIB_FACE:   VM::push( vm, context->_fid );  return 1;
         case ATTRIB_FACE_N: VM::push( vm, context->_fnor ); return 1;
         case ATTRIB_N:      VM::push( vm, context->_nor );  return 1;
         case ATTRIB_POS:    VM::push( vm, context->_pos );  return 1;
         case ATTRIB_UV:     VM::push( vm, context->_uv );   return 1;
      }
      return 0;
   }

   static inline void scanline( 
      const Vec3f* src, 
      int    inc, 
      int    u0, 
      int    u1, 
      int    flag, 
      float  error, 
      int&   res
   )
   {
      Vec3f p0   = *src;
      Vec3f p1   = *(src+inc*(u1-u0));
      Vec3f pinc = (p1-p0)/float(u1-u0);
      for( int i = u0+1; i < u1; ++i )
      {
         p0  += pinc;
         src += inc;
         if( (p0-*src).sqrLength() > error ) { res |= flag; return; }
      }
   }


   /*----- methods -----*/

   CacheData(): 
      _grid(0), 
      _vm(0), _mapping(0), _displacement(0), _precision(0.1f),
      _lastPatch(0), _lastPp(0), _patches(32), _trimData(0) 
   {}

   ParametricPatch* paramPatch( MetaSurface::Patch* p )
   {
      if( p == _lastPatch ) return _lastPp;
      if( !_patches.get( p, _lastPp ) ) _lastPp->init( *p );
      _lastPatch = p;
      return _lastPp;
   }

   TrimmingData* trimmingData()
   {
      if( !_trimData ) _trimData = new TrimmingData();
      return _trimData;
   }

   // Mapping and displacement.
   void openVM()
   {
      _vm             = VM::open( VM_CAT_MATH | VM_CAT_MAT, true );
      Context** inObj = (Context**)VM::newObject( _vm, sizeof(Context*) );
      *inObj          =  &_context;
      _displacement   = 0;
      _mapping        = 0;
      _dStackPos      = 2;

      VM::newMetaTable( _vm, "IN" );
      VM::set( _vm, -1, "__index", context_get );
      VM::setMetaTable( _vm, -2 );
   }

   void closeVM()
   {
      if( _vm ) VM::close( _vm );
      _vm = 0;
   }

   void setMapping( const MetaFunction* func )
   {
      if( _mapping == func ) return;

      VM::pop( _vm, VM::getTop( _vm )-1 );
      _mapping = func;

      if( _mapping )
      {
         VM::loadByteCode( _vm, func->code() );
         for( uint i = 0; i < func->parameters().arraySize(); ++i )
         {
            VM::push( _vm, func->parameters()[i] );
         }
      }
      _dStackPos    = VM::getTop( _vm ) + 1;
      _displacement = 0;
   }

   void setDisplacement( const MetaFunction* func )
   {
      if( _displacement == func ) return;

      VM::pop( _vm, VM::getTop( _vm )+1-_dStackPos );
      _displacement = func;

      if( _displacement )
      {
         VM::loadByteCode( _vm, func->code() );
         for( uint i = 0; i < func->parameters().arraySize(); ++i )
         {
            VM::push( _vm, func->parameters()[i] );
         }
      }
   }

   Vec2f computeMapping( const Vec3f& pos, const Vec2f& uv, const Vec3f& n )
   {
      // Set context.
      _context._pos = pos;
      _context._nor = n;
      _context._uv  = uv;

      // Set stack.
      int numParams = _dStackPos-3;
      VM::pushValue( _vm, 2 ); // Function.
      VM::pushValue( _vm, 1 ); // IN.
      for( int i = 0; i < numParams; ++i )
      {
         VM::pushValue( _vm, 3+i );
      }
      VM::ecall( _vm, 1+numParams, 1 );
      Vec2f result = VM::toVec2f( _vm, -1 );
      VM::pop( _vm );
      return result;
   }

   void computeDisplacement( Patch& p )
   {
      // Compute image size.
      Vec3f c0 = *p._controlPts[0];
      Vec3f c1 = *p._controlPts[1];
      Vec3f c2 = *p._controlPts[2];
      Vec3f c3 = *p._controlPts[3];

      Vec3f du0  = c1-c0;
      Vec3f du1  = c2-c3;
      Vec3f dv0  = c3-c0;
      Vec3f dv1  = c2-c1;

      float area = (length( cross( du0, dv1 ) ) + length( cross( du1, dv0 ) )) * 0.5f;
      float du   = CGM::max( length(du0), length(du1) );
      float dv   = du > 0.0f ? area / du : 0.0f;

      _dimImage.x = int(du*0.75f / _precision);
      _dimImage.y = int(dv*0.75f / _precision);

      _dimImage.x = CGM::nextPow2( _dimImage.x ) + 1;
      _dimImage.y = CGM::nextPow2( _dimImage.y ) + 1;

      //_dimImage.x = _dimImage.y = 20;

      _dimImage.x = CGM::clamp( _dimImage.x, 2, 1024 );
      _dimImage.y = CGM::clamp( _dimImage.y, 2, 1024 );

      // Allocate/resize.
      _geomImage.resize( _dimImage.x * _dimImage.y );

      // Compute image.
      ParametricPatch* pp = paramPatch( &p );
      int numParams       = VM::getTop( _vm ) - _dStackPos;
      int c               = 0;

      for( int y = 0; y < _dimImage.y; ++y  )
      {
         float v = float(y)/float(_dimImage.y-1);
         for( int x = 0; x < _dimImage.x; ++x, ++c )
         {
            float u = float(x)/float(_dimImage.x-1);

            // Set input parameters.
            _context._uv = Vec2f( u, v );
            pp->parameters( _context._uv, _context._pos, _context._nor );

            // Execute program.
            VM::pushValue( _vm, _dStackPos ); // Function.
            VM::pushValue( _vm, 1 ); // IN.
            for( int i = 1; i <= numParams; ++i )
            {
               VM::pushValue( _vm, _dStackPos+i );
            }
            VM::ecall( _vm, 1+numParams, 1 );

            // Read back results.
            _geomImage[c] = VM::toVec3f( _vm, -1 );
            VM::pop( _vm );
         }
      }

      // Fix boundaries.
   }

   void displacementPrecision( float precision ) { _precision = precision; }

   Vec3f displace( const Vec2f& uv ) const
   {
      float s = uv.x * float(_dimImage.x-1);
      float t = uv.y * float(_dimImage.y-1);

      int   xi, yi;
      float xf, yf;
      CGM::splitIntFrac( s, xi, xf );
      CGM::splitIntFrac( t, yi, yf );

      int incx = xi < _dimImage.x-1 ? 1 : 0;

      const Vec3f* p = &_geomImage[yi*_dimImage.x+xi];
      Vec3f bl = p[0];
      Vec3f br = p[incx];

      if( yi < _dimImage.y-1 ) p += _dimImage.x;

      Vec3f tl = p[0];
      Vec3f tr = p[incx];

      return CGM::bilinear(
         bl, br-bl,
         tl, tr-tl,
         xf, yf
      );
   }

   int flatness( 
      const Vec2f& uv0, 
      const Vec2f& uv1, 
      int          edge,
      float        error
   ) const
   {
      float error2 = error*error;

      // Find uv coordinates in texture space.
      Vec2f st0;
      Vec2f st1;

      st0.x = uv0.x * float(_dimImage.x-1);
      st0.y = uv0.y * float(_dimImage.y-1);
      st1.x = uv1.x * float(_dimImage.x-1);
      st1.y = uv1.y * float(_dimImage.y-1);

      const float r = 0.4990234375;
      int u0 = int( st0.x+r );
      int v0 = int( st0.y+r );
      int u1 = int( st1.x+r );
      int v1 = int( st1.y+r );

      /*
      int u0 = CGM::ceili( st0.x );
      int v0 = CGM::ceili( st0.y );
      int u1 = CGM::floori( st1.x );
      int v1 = CGM::floori( st1.y );
      */

      int subu   = 0;
      int subv   = 0;
      const Vec3f* src = 0;
      // Possible subdivision in uv.
      // Test u.
      if( u1-u0 > 1 )
      {
         int inc = 1;

         // Edge 0.
         if( edge&1 )
         {
            src = &_geomImage[u0+v0*_dimImage.x];
            scanline( src, inc, u0, u1, (1|4), error2, subu );
         }

         // Edge 2.
         if( edge&4 )
         {
            src = &_geomImage[u0+v1*_dimImage.x];
            scanline( src, inc, u0, u1, (1|16), error2, subu );
         }

         // Interior.
         for( int j = v0+1; (j < v1)&&(!subu); ++j )
         {
            src = &_geomImage[u0+j*_dimImage.x];
            scanline( src, inc, u0, u1, 1, error2, subu );
         }
      }

      // Test v.
      if( v1-v0 > 1 )
      {
         int inc = _dimImage.x;

         // Edge 3.
         if( edge&8 )
         {
            src = &_geomImage[u0+v0*_dimImage.x];
            scanline( src, inc, v0, v1, (2|32), error2, subv );
         }

         // Edge 1.
         if( edge&2 )
         {
            src = &_geomImage[u1+v0*_dimImage.x];
            scanline( src, inc, v0, v1, (2|8), error2, subv );
         }

         // Interior.
         for( int i = u0+1; (i < u1)&&(!subv); ++i )
         {
            src = &_geomImage[i+v0*_dimImage.x];
            scanline( src, inc, v0, v1, 2, error2, subv );
         }
      }
      return (subu|subv);
   }

   /*----- data members -----*/

   Vector<MetaSurface::Vertex*> _vptrs;
   Grid<MetaSurface::Vertex*>*  _grid;
   Context                      _context;
   
protected:

   virtual ~CacheData()
   {
      delete _trimData;
      delete _grid;
   }

   /*----- data members -----*/

   // Procedural mapping/displacement.
   VMState*                           _vm;
   const MetaFunction*                _mapping;
   const MetaFunction*                _displacement;
   int                                _dStackPos;
   Vector<Vec3f>                      _geomImage;
   Vec2i                              _dimImage;
   float                              _precision;

   // Patches.
   MetaSurface::Patch*                _lastPatch;
   ParametricPatch*                   _lastPp;
   LRUCache<void*,ParametricPatch>    _patches;
   TrimmingData*                      _trimData;
};

/*==============================================================================
   CLASS MetaSurface
==============================================================================*/

//------------------------------------------------------------------------------
//!
uint
MetaSurface::debugLevel()
{
   return _debugLevel;
}

//------------------------------------------------------------------------------
//!
void
MetaSurface::debugLevel( uint v )
{
   _debugLevel = v;
}

//------------------------------------------------------------------------------
//! 
MetaSurface::MetaSurface() :
   _numPatches(0),
   _numSubpatches(0),
   _begin( NULL ),
   _ptPool( 32 ),
   _vPool( 64 ),
   _spPool( 64 ),
   _pPool( 64 )
{
   // TODO: find better value for pool and hash.
   _cache     = new CacheData;
   _grid      = new HGrid();
   _aabbPool  = new AABBTree::Pool();
}

//------------------------------------------------------------------------------
//! 
MetaSurface::~MetaSurface()
{
   delete _grid;
   for( PatchIter pIt = patches(); pIt.isValid(); ++pIt )
   {
      delete pIt->_tree;
   }
}

//------------------------------------------------------------------------------
//! 
bool 
MetaSurface::trace( const Subpatch& sp, const Rayf& ray, Intersector::Hit& hit )
{
   // Consider subpatch to be a quad of 2 triangles.
   const Vec3f& a  = sp._corners[0]->_pos;
   const Vec3f& b  = sp._corners[1]->_pos;
   const Vec3f& c  = sp._corners[2]->_pos;
   const Vec3f& d  = sp._corners[3]->_pos;

   // 0: triangles (0,1,2),(0,2,3).
   // 1: triangles (0,1,3),(1,2,3).
   Vec3f ab = b - a;
   Vec3f ac = c - a;
   Vec3f ad = d - a;
   Vec3f n0 = cross(ab,ac);
   Vec3f n1 = cross(ac,ad);

   if( dot(n0,n1) > 0.0f )
   {
      if( Intersector::trace( a, b, c, ray, hit ) )
      {
         hit._normal = n0.normalize();
         return true;
      }
      if( Intersector::trace( a, c, d, ray, hit ) )
      {
         hit._normal = n1.normalize();
         return true;
      }
   }
   else
   {
      if( Intersector::trace( a, b, d, ray, hit ) )
      {
         hit._normal = cross(ab,ad).normalize();
         return true;
      }
      if( Intersector::trace( b, c, d, ray, hit ) )
      {
         hit._normal = cross(d-c,b-c).normalize();
         return true;
      } 
   }

   return false;
}

//------------------------------------------------------------------------------
//! 
void 
MetaSurface::computePoint( 
   const Subpatch& sp, 
   const Trimming& trim, 
   const Loop&     loop, 
   Vec3f&          pos 
) const
{
   Vector<Vertex*>& vptrs = _cache->_vptrs;
   vptrs.clear();
   // Compute a point contained in the loop.
   for( LoopLink* l = loop._link; l; l = l->_next )
   {
      vptrs.pushBack( l->_vertex );
   }
   
   pos = ::computePoint( vptrs, trim._x, trim._y );
   // Reproject on the subpatch.
   // 1. Find which triangle contains the point.
   if( trim._flags == 0 )
   {
      Vec2f v0 = sp._corners[0]->_pos( trim._x, trim._y ); 
      Vec2f v1 = sp._corners[2]->_pos( trim._x, trim._y ); 
      if( ccw( v1, v0, pos(trim._x, trim._y) ) )
      {
         // 2. reproject.
         trim._plane0.project( pos, trim._z );
      }
      else
      {
         // 2. reproject.
         trim._plane1.project( pos, trim._z );
      }
   }
   else
   {
      Vec2f v0 = sp._corners[3]->_pos( trim._x, trim._y ); 
      Vec2f v1 = sp._corners[1]->_pos( trim._x, trim._y ); 
      if( ccw( v1, v0, pos(trim._x, trim._y) ) )
      {
         // 2. reproject.
         trim._plane0.project( pos, trim._z );
      }
      else
      {
         // 2. reproject.
         trim._plane1.project( pos, trim._z );
      }
   }
}

//------------------------------------------------------------------------------
//! 
void 
MetaSurface::computeApproximation( 
   const Subpatch& sp, 
   Vec3f& v0,
   Vec3f& v1,
   Vec3f& v2,
   Vec3f& v3,
   Vec3f& n0,
   Vec3f& n1
)
{
   // Consider subpatch to be a quad of 2 triangles.
   const Vec3f& a  = sp._corners[0]->_pos;
   const Vec3f& b  = sp._corners[1]->_pos;
   const Vec3f& c  = sp._corners[2]->_pos;
   const Vec3f& d  = sp._corners[3]->_pos;

   // 0: triangles (0,1,2),(0,2,3).
   // 1: triangles (0,1,3),(1,2,3).
   Vec3f ab = b - a;
   Vec3f ac = c - a;
   Vec3f ad = d - a;
   n0 = cross(ab,ac);
   n1 = cross(ac,ad);

   if( dot(n0,n1) > 0.0f )
   {
      v0 = a;
      v1 = b;
      v2 = c;
      v3 = d;
      n0.normalize();
      n1.normalize();
   }
   else
   {
      v0 = d;
      v1 = a;
      v2 = b;
      v3 = c;
      n0 = cross(ab,ad).normalize();
      n1 = cross(d-c,b-c).normalize(); 
   }
}

//------------------------------------------------------------------------------
//! 
MetaSurface::Patch*
MetaSurface::createPatch()
{
   ++_numPatches;
   ++_numSubpatches;
   Patch* p = _pPool.alloc();
   memset( p, 0, sizeof(Patch) );

   p->_next                 = _begin;
   _begin                   = p;
   p->_flags                = 0;
   p->_edges                = 0;
   p->_tree                 = 0;
   p->_mapping              = 0;
   p->_displacement         = 0;
   p->_subpatch._corners[0] = &p->_corners[0];
   p->_subpatch._corners[1] = &p->_corners[1];
   p->_subpatch._corners[2] = &p->_corners[2];
   p->_subpatch._corners[3] = &p->_corners[3];
   p->_subpatch._patch      = p;
   p->_subpatch._flags      = 0x3c;
   p->_corners[0]._uv       = Vec2f( 0.0f, 0.0f );
   p->_corners[0]._next[1]  = &p->_corners[1];
   p->_corners[0]._next[2]  = &p->_corners[3];
   p->_corners[0]._flags    = (1<<0)|(1<<3); // Boundary patch vertex.
   p->_corners[0]._id       = INVALID;
   p->_corners[1]._uv       = Vec2f( 1.0f, 0.0f );
   p->_corners[1]._next[2]  = &p->_corners[2];
   p->_corners[1]._next[3]  = &p->_corners[0];
   p->_corners[1]._flags    = (1<<0)|(1<<1); // Boundary patch vertex.
   p->_corners[1]._id       = INVALID;
   p->_corners[2]._uv       = Vec2f( 1.0f, 1.0f );
   p->_corners[2]._next[0]  = &p->_corners[1];
   p->_corners[2]._next[3]  = &p->_corners[3];
   p->_corners[2]._flags    = (1<<1)|(1<<2); // Boundary patch vertex.
   p->_corners[2]._id       = INVALID;
   p->_corners[3]._uv       = Vec2f( 0.0f, 1.0f );
   p->_corners[3]._next[0]  = &p->_corners[0];
   p->_corners[3]._next[1]  = &p->_corners[2];
   p->_corners[3]._flags    = (1<<2)|(1<<3); // Boundary patch vertex.
   p->_corners[3]._id       = INVALID;
   p->_uv[0]                = Vec2f( 0.0f, 0.0f );
   p->_uv[1]                = Vec2f( 1.0f, 0.0f );
   p->_uv[2]                = Vec2f( 1.0f, 1.0f );
   p->_uv[3]                = Vec2f( 0.0f, 1.0f );
   return p;
}

//------------------------------------------------------------------------------
//! 
Vec3f*
MetaSurface::createPoint()
{
   Vec3f* v = _ptPool.alloc();
   *v       = Vec3f::zero();
   return v;
}

//------------------------------------------------------------------------------
//! 
void
MetaSurface::neighbors( Patch* p0, uint e0, Patch* p1, uint e1, int crease )
{
   // Set neighbor informations.
   p0->_neighbors[e0] = p1;
   p1->_neighbors[e1] = p0;
   p0->_edges         = setbits( p0->_edges, e0*2, 2, e1 );
   p1->_edges         = setbits( p1->_edges, e1*2, 2, e0 );
   p0->_edges         = setbits( p0->_edges, e0+8, 1, crease );
   p1->_edges         = setbits( p1->_edges, e1+8, 1, crease );

   Vertex* v0         = &p0->_corners[e0];
   Vertex* v1         = &p0->_corners[(e0+1)%4];
   Vertex* v2         = &p1->_corners[e1];
   Vertex* v3         = &p1->_corners[(e1+1)%4];
   v0->_next[e0]      = v3;
   v1->_next[e0]      = v2;
   v2->_next[e1]      = v1;
   v3->_next[e1]      = v0;
}

//------------------------------------------------------------------------------
//! 
void 
MetaSurface::subdivide( float gerror, float derror )
{
   // Compute surface details. We mainly needs at this point the displacement
   // data for the subdivision.
   computeDetails( derror );

   // Set the vm for computing mapping/displacement.
   _cache->displacementPrecision( derror );
   _cache->openVM();
   
   // Subdivide each patch taking into account geometric error and mapping error.
   for( PatchIter it = patches(); it.isValid(); ++it )
   {
      _cache->setMapping( it->_mapping );
      _cache->setDisplacement( it->_displacement );
      subdivide( *it, gerror );
   }

   _cache->closeVM();

   makeBilinear();
   buildSpatialSubdivision();
}

//------------------------------------------------------------------------------
//! 
void
MetaSurface::computeDetails( float /*precision*/ )
{
   uint i = 0;
   for( PatchIter pIt = patches(); pIt.isValid(); ++pIt )
   {
      pIt->_detailsID = i++;
   }
   /*
   // Allocate surface details patches.
   _details->clear();
   _details->reservePatches( numPatches() );

   // Fill in details patches size.
   for( PatchIter pIt = patches(); pIt.isValid(); ++pIt )
   {
      Vec2i size;
      Vec3f c0 = *pIt->_controlPts[0];
      Vec3f c1 = *pIt->_controlPts[1];
      Vec3f c2 = *pIt->_controlPts[2];
      Vec3f c3 = *pIt->_controlPts[3];

      float du0 = (c1-c0).length();
      float du1 = (c2-c3).length();
      float dv0 = (c3-c0).length();
      float dv1 = (c2-c1).length();

      size.x = CGM::ceili( CGM::max( du0, du1 ) / precision );
      size.y = CGM::ceili( CGM::max( dv0, dv1 ) / precision );

      pIt->_detailsID = _details->addPatch( size );
   }

   // Fill details neighbors.
   for( PatchIter pIt = patches(); pIt.isValid(); ++pIt )
   {
      SurfaceDetails::Patch& dp = _details->patch( pIt->_detailsID );
      dp._edges = pIt->_edges;
      dp._neighbors[0] = pIt->_neighbors[0]->_detailsID;
      dp._neighbors[1] = pIt->_neighbors[1]->_detailsID;
      dp._neighbors[2] = pIt->_neighbors[2]->_detailsID;
      dp._neighbors[3] = pIt->_neighbors[3]->_detailsID;
   }

   // Create atlas remapping data.
   _details->createAtlas();

   // Evaluate atlas.
   // Needed to evaluate the surface:
   // uv, pos, normal, blockId, face id, side uv, side id
   // PatchData
   _details->beginAtlas();
   for( PatchIter pIt = patches(); pIt.isValid(); ++pIt )
   {
      ParametricPatch* pp = _cache->paramPatch( pIt );
      _details->evaluatePatch( pIt->_detailsID, *pp, (pIt->_id)&(0x1fffffff), pIt->_id>>29 );
   }
   _details->endAtlas();
   */
}

//------------------------------------------------------------------------------
//! 
void 
MetaSurface::makeBilinear()
{
   // Snap corners vertices.
   for( PatchIter pIt = patches(); pIt.isValid(); ++pIt )
   {
      for( uint c = 0; c < 4; ++c )
      {
         Vertex* sv = &pIt->_corners[c];
         Patch* cp  = neighborPatch( *pIt, c );
         uint ce    = neighborEdge( *pIt, c );
         while( cp != pIt )
         {
            Patch& p = *cp;
            ce = (ce+1)%4;
            p._corners[ce]._pos = sv->_pos;
            cp = neighborPatch( p, ce );
            ce = neighborEdge( p, ce );
         }
         //pIt->_corners[c]._pos = *pIt->_controlPts[c];
      }
   }

   // Linearize edges.
   for( PatchIter pIt = patches(); pIt.isValid(); ++pIt )
   {
      for( uint c = 0; c < 4; ++c )
      {
         int nc   = (c+1)%4;
         int oc   = (c+2)%4;
         int axis = c&1;
         Vertex* sv = &pIt->_corners[c];
         Vertex* tv = &pIt->_corners[nc];

         while( sv != tv )
         {
            // Find end vertex.
            Vertex* ev;
            for( ev = sv->_next[nc]; ev->_next[oc] == 0; ev = ev->_next[nc] );

            // Linearized.
            for( Vertex* v = sv->_next[nc]; v != ev; v = v->_next[nc] )
            {
               float t = (v->_uv(axis)-sv->_uv(axis))/(ev->_uv(axis)-sv->_uv(axis));
               v->_pos = sv->_pos*(1.0f-t) + ev->_pos*t;
               v->_next[c]->_pos = v->_pos;
            }
            // Next subpatch.
            sv = ev;
         }
      }
   }
}

//------------------------------------------------------------------------------
//! 
void 
MetaSurface::subdivide( Patch& p, float gerror )
{
   // FIXME: only work for non-subdivided patches.
   ParametricPatch* pp = _cache->paramPatch( &p );

   // Compute corner vertices.
   Vec3f n[4];
   pp->parameters( p._corners[0]._uv, p._corners[0]._pos, n[0] );
   pp->parameters( p._corners[1]._uv, p._corners[1]._pos, n[1] );
   pp->parameters( p._corners[2]._uv, p._corners[2]._pos, n[2] );
   pp->parameters( p._corners[3]._uv, p._corners[3]._pos, n[3] );

#ifdef CONTROL_MESH
   p._corners[0]._pos = *p._controlPts[0];
   p._corners[1]._pos = *p._controlPts[1];
   p._corners[2]._pos = *p._controlPts[2];
   p._corners[3]._pos = *p._controlPts[3];
#endif

   _cache->_context._id   = p._id&0x1fffffff;
   _cache->_context._fid  = p._id>>29;
   _cache->_context._fnor = normalize(n[0]+n[1]+n[2]+n[3]);

   // Compute mapping.
   if( p._mapping )
   {
      p._uv[0] = _cache->computeMapping( p._corners[0]._pos, p._uv[0], n[0] );
      p._uv[1] = _cache->computeMapping( p._corners[1]._pos, p._uv[1], n[1] );
      p._uv[2] = _cache->computeMapping( p._corners[2]._pos, p._uv[2], n[2] );
      p._uv[3] = _cache->computeMapping( p._corners[3]._pos, p._uv[3], n[3] );
      //p._uv[0] = _cache->computeMapping( p._corners[0]._pos, p._corners[0]._uv, n[0] );
      //p._uv[1] = _cache->computeMapping( p._corners[1]._pos, p._corners[1]._uv, n[1] );
      //p._uv[2] = _cache->computeMapping( p._corners[2]._pos, p._corners[2]._uv, n[2] );
      //p._uv[3] = _cache->computeMapping( p._corners[3]._pos, p._corners[3]._uv, n[3] );
   }

#ifndef CONTROL_MESH
   // Compute displacement texture.
   if( p._displacement )
   {
      _cache->computeDisplacement( p );
      // Displacement
      p._corners[0]._pos = _cache->displace( p._corners[0]._uv );
      p._corners[1]._pos = _cache->displace( p._corners[1]._uv );
      p._corners[2]._pos = _cache->displace( p._corners[2]._uv );
      p._corners[3]._pos = _cache->displace( p._corners[3]._uv );

      // Subdivide subpatches recursively (iteratively!).
      for( SubpatchIter it = subpatches(p); it.isValid(); )
      {
         if( !subdivideDisplaced( *it, gerror ) ) ++it;
      }
   }
   else
   {
      // Subdivide subpatches recursively (iteratively!).
      for( SubpatchIter it = subpatches(p); it.isValid(); )
      {
         if( !subdivide( *it, gerror ) ) ++it;
      }
   }
#endif
}

//------------------------------------------------------------------------------
//! 
bool 
MetaSurface::subdivide( Subpatch& sp, float gerror )
{
   // Compute center vertex.
   Vertex& v0 = *sp._corners[0];
   Vertex& v1 = *sp._corners[1];
   Vertex& v2 = *sp._corners[2];
   Vertex& v3 = *sp._corners[3];

   ParametricPatch* pp = _cache->paramPatch( sp._patch );

   Vec3f n[5];
   Vec3f pos[5];
   Vec3f rpos[5];
   Vec2f uv[5];

   uv[0] = (v0._uv+v1._uv)*0.5f;
   uv[1] = (v1._uv+v2._uv)*0.5f;
   uv[2] = (v2._uv+v3._uv)*0.5f;
   uv[3] = (v3._uv+v0._uv)*0.5f;
   uv[4] = (v0._uv+v2._uv)*0.5f;

   Vec3f p0 = v0._pos;
   Vec3f p1 = v1._pos;
   Vec3f p2 = v2._pos;
   Vec3f p3 = v3._pos;

   pp->parameters( uv[0], rpos[0], n[0] );
   pp->parameters( uv[1], rpos[1], n[1] );
   pp->parameters( uv[2], rpos[2], n[2] );
   pp->parameters( uv[3], rpos[3], n[3] );
   pp->parameters( uv[4], rpos[4], n[4] );
   pos[4] = rpos[4];

   Vec3f lpos;
   int sub      = 0;
   float error2 = gerror*gerror;

   // Edge 0.
   lpos = (p0+p1)*0.5f;
   if( (sp._flags&4) && sqrLength( rpos[0]-lpos ) > error2 )
   {
      sub   |= (1|4);
      pos[0] = rpos[0];
   }
   else
   {
      pos[0] = lpos;
   }

   // Edge 2.
   lpos = (p2+p3)*0.5f;
   if( (sp._flags&16) && sqrLength( rpos[2]-lpos ) > error2 )
   {
      sub   |= (1|16);
      pos[2] = rpos[2];
   }
   else
   {
      pos[2] = lpos;
   }

   // Edge 1.
   lpos = (p1+p2)*0.5f;
   if( (sp._flags&8) && sqrLength( rpos[1]-lpos ) > error2 )
   {
      sub   |= (2|8);
      pos[1] = rpos[1];
   }
   else
   {
      pos[1] = lpos;
   }

   // Edge 3.
   lpos = (p3+p0)*0.5f;
   if( (sp._flags&32) && sqrLength( rpos[3]-lpos ) > error2 )
   {
      sub   |= (2|32);
      pos[3] = rpos[3];
   }
   else
   {
      pos[3] = lpos;
   }

   // Interior 0.
   lpos = (rpos[0]+rpos[2])*0.5f;
   if( (sub&2) == 0 && sqrLength( rpos[4]-lpos ) > error2 )
   {
      if( v2._uv.x-v0._uv.x > 0.0625f )
      sub |= 2;
   }

   // Interior 1.
   lpos = (rpos[1]+rpos[3])*0.5f;
   if( (sub&1) == 0 && sqrLength( rpos[4]-lpos ) > error2 )
   {
      if( v2._uv.y-v0._uv.y > 0.0625f )
      sub |= 1;
   }

   //if( v2._uv.y-v0._uv.y > 0.0625f ) sub = 1|2|4|8|16|32;

   sp._flags = (sub&0x3c) | (sp._flags&(~0x3c));
   switch( sub & 3 )
   {
      case 0: return false;
      case 1:
         subdivideu( sp, pos );
         return true;
      case 2:
         subdividev( sp, pos );
         return true;
      default:
         subdivideuv( sp, pos );
         return true;
   }
   return false;
}

/*
//------------------------------------------------------------------------------
//! 
bool 
MetaSurface::subdivideDisplaced( Subpatch& sp, float gerror )
{
   // Compute center vertex.
   Vertex& v0 = *sp._corners[0];
   Vertex& v1 = *sp._corners[1];
   Vertex& v2 = *sp._corners[2];
   Vertex& v3 = *sp._corners[3];

   Vec3f pos[5];
   Vec3f rpos[5];
   Vec2f uv[5];

   uv[0] = (v0._uv+v1._uv)*0.5f;
   uv[1] = (v1._uv+v2._uv)*0.5f;
   uv[2] = (v2._uv+v3._uv)*0.5f;
   uv[3] = (v3._uv+v0._uv)*0.5f;
   uv[4] = (v0._uv+v2._uv)*0.5f;

   Vec3f p0 = v0._pos;
   Vec3f p1 = v1._pos;
   Vec3f p2 = v2._pos;
   Vec3f p3 = v3._pos;

   rpos[0] = _cache->displace( uv[0] );
   rpos[1] = _cache->displace( uv[1] );
   rpos[2] = _cache->displace( uv[2] );
   rpos[3] = _cache->displace( uv[3] );
   rpos[4] = _cache->displace( uv[4] );

   pos[4] = rpos[4];

   Vec3f lpos;
   int sub      = 0;
   float error2 = gerror*gerror;

   // Edge 0.
   lpos = (p0+p1)*0.5f;
   if( (sp._flags&4) && sqrLength( rpos[0]-lpos ) > error2 )
   {
      sub   |= (1|4);
      pos[0] = rpos[0];
   }
   else
   {
      pos[0] = lpos;
   }

   // Edge 2.
   lpos = (p2+p3)*0.5f;
   if( (sp._flags&16) && sqrLength( rpos[2]-lpos ) > error2 )
   {
      sub   |= (1|16);
      pos[2] = rpos[2];
   }
   else
   {
      pos[2] = lpos;
   }

   // Edge 1.
   lpos = (p1+p2)*0.5f;
   if( (sp._flags&8) && sqrLength( rpos[1]-lpos ) > error2 )
   {
      sub   |= (2|8);
      pos[1] = rpos[1];
   }
   else
   {
      pos[1] = lpos;
   }

   // Edge 3.
   lpos = (p3+p0)*0.5f;
   if( (sp._flags&32) && sqrLength( rpos[3]-lpos ) > error2 )
   {
      sub   |= (2|32);
      pos[3] = rpos[3];
   }
   else
   {
      pos[3] = lpos;
   }

   // Interior 0.
   lpos = (rpos[0]+rpos[2])*0.5f;
   if( (sub&2) == 0 && sqrLength( rpos[4]-lpos ) > error2 )
   {
      if( v2._uv.x-v0._uv.x > 0.0625f )
      sub |= 2;
   }

   // Interior 1.
   lpos = (rpos[1]+rpos[3])*0.5f;
   if( (sub&1) == 0 && sqrLength( rpos[4]-lpos ) > error2 )
   {
      if( v2._uv.y-v0._uv.y > 0.0625f )
      sub |= 1;
   }

   //if( v2._uv.y-v0._uv.y > 0.0625f ) sub = 1|2|4|8|16|32; else sub = 0;
   if( v2._uv.y-v0._uv.y < 0.0625f ) sub = 0;
   if( v2._uv.x-v0._uv.x < 0.0625f ) sub = 0;

   sp._flags = (sub&0x3c) | (sp._flags&(~0x3c));
   switch( sub & 3 )
   {
      case 0: return false;
      case 1:
         subdivideu( sp, pos );
         return true;
      case 2:
         subdividev( sp, pos );
         return true;
      default:
         subdivideuv( sp, pos );
         return true;
   }
   return false;
}
*/

//------------------------------------------------------------------------------
//! 
bool 
MetaSurface::subdivideDisplaced( Subpatch& sp, float gerror )
{
   // Compute center vertex.
   Vertex& v0 = *sp._corners[0];
   Vertex& v1 = *sp._corners[1];
   Vertex& v2 = *sp._corners[2];
   Vertex& v3 = *sp._corners[3];

   Vec3f pos[5];
   int sub   = _cache->flatness( v0._uv, v2._uv, sp._flags >> 2, gerror );
   //int sub;
   //if( v2._uv.y-v0._uv.y >= 0.0625f ) sub = 1|2|4|8|16|32; else sub = 0;
   sp._flags = (sub&0x3c) | (sp._flags&(~0x3c));
   switch( sub & 3 )
   {
      case 0: return false;
      case 1:
         pos[0] = (sub&4)  ? _cache->displace( (v0._uv+v1._uv)*0.5f ) : (v0._pos+v1._pos)*0.5f;
         pos[2] = (sub&16) ? _cache->displace( (v2._uv+v3._uv)*0.5f ) : (v2._pos+v3._pos)*0.5f;
         subdivideu( sp, pos );
         return true;
      case 2:
         pos[1] = (sub&8)  ? _cache->displace( (v1._uv+v2._uv)*0.5f ) : (v1._pos+v2._pos)*0.5f;
         pos[3] = (sub&32) ? _cache->displace( (v3._uv+v0._uv)*0.5f ) : (v3._pos+v0._pos)*0.5f;
         subdividev( sp, pos );
         return true;
      default:
         pos[0] = (sub&4)  ? _cache->displace( (v0._uv+v1._uv)*0.5f ) : (v0._pos+v1._pos)*0.5f;
         pos[1] = (sub&8)  ? _cache->displace( (v1._uv+v2._uv)*0.5f ) : (v1._pos+v2._pos)*0.5f;
         pos[2] = (sub&16) ? _cache->displace( (v2._uv+v3._uv)*0.5f ) : (v2._pos+v3._pos)*0.5f;
         pos[3] = (sub&32) ? _cache->displace( (v3._uv+v0._uv)*0.5f ) : (v3._pos+v0._pos)*0.5f;
         pos[4] = _cache->displace( (v0._uv+v2._uv)*0.5f );
         subdivideuv( sp, pos );
         return true;
   }
   return false;
}

//------------------------------------------------------------------------------
//! 
void 
MetaSurface::subdivide( Subpatch& sp )
{
   _numSubpatches += 3;

   Vertex* v0 = sp._corners[0];
   Vertex* v1 = sp._corners[1];
   Vertex* v2 = sp._corners[2];
   Vertex* v3 = sp._corners[3];

   // Center uv.
   Vec2f uv = (v0->_uv + v2->_uv)*0.5f;

   // Add splitting vertices.
   Vertex* nv0 = subdivideEdge( sp, 0, 0.5f, 0 );
   Vertex* nv1 = subdivideEdge( sp, 1, 0.5f, 0 );
   Vertex* nv2 = subdivideEdge( sp, 2, 0.5f, 0 );
   Vertex* nv3 = subdivideEdge( sp, 3, 0.5f, 0 );

   // Add center vertex.
   Vertex* nv4   = _vPool.alloc();
   nv4->_uv      = uv;
   nv4->_id      = INVALID;
   nv4->_flags   = 0;
   nv4->_next[0] = nv0;
   nv4->_next[1] = nv1;
   nv4->_next[2] = nv2;
   nv4->_next[3] = nv3;
   nv4->_pos     = computePosition( *sp._patch, uv );

   nv0->_next[2] = nv4;
   nv1->_next[3] = nv4;
   nv2->_next[0] = nv4;
   nv3->_next[1] = nv4;

   // Add subpatches.
   Subpatch* sp1 = _spPool.alloc();
   Subpatch* sp2 = _spPool.alloc();
   Subpatch* sp3 = _spPool.alloc();
   sp1->_next    = sp2;
   sp2->_next    = sp3;
   sp3->_next    = sp._next;
   sp._next      = sp1;

   sp._corners[1]   = nv0;
   sp._corners[2]   = nv4;
   sp._corners[3]   = nv3;

   sp1->_corners[0] = nv0;
   sp1->_corners[1] = v1;
   sp1->_corners[2] = nv1;
   sp1->_corners[3] = nv4;
   sp1->_patch      = sp._patch;
   sp1->_flags      = 0;

   sp2->_corners[0] = nv4;
   sp2->_corners[1] = nv1;
   sp2->_corners[2] = v2;
   sp2->_corners[3] = nv2;
   sp2->_patch      = sp._patch;
   sp2->_flags      = 0;

   sp3->_corners[0] = nv3;
   sp3->_corners[1] = nv4;
   sp3->_corners[2] = nv2;
   sp3->_corners[3] = v3;
   sp3->_patch      = sp._patch;
   sp3->_flags      = 0;
}

//------------------------------------------------------------------------------
//! 
void 
MetaSurface::subdivideu( Subpatch& sp, const Vec3f pos[] )
{
   _numSubpatches  += 1;

   Vertex* v1       = sp._corners[1];
   Vertex* v2       = sp._corners[2];

   // Add splitting vertices.
   Vertex* nv0      = subdivideEdge( sp, 0, 0.5f, &pos[0] );
   Vertex* nv2      = subdivideEdge( sp, 2, 0.5f, &pos[2] );
   nv0->_next[2]    = nv2;
   nv2->_next[0]    = nv0;

   // Add subpatches.
   Subpatch* sp1    = _spPool.alloc();
   sp1->_next       = sp._next;
   sp._next         = sp1;
   sp._corners[1]   = nv0;
   sp._corners[2]   = nv2;
   sp1->_corners[0] = nv0;
   sp1->_corners[1] = v1;
   sp1->_corners[2] = v2;
   sp1->_corners[3] = nv2;
   sp1->_patch      = sp._patch;
   sp1->_flags      = sp._flags | 0x20;
   sp._flags       |= 0x08;
}

//------------------------------------------------------------------------------
//! 
void 
MetaSurface::subdividev( Subpatch& sp, const Vec3f pos[] )
{
   _numSubpatches  += 1;

   Vertex* v2       = sp._corners[2];
   Vertex* v3       = sp._corners[3];

   // Add splitting vertices.
   Vertex* nv1      = subdivideEdge( sp, 1, 0.5f, &pos[1] );
   Vertex* nv3      = subdivideEdge( sp, 3, 0.5f, &pos[3] );
   nv1->_next[3]    = nv3;
   nv3->_next[1]    = nv1;

   // Add subpatches.
   Subpatch* sp1    = _spPool.alloc();
   sp1->_next       = sp._next;
   sp._next         = sp1;
   sp._corners[2]   = nv1;
   sp._corners[3]   = nv3;
   sp1->_corners[0] = nv3;
   sp1->_corners[1] = nv1;
   sp1->_corners[2] = v2;
   sp1->_corners[3] = v3;
   sp1->_patch      = sp._patch;
   sp1->_flags      = sp._flags | 0x04;
   sp._flags       |= 0x10;
}

//------------------------------------------------------------------------------
//! 
void 
MetaSurface::subdivideuv( Subpatch& sp, const Vec3f pos[] )
{
   _numSubpatches  += 3;

   Vertex* v0       = sp._corners[0];
   Vertex* v1       = sp._corners[1];
   Vertex* v2       = sp._corners[2];
   Vertex* v3       = sp._corners[3];

   // Add splitting vertices.
   Vertex* nv0      = subdivideEdge( sp, 0, 0.5f, &pos[0] );
   Vertex* nv1      = subdivideEdge( sp, 1, 0.5f, &pos[1] );
   Vertex* nv2      = subdivideEdge( sp, 2, 0.5f, &pos[2] );
   Vertex* nv3      = subdivideEdge( sp, 3, 0.5f, &pos[3] );

   // Add center vertex.
   Vertex* nv4      = _vPool.alloc();
   nv4->_uv         = (v0->_uv + v2->_uv)*0.5f;
   nv4->_id         = INVALID;
   nv4->_flags      = 0;
   nv4->_next[0]    = nv0;
   nv4->_next[1]    = nv1;
   nv4->_next[2]    = nv2;
   nv4->_next[3]    = nv3;
   nv4->_pos        = pos[4];

   nv0->_next[2]    = nv4;
   nv1->_next[3]    = nv4;
   nv2->_next[0]    = nv4;
   nv3->_next[1]    = nv4;

   // Add subpatches.
   Subpatch* sp1    = _spPool.alloc();
   Subpatch* sp2    = _spPool.alloc();
   Subpatch* sp3    = _spPool.alloc();
   sp1->_next       = sp2;
   sp2->_next       = sp3;
   sp3->_next       = sp._next;
   sp._next         = sp1;

   sp._corners[1]   = nv0;
   sp._corners[2]   = nv4;
   sp._corners[3]   = nv3;

   sp1->_corners[0] = nv0;
   sp1->_corners[1] = v1;
   sp1->_corners[2] = nv1;
   sp1->_corners[3] = nv4;
   sp1->_patch      = sp._patch;
   sp1->_flags      = sp._flags | 0x30;

   sp2->_corners[0] = nv4;
   sp2->_corners[1] = nv1;
   sp2->_corners[2] = v2;
   sp2->_corners[3] = nv2;
   sp2->_patch      = sp._patch;
   sp2->_flags      = sp._flags | 0x24;

   sp3->_corners[0] = nv3;
   sp3->_corners[1] = nv4;
   sp3->_corners[2] = nv2;
   sp3->_corners[3] = v3;
   sp3->_patch      = sp._patch;
   sp3->_flags      = sp._flags | 0x0c;

   sp._flags       |= 0x18;
}

//------------------------------------------------------------------------------
//! 
void
MetaSurface::computeParameters( Patch& p, const Vec2f& uv, Vec3f& pos, Vec3f& normal ) const
{
   //_details->parameters( p._detailsID, uv, pos, normal );
   ParametricPatch* pp = _cache->paramPatch( &p );
   pp->parameters( uv, pos, normal );
}

//------------------------------------------------------------------------------
//! 
Vec3f
MetaSurface::computePosition( Patch& p, const Vec2f& uv ) const
{
   //return _details->position( p._detailsID, uv );
   Vec3f pos;
   Vec3f n;
   ParametricPatch* pp = _cache->paramPatch( &p );
   pp->parameters( uv, pos, n );
   return pos;
}

//------------------------------------------------------------------------------
//!
void 
MetaSurface::resetVertices( Patch& p ) const
{
   // Add triangles.
   for( SubpatchIter sIt = subpatches(p); sIt.isValid(); ++sIt )
   {
      // Do not need to be triangulate?
      if( isHidden( *sIt ) ) continue;

      // Normal triangulation?
      if( !isTrimmed(*sIt) )
      {
         // Accumulate all patch vertices in CCW and compute sides size.
         Vertex* v = sIt->_corners[0];

         for( uint c = 0; c < 4; ++c )
         {
            for( ; v != sIt->_corners[(c+1)%4]; v = v->_next[(c+1)%4] )
            {
               v->_id = INVALID;
            }
         }
      }
      else
      {
         // Trimmed subpatch triangulation.
         TrimmingData* trimData = _cache->trimmingData();
         Trimming* trim         = trimData->trim( sIt );
         for( Loop* loop = trim->_loops; loop; loop = loop->_next )
         {
            // Do not need to be triangulate?
            if( isHidden( *loop ) ) continue;

            for( LoopLink* l = loop->_link; l; l = l->_next )
            {
               Vertex* v = l->_vertex;
               v->_id = INVALID;
            }
         }
      }
   }
}

//------------------------------------------------------------------------------
//!
void 
MetaSurface::triangulate( 
   Patch&         p, 
   Vector<float>& vertices, 
   Vector<uint>&  indices,
   Vector<uint>*  faceInfos,
   bool           normals
) const
{
   // Do not need to be triangulate?
   if( isHidden( p ) ) return;

   uint stride = normals ? 8 : 5;
   uint offset = uint(vertices.size()) / stride;
   uint id     = 0;
   bool flip   = isFlipped(p);
   float nf    = flip ? -1.0f : 1.0f;
   float error = 2.0f*_errormax;

   uint startIndice = uint(indices.size());

   //if( p._detailsID <= 18 || p._detailsID >= 43 ) return;
   //if( p._detailsID != 43 ) return;
   //if( p._detailsID != 18 ) return;
   //if( p._detailsID != 18 && p._detailsID != 43) return;

   // Allocate vertex grid cache when starting anew.
   if( offset == 0 )
   {
      delete _cache->_grid;
      _cache->_grid = new Grid<Vertex*>( numSubpatches()*2, 1.0f/128.0f );
   }

   Vector<Vertex*>& vptrs = _cache->_vptrs;
   Grid<Vertex*>* grid    = _cache->_grid;
   uint nbVertices[4]; 

   resetVertices( p );

   // Compute patch uvs.
   //Vec2f uvp;
   //Vec2f du;
   //Vec2f dv;
   //_details->mapping( p._detailsID, uvp, du, dv );
   //Vec2f uvp(0.0f);
   //Vec2f du(1.0f,0.0f);
   //Vec2f dv(0.0f,1.0f);
   Vec2f uv0  = p._uv[0];
   Vec2f uv1  = p._uv[3];
   Vec2f duv0 = p._uv[1] - uv0;
   Vec2f duv1 = p._uv[2] - uv1;

   Vec3f n;
   Vec3f tp;

   // Add triangles.
   for( SubpatchIter sIt = subpatches(p); sIt.isValid(); ++sIt )
   {
      // Do not need to be triangulate?
      if( isHidden( *sIt ) ) continue;
      //if( !equal( sIt->_corners[0]->_uv, Vec2f( 0.125f, 0.125f ) ) ) continue; // 43
      //if( !equal( sIt->_corners[0]->_uv, Vec2f( 0.0f, 0.5f ) ) ) continue; // 18

      // Normal triangulation?
      if( !isTrimmed(*sIt) )
      {
         // Accumulate all patch vertices in CCW and compute sides size.
         vptrs.clear();
         Vertex* v = sIt->_corners[0];

         for( uint c = 0; c < 4; ++c )
         {
            nbVertices[c] = 0;
            for( ; v != sIt->_corners[(c+1)%4]; v = v->_next[(c+1)%4] )
            {
#ifdef PER_SUBPATCH_MAPPING
               // Add a new vertex.
               {
                  v->_id = offset + id++;
                  vertices.pushBack( v->_pos.x );
                  vertices.pushBack( v->_pos.y );
                  vertices.pushBack( v->_pos.z );
                  Vec2f uv = (v->_uv-sIt->_corners[0]->_uv)/(sIt->_corners[2]->_uv-sIt->_corners[0]->_uv);
                  vertices.pushBack( uv.x );
                  vertices.pushBack( uv.y );
                  if( normals )
                  {
                     computeParameters( p, v->_uv, tp, n );
                     vertices.pushBack( nf*n.x );
                     vertices.pushBack( nf*n.y );
                     vertices.pushBack( nf*n.z );
                  }
               }
#else
               // Add a new vertex.
               if( v->_id == INVALID ) 
               {
                  //Vec2f uv = uvp + du*v->_uv.x + dv*v->_uv.y;
                  Vec2f uv = CGM::bilinear( uv0, duv0, uv1, duv1, v->_uv.x, v->_uv.y );
                  if( normals )
                  {
                     computeParameters( p, v->_uv, tp, n );
                     n *= nf;
                  }

                  // Search for vertices.
                  Vec3i c0 = grid->cellCoord( v->_pos-error );
                  Vec3i c1 = grid->cellCoord( v->_pos+error );
                  for( int x = c0.x; x <= c1.x ; ++x )
                  {
                     for( int y = c0.y; y <= c1.y; ++y )
                     {
                        for( int z = c0.z; z <= c1.z; ++z )
                        {
                           Grid<Vertex*>::Link* l = grid->cell( Vec3i(x,y,z) );
                           for( ; l; l = l->_next )
                           {
                              // Have we found the vertex?
                              if( sqrLength( l->_obj->_pos-v->_pos) < (error*error) )
                              {
                                 // Test for vertex similarity.
                                 if( normals )
                                 {
                                    Vec2f& cuv = (Vec2f&)vertices[l->_obj->_id*stride+3];
                                    Vec3f& cn  = (Vec3f&)vertices[l->_obj->_id*stride+5];
                                    if( equal( cuv, uv ) && equal( cn, n ) ) v->_id = l->_obj->_id;
                                 }
                                 else
                                 {
                                    Vec2f& cuv = (Vec2f&)vertices[l->_obj->_id*stride+3];
                                    if( equal( cuv, uv ) ) v->_id = l->_obj->_id;
                                 }
                              }
                           }
                        }
                     }
                  }

                  if( v->_id == INVALID )
                  {
                     v->_id = offset + id++;
                     vertices.pushBack( v->_pos.x );
                     vertices.pushBack( v->_pos.y );
                     vertices.pushBack( v->_pos.z );
                     vertices.pushBack( uv.x );
                     vertices.pushBack( uv.y );
                     if( normals )
                     {
                        vertices.pushBack( n.x );
                        vertices.pushBack( n.y );
                        vertices.pushBack( n.z );
                     }
                     grid->add( grid->cellID( v->_pos ), v );
                  }
               }
#endif
               vptrs.pushBack( v );
               nbVertices[c]++;
            }
         }
         ::triangulate( vptrs, nbVertices, flip, indices );
      }
      else
      {
         // Trimmed subpatch triangulation.
         TrimmingData* trimData = _cache->trimmingData();
         Trimming* trim         = trimData->trim( sIt );

         //dump( *sIt );
         //dump( *trim );

         for( Loop* loop = trim->_loops; loop; loop = loop->_next )
         {
            // Do not need to be triangulate?
            if( isHidden( *loop ) ) continue;

            vptrs.clear();
            for( LoopLink* l = loop->_link; l; l = l->_next )
            {
               Vertex* v = l->_vertex;
#ifdef PER_SUBPATCH_MAPPING
               // Add a new vertex.
               {
                  v->_id = offset + id++;
                  vertices.pushBack( v->_pos.x );
                  vertices.pushBack( v->_pos.y );
                  vertices.pushBack( v->_pos.z );
                  Vec2f uv = (v->_uv-sIt->_corners[0]->_uv)/(sIt->_corners[2]->_uv-sIt->_corners[0]->_uv);
                  vertices.pushBack( uv.x );
                  vertices.pushBack( uv.y );
                  if( normals )
                  {
                     computeParameters( p, v->_uv, tp, n );
                     vertices.pushBack( nf*n.x );
                     vertices.pushBack( nf*n.y );
                     vertices.pushBack( nf*n.z );
                  }
               }
#else
               // Add a new vertex.
               if( v->_id == INVALID ) 
               {
                  //Vec2f uv = uvp + du*v->_uv.x + dv*v->_uv.y;
                  Vec2f uv = CGM::bilinear( uv0, duv0, uv1, duv1, v->_uv.x, v->_uv.y );
                  if( normals )
                  {
                     computeParameters( p, v->_uv, tp, n );
                     n *= nf;
                  }

                  // Search for vertices.
                  Vec3i c0 = grid->cellCoord( v->_pos-error );
                  Vec3i c1 = grid->cellCoord( v->_pos+error );
                  for( int x = c0.x; x <= c1.x ; ++x )
                  {
                     for( int y = c0.y; y <= c1.y; ++y )
                     {
                        for( int z = c0.z; z <= c1.z; ++z )
                        {
                           Grid<Vertex*>::Link* l = grid->cell( Vec3i(x,y,z) );
                           for( ; l; l = l->_next )
                           {
                              // Have we found the vertex?
                              if( sqrLength( l->_obj->_pos-v->_pos) < (error*error) )
                              {
                                 // Test for vertex similarity.
                                 if( normals )
                                 {
                                    Vec2f& cuv = (Vec2f&)vertices[l->_obj->_id*stride+3];
                                    Vec3f& cn  = (Vec3f&)vertices[l->_obj->_id*stride+5];
                                    if( equal( cuv, uv ) && equal( cn, n ) ) v->_id = l->_obj->_id;
                                 }
                                 else
                                 {
                                    Vec2f& cuv = (Vec2f&)vertices[l->_obj->_id*stride+3];
                                    if( equal( cuv, uv ) ) v->_id = l->_obj->_id;
                                 }
                              }
                           }
                        }
                     }
                  }

                  if( v->_id == INVALID )
                  {
                     v->_id = offset + id++;
                     vertices.pushBack( v->_pos.x );
                     vertices.pushBack( v->_pos.y );
                     vertices.pushBack( v->_pos.z );
                     vertices.pushBack( uv.x );
                     vertices.pushBack( uv.y );
                     if( normals )
                     {
                        vertices.pushBack( n.x );
                        vertices.pushBack( n.y );
                        vertices.pushBack( n.z );
                     }
                     grid->add( grid->cellID( v->_pos ), v );
                  }
               }
#endif
               vptrs.pushBack( v );
            }
            ::triangulate( vptrs, indices, flip, trim->_x, trim->_y );
         }
      }
   }

   // Face info.
   if( faceInfos )
   {
      int numTriangles = int(indices.size() - startIndice) / 3;
      for( ; numTriangles > 0 ; --numTriangles )
      {
         faceInfos->pushBack( p._id );
         faceInfos->pushBack( p._detailsID );
      }
   }
}

//------------------------------------------------------------------------------
//! 
void 
MetaSurface::buildSpatialSubdivision()
{
   if ( _debugLevel > 0 )
   {
      StdErr << "# of patches: " << _numPatches << "\n";
      StdErr << "# of subpatches: " << _numSubpatches << "\n";
   }

   Vector<void*> pptr;
   Vector<AABBoxf> bboxes;

   for( PatchIter pIt = patches(); pIt.isValid(); ++pIt )
   {
      // clear bbs
      pptr.clear();
      bboxes.clear();
      for( SubpatchIter spIt = subpatches(*pIt); spIt.isValid(); ++spIt )
      {
         // Compute bb.
         bboxes.pushBack( computeAABB( *spIt ) );
         pptr.pushBack( spIt );
      }
      // if > 1 create aabbtree.
      if( pptr.size() > 1 )
      {
         pIt->_tree = new AABBTree( _aabbPool );
         pIt->_tree->create( bboxes.data(), pptr.data(), uint(pptr.size()) );
         // Add patch to hgrid.
         _grid->add( pIt, pIt->_tree->region() );
      }
      else
      {
         // Add patch to hgrid.
         _grid->add( pIt, bboxes[0] );
      }
   }
}

/*==============================================================================
   Vertex insertion
==============================================================================*/

//------------------------------------------------------------------------------
//! 
MetaSurface::Vertex*
MetaSurface::insertVertex( 
   Subpatch&    sp, 
   uint         edge, 
   float        t,
   Vertex*      v,
   int          dir,
   const Vec3f& pos, 
   const Vec2f& uv 
)
{
   int idir  = (dir+2)%4;
   Vertex* n = v->_next[idir];

   // Create vertex.
   Vertex* nv              = _vPool.alloc();
   nv->_pos                = pos;
   nv->_uv                 = uv;
   nv->_id                 = INVALID;
   nv->_flags              = v->_flags & n->_flags;
   nv->_next[dir]          = v;
   nv->_next[idir]         = n;
   nv->_next[(dir+1)%4]    = 0;
   nv->_next[(idir+1)%4]   = 0;

   v->_next[idir]          = nv;
   n->_next[dir]           = nv;

   // Insert another vertex on boundary.
   if( nv->_flags & 0xf )
   {
      uint nedge  = neighborEdge( *sp._patch, edge );
      v           = v->_next[edge];
      n           = n->_next[edge];

      Vertex* nnv = _vPool.alloc();
      nnv->_flags = v->_flags & n->_flags;
      nnv->_id    = INVALID;

      // Update vertices neighbors
      if( ((dir+4-edge)%4) == 3 )
      {
         nnv->_next[(nedge+1)%4]  = v;
         nnv->_next[(nedge+3)%4]  = n;
         v->_next[(nedge+3)%4]    = nnv;
         n->_next[(nedge+1)%4]    = nnv;
      }
      else
      {
         nnv->_next[(nedge+3)%4]  = v;
         nnv->_next[(nedge+1)%4]  = n;
         v->_next[(nedge+1)%4]    = nnv;
         n->_next[(nedge+3)%4]    = nnv;
      }

      nnv->_next[(nedge+2)%4] = 0;
      nv->_next[edge]         = nnv;
      nnv->_next[nedge]       = nv;

      // Compute uv and position.
      nnv->_uv  = CGM::linear2( sp._corners[edge]->_next[edge]->_uv, 
                                sp._corners[(edge+1)%4]->_next[edge]->_uv, t );
      nnv->_pos = nv->_pos;
   }

   return nv;
}

//------------------------------------------------------------------------------
//! 
MetaSurface::Vertex*
MetaSurface::subdivideEdge( Subpatch& sp, uint edge, float t, const Vec3f* pos )
{
   uint axis = edge&1; 
   Vec2f uv  = CGM::linear2( sp._corners[edge]->_uv, sp._corners[(edge+1)%4]->_uv, t );
   
   // Search vertex.
   const float epsilon = (1.0f/65536.0f);
   Vertex* v           = 0;
   Vertex* vs          = 0;
   switch( edge )
   {
      case 0: vs = sp._corners[0]; v = prevVertex( vs, sp._corners[1], 0, uv.x+epsilon ); break;
      case 1: vs = sp._corners[1]; v = prevVertex( vs, sp._corners[2], 1, uv.y+epsilon ); break;
      case 2: vs = sp._corners[3]; v = prevVertex( vs, sp._corners[2], 0, uv.x+epsilon ); break;
      case 3: vs = sp._corners[0]; v = prevVertex( vs, sp._corners[3], 1, uv.y+epsilon ); break;
   }

   // Vertex found?
   if( (v != vs) && CGM::equal( v->_uv(axis), uv(axis), epsilon ) ) return v;

   return insertVertex( sp, edge, t, v, (axis+3)%4,pos ? *pos : computePosition( *sp._patch, uv ), uv );
}

//------------------------------------------------------------------------------
//! 
MetaSurface::Vertex*
MetaSurface::insertVertex( Subpatch& sp, uint edge, float t )
{
   uint ne = (edge+1)%4;

   if( t <= 0.0f ) return sp._corners[edge];
   if( t >= 1.0f ) return sp._corners[ne];

   uint axis = edge&1; 
   Vec2f uv  = CGM::linear2( sp._corners[edge]->_uv, sp._corners[ne]->_uv, t );
   Vec3f pos = CGM::linear2( sp._corners[edge]->_pos, sp._corners[ne]->_pos, t );

   // Search vertex.
   Vertex* v = 0;
   switch( edge )
   {
      case 0: v = prevVertex( sp._corners[0], sp._corners[1], 0, uv.x ); break;
      case 1: v = prevVertex( sp._corners[1], sp._corners[2], 1, uv.y ); break;
      case 2: v = prevVertex( sp._corners[3], sp._corners[2], 0, uv.x ); break;
      case 3: v = prevVertex( sp._corners[0], sp._corners[3], 1, uv.y ); break;
   }

   // Vertex found?
   if( equal( v->_pos, pos, _errormax ) ) return v;
   if( equal( v->_next[axis+1]->_pos, pos, _errormax ) ) return v->_next[axis+1];

   return insertVertex( sp, edge, t, v, (axis+3)%4, pos, uv );
}

//------------------------------------------------------------------------------
//! 
MetaSurface::Vertex* 
MetaSurface::insertVertex( Subpatch& sp, Trimming* trim, const Vec3f& pos )
{
   // Rounding precisions.
   const float error  = _errormax;
   const float error2 = error*error;

   // Find point in trimming info.
   for( Vertex* cv = trim->_vertices; cv; cv = cv->_next[0] )
      if( equal( pos, cv->_pos, error ) ) return cv;

   // Vertex non-existant, we need to create it.
   int x = trim->_x;
   int y = trim->_y;

   Vec2f p  = pos( x, y );
   Vec2f v0 = sp._corners[0]->_pos( x, y );
   Vec2f v1 = sp._corners[1]->_pos( x, y );
   Vec2f v2 = sp._corners[2]->_pos( x, y );
   Vec2f v3 = sp._corners[3]->_pos( x, y );

   // Compute distance from each edges.
   float tr0  = triArea( p, v0, v1 );
   float tr1  = triArea( p, v1, v2 );
   float tr2  = triArea( p, v2, v3 );
   float tr3  = triArea( p, v3, v0 );

   float dsq0 = tr0*tr0/sqrLength(v1-v0);
   float dsq1 = tr1*tr1/sqrLength(v2-v1);
   float dsq2 = tr2*tr2/sqrLength(v3-v2);
   float dsq3 = tr3*tr3/sqrLength(v0-v3);

   // Test if pos is on boundary 0 or 2.
   if( dsq0 < dsq2 )
   {
      if( dsq0 < error2 )
      {
         if( dsq1 < error2 ) return sp._corners[1];
         if( dsq3 < error2 ) return sp._corners[0];
         // insert on edge 0.
         float t = dot( p-v0, v1-v0 )/sqrLength(v1-v0);
         return insertVertex( sp, 0, t );
      }
   }
   else if( dsq2 < error2 )
   {
      if( dsq1 < error2 ) return sp._corners[2];
      if( dsq3 < error2 ) return sp._corners[3];
      // insert on edge 2.
      float t = dot( p-v2, v3-v2 )/sqrLength(v3-v2);
      return insertVertex( sp, 2, t );
   }
   // Test if pos is on boundary 1 or 3.
   if( dsq1 < dsq3 )
   {
      if( dsq1 < error2 )
      {
         // insert on edge 1.
         float t = dot( p-v1, v2-v1 )/sqrLength(v2-v1);
         return insertVertex( sp, 1, t );
      }
   }
   else if( dsq3 < error2 )
   {
      // insert on edge 3
      float t = dot( p-v3, v0-v3 )/sqrLength(v0-v3);
      return insertVertex( sp, 3, t );
   }

   Vec2f uv;

   // Compute uv coordinates with barycentric.
   if( trim->_flags == 0 )
   {
      // Compute barycentric of triangle 1: 0,1,2.
      float invd = 1.0f / triArea( v0, v1, v2 );
      float u    = tr1*invd;
      float w    = tr0*invd;
      float v    = 1.0f-u-w;

      if( v >= 0.0f )
      {
         uv = sp._corners[0]->_uv*u + sp._corners[1]->_uv*v + sp._corners[2]->_uv*w;
      }
      else
      {
         // Compute barycentric of triangle 2: 0,2,3.
         invd = 1.0f / triArea( v0, v2, v3 );
         u    = tr2*invd;
         v    = tr3*invd;
         w    = 1.0f-u-v;
         uv   = sp._corners[0]->_uv*u + sp._corners[2]->_uv*v + sp._corners[3]->_uv*w;
      }
   }
   else
   {
      // Compute barycentric of triangle 1: 0,1,3.
      float invd = 1.0f / triArea( v0, v1, v3 );
      float v    = tr3*invd;
      float w    = tr0*invd;
      float u    = 1.0f-v-w;

      if( u >= 0.0f )
      {
         uv = sp._corners[0]->_uv*u + sp._corners[1]->_uv*v + sp._corners[3]->_uv*w;
      }
      else
      {
         // Compute barycentric of triangle 2: 1,2,3.
         invd = 1.0f / triArea( v1, v2, v3 );
         u    = tr2*invd;
         w    = tr1*invd;
         v    = 1.0f-u-w;
         uv   = sp._corners[1]->_uv*u + sp._corners[2]->_uv*v + sp._corners[3]->_uv*w;
      }
   }

   // Create vertex.
   Vertex* nv   = _vPool.alloc();
   nv->_pos     = pos;
   nv->_uv      = uv;
   nv->_flags   = 0x10;
   nv->_id      = INVALID;
   nv->_next[0] = trim->_vertices;
   nv->_next[1] = 0;
   nv->_next[2] = 0;
   nv->_next[3] = 0;

   trim->_vertices = nv;

   return nv;
}

//------------------------------------------------------------------------------
//! 
MetaSurface::Vertex*
MetaSurface::insertVertex( Subpatch& sp, uint edge, const Vec3f& pos, float t )
{
   uint ne   = (edge+1)%4;
   uint axis = edge&1;
   Vec2f uv  = CGM::linear2( sp._corners[edge]->_uv, sp._corners[ne]->_uv, t );

   // Search vertex.
   Vertex* v = 0;
   switch( edge )
   {
      case 0: v = prevVertex( sp._corners[0], sp._corners[1], 0, uv.x ); break;
      case 1: v = prevVertex( sp._corners[1], sp._corners[2], 1, uv.y ); break;
      case 2: v = prevVertex( sp._corners[3], sp._corners[2], 0, uv.x ); break;
      case 3: v = prevVertex( sp._corners[0], sp._corners[3], 1, uv.y ); break;
   }

   // Vertex found?
   if( equal( v->_pos, pos, _errormax ) ) return v;
   if( equal( v->_next[axis+1]->_pos, pos, _errormax ) ) return v->_next[axis+1];

   return insertVertex( sp, edge, t, v, (axis+3)%4, pos, uv );
}

//------------------------------------------------------------------------------
//! 
MetaSurface::Vertex* 
MetaSurface::insertVertex( Subpatch& /*sp*/, Trimming* trim, const Vec3f& pos, const Vec2f& uv )
{
   // Rounding precisions.
   const float error2 = _errormax*_errormax;

   // Find point in trimming info.
   for( Vertex* cv = trim->_vertices; cv; cv = cv->_next[0] )
      if( sqrLength( pos-cv->_pos ) < error2 ) return cv;

   // Vertex non-existant, we need to create it.
   Vertex* nv   = _vPool.alloc();
   nv->_pos     = pos;
   nv->_uv      = uv;
   nv->_flags   = 0x10;
   nv->_id      = INVALID;
   nv->_next[0] = trim->_vertices;
   nv->_next[1] = 0;
   nv->_next[2] = 0;
   nv->_next[3] = 0;

   trim->_vertices = nv;

   return nv;
}


/*==============================================================================
   Trimming
==============================================================================*/

//------------------------------------------------------------------------------
//! 
MetaSurface::Trimming*
MetaSurface::trimming( Subpatch& sp )
{
   CHECK( isTrimmed( sp ) );
   TrimmingData* trimData = _cache->trimmingData();
   Trimming* trim         = trimData->trim( &sp );
   trimData->computeLoops( this, &sp, trim );
   return trim;
}

//------------------------------------------------------------------------------
//! 
MetaSurface::Loop* 
MetaSurface::loops( Subpatch& sp )
{
   CHECK( isTrimmed( sp ) );
   TrimmingData* trimData = _cache->trimmingData();
   Trimming* trim         = trimData->trim( &sp );
   trimData->computeLoops( this, &sp, trim );
   return trim->_loops;
}

//------------------------------------------------------------------------------
//! 
void 
MetaSurface::trim( Patch& pA, Patch& pB )
{
   // Test intersecting subpatches.
   if( pA._tree )
   {
      if( pB._tree )
      {
         // Tree-Tree intersection.
         pA._tree->findCollisions( this, *pB._tree, subpatchesOverlap );
      }
      else
      {
         // Tree-box intersecgion.
         AABBoxf box = computeAABB( pB._subpatch );
         pA._tree->findCollisions( this, &pB._subpatch, box, subpatchesOverlap );
      }
   }
   else
   {
      if( pB._tree )
      {
         // Box-Tree intersection.
         AABBoxf box = computeAABB( pA._subpatch );
         pB._tree->findCollisions( this, &pA._subpatch, box, subpatchesOverlap );
      }
      else
      {
         // Subpatch-subpatch intersection.
         subpatchesOverlap( this, &pA._subpatch, &pB._subpatch );
      }
   }
}

//------------------------------------------------------------------------------
//!
void
MetaSurface::trim( Subpatch& spA, Subpatch& spB )
{
   // Rounding precisions.
   const float error2 = _errormax*_errormax;
   
   // Retrieve intersection info for subpatch A & B.
   TrimmingData* trimData = _cache->trimmingData();
   Trimming* trimA = trimData->trim( &spA );
   Trimming* trimB = trimData->trim( &spB );

   Planef planeA0 = trimA->_plane0;
   Planef planeA1 = trimA->_plane1;
   Planef planeB0 = trimB->_plane0;
   Planef planeB1 = trimB->_plane1;

   // Compute distance from plane 0 of subpatch A.
   // B0-A0 && B1-A0.
   float db0[4];
   db0[0] = planeA0.evaluate( spB._corners[0]->_pos );
   db0[1] = planeA0.evaluate( spB._corners[1]->_pos );
   db0[2] = planeA0.evaluate( spB._corners[2]->_pos );
   db0[3] = planeA0.evaluate( spB._corners[3]->_pos );

   // Coplanarity robustness snapping.
   if( db0[0]*db0[0] < error2 ) db0[0] = 0.0f;
   if( db0[1]*db0[1] < error2 ) db0[1] = 0.0f;
   if( db0[2]*db0[2] < error2 ) db0[2] = 0.0f;
   if( db0[3]*db0[3] < error2 ) db0[3] = 0.0f;

   // Compute distance from plane 1 of subpatch A.
   // B0-A1 && B1-A1
   float db1[4];
   db1[0] = planeA1.evaluate( spB._corners[0]->_pos );
   db1[1] = planeA1.evaluate( spB._corners[1]->_pos );
   db1[2] = planeA1.evaluate( spB._corners[2]->_pos );
   db1[3] = planeA1.evaluate( spB._corners[3]->_pos );

   // Coplanarity robustness snapping.
   if( db1[0]*db1[0] < error2 ) db1[0] = 0.0f;
   if( db1[1]*db1[1] < error2 ) db1[1] = 0.0f;
   if( db1[2]*db1[2] < error2 ) db1[2] = 0.0f;
   if( db1[3]*db1[3] < error2 ) db1[3] = 0.0f;

   // Compute distance from plane 0 of subpatch B.
   // A0-B0 && A1-B0.
   float da0[4];
   da0[0] = planeB0.evaluate( spA._corners[0]->_pos );
   da0[1] = planeB0.evaluate( spA._corners[1]->_pos );
   da0[2] = planeB0.evaluate( spA._corners[2]->_pos );
   da0[3] = planeB0.evaluate( spA._corners[3]->_pos );

   // Coplanarity robustness snapping.
   if( da0[0]*da0[0] < error2 ) da0[0] = 0.0f;
   if( da0[1]*da0[1] < error2 ) da0[1] = 0.0f;
   if( da0[2]*da0[2] < error2 ) da0[2] = 0.0f;
   if( da0[3]*da0[3] < error2 ) da0[3] = 0.0f;

   // Compute distance from plane 1 of subpatch B.
   // A0-B1 && A1-B1.
   float da1[4];
   da1[0] = planeB1.evaluate( spA._corners[0]->_pos );
   da1[1] = planeB1.evaluate( spA._corners[1]->_pos );
   da1[2] = planeB1.evaluate( spA._corners[2]->_pos );
   da1[3] = planeB1.evaluate( spA._corners[3]->_pos );

   // Coplanarity robustness snapping.
   if( da1[0]*da1[0] < error2 ) da1[0] = 0.0f;
   if( da1[1]*da1[1] < error2 ) da1[1] = 0.0f;
   if( da1[2]*da1[2] < error2 ) da1[2] = 0.0f;
   if( da1[3]*da1[3] < error2 ) da1[3] = 0.0f;

   // Computing triangles indices.
   int idx[] = {0,1,2,2,3,0,3,0,1,1,2,3};
   int* a0 = idx + trimA->_flags*6;
   int* a1 = idx + trimA->_flags*6 + 3;
   int* b0 = idx + trimB->_flags*6;
   int* b1 = idx + trimB->_flags*6 + 3;

   // Intersection for triangle A0-B0
   trim( spA, spB, *trimA, *trimB, planeA0, planeB0, da0, db0, a0, b0 );
   // Intersection for triangle A0-B1
   trim( spA, spB, *trimA, *trimB, planeA0, planeB1, da1, db0, a0, b1 );
   // Intersection for triangle A1-B0
   trim( spA, spB, *trimA, *trimB, planeA1, planeB0, da0, db1, a1, b0 );
   // Intersection for triangle A1-B1
   trim( spA, spB, *trimA, *trimB, planeA1, planeB1, da1, db1, a1, b1 );
}

//------------------------------------------------------------------------------
//! 
bool closestPtSegment( const Vec3f& pt, const Vec3f& v0, const Vec3f& v1, float error, Vec3f& r, float& t )
{
   Vec3d e10  = v1-v0;
   Vec3d ptv0 = pt-v0;
   double e   = dot( ptv0, e10 );
   if( e <= 0.0 ) return false;
   double f   = dot( e10, e10 );
   if( e >= f ) return false;
   double td  = e / f;
   double d   = dot( ptv0, ptv0 ) - e*td;
   if( d > error ) return false;
   //if( dot( ptv0, ptv0 ) - e*t > error ) return false;
   r = v0 + td*e10;
   t = float(td);
   return true;
}


//------------------------------------------------------------------------------
//! 
bool closestPtSegmentSegment( 
   const Vec3f& v0, const Vec3f& v1, 
   const Vec3f& v2, const Vec3f& v3, 
   float error, 
   Vec3f& r1, float& s,
   Vec3f& r2, float& t
)
{
   Vec3d e10 = v1-v0;
   Vec3d e32 = v3-v2;
   Vec3d r   = v0-v2;

   double a = dot( e10, e10 );
   double b = dot( e10, e32 );
   double c = dot( e10, r );
   double e = dot( e32, e32 );
   double f = dot( e32, r );

   double denom = a*e-b*b;

   double sd = (b*f-c*e)/denom;

   if( sd < 0.0 || sd > 1.0 ) return false;

   double td = (b*sd+f)/e;

   if( td < 0.0 || td > 1.0 ) return false;

   r1 = v0 + e10*sd;
   r2 = v2 + e32*td;
   t  = float(td);
   s  = float(sd);

   if( sqrLength( r2-r1 ) > error ) return false;

   return true;
}

//------------------------------------------------------------------------------
//! 
bool closestPtTriangle( 
   const Vec3f& pt, 
   const Vec3f& v0, 
   const Vec3f& v1, 
   const Vec3f& v2,
   Vec3f& r,
   Vec3f& bary
)
{
   Vec3d v10  = v1-v0;
   Vec3d v20  = v2-v0;
   Vec3d ptv0 = pt-v0;
   Vec3d ptv1 = pt-v1;

   double d1  = dot( v10, ptv0 );
   double d2  = dot( v20, ptv0 );
   double d3  = dot( v10, ptv1 );
   double d4  = dot( v20, ptv1 );
   double vc  = d1*d4 - d3*d2;

   if( vc <= 0.0 ) return false;

   Vec3d ptv2 = pt-v2;
   double d5  = dot( v10, ptv2 );
   double d6  = dot( v20, ptv2 );
   double vb  = d5*d2 - d1*d6;

   if( vb <= 0.0 ) return false;

   double va  = d3*d6 - d5*d4;

   if( va <= 0.0 ) return false;

   // Pt is inside.
   double denum = 1.0 / (va+vb+vc);
   bary.y = float(vb*denum);
   bary.z = float(vc*denum);
   bary.x = 1.0f-bary.y-bary.z;
   r      = v0 + v10*bary.y + v20*bary.z;
   return true;
}

//------------------------------------------------------------------------------
//! 
void 
MetaSurface::trim(
   Subpatch& spA, 
   Subpatch& spB, 
   Trimming& trimA, 
   Trimming& trimB, 
   Planef&   /*planeA*/, 
   Planef&   /*planeB*/, 
   float*    da,
   float*    db, 
   int*      idxa, 
   int*      idxb
)
{
   // FIXME: There is still some "unsolved" illegal cases when computing
   // edge/edge intersection. Those could lead to find 2 or 3 feature points
   // per triangle. An edge should only be allowed to intersects twice if it
   // is contained in the plane of the other triangle (both its and vertices
   // distance equal 0).
   // For example, the edge/edge validation test could be replace as follow:
   // 1. compute the number of intersection on the other edge. Vertex count.
   // 2. Test for edge/edge intersection only if there is no intersection or
   // if the other edge is in the triangle plane.

   /*
   static int numTriTri = 0;
   ++numTriTri;
   if( numTriTri == 5877 )
   {
      StdErr << "BAD TriTri\n";
   }
   */

   // Rounding precisions.
   const float errorp = _errormax*_errormax;
   const float errore = 2.0f*errorp;
   const float errorv = 4.0f*errorp;
   const float erroree = 8.0f*errorp;

   float da0    = da[idxa[0]];
   float da1    = da[idxa[1]];
   float da2    = da[idxa[2]];
   float da0da1 = da0*da1;
   float da0da2 = da0*da2;

   // Same side and not coplanar?
   if( da0da1 > 0.0f && da0da2 > 0.0f ) return;
   // Coplanar?
   if( da0 == 0.0f && da1 == 0.0f && da2 == 0.0f ) return;

   float db0    = db[idxb[0]];
   float db1    = db[idxb[1]];
   float db2    = db[idxb[2]];
   float db0db1 = db0*db1;
   float db0db2 = db0*db2;

   // Same side and not coplanar?
   if( db0db1 > 0.0f && db0db2 > 0.0f ) return;
   // Coplanar?
   if( db0 == 0.0f && db1 == 0.0f && db2 == 0.0f ) return;

   Vec3f va0 = spA._corners[idxa[0]]->_pos;
   Vec3f va1 = spA._corners[idxa[1]]->_pos;
   Vec3f va2 = spA._corners[idxa[2]]->_pos;
   Vec3f vb0 = spB._corners[idxb[0]]->_pos;
   Vec3f vb1 = spB._corners[idxb[1]]->_pos;
   Vec3f vb2 = spB._corners[idxb[2]]->_pos;

   Vec2f uva0 = spA._corners[idxa[0]]->_uv;
   Vec2f uva1 = spA._corners[idxa[1]]->_uv;
   Vec2f uva2 = spA._corners[idxa[2]]->_uv;
   Vec2f uvb0 = spB._corners[idxb[0]]->_uv;
   Vec2f uvb1 = spB._corners[idxb[1]]->_uv;
   Vec2f uvb2 = spB._corners[idxb[2]]->_uv;

   // Test all vertices states.
   // Important: an edge could intersect 2 vertices...
   int vaStates[3] = {-1,-1,-1};
   int vbStates[3] = {-1,-1,-1};
   int eaStates[3] = {-1,-1,-1};
   int ebStates[3] = {-1,-1,-1};

   Vertex* va[4];
   Vertex* vb[4];
   int amasks[4];
   int bmasks[4];
   int numVA = 0;
   int numVB = 0;

   float s, t;
   Vec3f pt0;
   Vec3f pt1;
   Vec3f bary;

   // 1. Vertex-Vertex
   if( da0 == 0.0f )
   {
      if( (db0 == 0.0f) && (sqrLength(va0-vb0) < errorv) )
      {
         amasks[numVA] = 0x5; bmasks[numVB] = 0x5;
         vaStates[0] = 0;     vbStates[0] = 0;
         va[numVA++] = spA._corners[idxa[0]];
         vb[numVB++] = spB._corners[idxb[0]];
      }
      else
      if( (db1 == 0.0f) && (sqrLength(va0-vb1) < errorv) )
      {
         amasks[numVA] = 0x5; bmasks[numVB] = 0x3;
         vaStates[0] = 1;     vbStates[1] = 0;
         va[numVA++] = spA._corners[idxa[0]];
         vb[numVB++] = spB._corners[idxb[1]];
      }
      else
      if( (db2 == 0.0f) && (sqrLength(va0-vb2) < errorv) )
      {
         amasks[numVA] = 0x5; bmasks[numVB] = 0x6;
         vaStates[0] = 2;     vbStates[2] = 0;
         va[numVA++] = spA._corners[idxa[0]];
         vb[numVB++] = spB._corners[idxb[2]];
      }
      else
      // Vertex-edge.
      {
         if( closestPtSegment( va0, vb0, vb1, errore, pt0, t ) )
         {
            amasks[numVA] = 0x5; bmasks[numVB] = 0x1;
            vaStates[0] = 3;     ebStates[0] = 0;
            va[numVA++] = spA._corners[idxa[0]];
            vb[numVB++] = insertVertex( spB, idxb[0], pt0, t );
         }
         else if( closestPtSegment( va0, vb1, vb2, errore, pt0, t ) )
         {
            amasks[numVA] = 0x5; bmasks[numVB] = 0x2;
            vaStates[0] = 4;     ebStates[1] = 0;
            va[numVA++] = spA._corners[idxa[0]];
            vb[numVB++] = insertVertex( spB, idxb[1], pt0, t );
         }
         else if( closestPtSegment( va0, vb2, vb0, errore, pt0, t ) )
         {
            amasks[numVA] = 0x5; bmasks[numVB] = 0x4;
            vaStates[0] = 5;     ebStates[2] = 0;
            va[numVA++] = spA._corners[idxa[0]];
            // vb2-vb0 is ot really an edge.
            vb[numVB++] = insertVertex( spB, &trimB, pt0, CGM::linear2( uvb2, uvb0, t ) );
         }
         // Vertex-triangle.
         else if( closestPtTriangle( va0, vb0, vb1, vb2, pt0, bary ) )
         {
            amasks[numVA] = 0x5; bmasks[numVB] = 0x8;
            vaStates[0] = 6;
            va[numVA++] = spA._corners[idxa[0]];
            vb[numVB++] = insertVertex( spB, &trimB, pt0, uvb0*bary.x + uvb1*bary.y + uvb2*bary.z );
         }
      }
   }
   if( da1 == 0.0f )
   {
      if( (db0 == 0.0f) && (sqrLength(va1-vb0) < errorv) )
      {
         amasks[numVA] = 0x3; bmasks[numVB] = 0x5;
         vaStates[1] = 0;     vbStates[0] = 1;
         va[numVA++] = spA._corners[idxa[1]];
         vb[numVB++] = spB._corners[idxb[0]];
      }
      else
      if( (db1 == 0.0f) && (sqrLength(va1-vb1) < errorv) )
      {
         amasks[numVA] = 0x3; bmasks[numVB] = 0x3;
         vaStates[1] = 1;     vbStates[1] = 1;
         va[numVA++] = spA._corners[idxa[1]];
         vb[numVB++] = spB._corners[idxb[1]];
      }
      else
      if( (db2 == 0.0f) && (sqrLength(va1-vb2) < errorv) )
      {
         amasks[numVA] = 0x3; bmasks[numVB] = 0x6;
         vaStates[1] = 2;     vbStates[2] = 1;
         va[numVA++] = spA._corners[idxa[1]];
         vb[numVB++] = spB._corners[idxb[2]];
      }
      else
      // Vertex-edge.
      {
         if( closestPtSegment( va1, vb0, vb1, errore, pt0, t ) )
         {
            amasks[numVA] = 0x3; bmasks[numVB] = 0x1;
            vaStates[1] = 3;      ebStates[0] = 1;
            va[numVA++] = spA._corners[idxa[1]];
            vb[numVB++] = insertVertex( spB, idxb[0], pt0, t );
         }
         else if( closestPtSegment( va1, vb1, vb2, errore, pt0, t ) )
         {
            amasks[numVA] = 0x3; bmasks[numVB] = 0x2;
            vaStates[1] = 4;     ebStates[1] = 1;
            va[numVA++] = spA._corners[idxa[1]];
            vb[numVB++] = insertVertex( spB, idxb[1], pt0, t );
         }
         else if( closestPtSegment( va1, vb2, vb0, errore, pt0, t ) )
         {
            amasks[numVA] = 0x3; bmasks[numVB] = 0x4;
            vaStates[1] = 5;     ebStates[2] = 1;
            va[numVA++] = spA._corners[idxa[1]];
            // vb2-vb0 is ot really an edge.
            vb[numVB++] = insertVertex( spB, &trimB, pt0, CGM::linear2( uvb2, uvb0, t ) );
         }
         // Vertex-triangle.
         else if( closestPtTriangle( va1, vb0, vb1, vb2, pt0, bary ) )
         {
            amasks[numVA] = 0x3; bmasks[numVB] = 0x8;
            vaStates[1] = 6;
            va[numVA++] = spA._corners[idxa[1]];
            vb[numVB++] = insertVertex( spB, &trimB, pt0, uvb0*bary.x + uvb1*bary.y + uvb2*bary.z );
         }
      }
   }
   if( da2 == 0.0f )
   {
      if( (db0 == 0.0f) && (sqrLength(va2-vb0) < errorv) )
      {
         amasks[numVA] = 0x6; bmasks[numVB] = 0x5;
         vaStates[2] = 0;     vbStates[0] = 2;
         va[numVA++] = spA._corners[idxa[2]];
         vb[numVB++] = spB._corners[idxb[0]];
      }
      else
      if( (db1 == 0.0f) && (sqrLength(va2-vb1) < errorv) )
      {
         amasks[numVA] = 0x6; bmasks[numVB] = 0x3;
         vaStates[2] = 1;     vbStates[1] = 2;
         va[numVA++] = spA._corners[idxa[2]];
         vb[numVB++] = spB._corners[idxb[1]];
      }
      else
      if( (db2 == 0.0f) && (sqrLength(va2-vb2) < errorv) )
      {
         amasks[numVA] = 0x6; bmasks[numVB] = 0x6;
         vaStates[2] = 2;     vbStates[2] = 2;
         va[numVA++] = spA._corners[idxa[2]];
         vb[numVB++] = spB._corners[idxb[2]];
      }
      else
      // Vertex-edge.
      {
         if( closestPtSegment( va2, vb0, vb1, errore, pt0, t ) )
         {
            amasks[numVA] = 0x6; bmasks[numVB] = 0x1;
            vaStates[2] = 3;     ebStates[0] = 2;
            va[numVA++] = spA._corners[idxa[2]];
            vb[numVB++] = insertVertex( spB, idxb[0], pt0, t );
         }
         else if( closestPtSegment( va2, vb1, vb2, errore, pt0, t ) )
         {
            amasks[numVA] = 0x6; bmasks[numVB] = 0x2;
            vaStates[2] = 4;     ebStates[1] = 2;
            va[numVA++] = spA._corners[idxa[2]];
            vb[numVB++] = insertVertex( spB, idxb[1], pt0, t );
         }
         else if( closestPtSegment( va2, vb2, vb0, errore, pt0, t ) )
         {
            amasks[numVA] = 0x6; bmasks[numVB] = 0x4;
            vaStates[2] = 5;     ebStates[2] = 2;
            va[numVA++] = spA._corners[idxa[2]];
            // vb2-vb0 is ot really an edge.
            vb[numVB++] = insertVertex( spB, &trimB, pt0, CGM::linear2( uvb2, uvb0, t ) );
         }
         // Vertex-triangle.
         else if( closestPtTriangle( va2, vb0, vb1, vb2, pt0, bary ) )
         {
            amasks[numVA] = 0x6; bmasks[numVB] = 0x8;
            vaStates[2] = 6;
            va[numVA++] = spA._corners[idxa[2]];
            vb[numVB++] = insertVertex( spB, &trimB, pt0, uvb0*bary.x + uvb1*bary.y + uvb2*bary.z );
         }
      }
   }
   // Vertex-edge.
   if( (db0 == 0.0f) && (vbStates[0] == -1) )
   {
      if( closestPtSegment( vb0, va0, va1, errore, pt0, t ) )
      {
         bmasks[numVB] = 0x5; amasks[numVA] = 0x1;
         vbStates[0] = 3;     eaStates[0] = 0;
         vb[numVB++] = spB._corners[idxb[0]];
         va[numVA++] = insertVertex( spA, idxa[0], pt0, t );
      }
      else if( closestPtSegment( vb0, va1, va2, errore, pt0, t ) )
      {
         bmasks[numVB] = 0x5; amasks[numVA] = 0x2;
         vbStates[0] = 4;     eaStates[1] = 0;
         vb[numVB++] = spB._corners[idxb[0]];
         va[numVA++] = insertVertex( spA, idxa[1], pt0, t );
      }
      else if( closestPtSegment( vb0, va2, va0, errore, pt0, t ) )
      {
         bmasks[numVB] = 0x5; amasks[numVA] = 0x4;
         vbStates[0] = 5;     eaStates[2] = 0;
         vb[numVB++] = spB._corners[idxb[0]];
         // va2-va0 is ot really an edge.
         va[numVA++] = insertVertex( spA, &trimA, pt0, CGM::linear2( uva2, uva0, t ) );
      }
      // Vertex-triangle.
      else if( closestPtTriangle( vb0, va0, va1, va2, pt0, bary ) )
      {
         bmasks[numVB] = 0x5; amasks[numVA] = 0x8;
         vbStates[0] = 6;
         vb[numVB++] = spB._corners[idxb[0]];
         va[numVA++] = insertVertex( spA, &trimA, pt0, uva0*bary.x + uva1*bary.y + uva2*bary.z );
      }
   }
   if( (db1 == 0.0f) && (vbStates[1] == -1) )
   {
      if( closestPtSegment( vb1, va0, va1, errore, pt0, t ) )
      {
         bmasks[numVB] = 0x3; amasks[numVA] = 0x1;
         vbStates[1] = 3;     eaStates[0] = 1;
         vb[numVB++] = spB._corners[idxb[1]];
         va[numVA++] = insertVertex( spA, idxa[0], pt0, t );
      }
      else if( closestPtSegment( vb1, va1, va2, errore, pt0, t ) )
      {
         bmasks[numVB] = 0x3; amasks[numVA] = 0x2;
         vbStates[1] = 4;     eaStates[1] = 1;
         vb[numVB++] = spB._corners[idxb[1]];
         va[numVA++] = insertVertex( spA, idxa[1], pt0, t );
      }
      else if( closestPtSegment( vb1, va2, va0, errore, pt0, t ) )
      {
         bmasks[numVB] = 0x3; amasks[numVA] = 0x4;
         vbStates[1] = 5;     eaStates[2] = 1;
         vb[numVB++] = spB._corners[idxb[1]];
         // va2-va0 is ot really an edge.
         va[numVA++] = insertVertex( spA, &trimA, pt0, CGM::linear2( uva2, uva0, t ) );
      }
      // Vertex-triangle.
      else if( closestPtTriangle( vb1, va0, va1, va2, pt0, bary ) )
      {
         bmasks[numVB] = 0x3; amasks[numVA] = 0x8;
         vbStates[1] = 6;
         vb[numVB++] = spB._corners[idxb[1]];
         va[numVA++] = insertVertex( spA, &trimA, pt0, uva0*bary.x + uva1*bary.y + uva2*bary.z );
      }
   }
   if( (db2 == 0.0f) && (vbStates[2] == -1) )
   {
      if( closestPtSegment( vb2, va0, va1, errore, pt0, t ) )
      {
         bmasks[numVB] = 0x6; amasks[numVA] = 0x1;
         vbStates[2] = 3;     eaStates[0] = 2;
         vb[numVB++] = spB._corners[idxb[2]];
         va[numVA++] = insertVertex( spA, idxa[0], pt0, t );
      }
      else if( closestPtSegment( vb2, va1, va2, errore, pt0, t ) )
      {
         bmasks[numVB] = 0x6; amasks[numVA] = 0x2;
         vbStates[2] = 4;     eaStates[1] = 2;
         vb[numVB++] = spB._corners[idxb[2]];
         va[numVA++] = insertVertex( spA, idxa[1], pt0, t );
      }
      else if( closestPtSegment( vb2, va2, va0, errore, pt0, t ) )
      {
         bmasks[numVB] = 0x6; amasks[numVA] = 0x4;
         vbStates[2] = 5;     eaStates[2] = 2;
         vb[numVB++] = spB._corners[idxb[2]];
         // va2-va0 is ot really an edge.
         va[numVA++] = insertVertex( spA, &trimA, pt0, CGM::linear2( uva2, uva0, t ) );
      }
      // Vertex-triangle.
      else if( closestPtTriangle( vb2, va0, va1, va2, pt0, bary ) )
      {
         bmasks[numVB] = 0x6; amasks[numVA] = 0x8;
         vbStates[2] = 6;
         vb[numVB++] = spB._corners[idxb[2]];
         va[numVA++] = insertVertex( spA, &trimA, pt0, uva0*bary.x + uva1*bary.y + uva2*bary.z );
      }
   }

   float da1da2 = da1*da2;
   float db1db2 = db1*db2;

   // 2. Test all edges of triangle A against triangle B.
   // Testing edge a0.
   if( (da0da1 < 0.0f) && (eaStates[0] == -1) )
   {
      // Edge-edge.
      if( closestPtSegmentSegment( va0, va1, vb0, vb1, errorp, pt0, s, pt1, t ) )
      {
         if( (vbStates[0] == -1 || sqrLength(pt1-vb0) > erroree) && (vbStates[1] == -1 || sqrLength(pt1-vb1) > erroree) )
         {
            amasks[numVA] = 0x1; bmasks[numVB] = 0x1;
            eaStates[0] = 3;     ebStates[0] = 3;
            va[numVA++] = insertVertex( spA, idxa[0], pt0, s );
            vb[numVB++] = insertVertex( spB, idxb[0], pt1, t );
         }
      }
      else if( closestPtSegmentSegment( va0, va1, vb1, vb2, errorp, pt0, s, pt1, t ) )
      {
         if( (vbStates[1] == -1 || sqrLength(pt1-vb1) > erroree) && (vbStates[2] == -1 || sqrLength(pt1-vb2) > erroree) )
         {
            amasks[numVA] = 0x1; bmasks[numVB] = 0x2;
            eaStates[0] = 4;     ebStates[1] = 3;
            va[numVA++] = insertVertex( spA, idxa[0], pt0, s );
            vb[numVB++] = insertVertex( spB, idxb[1], pt1, t );
         }
      }
      else if( closestPtSegmentSegment( va0, va1, vb2, vb0, errorp, pt0, s, pt1, t ) )
      {
         if( (vbStates[2] == -1 || sqrLength(pt1-vb2) > erroree) && (vbStates[0] == -1 || sqrLength(pt1-vb0) > erroree) )
         {
            amasks[numVA] = 0x1; bmasks[numVB] = 0x4;
            eaStates[0] = 5;     ebStates[2] = 3;
            va[numVA++] = insertVertex( spA, idxa[0], pt0, s );
            vb[numVB++] = insertVertex( spB, &trimB, pt1, CGM::linear2( uvb2, uvb0, t ) );
         }
      }
      // Edge-triangle.
      else
      {
         t   = da0/(da0-da1);
         pt0 = va0+(va1-va0)*t;
         if( closestPtTriangle( pt0, vb0, vb1, vb2, pt1, bary ) )
         {
            amasks[numVA] = 0x1; bmasks[numVB] = 0x8;
            eaStates[0] = 6;
            va[numVA++] = insertVertex( spA, idxa[0], pt0, t );
            vb[numVB++] = insertVertex( spB, &trimB, pt1, uvb0*bary.x + uvb1*bary.y + uvb2*bary.z );
         }
      }
   }

   // Testing edge a1.
   if( (da1da2 < 0.0f) && (eaStates[1] == -1) )
   {
      // Edge-edge.
      if( closestPtSegmentSegment( va1, va2, vb0, vb1, errorp, pt0, s, pt1, t ) )
      {
         if( (vbStates[0] == -1 || sqrLength(pt1-vb0) > erroree) && (vbStates[1] == -1 || sqrLength(pt1-vb1) > erroree) )
         {
            amasks[numVA] = 0x2; bmasks[numVB] = 0x1;
            eaStates[1] = 3;     ebStates[0] = 4;
            va[numVA++] = insertVertex( spA, idxa[1], pt0, s );
            vb[numVB++] = insertVertex( spB, idxb[0], pt1, t );
         }
      }
      else if( closestPtSegmentSegment( va1, va2, vb1, vb2, errorp, pt0, s, pt1, t ) )
      {
         if( (vbStates[1] == -1 || sqrLength(pt1-vb1) > erroree) && (vbStates[2] == -1 || sqrLength(pt1-vb2) > erroree) )
         {
            amasks[numVA] = 0x2; bmasks[numVB] = 0x2;
            eaStates[1] = 4;     ebStates[1] = 4;
            va[numVA++] = insertVertex( spA, idxa[1], pt0, s );
            vb[numVB++] = insertVertex( spB, idxb[1], pt1, t );
         }
      }
      else if( closestPtSegmentSegment( va1, va2, vb2, vb0, errorp, pt0, s, pt1, t ) )
      {
         if( (vbStates[2] == -1 || sqrLength(pt1-vb2) > erroree) && (vbStates[0] == -1 || sqrLength(pt1-vb0) > erroree) )
         {
            amasks[numVA] = 0x2; bmasks[numVB] = 0x4;
            eaStates[1] = 5;     ebStates[2] = 4;
            va[numVA++] = insertVertex( spA, idxa[1], pt0, s );
            vb[numVB++] = insertVertex( spB, &trimB, pt1, CGM::linear2( uvb2, uvb0, t ) );
         }
      }
      // Edge-triangle.
      else
      {
         t   = da1/(da1-da2);
         pt0 = va1+(va2-va1)*t;
         if( closestPtTriangle( pt0, vb0, vb1, vb2, pt1, bary ) )
         {
            amasks[numVA] = 0x2; bmasks[numVB] = 0x8;
            eaStates[1] = 6;
            va[numVA++] = insertVertex( spA, idxa[1], pt0, t );
            vb[numVB++] = insertVertex( spB, &trimB, pt1, uvb0*bary.x + uvb1*bary.y + uvb2*bary.z );
         }
      }
   }

   // Testing edge a2.
   if( (da0da2 < 0.0f) && (eaStates[2] == -1) )
   {
      // Edge-edge.
      if( closestPtSegmentSegment( va2, va0, vb0, vb1, errorp, pt0, s, pt1, t ) )
      {
         if( (vbStates[0] == -1 || sqrLength(pt1-vb0) > erroree) && (vbStates[1] == -1 || sqrLength(pt1-vb1) > erroree) )
         {
            amasks[numVA] = 0x4; bmasks[numVB] = 0x1;
            eaStates[2] = 3;     ebStates[0] = 5;
            va[numVA++] = insertVertex( spA, &trimA, pt0, CGM::linear2( uva2, uva0, s ) );
            vb[numVB++] = insertVertex( spB, idxb[0], pt1, t );
         }
      }
      else if( closestPtSegmentSegment( va2, va0, vb1, vb2, errorp, pt0, s, pt1, t ) )
      {
         if( (vbStates[1] == -1 || sqrLength(pt1-vb1) > erroree) && (vbStates[2] == -1 || sqrLength(pt1-vb2) > erroree) )
         {
            amasks[numVA] = 0x4; bmasks[numVB] = 0x2;
            eaStates[2] = 4;     ebStates[1] = 5;
            va[numVA++] = insertVertex( spA, &trimA, pt0, CGM::linear2( uva2, uva0, s ) );
            vb[numVB++] = insertVertex( spB, idxb[1], pt1, t );
         }
      }
      else if( closestPtSegmentSegment( va2, va0, vb2, vb0, errorp, pt0, s, pt1, t ) )
      {
         if( (vbStates[2] == -1 || sqrLength(pt1-vb2) > erroree) && (vbStates[0] == -1 || sqrLength(pt1-vb0) > erroree) )
         {
            amasks[numVA] = 0x4; bmasks[numVB] = 0x4;
            eaStates[2] = 5;     ebStates[2] = 5;
            va[numVA++] = insertVertex( spA, &trimA, pt0, CGM::linear2( uva2, uva0, s ) );
            vb[numVB++] = insertVertex( spB, &trimB, pt1, CGM::linear2( uvb2, uvb0, t ) );
         }
      }
      // Edge-triangle.
      else
      {
         t   = da2/(da2-da0);
         pt0 = va2+(va0-va2)*t;
         if( closestPtTriangle( pt0, vb0, vb1, vb2, pt1, bary ) )
         {
            amasks[numVA] = 0x4; bmasks[numVB] = 0x8;
            eaStates[2] = 6;
            va[numVA++] = insertVertex( spA, &trimA, pt0, CGM::linear2( uva2, uva0, t ) );
            vb[numVB++] = insertVertex( spB, &trimB, pt1, uvb0*bary.x + uvb1*bary.y + uvb2*bary.z );
         }
      }
   }

   // 3. Test all edges of triangle B against triangle A.
   // Testing edge b0.
   if( (db0db1 < 0.0f) && (ebStates[0] == -1) )
   {
      // Edge-edge.
      if( closestPtSegmentSegment( vb0, vb1, va0, va1, errorp, pt0, s, pt1, t ) )
      {
         if( (vaStates[0] == -1 || sqrLength(pt1-va0) > erroree) && (vaStates[1] == -1 || sqrLength(pt1-va1) > erroree) )
         {
            bmasks[numVB] = 0x1; amasks[numVA] = 0x1;
            ebStates[0] = 3;     eaStates[0] = 3;
            vb[numVB++] = insertVertex( spB, idxb[0], pt0, s );
            va[numVA++] = insertVertex( spA, idxa[0], pt1, t );
         }
      }
      else if( closestPtSegmentSegment( vb0, vb1, va1, va2, errorp, pt0, s, pt1, t ) )
      {
         if( (vaStates[1] == -1 || sqrLength(pt0-va1) > erroree) && (vaStates[2] == -1 || sqrLength(pt1-va2) > erroree) )
         {
            bmasks[numVB] = 0x1; amasks[numVA] = 0x2;
            ebStates[0] = 4;     eaStates[1] = 3;
            vb[numVB++] = insertVertex( spB, idxb[0], pt0, s );
            va[numVA++] = insertVertex( spA, idxa[1], pt1, t );
         }
      }
      else if( closestPtSegmentSegment( vb0, vb1, va2, va0, errorp, pt0, s, pt1, t ) )
      {
         if( (vaStates[2] == -1 || sqrLength(pt1-va2) > erroree) && (vaStates[0] == -1 || sqrLength(pt1-va0) > erroree) )
         {
            bmasks[numVB] = 0x1; amasks[numVA] = 0x4;
            ebStates[0] = 5;     eaStates[2] = 3;
            vb[numVB++] = insertVertex( spB, idxb[0], pt0, s );
            va[numVA++] = insertVertex( spA, &trimA, pt1, CGM::linear2( uva2, uva0, t ) );
         }
      }
      // Edge-triangle.
      else
      {
         t   = db0/(db0-db1);
         pt0 = vb0+(vb1-vb0)*t;
         if( closestPtTriangle( pt0, va0, va1, va2, pt1, bary ) )
         {
            bmasks[numVB] = 0x1; amasks[numVA] = 0x8;
            ebStates[0] = 6;
            vb[numVB++] = insertVertex( spB, idxb[0], pt0, t );
            va[numVA++] = insertVertex( spA, &trimA, pt1, uva0*bary.x + uva1*bary.y + uva2*bary.z );
         }
      }
   }

   // Testing edge b1.
   if( (db1db2 < 0.0f) && (ebStates[1] == -1) )
   {
      // Edge-edge.
      if( closestPtSegmentSegment( vb1, vb2, va0, va1, errorp, pt0, s, pt1, t ) )
      {
         if( (vaStates[0] == -1 || sqrLength(pt1-va0) > erroree) && (vaStates[1] == -1 || sqrLength(pt1-va1) > erroree) )
         {
            bmasks[numVB] = 0x2; amasks[numVA] = 0x1;
            ebStates[1] = 3;     eaStates[0] = 4;
            vb[numVB++] = insertVertex( spB, idxb[1], pt0, s );
            va[numVA++] = insertVertex( spA, idxa[0], pt1, t );
         }
      }
      else if( closestPtSegmentSegment( vb1, vb2, va1, va2, errorp, pt0, s, pt1, t ) )
      {
         if( (vaStates[1] == -1 || sqrLength(pt1-va1) > erroree) && (vaStates[2] == -1 || sqrLength(pt1-va2) > erroree) )
         {
            bmasks[numVB] = 0x2; amasks[numVA] = 0x2;
            ebStates[1] = 4;     eaStates[1] = 4;
            vb[numVB++] = insertVertex( spB, idxb[1], pt0, s );
            va[numVA++] = insertVertex( spA, idxa[1], pt1, t );
         }
      }
      else if( closestPtSegmentSegment( vb1, vb2, va2, va0, errorp, pt0, s, pt1, t ) )
      {
         if( (vaStates[2] == -1 || sqrLength(pt1-va2) > erroree) && (vaStates[0] == -1 || sqrLength(pt1-va0) > erroree) )
         {
            bmasks[numVB] = 0x2; amasks[numVA] = 0x4;
            ebStates[1] = 5;     eaStates[2] = 4;
            vb[numVB++] = insertVertex( spB, idxb[1], pt0, s );
            va[numVA++] = insertVertex( spA, &trimA, pt1, CGM::linear2( uva2, uva0, t ) );
         }
      }
      // Edge-triangle.
      else
      {
         t   = db1/(db1-db2);
         pt0 = vb1+(vb2-vb1)*t;
         if( closestPtTriangle( pt0, va0, va1, va2, pt1, bary ) )
         {
            bmasks[numVB] = 0x2; amasks[numVA] = 0x8;
            ebStates[1] = 6;
            vb[numVB++] = insertVertex( spB, idxb[1], pt0, t );
            va[numVA++] = insertVertex( spA, &trimA, pt1, uva0*bary.x + uva1*bary.y + uva2*bary.z );
         }
      }
   }

   // Testing edge b2.
   if( (db0db2 < 0.0f) && (ebStates[2] == -1) )
   {
      // Edge-edge.
      if( closestPtSegmentSegment( vb2, vb0, va0, va1, errorp, pt0, s, pt1, t ) )
      {
         if( (vaStates[0] == -1 || sqrLength(pt1-va0) > erroree) && (vaStates[1] == -1 || sqrLength(pt1-va1) > erroree) )
         {
            bmasks[numVB] = 0x4; amasks[numVA] = 0x1;
            ebStates[2] = 3;     eaStates[0] = 5;
            vb[numVB++] = insertVertex( spB, &trimB, pt0, CGM::linear2( uvb2, uvb0, s ) );
            va[numVA++] = insertVertex( spA, idxa[0], pt1, t );
         }
      }
      else if( closestPtSegmentSegment( vb2, vb0, va1, va2, errorp, pt0, s, pt1, t ) )
      {
         if( (vaStates[1] == -1 || sqrLength(pt1-va1) > erroree) && (vaStates[2] == -1 || sqrLength(pt1-va2) > erroree) )
         {
            bmasks[numVB] = 0x4; amasks[numVA] = 0x2;
            ebStates[2] = 4;     eaStates[1] = 5;
            vb[numVB++] = insertVertex( spB, &trimB, pt0, CGM::linear2( uvb2, uvb0, s ) );
            va[numVA++] = insertVertex( spA, idxa[1], pt1, t );
         }
      }
      else if( closestPtSegmentSegment( vb2, vb0, va2, va0, errorp, pt0, s, pt1, t ) )
      {
         if( (vaStates[2] == -1 || sqrLength(pt1-va2) > erroree) && (vaStates[0] == -1 || sqrLength(pt1-va0) > erroree) )
         {
            bmasks[numVB] = 0x4; amasks[numVA] = 0x4;
            ebStates[2] = 5;     eaStates[2] = 5;
            vb[numVB++] = insertVertex( spB, &trimB, pt0, CGM::linear2( uvb2, uvb0, s ) );
            va[numVA++] = insertVertex( spA, &trimA, pt1, CGM::linear2( uva2, uva0, t ) );
         }
      }
      // Edge-triangle.
      else
      {
         t   = db2/(db2-db0);
         pt0 = vb2+(vb0-vb2)*t;
         if( closestPtTriangle( pt0, va0, va1, va2, pt1, bary ) )
         {
            bmasks[numVB] = 0x4; amasks[numVA] = 0x8;
            ebStates[2] = 6;
            vb[numVB++] = insertVertex( spB, &trimB, pt0, CGM::linear2( uvb2, uvb0, t ) );
            va[numVA++] = insertVertex( spA, &trimA, pt1, uva0*bary.x + uva1*bary.y + uva2*bary.z );
         }
      }
   }

   // 4. Edge creation.
   if( numVA > 1 )
   {
      TrimmingData* trimData = _cache->trimmingData();
      // Compute type of edge insertion.
      int edgea = amasks[0] & amasks[1];
      int edgeb = bmasks[0] & bmasks[1];
      // Remove triangle-triangle and edge 2 (interior edge) cases.
      if( edgea >= 4 ) edgea = 0;
      if( edgeb >= 4 ) edgeb = 0;

      if( edgea == 0 )
      {
         if( edgeb == 0 )
         {
            trimData->addEdge( &trimA, va[0], va[1] );
            trimData->addEdge( &trimB, vb[0], vb[1] );
         }
         else
         {
            if( vb[0] == vb[1] ) return;
            // edge B and face A.
            // Find direction in which to move in pointer.
            int axis = idxb[edgeb>>1]&1;
            int incb = vb[0]->_uv(axis) < vb[1]->_uv(axis) ? axis+1: (axis+3)%4;
            // Insert an edge for each vertex of B contained inside A.
            Vertex* ca = va[0];
            for( Vertex* cb = vb[0]->_next[incb]; cb != vb[1]; cb = cb->_next[incb] )
            {
               if( closestPtTriangle( cb->_pos, va0, va1, va2, pt0, bary ) )
               {
                  Vertex* na = insertVertex( spA, &trimA, pt0, uva0*bary.x + uva1*bary.y + uva2*bary.z );
                  trimData->addEdge( &trimA, ca, na );
                  ca = na;
               }
            }
            trimData->addEdge( &trimA, ca, va[1] );
         }
      }
      else
      {
         if( edgeb == 0 )
         {
            if( va[0] == va[1] ) return;
            // edge A and face B.
            // Find direction in which to move in pointer.
            int axis = idxa[edgea>>1]&1;
            int inca = va[0]->_uv(axis) < va[1]->_uv(axis) ? axis+1: (axis+3)%4;
            // Insert an edge for each vertex of A contained inside B.
            Vertex* cb = vb[0];
            for( Vertex* ca = va[0]->_next[inca]; ca != va[1]; ca = ca->_next[inca] )
            {
               if( closestPtTriangle( ca->_pos, vb0, vb1, vb2, pt0, bary ) )
               {
                  Vertex* nb = insertVertex( spB, &trimB, pt0, uvb0*bary.x + uvb1*bary.y + uvb2*bary.z );
                  trimData->addEdge( &trimB, cb, nb );
                  cb = nb;
               }
            }
            trimData->addEdge( &trimB, cb, vb[1] );
         }
         else
         {
            if( va[0] == va[1] ) return;
            if( vb[0] == vb[1] ) return;
            // edge A and edge B.
            int axisa = idxa[edgea>>1]&1;
            int inca  = va[0]->_uv(axisa) < va[1]->_uv(axisa) ? axisa+1: (axisa+3)%4;
            int axisb = idxb[edgeb>>1]&1;
            int incb  = vb[0]->_uv(axisb) < vb[1]->_uv(axisb) ? axisb+1: (axisb+3)%4;
            // 1. for each vertex between vb[0] and vb[1] insert it in edge A.
            for( Vertex* cb = vb[0]->_next[incb]; cb != vb[1]; cb = cb->_next[incb] )
            {
               if( (edgea>>1) == 0 )
               {
                  if( closestPtSegment( cb->_pos, va0, va1, errorp, pt0, t ) )
                     insertVertex( spA, idxa[edgea>>1], pt0, t );
               }
               else
               {
                  if( closestPtSegment( cb->_pos, va1, va2, errorp, pt0, t ) )
                     insertVertex( spA, idxa[edgea>>1], pt0, t );
               }
            }
            // 2. insert vertices between va[0] and va[1] in edge B.
            for( Vertex* ca = va[0]->_next[inca]; ca != va[1]; ca = ca->_next[inca] )
            {
               if( (edgeb>>1) == 0 )
               {
                  if( closestPtSegment( ca->_pos, vb0, vb1, errorp, pt0, t ) )
                     insertVertex( spB, idxb[edgeb>>1], pt0, t );
               }
               else
               {
                  if( closestPtSegment( ca->_pos, vb1, vb2, errorp, pt0, t ) )
                     insertVertex( spB, idxb[edgeb>>1], pt0, t );
               }
            }
         }
      }
   }
}

/*==============================================================================
   INTERSECTION
==============================================================================*/

//------------------------------------------------------------------------------
//! 
bool 
MetaSurface::trace( const Rayf& ray )
{
   // FIXME: optimized with raycasting throw hgrid and AABBTrees.
   for( PatchIter pIt = patches(); pIt.isValid(); ++pIt )
   {
      if( isHidden( *pIt ) ) continue;
      for( SubpatchIter spIt = subpatches(*pIt); spIt.isValid(); ++spIt )
      {
         if( isHidden( *spIt ) ) continue;

         // Retreive triangle approximation.
         Vec3f a, b, c, d;
         Vec3f n0, n1;
         computeApproximation( *spIt, a, b, c, d, n0, n1 );
         Intersector::Hit hit;
         hit._t = 1.0f;
         if( isTrimmed( *spIt ) )
         {
            // FIXME: use custom code to NOT hit holes.
            // Intersect first triangle.
            if( Intersector::trace( a, b, c, ray, hit ) ) return true;
            // Intersect second triangle.
            if( Intersector::trace( a, c, d, ray, hit ) ) return true;
         }
         else
         {
            // Intersect first triangle.
            if( Intersector::trace( a, b, c, ray, hit ) ) return true;
            // Intersect second triangle.
            if( Intersector::trace( a, c, d, ray, hit ) ) return true;
         }
      }
   }
   return false;
}

//------------------------------------------------------------------------------
//! 
bool 
MetaSurface::trace( const Rayf& ray, Intersector::Hit& hit )
{
   bool intersected = false;

   // FIXME: optimized with raycasting throw hgrid and AABBTrees.
   for( PatchIter pIt = patches(); pIt.isValid(); ++pIt )
   {
      if( isHidden( *pIt ) ) continue;
      for( SubpatchIter spIt = subpatches(*pIt); spIt.isValid(); ++spIt )
      {
         if( isHidden( *spIt ) ) continue;

         // Retreive triangle approximation.
         Vec3f a, b, c, d;
         Vec3f n0, n1;
         computeApproximation( *spIt, a, b, c, d, n0, n1 );

         if( isTrimmed( *spIt ) )
         {
            // FIXME: use custom code to NOT hit holes.
            // Intersect first triangle.
            if( Intersector::trace( a, b, c, ray, hit ) )
            {
               hit._normal = isFlipped( *pIt ) ? -n0 : n0;
               hit._face   = spIt;
               intersected = true;
            }
            // Intersect second triangle.
            if( Intersector::trace( a, c, d, ray, hit ) )
            {
               hit._normal = isFlipped( *pIt ) ? -n1 : n1;
               hit._face   = spIt;
               intersected = true;
            }
         }
         else
         {
            // Intersect first triangle.
            if( Intersector::trace( a, b, c, ray, hit ) )
            {
               hit._normal = isFlipped( *pIt ) ? -n0 : n0;
               hit._face   = spIt;
               intersected = true;
            }
            // Intersect second triangle.
            if( Intersector::trace( a, c, d, ray, hit ) )
            {
               hit._normal = isFlipped( *pIt ) ? -n1 : n1;
               hit._face   = spIt;
               intersected = true;
            }
         }
      }
   }
   return intersected;
}

/*==============================================================================
   DEBUG
==============================================================================*/

//------------------------------------------------------------------------------
//! 
void 
MetaSurface::dump( const Subpatch& sp, const Trimming& trim ) const
{
   StdErr << "x: " << trim._x << " y: " << trim._y << " flags: " << trim._flags << "\n";
   StdErr << "plane0: " << trim._plane0 << "\n";
   StdErr << "plane1: " << trim._plane1 << "\n";
   StdErr << "v: ";
   for( Vertex* v = trim._vertices; v; v = v->_next[0] )
   {
      StdErr << v->_pos << " " << v->_uv << "\n";
   }
   StdErr << "\n";
   for( Edge* e = trim._edges; e; e = e->_next )
   {
      StdErr << "e: ";
      // Find v0.
      int vc = 0;
      for( Vertex* v = trim._vertices; v; v = v->_next[0], ++vc )
      {
         if( v == e->_v0 ) 
         {
            StdErr << "i[" << vc << "] ";
            break;
         }
      }
      Vertex* v = sp._corners[0];
      vc = 0;
      for( uint c = 0; c < 4; ++c )
      {
         for( ; v != sp._corners[(c+1)%4]; v = v->_next[(c+1)%4], ++vc )
         {
            if( v == e->_v0 ) StdErr << "v[" << vc << "] ";
         }
      }
      // Find v1.
      vc = 0;
      for( Vertex* v = trim._vertices; v; v = v->_next[0], ++vc )
      {
         if( v == e->_v1 ) 
         {
            StdErr << "i[" << vc << "] ";
            break;
         }
      }
      v = sp._corners[0];
      vc = 0;
      for( uint c = 0; c < 4; ++c )
      {
         for( ; v != sp._corners[(c+1)%4]; v = v->_next[(c+1)%4], ++vc )
         {
            if( v == e->_v1 ) StdErr << "v[" << vc << "] ";
         }
      }
      StdErr << e->_v0->_pos << " " << e->_v1->_pos << "\n";
   }
   for( Loop* l = trim._loops; l; l = l->_next )
   {
      StdErr << "loop: ";
      for( LoopLink* ll = l->_link; ll; ll = ll->_next )
      {
         StdErr << ll->_vertex->_pos << " ";
      }
      StdErr << "\n";
   }
}

//------------------------------------------------------------------------------
//! 
void
MetaSurface::dump( const Subpatch& sp )
{
   StdErr << "subpatch\n";
   StdErr << "flags: " << sp._flags << "\n";

   Vertex* v = sp._corners[0];
   for( uint c = 0; c < 4; ++c )
   {
      for( ; v != sp._corners[(c+1)%4]; v = v->_next[(c+1)%4] )
      {
         StdErr << "  v: " << v->_pos << " " << v->_uv << "\n";
      }
   }
}

NAMESPACE_END
