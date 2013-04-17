/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Procedural/BSP2.h>

#include <Base/ADT/List.h>

#include <Base/Dbg/DebugStream.h>

#include <algorithm>

/*==============================================================================
   UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

DBG_STREAM( os_bsp2, "BSP2" );

enum {
   ON,
   ON_FLIPPED,
   FRONT,
   BACK,
   SPANNING
};

enum {
   IN,
   OUT,
   MIX
};

//------------------------------------------------------------------------------
//! 
void
toEdges( const BoundaryPolygon& poly, Vector<Edgef>& edges, uint mainAxis )
{
   Vec3d pnormal(0.0);
   pnormal(mainAxis) = poly.normal()(mainAxis);
   size_t i = poly.numVertices()-1;
   for( size_t j = 0; j < poly.numVertices(); i=j++ )
   {
      Vec3d v0     = poly.vertex(i);
      Vec3d v1     = poly.vertex(j);
      v0(mainAxis) = 0.0;
      v1(mainAxis) = 0.0;
      Vec3d normal = cross( v1-v0, pnormal );

      // Project normal into the plane of the main axis.
      normal.normalize();

      Edgef edge( poly.vertex(i), poly.vertex(j), normal );
      edges.pushBack( edge );
   }
}

//------------------------------------------------------------------------------
//!
uint 
classify( const Planed& plane, const Edgef& edge, double epsilon )
{
   DBG_BLOCK( os_bsp2, "classify with " << plane );
   DBG_MSG( os_bsp2, edge.vertex(0) << edge.vertex(1) << " n: " << edge.normal() );
   // Find the closest vertex to the plane.
   double minDist = CGConstd::infinity();
   double maxDist = -CGConstd::infinity();
   
   for( uint i = 0; i < 2; ++i )
   {
      double dist = plane.distance( edge.vertex(i) );
      if( dist < minDist ) minDist = dist;
      if( dist > maxDist ) maxDist = dist;
   }

   DBG_MSG( os_bsp2, "min: " << minDist << " max: " << maxDist );
   
   // Evaluate polygon classification.
   if( CGM::abs(maxDist) < epsilon && CGM::abs(minDist) < epsilon )
   {
      if( edge.normal().dot( plane.direction() ) > 0.0 )
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
split( const Edgef& edge, const Planed& plane, Edgef& frontEdge, Edgef& backEdge )
{
   // Find intersection points.
   Vec3d v0 = edge.vertex(0);
   Vec3d v1 = edge.vertex(1);

   double distV0   = plane.distance( v0 );
   Vec3d dir       = v1 - v0;
   Vec3d pt        = v0 - dir*(distV0 / plane.direction().dot(dir));

   if( distV0 > 0.0 )
   {
      frontEdge.set( v0, pt, edge.normal() );
      backEdge.set( pt, v1, edge.normal() );      
   }
   else
   {
      backEdge.set( v0, pt, edge.normal() );
      frontEdge.set( pt, v1, edge.normal() ); 
   }
}


//------------------------------------------------------------------------------
//!
void 
split( 
   const BoundaryPolygon& poly, 
   const Planed&          plane, 
   BoundaryPolygon&       frontFace,
   BoundaryPolygon&       backFace, 
   double                 epsilon
)
{
   DBG_BLOCK( os_bsp2, "split" );

   size_t count = poly.numVertices();
   Vec3d ptA    = poly.vertex( count-1 );
   double distA = plane.distance( ptA );
   
   DBG_MSG( os_bsp2, "distA: " << distA << " plane: " << plane );

   for( size_t i = 0; i < count; ++i )
   {
      Vec3d ptB    = poly.vertex(i);
      double distB = plane.distance( ptB );
      if( distB > epsilon )
      {
         if( distA < -epsilon )
         {
            DBG_MSG( os_bsp2, "A: " << ptA << " B: " << ptB );
            Vec3d dir  = ptB - ptA;
            Vec3d pt   = ptA - dir*(distA / dir.dot(plane.direction()));
            frontFace.addVertex( pt );
            backFace.addVertex( pt );
            DBG_MSG( os_bsp2, "dir: " << dir << " dot: " << plane.direction().dot(dir)  << " pt: " << pt );
         }
         frontFace.addVertex( ptB );
      }
      else if( distB < -epsilon )
      {
         if( distA > epsilon )
         {
            DBG_MSG( os_bsp2, "A: " << ptA << " B: " << ptB );
            Vec3d dir  = ptB - ptA;
            Vec3d pt   = ptA - dir*(distA / dir.dot(plane.direction()));
            frontFace.addVertex( pt );
            backFace.addVertex( pt );
            DBG_MSG( os_bsp2, "dir: " << dir << " dot: " << plane.direction().dot(dir)  << " pt: " << pt );
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
uint
regionStatus( const Planed& plane, const Vec3f& p, const Vector<Edgef>& edges, double epsilon )
{
   // Find the nearest edge: 
   // -The most parallel edge which contains the nearest vertex.
   double minDist = CGConstd::infinity();
   uint minVertex = 0;
   uint minEdge   = 0;
   for( uint e = 0; e < edges.size(); ++e )
   {
      // Compute edge distance.
      double edgeDist = CGConstd::infinity();
      uint v          = 0;
      for( uint i = 0; i < 2; ++i )
      {
         double dist = CGM::abs( plane.distance( edges[e].vertex(i) ) );
         if( dist < edgeDist )
         {
            edgeDist = dist;
            v        = i;
         }
      }
      // Keep the nearest edge.
      if( edgeDist < minDist-epsilon )
      {
         minDist   = edgeDist;
         minEdge   = e;
         minVertex = v;
      }
   }

   // Find the vertex on other edge.
   for( uint e = 0; e < edges.size(); ++e )
   {
      if( e == minEdge ) continue;
      if( equal( edges[minEdge].vertex(minVertex), edges[e].vertex(1-minVertex), (float)epsilon) )
      {
         Planed cplane( edges[minEdge].normal(), edges[minEdge].vertex(minVertex) );
         return ( cplane.distance( edges[e].vertex(minVertex) ) > 0.0 ) ? IN : OUT;
      }
   }

   Planed cplane( edges[minEdge].normal(), edges[minEdge].vertex(minVertex) );
   return ( cplane.distance( p ) > 0.0 ) ? OUT : IN;
}

//------------------------------------------------------------------------------
//! 
bool parallel( const Vec3f& v0, const Vec3f& v1, const Vec3f& v2 )
{
   Vec3f dir0 = normalize( v1 - v0 );
   Vec3f dir1 = normalize( v2 - v1 );
   return CGM::equal( dot( dir0, dir1 ), 1.0f, CGConstf::epsilon(64.0f) );
}

//------------------------------------------------------------------------------
//!
bool compSize( const Edgef& a, const Edgef& b )
{
   return a.sqrLength() > b.sqrLength();
}

UNNAMESPACE_END


NAMESPACE_BEGIN

/*==============================================================================
   BSP2Node
==============================================================================*/

BSP2Node BSP2Node::_in;
BSP2Node BSP2Node::_out;

/*==============================================================================
   CLASS BSP2
==============================================================================*/

//------------------------------------------------------------------------------
//!
BSP2::BSP2() :
   _root(&BSP2Node::_out),
   _back(1.0f),
   _precision( 1.0f/1024.0f ),
   _epsilon( 0.0005 )
{
}

//------------------------------------------------------------------------------
//!
BSP2::~BSP2()
{
   clear();
}

//------------------------------------------------------------------------------
//!
void 
BSP2::build( const BoundaryPolygon& face )
{
   DBG_BLOCK( os_bsp2, "BSP2::build" );
   clear();

   // Keep the main ID.
   _id = face.id();

   // Compute main axis. 
   // Components on this axis will be removed from computations.
   const Vec3f& n = face.normal();
   _mainAxis = n.maxComponent();

   DBG_MSG( os_bsp2, "main axis: " << _mainAxis );

   // Convert polygon to edges.
   Vector<Edgef> edges;
   toEdges( face, edges, _mainAxis );

   _plane = face.plane();
   _root  = buildNode( edges, face.id(), ConstString() );
}
   
//------------------------------------------------------------------------------
//!
void 
BSP2::add( const BoundaryPolygon& face )
{
   DBG_BLOCK( os_bsp2, "BSP2::add" );
   DBG_MSG( os_bsp2, face );
   Vector<Edgef> edges;
   toEdges( face, edges, _mainAxis );
   _id   = face.id();
   _root = incrementalOp( UNION, _root, edges, face.id(), ConstString() );
}

//------------------------------------------------------------------------------
//!
void 
BSP2::remove( const BoundaryPolygon& face )
{
   DBG_BLOCK( os_bsp2, "BSP2::remove" );
   Vector<Edgef> edges;
   toEdges( face, edges, _mainAxis );
   for( uint i = 0; i < edges.size(); ++i )
   {
      edges[i].inverse();
   }
   _id   = face.id();
   _root = incrementalOp( INTERSECTION, _root, edges, face.id(), ConstString() );
}

//------------------------------------------------------------------------------
//! 
void
BSP2::intersect( const BoundaryPolygon& face )
{
   DBG_BLOCK( os_bsp2, "BSP2::intersect" );
   Vector<Edgef> edges;
   toEdges( face, edges, _mainAxis );
   _id   = face.id();
   _root = incrementalOp( INTERSECTION, _root, edges, face.id(), ConstString() );
}

//------------------------------------------------------------------------------
//! 
void
BSP2::removeProj( BoundaryPolygon& face )
{
   double cosa = dot( _plane.direction(), Vec3d(face.normal()) );

   if( equal( cosa, 0.0, _epsilon ) ) return;

   // Project on plane and test for validity (at least one vertex in front).
   bool inFront = false;
   bool inside  = false;
   for( uint i = 0; i < face.numVertices(); ++i )
   {
      Vec3f pt       = face.vertex(i);
      double d       = _plane.distance( pt );
      inFront       |= d > (-1.0/1024.0);
      inside        |= d < _back;
      face.vertex(i) = pt - _plane.direction()*d;
   }

   // Face is completely behind?
   if( !inFront || !inside ) return;

   remove( face );
}

//------------------------------------------------------------------------------
//!
void 
BSP2::computePolygons( Vector< RCP<BoundaryPolygon> >& polys )
{
   DBG_BLOCK( os_bsp2, "BSP2::computePolygons" );
   updateBoundary( _root );
   polys.clear();
   Vector<Edgef> edges;
   retrieveBoundary( _root, edges );
   buildPolygons( edges, polys );
}

//------------------------------------------------------------------------------
//! 
void
BSP2::computeConvexPolygons( Vector< RCP<BoundaryPolygon> >& polys ) const
{
   DBG_BLOCK( os_bsp2, "BSP2:computeConvexPolygons" );
   polys.clear();
   
   RCP<BoundaryPolygon> poly( new BoundaryPolygon() );
   poly->reserveVertices(4);

   float v = 8192.0f;

   switch( _mainAxis )
   {
      case 0:
         // x
         poly->addVertex( Vec3f(0.0f, -v, -v ) );
         poly->addVertex( Vec3f(0.0f,  v, -v ) );
         poly->addVertex( Vec3f(0.0f,  v,  v ) );
         poly->addVertex( Vec3f(0.0f, -v,  v ) ); 
         break;
      case 1:
         // y
         poly->addVertex( Vec3f( -v, 0.0f, -v ) );
         poly->addVertex( Vec3f( -v, 0.0f,  v ) );
         poly->addVertex( Vec3f(  v, 0.0f,  v ) );
         poly->addVertex( Vec3f(  v, 0.0f, -v ) );
         break;
      case 2:
         // z
         poly->addVertex( Vec3f(  -v, -v, 0.0f ) ); 
         poly->addVertex( Vec3f(   v, -v, 0.0f ) ); 
         poly->addVertex( Vec3f(   v,  v, 0.0f ) ); 
         poly->addVertex( Vec3f(  -v,  v, 0.0f ) ); 
         break;
   }

   poly->computeDerivedData();

   if( _plane.direction()(_mainAxis) < 0.0 ) poly->inverse();
   
   clipPolygon( _root, poly, polys );

   // Project polygons on the main plane.
   for( uint p = 0; p < polys.size(); ++p )
   {
      for( uint v = 0; v < polys[p]->numVertices(); ++v )
      {
         polys[p]->vertex(v)(_mainAxis) = 0.0f;
         polys[p]->vertex(v)(_mainAxis) = float(_plane.evaluate( polys[p]->vertex(v) ) / -_plane.direction()(_mainAxis)); 
      }
      polys[p]->computeDerivedData();
   }
}

//------------------------------------------------------------------------------
//! 
float
BSP2::computeArea() const
{
   Vector< RCP<BoundaryPolygon> > polys;
   computeConvexPolygons( polys );

   float area = 0.0f;

   for( uint i = 0; i < polys.size(); ++i )
   {
      area += polys[i]->area();
   }
   return area;
}

//------------------------------------------------------------------------------
//!
BSP2Node* 
BSP2::buildNode( 
   const Vector<Edgef>& edges, 
   const ConstString&   inID, 
   const ConstString&   outID
)
{
   DBG_BLOCK( os_bsp2, "BSP2::buildNode" );
   DBG_MSG( os_bsp2, edges.size() << " nodes" );
   
   // Choose a plane.
   Planed plane( edges[0].normal(), edges[0].vertex(0) );
   
   DBG_MSG( os_bsp2, "plane: " << plane << " length: " << plane.direction().length() );

   for( uint i = 0; i < edges.size(); ++i )
   {
      DBG_MSG( os_bsp2, " n: " << edges[i].normal() << " v: " << edges[i].vertex(0) << edges[i].vertex(1) );
   }

   // New node.
   BSP2Node* node = new BSP2Node( plane );
   node->add( edges[0] );
   node->backID( inID );
   node->frontID( outID );
   
   // Split faces.
   Vector<Edgef> front;
   Vector<Edgef> back;
   bool sameOrient = true;
   
   for( uint i = 1; i < edges.size(); ++i )
   {
      uint res = classify( plane, edges[i], _epsilon );
      switch( res )
      {
         case ON:
         {
            DBG_MSG( os_bsp2, "on" );
            node->add( edges[i] );
         } break;
         case ON_FLIPPED:
         {
            DBG_MSG( os_bsp2, "flipped" );
            node->add( edges[i] );
            node->frontID( _id );
            sameOrient = false;
         } break;
         case FRONT:
         {
            DBG_MSG( os_bsp2, "front" );
            front.pushBack( edges[i] );
         } break;
         case BACK:
         {
            DBG_MSG( os_bsp2, "back" );
            back.pushBack( edges[i] );
         } break;
         case SPANNING:
         {
            DBG_MSG( os_bsp2, "spanning" );
            Edgef frontEdge;
            Edgef backEdge;
            split( edges[i], plane, frontEdge, backEdge );
            front.pushBack( frontEdge );
            back.pushBack( backEdge );
         } break;
      }
   }
   
   // Back (inside).
   if( back.empty() )
   {
      DBG_MSG( os_bsp2, "back empty" );
      if( sameOrient )
         node->backIn();
      else
         node->backOut();
   }
   else
   {
      node->back( buildNode( back, inID, outID ) );
   }
   
   // Front (outside).
   if( front.empty() )
   {
      DBG_MSG( os_bsp2, "front empty" );
      if( sameOrient )
         node->frontOut();
      else
         node->frontIn();
   }
   else
   {
      node->front( buildNode( front, inID, outID ) );
   }
   return node;
}

//------------------------------------------------------------------------------
//!
BSP2Node* 
BSP2::incrementalOp( 
   Operation            op, 
   BSP2Node*            node, 
   const Vector<Edgef>& edges,
   const ConstString&   inID,
   const ConstString&   outID
)
{
   DBG_BLOCK( os_bsp2, "BSP2::incrementalOp" );
   
   if( node->isLeaf() )
   {
      DBG_MSG( os_bsp2, "node" );
      switch( op )
      {
         case UNION:
         {
            if( node->in() )
               return node;
            else
               return buildNode( edges, inID, outID );
         } break;
         case INTERSECTION:
         {
            if( node->in() )
               return buildNode( edges, inID, outID );
            else
               return node;
         } break;
         default: 
            break;
      }
   }
   else
   {
      Vector<Edgef> front;
      Vector<Edgef> back;

      uint backRegion  = 0xffff;
      uint frontRegion = 0xffff;

      ConstString frontID = node->frontID();
      ConstString backID  = node->backID();

      DBG_MSG( os_bsp2, edges.size() << " edges to classify" );
      for( uint i = 0; i < edges.size(); ++i )
      {
         uint res = classify( node->plane(), edges[i], _epsilon );
         switch( res )
         {
            case ON:
            {
               DBG_MSG( os_bsp2, "on" );
               node->add( edges[i] );
               if( node->backID().isNull() ) node->backID( inID );
               backRegion  = IN;
               frontRegion = OUT;
            } break;
            case ON_FLIPPED:
            {
               DBG_MSG( os_bsp2, "on flipped" );
               node->add( edges[i] );
               if( node->frontID().isNull() ) node->frontID( inID );
               backRegion  = OUT;
               frontRegion = IN;
            } break;
            case FRONT:
            {
               DBG_MSG( os_bsp2, "front" );
               front.pushBack( edges[i] );
            } break;
            case BACK:
            {
               DBG_MSG( os_bsp2, "back" );
               back.pushBack( edges[i] );
            } break;
            case SPANNING:
            {
               DBG_MSG( os_bsp2, "spanning" );
               Edgef frontEdge;
               Edgef backEdge;
               split( edges[i], node->plane(), frontEdge, backEdge );
               front.pushBack( frontEdge );
               back.pushBack( backEdge );
            } break;
         }
      }
      
      // Compute vertex for region status.
      Vec3f pt(0.0f);
      if( node->numEdges() > 0 ) pt = (node->edge(0).vertex(0)+node->edge(0).vertex(1))*0.5f;

      // Back (inside).
      if( back.empty() )
      {
         DBG_MSG( os_bsp2, "back empty" );
         if( backRegion > 0xf ) backRegion = regionStatus( node->plane(), pt, edges, _epsilon );
         
         switch( op )
         {
            case UNION:
               if( backRegion == IN )
               {
                  node->backIn();
                  if( !inID.isNull() ) node->backID( inID );
               }
               break;
            case INTERSECTION:
               if( backRegion == OUT ) node->backOut();
               break;
            default: break;
         }
      }
      else
      {
         node->back( incrementalOp( op, node->back(), back, inID, backID ) );
      }
      
      // Front (outside).
      if( front.empty() )
      {
         DBG_MSG( os_bsp2, "front empty" );
         if( frontRegion > 0xf ) frontRegion = regionStatus( node->plane().getInversed(), pt, edges, _epsilon );

         switch( op )
         {
            case UNION:
               if( frontRegion == IN )
               {
                  node->frontIn();
                  if( !inID.isNull() ) node->frontID( inID );
               }
               break;
            case INTERSECTION:
               if( frontRegion == OUT ) node->frontOut();
               break;
            default: break;
         }
      }
      else
      {
         node->front( incrementalOp( op, node->front(), front, inID, frontID ) );
      }

      // Reduction.
      if( node->back() == node->front() )
      {
         DBG_MSG( os_bsp2, "Reduction" );
         return node->back();
      }
   }
   return node;
}

//------------------------------------------------------------------------------
//!
void 
BSP2::updateBoundary( BSP2Node* node )
{
   DBG_BLOCK( os_bsp2, "BSP2::updateBoundary" );
   if( node->isLeaf() ) return;
   
   updateBoundary( node->back() );
   updateBoundary( node->front() );
   
   // create edge lists.
   Vector<Edgef> front;
   Vector<Edgef> back;
   
   DBG_MSG( os_bsp2, node->numEdges() << " edges" );
   
   for( uint i = 0; i < node->numEdges(); ++i )
   {
      if( node->edge(i).normal().dot( node->plane().direction() ) > 0.0 )
         front.pushBack( node->edge(i) );
      else
         back.pushBack( node->edge(i) );
   }
   
   // Clip lists.
   clipBoundary( node->back(), front, OUT );
   clipBoundary( node->front(), front, IN );
   
   clipBoundary( node->back(), back, IN );
   clipBoundary( node->front(), back, OUT );

   // Merge lists.
   node->clearEdges();
   DBG_MSG( os_bsp2, "back: " << back.size() );
   DBG_MSG( os_bsp2, "front: " << front.size() );
   
   // 1. Back faces
   DBG_MSG( os_bsp2, "Adding faces to node!!!!" );
   DBG_MSG( os_bsp2, "BACK" );
   
   std::sort( back.begin(), back.end(), compSize );
   for( uint i = 0; i < back.size(); ++i )
   {
      Vec3f centroid = back[i].center();
      for( uint j = 0; j < i; ++j )
      {
         if( back[j].valid() && back[j].isInside( centroid ) )
         {
            DBG_MSG( os_bsp2, "deleting back face " << i );
            back[i].invalidate();
            break;
         }
      }
      if( back[i].valid() ) node->add( back[i] );
   }
   
   // 2. Front faces.
   DBG_MSG( os_bsp2, "FRONT" );
   std::sort( front.begin(), front.end(), compSize );
   for( uint i = 0; i < front.size(); ++i )
   {
      Vec3f centroid = front[i].center();
      for( uint j = 0; j < i; ++j )
      {
         if( front[j].valid() && front[j].isInside( centroid ) )
         {
            DBG_MSG( os_bsp2, "deleting front face " << i );
            front[i].invalidate();
            break;
         }
      }
      if( front[i].valid() ) node->add( front[i] );
   }

   // Temporary hack.
   if( !node->backID().isNull() ) _id = node->backID();
}

//------------------------------------------------------------------------------
//!
void 
BSP2::retrieveBoundary( BSP2Node* node, Vector<Edgef>& edges )
{
   DBG_BLOCK( os_bsp2, "BSP2::retrieveBoundary" );
   if( node->isLeaf() ) return;
   
   retrieveBoundary( node->back(), edges );
   retrieveBoundary( node->front(), edges );
   
   for( uint i = 0; i < node->numEdges(); ++i )
   {
      edges.pushBack( node->edge(i) );
   }
}

//------------------------------------------------------------------------------
//!
void 
BSP2::clipBoundary( BSP2Node* node, Vector<Edgef>& edges, int region )
{
   DBG_BLOCK( os_bsp2, "BSP2::clipBoundary" );
   if( edges.empty() )
   {
      DBG_MSG( os_bsp2, "empty" );
      return;
   }
   
   if( node->isLeaf() )
   {
      if( region == IN )
      {
         if( node->in() )
         {
            edges.clear();
            DBG_MSG( os_bsp2, "IN: deleting" );
         }
      }
      else
      {
         if( node->out() )
         {
            edges.clear();
            DBG_MSG( os_bsp2, "OUT: deleting" );
         }
      }
   }
   else
   {
      Vector<Edgef> front;
      Vector<Edgef> back;
      
      DBG_MSG( os_bsp2, "clip plane: " << node->plane() );
   
      for( uint i = 0; i < edges.size(); ++i )
      {
         uint res = classify( node->plane(), edges[i], _epsilon );
         switch( res )
         {
            case ON:
            {
               DBG_MSG( os_bsp2, "on" );
               printf( "ERROR: on boundary.\n" );
            } break;
            case ON_FLIPPED:
            {
               DBG_MSG( os_bsp2, "flipped" );
               printf( "ERROR: on boundary flipped.\n" );
            } break;
            case FRONT:
            {
               DBG_MSG( os_bsp2, "front" );
               front.pushBack( edges[i] );
            } break;
            case BACK:
            {
               DBG_MSG( os_bsp2, "back" );
               back.pushBack( edges[i] );
            } break;
            case SPANNING:
            {
               DBG_MSG( os_bsp2, "spanning" );
               Edgef frontEdge;
               Edgef backEdge;
               split( edges[i], node->plane(), frontEdge, backEdge );
               front.pushBack( frontEdge );
               back.pushBack( backEdge );
            } break;
         }
      }
      
      clipBoundary( node->back(), back, region );
      clipBoundary( node->front(), front, region );
      
      // Merge lists.
      edges.clear();
      for( uint i = 0; i < back.size(); ++i )
      {
         edges.pushBack( back[i] );
      }
      for( uint i = 0; i < front.size(); ++i )
      {
         edges.pushBack( front[i] );
      }
   }
}

//------------------------------------------------------------------------------
//!
void 
BSP2::buildPolygons( Vector<Edgef>& edges, Vector< RCP<BoundaryPolygon> >& polys )
{
   DBG_BLOCK( os_bsp2, "BSP2::buildPolygons" );
   DBG_MSG( os_bsp2, edges.size() << " edges" );

   RCP<BoundaryPolygon> face;
   uint numEdges = uint(edges.size());
      
   uint e = 0;
     
   Vec3f v0;
   Vec3f v1;

   while( numEdges > 0 )
   {
      // Start a new polygon?
      if( face.isNull() )
      {
         DBG_MSG( os_bsp2, "Starting polygon" );
         v0    = edges[e].vertex(0);
         v1    = edges[e].vertex(1);
         face  = new BoundaryPolygon();
         face->id( _id );
         face->addVertex( v1 );
         polys.pushBack( face );
         ++e;
         --numEdges;
      }
      else
      {
         // Search next edge.
         bool found = false;
         for( uint ne = e; ne < edges.size(); ++ne )
         {
            // Do we have the next edge?
            if( edges[ne].vertex(0).equal( v1, _precision ) )
            {
               DBG_MSG( os_bsp2, "found next edge" );
               Vec3f nv = edges[ne].vertex(1);

               // Squash last vertex if edges are collinear.
               if( parallel( v0, v1, nv ) )
               {
                  DBG_MSG( os_bsp2, "removing vertex" );
                  face->removeVertex( face->numVertices()-1 );
               }
               face->addVertex( nv );
              
               v0        = v1;
               v1        = nv;
               found     = true;
               edges[ne] = edges.back();
               edges.popBack();
               --numEdges;
               break;
            }
         }
            
         // Polygon completed?
         if( !found || numEdges == 0 )
         {
            // Remove last vertex if last and first edge are collinear.
            if( parallel( v0, v1, face->vertex(0) ) )
            {
               DBG_MSG( os_bsp2, "removing vertex" );
               face->removeVertex( face->numVertices()-1 );
            }
            DBG_MSG( os_bsp2, "v: " << face->numVertices() );
            face->computeDerivedData();
            face = 0;
         }
      }
   } 
}

//------------------------------------------------------------------------------
//! 
void 
BSP2::clipPolygon( 
   BSP2Node*                       node, 
   const RCP<BoundaryPolygon>&     poly,
   Vector< RCP<BoundaryPolygon> >& polys
) const
{
   DBG_BLOCK( os_bsp2, "BSP2::clipPolygon" );
   if( node->isLeaf() )
   {
      if( node->in() )
      {
         polys.pushBack( poly );
      }
   }
   else
   {
      RCP<BoundaryPolygon> polyBack( new BoundaryPolygon() );
      RCP<BoundaryPolygon> polyFront( new BoundaryPolygon() );
      split( *poly, node->plane(), *polyFront, *polyBack, _epsilon );

      polyBack->id( node->backID() );
      polyFront->id( node->frontID() );
      
      clipPolygon( node->back(), polyBack, polys );
      clipPolygon( node->front(), polyFront, polys );
   }
}

//------------------------------------------------------------------------------
//! 
void BSP2::dump() const
{
   dump( _root );
}

//------------------------------------------------------------------------------
//! 
void 
BSP2::dump( BSP2Node* node ) const
{
   DBG_BLOCK( os_bsp2, "dump node: " << node );

   if( node->isLeaf() )
   {
      if( node->in() )
      {
         DBG_MSG( os_bsp2, "IN" );
      }
      else
      {
         DBG_MSG( os_bsp2, "OUT" );
      }
      return;
   }

   DBG_MSG( os_bsp2, "p: "    << node->plane() << " #edges: " << node->numEdges() );
   DBG_MSG( os_bsp2, "back: " << node->back()  << " front: "  << node->front() );
   for( uint e = 0; e < node->numEdges(); ++e  )
   {
      const Edgef edge = node->edge(e);
      DBG_MSG( os_bsp2, " " << edge.vertex(0) << edge.vertex(1) << " n: " << edge.normal() );
   }
   dump( node->back() );
   dump( node->front() );
}

NAMESPACE_END
