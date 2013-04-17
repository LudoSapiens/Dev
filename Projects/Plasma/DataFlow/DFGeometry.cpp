/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/DataFlow/DFGeometry.h>
#include <Plasma/DataFlow/DFPatch.h>
#include <Plasma/Geometry/MeshGeometry.h>

#include <CGMath/HGrid.h>
#include <CGMath/RayCaster.h>

#include <algorithm>

//#define PER_SUBPATCH_MAPPING 1

/*==============================================================================
   UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

float _errormax = (1.0f/4096.0f);

//------------------------------------------------------------------------------
//!
template< typename T > T
baryc( const T& a, const T& b, const T& c, const Vec3f& bc )
{
   return a*bc.x + b*bc.y + c*bc.z;
}

//------------------------------------------------------------------------------
//!
bool lt( const Vec2f& a, const Vec2f& b )
{
   return (a.x < b.x) || ( (a.x == b.x) && (a.y < b.y) );
}

//------------------------------------------------------------------------------
//!
inline uint16_t
prevVertex( DFGeometry::Patch& p, uint16_t vs, uint16_t ve, uint axis, float val )
{
   while( (p._vertices[vs]._uv(axis) <= val) && (vs != ve) )
   {
      vs = p._vertices[vs]._next[axis+1];
   }
   return p._vertices[vs]._next[(axis+3)%4];
}

//------------------------------------------------------------------------------
//!
inline AABBoxf
computeAABB( const DFGeometry::Patch& p )
{
   AABBoxf box = AABBoxf::empty();
   for( auto v = p._vertices.begin(); v != p._vertices.end(); ++v )
   {
      box |= v->_pos;
   }
   box.grow( 1.0f/1024.0f );
   return box;
}

//------------------------------------------------------------------------------
//!
inline AABBoxf
computeAABB( const DFGeometry::Patch& p, uint16_t spID )
{
   const DFGeometry::Subpatch& sp = p._subpatches[spID];
   AABBoxf box = AABBoxf::empty();
   box |= p._vertices[sp._corners[0]]._pos;
   box |= p._vertices[sp._corners[1]]._pos;
   box |= p._vertices[sp._corners[2]]._pos;
   box |= p._vertices[sp._corners[3]]._pos;
   box.grow( 1.0f/1024.0f );
   return box;
}


/*==============================================================================
   INTERSECTIONS
==============================================================================*/

struct Overlap
{
   DFGeometry* _geom;
   uint32_t    _p0;
   uint32_t    _p1;
};

//------------------------------------------------------------------------------
//!
void patchesOverlap( void* data, void* objA, void* objB )
{
   DFGeometry* geom  = (DFGeometry*)data;
   size_t idA        = (size_t)objA;
   size_t idB        = (size_t)objB;

   geom->trimPatches( uint32_t(idA), uint32_t(idB) );
}

//------------------------------------------------------------------------------
//!
void
subpatchesOverlap( void* data, void* objA, void* objB )
{
   Overlap* ov       = (Overlap*)data;
   size_t idA        = (size_t)objA;
   size_t idB        = (size_t)objB;

   ov->_geom->trimSubpatches( ov->_p0, uint16_t(idA), ov->_p1, uint16_t(idB) );
}

/*==============================================================================
   CLASSIFICATION
==============================================================================*/

enum {
   VOL_IN  = 1,
   VOL_OUT = 2,
   B_IN    = 4,
   B_OUT   = 8
};

struct Classify
{
   AABBoxf _region;
   Rayf    _ray;
   int     _count;
   bool    _boundary;
};

//------------------------------------------------------------------------------
//!
void patchesClassify( void* data, void* objA, void* objB )
{
   DFGeometry* geom = (DFGeometry*)data;
   size_t idA       = (size_t)objA;
   Classify* cl     = (Classify*)objB;

   // Patch is not visible.
   DFGeometry::Patch& p = geom->patch(uint(idA));
   if( p.isHidden() ) return;

   int count[] = { -1, 1 };

   // Create triangle to test intersection.
   Vector<float> vertices;
   Vector<uint> indices;
   Grid<DFGeometry::Vertex*> grid( uint(p._vertices.size())*2, 1.0f/128.0f );
   geom->triangulate( p, grid, vertices, indices, nullptr, true );

   RayCaster::Hitf hit;
   size_t numTri = indices.size() / 3;
   for( size_t t = 0; t < numTri; ++t )
   {
      hit._t  = CGConstf::infinity();
      uint i0 = indices[t*3+0]*8;
      uint i1 = indices[t*3+1]*8;
      uint i2 = indices[t*3+2]*8;
      Vec3f v0( vertices[i0], vertices[i0+1], vertices[i0+2] );
      Vec3f v1( vertices[i1], vertices[i1+1], vertices[i1+2] );
      Vec3f v2( vertices[i2], vertices[i2+1], vertices[i2+2] );
      if( RayCaster::hit( cl->_ray, v0, v1, v2, hit ) )
      {
         cl->_count += hit._backFacing ? count[0]: count[1];
         if( hit._t < _errormax*8.0f ) cl->_boundary = true;
      }
   }
}

//------------------------------------------------------------------------------
//!
int classifyPoint( const Vec3f& pos, const Vec3f& dir, DFGeometry* geom, HGrid& grid )
{
   // 2. Throw rays.
   // Compute main axis.
   Classify data;
   int axis       = dir.maxComponent();
   data._count    = 0;
   data._boundary = false;
   data._ray      = Rayf( pos, Vec3f(0.0f) );

   // Compute test region.
   data._region.set( pos );
   if( dir(axis) < 0.0f )
   {
      data._region.min(axis)      = -1e4f;
      data._region.max(axis)     += _errormax*4.0f;
      data._ray.direction()(axis) = -1.0f;
      data._ray.origin()(axis)   += _errormax*4.0f;
   }
   else
   {
      data._region.max(axis)      = 1e4f;
      data._region.min(axis)     -= _errormax*4.0f;
      data._ray.direction()(axis) = 1.0f;
      data._ray.origin()(axis)   -= _errormax*4.0f;
   }

   grid.findCollisions( geom, &data, data._region, patchesClassify );
   return (data._count == 0) ? (data._boundary ? B_OUT : VOL_OUT) : (data._boundary ? B_IN : VOL_IN);
}

/*==============================================================================
   TRIANGULATION
==============================================================================*/

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

//------------------------------------------------------------------------------
//!
float triangleCost(
   const Vector<DFGeometry::Vertex*>& vptrs,
   const Vector<int>&                 prev,
   const Vector<int>&                 next,
   uint                               x,
   uint                               y,
   uint                               i
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
//! Ear-clipping.
void triangulate(
   const Vector<DFGeometry::Vertex*>& vptrs,
   Vector<uint>&                      indices,
   bool                               flip,
   uint                               x,
   uint                               y
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
//! Stripping + fanning.
void triangulate(
   const Vector<DFGeometry::Vertex*>& vptrs,
   uint                               nbVertices[4],
   bool                               flip,
   Vector<uint>&                      indices
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
float triangleCost(
   const DFGeometry::Patch& p,
   uint16_t*                vertices,
   const Vector<int>&       prev,
   const Vector<int>&       next,
   uint                     x,
   uint                     y,
   uint                     i
)
{
   int pi  = prev[i];
   int ni  = next[i];
   Vec2f a = p._vertices[vertices[pi]]._pos(x,y);
   Vec2f b = p._vertices[vertices[i]]._pos(x,y);
   Vec2f c = p._vertices[vertices[ni]]._pos(x,y);

   // Is triangle convex (CCW)?
   float area;
   if( !ccw( a, b, c, area ) ) return CGConstf::infinity();

   // Is it an ear?
   int j = next[ni];
   do {
      // Is current vertex in triangle?
      if( pointInTriangle( p._vertices[vertices[j]]._pos(x,y), a, b, c ) )
      {
         if( (vertices[j] != vertices[pi]) && (vertices[j] != vertices[i]) && (vertices[j] != vertices[ni]) )
            return CGConstf::infinity();
      }
      j = next[j];
   } while( j != pi );
   // Compute cost.
   return area;
}

//------------------------------------------------------------------------------
//!
void computePoint(
   const DFGeometry::Patch& p,
   uint16_t*                vertices,
   uint                     numV,
   uint                     x,
   uint                     y,
   Vec3f&                   pos,
   Vec3f&                   n
)
{
   // Create double link list of vertices.
   Vector<int> prev(numV);
   Vector<int> next(numV);
   for( uint i = 0; i < numV; ++i )
   {
      prev[i] = i-1;
      next[i] = i+1;
   }
   prev[0]      = numV-1;
   next[numV-1] = 0;

   // Compute which vertices are ears.
   uint besti     = 0;
   float bestCost = CGConstf::infinity();
   Vector<float> cost(numV);
   for( uint i = 0; i < numV; ++i )
   {
      cost[i] = triangleCost( p, vertices, prev, next, x, y, i );
      if( cost[i] < bestCost )
      {
         besti    = i;
         bestCost = cost[i];
      }
   }

   if( bestCost < -1e-5f || numV <= 4 )
   {
      const DFGeometry::Vertex& v0 = p._vertices[vertices[prev[besti]]];
      const DFGeometry::Vertex& v1 = p._vertices[vertices[besti]];
      const DFGeometry::Vertex& v2 = p._vertices[vertices[next[besti]]];
      pos = v0._pos*0.25f + v1._pos*0.25f + v2._pos*0.5f;
      n   = normalize(v0._n*0.25f + v1._n*0.25f + v2._n*0.5f);
      return;
   }

   // Find a better triangle.

   // Remove vertices.
   next[prev[besti]] = next[besti];
   prev[next[besti]] = prev[besti];
   --numV;
   uint i = prev[besti];

   while( numV > 3 )
   {
      // Update neighbot cost.
      cost[i]       = triangleCost( p, vertices, prev, next, x, y, i );
      cost[next[i]] = triangleCost( p, vertices, prev, next, x, y, next[i] );

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
         const DFGeometry::Vertex& v0 = p._vertices[vertices[prev[i]]];
         const DFGeometry::Vertex& v1 = p._vertices[vertices[i]];
         const DFGeometry::Vertex& v2 = p._vertices[vertices[next[i]]];
         pos = v0._pos*0.25f + v1._pos*0.25f + v2._pos*0.5f;
         n   = v0._n*0.25f   + v1._n*0.25f   + v2._n*0.5f;
      }

      // Remove vertices.
      next[prev[i]] = next[i];
      prev[next[i]] = prev[i];
      --numV;
      i = prev[i];
   }

   n.normalize();
}

/*==============================================================================
   LOOPS BUILDING
==============================================================================*/

//------------------------------------------------------------------------------
//!
struct TrimEdge
{
   uint16_t _v0;
   uint16_t _v1;
   int      _group;
};

//------------------------------------------------------------------------------
//!
struct CompVert
{
   CompVert( DFGeometry::Patch& p, const Vec2f& v, int x, int y ):
      _p(p), _v(v), _x(x), _y(y) {}

   bool operator()( uint16_t a, uint16_t b ) const
   {
      float dista = (_v-_p._vertices[a]._pos(_x,_y)).sqrLength();
      float distb = (_v-_p._vertices[b]._pos(_x,_y)).sqrLength();
      return dista < distb;
   }

   DFGeometry::Patch& _p;
   Vec2f              _v;
   int                _x;
   int                _y;
};

//------------------------------------------------------------------------------
//!
struct CompEdge
{
   CompEdge( DFGeometry::Patch& p ,DFGeometry::Trimming& t ):
      _p(p), _t(t) {}

   bool operator()( uint16_t a, uint16_t b ) const
   {
      auto e0 = _t._edges.begin()+a;
      auto e1 = _t._edges.begin()+b;

      Vec2f v0 = _p._vertices[e0->first]._pos(_t._x,_t._y);
      Vec2f v1 = _p._vertices[e1->first]._pos(_t._x,_t._y);
      return lt( v0, v1 );
   }

   DFGeometry::Patch&    _p;
   DFGeometry::Trimming& _t;
};

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   DEBUG
==============================================================================*/

void dump( DFGeometry::Patch& p, DFGeometry::Subpatch& sp )
{
   uint16_t vID = sp._corners[0];
   StdErr << "subpatch\n";
   for( uint c = 0; c < 4; ++c )
   {
      for( ; vID != sp._corners[(c+1)%4]; vID = p._vertices[vID]._next[(c+1)%4] )
      {
         StdErr << p._vertices[vID]._pos << p._vertices[vID]._uv << nl;
      }
   }
}

//------------------------------------------------------------------------------
//!
void dump( DFGeometry::Patch& p, DFGeometry::Trimming& t )
{
   for( auto e = t._edges.begin(); e != t._edges.end(); ++e )
   {
      StdErr << "e: " << p._vertices[e->first]._pos << p._vertices[e->first]._uv
                      << p._vertices[e->second]._pos << p._vertices[e->second]._uv << nl;
   }
   int stLoop = 1;
   for( int l = 0; l < t._loops[0]; ++l, stLoop += t._loops[stLoop]+2 )
   {
      StdErr << "loop: " << l << nl;
      int endLoop = stLoop + t._loops[stLoop]+2;
      for( int i = stLoop+2 ; i < endLoop; ++i )
      {
         DFGeometry::Vertex& v = p._vertices[t._loops[i]];
         StdErr << v._pos << v._uv << nl;
      }
   }
}

/*==============================================================================
   CLASS LoopBuilder
==============================================================================*/

class LoopBuilder
{
public:

   /*----- methods -----*/

   LoopBuilder( DFGeometry* g ): _geom(g) {}
   void computeLoops( DFGeometry::Patch&, DFGeometry::Subpatch&, DFGeometry::Trimming& );

protected:

   uint16_t findNextEdge( DFGeometry::Patch& p, int e, int x, int y );
   void fixLoop( DFGeometry::Patch& p, int x, int y );
   void tagLoop( DFGeometry::Patch& p, int e, int group, int x, int y );
   bool testEdges( DFGeometry::Patch& p, uint16_t v0p, const Vec2f& v0, const Vec2f& v1, int x, int y );
   void removeIntersections( DFGeometry::Patch& p, DFGeometry::Trimming& t );

   inline void addEdge( const TrimEdge& e, uint id )
   {
      _trimEdges.pushBack( e );
      _starsNext.pushBack( _starsFirst[e._v0] );
      _starsFirst[e._v0] = id;
   }

   /*----- types -----*/

   typedef Pair<float,uint16_t> VertexFix;

   /*----- data members -----*/

   DFGeometry*                   _geom;
   Vector<uint16_t>              _vertices;
   Vector<TrimEdge>              _trimEdges;
   Vector<uint16_t>              _starsFirst;
   Vector<uint16_t>              _starsNext;
   Vector<uint16_t>              _conVertices;
   Vector<uint16_t>              _minVertices;
   Vector<uint16_t>              _stack;
   Vector<uint16_t>              _edges;
   Vector< Vector< VertexFix > > _edgesFix;
};

//------------------------------------------------------------------------------
//!
void LoopBuilder::computeLoops( DFGeometry::Patch& p, DFGeometry::Subpatch& sp, DFGeometry::Trimming& t )
{
   if( !t.isInvalid() ) return;

   // 1. Solve edges intersections.
   removeIntersections( p, t );

   // 2. Creating vertices info and edges for subpatch main loop.
   _vertices.clear();
   _starsFirst.clear();
   _starsNext.clear();
   _trimEdges.clear();

   TrimEdge edge;
   edge._group = 0;

   uint16_t vID = sp._corners[0];
   uint i       = 0;
   for( uint c = 0; c < 4; ++c )
   {
      for( ; vID != sp._corners[(c+1)%4]; vID = p._vertices[vID]._next[(c+1)%4], ++i )
      {
         // Vertex info.
         p._vertices[vID]._id = i;
         _vertices.pushBack(vID);
         _starsFirst.pushBack(i);
         // Edge info.
         edge._v0 = i;
         edge._v1 = i+1;
         _trimEdges.pushBack( edge );
         _starsNext.pushBack( DFGeometry::NULL_ID16 );
      }
   }
   _trimEdges.back()._v1 = 0;

   // 3. Create vertices info and edges for interior edges.
   // Interior vertices.
   for( vID = t._vertices; vID != DFGeometry::NULL_ID16; vID = p._vertices[vID]._next[0], ++i )
   {
      p._vertices[vID]._id = i;
      _vertices.pushBack(vID);
      _starsFirst.pushBack( DFGeometry::NULL_ID16 );
   }
   // Interior edges (count for 2 half edges).
   i = uint(_trimEdges.size());
   int eID = 0;
   for( auto e = t._edges.begin(); e != t._edges.end(); ++e, ++eID )
   {
      if( _edgesFix[eID].empty() )
      {
         edge._v0 = p._vertices[e->first]._id;
         edge._v1 = p._vertices[e->second]._id;
         addEdge( edge, i++ );

         CGM::swap( edge._v0, edge._v1 );
         addEdge( edge, i++ );
      }
      else
      {
         // Sort vertices.
         std::sort( _edgesFix[eID].begin(), _edgesFix[eID].end() );

         // Add edges.
         uint16_t v0 = e->first;
         for( auto ef = _edgesFix[eID].begin(); ef != _edgesFix[eID].end(); ++ef )
         {
            // Skip duplicate.
            if( ef->second == v0 ) continue;
            edge._v0 = p._vertices[v0]._id;
            edge._v1 = p._vertices[ef->second]._id;
            addEdge( edge, i++ );

            CGM::swap( edge._v0, edge._v1 );
            addEdge( edge, i++ );
            v0 = ef->second;
         }
         edge._v0 = p._vertices[v0]._id;
         edge._v1 = p._vertices[e->second]._id;
         addEdge( edge, i++ );

         CGM::swap( edge._v0, edge._v1 );
         addEdge( edge, i++ );
      }
   }

   // 4. Find and connect disjoint loops.
   fixLoop( p, t._x, t._y );

   // 5. Create loops.
   // Clear old loops.
   Vector<uint16_t> loops;
   t.invalidate(0);
   t._loops.swap( loops );
   t._loops.pushBack(0); // 0 loops.

   for( i = 0; i < _trimEdges.size(); ++i )
   {
      // Edge already in a loop?
      if( _trimEdges[i]._group < 0 ) continue;

      // New loop.
      ++t._loops[0];        // #loops
      size_t stLoop = t._loops.size();
      t._loops.pushBack(0); // #vertex
      t._loops.pushBack(0); // flags

      // First vertex.
      t._loops.pushBack( _vertices[_trimEdges[i]._v0] );
      ++t._loops[stLoop];
      // Set edge as done.
      _trimEdges[i]._group = -1;

      int j = i;
      while( true )
      {
         // Find next edge
         j = findNextEdge( p, j, t._x, t._y );
         // No more edge?
         if( j == DFGeometry::NULL_ID16 ) break;

         // Add vertex.
         t._loops.pushBack( _vertices[_trimEdges[j]._v0] );
         ++t._loops[stLoop];
         // Set edge as done.
         _trimEdges[j]._group = -1;
      }
   }

   // Set hidden flags for the loops.
   Vec3f pos;
   Vec3f n;
   for( int l = 0, i = 1; l < t._loops[0]; ++l, i += t._loops[i]+2 )
   {
      _geom->getLoopPoint( p, &t._loops[i+2], t._loops[i], t._x, t._y, pos, n );
      for( int l2 = 0, i2 = 1; l2 < loops[0]; ++l2, i2 += loops[i2]+2 )
      {
         if( _geom->pointInLoop( p, &loops[i2+2], loops[i2], t._x, t._y, pos ) )
            t._loops[i+1] = loops[i2+1];
      }
   }
}

//------------------------------------------------------------------------------
//!
uint16_t LoopBuilder::findNextEdge( DFGeometry::Patch& p, int e, int x, int y )
{
   uint16_t v0 = _trimEdges[e]._v0;
   uint16_t v1 = _trimEdges[e]._v1;
   uint16_t re = DFGeometry::NULL_ID16;
   float ra    = CGConstf::infinity();
   e           = _starsFirst[v1];

   Vec2f p0 = p._vertices[_vertices[v0]]._pos(x,y);
   Vec2f p1 = p._vertices[_vertices[v1]]._pos(x,y);
   Vec2f d0 = (p0-p1).getNormalized();

   while( e != DFGeometry::NULL_ID16 )
   {
      float angle = 2.0f;
      uint16_t v2 = _trimEdges[e]._v1;
      if( v2 != v0 )
      {
         Vec2f p2  = p._vertices[_vertices[v2]]._pos(x,y);
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
   if( re != DFGeometry::NULL_ID16 && _trimEdges[re]._group < 0 ) re = DFGeometry::NULL_ID16;
   return re;
}

//------------------------------------------------------------------------------
//!
void LoopBuilder::fixLoop( DFGeometry::Patch& p, int x, int y )
{
   _minVertices.clear();

   int group = 1;
   // Find loops.
   for( uint i = 0; i < _trimEdges.size(); ++i )
   {
      if( _trimEdges[i]._group > 0 ) continue;
      tagLoop( p, i, group, x, y );
      ++group;
   }
   // Connect loop.
   if( group > 2 )
   {
      for( uint g = 1; g < _minVertices.size(); ++g )
      {
         _conVertices.clear();
         Vec2f v1 = p._vertices[_minVertices[g]]._pos(x,y);
         // Create a list of potential vertices to connect to( < x ).
         for( uint v = 0; v < _vertices.size(); ++v )
         {
            if( p._vertices[_vertices[v]]._pos(x) < v1.x ) _conVertices.pushBack( _vertices[v] );
         }
         // Sort list by distance.
         std::sort( _conVertices.begin(), _conVertices.end(), CompVert( p, v1, x, y ) );

         // Find witch vertex to connect to and create edges.
         for( uint v = 0; v < _conVertices.size(); ++v )
         {
            Vec2f v0 = p._vertices[_conVertices[v]]._pos(x,y);
            // Test against edges.
            if( testEdges( p, _conVertices[v], v0, v1, x, y ) )
            {
               // Add edges.
               TrimEdge edge;
               edge._group = g+1;
               edge._v0 = p._vertices[_minVertices[g]]._id;
               edge._v1 = p._vertices[_conVertices[v]]._id;
               _trimEdges.pushBack( edge );
               _starsNext.pushBack( _starsFirst[edge._v0] );
               _starsFirst[edge._v0] = uint16_t(_trimEdges.size())-1;

               CGM::swap( edge._v0, edge._v1 );
               _trimEdges.pushBack( edge );
               _starsNext.pushBack( _starsFirst[edge._v0] );
               _starsFirst[edge._v0] = uint16_t(_trimEdges.size())-1;
               break;
            }
         }
      }
   }
}

//------------------------------------------------------------------------------
//!
void LoopBuilder::tagLoop( DFGeometry::Patch& p, int e, int group, int x, int y )
{
   uint16_t minVertex = _vertices[_trimEdges[e]._v0];
   Vec2f v            = p._vertices[minVertex]._pos(x,y);
   while( 1 )
   {
      // Tag edge.
      _trimEdges[e]._group = group;
      // Keep min loop's vertex.
      uint16_t vertex  = _vertices[_trimEdges[e]._v1];
      Vec2f cv         = p._vertices[vertex]._pos(x,y);
      if( (cv.x < v.x) || (cv.x == v.x && cv.y < v.y) )
      {
         v         = cv;
         minVertex = vertex;
      }
      // Find next edges.
      int ne = _starsFirst[_trimEdges[e]._v1];
      while( ne != DFGeometry::NULL_ID16 )
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

//------------------------------------------------------------------------------
//!
bool LoopBuilder::testEdges( DFGeometry::Patch& p, uint16_t v0p, const Vec2f& v0, const Vec2f& v1, int x, int y )
{
   for( uint e = 0; e < _trimEdges.size(); ++e )
   {
      uint16_t v2p = _vertices[_trimEdges[e]._v0];
      uint16_t v3p = _vertices[_trimEdges[e]._v1];
      if( p._vertices[v2p]._pos(x) > p._vertices[v3p]._pos(x) ) CGM::swap( v2p, v3p );

      if( p._vertices[v2p]._pos(x) < v1.x && p._vertices[v3p]._pos(x) > v0.x )
      {
         if( v2p == v0p || v3p == v0p ) continue;
         Vec2f v2 = p._vertices[v2p]._pos( x, y );
         Vec2f v3 = p._vertices[v3p]._pos( x, y );

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

//------------------------------------------------------------------------------
//!
void LoopBuilder::removeIntersections( DFGeometry::Patch& p, DFGeometry::Trimming& t )
{
   int x = t._x;
   int y = t._y;

   // Prepare memory for edge correction structure.
   _edgesFix.clear();
   _edgesFix.resize( t._edges.size() );

   // Sort edges by x and y.
   _edges.resize( t._edges.size() );
   int eID = 0;
   for( auto e = t._edges.begin(); e != t._edges.end(); ++e, ++eID ) _edges[eID] = eID;
   std::sort( _edges.begin(), _edges.end(), CompEdge( p, t ) );

   // First we detect intersections and overlaps.
   for( auto e0 = _edges.begin(); e0 != _edges.end(); ++e0 )
   {
      int e0ID = *e0;
      for( auto e1 = e0+1; e1 != _edges.end(); ++e1 )
      {
         int e1ID = *e1;
         uint16_t v1p = (t._edges.begin()+e0ID)->second;
         uint16_t v2p = (t._edges.begin()+e1ID)->first;

         // No more intersection since edges are sorted by X.
         if( p._vertices[v1p]._pos(x) + _errormax < p._vertices[v2p]._pos(x) ) break;

         uint16_t v0p = (t._edges.begin()+e0ID)->first;
         uint16_t v3p = (t._edges.begin()+e1ID)->second;

         // Compute intersection.
         Vec2f v0 = p._vertices[v0p]._pos( x, y );
         Vec2f v1 = p._vertices[v1p]._pos( x, y );
         Vec2f v2 = p._vertices[v2p]._pos( x, y );
         Vec2f v3 = p._vertices[v3p]._pos( x, y );

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

               // We have an overlap!!!
               StdErr << "ERROR: we have an overlap.\n";
               /*
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
                  */
            }
         }
         else
         {
            // 1. compute intersection.
            float t0  = num0/den;
            float t1  = num1/den;
            // Use t1 and not t0 to make sure pos.x >= v2.x.
            Vec3f pos = p._vertices[v2p]._pos*(1.0f-t1) + p._vertices[v3p]._pos*t1;
            Vec2f p2d = pos(x,y);

            // 2. test v0
            if( equal( v0, p2d, _errormax ) )
            {
               // 2.1 test v2
               if( equal( v2, p2d, _errormax ) ) continue;
               // 2.2 test v3
               if( equal( v3, p2d, _errormax ) ) continue;
               // 2.3 test t1
               if( t1 < 0.0f || t1 > 1.0f ) continue;
               // Intersection v0 and e1: add v0 to edge e1.
               _edgesFix[e1ID].pushBack( VertexFix( t1, v0p ) );
            }
            // 3. test v1
            else if( equal( v1, p2d, _errormax ) )
            {
               // 3.1 test v2
               if( equal( v2, p2d, _errormax ) ) continue;
               // 3.2 test v3
               if( equal( v3, p2d, _errormax ) ) continue;
               // 3.3 test t1
               if( t1 < 0.0f || t1 > 1.0f ) continue;
               // Intersection v1 and e1: add v1 to edge e1.
               _edgesFix[e1ID].pushBack( VertexFix( t1, v1p ) );
            }
            // 4. test t0.
            else if( 0.0f < t0 && t0 < 1.0f )
            {
               // 5. test v2
               if( equal( v2, p2d, _errormax ) )
               {
                  // Intersection e0 and v2: add v2 to e0.
                  _edgesFix[e0ID].pushBack( VertexFix( t0, v2p ) );
               }
               // 6. test v3
               else if( equal( v3, p2d, _errormax ) )
               {
                  // Intersection e0 and v3: add v3 to e0.
                  _edgesFix[e0ID].pushBack( VertexFix( t0, v3p ) );
               }
               // 7. test t1
               else if ( 0.0f < t1 && t1 < 1.0f )
               {
                  // Intersection e0 and e1: add pos to e0 and e1.
                  Vec3f n     = normalize( p._vertices[v2p]._n*(1.0f-t1) + p._vertices[v3p]._n*t1 );
                  Vec2f uv    = p._vertices[v2p]._uv*(1.0f-t1) + p._vertices[v3p]._uv*t1;
                  uint16_t nv = _geom->insertFaceVertex( p, t, pos, n, uv );
                  _edgesFix[e0ID].pushBack( VertexFix( t0, nv ) );
                  _edgesFix[e1ID].pushBack( VertexFix( t1, nv ) );
               }
            }
         }
      }
   }
}

/*==============================================================================
   CLASS DFGeometry
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFGeometry::DFGeometry(): Geometry( DFGEOMETRY ), _numSubpatches(0)
{
   _aabbPool = new AABBTree::Pool();
}

//------------------------------------------------------------------------------
//!
DFGeometry::~DFGeometry()
{
   for( auto a = _aabbs.begin(); a != _aabbs.end(); ++a )
   {
      if( *a ) delete (*a);
   }
}

//------------------------------------------------------------------------------
//!
uint32_t DFGeometry::addPatch()
{
   _patches.pushBack( Patch() );
   Patch& p = _patches.back();

   p._flags         = 0;
   p._edges         = 0;
   p._id            = 0;
   p._controlPts[0] = NULL_ID32;
   p._controlPts[1] = NULL_ID32;
   p._controlPts[2] = NULL_ID32;
   p._controlPts[3] = NULL_ID32;
   p._neighbors[0]  = NULL_ID32;
   p._neighbors[1]  = NULL_ID32;
   p._neighbors[2]  = NULL_ID32;
   p._neighbors[3]  = NULL_ID32;
   p._uv[0]         = Vec2f( 0.0f, 0.0f );
   p._uv[1]         = Vec2f( 1.0f, 0.0f );
   p._uv[2]         = Vec2f( 1.0f, 1.0f );
   p._uv[3]         = Vec2f( 0.0f, 1.0f );

   return uint32_t(_patches.size())-1;
}

//------------------------------------------------------------------------------
//!
uint32_t DFGeometry::addPatch( const Patch& p )
{
   _patches.pushBack( p );
   return uint32_t(_patches.size())-1;
}

//------------------------------------------------------------------------------
//!
uint32_t DFGeometry::addPatch( uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3, int creases )
{
   _patches.pushBack( Patch() );
   Patch& p = _patches.back();

   p._flags         = 0;
   p._edges         = (creases&0xf)<<8;
   p._id            = 0;
   p._controlPts[0] = v0;
   p._controlPts[1] = v1;
   p._controlPts[2] = v2;
   p._controlPts[3] = v3;
   p._neighbors[0]  = NULL_ID32;
   p._neighbors[1]  = NULL_ID32;
   p._neighbors[2]  = NULL_ID32;
   p._neighbors[3]  = NULL_ID32;
   p._uv[0]         = Vec2f( 0.0f, 0.0f );
   p._uv[1]         = Vec2f( 1.0f, 0.0f );
   p._uv[2]         = Vec2f( 1.0f, 1.0f );
   p._uv[3]         = Vec2f( 0.0f, 1.0f );

   return uint32_t(_patches.size())-1;
}

//------------------------------------------------------------------------------
//!
uint32_t DFGeometry::addControlPoint( const Vec3f& p )
{
   _controlPts.pushBack( p );
   return uint32_t(_controlPts.size())-1;
}

//------------------------------------------------------------------------------
//!
uint32_t DFGeometry::addTrimming()
{
   _trimmings.pushBack( Trimming() );
   Trimming& t = _trimmings.back();

   t._x        = 0;
   t._y        = 1;
   t._z        = 2;
   t._flags    = 0;
   t._vertices = NULL_ID16;

   return uint32_t(_trimmings.size())-1;
}

//------------------------------------------------------------------------------
//!
uint32_t DFGeometry::addTrimming( const Trimming& t )
{
   _trimmings.pushBack( t );
   return uint32_t(_trimmings.size())-1;
}

//------------------------------------------------------------------------------
//!
void DFGeometry::neighbors( uint32_t pID0, uint e0, uint32_t pID1, uint e1, int crease )
{
   // Set neighbor informations.
   Patch& p0         = _patches[pID0];
   Patch& p1         = _patches[pID1];
   p0._neighbors[e0] = pID1;
   p1._neighbors[e1] = pID0;
   p0._edges         = setbits( p0._edges, e0*2, 2, e1 );
   p1._edges         = setbits( p1._edges, e1*2, 2, e0 );
   p0._edges         = setbits( p0._edges, e0+8, 1, crease );
   p1._edges         = setbits( p1._edges, e1+8, 1, crease );
}

//------------------------------------------------------------------------------
//!
void DFGeometry::computeNeighbors()
{
   Vector<uint32_t> verticesStars( _controlPts.size(), NULL_ID32 );
   Vector<uint32_t> edgesStars( _patches.size() * 4, NULL_ID32 );

   uint32_t edge = 0;
   for( auto p = _patches.begin(); p != _patches.end(); ++p )
   {
      uint32_t v0 = p->_controlPts[0];
      uint32_t v1 = p->_controlPts[1];
      uint32_t v2 = p->_controlPts[2];
      uint32_t v3 = p->_controlPts[3];
      addEdgeNeighbor( v0, v1, edge++, verticesStars, edgesStars );
      addEdgeNeighbor( v1, v2, edge++, verticesStars, edgesStars );
      addEdgeNeighbor( v2, v3, edge++, verticesStars, edgesStars );
      addEdgeNeighbor( v3, v0, edge++, verticesStars, edgesStars );
   }
}

//------------------------------------------------------------------------------
//!
void DFGeometry::addEdgeNeighbor(
   uint32_t          v0,
   uint32_t          v1,
   uint32_t          e,
   Vector<uint32_t>& vs,
   Vector<uint32_t>& es
)
{
   // First edge connect to vertex v0?
   uint32_t firstEdge = vs[v0];
   if( firstEdge == NULL_ID32 )
   {
      vs[v0] = e;
      es[e]  = e;
      return;
   }

   uint32_t p0 = e>>2;
   uint32_t e0 = e&0x3;
   int c       = _patches[p0].crease(e0);

   // Find neighbor edge.
   uint32_t curEdge = firstEdge;
   do
   {
      uint32_t prevEdge = (curEdge&0x3) == 0 ? curEdge+3 : curEdge-1;
      uint32_t p1       = prevEdge>>2;
      uint32_t e1       = prevEdge&0x3;
      if( v1 == _patches[p1]._controlPts[e1] )
      {
         // Connects neighbors edges.
         neighbors( p0, e0, p1, e1, c );
      }
      curEdge = es[curEdge];
   } while( curEdge != firstEdge );

   // Add edge to star.
   es[e]         = es[firstEdge];
   es[firstEdge] = e;
}

/*==============================================================================
   Subdivision
==============================================================================*/

//------------------------------------------------------------------------------
//!
void DFGeometry::subdivide( float error )
{
   for( size_t i = 0; i < _patches.size(); ++i )
   {
      subdivide( _patches[i], error );
   }
}

//------------------------------------------------------------------------------
//!
void DFGeometry::subdivide( Patch& p, float error )
{
   DFPatch pp;
   pp.init( *this, p );

   if( !p._subpatches.empty() ) return;

   // Create first subpatch.
   p._subpatches.pushBack( Subpatch() );
   Subpatch& sp   = p._subpatches.back();
   sp._corners[0] = 0;
   sp._corners[1] = 1;
   sp._corners[2] = 2;
   sp._corners[3] = 3;
   sp._flags      = 0x3c; // non linear edges.
   sp._trimming   = NULL_ID32;

   // Create first subpatch corners.
   _numSubpatches  += 1;
   p._vertices.pushBack( Vertex() );
   p._vertices.pushBack( Vertex() );
   p._vertices.pushBack( Vertex() );
   p._vertices.pushBack( Vertex() );

   p._vertices[0]._uv       = Vec2f( 0.0f, 0.0f );
   p._vertices[0]._next[0]  = NULL_ID16;
   p._vertices[0]._next[1]  = 1;
   p._vertices[0]._next[2]  = 3;
   p._vertices[0]._next[3]  = NULL_ID16;
   p._vertices[0]._flags    = (1<<0)|(1<<3); // Boundary patch vertex.

   p._vertices[1]._uv       = Vec2f( 1.0f, 0.0f );
   p._vertices[1]._next[0]  = NULL_ID16;
   p._vertices[1]._next[1]  = NULL_ID16;
   p._vertices[1]._next[2]  = 2;
   p._vertices[1]._next[3]  = 0;
   p._vertices[1]._flags    = (1<<0)|(1<<1); // Boundary patch vertex.

   p._vertices[2]._uv       = Vec2f( 1.0f, 1.0f );
   p._vertices[2]._next[0]  = 1;
   p._vertices[2]._next[1]  = NULL_ID16;
   p._vertices[2]._next[2]  = NULL_ID16;
   p._vertices[2]._next[3]  = 3;
   p._vertices[2]._flags    = (1<<1)|(1<<2); // Boundary patch vertex.

   p._vertices[3]._uv       = Vec2f( 0.0f, 1.0f );
   p._vertices[3]._next[0]  = 0;
   p._vertices[3]._next[1]  = 2;
   p._vertices[3]._next[2]  = NULL_ID16;
   p._vertices[3]._next[3]  = NULL_ID16;
   p._vertices[3]._flags    = (1<<2)|(1<<3); // Boundary patch vertex.

   // Subpatch positions.
   pp.parameters( p._vertices[0]._uv, p._vertices[0]._pos, p._vertices[0]._n );
   pp.parameters( p._vertices[1]._uv, p._vertices[1]._pos, p._vertices[1]._n );
   pp.parameters( p._vertices[2]._uv, p._vertices[2]._pos, p._vertices[2]._n );
   pp.parameters( p._vertices[3]._uv, p._vertices[3]._pos, p._vertices[3]._n );

   // Subdivide until all subpatches meets the planarity criteria.
   for( size_t i = 0; i < p._subpatches.size(); )
   {
      if( !subdivide( p, uint(i), pp, error ) ) ++i;
   }
}

//------------------------------------------------------------------------------
//!
bool DFGeometry::subdivide( Patch& p, uint spID, const DFPatch& pp, float error )
{
   Subpatch& sp = p._subpatches[spID];

   // Compute 5 new subdivided points (cross section).
   Vertex& v0 = p._vertices[sp._corners[0]];
   Vertex& v1 = p._vertices[sp._corners[1]];
   Vertex& v2 = p._vertices[sp._corners[2]];
   Vertex& v3 = p._vertices[sp._corners[3]];

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

   pp.parameters( uv[0], rpos[0], n[0] );
   pp.parameters( uv[1], rpos[1], n[1] );
   pp.parameters( uv[2], rpos[2], n[2] );
   pp.parameters( uv[3], rpos[3], n[3] );
   pp.parameters( uv[4], rpos[4], n[4] );
   pos[4] = rpos[4];

   // Find if we keep the new subdivided points or approximate by the linear position.
   Vec3f lpos;
   int sub      = 0;
   float error2 = error*error;

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
      if( v2._uv.x-v0._uv.x > 0.0625f ) sub |= 2;
   }

   // Interior 1.
   lpos = (rpos[1]+rpos[3])*0.5f;
   if( (sub&1) == 0 && sqrLength( rpos[4]-lpos ) > error2 )
   {
      if( v2._uv.y-v0._uv.y > 0.0625f ) sub |= 1;
   }

   //if( v2._uv.y-v0._uv.y > 0.0625f ) sub = 1|2|4|8|16|32;

   sp._flags = (sub&0x3c) | (sp._flags&(~0x3c));
   switch( sub & 3 )
   {
      case 0: return false;
      case 1:
         subdivideu( p, spID, pos, n, uv );
         return true;
      case 2:
         subdividev( p, spID, pos, n, uv );
         return true;
      default:
         subdivideuv( p, spID, pos, n, uv );
         return true;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
void DFGeometry::subdivideu( Patch& p, uint spID, const Vec3f pos[], const Vec3f n[], const Vec2f uv[] )
{
   _numSubpatches  += 1;
   p._subpatches.pushBack( Subpatch() );

   Subpatch& sp0   = p._subpatches[spID];
   Subpatch& sp1   = p._subpatches.back();
   sp1._trimming   = NULL_ID32;

   // Add splitting vertices.
   uint16_t nvID0  = subdivideEdge( p, sp0, 0, pos[0], n[0], uv[0] );
   uint16_t nvID2  = subdivideEdge( p, sp0, 2, pos[2], n[2], uv[2] );
   p._vertices[nvID0]._next[2] = nvID2;
   p._vertices[nvID2]._next[0] = nvID0;

   // Set edges flag.
   sp1._flags      = sp0._flags | 0x20;
   sp0._flags     |= 0x08;

   // Set subpatches vertices.
   uint16_t vID1   = sp0._corners[1];
   uint16_t vID2   = sp0._corners[2];
   sp0._corners[1] = nvID0;
   sp0._corners[2] = nvID2;
   sp1._corners[0] = nvID0;
   sp1._corners[1] = vID1;
   sp1._corners[2] = vID2;
   sp1._corners[3] = nvID2;

}
// TODO: verify edge flags...
//------------------------------------------------------------------------------
//!
void DFGeometry::subdividev( Patch& p, uint spID, const Vec3f pos[], const Vec3f n[], const Vec2f uv[] )
{
   _numSubpatches  += 1;
   p._subpatches.pushBack( Subpatch() );

   Subpatch& sp0   = p._subpatches[spID];
   Subpatch& sp1   = p._subpatches.back();
   sp1._trimming   = NULL_ID32;

   // Add splitting vertices.
   uint16_t nvID1  = subdivideEdge( p, sp0, 1, pos[1], n[1], uv[1] );
   uint16_t nvID3  = subdivideEdge( p, sp0, 3, pos[3], n[3], uv[3] );
   p._vertices[nvID1]._next[3] = nvID3;
   p._vertices[nvID3]._next[1] = nvID1;

   // Set edges flag.
   sp1._flags      = sp0._flags | 0x04;
   sp0._flags     |= 0x10;

   // Set subpatches vertices.
   uint16_t vID2   = sp0._corners[2];
   uint16_t vID3   = sp0._corners[3];
   sp0._corners[2] = nvID1;
   sp0._corners[3] = nvID3;
   sp1._corners[0] = nvID3;
   sp1._corners[1] = nvID1;
   sp1._corners[2] = vID2;
   sp1._corners[3] = vID3;
}

//------------------------------------------------------------------------------
//!
void DFGeometry::subdivideuv( Patch& p, uint spID, const Vec3f pos[], const Vec3f n[], const Vec2f uv[] )
{
   _numSubpatches += 3;
   size_t id       = p._subpatches.size();

   // Add new subpatches.
   p._subpatches.pushBack( Subpatch() );
   p._subpatches.pushBack( Subpatch() );
   p._subpatches.pushBack( Subpatch() );

   Subpatch& sp0   = p._subpatches[spID];
   Subpatch& sp1   = p._subpatches[id];
   Subpatch& sp2   = p._subpatches[id+1];
   Subpatch& sp3   = p._subpatches[id+2];

   sp1._trimming   = NULL_ID32;
   sp2._trimming   = NULL_ID32;
   sp3._trimming   = NULL_ID32;

   // Add splitting vertices.
   uint16_t nvID0  = subdivideEdge( p, sp0, 0, pos[0], n[0], uv[0] );
   uint16_t nvID1  = subdivideEdge( p, sp0, 1, pos[1], n[1], uv[1] );
   uint16_t nvID2  = subdivideEdge( p, sp0, 2, pos[2], n[2], uv[2] );
   uint16_t nvID3  = subdivideEdge( p, sp0, 3, pos[3], n[3], uv[3] );

   // Add center vertex.
   uint16_t nvID4  = uint16_t(p._vertices.size());
   p._vertices.pushBack( Vertex() );
   Vertex& nv4     = p._vertices[nvID4];
   nv4._uv         = uv[4];
   nv4._flags      = 0;
   nv4._next[0]    = nvID0;
   nv4._next[1]    = nvID1;
   nv4._next[2]    = nvID2;
   nv4._next[3]    = nvID3;
   nv4._pos        = pos[4];
   nv4._n          = n[4];

   uint16_t vID1   = sp0._corners[1];
   uint16_t vID2   = sp0._corners[2];
   uint16_t vID3   = sp0._corners[3];

   // Set link to center vertex.
   p._vertices[nvID0]._next[2] = nvID4;
   p._vertices[nvID1]._next[3] = nvID4;
   p._vertices[nvID2]._next[0] = nvID4;
   p._vertices[nvID3]._next[1] = nvID4;

   // Set subpatches vertices.
   sp0._corners[1] = nvID0;
   sp0._corners[2] = nvID4;
   sp0._corners[3] = nvID3;

   sp1._corners[0] = nvID0;
   sp1._corners[1] = vID1;
   sp1._corners[2] = nvID1;
   sp1._corners[3] = nvID4;
   sp1._flags      = sp0._flags | 0x30;

   sp2._corners[0] = nvID4;
   sp2._corners[1] = nvID1;
   sp2._corners[2] = vID2;
   sp2._corners[3] = nvID2;
   sp2._flags      = sp0._flags | 0x24;

   sp3._corners[0] = nvID3;
   sp3._corners[1] = nvID4;
   sp3._corners[2] = nvID2;
   sp3._corners[3] = vID3;
   sp3._flags      = sp0._flags | 0x0c;

   sp0._flags     |= 0x18;
}

//------------------------------------------------------------------------------
//!
uint16_t
DFGeometry::subdivideEdge(
   Patch&       p,
   Subpatch&    sp,
   uint         edge,
   const Vec3f& pos,
   const Vec3f& n,
   const Vec2f& uv
)
{
   uint axis = edge&1;

   // Search vertex.
   const float epsilon = (1.0f/65536.0f);
   uint16_t pv         = 0;
   switch( edge )
   {
      case 0: pv = prevVertex( p, sp._corners[0], sp._corners[1], 0, uv.x+epsilon ); break;
      case 1: pv = prevVertex( p, sp._corners[1], sp._corners[2], 1, uv.y+epsilon ); break;
      case 2: pv = prevVertex( p, sp._corners[3], sp._corners[2], 0, uv.x+epsilon ); break;
      case 3: pv = prevVertex( p, sp._corners[0], sp._corners[3], 1, uv.y+epsilon ); break;
   }

   // Vertex found?
   if( CGM::equal( p._vertices[pv]._uv(axis), uv(axis), epsilon ) ) return pv;

   return insertEdgeVertex( p, pv, axis+1, pos, n, uv );
}

//------------------------------------------------------------------------------
//!
uint16_t DFGeometry::insertEdgeVertex(
   Patch&       p,
   uint16_t     pvID,
   int          dir,
   const Vec3f& pos,
   const Vec3f& n,
   const Vec2f& uv
)
{
   // Add new vertex.
   uint16_t vID  = uint16_t(p._vertices.size());
   p._vertices.pushBack( Vertex() );
   Vertex& v     = p._vertices[vID];

   // Find previous and next vertices.
   Vertex& pv    = p._vertices[pvID];
   uint16_t nvID = pv._next[dir];
   Vertex& nv    = p._vertices[nvID];

   // Set new vertex info.
   v._pos              = pos;
   v._n                = n;
   v._uv               = uv;
   v._flags            = pv._flags & nv._flags;
   //v._id             = INVALID;

   int idir            = (dir+2)%4;
   v._next[idir]       = pvID;
   v._next[dir]        = nvID;
   v._next[(dir+1)%4]  = NULL_ID16;
   v._next[(idir+1)%4] = NULL_ID16;

   pv._next[dir]       = vID;
   nv._next[idir]      = vID;

   return vID;
}

//------------------------------------------------------------------------------
//!
uint16_t
DFGeometry::insertEdgeVertex( Patch& p, Subpatch& sp, uint edge, const Vec3f& pos, float t )
{
   uint ne    = (edge+1)%4;
   uint axis  = edge&1;

   Vertex& c0 = p._vertices[sp._corners[edge]];
   Vertex& c1 = p._vertices[sp._corners[ne]];

   Vec2f uv   = CGM::linear2( c0._uv, c1._uv, t );
   Vec3f n    = normalize( CGM::linear2( c0._n, c1._n, t ) );

   // Search vertex.
   uint16_t pv = 0;
   switch( edge )
   {
      case 0: pv = prevVertex( p, sp._corners[0], sp._corners[1], 0, uv.x ); break;
      case 1: pv = prevVertex( p, sp._corners[1], sp._corners[2], 1, uv.y ); break;
      case 2: pv = prevVertex( p, sp._corners[3], sp._corners[2], 0, uv.x ); break;
      case 3: pv = prevVertex( p, sp._corners[0], sp._corners[3], 1, uv.y ); break;
   }

   // Vertex found?
   Vertex& v = p._vertices[pv];
   if( equal( v._pos, pos, _errormax ) ) return pv;
   if( equal( p._vertices[v._next[axis+1]]._pos, pos, _errormax ) ) return v._next[axis+1];

   return insertEdgeVertex( p, pv, axis+1, pos, n, uv );
}

//------------------------------------------------------------------------------
//!
uint16_t
DFGeometry::insertFaceVertex( Patch& p, Trimming& trim, const Vec3f& pos, const Vec3f& n, const Vec2f& uv )
{
   // Rounding precisions.
   const float error2 = _errormax*_errormax;

   // Find point in trimming info.
   for( uint16_t cv = trim._vertices; cv != NULL_ID16; cv = p._vertices[cv]._next[0] )
      if( sqrLength( pos-p._vertices[cv]._pos ) < error2 ) return cv;

   // Vertex non-existant, we need to create it.
   uint16_t vID  = uint16_t(p._vertices.size());
   p._vertices.pushBack( Vertex() );
   Vertex& v     = p._vertices[vID];

   v._pos     = pos;
   v._n       = n;
   v._uv      = uv;
   v._flags   = 0x10;
   //v->_id      = INVALID;
   v._next[0] = trim._vertices;
   v._next[1] = NULL_ID16;
   v._next[2] = NULL_ID16;
   v._next[3] = NULL_ID16;

   trim._vertices = vID;

   return vID;
}

/*==============================================================================
   Geometry
==============================================================================*/

//------------------------------------------------------------------------------
//!
bool materialSort( DFGeometry::Patch* a, DFGeometry::Patch* b )
{
   return a->_id < b->_id;
}

//------------------------------------------------------------------------------
//!
void DFGeometry::updateMesh()
{
   // Compute mesh.
   _vertices.clear();
   _indices.clear();
   _bbox = AABBoxf::empty();
   Geometry::clearPatches();

   // Nothing to build?
   if( numPatches() == 0 ) return;

   Grid<Vertex*> grid( numSubpatches()*2, 1.0f/128.0f );

   // Regroup and sort patches by material ID.
   Vector<Patch*> patches;
   patches.reserve( numPatches() );
   for( auto cur = _patches.begin(); cur != _patches.end(); ++cur )
   {
      patches.pushBack( cur );
   }
   std::sort( patches.begin(), patches.end(), materialSort );

   // Triangulate by material ID.
   uint32_t id = patches[0]->_id;
   uint idx    = 0;
   for( auto cur = patches.begin(); cur != patches.end(); ++cur )
   {
      uint32_t cid = (*cur)->_id;
      if( cid != id )
      {
         Geometry::addPatch( idx, uint(_indices.size())-idx, id );
         idx = uint(_indices.size());
         id  = cid;
      }
      triangulate( **cur, grid, _vertices, _indices, nullptr, true );
   }
   Geometry::addPatch( idx, uint(_indices.size())-idx, id );

   // Compute bb.
   for( size_t i = 0; i < _vertices.size(); i+=3 )
   {
      _bbox |= Vec3f( &_vertices[i] );
   }
}

//------------------------------------------------------------------------------
//!
RCP<MeshGeometry>
DFGeometry::createMesh()
{
   updateMesh();
   RCP<MeshGeometry> mesh = new MeshGeometry();

   const int attribs[] = {
      MeshGeometry::POSITION,
      MeshGeometry::MAPPING,
      MeshGeometry::NORMAL,
      0
   };
   for( uint i = 0; i < Geometry::numPatches(); ++i )
   {
      const PatchInfo& pi = patchInfo(i);
      mesh->addPatch( pi.rangeStart(), pi.rangeSize(), pi.materialID() );
   }
   //mesh->addPatch( 0, uint(_indices.size()), 0 );
   mesh->collisionType( _collisionType );
   mesh->allocateIndices( uint(_indices.size()) );
   mesh->copyIndices( _indices.data() );
   mesh->setAttributes( attribs );
   mesh->allocateVertices( uint(_vertices.size())/8 );
   mesh->copyAttributes( _vertices.data(), 8, 8, 0 );
   mesh->updateProperties();

   return mesh;
}

//------------------------------------------------------------------------------
//!
void DFGeometry::computeRenderableGeometry()
{
   StdErr << "computing renderable geometry...\n";

   // Transfer to Gfx.
   _rgeom = Core::gfx()->createGeometry( Gfx::PRIM_TRIANGLES );
   RCP<Gfx::VertexBuffer> vbuffer = Core::gfx()->createBuffer(
      Gfx::BUFFER_FLAGS_NONE, _vertices.dataSize(), _vertices.data()
   );
   vbuffer->addAttribute( Gfx::ATTRIB_TYPE_POSITION,  Gfx::ATTRIB_FMT_32F_32F_32F, 0 );
   vbuffer->addAttribute( Gfx::ATTRIB_TYPE_TEXCOORD0, Gfx::ATTRIB_FMT_32F_32F, 12 );
   vbuffer->addAttribute( Gfx::ATTRIB_TYPE_NORMAL,    Gfx::ATTRIB_FMT_32F_32F_32F, 20 );
   _rgeom->addBuffer( vbuffer );

   if( _vertices.size() < (1<<8) )
   {
      // Recompact into 8b indices.
      Vector<uint8_t> indices8( _indices.size() );
      for( size_t i = 0; i < _indices.size(); ++i )
      {
         indices8[i] = _indices[i];
      }
      _rgeom->indexBuffer(
         Core::gfx()->createBuffer(
            Gfx::INDEX_FMT_8, Gfx::BUFFER_FLAGS_NONE, indices8.dataSize(), indices8.data()
         )
      );
   }
   else
   if( _vertices.size() < (1<<16) )
   {
      // Recompact into 16b indices.
      Vector<uint16_t> indices16( _indices.size() );
      for( size_t i = 0; i < _indices.size(); ++i )
      {
         indices16[i] = _indices[i];
      }
      _rgeom->indexBuffer(
         Core::gfx()->createBuffer(
            Gfx::INDEX_FMT_16, Gfx::BUFFER_FLAGS_NONE, indices16.dataSize(), indices16.data()
         )
      );
   }
   else
   {
      _rgeom->indexBuffer(
         Core::gfx()->createBuffer(
            Gfx::INDEX_FMT_32, Gfx::BUFFER_FLAGS_NONE, _indices.dataSize(), _indices.data()
         )
      );
   }
}

//------------------------------------------------------------------------------
//!
void
DFGeometry::triangulate(
   Patch&         p,
   Grid<Vertex*>& grid,
   Vector<float>& vertices,
   Vector<uint>&  indices,
   Vector<uint>*  faceInfos,
   bool           normals
) const
{
   // Do not need to be triangulate?
   if( p.isHidden() ) return;

   uint stride = normals ? 8 : 5;
   uint offset = uint(vertices.size()) / stride;
   uint id     = 0;
   bool flip   = p.isFlipped();
   float nf    = flip ? -1.0f : 1.0f;
   float error = 2.0f*_errormax;

   uint startIndice = uint(indices.size());
   uint nbVertices[4];

   // Reset all vertices ID.
   for( auto it = p._vertices.begin(); it != p._vertices.end() ; ++it )
   {
      it->_id = NULL_ID32;
   }

   Vector<Vertex*> vptrs;

   // Compute patch uvs.
   Vec2f uv0  = p._uv[0];
   Vec2f uv1  = p._uv[3];
   Vec2f duv0 = p._uv[1] - uv0;
   Vec2f duv1 = p._uv[2] - uv1;

   Vec3f n;
   Vec3f tp;

   // Add triangles.
   for( auto sIt = p._subpatches.begin(); sIt != p._subpatches.end(); ++sIt )
   {
      // Do not need to be triangulate?
      if( sIt->isHidden() ) continue;

      // Normal triangulation?
      if( !sIt->isTrimmed()  || _trimmings[sIt->_trimming]._loops[0] == 0 )
      {
         // Accumulate all patch vertices in CCW and compute sides size.
         vptrs.clear();
         uint16_t vID = sIt->_corners[0];

         for( uint c = 0; c < 4; ++c )
         {
            nbVertices[c] = 0;
            for( ; vID != sIt->_corners[(c+1)%4]; vID = p._vertices[vID]._next[(c+1)%4] )
            {
               Vertex& v = p._vertices[vID];
#ifdef PER_SUBPATCH_MAPPING
               // Add a new vertex.
               {
                  v._id = offset + id++;
                  vertices.pushBack( v._pos.x );
                  vertices.pushBack( v._pos.y );
                  vertices.pushBack( v._pos.z );
                  Vec2f uv = (v._uv-p._vertices[sIt->_corners[0]]._uv)/(p._vertices[sIt->_corners[2]]._uv-p._vertices[sIt->_corners[0]]._uv);
                  vertices.pushBack( uv.x );
                  vertices.pushBack( uv.y );
                  if( normals )
                  {
                     n = v._n;
                     vertices.pushBack( nf*n.x );
                     vertices.pushBack( nf*n.y );
                     vertices.pushBack( nf*n.z );
                  }
               }
#else
               // Add a new vertex.
               if( v._id == NULL_ID32 )
               {
                  Vec2f uv = CGM::bilinear( uv0, duv0, uv1, duv1, v._uv.x, v._uv.y );
                  if( normals ) n = v._n*nf;

                  // Search for vertices.
                  Vec3i c0 = grid.cellCoord( v._pos-error );
                  Vec3i c1 = grid.cellCoord( v._pos+error );
                  for( int x = c0.x; x <= c1.x ; ++x )
                  {
                     for( int y = c0.y; y <= c1.y; ++y )
                     {
                        for( int z = c0.z; z <= c1.z; ++z )
                        {
                           auto l = grid.cell( Vec3i(x,y,z) );
                           for( ; l; l = l->_next )
                           {
                              // Have we found the vertex?
                              if( sqrLength(l->_obj->_pos-v._pos) < (error*error) )
                              {
                                 // Test for vertex similarity.
                                 if( normals )
                                 {
                                    Vec2f& cuv = (Vec2f&)vertices[l->_obj->_id*stride+3];
                                    Vec3f& cn  = (Vec3f&)vertices[l->_obj->_id*stride+5];
                                    if( equal( cuv, uv ) && equal( cn, n ) ) v._id = l->_obj->_id;
                                 }
                                 else
                                 {
                                    Vec2f& cuv = (Vec2f&)vertices[l->_obj->_id*stride+3];
                                    if( equal( cuv, uv ) ) v._id = l->_obj->_id;
                                 }
                              }
                           }
                        }
                     }
                  }

                  if( v._id == NULL_ID32 )
                  {
                     v._id = offset + id++;
                     vertices.pushBack( v._pos.x );
                     vertices.pushBack( v._pos.y );
                     vertices.pushBack( v._pos.z );
                     vertices.pushBack( uv.x );
                     vertices.pushBack( uv.y );
                     if( normals )
                     {
                        vertices.pushBack( n.x );
                        vertices.pushBack( n.y );
                        vertices.pushBack( n.z );
                     }
                     grid.add( grid.cellID( v._pos ), &v );
                  }
               }
#endif
               vptrs.pushBack( &v );
               nbVertices[c]++;
            }
         }
         ::triangulate( vptrs, nbVertices, flip, indices );
      }
      else
      {
         // Trimmed subpatch triangulation.
         const Trimming& trim = _trimmings[sIt->_trimming];

         int stLoop = 1;
         for( int l = 0; l < trim._loops[0]; ++l, stLoop += trim._loops[stLoop]+2 )
         {
            int endLoop = stLoop + trim._loops[stLoop]+2;
            // Do not need to be triangulate?
            if( trim.isHidden( stLoop+1 ) ) continue;
            vptrs.clear();
            for( int i = stLoop+2 ; i < endLoop; ++i )
            {
               Vertex& v = p._vertices[trim._loops[i]];
#ifdef PER_SUBPATCH_MAPPING
               // Add a new vertex.
               {
                  v._id = offset + id++;
                  vertices.pushBack( v._pos.x );
                  vertices.pushBack( v._pos.y );
                  vertices.pushBack( v._pos.z );
                  Vec2f uv = (v._uv-p._vertices[sIt->_corners[0]]._uv)/(p._vertices[sIt->_corners[2]]._uv-p._vertices[sIt->_corners[0]]._uv);
                  vertices.pushBack( uv.x );
                  vertices.pushBack( uv.y );
                  if( normals )
                  {
                     n = v._n;
                     vertices.pushBack( nf*n.x );
                     vertices.pushBack( nf*n.y );
                     vertices.pushBack( nf*n.z );
                  }
               }
#else
               // Add a new vertex.
               if( v._id == NULL_ID32 )
               {
                  Vec2f uv = CGM::bilinear( uv0, duv0, uv1, duv1, v._uv.x, v._uv.y );
                  if( normals ) n = v._n*nf;

                  // Search for vertices.
                  Vec3i c0 = grid.cellCoord( v._pos-error );
                  Vec3i c1 = grid.cellCoord( v._pos+error );
                  for( int x = c0.x; x <= c1.x ; ++x )
                  {
                     for( int y = c0.y; y <= c1.y; ++y )
                     {
                        for( int z = c0.z; z <= c1.z; ++z )
                        {
                           auto l = grid.cell( Vec3i(x,y,z) );
                           for( ; l; l = l->_next )
                           {
                              // Have we found the vertex?
                              if( sqrLength( l->_obj->_pos-v._pos) < (error*error) )
                              {
                                 // Test for vertex similarity.
                                 if( normals )
                                 {
                                    Vec2f& cuv = (Vec2f&)vertices[l->_obj->_id*stride+3];
                                    Vec3f& cn  = (Vec3f&)vertices[l->_obj->_id*stride+5];
                                    if( equal( cuv, uv ) && equal( cn, n ) ) v._id = l->_obj->_id;
                                 }
                                 else
                                 {
                                    Vec2f& cuv = (Vec2f&)vertices[l->_obj->_id*stride+3];
                                    if( equal( cuv, uv ) ) v._id = l->_obj->_id;
                                 }
                              }
                           }
                        }
                     }
                  }

                  if( v._id == NULL_ID32 )
                  {
                     v._id = offset + id++;
                     vertices.pushBack( v._pos.x );
                     vertices.pushBack( v._pos.y );
                     vertices.pushBack( v._pos.z );
                     vertices.pushBack( uv.x );
                     vertices.pushBack( uv.y );
                     if( normals )
                     {
                        vertices.pushBack( n.x );
                        vertices.pushBack( n.y );
                        vertices.pushBack( n.z );
                     }
                     grid.add( grid.cellID( v._pos ), &v );
                  }
               }
#endif
               vptrs.pushBack( &v );
            }
            ::triangulate( vptrs, indices, flip, trim._x, trim._y );
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
      }
   }
}

/*==============================================================================
   Operations
==============================================================================*/

//------------------------------------------------------------------------------
//!
RCP<DFGeometry> DFGeometry::clone() const
{
   RCP<DFGeometry> geom( new DFGeometry() );
   // Geometry.
   geom->_collisionType = _collisionType;
   geom->_com           = _com;
   geom->_inertiaTensor = _inertiaTensor;
   // DFGeometry.
   geom->_numSubpatches = _numSubpatches;
   geom->_patches       = _patches;
   geom->_controlPts    = _controlPts;
   geom->_trimmings     = _trimmings;
   return geom;
}

//------------------------------------------------------------------------------
//!
RCP<DFGeometry> DFGeometry::transform( const Reff& r ) const
{
   RCP<DFGeometry> geom = clone();

   Mat4f m  = r.toMatrix();
   Mat3f m3 = r.orientation().toMatrix3();

   // Transform controlPts.
   for( auto c = geom->_controlPts.begin(); c != geom->_controlPts.end(); ++c )
   {
      (*c) = m * (*c);
   }
   // Transform vertices inside patches.
   for( auto p = geom->_patches.begin(); p != geom->_patches.end(); ++p )
   {
      for( auto v = (*p)._vertices.begin(); v != (*p)._vertices.end(); ++v )
      {
         (*v)._pos = m  * (*v)._pos;
         (*v)._n   = m3 * (*v)._n;
      }
   }
   // Transform trimming info.
   for( auto t = geom->_trimmings.begin(); t != geom->_trimmings.end(); ++t )
   {
      // TODO: compute _x, _y, _z, _plane0 & _plane1.
   }
   return geom;
}

//------------------------------------------------------------------------------
//!
void DFGeometry::merge( DFGeometry* geom )
{
   mergeAndClips( geom, VOL_OUT | B_IN, VOL_OUT );
}

//------------------------------------------------------------------------------
//!
void DFGeometry::subtract( DFGeometry* geom )
{
   if( geom == nullptr ) return;
   uint32_t newPatchID = uint32_t(_patches.size());
   mergeAndClips( geom, VOL_OUT, VOL_IN | B_OUT );

   for( auto p = _patches.begin()+newPatchID; p != _patches.end(); ++p )
   {
      p->flip( !p->isFlipped() );
   }
}

//------------------------------------------------------------------------------
//!
void DFGeometry::intersect( DFGeometry* geom )
{
   mergeAndClips( geom, VOL_IN | B_IN, VOL_IN );
}

//------------------------------------------------------------------------------
//! Boolean operations belonging for A & B.
//! Union: A in(h), out(k), in border(k), out border(h)
//! Union: B in(h), out(k), in border(h), out border(h)
//! Difference: A in(h), out(k), in border(h), out border(h)
//! Difference: B in(k), out(h), in border(h), out border(k)
//! Intersection: A in(k), out(h), in border(k), out border(h)
//! Intersection: B in(k), out(h), in border(h), out border(h)
void DFGeometry::mergeAndClips( DFGeometry* geom, int maskA, int maskB )
{
   if( geom == nullptr ) return;
   // 1. Add geometry to this one.
   uint32_t offsetID   = uint32_t(_controlPts.size());
   uint32_t newPatchID = uint32_t(_patches.size());
   uint32_t newTrimID  = uint32_t(_trimmings.size());
   _numSubpatches   += geom->_numSubpatches;
   _controlPts.append( geom->_controlPts );
   _patches.append( geom->_patches );
   _trimmings.append( geom->_trimmings );

   // Update IDs.
   for( auto p = _patches.begin()+newPatchID; p != _patches.end(); ++p )
   {
      (*p)._controlPts[0] += offsetID;
      (*p)._controlPts[1] += offsetID;
      (*p)._controlPts[2] += offsetID;
      (*p)._controlPts[3] += offsetID;
      (*p)._neighbors[0]  += newPatchID;
      (*p)._neighbors[1]  += newPatchID;
      (*p)._neighbors[2]  += newPatchID;
      (*p)._neighbors[3]  += newPatchID;

      if( p->isTrimmed() )
      {
         for( auto s = p->_subpatches.begin(); s != p->_subpatches.end(); ++s )
         {
            if( s->isTrimmed() ) s->_trimming += newTrimID;
         }
      }
   }

   // 2. Clip geometry.
   // 2.1 Construct acceleration structure.
   HGrid grid;
   for( size_t pID = 0; pID < newPatchID; ++pID )
   {
      Patch& p = _patches[pID];
      // Add patch to the hierarchical grid.
      if( !p.isHidden() ) grid.add( (void*)pID, computeAABB(p) );
   }
   _aabbs.resize( _patches.size(), nullptr );


   // 2.2 Test all new patches for intersections.
   for( size_t pID = newPatchID; pID < _patches.size(); ++pID )
   {
      Patch& p = _patches[pID];
      if( !p.isHidden() ) grid.findCollisions( this, (void*)pID, computeAABB(p), patchesOverlap );
   }

   // 3 Check which patches/subpatches and loops belong to the final mesh and
   // compute the loops.
   // 3.1 Compute loops for new geometry (B).
   updateLoops( newPatchID, _patches.size() );

   // 3.2 Classify new geometry (B) against old one (A).
   classify( newPatchID, _patches.size(), maskB, this, grid );

   // 3.3 Construct loops for current geometry (A).
   updateLoops( 0, newPatchID );

   // 3.4 Classify current geometry (A) against new (B).
   // 3.4.1 Construct acceleration structure.
   HGrid gridB;
   for( size_t pID = 0; pID < geom->numPatches(); ++pID )
   {
      const Patch& p = geom->patch(uint(pID));
      // Add patch to the hierarchical grid.
      if( !p.isHidden() ) gridB.add( (void*)pID, computeAABB(p) );
   }
   // 3.4.2 Classify.
   classify( 0, newPatchID, maskA, geom, gridB );
}

//------------------------------------------------------------------------------
//!
void DFGeometry::getSurfacePoint( const Patch& p, const Subpatch& sp, Vec3f& pos, Vec3f& n )
{
   // Consider subpatch to be a quad of 2 triangles.
   const Vertex& c0 = p._vertices[sp._corners[0]];
   const Vertex& c1 = p._vertices[sp._corners[1]];
   const Vertex& c2 = p._vertices[sp._corners[2]];
   const Vertex& c3 = p._vertices[sp._corners[3]];

   // 0: triangles (0,1,2),(0,2,3).
   // 1: triangles (0,1,3),(1,2,3).
   Vec3f ab  = c1._pos - c0._pos;
   Vec3f ac  = c2._pos - c0._pos;
   Vec3f ad  = c3._pos - c0._pos;
   Vec3f n0  = cross(ab,ac);
   Vec3f n1  = cross(ac,ad);

   if( dot(n0,n1) > 0.0f )
   {
      pos = (c0._pos + c2._pos)*0.5f;
      n   = normalize((c0._n + c2._n)*0.5f);
   }
   else
   {
      pos = (c1._pos + c3._pos)*0.5f;
      n   = normalize((c1._n + c3._n)*0.5f);
   }
}

//------------------------------------------------------------------------------
//!
void DFGeometry::getLoopPoint(
   const Patch& p,
   uint16_t*    vertices,
   uint         numV,
   uint         x,
   uint         y,
   Vec3f&       pos,
   Vec3f&       n
)
{
   // TODO: If we have problem with the position of the vertex we can project it
   // onto one of the 2 triangles approximating the sp.
   // MUCH Better: add an edge crossing the sp to correspond correctly to the 3D
   // geometry used to compute the intersection.

   // Invalid loop!
   if( numV < 3 )
   {
      pos = p._vertices[vertices[0]]._pos;
      n   = p._vertices[vertices[0]]._n;
      return;
   }

   // A simple triangle.
   if( numV == 3 )
   {
      pos = p._vertices[vertices[0]]._pos*0.25f +
            p._vertices[vertices[1]]._pos*0.25f +
            p._vertices[vertices[2]]._pos*0.5f;
      n   = p._vertices[vertices[0]]._n*0.25f +
            p._vertices[vertices[1]]._n*0.25f +
            p._vertices[vertices[2]]._n*0.5f;
      n.normalize();
      return;
   }
   computePoint( p, vertices, numV, x, y, pos, n );
}

//------------------------------------------------------------------------------
//!
bool DFGeometry::pointInLoop(
   const Patch& p,
   uint16_t*    vertices,
   uint         numV,
   uint         x,
   uint         y,
   const Vec3f& pt
)
{
   bool in = false;
   for( uint vID0 = numV-1, vID1 = 0; vID1 < numV; vID0 = vID1++ )
   {
      const Vec3f& v0 = p._vertices[vertices[vID0]]._pos;
      const Vec3f& v1 = p._vertices[vertices[vID1]]._pos;
      if( ((v0(y) <= pt(y)) && (v1(y) > pt(y))) || ((v0(y) >  pt(y)) && (v1(y) <= pt(y))) )
      {
         float vt = (pt(y)-v0(y)) / (v1(y)-v0(y));
         if( pt(x) < v0(x) + vt * (v1(x)-v0(x)) ) in = !in;
      }
   }
   return in;
}


/*==============================================================================
   Trimming
==============================================================================*/

//------------------------------------------------------------------------------
//!
void DFGeometry::trimPatches( uint32_t p0, uint32_t p1 )
{
   AABBTree* tree0 = _aabbs[p0];
   AABBTree* tree1 = _aabbs[p1];

   // 1. building AABBtree.
   Vector<void*> ids;
   Vector<AABBoxf> bboxes;

   // Tree for patch p0.
   if( !tree0 && _patches[p0]._subpatches.size() > 1 )
   {
      Patch& p = _patches[p0];
      ids.resize( p._subpatches.size() );
      bboxes.resize( p._subpatches.size() );
      const uint16_t n = uint16_t(p._subpatches.size());
      for( uint16_t s = 0; s < n; ++s )
      {
         ids[s]    = (void*)s;
         bboxes[s] = computeAABB( p, s );
      }

      tree0 = new AABBTree( _aabbPool );
      tree0->create( bboxes.data(), ids.data(), uint(ids.size()) );
      _aabbs[p0] = tree0;
   }

   // Tree for patch p1.
   if( !tree1 && _patches[p1]._subpatches.size() > 1 )
   {
      Patch& p = _patches[p1];
      ids.resize( p._subpatches.size() );
      bboxes.resize( p._subpatches.size() );
      const uint16_t n = uint16_t(p._subpatches.size());
      for( uint16_t s = 0; s < n; ++s )
      {
         ids[s]    = (void*)s;
         bboxes[s] = computeAABB( p, s );
      }

      tree1 = new AABBTree( _aabbPool );
      tree1->create( bboxes.data(), ids.data(), uint(ids.size()) );
      _aabbs[p1] = tree1;
   }


   // 2. Test intersecting subpatches.
   Overlap ov;
   ov._geom = this;
   ov._p0   = p0;
   ov._p1   = p1;
   if( tree0 )
   {
      if( tree1 )
      {
         // Tree-Tree intersection.
         tree0->findCollisions( &ov, *tree1, subpatchesOverlap );
      }
      else
      {
         // Tree-box intersecgion.
         AABBoxf box = computeAABB( _patches[p1], 0 );
         tree0->findCollisions( &ov, 0, box, subpatchesOverlap );
      }
   }
   else
   {
      if( tree1 )
      {
         // Box-Tree intersection.
         AABBoxf box = computeAABB( _patches[p0], 0 );
         ov._p0 = p1;
         ov._p1 = p0;
         tree1->findCollisions( &ov, 0, box, subpatchesOverlap );
      }
      else
      {
         // Subpatch-subpatch intersection.
         subpatchesOverlap( &ov, 0, 0 );
      }
   }
}

//------------------------------------------------------------------------------
//!
void DFGeometry::trimSubpatches( uint32_t p0, uint16_t sp0, uint32_t p1, uint16_t sp1 )
{
   // Rounding precisions.
   const float error2 = _errormax*_errormax;

   // Retrieve intersection info for subpatch A & B.
   Patch& pA     = _patches[p0];
   Patch& pB     = _patches[p1];
   Subpatch& spA = pA._subpatches[sp0];
   Subpatch& spB = pB._subpatches[sp1];

   // Test if subpatches are visible.
   if( spA.isHidden() || spB.isHidden() ) return;

   uint trSp0 = getTrimming( _patches[p0], sp0 );
   uint trSp1 = getTrimming( _patches[p1], sp1 );
   Trimming& trimA = _trimmings[trSp0];
   Trimming& trimB = _trimmings[trSp1];

   Planef planeA0 = trimA._plane0;
   Planef planeA1 = trimA._plane1;
   Planef planeB0 = trimB._plane0;
   Planef planeB1 = trimB._plane1;

   // Compute distance from plane 0 of subpatch A.
   // B0-A0 && B1-A0.
   float db0[4];
   db0[0] = planeA0.evaluate( pB._vertices[spB._corners[0]]._pos );
   db0[1] = planeA0.evaluate( pB._vertices[spB._corners[1]]._pos );
   db0[2] = planeA0.evaluate( pB._vertices[spB._corners[2]]._pos );
   db0[3] = planeA0.evaluate( pB._vertices[spB._corners[3]]._pos );

   // Coplanarity robustness snapping.
   if( db0[0]*db0[0] < error2 ) db0[0] = 0.0f;
   if( db0[1]*db0[1] < error2 ) db0[1] = 0.0f;
   if( db0[2]*db0[2] < error2 ) db0[2] = 0.0f;
   if( db0[3]*db0[3] < error2 ) db0[3] = 0.0f;

   // Compute distance from plane 1 of subpatch A.
   // B0-A1 && B1-A1
   float db1[4];
   db1[0] = planeA1.evaluate( pB._vertices[spB._corners[0]]._pos );
   db1[1] = planeA1.evaluate( pB._vertices[spB._corners[1]]._pos );
   db1[2] = planeA1.evaluate( pB._vertices[spB._corners[2]]._pos );
   db1[3] = planeA1.evaluate( pB._vertices[spB._corners[3]]._pos );

   // Coplanarity robustness snapping.
   if( db1[0]*db1[0] < error2 ) db1[0] = 0.0f;
   if( db1[1]*db1[1] < error2 ) db1[1] = 0.0f;
   if( db1[2]*db1[2] < error2 ) db1[2] = 0.0f;
   if( db1[3]*db1[3] < error2 ) db1[3] = 0.0f;

   // Compute distance from plane 0 of subpatch B.
   // A0-B0 && A1-B0.
   float da0[4];
   da0[0] = planeB0.evaluate( pA._vertices[spA._corners[0]]._pos );
   da0[1] = planeB0.evaluate( pA._vertices[spA._corners[1]]._pos );
   da0[2] = planeB0.evaluate( pA._vertices[spA._corners[2]]._pos );
   da0[3] = planeB0.evaluate( pA._vertices[spA._corners[3]]._pos );

   // Coplanarity robustness snapping.
   if( da0[0]*da0[0] < error2 ) da0[0] = 0.0f;
   if( da0[1]*da0[1] < error2 ) da0[1] = 0.0f;
   if( da0[2]*da0[2] < error2 ) da0[2] = 0.0f;
   if( da0[3]*da0[3] < error2 ) da0[3] = 0.0f;

   // Compute distance from plane 1 of subpatch B.
   // A0-B1 && A1-B1.
   float da1[4];
   da1[0] = planeB1.evaluate( pA._vertices[spA._corners[0]]._pos );
   da1[1] = planeB1.evaluate( pA._vertices[spA._corners[1]]._pos );
   da1[2] = planeB1.evaluate( pA._vertices[spA._corners[2]]._pos );
   da1[3] = planeB1.evaluate( pA._vertices[spA._corners[3]]._pos );

   // Coplanarity robustness snapping.
   if( da1[0]*da1[0] < error2 ) da1[0] = 0.0f;
   if( da1[1]*da1[1] < error2 ) da1[1] = 0.0f;
   if( da1[2]*da1[2] < error2 ) da1[2] = 0.0f;
   if( da1[3]*da1[3] < error2 ) da1[3] = 0.0f;

   // Computing triangles indices.
   int idx[] = {0,1,2,2,3,0,3,0,1,1,2,3};
   int* a0 = idx + trimA.type()*6;
   int* a1 = idx + trimA.type()*6 + 3;
   int* b0 = idx + trimB.type()*6;
   int* b1 = idx + trimB.type()*6 + 3;

   // Intersection for triangle A0-B0
   trim( pA, pB, spA, spB, trimA, trimB, da0, db0, a0, b0 );
   // Intersection for triangle A0-B1
   trim( pA, pB, spA, spB, trimA, trimB, da1, db0, a0, b1 );
   // Intersection for triangle A1-B0
   trim( pA, pB, spA, spB, trimA, trimB, da0, db1, a1, b0 );
   // Intersection for triangle A1-B1
   trim( pA, pB, spA, spB, trimA, trimB, da1, db1, a1, b1 );
}

//------------------------------------------------------------------------------
//!
uint DFGeometry::getTrimming( Patch& p, uint16_t spID )
{
   Subpatch& sp = p._subpatches[spID];

   if( sp._trimming == NULL_ID32 )
   {
      p.trim(1);

      sp._trimming = uint32_t(_trimmings.size());
      _trimmings.pushBack( Trimming() );
      Trimming& tr = _trimmings.back();

      tr._vertices = NULL_ID16;
      tr._loops.pushBack(0); // 0 loops.

      // Compute triangulation pattern.
      // 0: triangles (0,1,2),(0,2,3).
      // 1: triangles (0,1,3),(1,2,3).
      Vertex& c0 = p._vertices[sp._corners[0]];
      Vertex& c1 = p._vertices[sp._corners[1]];
      Vertex& c2 = p._vertices[sp._corners[2]];
      Vertex& c3 = p._vertices[sp._corners[3]];

      Vec3f ab  = c1._pos - c0._pos;
      Vec3f ac  = c2._pos - c0._pos;
      Vec3f ad  = c3._pos - c0._pos;
      Vec3f n0  = cross(ab,ac);
      Vec3f n1  = cross(ac,ad);

      if( dot(n0,n1) > 0.0f )
      {
         tr._flags  = 0;
         tr._plane0 = Planef( n0, c0._pos ).normalize();
         tr._plane1 = Planef( n1, c0._pos ).normalize();
      }
      else
      {
         tr._flags  = 1;
         tr._plane0 = Planef( cross(ab,ad), c0._pos ).normalize();
         tr._plane1 = Planef( c1._pos, c2._pos, c3._pos ).normalize();
      }

      // Compute projection axes.
      uint maxc0 = tr._plane0.direction().maxComponent();
      uint maxc1 = tr._plane1.direction().maxComponent();

      if( tr._plane0.direction()(maxc0) > tr._plane1.direction()(maxc1) )
      {
         tr._z = maxc0;
         tr._x = (maxc0+1)%3;
         tr._y = (maxc0+2)%3;
         if( tr._plane0.direction()(maxc0) < 0.0f ) CGM::swap( tr._x, tr._y );
      }
      else
      {
         tr._z = maxc1;
         tr._x = (maxc1+1)%3;
         tr._y = (maxc1+2)%3;
         if( tr._plane1.direction()(maxc1) < 0.0f ) CGM::swap( tr._x, tr._y );
      }
   }

   return sp._trimming;
}
UNNAMESPACE_BEGIN
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

   if( CGM::equal( denom, 0.0 ) ) return false;

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
UNNAMESPACE_END

//------------------------------------------------------------------------------
//!
void
DFGeometry::trim(
   Patch&    pA,
   Patch&    pB,
   Subpatch& spA,
   Subpatch& spB,
   Trimming& trimA,
   Trimming& trimB,
   float*    da,
   float*    db,
   int*      idxa,
   int*      idxb
)
{
   // TODO: should add code to skip adding new feature when we are already at
   // 2 feature by triangle.

   // FIXME: There is still some "unsolved" illegal cases when computing
   // edge/edge intersection. Those could lead to find 2 or 3 feature points
   // per triangle. An edge should only be allowed to intersects twice if it
   // is contained in the plane of the other triangle (both its and vertices
   // distance equal 0).
   // For example, the edge/edge validation test could be replace as follow:
   // 1. compute the number of intersection on the other edge. Vertex count.
   // 2. Test for edge/edge intersection only if there is no intersection or
   // if the other edge is in the triangle plane.

   // Rounding precisions.
   const float errorp  = _errormax*_errormax;
   const float errore  = 2.0f*errorp;
   const float errorv  = 4.0f*errorp;
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

   Vertex& ca0 = pA._vertices[spA._corners[idxa[0]]];
   Vertex& ca1 = pA._vertices[spA._corners[idxa[1]]];
   Vertex& ca2 = pA._vertices[spA._corners[idxa[2]]];

   Vertex& cb0 = pB._vertices[spB._corners[idxb[0]]];
   Vertex& cb1 = pB._vertices[spB._corners[idxb[1]]];
   Vertex& cb2 = pB._vertices[spB._corners[idxb[2]]];

   Vec3f va0 = ca0._pos;
   Vec3f va1 = ca1._pos;
   Vec3f va2 = ca2._pos;

   Vec3f vb0 = cb0._pos;
   Vec3f vb1 = cb1._pos;
   Vec3f vb2 = cb2._pos;

   Vec2f uva0 = ca0._uv;
   Vec2f uva1 = ca1._uv;
   Vec2f uva2 = ca2._uv;
   Vec2f uvb0 = cb0._uv;
   Vec2f uvb1 = cb1._uv;
   Vec2f uvb2 = cb2._uv;

   Vec3f na0 = ca0._n;
   Vec3f na1 = ca1._n;
   Vec3f na2 = ca2._n;
   Vec3f nb0 = cb0._n;
   Vec3f nb1 = cb1._n;
   Vec3f nb2 = cb2._n;


   // Test all vertices states.
   // Important: an edge could intersect 2 vertices...
   int vaStates[3] = {-1,-1,-1};
   int vbStates[3] = {-1,-1,-1};
   int eaStates[3] = {-1,-1,-1};
   int ebStates[3] = {-1,-1,-1};

   uint16_t va[4];
   uint16_t vb[4];
   // Keep tracks of wich edges the feature intersecting points are on.
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
         vaStates[0]   = 0;   vbStates[0]   = 0;
         va[numVA++]   = spA._corners[idxa[0]];
         vb[numVB++]   = spB._corners[idxb[0]];
      }
      else
      if( (db1 == 0.0f) && (sqrLength(va0-vb1) < errorv) )
      {
         amasks[numVA] = 0x5; bmasks[numVB] = 0x3;
         vaStates[0]   = 1;   vbStates[1]   = 0;
         va[numVA++]   = spA._corners[idxa[0]];
         vb[numVB++]   = spB._corners[idxb[1]];
      }
      else
      if( (db2 == 0.0f) && (sqrLength(va0-vb2) < errorv) )
      {
         amasks[numVA] = 0x5; bmasks[numVB] = 0x6;
         vaStates[0]   = 2;   vbStates[2]   = 0;
         va[numVA++]   = spA._corners[idxa[0]];
         vb[numVB++]   = spB._corners[idxb[2]];
      }
      else
      // Vertex-edge.
      {
         if( closestPtSegment( va0, vb0, vb1, errore, pt0, t ) )
         {
            amasks[numVA] = 0x5; bmasks[numVB] = 0x1;
            vaStates[0]   = 3;   ebStates[0]   = 0;
            va[numVA++]   = spA._corners[idxa[0]];
            vb[numVB++]   = insertEdgeVertex( pB, spB, idxb[0], pt0, t );
         }
         else if( closestPtSegment( va0, vb1, vb2, errore, pt0, t ) )
         {
            amasks[numVA] = 0x5; bmasks[numVB] = 0x2;
            vaStates[0]   = 4;   ebStates[1]   = 0;
            va[numVA++]   = spA._corners[idxa[0]];
            vb[numVB++]   = insertEdgeVertex( pB, spB, idxb[1], pt0, t );
         }
         else if( closestPtSegment( va0, vb2, vb0, errore, pt0, t ) )
         {
            amasks[numVA] = 0x5; bmasks[numVB] = 0x4;
            vaStates[0]   = 5;   ebStates[2]   = 0;
            va[numVA++]   = spA._corners[idxa[0]];
            // vb2-vb0 is not really an edge.
            vb[numVB++]   = insertFaceVertex( pB, trimB, pt0, CGM::linear2( nb2, nb0, t ), CGM::linear2( uvb2, uvb0, t ) );
         }
         // Vertex-triangle.
         else if( closestPtTriangle( va0, vb0, vb1, vb2, pt0, bary ) )
         {
            amasks[numVA] = 0x5; bmasks[numVB] = 0x8;
            vaStates[0]   = 6;
            va[numVA++]   = spA._corners[idxa[0]];
            vb[numVB++]   = insertFaceVertex( pB, trimB, pt0, baryc( nb0, nb1, nb2, bary ), baryc( uvb0, uvb1, uvb2, bary ) );
         }
      }
   }
   if( da1 == 0.0f )
   {
      if( (db0 == 0.0f) && (sqrLength(va1-vb0) < errorv) )
      {
         amasks[numVA] = 0x3; bmasks[numVB] = 0x5;
         vaStates[1]   = 0;   vbStates[0]   = 1;
         va[numVA++]   = spA._corners[idxa[1]];
         vb[numVB++]   = spB._corners[idxb[0]];
      }
      else
      if( (db1 == 0.0f) && (sqrLength(va1-vb1) < errorv) )
      {
         amasks[numVA] = 0x3; bmasks[numVB] = 0x3;
         vaStates[1]   = 1;   vbStates[1]   = 1;
         va[numVA++]   = spA._corners[idxa[1]];
         vb[numVB++]   = spB._corners[idxb[1]];
      }
      else
      if( (db2 == 0.0f) && (sqrLength(va1-vb2) < errorv) )
      {
         amasks[numVA] = 0x3; bmasks[numVB] = 0x6;
         vaStates[1]   = 2;   vbStates[2]   = 1;
         va[numVA++]   = spA._corners[idxa[1]];
         vb[numVB++]   = spB._corners[idxb[2]];
      }
      else
      // Vertex-edge.
      {
         if( closestPtSegment( va1, vb0, vb1, errore, pt0, t ) )
         {
            amasks[numVA] = 0x3; bmasks[numVB] = 0x1;
            vaStates[1]   = 3;   ebStates[0]   = 1;
            va[numVA++]   = spA._corners[idxa[1]];
            vb[numVB++]   = insertEdgeVertex( pB, spB, idxb[0], pt0, t );
         }
         else if( closestPtSegment( va1, vb1, vb2, errore, pt0, t ) )
         {
            amasks[numVA] = 0x3; bmasks[numVB] = 0x2;
            vaStates[1]   = 4;   ebStates[1]   = 1;
            va[numVA++]   = spA._corners[idxa[1]];
            vb[numVB++]   = insertEdgeVertex( pB, spB, idxb[1], pt0, t );
         }
         else if( closestPtSegment( va1, vb2, vb0, errore, pt0, t ) )
         {
            amasks[numVA] = 0x3; bmasks[numVB] = 0x4;
            vaStates[1]   = 5;   ebStates[2]   = 1;
            va[numVA++]   = spA._corners[idxa[1]];
            // vb2-vb0 is ot really an edge.
            vb[numVB++]   = insertFaceVertex( pB, trimB, pt0, CGM::linear2( nb2, nb0, t ), CGM::linear2( uvb2, uvb0, t ) );
         }
         // Vertex-triangle.
         else if( closestPtTriangle( va1, vb0, vb1, vb2, pt0, bary ) )
         {
            amasks[numVA] = 0x3; bmasks[numVB] = 0x8;
            vaStates[1]   = 6;
            va[numVA++]   = spA._corners[idxa[1]];
            vb[numVB++]   = insertFaceVertex( pB, trimB, pt0, baryc( nb0, nb1, nb2, bary ), baryc( uvb0, uvb1, uvb2, bary ) );
         }
      }
   }
   if( da2 == 0.0f )
   {
      if( (db0 == 0.0f) && (sqrLength(va2-vb0) < errorv) )
      {
         amasks[numVA] = 0x6; bmasks[numVB] = 0x5;
         vaStates[2]   = 0;   vbStates[0]   = 2;
         va[numVA++]   = spA._corners[idxa[2]];
         vb[numVB++]   = spB._corners[idxb[0]];
      }
      else
      if( (db1 == 0.0f) && (sqrLength(va2-vb1) < errorv) )
      {
         amasks[numVA] = 0x6; bmasks[numVB] = 0x3;
         vaStates[2]   = 1;   vbStates[1]   = 2;
         va[numVA++]   = spA._corners[idxa[2]];
         vb[numVB++]   = spB._corners[idxb[1]];
      }
      else
      if( (db2 == 0.0f) && (sqrLength(va2-vb2) < errorv) )
      {
         amasks[numVA] = 0x6; bmasks[numVB] = 0x6;
         vaStates[2]   = 2;   vbStates[2]   = 2;
         va[numVA++]   = spA._corners[idxa[2]];
         vb[numVB++]   = spB._corners[idxb[2]];
      }
      else
      // Vertex-edge.
      {
         if( closestPtSegment( va2, vb0, vb1, errore, pt0, t ) )
         {
            amasks[numVA] = 0x6; bmasks[numVB] = 0x1;
            vaStates[2]   = 3;   ebStates[0]   = 2;
            va[numVA++]   = spA._corners[idxa[2]];
            vb[numVB++]   = insertEdgeVertex( pB, spB, idxb[0], pt0, t );
         }
         else if( closestPtSegment( va2, vb1, vb2, errore, pt0, t ) )
         {
            amasks[numVA] = 0x6; bmasks[numVB] = 0x2;
            vaStates[2]   = 4;   ebStates[1]   = 2;
            va[numVA++]   = spA._corners[idxa[2]];
            vb[numVB++]   = insertEdgeVertex( pB, spB, idxb[1], pt0, t );
         }
         else if( closestPtSegment( va2, vb2, vb0, errore, pt0, t ) )
         {
            amasks[numVA] = 0x6; bmasks[numVB] = 0x4;
            vaStates[2]   = 5;   ebStates[2]   = 2;
            va[numVA++]   = spA._corners[idxa[2]];
            // vb2-vb0 is ot really an edge.
            vb[numVB++]   = insertFaceVertex( pB, trimB, pt0, CGM::linear2( nb2, nb0, t ), CGM::linear2( uvb2, uvb0, t ) );
         }
         // Vertex-triangle.
         else if( closestPtTriangle( va2, vb0, vb1, vb2, pt0, bary ) )
         {
            amasks[numVA] = 0x6; bmasks[numVB] = 0x8;
            vaStates[2]   = 6;
            va[numVA++]   = spA._corners[idxa[2]];
            vb[numVB++]   = insertFaceVertex( pB, trimB, pt0, baryc( nb0, nb1, nb2, bary ), baryc( uvb0, uvb1, uvb2, bary ) );
         }
      }
   }
   // Vertex-edge.
   if( (db0 == 0.0f) && (vbStates[0] == -1) )
   {
      if( closestPtSegment( vb0, va0, va1, errore, pt0, t ) )
      {
         bmasks[numVB] = 0x5; amasks[numVA] = 0x1;
         vbStates[0]   = 3;   eaStates[0]   = 0;
         vb[numVB++]   = spB._corners[idxb[0]];
         va[numVA++]   = insertEdgeVertex( pA, spA, idxa[0], pt0, t );
      }
      else if( closestPtSegment( vb0, va1, va2, errore, pt0, t ) )
      {
         bmasks[numVB] = 0x5; amasks[numVA] = 0x2;
         vbStates[0]   = 4;   eaStates[1]   = 0;
         vb[numVB++]   = spB._corners[idxb[0]];
         va[numVA++]   = insertEdgeVertex( pA, spA, idxa[1], pt0, t );
      }
      else if( closestPtSegment( vb0, va2, va0, errore, pt0, t ) )
      {
         bmasks[numVB] = 0x5; amasks[numVA] = 0x4;
         vbStates[0]   = 5;   eaStates[2]   = 0;
         vb[numVB++]   = spB._corners[idxb[0]];
         // va2-va0 is ot really an edge.
         va[numVA++]   = insertFaceVertex( pA, trimA, pt0, CGM::linear2( na2, na0, t ), CGM::linear2( uva2, uva0, t ) );
      }
      // Vertex-triangle.
      else if( closestPtTriangle( vb0, va0, va1, va2, pt0, bary ) )
      {
         bmasks[numVB] = 0x5; amasks[numVA] = 0x8;
         vbStates[0]   = 6;
         vb[numVB++]   = spB._corners[idxb[0]];
         va[numVA++]   = insertFaceVertex( pA, trimA, pt0, baryc( na0, na1, na2, bary ), baryc( uva0, uva1, uva2, bary ) );
      }
   }
   if( (db1 == 0.0f) && (vbStates[1] == -1) )
   {
      if( closestPtSegment( vb1, va0, va1, errore, pt0, t ) )
      {
         bmasks[numVB] = 0x3; amasks[numVA] = 0x1;
         vbStates[1]   = 3;   eaStates[0]   = 1;
         vb[numVB++]   = spB._corners[idxb[1]];
         va[numVA++]   = insertEdgeVertex( pA, spA, idxa[0], pt0, t );
      }
      else if( closestPtSegment( vb1, va1, va2, errore, pt0, t ) )
      {
         bmasks[numVB] = 0x3; amasks[numVA] = 0x2;
         vbStates[1]   = 4;   eaStates[1]   = 1;
         vb[numVB++]   = spB._corners[idxb[1]];
         va[numVA++]   = insertEdgeVertex( pA, spA, idxa[1], pt0, t );
      }
      else if( closestPtSegment( vb1, va2, va0, errore, pt0, t ) )
      {
         bmasks[numVB] = 0x3; amasks[numVA] = 0x4;
         vbStates[1]   = 5;   eaStates[2]   = 1;
         vb[numVB++]   = spB._corners[idxb[1]];
         // va2-va0 is ot really an edge.
         va[numVA++]   = insertFaceVertex( pA, trimA, pt0, CGM::linear2( na2, na0, t ), CGM::linear2( uva2, uva0, t ) );
      }
      // Vertex-triangle.
      else if( closestPtTriangle( vb1, va0, va1, va2, pt0, bary ) )
      {
         bmasks[numVB] = 0x3; amasks[numVA] = 0x8;
         vbStates[1]   = 6;
         vb[numVB++]   = spB._corners[idxb[1]];
         va[numVA++]   = insertFaceVertex( pA, trimA, pt0, baryc( na0, na1, na2, bary ), baryc( uva0, uva1, uva2, bary ) );
      }
   }
   if( (db2 == 0.0f) && (vbStates[2] == -1) )
   {
      if( closestPtSegment( vb2, va0, va1, errore, pt0, t ) )
      {
         bmasks[numVB] = 0x6; amasks[numVA] = 0x1;
         vbStates[2]   = 3;   eaStates[0]   = 2;
         vb[numVB++]   = spB._corners[idxb[2]];
         va[numVA++]   = insertEdgeVertex( pA, spA, idxa[0], pt0, t );
      }
      else if( closestPtSegment( vb2, va1, va2, errore, pt0, t ) )
      {
         bmasks[numVB] = 0x6; amasks[numVA] = 0x2;
         vbStates[2]   = 4;   eaStates[1]   = 2;
         vb[numVB++]   = spB._corners[idxb[2]];
         va[numVA++]   = insertEdgeVertex( pA, spA, idxa[1], pt0, t );
      }
      else if( closestPtSegment( vb2, va2, va0, errore, pt0, t ) )
      {
         bmasks[numVB] = 0x6; amasks[numVA] = 0x4;
         vbStates[2]   = 5;   eaStates[2]   = 2;
         vb[numVB++]   = spB._corners[idxb[2]];
         // va2-va0 is ot really an edge.
         va[numVA++]   = insertFaceVertex( pA, trimA, pt0, CGM::linear2( na2, na0, t ), CGM::linear2( uva2, uva0, t ) );
      }
      // Vertex-triangle.
      else if( closestPtTriangle( vb2, va0, va1, va2, pt0, bary ) )
      {
         bmasks[numVB] = 0x6; amasks[numVA] = 0x8;
         vbStates[2]   = 6;
         vb[numVB++]   = spB._corners[idxb[2]];
         va[numVA++]   = insertFaceVertex( pA, trimA, pt0, baryc( na0, na1, na2, bary ), baryc( uva0, uva1, uva2, bary ) );
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
            eaStates[0]   = 3;   ebStates[0]   = 3;
            va[numVA++]   = insertEdgeVertex( pA, spA, idxa[0], pt0, s );
            vb[numVB++]   = insertEdgeVertex( pB, spB, idxb[0], pt1, t );
         }
      }
      else if( closestPtSegmentSegment( va0, va1, vb1, vb2, errorp, pt0, s, pt1, t ) )
      {
         if( (vbStates[1] == -1 || sqrLength(pt1-vb1) > erroree) && (vbStates[2] == -1 || sqrLength(pt1-vb2) > erroree) )
         {
            amasks[numVA] = 0x1; bmasks[numVB] = 0x2;
            eaStates[0]   = 4;   ebStates[1]   = 3;
            va[numVA++]   = insertEdgeVertex( pA, spA, idxa[0], pt0, s );
            vb[numVB++]   = insertEdgeVertex( pB, spB, idxb[1], pt1, t );
         }
      }
      else if( closestPtSegmentSegment( va0, va1, vb2, vb0, errorp, pt0, s, pt1, t ) )
      {
         if( (vbStates[2] == -1 || sqrLength(pt1-vb2) > erroree) && (vbStates[0] == -1 || sqrLength(pt1-vb0) > erroree) )
         {
            amasks[numVA] = 0x1; bmasks[numVB] = 0x4;
            eaStates[0]   = 5;   ebStates[2]   = 3;
            va[numVA++]   = insertEdgeVertex( pA, spA, idxa[0], pt0, s );
            vb[numVB++]   = insertFaceVertex( pB, trimB, pt1, CGM::linear2( nb2, nb0, t ), CGM::linear2( uvb2, uvb0, t ) );
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
            eaStates[0]   = 6;
            va[numVA++]   = insertEdgeVertex( pA, spA, idxa[0], pt0, t );
            vb[numVB++]   = insertFaceVertex( pB, trimB, pt1, baryc( nb0, nb1, nb2, bary ), baryc( uvb0, uvb1, uvb2, bary ) );
         }
      }
   }
   else if( da0da1 == 0.0f )
   {
      // Edge-edge.
      if( closestPtSegmentSegment( va0, va1, vb0, vb1, errorp, pt0, s, pt1, t ) )
      {
         if( (vaStates[0] == -1 || sqrLength(pt1-va0) > erroree) && (vaStates[1] == -1 || sqrLength(pt1-va1) > erroree) )
         {
            amasks[numVA] = 0x1; bmasks[numVB] = 0x1;
            eaStates[0]   = 3;   ebStates[0]   = 3;
            va[numVA++]   = insertEdgeVertex( pA, spA, idxa[0], pt0, s );
            vb[numVB++]   = insertEdgeVertex( pB, spB, idxb[0], pt1, t );
         }
      }
      if( closestPtSegmentSegment( va0, va1, vb1, vb2, errorp, pt0, s, pt1, t ) )
      {
         if( (vaStates[0] == -1 || sqrLength(pt1-va0) > erroree) && (vaStates[1] == -1 || sqrLength(pt1-va1) > erroree) )
         {
            amasks[numVA] = 0x1; bmasks[numVB] = 0x2;
            eaStates[0]   = 4;   ebStates[1]   = 3;
            va[numVA++]   = insertEdgeVertex( pA, spA, idxa[0], pt0, s );
            vb[numVB++]   = insertEdgeVertex( pB, spB, idxb[1], pt1, t );
         }
      }
      if( closestPtSegmentSegment( va0, va1, vb2, vb0, errorp, pt0, s, pt1, t ) )
      {
         if( (vaStates[0] == -1 || sqrLength(pt1-va0) > erroree) && (vaStates[1] == -1 || sqrLength(pt1-va1) > erroree) )
         {
            amasks[numVA] = 0x1; bmasks[numVB] = 0x4;
            eaStates[0]   = 5;   ebStates[2]   = 3;
            va[numVA++]   = insertEdgeVertex( pA, spA, idxa[0], pt0, s );
            vb[numVB++]   = insertFaceVertex( pB, trimB, pt1, CGM::linear2( nb2, nb0, t ), CGM::linear2( uvb2, uvb0, t ) );
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
            eaStates[1]   = 3;   ebStates[0]   = 4;
            va[numVA++]   = insertEdgeVertex( pA, spA, idxa[1], pt0, s );
            vb[numVB++]   = insertEdgeVertex( pB, spB, idxb[0], pt1, t );
         }
      }
      else if( closestPtSegmentSegment( va1, va2, vb1, vb2, errorp, pt0, s, pt1, t ) )
      {
         if( (vbStates[1] == -1 || sqrLength(pt1-vb1) > erroree) && (vbStates[2] == -1 || sqrLength(pt1-vb2) > erroree) )
         {
            amasks[numVA] = 0x2; bmasks[numVB] = 0x2;
            eaStates[1]   = 4;   ebStates[1]   = 4;
            va[numVA++]   = insertEdgeVertex( pA, spA, idxa[1], pt0, s );
            vb[numVB++]   = insertEdgeVertex( pB, spB, idxb[1], pt1, t );
         }
      }
      else if( closestPtSegmentSegment( va1, va2, vb2, vb0, errorp, pt0, s, pt1, t ) )
      {
         if( (vbStates[2] == -1 || sqrLength(pt1-vb2) > erroree) && (vbStates[0] == -1 || sqrLength(pt1-vb0) > erroree) )
         {
            amasks[numVA] = 0x2; bmasks[numVB] = 0x4;
            eaStates[1]   = 5;   ebStates[2]   = 4;
            va[numVA++]   = insertEdgeVertex( pA, spA, idxa[1], pt0, s );
            vb[numVB++]   = insertFaceVertex( pB, trimB, pt1, CGM::linear2( nb2, nb0, t ), CGM::linear2( uvb2, uvb0, t ) );
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
            eaStates[1]   = 6;
            va[numVA++]   = insertEdgeVertex( pA, spA, idxa[1], pt0, t );
            vb[numVB++]   = insertFaceVertex( pB, trimB, pt1, baryc( nb0, nb1, nb2, bary ), baryc( uvb0, uvb1, uvb2, bary ) );
         }
      }
   }
   else if( da1da2 == 0.0f )
   {
      // Edge-edge.
      if( closestPtSegmentSegment( va1, va2, vb0, vb1, errorp, pt0, s, pt1, t ) )
      {
         if( (vaStates[1] == -1 || sqrLength(pt1-va1) > erroree) && (vaStates[2] == -1 || sqrLength(pt1-va2) > erroree) )
         {
            amasks[numVA] = 0x2; bmasks[numVB] = 0x1;
            eaStates[1]   = 3;   ebStates[0]   = 4;
            va[numVA++]   = insertEdgeVertex( pA, spA, idxa[1], pt0, s );
            vb[numVB++]   = insertEdgeVertex( pB, spB, idxb[0], pt1, t );
         }
      }
      if( closestPtSegmentSegment( va1, va2, vb1, vb2, errorp, pt0, s, pt1, t ) )
      {
         if( (vaStates[1] == -1 || sqrLength(pt1-va1) > erroree) && (vaStates[2] == -1 || sqrLength(pt1-va2) > erroree) )
         {
            amasks[numVA] = 0x2; bmasks[numVB] = 0x2;
            eaStates[1]   = 4;   ebStates[1]   = 4;
            va[numVA++]   = insertEdgeVertex( pA, spA, idxa[1], pt0, s );
            vb[numVB++]   = insertEdgeVertex( pB, spB, idxb[1], pt1, t );
         }
      }
      if( closestPtSegmentSegment( va1, va2, vb2, vb0, errorp, pt0, s, pt1, t ) )
      {
         if( (vaStates[1] == -1 || sqrLength(pt1-va1) > erroree) && (vaStates[2] == -1 || sqrLength(pt1-va2) > erroree) )
         {
            amasks[numVA] = 0x2; bmasks[numVB] = 0x4;
            eaStates[1]   = 5;   ebStates[2]   = 4;
            va[numVA++]   = insertEdgeVertex( pA, spA, idxa[1], pt0, s );
            vb[numVB++]   = insertFaceVertex( pB, trimB, pt1, CGM::linear2( nb2, nb0, t ), CGM::linear2( uvb2, uvb0, t ) );
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
            eaStates[2]   = 3;   ebStates[0]   = 5;
            va[numVA++]   = insertFaceVertex( pA, trimA, pt0, CGM::linear2( na2, na0, s ), CGM::linear2( uva2, uva0, s ) );
            vb[numVB++]   = insertEdgeVertex( pB, spB, idxb[0], pt1, t );
         }
      }
      else if( closestPtSegmentSegment( va2, va0, vb1, vb2, errorp, pt0, s, pt1, t ) )
      {
         if( (vbStates[1] == -1 || sqrLength(pt1-vb1) > erroree) && (vbStates[2] == -1 || sqrLength(pt1-vb2) > erroree) )
         {
            amasks[numVA] = 0x4; bmasks[numVB] = 0x2;
            eaStates[2]   = 4;   ebStates[1]   = 5;
            va[numVA++]   = insertFaceVertex( pA, trimA, pt0, CGM::linear2( na2, na0, s ), CGM::linear2( uva2, uva0, s ) );
            vb[numVB++]   = insertEdgeVertex( pB, spB, idxb[1], pt1, t );
         }
      }
      else if( closestPtSegmentSegment( va2, va0, vb2, vb0, errorp, pt0, s, pt1, t ) )
      {
         if( (vbStates[2] == -1 || sqrLength(pt1-vb2) > erroree) && (vbStates[0] == -1 || sqrLength(pt1-vb0) > erroree) )
         {
            amasks[numVA] = 0x4; bmasks[numVB] = 0x4;
            eaStates[2]   = 5;   ebStates[2]   = 5;
            va[numVA++]   = insertFaceVertex( pA, trimA, pt0, CGM::linear2( na2, na0, s ), CGM::linear2( uva2, uva0, s ) );
            vb[numVB++]   = insertFaceVertex( pB, trimB, pt1, CGM::linear2( nb2, nb0, t ), CGM::linear2( uvb2, uvb0, t ) );
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
            eaStates[2]   = 6;
            va[numVA++]   = insertFaceVertex( pA, trimA, pt0, CGM::linear2( na2, na0, t ), CGM::linear2( uva2, uva0, t ) );
            vb[numVB++]   = insertFaceVertex( pB, trimB, pt1, baryc( nb0, nb1, nb2, bary ), baryc( uvb0, uvb1, uvb2, bary ) );
         }
      }
   }
   else if( da0da2 == 0.0f )
   {
      // Edge-edge.
      if( closestPtSegmentSegment( va2, va0, vb0, vb1, errorp, pt0, s, pt1, t ) )
      {
         if( (vaStates[0] == -1 || sqrLength(pt1-va0) > erroree) && (vaStates[2] == -1 || sqrLength(pt1-va2) > erroree) )
         {
            amasks[numVA] = 0x4; bmasks[numVB] = 0x1;
            eaStates[2]   = 3;   ebStates[0]   = 5;
            va[numVA++]   = insertFaceVertex( pA, trimA, pt0, CGM::linear2( na2, na0, s ), CGM::linear2( uva2, uva0, s ) );
            vb[numVB++]   = insertEdgeVertex( pB, spB, idxb[0], pt1, t );
         }
      }
      if( closestPtSegmentSegment( va2, va0, vb1, vb2, errorp, pt0, s, pt1, t ) )
      {
         if( (vaStates[0] == -1 || sqrLength(pt1-va0) > erroree) && (vaStates[2] == -1 || sqrLength(pt1-va2) > erroree) )
         {
            amasks[numVA] = 0x4; bmasks[numVB] = 0x2;
            eaStates[2]   = 4;   ebStates[1]   = 5;
            va[numVA++]   = insertFaceVertex( pA, trimA, pt0, CGM::linear2( na2, na0, s ), CGM::linear2( uva2, uva0, s ) );
            vb[numVB++]   = insertEdgeVertex( pB, spB, idxb[1], pt1, t );
         }
      }
      if( closestPtSegmentSegment( va2, va0, vb2, vb0, errorp, pt0, s, pt1, t ) )
      {
         if( (vaStates[0] == -1 || sqrLength(pt1-va0) > erroree) && (vaStates[2] == -1 || sqrLength(pt1-va2) > erroree) )
         {
            amasks[numVA] = 0x4; bmasks[numVB] = 0x4;
            eaStates[2]   = 5;   ebStates[2]   = 5;
            va[numVA++]   = insertFaceVertex( pA, trimA, pt0, CGM::linear2( na2, na0, s ), CGM::linear2( uva2, uva0, s ) );
            vb[numVB++]   = insertFaceVertex( pB, trimB, pt1, CGM::linear2( nb2, nb0, t ), CGM::linear2( uvb2, uvb0, t ) );
         }
      }
   }


   // 3. Test all edges of triangle B against triangle A.
   // Testing edge b0.
   if( (db0db1 < 0.0f) && (ebStates[0] == -1) )
   {
      t   = db0/(db0-db1);
      pt0 = vb0+(vb1-vb0)*t;
      if( closestPtTriangle( pt0, va0, va1, va2, pt1, bary ) )
      {
         bmasks[numVB] = 0x1; amasks[numVA] = 0x8;
         ebStates[0]   = 6;
         vb[numVB++]   = insertEdgeVertex( pB, spB, idxb[0], pt0, t );
         va[numVA++]   = insertFaceVertex( pA, trimA, pt1, baryc( na0, na1, na2, bary ), baryc( uva0, uva1, uva2, bary ) );
      }
   }

   // Testing edge b1.
   if( (db1db2 < 0.0f) && (ebStates[1] == -1) )
   {
      // Edge-triangle.
      t   = db1/(db1-db2);
      pt0 = vb1+(vb2-vb1)*t;
      if( closestPtTriangle( pt0, va0, va1, va2, pt1, bary ) )
      {
         bmasks[numVB] = 0x2; amasks[numVA] = 0x8;
         ebStates[1]   = 6;
         vb[numVB++]   = insertEdgeVertex( pB, spB, idxb[1], pt0, t );
         va[numVA++]   = insertFaceVertex( pA, trimA, pt1, baryc( na0, na1, na2, bary ), baryc( uva0, uva1, uva2, bary ) );
      }
   }

   // Testing edge b2.
   if( (db0db2 < 0.0f) && (ebStates[2] == -1) )
   {
      t   = db2/(db2-db0);
      pt0 = vb2+(vb0-vb2)*t;
      if( closestPtTriangle( pt0, va0, va1, va2, pt1, bary ) )
      {
         bmasks[numVB] = 0x4; amasks[numVA] = 0x8;
         ebStates[2]   = 6;
         vb[numVB++]   = insertFaceVertex( pB, trimB, pt0, CGM::linear2( nb2, nb0, t ), CGM::linear2( uvb2, uvb0, t ) );
         va[numVA++]   = insertFaceVertex( pA, trimA, pt1, baryc( na0, na1, na2, bary ), baryc( uva0, uva1, uva2, bary ) );
      }
   }

   // 4. Edge creation.
   if( numVA > 1 )
   {
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
            addEdge( pA, trimA, va[0], va[1] );
            addEdge( pB, trimB, vb[0], vb[1] );
         }
         else
         {
            if( vb[0] == vb[1] ) return;
            // edge B and face A.
            // Find direction in which to move in pointer.
            int axis = idxb[edgeb>>1]&1;
            int incb = pB._vertices[vb[0]]._uv(axis) < pB._vertices[vb[1]]._uv(axis) ? axis+1: (axis+3)%4;
            // Insert an edge for each vertex of B contained inside A.
            uint16_t ca = va[0];
            for( uint16_t cb = pB._vertices[vb[0]]._next[incb]; cb != vb[1]; cb = pB._vertices[cb]._next[incb] )
            {
               if( closestPtTriangle( pB._vertices[cb]._pos, va0, va1, va2, pt0, bary ) )
               {
                  uint16_t na = insertFaceVertex( pA, trimA, pt0, baryc( na0, na1, na2, bary ), baryc( uva0, uva1, uva2, bary ) );
                  addEdge( pA, trimA, ca, na );
                  ca = na;
               }
            }
            addEdge( pA, trimA, ca, va[1] );
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
            int inca = pA._vertices[va[0]]._uv(axis) < pA._vertices[va[1]]._uv(axis) ? axis+1: (axis+3)%4;
            // Insert an edge for each vertex of A contained inside B.
            uint16_t cb = vb[0];
            for( uint16_t ca = pA._vertices[va[0]]._next[inca]; ca != va[1]; ca = pA._vertices[ca]._next[inca] )
            {
               if( closestPtTriangle( pA._vertices[ca]._pos, vb0, vb1, vb2, pt0, bary ) )
               {
                  uint16_t nb = insertFaceVertex( pB, trimB, pt0, baryc( nb0, nb1, nb2, bary ), baryc( uvb0, uvb1, uvb2, bary ) );
                  addEdge( pB, trimB, cb, nb );
                  cb = nb;
               }
            }
            addEdge( pB, trimB, cb, vb[1] );
         }
         else
         {
            if( va[0] == va[1] ) return;
            if( vb[0] == vb[1] ) return;
            // edge A and edge B.
            int axisa = idxa[edgea>>1]&1;
            int inca  = pA._vertices[va[0]]._uv(axisa) < pA._vertices[va[1]]._uv(axisa) ? axisa+1: (axisa+3)%4;
            int axisb = idxb[edgeb>>1]&1;
            int incb  = pB._vertices[vb[0]]._uv(axisb) < pB._vertices[vb[1]]._uv(axisb) ? axisb+1: (axisb+3)%4;
            // 1. for each vertex between vb[0] and vb[1] insert it in edge A.
            for( uint16_t cb = pB._vertices[vb[0]]._next[incb]; cb != vb[1]; cb = pB._vertices[cb]._next[incb] )
            {
               if( (edgea>>1) == 0 )
               {
                  if( closestPtSegment( pB._vertices[cb]._pos, va0, va1, errorp, pt0, t ) )
                     insertEdgeVertex( pA, spA, idxa[edgea>>1], pt0, t );
               }
               else
               {
                  if( closestPtSegment( pB._vertices[cb]._pos, va1, va2, errorp, pt0, t ) )
                     insertEdgeVertex( pA, spA, idxa[edgea>>1], pt0, t );
               }
            }
            // 2. insert vertices between va[0] and va[1] in edge B.
            for( uint16_t ca = pA._vertices[va[0]]._next[inca]; ca != va[1]; ca = pA._vertices[ca]._next[inca] )
            {
               if( (edgeb>>1) == 0 )
               {
                  if( closestPtSegment( pA._vertices[ca]._pos, vb0, vb1, errorp, pt0, t ) )
                     insertEdgeVertex( pB, spB, idxb[edgeb>>1], pt0, t );
               }
               else
               {
                  if( closestPtSegment( pA._vertices[ca]._pos, vb1, vb2, errorp, pt0, t ) )
                     insertEdgeVertex( pB, spB, idxb[edgeb>>1], pt0, t );
               }
            }
         }
      }
   }
}

//------------------------------------------------------------------------------
//!
void DFGeometry::addEdge( Patch& p, Trimming& t, uint16_t v0, uint16_t v1 )
{
   // Sort vertices in edge to eliminate double edge.
   int x = t._x;
   int y = t._y;
   if( lt( p._vertices[v1]._pos(x,y), p._vertices[v0]._pos(x,y) ) ) CGM::swap( v0, v1 );

   // TODO: test for edge validity (use uvs).

   // Keep a unique edge.
   t._edges.add( Pair<uint16_t,uint16_t>( v0, v1 ) );
   // Invalidate the loops.
   t.invalidate(1);
}

//------------------------------------------------------------------------------
//!
void DFGeometry::updateLoops( size_t begin, size_t end )
{
   LoopBuilder loopBuilder( this );
   for( auto p = _patches.begin()+begin; p != _patches.begin()+end; ++p )
   {
      if( p->isHidden() || !p->isTrimmed() ) continue;
      for( auto s = p->_subpatches.begin(); s != p->_subpatches.end(); ++s )
      {
         if( s->isTrimmed() ) loopBuilder.computeLoops( *p, *s, _trimmings[s->_trimming] );
      }
   }
}

//------------------------------------------------------------------------------
//!
void DFGeometry::classify( size_t begin, size_t end, int mask, DFGeometry* geom, HGrid& grid )
{
   Vec3f pos;
   Vec3f n;
   for( auto p = _patches.begin()+begin; p != _patches.begin()+end; ++p )
   {
      if( p->isHidden() ) continue;
      if( !p->isTrimmed() )
      {
         // Select a point inside.
         getSurfacePoint( *p, p->_subpatches[0], pos, n );
         // Test point.
         int c = classifyPoint( pos, n, geom, grid );
         if( (c & mask) == 0 ) p->hide(1);
      }
      else
      {
         for( auto s = p->_subpatches.begin(); s != p->_subpatches.end(); ++s )
         {
            if( s->isHidden() ) continue;
            if( s->isTrimmed() && _trimmings[s->_trimming]._loops[0] > 0 )
            {
               Trimming& trim = _trimmings[s->_trimming];
               for( int l = 0, i = 1; l < trim._loops[0]; ++l, i += trim._loops[i]+2 )
               {
                  // Select a point inside.
                  getLoopPoint( *p, &trim._loops[i+2], trim._loops[i], trim._x, trim._y, pos, n );
                  // Test point.
                  int c = classifyPoint( pos, n, geom, grid );
                  if( (c & mask) == 0 ) trim.hide(i+1, 1);
               }
               // Optimize trimmings by removing non usefull loops.
               cleanTrimming( trim );
            }
            else
            {
               // Select a point inside.
               getSurfacePoint( *p, *s, pos, n );
               // Test point.
               int c = classifyPoint( pos, n, geom, grid );
               if( (c & mask) == 0 ) s->hide(1);
            }
         }
      }
   }
}

//------------------------------------------------------------------------------
//!
void DFGeometry::cleanTrimming( Trimming& trim )
{
   // Do we have loops.
   if( trim._loops[0] == 0 ) return;

   // For now clear all loops when they are all visible.
   // TODO: clear when all hidden, but we need to change the subpatch flag.
   // TODO: merge loops when neighboors and having the same visibility.

   bool hidden = trim.isHidden(2);
   bool same   = true;
   for( int l = 0, i = 1; l < trim._loops[0]; ++l, i += trim._loops[i]+2 )
   {
      if( hidden != trim.isHidden(i+1) )
      {
         same = false;
         break;
      }
   }
   if( same && !hidden )
   {
      trim._loops.clear();
      trim._loops.pushBack(0);
   }
}


NAMESPACE_END
