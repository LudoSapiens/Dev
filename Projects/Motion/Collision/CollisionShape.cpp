/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Motion/Collision/CollisionShape.h>
#include <Motion/Collision/CollisionInfo.h>
#include <Motion/Collision/BasicShapes.h>

#include <CGMath/CGMath.h>
#include <CGMath/Dist.h>
#include <CGMath/HEALPix.h>
#include <CGMath/Plane.h>

#include <Base/ADT/Vector.h>
#include <Base/ADT/Heap.h>
#include <Base/Dbg/DebugStream.h>
#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>

#include <cassert>


/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

DBG_STREAM( os_col, "Collision" );
DBG_STREAM( os_poly, "Polytope" );

/*==============================================================================
   CLASS TCSOVertex
==============================================================================*/

class TCSOVertex
{
public:

   /*----- methods -----*/

   TCSOVertex() {}
      
   inline void farthestPointAlong( 
      CollisionShape* a, const Reff& refA,
      CollisionShape* b, const Reff& refB,
      const Vec3f& dir
   )
   {
      _wPtA = a->getFarthestPointAlong( refA,  dir );
      _wPtB = b->getFarthestPointAlong( refB, -dir );
      _dir  = dir;
      recomputeTCSOPoint();
   }
   
   // dir Must be normalized!!!!
   inline void farthestPointAlongWithMargin( 
      CollisionShape* a, const Reff& refA,
      CollisionShape* b, const Reff& refB,
      const Vec3f& dir
   )
   {
      _wPtA = a->getFarthestPointAlong( refA,  dir ) + dir*a->margin();
      _wPtB = b->getFarthestPointAlong( refB, -dir ) - dir*b->margin();
      _dir  = dir;
      recomputeTCSOPoint();
   }
      
   inline void recomputeTCSOPoint() { _tcsoPt = _wPtA - _wPtB; }
   
   inline void addMargin( CollisionShape* a, CollisionShape* b )
   {
      _dir.normalize();
      _wPtA += _dir*a->margin();
      _wPtB -= _dir*b->margin();
      recomputeTCSOPoint();
   }
      
   // Accessors.
   inline const Vec3f&  pt() const { return _tcsoPt; }
   inline const Vec3f&  witnessPointA() const { return _wPtA; }
   inline const Vec3f&  witnessPointB() const { return _wPtB; }
   inline const Vec3f&  dir() const { return _dir; }

protected:

   /*----- data members -----*/
   
   Vec3f  _tcsoPt;
   Vec3f  _wPtA;
   Vec3f  _wPtB;
   Vec3f  _dir;

}; //class TCSOVertex

//------------------------------------------------------------------------------
//!
inline TextStream&
operator<<
( TextStream& stream, const TCSOVertex& tcso )
{
   return stream << "TCSO(" << tcso.pt() << "=" 
                 << tcso.witnessPointA() << "-" << tcso.witnessPointB() << ")";
}

/*==============================================================================
   CLASS Simplex
==============================================================================*/
//! A simplex structure structure is from 0 to 4 points.
//!   1: point
//!   2: line
//!   3: triangle
//!   4: tetrahedron
template< typename V >
class Simplex:
   public Vector< V >
{
public:
   Simplex<V>( const uint rsize = 4 ) { Vector<V>::reserve(rsize); }
   ~Simplex<V>() {}
}; //class Simplex

/*==============================================================================
  CLASS Polytope
==============================================================================*/
//! Simple polytope made from triangles with a directed edges structure.

class Polytope
{
public:
   
   /*----- types and enumerations ----*/

   typedef TCSOVertex Vertex;

   struct Triangle
   {
      float  _distance;
      Vec3f  _dir;
      ushort _v[3];
      bool   _active;
   };
   
   struct Comp
   {
      bool  operator()( const Triangle* lhs, const Triangle* rhs ) const
      {
         return rhs->_distance < lhs->_distance;  //a > b <==> b < a
      }
   };
   
   /*----- methods -----*/

   inline Polytope() :
      _numVertices(0),
      _numTriangles(0)
   {
   }

   inline void addVertex( const Vertex& v )
   {
      _vertices[_numVertices++] = v;
   }
   
   inline Triangle* addTriangle( uint v0, uint v1, uint v2, uint h0, uint h1, uint h2 )
   {
      _neighbors[_numTriangles*3+0] = h0;
      _neighbors[_numTriangles*3+1] = h1;
      _neighbors[_numTriangles*3+2] = h2;
      
      Triangle& tri = _triangles[_numTriangles++];
      tri._v[0] = v0;
      tri._v[1] = v1;
      tri._v[2] = v2;
      
      const Vec3f& p0 = _vertices[v0].pt();
      const Vec3f& p1 = _vertices[v1].pt();
      const Vec3f& p2 = _vertices[v2].pt();

      Planef plane( p0, p1, p2 );
      plane.normalize();
      tri._dir      = plane.direction();
      tri._distance = -plane.d();
      tri._active   = true;
      
      DBG_MSG( os_col, "e10: " << p1-p0 << " , e20: " << p2-p0 );
      DBG_MSG( os_col, "tri: " << p0 << ", " << p1 << " , " << p2 );

      return &tri;
   }
   
   
   inline int numVertices() const { return _numVertices; }
   inline int numTriangles() const { return _numTriangles; }
   
   // Accessors.
   inline const Vertex& vertex( uint idx ) const { return _vertices[idx]; }
   inline const Triangle& triangle( uint idx ) const { return _triangles[idx];  }
   
   // Adjacencies.
   inline uint nextEdge( uint he ) const { return (he%3)==2 ? he-2: he+1; }
   inline uint prevEdge( uint he ) const { return (he%3)==0 ? he+2: he-1; }
   inline uint neighbor( uint he ) const { return _neighbors[he]; }
   inline uint vertexIndexA( uint he ) const { return _triangles[he/3]._v[he%3]; }
   inline uint vertexIndexB( uint he ) const { return vertexIndexA( nextEdge(he) ); }
   inline void updateNeighbor( uint edge, uint neig ) { _neighbors[edge] = neig; }
   
   // Silhouette.
   inline void clearSilhouette()
   {
      _numSil = 0;
   }
   inline int silhouetteLength() const
   {
      return _numSil;
   }
   inline uint silhouette( uint i ) const { return _silhouette[i]; }
   
   void addSilhouette( uint he, const Vec3f& pt )
   {
      uint ne = neighbor(he);
      uint tri = ne / 3;
      
      if( _triangles[tri]._active )
      {
         if( pt.dot( _triangles[tri]._dir ) <= _triangles[tri]._distance )
         {
            _silhouette[_numSil++] = he;
         }
         else
         {
            _triangles[tri]._active = false;
            addSilhouette( nextEdge( ne ), pt );
            addSilhouette( prevEdge( ne ), pt );
         }
      }
   }

   
   void dump()
   {
      static int numDump = 1;
      
      if( numDump == 1 )
      {
         DBG_MSG( os_poly, "polytopes = {}" );
      }
      DBG_MSG( os_poly, "polytopes[" << numDump << "] = plasma.surface{" );
      
      DBG_MSG( os_poly, " mapping = {" );
      DBG_MSG( os_poly, "  {0,0}" );
      DBG_MSG( os_poly, " }," ); // End mapping.
      
      DBG_MSG( os_poly, " vertex = {" );
      for( int i = 0; i < _numVertices; ++i )
      {
         DBG_MSG( os_poly, "  {" << _vertices[i].pt().x << "," << _vertices[i].pt().y << "," << _vertices[i].pt().z << "}," );
      }
      DBG_MSG( os_poly, " }," ); // End vertex.
      
      DBG_MSG( os_poly, " patch = {" );
      DBG_MSG( os_poly, "  {" ); // Start patch 0.
      DBG_MSG( os_poly, "   mat = 0," );
      
      DBG_MSG( os_poly, "   face = {" );
      for( int i = 0; i < _numTriangles; ++i )
      {
         if( !_triangles[i]._active )
         {
            continue;
         }
         DBG_MSG( os_poly, "    { v={" << _triangles[i]._v[0] << "," << _triangles[i]._v[1] << "," << _triangles[i]._v[2] << "} }," );
      }
      DBG_MSG( os_poly, "   }" ); // End face.
      
      DBG_MSG( os_poly, "  }" ); // End patch 1.
      DBG_MSG( os_poly, " }" ); // End patch.
      
      DBG_MSG( os_poly, "}" ); // End surface.
      
      ++numDump;
   }
   
protected:
   
   /*----- data members -----*/

   int      _numVertices;
   int      _numTriangles;
   Vertex   _vertices[256];
   Triangle _triangles[256];
   uint     _neighbors[256*3];

   int      _numSil;
   uint     _silhouette[256];
}; 

//------------------------------------------------------------------------------
//! Finds the collision location(s) using the Edge-Expanding Polytope Algorithm
//! This is a variant of GJK-EPA (van den Bergen).
void findCollisionEPA( 
   CollisionShape* a, const Reff& refA,
   CollisionShape* b, const Reff& refB,
   Simplex<TCSOVertex>& simplex, 
   CollisionInfo& info
)
{
   DBG_BLOCK( os_col, "CollisionShape::findCollisionEPA" );
   
   // Enlarge simplex with margin.
   for( uint i = 0; i < simplex.size(); ++i )
   {
      simplex[i].addMargin( a, b );
   }
    
   // Handle non-tetrahedral simplex cases.
   switch( simplex.size() )
   {
      case 1:
      {
         info.addContact(
            CollisionInfo::Contact( 
               simplex[0].witnessPointA(),
               simplex[0].witnessPointB(),
               -simplex[0].dir(),
               refA.globalToLocal() * simplex[0].witnessPointA(),
               refB.globalToLocal() * simplex[0].witnessPointB()
            )
         );
         
         DBG_MSG( os_col, "Point case" );
         DBG_MSG( os_col, ">>> " << info );
         return;
      } break;
      case 2:
      {
         // Line (which contains the origin).
         DBG_MSG( os_col, "Line case" );

         // 1. Compute a perpendicular vector.
         Vec3f e01 = simplex[1].pt() - simplex[0].pt();
         Vec3f dir = Vec3f::perpendicular(e01).normalize();

         // 2. Find a new feature point on the TCSO.
         TCSOVertex v;
         v.farthestPointAlongWithMargin( a, refA, b, refB, dir );
         
         DBG_MSG( os_col, "E01: " << e01 );
         DBG_MSG( os_col, "Dir: " << dir );
         DBG_MSG( os_col, "V: " << v );

         // 3. Check if the new TCSO point is on the line.
         if( CGM::equal(dir.dot(v.pt()-simplex[0].pt()), 0.0f) )
         {
            // The origin sits on the TCSO (on an edge).
            // Find the interpolated point on the edge.
            float w = -simplex[0].pt()(0) / e01(0);

            // Note: we only do A, since B should be identical (depth=0).
            Vec3f p = CGM::linear(
               simplex[0].witnessPointA(),
               simplex[1].witnessPointA()-simplex[0].witnessPointA(),
               w
            );
            // Direction can't be determined (this should not be a collision).
            info.addContact( 
               CollisionInfo::Contact( 
                  p, p, Vec3f::zero(), 
                  refA.globalToLocal() * p,
                  refB.globalToLocal() * p
               )
            );
            
            DBG_MSG( os_col, "Origin is on TCSO edge" );
            DBG_MSG( os_col, ">>> " << info );
            return;
         }
         else
         {
            // Make the simplex a triangle.
            simplex.pushBack(v);
            
            DBG_MSG( os_col, "Fallthrough triangle case" );
         }
         // Let the triangle routine handle this below (next case statement).
      } /* do not break... fall through below */;
      case 3:
      {
         // Triangle.

         // 1. Find a ray perpendicular to the triangle.
         Vec3f e01 = simplex[1].pt() - simplex[0].pt();
         Vec3f e02 = simplex[2].pt() - simplex[0].pt();
         Vec3f dir = e01.cross(e02);

         // 2. Find a new feature point on the TCSO.
         TCSOVertex v;
         v.farthestPointAlongWithMargin( a, refA, b, refB, dir.getNormalized() );
         DBG_MSG( os_col, "V: " << v );

         // 3. Check if the TCSO point is on the triangle.
         if( CGM::equal( dir.dot(v.pt() - simplex[0].pt()), 0.0f ) )
         {
            // The origin lies on the triangle.

            // Find the barycentric coordinate corresponding to the origin.
            // The idea is to precalculate the total triangle area,
            // and then calculate 2 of the 3 sub-triangle areas.
            // Tricks:
            //  - The barycentric weights are area fractions.
            //  - A triangle area is half the norm of the cross of 2 edges.
            //  - We skip the halving of areas (balances out in the divide).
            //  - We avoid calculating the norm by using the dot with the normal.
            //  - 2xArea w/dot(normal) = (2xArea)^2 = 4xAreaSq.
            Vec3f e0p = v.pt() - simplex[0].pt();
            float oo_4xAreaSq = 1.0f / dir.dot(dir);
            float w2 = e01.cross(e0p).dot(dir) * oo_4xAreaSq;
            float w1 = e0p.cross(e02).dot(dir) * oo_4xAreaSq;

            // Note: we only do A, since B should be identical (depth=0).
            Vec3f p = simplex[0].witnessPointA() * (1.0f - w1 - w2)
                    + simplex[1].witnessPointA() * w1
                    + simplex[2].witnessPointA() * w2;
            // This should not be a collision.
            info.addContact( 
               CollisionInfo::Contact( 
                  p, p, -dir.getNormalized(),
                  refA.globalToLocal() * p,
                  refB.globalToLocal() * p
               ) 
            );
            
            DBG_MSG( os_col, "Origin is on TCSO triangle" );
            DBG_MSG( os_col, ">>> " << info );
            return;
         }
         else
         {
            DBG_MSG( os_col, "Fallthrough triangle case" );
            // Make the simplex a tetrahedron.
            simplex.pushBack(v);
         }
      } break;
      case 4:
      {
         // Nothing to do (continue past the switch..case statement).
      } break;
      default:
      {
         printf( "Simplex has an invalid number of vertices: %d\n", (int)simplex.size() );
         CHECK(false);
         return;
      }
   }

   // Here, the simplex has 4 vertices.
   //             V3
   //              . 
   //             /!;,
   //            / `;;,
   //           /   !;;;,
   //          /    `;;;;,
   //         /      !;;;;,
   //        /       `;;;;;) V2
   //       /         !;;;/
   //    V0 ``--__    `:;/
   //             ``--_!/
   //                V1
   //
   //     v3_________v2_________v3
   //       \ e10    /\  e6    /
   //        \   e11/  \    e7/
   //         \e9  /  e1\e8  /
   //          \  /e0    \  /
   //           \/___e2___\/
   //          v0\  e5    /v1
   //             \    e3/
   //              \e4  /
   //               \  /
   //                \/
   //                v3
   //

   // Initialize the polytope and the heap.
   typedef Heap< Polytope::Triangle*, Polytope::Comp >  HeapType;
   HeapType heap;
   Polytope poly;

   // 1. Vertices.
   poly.addVertex( simplex[0] );
   poly.addVertex( simplex[1] );
   poly.addVertex( simplex[2] );
   poly.addVertex( simplex[3] );

   // 2. Triangles.
   heap.push( poly.addTriangle( 0, 2, 1, 11, 8, 5 ) );
   heap.push( poly.addTriangle( 1, 3, 0,  7, 9, 2 ) );
   heap.push( poly.addTriangle( 2, 3, 1, 10, 3, 1 ) );
   heap.push( poly.addTriangle( 0, 3, 2,  4, 6, 0 ) );
   
   poly.dump();
   
   // Run the algorithm.
   TCSOVertex np;
   Polytope::Triangle* topTri = 0;
   const uint maxIterations   = 30;
   for( uint curIteration = 0; curIteration < maxIterations; )
   {
      topTri = heap.root();
      heap.pop();
      
      if( !topTri->_active )
      {
         continue;
      }
      
      DBG_BLOCK( os_col, "Iteration #" << curIteration );
      DBG_MSG( os_col, "Root triangle: " << (void*)(topTri - &poly.triangle(0)) << " dist=" << topTri->_distance );
      
      ++curIteration;
      
      // Find new point.
      Vec3f dir = topTri->_dir;
      np.farthestPointAlongWithMargin( a, refA, b, refB, dir );
      
      DBG_MSG( os_col, "dir: " << dir );

      // Test if point is far enough else break.
      float newDist = dir.dot( np.pt() );
      DBG_MSG( os_col, "New distance: " << newDist );
      
      const float frac = 1.005f;
      if( newDist <=  frac*topTri->_distance )
      {
         DBG_MSG( os_col, "Finish!!!" );
         break;
      }
      
      DBG_MSG( os_col, np );
      
      // Expand polytope.
      uint tri = topTri - &poly.triangle(0);
      
      // 1. Compute visible silhouette.
      topTri->_active = false;
      poly.clearSilhouette();
      poly.addSilhouette( tri*3 + 0, np.pt() );
      poly.addSilhouette( tri*3 + 1, np.pt() );
      poly.addSilhouette( tri*3 + 2, np.pt() );
      
      // 2. Add new vertex.
      int npId = poly.numVertices();
      poly.addVertex( np );
      
      // 3. Create triangle strip.
      uint startEdge = poly.numTriangles()*3;
      uint lastEdge  = (poly.numTriangles() + poly.silhouetteLength() )*3 - 1;
      
      uint prevEdge = lastEdge;
      uint nextEdge;
      
      DBG_MSG( os_col, "silhouetteLength: " << poly.silhouetteLength() );
      
      for( int i = 0; i < poly.silhouetteLength(); ++i )
      {
         uint newEdge = poly.numTriangles() * 3;
         uint silhouetteEdge = poly.silhouette(i);
         
         DBG_MSG( os_col, "sil: " << silhouetteEdge );
         
         nextEdge = newEdge+3;
         if( nextEdge > lastEdge )
         {
            nextEdge = startEdge;
         }
         
         // Create triangle.
         heap.push( 
            poly.addTriangle(
               npId,
               poly.vertexIndexA( silhouetteEdge ),
               poly.vertexIndexB( silhouetteEdge ),
               prevEdge, poly.neighbor( silhouetteEdge ), nextEdge
            )
         );
         
         prevEdge = newEdge+2;
         
         // Update neighbor.
         poly.updateNeighbor( poly.neighbor( silhouetteEdge ), newEdge+1 ); 
      }
      
      poly.dump();
   } 
   
   // Compute contact points.
   const TCSOVertex& tcsoV0 = poly.vertex( topTri->_v[0] );
   const TCSOVertex& tcsoV1 = poly.vertex( topTri->_v[1] );
   const TCSOVertex& tcsoV2 = poly.vertex( topTri->_v[2] );
   const Vec3f& v0 = tcsoV0.pt();
   const Vec3f& v1 = tcsoV1.pt();
   const Vec3f& v2 = tcsoV2.pt();
   Vec3f e01       = v1 - v0;
   Vec3f e02       = v2 - v0;
   Vec3f n         = e01.cross(e02);

   // Evaluate collision point using barycentric interpolation.
   Vec3f colPointA, colPointB;
   
   float oo_4xAreaSq = -1.0f / n.dot(n);
   float w2 = (e01.cross(v0)).dot(n) * oo_4xAreaSq;
   float w1 = (v0.cross(e02)).dot(n) * oo_4xAreaSq;
   float w0 = 1.0f - w1 - w2;
   colPointA = tcsoV0.witnessPointA() * w0
             + tcsoV1.witnessPointA() * w1
             + tcsoV2.witnessPointA() * w2;
   colPointB = tcsoV0.witnessPointB() * w0
             + tcsoV1.witnessPointB() * w1
             + tcsoV2.witnessPointB() * w2;


   Vec3f colDir = colPointB - colPointA;
   colDir.normalize();
   DBG_MSG( os_col, "Contact is at:"
                 << " ptA" << colPointA  << " ptB" << colPointB  << " dir" << colDir
                 << " depth=" << topTri->_distance );
   info.addContact( 
      CollisionInfo::Contact( 
         colPointA, colPointB, colDir,
         refA.globalToLocal() * colPointA,
         refB.globalToLocal() * colPointB
      )
   );
}

//------------------------------------------------------------------------------
//!
inline void
handleTriangle( Simplex<TCSOVertex>& simplex, Vec3f& dir )
{
   // Compute triangle direction.
   Vec3f e21  = simplex[1].pt() - simplex[2].pt();
   Vec3f e20  = simplex[0].pt() - simplex[2].pt();
   Vec3f t012 = e20.cross( e21 );
   
   Vec3f o = -simplex[2].pt();
   
   if( e20.cross(t012).dot(o) > 0.0f )
   {
      if( e20.dot(o) > 0.0f )
      {
         dir = e20.cross(o).cross(e20);
         simplex[1] = simplex[2];
         simplex.popBack();
         
         DBG_MSG( os_col, "Keeping E20" );
         DBG_MSG( os_col, "dir: " << dir );
      }
      else
      if( e21.dot(o) > 0.0f )
      {
         dir = e21.cross(o).cross(e21);
         simplex[0] = simplex[1];
         simplex[1] = simplex[2];
         simplex.popBack();
         
         DBG_MSG( os_col, "Keeping E21" );
         DBG_MSG( os_col, "dir: " << dir );
      }
      else
      {
         dir = o;
         simplex[0] = simplex[2];
         simplex.popBack();
         simplex.popBack();
         
         DBG_MSG( os_col, "Keeping 2" );
         DBG_MSG( os_col, "dir: " << dir );
      }
   }
   else
   {
      if( t012.cross(e21).dot(o) > 0.0f )
      {
         if( e21.dot(o) > 0.0f )
         {
            dir = e21.cross(o).cross(e21);
            simplex[0] = simplex[1];
            simplex[1] = simplex[2];
            simplex.popBack();
            
            DBG_MSG( os_col, "Keeping E21" );
            DBG_MSG( os_col, "dir: " << dir );
         }
         else
         {
            dir = o;
            simplex[0] = simplex[2];
            simplex.popBack();
            simplex.popBack();
            
            DBG_MSG( os_col, "Keeping 2" );
            DBG_MSG( os_col, "dir: " << dir );
         }
      }
      else
      {
         DBG_MSG( os_col, "T012: " << t012 );
         DBG_MSG( os_col, "dot: " << t012.dot(o) );
         if( t012.dot(o) > 0.0f )
         {
            dir = t012;

            DBG_MSG( os_col, "Keeping T012" );
            DBG_MSG( os_col, "dir: " << dir );
         }
         else
         {
            dir = -t012;
            CGM::swap( simplex[0], simplex[1] );

            DBG_MSG( os_col, "Keeping T102" );
            DBG_MSG( os_col, "dir: " << dir );
         }
      }
   }
}

//------------------------------------------------------------------------------
//!
inline bool
encloseOrigin( Simplex<TCSOVertex>& simplex, Vec3f& dir )
{
   switch( simplex.size() )
   {
      case 2:
      {
         Vec3f e10 = simplex[0].pt() - simplex[1].pt();
         Vec3f o   = -simplex[1].pt();
         if( e10.dot(o) > 0.0f )
         {
            dir = e10.cross(o).cross(e10);
         }
         else
         {
            simplex[0] = simplex[1];
            simplex.popBack();
            dir = o;
         }
       
         DBG_MSG( os_col, "Line case." );
         DBG_MSG( os_col, "1: " << simplex[1].pt() << " 0: " << simplex[0].pt() );
         DBG_MSG( os_col, "dir: " << dir );
      } break;
      case 3:
      {
         DBG_MSG( os_col, "Triangle case." );

         handleTriangle( simplex, dir );
      } break;
      case 4:
      {
         DBG_MSG( os_col, "Tetrahedron case." );
         
         // Test if the origin is enclosed in the tetrahedron.
         // - If it is, we have a collision,
         // - otherwise, kill the farthest point, and find the next.
         
         //Test triangle 013
         Planef plane( simplex[0].pt(), simplex[1].pt(), simplex[3].pt() );
         if( plane.inFront(Vec3f::zero()) )
         {
            DBG_MSG( os_col, "Triangle 013" );
            // Going this way will get us closer to the origin,
            // so let's get rid of [2].
            simplex[2] = simplex[3];
            simplex.popBack();
            
            // FIXME: Duplicate computation of triangle direction.
            handleTriangle( simplex, dir );
            break;
         }

         // Test triangle 123.
         plane = Planef( simplex[1].pt(), simplex[2].pt(), simplex[3].pt() );
         if( plane.inFront(Vec3f::zero()) )
         {
            DBG_MSG( os_col, "Triangle 123" );
            // Going this way will get us closer to the origin
            // so let's get rid of [0].
            simplex[0] = simplex[1];
            simplex[1] = simplex[2];
            simplex[2] = simplex[3];
            simplex.popBack();
            handleTriangle( simplex, dir );
            break;
         }

         // Test triangle 203.
         plane = Planef( simplex[2].pt(), simplex[0].pt(), simplex[3].pt() );
         if( plane.inFront(Vec3f::zero()) )
         {
            DBG_MSG( os_col, "Triangle 203" );
            // Going this way will get us closer to the origin
            // so let's get rid of [1].
            simplex[1] = simplex[0];
            simplex[0] = simplex[2];
            simplex[2] = simplex[3];
            simplex.popBack();
            handleTriangle( simplex, dir );
            break;
         }

         // Getting here indicates we have the origin enclosed.
         DBG_MSG( os_col, "0 inside" );
         return true;
      } break;
      default: 
         assert(0);
         break;
   }
   
   return false;
}

UNNAMESPACE_END


NAMESPACE_BEGIN


const float sLengthErr = 1e-6f;
const float sSqrLengthErr = sLengthErr*sLengthErr;

/*==============================================================================
   CLASS CollisionShape
==============================================================================*/

//------------------------------------------------------------------------------
//!
bool
CollisionShape::collide( CollisionPair& pair )
{
   DBG_BLOCK( os_col, "CollisionShape::Collide" );

   return collide( pair.bodyA()->shape(), pair.bodyA()->simReferential(),
                   pair.bodyB()->shape(), pair.bodyB()->simReferential(),
                   *(pair.allocateInfo()) );
}

//------------------------------------------------------------------------------
//!
bool
CollisionShape::collide( 
   CollisionShape* shapeA, const Reff& refA,
   CollisionShape* shapeB, const Reff& refB,
   CollisionInfo& info
)
{
   DBG_BLOCK( os_col, "CollisionShape::Collide" );

   bool ret = false;

   bool reverse;
   const Reff* shRefA;
   const Reff* shRefB;
   if( shapeB->_type < shapeA->_type )
   {
      // Swap them to guarantee that a<=b.
      CollisionShape* tmp = shapeA;
      shapeA  = shapeB;
      shapeB  = tmp;
      shRefA  = &refB;
      shRefB  = &refA;
      reverse = true;
   }
   else
   {
      shRefA  = &refA;
      shRefB  = &refB;
      reverse = false;
   }
   
   switch( shapeA->_type )
   {
      case GROUP:
      {
         CollisionGroup* group = (CollisionGroup*)shapeA;
         for( uint i = 0; i < group->numShapes(); ++i )
         {
            CollisionShape* shapeA_ = group->shape(i);
            Reff shRefA_ = (*shRefA) * group->shapeRef(i);
            ret |= collide( shapeA_, shRefA_, shapeB, *shRefB, info );
         }
      }  break;
      case SPHERE:
      {
         switch( shapeB->_type )
         {
            case GROUP:
            {
               CollisionGroup* group = (CollisionGroup*)shapeB;
               for( uint i = 0; i < group->numShapes(); ++i )
               {
                  CollisionShape* shapeB_ = group->shape(i);
                  Reff shRefB_ = (*shRefB) * group->shapeRef(i);
                  ret |= collide( shapeA, *shRefA, shapeB_, shRefB_, info );
               }
            }  break;
            case SPHERE: ret = collideSphereSphere( (SphereShape*)shapeA, *shRefA, (SphereShape*)shapeB, *shRefB, info, reverse ); break;
            case BOX:    ret = collideSphereBox( (SphereShape*)shapeA, *shRefA, (BoxShape*)shapeB, *shRefB, info, reverse );    break;
            default:     ret = collideGJK( shapeA, *shRefA, shapeB, *shRefB, info );         break;
         }
      }  break;
      default: ret = collideGJK( shapeA, *shRefA, shapeB, *shRefB, info ); break;
   }

   return ret;
}

//------------------------------------------------------------------------------
//! Collides 2 spheres together.
//! The collision, if it exists, is single, and
//! should push object 'A' in the direction of the specified normal.
bool
CollisionShape::collideSphereSphere( 
   SphereShape* sphA, const Reff& refA,
   SphereShape* sphB, const Reff& refB,
   CollisionInfo& info,
   bool reverse
)
{
   // Trace a vector from B to A.
   Vec3f ba = refA.position() - refB.position();

   // Get its length squared (avoids the sqrt if no collision).
   float dist_sq = ba.sqrLength();

   float r = sphA->radius() + sphB->radius();
   float r_sq = r * r;

   // Check if both spheres are close enough for a contact (in squared distances).
   if( dist_sq <= r_sq )
   {
      // Reusing ba for the normal.
      if( dist_sq == 0 )
      {
         // Special case where both spheres are exactly at the same location.

         // Need to set a non-zero normal to incur a movement.
         // Should probably alternate directions to even out energy.
         ba = Vec3f(0.0f, 1.0f, 0.0f);
      }
      else
      {
         // Calculate the true distance between the centers.
         float dist = CGM::sqrt(dist_sq);

         // Normalize ba.
         ba *= (1.0f/dist);
      }
      
      Vec3f posA = refA.position() - ba * sphA->radius();
      Vec3f posB = refB.position() + ba * sphB->radius();
      
      Vec3f localPosA = refA.globalToLocal() * posA;
      Vec3f localPosB = refB.globalToLocal() * posB;
      
      if( reverse )
      {
         info.addContact( CollisionInfo::Contact( posB, posA, -ba, localPosB, localPosA ) );
      }
      else
      {
         info.addContact( CollisionInfo::Contact( posA, posB, ba, localPosA, localPosB ) );
      }
      
      return true;
   }
   
   return false;
}

//------------------------------------------------------------------------------
//! From ODE.
bool
CollisionShape::collideSphereBox( 
   SphereShape* sphere, const Reff& sphereRef,
   BoxShape*    box,    const Reff& boxRef,
   CollisionInfo& info,
   bool reverse
)
{
   int onborder = 0;
   
   Vec3f p = sphereRef.position() - boxRef.position();
   Vec3f l = box->size() * 0.5f;
   
   Mat3f xform = boxRef.orientation().toMatrix3();
   Vec3f t = xform.getTransposed() * p;

   if( t.x < -l.x ) { t.x = -l.x; onborder = 1; }
   if( t.x >  l.x ) { t.x =  l.x; onborder = 1; }
   
   if( t.y < -l.y ) { t.y = -l.y; onborder = 1; }
   if( t.y >  l.y ) { t.y =  l.y; onborder = 1; }
   
   if( t.z < -l.z ) { t.z = -l.z; onborder = 1; }
   if( t.z >  l.z ) { t.z =  l.z; onborder = 1; }
   
   // Sphere is inside box?
   if( !onborder )
   {
      float minDistance = l.x - CGM::abs( t.x );
      int minI = 0;
      for( int i = 1; i < 3; ++i )
      {
         float faceDistance = l(i) - CGM::abs( t(i) );
         if( faceDistance < minDistance )
         {
            minDistance = faceDistance;
            minI = i;
         }
      }
      
      Vec3f normal( 0.0f, 0.0f, 0.0f );
      normal(minI) = CGM::sign( t(minI) );
      normal = xform * normal;
      
      Vec3f posA = sphereRef.position() - normal * sphere->radius();
      Vec3f posB = sphereRef.position() + normal * minDistance; // FIXME BoxRef?
      
      Vec3f localPosA = sphereRef.globalToLocal() * posA;
      Vec3f localPosB = boxRef.globalToLocal() * posB;
      
      if( reverse )
      {
         info.addContact( CollisionInfo::Contact( posB, posA, -normal, localPosB, localPosA ) );
      }
      else
      {
         info.addContact( CollisionInfo::Contact( posA, posB, normal, localPosA, localPosB ) );
      }
      return true;
   }
   
   Vec3f q = xform * t;
   Vec3f r = p - q;
   
   float depth = sphere->radius() - r.length();
   
   if( depth < 0.0f )
   {
      return false;
   }
   
   r.normalize();
      
   Vec3f posA = sphereRef.position() - r * sphere->radius();
   Vec3f posB = sphereRef.position() - r * ( sphere->radius() - depth );
   
   Vec3f localPosA = sphereRef.globalToLocal() * posA;
   Vec3f localPosB = boxRef.globalToLocal() * posB;
   
   if( reverse )
   {
      info.addContact( CollisionInfo::Contact( posB, posA, -r, localPosB, localPosA ) );
   }
   else
   {
      info.addContact( CollisionInfo::Contact( posA, posB, r, localPosA, localPosB ) );
   }
  
   return true;
}

//------------------------------------------------------------------------------
//!
//! Performs a collision check using the Gilbert-Johnson-Keerthi algorithm.
bool
CollisionShape::collideGJK( 
   CollisionShape* a, const Reff& refA,
   CollisionShape* b, const Reff& refB,
   CollisionInfo& info
)
{
   DBG_BLOCK( os_col, "CollisionShape::collideGJK" );

   Vec3f dir = info.separatingAxis();

   Simplex<TCSOVertex> simplex;
   TCSOVertex p;
   
   const uint maxIterations = 20;
   float marginsSqr = a->margin() + b->margin() + 0.001f;
   marginsSqr *= marginsSqr;
   
   // Compute first simplex point.
   p.farthestPointAlong( a, refA, b, refB, dir );
   simplex.pushBack(p);
   dir = -p.pt();
   
   DBG_MSG( os_col, p );
   DBG_MSG( os_col, "dir: " << dir );
   
   // Try to enclose the origin within a simplex.
   for( uint i = 0; i < maxIterations; ++i )
   {
      float dirSqr = dir.sqrLength();
      
      // 1. Check for degenerate case(s).
      if( dirSqr < sSqrLengthErr )
      {
         DBG_MSG( os_col, "SPECIAL CASE: degenerate direction (0,0,0) after " << i << " iterations." );
         findCollisionEPA( a, refA, b, refB, simplex, info );
         return true;
      }
      
      
      // 2. Get new candidate point on the TCSO using the Minkowski sum.
      p.farthestPointAlong( a, refA, b, refB, dir );
      DBG_MSG( os_col, "P: " << p );
      
      // 3. Check if intersection is possible.
      float dotDirP = dir.dot( p.pt() );
      DBG_MSG( os_col, dotDirP*dotDirP << " > " << marginsSqr*dirSqr << " ?" );
      if( dotDirP < 0.0f && dotDirP*dotDirP > marginsSqr*dirSqr )
      {
         DBG_MSG( os_col, "No intersection after " << i << " iterations." );
         info.separatingAxis( dir );
         return false;
      }
      
      // 4. Check if we have an intersection in the margin.
      const float frac = (0.001f*0.001f);
      float dotDirS = dir.dot( simplex[0].pt() );
      if( dotDirS - dotDirP >= frac*dotDirS )
      {
         DBG_MSG( os_col, "Found distance between objects a and b in " << i << " iterations." );
         
         // Compute the collision points.
         Vec3f ptA;
         Vec3f ptB;
         switch( simplex.size() )
         {
            case 1:
            {
               DBG_MSG( os_col, "Simplex 1" );
               ptA = simplex[0].witnessPointA();
               ptB = simplex[0].witnessPointB();
            } break;
            case 2:
            {
               DBG_MSG( os_col, "Simplex 2" );
               Vec3f e01 = simplex[1].pt() - simplex[0].pt();
               float t   = -simplex[0].pt().dot(e01) / e01.sqrLength();
               ptA = simplex[0].witnessPointA() * (1.0f-t) + simplex[1].witnessPointA() * t;
               ptB = simplex[0].witnessPointB() * (1.0f-t) + simplex[1].witnessPointB() * t;
               
               DBG_MSG( os_col, "A0: " << simplex[0].witnessPointA() << " A1: " <<simplex[1].witnessPointA() );
               DBG_MSG( os_col, "B0: " << simplex[0].witnessPointB() << " B1: " <<simplex[1].witnessPointB() );
               DBG_MSG( os_col, "t: " << t << " A: " << ptA << " B: " << ptB );
            } break;
            case 3:
            {
               DBG_MSG( os_col, "Simplex 3" );
               Vec3f e01 = simplex[1].pt() - simplex[0].pt();
               Vec3f e02 = simplex[2].pt() - simplex[0].pt();
               float oo_4xAreaSq = -1.0f / dirSqr;
               float w2 = (e01.cross(simplex[0].pt())).dot(dir) * oo_4xAreaSq;
               float w1 = (simplex[0].pt().cross(e02)).dot(dir) * oo_4xAreaSq;
               float w0 = 1.0f - w1 - w2;
               ptA = simplex[0].witnessPointA() * w0 +
                     simplex[1].witnessPointA() * w1 +
                     simplex[2].witnessPointA() * w2;
               ptB = simplex[0].witnessPointB() * w0 +
                     simplex[1].witnessPointB() * w1 +
                     simplex[2].witnessPointB() * w2;
            } break;
         }
         // Enlarge pts by margins.
         dir.normalize();
         ptA += dir*a->margin();
         ptB -= dir*b->margin();
         
         Vec3f localPtA = refA.globalToLocal() * ptA;
         Vec3f localPtB = refB.globalToLocal() * ptB;
         
         info.addContact( CollisionInfo::Contact( ptA, ptB, -dir, localPtA, localPtB ) );
         DBG_MSG( os_col, "a: " << ptA << " b: " << ptB << " n: " << -dir );
         info.separatingAxis( dir );
         return true;
      }
      
      // 5. Add that TCSO point to the simplex.
      simplex.pushBack(p);
   
      // 6. Update simplex and direction.
      if( encloseOrigin( simplex, dir ) )
      {
         DBG_MSG( os_col, "Intersection found in " << i << " iterations." );
         findCollisionEPA( a, refA, b, refB, simplex, info );
         info.separatingAxis( dir );
         return true;
      }
   }

   return false;
}

//------------------------------------------------------------------------------
//!
CollisionShape::CollisionShape( uint type ) :
   _type( type ),
   _margin( 0.0f ),
   _bbox( AABBoxf::empty() )
{
}

//------------------------------------------------------------------------------
//!
CollisionShape::~CollisionShape()
{
}

NAMESPACE_END
