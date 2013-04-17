/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/DataFlow/DFGeomOps.h>

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

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
inline bool ccw( const Vec2f& a, const Vec2f& b, const Vec2f& c )
{
   return triArea( a, b, c ) < 0.0f;
}

/*==============================================================================
   CLASS Quadrangulator
==============================================================================*/

struct Vertex{
   Vec3f _pos;
   uint  _flag;
};

struct HEdge{
   ushort _vertex;
   ushort _next;
   ushort _prev;
   ushort _neighbor;
};

struct Face{
   uint _size;
   uint _e0;
};

enum{
   NULL_ID = 0xffff
};

class Quadrangulator{
public:
   Quadrangulator() {}

   void build( const DFPolygon& p )
   {
      size_t numVertices = p.numVertices();
      _vertices.resize( numVertices );
      _hedges.resize( numVertices );
      for( size_t i = 0; i < numVertices; ++i )
      {
         _vertices[i]._pos    = p.vertex(i);
         _vertices[i]._flag   = 0;
         _hedges[i]._vertex   = ushort(i);
         _hedges[i]._next     = ushort((i+1)%numVertices);
         _hedges[i]._prev     = ushort((i-1+numVertices)%numVertices);
         _hedges[i]._neighbor = NULL_ID;
      }
      _plane = p.plane();
      _plane.direction().secAxes( _x, _y );
   }

   void quadrangulate( const Vec3f& x, const Vec3f& y )
   {
      // Nothing to do if already a quad.
      if( _hedges.size() == 4 ) return;

      // Compute witch vertices needs to be clipped and in what directions.
      for( uint i = 0; i < _hedges.size(); ++i )
      {
         // Compute tangents.
         HEdge* he0 = &_hedges[i];
         HEdge* he1 = &_hedges[he0->_next];
         HEdge* he2 = &_hedges[he1->_next];
         Vec3f v0   = _vertices[he0->_vertex]._pos;
         Vec3f v1   = _vertices[he1->_vertex]._pos;
         Vec3f v2   = _vertices[he2->_vertex]._pos;
         Vec3f e0   = normalize(v0-v1);
         Vec3f e1   = normalize(v2-v1);
         Vec3f t0   = cross( _plane.direction(), e0 );
         Vec3f t1   = cross( e1, _plane.direction() );

         uint flag = 0;

         // Verify which direction is contained inside.
         // Test  different condition depending on the convexity of the corner.
         float convexity = dot( cross( e1, e0 ), _plane.direction() );
         float t0x = dot( t0, x );
         float t1x = dot( t1, x );
         float t0y = dot( t0, y );
         float t1y = dot( t1, y );

         if( convexity > 0.0f )
         {
            flag |= (t0x<0)&&(t1x<0) ? 1 : 0;  // +x
            flag |= (t0x>0)&&(t1x>0) ? 2 : 0;  // -x
            flag |= (t0y<0)&&(t1y<0) ? 4 : 0;  // +y
            flag |= (t0y>0)&&(t1y>0) ? 8 : 0;  // -y
         }
         else
         {
            flag |= (t0x<0)||(t1x<0) ? 1 : 0;  // +x
            flag |= (t0x>0)||(t1x>0) ? 2 : 0;  // -x
            flag |= (t0y<0)||(t1y<0) ? 4 : 0;  // +y
            flag |= (t0y>0)||(t1y>0) ? 8 : 0;  // -y
         }

         // Remove direction near existant edges.
         if( flag > 0 )
         {
            const float cosError = 0.9f;
            // Test X axis.
            float e0x = dot( e0, x );
            float e1x = dot( e1, x );
            if( e0x >  cosError ) flag &= 0xe;
            if( e1x >  cosError ) flag &= 0xe;
            if( e0x < -cosError ) flag &= 0xd;
            if( e1x < -cosError ) flag &= 0xd;
            // Test Y axis.
            float e0y = dot( e0, y );
            float e1y = dot( e1, y );
            if( e0y >  cosError ) flag &= 0xb;
            if( e1y >  cosError ) flag &= 0xb;
            if( e0y < -cosError ) flag &= 0x7;
            if( e1y < -cosError ) flag &= 0x7;
         }

         _vertices[he1->_vertex]._flag = flag;
      }

      // Loop on all exterior vertices and do some clipping when necessary.
      uint numVertices = uint(_vertices.size());
      for( uint i = 0; i < numVertices; ++i )
      {
         uint flag = _vertices[_hedges[i]._vertex]._flag;
         if( flag == 0 ) continue;
         if( flag & 1 ) clip( i,  x, 0xd );
         if( flag & 2 ) clip( i, -x, 0xe );
         if( flag & 4 ) clip( i,  y, 0x7 );
         if( flag & 8 ) clip( i, -y, 0xb );
      }

      // Subdivide to obtains only quads.
      subdivideToQuads();
   }

   void splitEdge( uint edge, float t, uint flag )
   {
      HEdge* he0 = &_hedges[edge];
      HEdge* he1 = &_hedges[he0->_next];
      Vec3f v0   = _vertices[he0->_vertex]._pos;
      Vec3f v2   = _vertices[he1->_vertex]._pos;
      // Create new vertices.
      Vertex v;
      v._flag = flag;
      v._pos  = CGM::linear2( v0, v2, t );
      _vertices.pushBack(v);
      // Update edges.
      HEdge he;
      he._vertex              = ushort(_vertices.size())-1;
      he._neighbor            = _hedges[edge]._neighbor;
      he._next                = _hedges[edge]._next;
      he._prev                = edge;
      _hedges[edge]._next     = ushort(_hedges.size());
      _hedges[he._next]._prev = ushort(_hedges.size());
      _hedges.pushBack(he);
      // add a neighbor edge.
      if( he._neighbor != NULL_ID )
      {
         uint nei                = he._neighbor;
         _hedges[nei]._neighbor  = ushort(_hedges.size())-1;
         he._neighbor            = edge;
         he._next                = _hedges[nei]._next;
         he._prev                = nei;
         _hedges[edge]._neighbor = ushort(_hedges.size());
         _hedges[nei]._next      = ushort(_hedges.size());
         _hedges[he._next]._prev = ushort(_hedges.size());
         _hedges.pushBack(he);
      }
   }

   void splitNextFace( uint e, uint flag )
   {
      uint nei = _hedges[e]._neighbor;
      if( nei == NULL_ID ) return;

      // Find second edge.
      uint ne = _hedges[nei]._next;

      // Find edge to split.
      ne = _hedges[ne]._next;
      if( _vertices[_hedges[ne]._vertex]._flag & 0x10 )
         ne = _hedges[ne]._next;
      // Is the edge already split?
      uint nne = _hedges[ne]._next;
      if( _vertices[_hedges[nne]._vertex]._flag & 0x10 ) return;
      // Split and propagate.
      splitEdge( ne, 0.5f, flag );
      splitNextFace( ne, flag );
   }

   void insertEdge( uint e0, uint e1 )
   {
      HEdge he;
      he._vertex   = _hedges[e0]._vertex;
      he._neighbor = ushort(_hedges.size()) + 1;
      he._next     = e1;
      he._prev     = _hedges[e0]._prev;
      _hedges.pushBack(he);
      HEdge nhe;
      nhe._vertex   = _hedges[e1]._vertex;
      nhe._neighbor = ushort(_hedges.size())-1;
      nhe._next     = e0;
      nhe._prev     = _hedges[e1]._prev;
      _hedges.pushBack(nhe);
      _hedges[he._prev]._next  = nhe._neighbor;
      _hedges[e1]._prev        = nhe._neighbor;
      _hedges[e0]._prev        = he._neighbor;
      _hedges[nhe._prev]._next = he._neighbor;
   }

   void clip( uint edge, const Vec3f& dir, uint mask )
   {
      float error = 0.0001f;
      Vec2f d0    = normalize( dir(_x,_y) );
      Vec3f ori   = _vertices[_hedges[edge]._vertex]._pos;

      // Find the correct edge so that dir and edge are in the same polygon.
      uint pedge  = _hedges[edge]._prev;
      Vec2f e0    = normalize( (_vertices[_hedges[_hedges[edge]._next]._vertex]._pos-ori)(_x,_y) );
      for( uint nedge = _hedges[pedge]._neighbor; nedge != NULL_ID;  )
      {
         // Check if dir is between edge and nedge.
         Vec2f e1 = normalize( (_vertices[_hedges[pedge]._vertex]._pos-ori)(_x,_y) );
         if( ccw( e0, d0, e1 ) ) break;

         // Next edge.
         edge  = nedge;
         pedge = _hedges[edge]._prev;
         nedge = _hedges[pedge]._neighbor;
         e0    = e1;
      }

      // Search in each edge for the nearest intersection.
      float t   = CGConstf::infinity();
      float t2  = 0.0f;
      uint mine = 0;
      int ve    = -1;
      for( uint e = _hedges[edge]._next; e != edge; e = _hedges[e]._next )
      {
         HEdge* he0 = &_hedges[e];
         HEdge* he1 = &_hedges[he0->_next];
         Vec3f v0   = _vertices[he0->_vertex]._pos;
         Vec3f v1   = _vertices[he1->_vertex]._pos;

         // Compute edge intersection.
         Vec2f d1   = (v1-v0)(_x,_y);
         Vec2f d2   = (ori-v0)(_x,_y);

         float den  = (d1.y*d0.x)-(d1.x*d0.y);
         float num0 = (d1.x*d2.y)-(d1.y*d2.x);
         float num1 = (d0.x*d2.y)-(d0.y*d2.x);

         if( den <= 0.0f ) continue;

         // Edges are parallel.
         if( CGM::equal( den, 0.0f ) )
         {
            // Edges overlapping.
            if( CGM::equal( num0, 0.0f ) && CGM::equal( num1, 0.0f ))
            {
               // Test only for v0. v1 will be test on another edge.
               float t0 = -dot( d2, d0 );
               if( t0 < error ) continue;
               if( t0 < t )
               {
                  t    = t0;
                  mine = e;
                  ve   = 0;
               }
            }
         }
         else
         {
            float t0 = num0/den;
            float t1 = num1/den;

            // Is the intersection in a valid range on dir?
            if( t0 < error || t0 > t ) continue;

            // Testing vertices.
            Vec3f pos = CGM::linear2( v0, v1, t1 );
            if( equal( pos, v0, error ) )
            {
               t    = t0;
               mine = e;
               ve   = 0;
            }
            else if( equal( pos, v1, error ) )
            {
               t    = t0;
               mine = _hedges[e]._next;
               ve   = 0;
            }
            // Testing edge.
            else if( t1 > 0.0f && t1 < 1.0f )
            {
               t    = t0;
               t2   = t1;
               mine = e;
               ve   = -1;
            }
         }
      }

      // Insert new edge.
      // 1. Add a new vertex if necessary by spliting.
      uint next = NULL_ID;
      if( ve < 0 )
      {
         splitEdge( mine, t2, 0 );
         next = _hedges[mine]._neighbor;
         mine = _hedges[mine]._next;
      }

      // 2. Insert edge.
      insertEdge( edge, mine );

      // 3. possibly remove flag of the destination vertex.
      _vertices[_hedges[mine]._vertex]._flag &= mask;

      // Continue clipping if not on exterior boundary.
      if( next != NULL_ID ) clip( next, dir, mask );
   }

   void buildFaces()
   {
      _faces.clear();
      Vector<bool> done( _hedges.size(), false );
      for( uint i = 0; i < _hedges.size(); ++i )
      {
         if( done[i] ) continue;
         // We have found a new poly, now construct it.
         Face face;
         face._e0   = i;
         face._size = 0;
         uint e     = i;
         do
         {
            ++face._size;
            done[e] = true;
            e       = _hedges[e]._next;
         } while( e != i );
         _faces.pushBack( face );
      }
   }

   void subdivideToQuads()
   {
      buildFaces();
      // 1. Split edges of non-quad faces.
      for( size_t f = 0; f < _faces.size(); ++f )
      {
         // Skip quad faces.
         if( _faces[f]._size == 4 ) continue;
         // Subdivide each original edges.
         uint e   = _faces[f]._e0;
         uint end = e;
         do
         {
            uint ne = _hedges[e]._next;
            // Has the edge already been split?
            if( _vertices[_hedges[ne]._vertex]._flag & 0x10 )
            {
               ne = _hedges[ne]._next;
            }
            else
            {
               // split edge.
               splitEdge( e, 0.5f, 0x10 );
               splitNextFace( e, 0x10 );
            }
            e = ne;
         } while( e != end );
      }
      // 2. Subdivide faces according to new vertices introduced in previous phase.
      uint midEdge[16];
      for( size_t f = 0; f < _faces.size(); ++f )
      {
         // Find mid-edge vertices.
         uint numME = 0;
         uint e     = _faces[f]._e0;
         uint end   = e;
         do
         {
            if( _vertices[_hedges[e]._vertex]._flag & 0x10 )
               midEdge[numME++] = e;
            e = _hedges[e]._next;
         } while( e != end );

         if( numME == 2 )
         {
            insertEdge( midEdge[0], midEdge[1] );
         }
         else
         {
            // Create vertex.
            ushort vid = ushort(_vertices.size());
            Vertex v;
            v._flag = 0x20;
            v._pos  = Vec3f(0.0f);
            // Create edges.
            ushort se = ushort(_hedges.size());
            for( uint i = 0; i < numME; ++i )
            {
               uint e0 = midEdge[i];
               v._pos += _vertices[_hedges[e0]._vertex]._pos;
               // First hedge.
               HEdge he;
               he._vertex   = _hedges[e0]._vertex;
               he._neighbor = se+2*i+1;
               he._next     = se+((i+numME-1)%numME)*2+1;
               he._prev     = _hedges[e0]._prev;
               _hedges.pushBack(he);
               // Second hedge.
               HEdge nhe;
               nhe._vertex   = vid;
               nhe._neighbor = se+2*i;
               nhe._next     = e0;
               nhe._prev     = se+((i+1)%numME)*2;
               _hedges.pushBack(nhe);
               _hedges[he._prev]._next  = nhe._neighbor;
               _hedges[e0]._prev        = he._neighbor;
            }

            // Add vertex.
            v._pos /= float(_faces[f]._size);
            _vertices.pushBack(v);
         }
      }
      // 3. Rebuild the faces.
      buildFaces();
   }

   RCP<DFGeometry> createExtrusion( const Vec3f& h )
   {
      // Create geometry.
      RCP<DFGeometry> geom = new DFGeometry();
      // 1. Create vertices.
      for( auto v = _vertices.begin(); v != _vertices.end(); ++v )
      {
         geom->addControlPoint( v->_pos );
         geom->addControlPoint( v->_pos + h );
      }
      // 2. Create side quads (needs some tagging on the edges).
      for( uint i = 0; i < _hedges.size(); ++i )
      {
         // Do we have an exterior edge?
         if( _hedges[i]._neighbor == NULL_ID )
         {
            uint32_t v0 = _hedges[i]._vertex*2;
            uint32_t v1 = _hedges[_hedges[i]._next]._vertex*2;
            geom->addPatch( v0, v1, v1+1, v0+1, 0xf );
         }
      }

      // 3. Create top/bottom quads.
      for( auto f = _faces.begin(); f != _faces.end(); ++f )
      {
         if( f->_size != 4 ) continue;
         uint e      = f->_e0;
         uint32_t v0 = _hedges[e]._vertex*2; e = _hedges[e]._next;
         uint32_t v1 = _hedges[e]._vertex*2; e = _hedges[e]._next;
         uint32_t v2 = _hedges[e]._vertex*2; e = _hedges[e]._next;
         uint32_t v3 = _hedges[e]._vertex*2;
         // Bottom.
         geom->addPatch( v0, v3, v2, v1, 0xf );
         // Top.
         geom->addPatch( v0+1, v1+1, v2+1, v3+1, 0xf );
      }

      geom->computeNeighbors();
      geom->subdivide( 0.01f );

      return geom;
   }

private:

   Planef         _plane;
   Vector<Vertex> _vertices;
   Vector<HEdge>  _hedges;
   Vector<Face>   _faces;
   int            _x;
   int            _y;
};


UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   NAMESPACE DFGeomOps
==============================================================================*/

//------------------------------------------------------------------------------
//!
RCP<DFGeometry>
DFGeomOps::extrude( const DFPolygon& poly, float height )
{
   Quadrangulator q;
   q.build( poly );
   q.quadrangulate( Vec3f(1.0f,0.0f,0.0f), Vec3f(0.f,1.0f,0.0f) );
   return q.createExtrusion( Vec3f( 0.0f, 0.0f, height ) );
}

//------------------------------------------------------------------------------
//!
RCP<DFGeometry>
DFGeomOps::clone( const DFGeometry& geom, uint num, const Vec3f& offset, const Reff& rot )
{
   if( num == 0 ) return nullptr;

   RCP<DFGeometry> g         = geom.clone();
   uint32_t numPatches       = g->numPatches();
   uint32_t numControlPoints = g->numControlPoints();
   uint32_t numTrimmings     = g->numTrimmings();

   g->reservePatches( numPatches * num );
   g->reserveControlPoints( numControlPoints * num );
   g->reserveTrimmings( numTrimmings * num );


   Mat4f rot1 = rot.orientation().toMatrix();
   Mat4f tb   = Mat4f::translation( -rot.position() );
   Mat4f rotc = Mat4f::identity();
   Vec3f offc = Vec3f(0.0f);

   for( uint i = 1; i < num; ++i )
   {
      // Offset patch, control point and trimming.
      uint32_t offPatch         = g->numPatches();
      uint32_t offControlPoints = g->numControlPoints();
      uint32_t offTrimming      = g->numTrimmings();

      // Transformation for current copy.
      rotc     = rot1 * rotc;
      offc    += rotc * offset;
      Mat4f tc = Mat4f::translation( offc + rot.position() );
      Mat4f m  = tc * rotc * tb;

      //Mat4f m = Mat4f::translation( offset*float(i) );

      // Control pts.
      for( uint32_t c = 0; c < numControlPoints; ++c )
      {
         g->addControlPoint( m*g->controlPoint(c) );
      }

      // Patches & subpatches.
      for( uint32_t p = 0; p < numPatches; ++p )
      {
         DFGeometry::Patch& patch = g->patch( g->addPatch( g->patch(p) ) );
         patch._controlPts[0]    += offControlPoints;
         patch._controlPts[1]    += offControlPoints;
         patch._controlPts[2]    += offControlPoints;
         patch._controlPts[3]    += offControlPoints;

         patch._neighbors[0]     += offPatch;
         patch._neighbors[1]     += offPatch;
         patch._neighbors[2]     += offPatch;
         patch._neighbors[3]     += offPatch;

         // Vertices.
         for( auto sp = patch._subpatches.begin(); sp != patch._subpatches.end(); ++sp )
         {
            (*sp)._trimming += offTrimming;
         }

         // Subpatches.
         for( auto v = patch._vertices.begin(); v != patch._vertices.end(); ++v )
         {
            (*v)._pos = m*(*v)._pos;
            //(*v)._n   = r*(*v)._n;
         }
      }

      // Trimmings.
      for( uint32_t t = 0; t < numTrimmings; ++t )
      {
         g->addTrimming( g->trimming(t) );
      }
   }

   return g;
}

NAMESPACE_END
