/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Procedural/BSP3.h>
#include <Plasma/Procedural/BSP2.h>

#include <Base/ADT/List.h>

#include <Base/Dbg/DebugStream.h>

#include <algorithm>

/*==============================================================================
   UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

DBG_STREAM( os_bsp3, "BSP3" );

enum {
   ON,
   ON_FLIPPED,
   FRONT,
   BACK,
   SPANNING
};

enum {
   IN,
   OUT
};

#if 0
//------------------------------------------------------------------------------
//!
bool compArea( const RCP<BoundaryPolygon>& a, const RCP<BoundaryPolygon>& b )
{
   return a->area() > b->area();
}

//------------------------------------------------------------------------------
//!
uint
regionStatus( 
   const Planef&                         plane,
   const Vec3f&                          p,
   const Vector< RCP<BoundaryPolygon> >& faces, 
   float                                 epsilon
)
{
   // Find the nearest face: 
   // -The most parallel face which contains the nearest vertex.
   float minDist  = CGConstf::infinity();
   uint minVertex = 0;
   uint minFace   = 0;
   for( uint f = 0; f < faces.size(); ++f )
   {
      // Compute polygon distance.
      float polyDist = CGConstf::infinity();
      uint v         = 0;
      for( uint i = 0; i < faces[f]->numVertices(); ++i )
      {
         float dist = CGM::abs( plane.distance( faces[f]->vertex(i) ) );
         if( dist < polyDist )
         {
            polyDist = dist;
            v        = i;
         }
      }
      
      // Keep the nearest polygon.
      if( polyDist < minDist-epsilon )
      {
         minDist   = polyDist;
         minFace   = f;
         minVertex = v;
      }
   }

   // Find the vertex on another face.
   for( uint f = 0; f < faces.size(); ++f )
   {
      if( f == minFace ) continue;
      for( uint i = 0; i < faces[f]->numVertices(); ++i )
      {
         if( equal( faces[f]->vertex(i), faces[minFace]->vertex(minVertex), epsilon ) )
         {
            return ( faces[minFace]->plane().distance( faces[f]->computeCentroid() ) > 0.0f ) ? IN : OUT;
         }
      }
   }

   return ( faces[minFace]->plane().distance( p ) > 0.0f ) ? OUT : IN;
}
#endif

//------------------------------------------------------------------------------
//!
uint 
classify( const Planef& plane, const BoundaryPolygon& poly, float epsilon )
{
   DBG_BLOCK( os_bsp3, "classify" );
   DBG_MSG( os_bsp3, plane << poly.plane() );

   // Find the closest vertex to the plane.
   float minDist = CGConstf::infinity();
   float maxDist = -CGConstf::infinity();
   
   for( uint i = 0; i < poly.numVertices(); ++i )
   {
      float dist = plane.distance( poly.vertex(i) );
      if( dist < minDist ) minDist = dist;
      if( dist > maxDist ) maxDist = dist;
   }
   
   DBG_MSG( os_bsp3, "min: " << minDist << " max: " << maxDist << " e:" << epsilon );

   // Evaluate polygon classification.
   if( CGM::abs(maxDist) < epsilon && CGM::abs(minDist) < epsilon )
   {
      if( poly.normal().dot( plane.direction() ) > 0.0f )
      {
         return ON;
      }
      else
      {
         return ON_FLIPPED;
      }
   }
   else if( maxDist < epsilon )
   {
      return BACK;
   }
   else if( minDist > -epsilon )
   {
      return FRONT;
   }
   return SPANNING;
}

//------------------------------------------------------------------------------
//!
void 
split( 
   const BoundaryPolygon& poly, 
   const Planef&          plane, 
   BoundaryPolygon&       frontFace, 
   BoundaryPolygon&       backFace, 
   float                  epsilon
)
{
   size_t count = poly.numVertices();
   Vec3f ptA    = poly.vertex( count-1 );
   double distA = plane.distance( ptA );
   
   for( size_t i = 0; i < count; ++i )
   {
      Vec3f ptB    = poly.vertex(i);
      double distB = plane.distance( ptB );
      if( distB > epsilon )
      {
         if( distA < -epsilon )
         {
            Vec3d dir  = ptB - ptA;
            Vec3f pt   = ptA - dir*(distA / dir.dot(plane.direction()));
            frontFace.addVertex( pt );
            backFace.addVertex( pt );
         }
         frontFace.addVertex( ptB );
      }
      else if( distB < -epsilon )
      {
         if( distA > epsilon )
         {
            Vec3d dir  = ptB - ptA;
            Vec3f pt   = ptA - dir*(distA / dir.dot(plane.direction()));
            frontFace.addVertex( pt );
            backFace.addVertex( pt );
         }
         backFace.addVertex( ptB );
      }
      else
      {
         frontFace.addVertex( ptB );
         backFace.addVertex( ptB );
      }
      ptA   = ptB;
      distA = distB;
   }
   frontFace.id( poly.id() );
   backFace.id( poly.id() );
   frontFace.plane( poly.plane() );
   backFace.plane( poly.plane() );
}

//------------------------------------------------------------------------------
//!
void
removeTVertices( const Vector< RCP<BoundaryPolygon> >& faces, float precision )
{
   DBG_BLOCK( os_bsp3, "removeTVertices" );
   
   if( faces.size() <= 1 ) return;
   
   for( uint i = 0; i < faces.size(); ++i )
   {
      DBG_MSG( os_bsp3, "doing poly " << i );
      BoundaryPolygon* face1 = faces[i].ptr();
      CHECK( face1->numVertices() > 2 );
      Vec3f v0 = face1->vertex( face1->numVertices()-1 );
      for( uint v = 0; v < face1->numVertices(); ++v )
      {
         Vec3f v1  = face1->vertex(v);
         Vec3f dv1 = v1-v0;
         float dn1 = 1.0f / dv1.sqrLength();

         DBG_MSG( os_bsp3, "Testing edge: " << v0 << v1 );
         
         // Test edge v against all vertices of all other polygons.
         for( uint j = 0; j < faces.size(); ++j )
         {
            // Do not test the same polygon.
            if( i == j ) continue;
            BoundaryPolygon* face2 = faces[j].ptr();
            for( uint w = 0; w < face2->numVertices(); ++w )
            {
               // Is vertex w on edge v ?
               Vec3f v2  = face2->vertex(w);
               Vec3f dv2 = v2-v0;
               float t   = dv1.dot(dv2)*dn1;
               
               if( CGConstf::epsilon(64.0f) < t && t < (1.0f-CGConstf::epsilon(64.0f)) )
               {
                  DBG_MSG( os_bsp3, "T: " << t << " (" << CGConstf::epsilon(64.0f) << ", " << (1.0f-CGConstf::epsilon(64.0f)) << ")" );
                  DBG_MSG( os_bsp3, "posible intersection:" << v0 << v1 << v2 );
                  float dn2 = 1.0f / dv2.sqrLength();
                  if( CGM::equal( dv2.dot(dv1*t)*dn2, 1.0f ) )
                  {
                     if( !v2.equal( v0, precision ) && !v2.equal( v1, precision ) )
                     {
                        DBG_MSG( os_bsp3, "inserting vertex " << v2 );
                        face1->insertVertex( v, v2 );
                        v1  = v2;
                        dv1 = dv2;
                        dn1 = dn2;   
                     }
                  }
               }
            }
         }
         
         v0 = v1;
      }
   }
}

/*==============================================================================
   CLASS Reducer
==============================================================================*/

class Reducer
{
public:
   
   /*----- classes -----*/
   
   struct Edge {
      Edge( uint v0, uint v1 ) :
         _v0(v0), _v1(v1), _count(1)
      {}
         
      bool equal( uint v0, uint v1 )
      {
         return ( _v0==v0 && _v1==v1 ) || ( _v1==v0 && _v0==v1 );
      }
      
      uint _v0;
      uint _v1;
      uint _count;
   };

   /*----- methods -----*/

   Reducer( float precision ) : _precision( precision ) {}

   void id( const ConstString& str ) { _id = str; }

   void clear()
   {
      _vertices.clear();
      _edges.clear();
   }
   
   uint add( const Vec3f& v )
   {
      const uint n = uint(_vertices.size());
      for( uint i = 0; i < n; ++i )
      {
         if( _vertices[i].equal( v, _precision ) ) return i;
      }
      _vertices.pushBack( v );
      return n;
   }
   
   uint add( uint v0, uint v1 )
   {
      DBG_BLOCK( os_bsp3, "add edge" );
      DBG_MSG( os_bsp3, _vertices[v0] << " " << _vertices[v1] );
      const uint n = uint(_edges.size());
      for( uint i = 0; i < n; ++i )
      {
         if( _edges[i].equal( v0, v1 ) )
         {
            _edges[i]._count++;
            return i;
         }
      }
      _edges.pushBack( Edge( v0, v1 ) );
      return n;
   }
   
   void reduce()
   {
      DBG_BLOCK( os_bsp3, "reduce" );
      DBG_MSG( os_bsp3, "#vertices: " << _vertices.size() );
      const uint n = uint(_vertices.size());
      for( uint i = 0; i < n; ++i )
      {
         DBG_MSG( os_bsp3, " " << _vertices[i] );
      }

      // Remove all edges with count > 1.
      // Find t.
      Vector<Edge>::Iterator it = _edges.begin();
      for( ; it != _edges.end(); ++it )
      {
         if( (*it)._count > 1 )
         {
            DBG_MSG( os_bsp3, "found a duplicated edge" );
            break;
         }
      }
   
      // No duplicated edge found.
      if( it == _edges.end() ) return;

      // Remove all occurance.
      Vector<Edge>::Iterator cur = it;
      for( ++it; it != _edges.end(); ++it )
      {
         if( (*it)._count == 1 )
         {
            DBG_MSG( os_bsp3, "copying over" );
            *cur = *it;
            ++cur;
         }
         else
         {
            DBG_MSG( os_bsp3, "found a duplicated edge." );
         }
      }
   
      // Erase all elements.
      _edges.erase( cur, _edges.end() );
   }
   
   bool parallel( uint e0, uint e1 ) const
   {
      Vec3f dir0 = _vertices[_edges[e0]._v1] - _vertices[_edges[e0]._v0];
      Vec3f dir1 = _vertices[_edges[e1]._v1] - _vertices[_edges[e1]._v0];
      // FIXME: This test could be optimized.
      return CGM::equal( dir0.getNormalized().dot( dir1.getNormalized() ), 1.0f, CGConstf::epsilon(64.0f) );
   }
   
   void computePolygons( Vector< RCP<BoundaryPolygon> >& faces )
   {
      DBG_BLOCK( os_bsp3, "computePolygons" );
      DBG_MSG( os_bsp3, "v: " << _vertices.size() << " e: " << _edges.size() );
      
      RCP<BoundaryPolygon> face;
      uint cedge    = 0;
      uint sedge    = 0;
      uint numEdges = uint(_edges.size());
      uint e        = 0;
      
      while( numEdges > 0 )
      {
         // Skip edge?
         if( _edges[e]._count == 0 )
         {
            DBG_MSG( os_bsp3, "skip edge" );
            ++e;
         }
         // Start a new polygon?
         else if( face == 0 )
         {
            DBG_MSG( os_bsp3, "Starting polygon" );
            sedge = e;
            cedge = e;
            face  = new BoundaryPolygon();
            face->id( _id );
            face->addVertex( _vertices[_edges[e]._v1] );
            faces.pushBack( face );
            ++e;
            --numEdges;
         }
         else
         {
            // Search next edge.
            bool found = false;
            for( uint ne = e; ne < uint(_edges.size()); ++ne )
            {
               // Edge valid?
               if( _edges[ne]._count != 0 )
               {
                  // Do we have the next edge?
                  if( _edges[ne]._v0 == _edges[cedge]._v1 )
                  {
                     DBG_MSG( os_bsp3, "found next edge" );
                     // Squash last vertex if edges are collinear.
                     if( parallel( cedge, ne ) )
                     {
                        DBG_MSG( os_bsp3, "removing vertex" );
                        face->removeVertex( face->numVertices()-1 );
                     }
                     face->addVertex( _vertices[_edges[ne]._v1] );
               
                     found = true;
                     cedge = ne;
                     _edges[ne]._count = 0;
                     --numEdges;
                     break;
                  }
               }
            }
            
            // Polygon completed?
            if( !found || numEdges == 0 )
            {
               // Remove last vertex if last and first edge are collinear.
               if( parallel( sedge, cedge ) )
               {
                  DBG_MSG( os_bsp3, "removing vertex" );
                  face->removeVertex( face->numVertices()-1 );
               }
               DBG_MSG( os_bsp3, "v: " << face->numVertices() );
               face->computeDerivedData();
               DBG_MSG( os_bsp3, "adding poly " << faces.size()-1 << "  " << *face );
               face = 0;
            }
         }
      }
   }
 
private:
   
   /*----- data members -----*/

   ConstString   _id;
   float         _precision;
   Vector<Vec3f> _vertices;
   Vector<Edge>  _edges;
};


//------------------------------------------------------------------------------
//! 
void
makeConvex( Vector< RCP<BoundaryPolygon> >& faces )
{
   DBG_BLOCK( os_bsp3, "makeConvex" );
   for( uint i = 0; i < faces.size(); )
   {
      DBG_MSG( os_bsp3, "poly " << i << " " << *faces[i] );
      if( !faces[i]->isConvex() )
      {
         DBG_MSG( os_bsp3, "Non convex" );

         BSP2 bsp;
         bsp.build( *faces[i] );

         faces[i] = faces.back();
         faces.popBack();

         Vector< RCP<BoundaryPolygon> > polys;
         bsp.computeConvexPolygons( polys );
         DBG_MSG( os_bsp3, "New poly" );
         for( uint j = 0; j < polys.size(); ++j )
         {
            faces.pushBack( polys[j] );
         }
      }
      else
      {
         ++i;
      }
   }
}

UNNAMESPACE_END


NAMESPACE_BEGIN

/*==============================================================================
   BSP3Node
==============================================================================*/

BSP3Node BSP3Node::_in;
BSP3Node BSP3Node::_out;

/*==============================================================================
   CLASS BSP3
==============================================================================*/

//------------------------------------------------------------------------------
//!
BSP3::BSP3() :
   _root(&BSP3Node::_out),
   _oproot(&BSP3Node::_out),
   _precision( 0.001f ),
   _epsilon( 0.0005f )
{
}

//------------------------------------------------------------------------------
//!
BSP3::~BSP3()
{
   clear();
}

//------------------------------------------------------------------------------
//!
//
void 
BSP3::build( Vector< RCP<BoundaryPolygon> >& faces )
{
   DBG_BLOCK( os_bsp3, "BSP3::build" );
   clear();
   makeConvex( faces );
   _root = buildNode( faces );
}
   
//------------------------------------------------------------------------------
//!
void 
BSP3::add( Vector< RCP<BoundaryPolygon> >& faces )
{
   DBG_BLOCK( os_bsp3, "BSP3::add" );
   makeConvex( faces );
   _oproot = buildNode( faces );
   _root   = incrementalOp( UNION, _root, faces );
   clearOpTree();
}

//------------------------------------------------------------------------------
//!
void 
BSP3::remove( Vector< RCP<BoundaryPolygon> >& faces )
{
   DBG_BLOCK( os_bsp3, "BSP3::remove" );
   for( uint i = 0; i < faces.size(); ++i )
   {
      faces[i]->inverse();
   }
   makeConvex( faces );
   _oproot = buildNode( faces );
   _root   = incrementalOp( INTERSECTION, _root, faces );
   clearOpTree();
}

//------------------------------------------------------------------------------
//!
void 
BSP3::intersect( Vector< RCP<BoundaryPolygon> >& faces )
{
   DBG_BLOCK( os_bsp3, "BSP3::intersect" );
   makeConvex( faces );
   _oproot = buildNode( faces );
   _root   = incrementalOp( INTERSECTION, _root, faces );
   clearOpTree();
}

//------------------------------------------------------------------------------
//!
void 
BSP3::computeBoundary( Vector< RCP<BoundaryPolygon> >& faces, bool reduced )
{
   DBG_BLOCK( os_bsp3, "BSP3::computeBoundary" );

   //dump();
   updateBoundary( _root );
   faces.clear();
   retrieveBoundary( _root, faces );
   
   if( reduced ) reduceBoundary( faces );
}

//------------------------------------------------------------------------------
//!
BSP3Node* 
BSP3::buildNode( const Vector< RCP<BoundaryPolygon> >& faces )
{
   DBG_BLOCK( os_bsp3, "BSP3::buildNode" );
   DBG_MSG( os_bsp3, faces.size() << " faces" );
   
   Planef plane = faces[0]->plane();

   DBG_MSG( os_bsp3, "plane: " << plane << " length: " << plane.direction().length() );

   for( uint i = 0; i < faces.size(); ++i )
   {
      DBG_MSG( os_bsp3, *(faces[i]) );
   }

   // New node.
   BSP3Node* node = new BSP3Node( plane );
   node->add( faces[0] );
   
   // Split faces.
   Vector< RCP<BoundaryPolygon> > front;
   Vector< RCP<BoundaryPolygon> > back;
   bool sameOrient = true;
   
   for( uint i = 1; i < faces.size(); ++i )
   {
      uint res = classify( plane, *faces[i], _epsilon );
      switch( res )
      {
         case ON:
         {
            DBG_MSG( os_bsp3, "on" );
            node->add( faces[i] );
         } break;
         case ON_FLIPPED:
         {
            DBG_MSG( os_bsp3, "flipped" );
            node->add( faces[i] );
            sameOrient = false;
         } break;
         case FRONT:
         {
            DBG_MSG( os_bsp3, "front" );
            front.pushBack( faces[i] );
         } break;
         case BACK:
         {
            DBG_MSG( os_bsp3, "back" );
            back.pushBack( faces[i] );
         } break;
         case SPANNING:
         {
            DBG_MSG( os_bsp3, "spanning" );
            RCP<BoundaryPolygon> frontFace( new BoundaryPolygon() );
            RCP<BoundaryPolygon> backFace( new BoundaryPolygon() );
            split( *faces[i], plane, *frontFace, *backFace, _epsilon );
            front.pushBack( frontFace );
            back.pushBack( backFace );
         } break;
      }
   }
   
   // Back (inside).
   if( back.empty() )
   {
      DBG_MSG( os_bsp3, "back empty" );
      if( sameOrient )
         node->backIn();
      else
         node->backOut();
   }
   else
   {
      node->back( buildNode( back ) );
   }
   
   // Front (outside).
   if( front.empty() )
   {
      DBG_MSG( os_bsp3, "front empty" );
      if( sameOrient )
         node->frontOut();
      else
         node->frontIn();
   }
   else
   {
      node->front( buildNode( front ) );
   }
   
   return node;
}

//------------------------------------------------------------------------------
//!
BSP3Node* 
BSP3::incrementalOp(
   Operation                             op, 
   BSP3Node*                             node, 
   const Vector< RCP<BoundaryPolygon> >& faces
)
{
   DBG_BLOCK( os_bsp3, "BSP3::incrementalOp" );

   for( uint i = 0; i < faces.size(); ++i )
   {
      DBG_MSG( os_bsp3, *(faces[i]) );
   } 

   if( node->isLeaf() )
   {
      DBG_MSG( os_bsp3, "node" );
      switch( op )
      {
         case UNION:
         {
            DBG_MSG( os_bsp3, "UNION" );
            if( node->in() )
            {
               return node;
            }
            else
            {
               return buildNode( faces );
            }
         } break;
         case INTERSECTION:
         {
            DBG_MSG( os_bsp3, "INTERSECTION" );
            if( node->in() )
            {
               return buildNode( faces );
            }
            else
            {
               return node;
            }
         } break;
         default: 
            break;
      }
   }
   else
   {
      Vector< RCP<BoundaryPolygon> > front;
      Vector< RCP<BoundaryPolygon> > back;

      uint backRegion  = 0xffff;
      uint frontRegion = 0xffff;

      DBG_MSG( os_bsp3, faces.size() << " faces to classify" );
      for( uint i = 0; i < faces.size(); ++i )
      {
         uint res = classify( node->plane(), *faces[i], _epsilon );
         switch( res )
         {
            case ON:
            {
               DBG_MSG( os_bsp3, "on" );
               node->add( faces[i] );
               backRegion  = IN;
               frontRegion = OUT;
            } break;
            case ON_FLIPPED:
            {
               DBG_MSG( os_bsp3, "on flipped" );
               node->add( faces[i] );
               backRegion  = OUT;
               frontRegion = IN;
            } break;
            case FRONT:
            {
               DBG_MSG( os_bsp3, "front" );
               front.pushBack( faces[i] );
            } break;
            case BACK:
            {
               DBG_MSG( os_bsp3, "back" );
               back.pushBack( faces[i] );
            } break;
            case SPANNING:
            {
               DBG_MSG( os_bsp3, "spanning" );
               RCP<BoundaryPolygon> frontFace( new BoundaryPolygon() );
               RCP<BoundaryPolygon> backFace( new BoundaryPolygon() );
               split( *faces[i], node->plane(), *frontFace, *backFace, _epsilon );
               front.pushBack( frontFace );
               back.pushBack( backFace );
            } break;
         }
      }
      
      // Compute vertex for region status.
      Vec3f pt(0.0f);
      if( node->numPolygons() > 0 ) pt = node->polygon(0)->computeCentroid();

      // Back (inside).
      if( back.empty() )
      {
         DBG_MSG( os_bsp3, "back empty" );
         if( backRegion > 0xf ) backRegion = inclusion( _oproot, pt );

         switch( op )
         {
            case UNION:
               if( backRegion == IN ) node->backIn();
               break;
            case INTERSECTION:
               if( backRegion == OUT ) node->backOut();
               break;
            default: break;
         }
      }
      else
      {
         node->back( incrementalOp( op, node->back(), back ) );
      }
      
      // Front (outside).
      if( front.empty() )
      {
         DBG_MSG( os_bsp3, "front empty" );
         if( frontRegion > 0xf ) frontRegion = inclusion( _oproot, pt );

         switch( op )
         {
            case UNION:
               if( frontRegion == IN ) node->frontIn();
               break;
            case INTERSECTION:
               if( frontRegion == OUT ) node->frontOut();
               break;
            default: break;
         }
      }
      else
      {
         node->front( incrementalOp( op, node->front(), front ) );
      }

      // Reduction.
      if( node->back() == node->front() )
      {
         DBG_MSG( os_bsp3, "Reduction" );
         return node->back();
      }
   }
   
   return node;
}

//------------------------------------------------------------------------------
//!
void 
BSP3::updateBoundary( BSP3Node* node )
{
   DBG_BLOCK( os_bsp3, "BSP3::updateBoundary" );
   if( node->isLeaf() ) return;
   
   updateBoundary( node->back() );
   updateBoundary( node->front() );
   
   // create polygon lists.
   Vector< RCP<BoundaryPolygon> > front;
   Vector< RCP<BoundaryPolygon> > back;
   
   DBG_MSG( os_bsp3, node->numPolygons() << " faces" );
   
   for( uint i = 0; i < node->numPolygons(); ++i )
   {
      if( node->polygon(i)->normal().dot( node->plane().direction() ) > 0.0f )
      {
         front.pushBack( node->polygon(i) );
      }
      else
      {
         back.pushBack( node->polygon(i) );
      }
   }
   
   // Clip lists.
   if( !front.empty() ) clipBoundary( node->back(),  front, OUT );
   if( !front.empty() ) clipBoundary( node->front(), front, IN );
   if( !back.empty() )  clipBoundary( node->back(),  back, IN );
   if( !back.empty() )  clipBoundary( node->front(), back, OUT );
   
   // Merge lists.
   node->clearPolygons();
   
   DBG_MSG( os_bsp3, "back: " << back.size() );
   DBG_MSG( os_bsp3, "front: " << front.size() );
   
   // 1. Back faces
   DBG_MSG( os_bsp3, "Adding faces to node!!!!" );
   DBG_MSG( os_bsp3, "BACK" );

   if( back.size() == 1 )
   {
      node->add( back[0] );
   }
   else
   {
      if( !back.empty() )
      {
         BSP2 bsp;
         bsp.build( *back[0] );
         for( uint i = 1; i < back.size(); ++i )
         {
            bsp.add( *back[i] );
         }
         bsp.computeConvexPolygons( back );
         for( uint i = 0; i < back.size(); ++i )
         {
            node->add( back[i] );
         }
      }
   }

   // 2. Front faces.
   DBG_MSG( os_bsp3, "FRONT" );

   if( front.size() == 1 )
   {
      node->add( front[0] );
   }
   else
   {
      if( !front.empty() )
      {
         BSP2 bsp;
         bsp.build( *front[0] );
         for( uint i = 1; i < front.size(); ++i )
         {
            bsp.add( *front[i] );
         }
         bsp.computeConvexPolygons( front );
         for( uint i = 0; i < front.size(); ++i )
         {
            node->add( front[i] );
         }
      }
   }
}

//------------------------------------------------------------------------------
//!
void 
BSP3::retrieveBoundary( BSP3Node* node, Vector< RCP<BoundaryPolygon> >& faces )
{
   DBG_BLOCK( os_bsp3, "BSP3::retrieveBoundary" );
   if( node->isLeaf() ) return;
   
   retrieveBoundary( node->back(), faces );
   retrieveBoundary( node->front(), faces );
   
   for( uint i = 0; i < node->numPolygons(); ++i )
   {
      faces.pushBack( node->polygon(i) );
   }
}

//------------------------------------------------------------------------------
//!
void 
BSP3::clipBoundary(
   BSP3Node*                       node, 
   Vector< RCP<BoundaryPolygon> >& faces,
   int                             region
)
{
   DBG_BLOCK( os_bsp3, "BSP3::clipBoundary" );
   if( node->isLeaf() )
   {
      if( region == IN )
      {
         if( node->in() )
         {
            faces.clear();
            DBG_MSG( os_bsp3, "IN: deleting" );
         }
      }
      else
      {
         if( node->out() )
         {
            faces.clear();
            DBG_MSG( os_bsp3, "OUT: deleting" );
         }
      }
   }
   else
   {
      Vector< RCP<BoundaryPolygon> > front;
      Vector< RCP<BoundaryPolygon> > back;
      
      DBG_MSG( os_bsp3, "clip plane: " << node->plane() );
   
      for( uint i = 0; i < faces.size(); ++i )
      {
         uint res = classify( node->plane(), *faces[i], _epsilon );
         switch( res )
         {
            case ON:
            {
               DBG_MSG( os_bsp3, "on" );
               printf( "ERROR: on boundary.\n" );
            } break;
            case ON_FLIPPED:
            {
               DBG_MSG( os_bsp3, "flipped" );
               printf( "ERROR: on boundary flipped.\n" );
            } break;
            case FRONT:
            {
               DBG_MSG( os_bsp3, "front" );
               front.pushBack( faces[i] );
            } break;
            case BACK:
            {
               DBG_MSG( os_bsp3, "back" );
               back.pushBack( faces[i] );
            } break;
            case SPANNING:
            {
               DBG_MSG( os_bsp3, "spanning" );
               RCP<BoundaryPolygon> frontFace( new BoundaryPolygon() );
               RCP<BoundaryPolygon> backFace( new BoundaryPolygon() );
               split( *faces[i], node->plane(), *frontFace, *backFace, _epsilon );
               front.pushBack( frontFace );
               back.pushBack( backFace );
            } break;
         }
      }
      
      if( !back.empty() )  clipBoundary( node->back(), back, region );
      if( !front.empty() ) clipBoundary( node->front(), front, region );
      
      // Merge lists.
      faces.clear();
      for( uint i = 0; i < back.size(); ++i )
      {
         faces.pushBack( back[i] );
      }
      for( uint i = 0; i < front.size(); ++i )
      {
         faces.pushBack( front[i] );
      }
   }
}

//------------------------------------------------------------------------------
//!
void 
BSP3::reduceBoundary( Vector< RCP<BoundaryPolygon> >& faces )
{
   DBG_BLOCK( os_bsp3, "BSP3::reduceBoundary" );
   DBG_MSG( os_bsp3, faces.size() << " faces" );

   for( uint i = 0; i < faces.size(); ++i )
   {
      DBG_MSG( os_bsp3, *faces[i] );
   }

   // Classify faces by planes.
   List< Vector< RCP<BoundaryPolygon> > > planes;
   List< Vector< RCP<BoundaryPolygon> > >::Iterator it;
   for( uint i = 0; i < faces.size(); ++i )
   {
      DBG_MSG( os_bsp3, "face plane: " << faces[i]->plane() );
      Vector< RCP<BoundaryPolygon> >* pFaces = 0;
      for( it = planes.begin(); it != planes.end(); ++it )
      {
         DBG_MSG( os_bsp3, "  comp plane: " << (*it)[0]->plane() );
         if( (*it)[0]->plane().equal( faces[i]->plane(), _epsilon ) && 
             ((*it)[0]->id() == faces[i]->id() ) )
         {
            DBG_MSG( os_bsp3, "equal" );
            pFaces = &(*it);
            break;
         }
      }
      if( pFaces == 0 )
      {
         DBG_MSG( os_bsp3, "new plane" );
         planes.pushBack( Vector< RCP<BoundaryPolygon> >() );
         pFaces = &planes.back();
      }
      pFaces->pushBack( faces[i] );
   }
   
   DBG_MSG( os_bsp3, planes.size() << " planes" );
   
   // Simplify faces.
   Reducer reducer( _precision );
   Vector< RCP<BoundaryPolygon> > resFaces;
   for( it = planes.begin(); it != planes.end(); ++it )
   {
      // Nothing to reduce if only one polygon.
      if( (*it).size() == 1 )
      {
         resFaces.pushBack( (*it)[0] );
         continue;
      }

      DBG_MSG( os_bsp3, "starting plane" );
      for( uint i = 0; i < (*it).size(); ++i )
      {
         DBG_MSG( os_bsp3, "poly " << i );
         DBG_MSG( os_bsp3, *(*it)[i] );
      }

      removeTVertices( *it, _precision );
      
      // Add polygon to reducer.
      reducer.clear();
      reducer.id( (*it)[0]->id() );
      for( uint i = 0; i < (*it).size(); ++i )
      {
         BoundaryPolygon* face = (*it)[i].ptr();
         size_t numVertices = face->numVertices();
         
         // Add each edge and vertex to the reducer.
         uint v0 = reducer.add( face->vertex( numVertices-1 ) );
         for( size_t v = 0; v < numVertices; ++v )
         {
            uint v1 = reducer.add( face->vertex(v) );
            reducer.add( v0, v1 );
            v0 = v1;
         }
      }
      // Removed duplicated edges.
      reducer.reduce();
      
      // Reconstruct polygons.
      reducer.computePolygons( resFaces );
   }
   // Remove t-vertices on the final faces.
   removeTVertices( resFaces, _precision );

   DBG_MSG( os_bsp3, "OUT POLYGONS" );
   for( uint p = 0; p < resFaces.size(); ++p )
   {
      DBG_MSG( os_bsp3, p << "  " << *resFaces[p] );
   }
   
   faces.swap( resFaces );
}

//------------------------------------------------------------------------------
//! 
int 
BSP3::inclusion( const BSP3Node* node, const Vec3f& pt )
{
   if( node->isLeaf() ) return node->in() ? IN : OUT;
   return node->plane().inFront(pt) ? 
      inclusion( node->front(), pt ) :
      inclusion( node->back(), pt );
}

/*==============================================================================
   DEBUG METHODS
==============================================================================*/

//------------------------------------------------------------------------------
//! 
void
BSP3::dump() const
{
   dump( _root );
}

//------------------------------------------------------------------------------
//! 
void 
BSP3::dump( BSP3Node* node ) const
{
   DBG_BLOCK( os_bsp3, "dump node: " << node );
   
   if( node->isLeaf() )
   {
      if( node->in() )
      {
         DBG_MSG( os_bsp3, "IN" );
      }
      else
      {
         DBG_MSG( os_bsp3, "OUT" );
      }
      return;
   }

   DBG_MSG( os_bsp3, "p: "    << node->plane() << " #polys: " << node->numPolygons() );
   DBG_MSG( os_bsp3, "back: " << node->back()  << " front: "  << node->front() );
   for( uint p = 0; p < node->numPolygons(); ++p  )
   {
      DBG_MSG( os_bsp3, " " << *node->polygon(p) );
   }
   dump( node->back() );
   dump( node->front() );
}


NAMESPACE_END
