/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Procedural/Boundary.h>
#include <Plasma/Procedural/BSP2.h>
#include <Plasma/Procedural/BSP3.h>
#include <Plasma/Procedural/Component.h>

#include <Base/Dbg/DebugStream.h>

/*==============================================================================
   UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

DBG_STREAM( os_b, "Boundary" );

const float _errorPlane = 1.0f / 1024.0f;

UNNAMESPACE_END


NAMESPACE_BEGIN

/*==============================================================================
   CLASS Boundary
==============================================================================*/

//------------------------------------------------------------------------------
//! 
RCP<Boundary>
Boundary::create( const BoundaryPolygon& face )
{
   DBG_BLOCK( os_b, "Boundary::create" );

   RCP<Boundary> boundary = new Boundary();
   boundary->_type = POLYGON;
   boundary->reserveVertices( uint(face.numVertices()) );
   boundary->reserveHEdges( uint(face.numVertices()) );
   boundary->reserveFaces( 1 );

   boundary->addFace( uint(face.numVertices()), 0 );
   boundary->id( 0, face.id() );
   uint v0 = uint(face.numVertices())-1;
   for( uint v1 = 0; v1 < face.numVertices(); v0=v1++ )
   {
      boundary->addVertex( face.vertex(v1) );
      boundary->addHEdge( v0 );
   }
   boundary->computeDerivedData();
   return boundary;
}

//------------------------------------------------------------------------------
//! 
RCP<Boundary> 
Boundary::create( const Boundary& b, const AARectf& r, const Reff& ref )
{
   DBG_BLOCK( os_b, "Boundary::create" );
   DBG_MSG( os_b, "rect " << r << " ref: " << ref );

   BSP2 bsp;
   Vector< RCP<BoundaryPolygon> > polys;

   // BSP for initial boundary.
   BoundaryPolygon::create( b, polys );
   bsp.build( *polys[0] );
   for( uint i = 1; i < polys.size(); ++i )
   {
      bsp.add( *polys[i] );
   }

   // Intersect the BSP with the bounding box.
   RCP<BoundaryPolygon> poly = BoundaryPolygon::create( r, ref.toMatrix() );
   bsp.intersect( *poly );

   // Create the new boundary.
   bsp.computePolygons( polys );
   RCP<Boundary> boundary = create( polys );
   boundary->_type        = POLYGON;

   return boundary;
}

//------------------------------------------------------------------------------
//! 
RCP<Boundary> 
Boundary::create( const Boundary& b, int face, const AARectf& r, const Reff& ref )
{
   DBG_BLOCK( os_b, "Boundary::create" );
   DBG_MSG( os_b, "rect " << r << " ref: " << ref );

   BSP2 bsp;

   // BSP for initial boundary.
   RCP<BoundaryPolygon> poly0 = BoundaryPolygon::create( b, face );
   bsp.build( *poly0 );

   // Intersect the BSP with the bounding box.
   RCP<BoundaryPolygon> poly1 = BoundaryPolygon::create( r, ref.toMatrix() );
   bsp.intersect( *poly1 );

   // Create the new boundary.
   Vector< RCP<BoundaryPolygon> > polys;
   bsp.computePolygons( polys );
   RCP<Boundary> boundary = create( polys );
   boundary->_type        = POLYGON;

   return boundary;
}

//------------------------------------------------------------------------------
//! 
RCP<Boundary>
Boundary::create( const AABBoxf& bbox )
{
   RCP<Boundary> boundary = new Boundary();
   int dimension = bbox.size().z < CGConstf::epsilon(256.0f) ? 2 : 3;

   if( dimension == 2 )
   {
      boundary->_type = POLYGON;
      boundary->reserveVertices( 4 );
      boundary->reserveHEdges( 4 );
      boundary->reserveFaces( 1 );

      boundary->addVertex( bbox.corner(0) );
      boundary->addVertex( bbox.corner(1) );
      boundary->addVertex( bbox.corner(2) );
      boundary->addVertex( bbox.corner(3) );
      boundary->addFace( 0, 1, 3, 2 );
   }
   else
   {
      boundary->_type = POLYHEDRON;
      boundary->reserveVertices( 8 );
      boundary->reserveHEdges( 24 );
      boundary->reserveFaces( 6 );

      boundary->addVertex( bbox.corner(0) );
      boundary->addVertex( bbox.corner(1) );
      boundary->addVertex( bbox.corner(2) );
      boundary->addVertex( bbox.corner(3) );
      boundary->addVertex( bbox.corner(4) );
      boundary->addVertex( bbox.corner(5) );
      boundary->addVertex( bbox.corner(6) );
      boundary->addVertex( bbox.corner(7) );

      boundary->addFace( 0, 4, 6, 2 );
      boundary->addFace( 5, 1, 3, 7 );
      boundary->addFace( 0, 1, 5, 4 );
      boundary->addFace( 3, 2, 6, 7 );
      boundary->addFace( 1, 0, 2, 3 );
      boundary->addFace( 4, 5, 7, 6 );
   }

   boundary->computeDerivedData();
   return boundary;
}


//------------------------------------------------------------------------------
//!
RCP<Boundary> 
Boundary::create( const Vec3f& dim, const Reff& ref )
{
   RCP<Boundary> boundary = new Boundary();
   boundary->_type = POLYHEDRON;
   
   boundary->reserveVertices( 8 );
   boundary->reserveHEdges( 24 );
   boundary->reserveFaces( 6 );
      
   Mat4f mat = ref.localToGlobal();
      
   boundary->addVertex( mat * Vec3f( 0.0f,  0.0f,  0.0f ) );
   boundary->addVertex( mat * Vec3f( dim.x, 0.0f,  0.0f ) );
   boundary->addVertex( mat * Vec3f( 0.0f,  dim.y, 0.0f ) );
   boundary->addVertex( mat * Vec3f( dim.x, dim.y, 0.0f ) );
   boundary->addVertex( mat * Vec3f( 0.0f,  0.0f,  dim.z ) );
   boundary->addVertex( mat * Vec3f( dim.x, 0.0f,  dim.z ) );
   boundary->addVertex( mat * Vec3f( 0.0f,  dim.y, dim.z ) );
   boundary->addVertex( mat * Vec3f( dim.x, dim.y, dim.z ) );

   boundary->addFace( 0, 4, 6, 2 );
   boundary->addFace( 5, 1, 3, 7 );
   boundary->addFace( 0, 1, 5, 4 );
   boundary->addFace( 3, 2, 6, 7 );
   boundary->addFace( 1, 0, 2, 3 );
   boundary->addFace( 4, 5, 7, 6 );

   boundary->computeDerivedData();
   
   return boundary;
}

//------------------------------------------------------------------------------
//!
RCP<Boundary>
Boundary::create( const Vector< RCP<BoundaryPolygon> >& faces )
{
   DBG_BLOCK( os_b, "Boundary::create" );
   DBG_MSG( os_b, faces.size() << " # of faces" );
   RCP<Boundary> boundary = new Boundary();
   boundary->_type        = POLYHEDRON;
   
   float threshold = 1.0f/1024.0f;

   for( uint i = 0; i < faces.size(); ++i )
   {
      uint numVertices = uint(faces[i]->numVertices());
      DBG_MSG( os_b, "face " << i << " with " << numVertices << " vertices" );
     
      // Add a new face.
      boundary->addFace( numVertices, boundary->numHEdges() );
      boundary->id( i, faces[i]->id() );
      for( uint v = 0; v < numVertices; ++v )
      {
         uint v0 = boundary->addUniqueVertex( faces[i]->vertex(v), threshold );
         boundary->addHEdge( v0 );
         DBG_MSG( os_b, "v: " << boundary->vertex(v0) );
      }
   }
   boundary->computeDerivedData();
   return boundary;
}

//------------------------------------------------------------------------------
//! 
RCP<Boundary>
Boundary::create( const BoundaryPolygon& face, float h )
{
   return create( face, face.normal()*h );
}

//------------------------------------------------------------------------------
//! 
RCP<Boundary>
Boundary::create( const BoundaryPolygon& face, const Vec3f& dir )
{
   DBG_BLOCK( os_b, "Boundary::create" );
   RCP<Boundary> boundary = new Boundary();
   boundary->_type = PRISM;

   int nv = int(face.numVertices());

   // Reserve memory.
   boundary->reserveVertices( nv*2 );
   boundary->reserveHEdges( nv*6 );
   boundary->reserveFaces( nv+2 );

   // Add vertices.
   if( CGM::dot( dir, face.normal() ) > 0 )
   {
      for( int v = 0; v < nv; ++v )
      {
         boundary->addVertex( face.vertex(v) );
         boundary->addVertex( face.vertex(v)+dir );
      }
   }
   else
   {
      for( int v = nv-1; v >= 0; --v )
      {
         boundary->addVertex( face.vertex(v) );
         boundary->addVertex( face.vertex(v)+dir );
      }
   }

   // Bottom.
   boundary->addFace( nv, 0 );
   boundary->id( 0, face.id() );
   for( int v = nv-1; v >= 0; --v )
   {
      boundary->addHEdge( v*2 );
   }
   // Top.
   boundary->addFace( nv, nv );
   boundary->id( 1, face.id() );
   for( int v = 0; v < nv; ++v )
   {
      boundary->addHEdge( v*2+1 );
   }
   // Sides.
   for( int v0 = 0; v0 < nv; ++v0 )
   {
      int v1 = (v0+1)%nv;
      boundary->addFace( v0*2, v1*2, v1*2+1, v0*2+1 );
      boundary->id( v0+2, face.id() );
   }

   boundary->computeDerivedData();
   return boundary;
}

//------------------------------------------------------------------------------
//! 
RCP<Boundary>
Boundary::create( const BoundaryPolygon& srcPoly, const BoundaryPolygon& dstPoly )
{
   DBG_BLOCK( os_b, "Boundary::create" );
   RCP<Boundary> boundary = new Boundary();
   boundary->_type = POLYHEDRON;

   uint nv = uint(srcPoly.numVertices());
   CHECK( srcPoly.numVertices() == dstPoly.numVertices() );

   // Reserve memory.
   boundary->reserveVertices( nv*2 );
   boundary->reserveHEdges( nv*6 );
   boundary->reserveFaces( nv+2 );

   // Add vertices.
   Vector<uint>  vmap( nv );
   for( uint v = 0; v < nv; ++v )
   {
      boundary->addVertex( srcPoly.vertex(v) );
   }
   for( uint v = 0; v < nv; ++v )
   {
      vmap[v] = boundary->addUniqueVertex( dstPoly.vertex(v), _errorPlane );
   }

   // Bottom.
   boundary->addFace( nv, 0 );
   boundary->id( 0, srcPoly.id() );
   for( uint v = 0; v < nv; ++v )
   {
      boundary->addHEdge( nv-v-1 );
   }
   // Sides.
   for( uint v0 = 0; v0 < nv; ++v0 )
   {
      int v1 = (v0+1)%nv;
      int v2 = vmap[v1];
      int v3 = vmap[v0];
      if( v2 != v3 )
      {
         // Check if all vertices are coplanar or not.
         const Vec3f& p0 = boundary->vertex(v0);
         const Vec3f& p1 = boundary->vertex(v1);
         const Vec3f& p2 = boundary->vertex(v2);
         const Vec3f& p3 = boundary->vertex(v3);
         Planef plane( p0, p1, p2 );
         if( plane.contains(p3, _errorPlane) )
         {
            // Everything is coplanar, just add the polygon.
            boundary->addFace( v0, v1, v2, v3 );
         }
         else
         {
            // Not coplanar, so determine if we are convex or not.
            Vec3f n012 = plane.direction();
            Vec3f n023 = CGM::cross( p2-p0, p3-p0 );
            Vec3f n013 = CGM::cross( p1-p0, p3-p0 );
            Vec3f n123 = CGM::cross( p2-p1, p3-p1 );
            float dot1 = dot( n012, n023 );
            float dot2 = dot( n013, n123 );
            if( dot1 > dot2 )
            {
               // Any diagonal is valid (is one really better?).
               boundary->addFace( v0, v1, v2 );
               boundary->addFace( v0, v2, v3 );
            }
            else
            {
               // Use the other diagonal.
               boundary->addFace( v0, v1, v3 );
               boundary->addFace( v1, v2, v3 );
            }
         }
      }
      else
      {
         boundary->addFace( v0, v1, v2 );
      }
   }

   // Top.
   Vector<uint> verts;
   verts.reserve( nv );
   uint prev = vmap[0];
   verts.pushBack( prev );
   for( uint i = 1; i < nv; ++i )
   {
      uint v = vmap[i];
      // Skip degenerate edge.
      if( v == prev ) continue;

      verts.pushBack( v );
      if( v < prev )
      {
         // Search beginning of the loop.
         int j = int(verts.size()) - 2;
         while( verts[j] != v ) --j;
         uint s  = uint(verts.size());
         uint ps = s - j - 1;
         if( ps > 2 )
         {
            boundary->addFace( ps, boundary->numHEdges() );
            for( uint k = j+1; k < s; ++k )
            {
               boundary->addHEdge( verts[k] );
            }
         }
         // else degenerate triangle.
         verts.resize( j+1 );
      }
      prev = v;
   }
   // Add left-over polygon (if any).
   uint s = uint(verts.size());
   if( s > 2 )
   {
      boundary->addFace( s, boundary->numHEdges() );
      for( uint k = 0; k < s; ++k )
      {
         boundary->addHEdge( verts[k] );
      }
   }

   // Propagate the source polygon's id to every face.
   uint nf = boundary->numFaces();
   for( uint i = 0; i < nf; ++i )
   {
      boundary->id( i, srcPoly.id() );
   }

   boundary->computeDerivedData();

   return boundary;
}

//------------------------------------------------------------------------------
//! 
RCP<Boundary>
Boundary::create( const Boundary& b, float h )
{
   // Only extrude for 2D boundary, for now.
   if( b.numFaces() != 1 ) return 0;

   return create( b, 0, h );
}

//------------------------------------------------------------------------------
//! 
RCP<Boundary>
Boundary::create( const Boundary& b, int f, float h )
{
   DBG_BLOCK( os_b, "Boundary::create" );
   RCP<Boundary> boundary = new Boundary();
   boundary->_type = PRISM;

   int nv    = b.numVertices(f);
   Vec3f dir = b.face(f).normal() * h;

   // Reserve memory.
   boundary->reserveVertices( nv*2 );
   boundary->reserveHEdges( nv*6 );
   boundary->reserveFaces( nv+2 );

    // Add vertices.
   if( h > 0.0f )
   {
      for( int v = 0; v < nv; ++v )
      {
         const Vec3f& p = b.vertex(f,v);
         boundary->addVertex( p );
         boundary->addVertex( p+dir );
      }
   }
   else
   {
      for( int v = nv-1; v >= 0; --v )
      {
         const Vec3f& p = b.vertex(f,v);
         boundary->addVertex( p );
         boundary->addVertex( p+dir );
      }
   }

   const ConstString& id = b.id(f);

   // Bottom.
   boundary->addFace( nv, 0 );
   boundary->id( 0, id );
   for( int v = nv-1; v >= 0; --v )
   {
      boundary->addHEdge( v*2 );
   }
   // Top.
   boundary->addFace( nv, nv );
   boundary->id( 1,id );
   for( int v = 0; v < nv; ++v )
   {
      boundary->addHEdge( v*2+1 );
   }
   // Sides.
   for( int v0 = 0; v0 < nv; ++v0 )
   {
      int v1 = (v0+1)%nv;
      boundary->addFace( v0*2, v1*2, v1*2+1, v0*2+1 );
      boundary->id( v0+2, id );
   }

   boundary->computeDerivedData();
   return boundary;
}

//------------------------------------------------------------------------------
//! 
RCP<Boundary> 
Boundary::create( const Boundary& b, const AABBoxf& r, const Reff& ref )
{
   BSP3 bsp;
   Vector< RCP<BoundaryPolygon> > polys;

   // BSP for initial boundary.
   BoundaryPolygon::create( b, polys );
   bsp.build( polys );

   // Intersect the BSP with the bounding box.
   BoundaryPolygon::create( r, ref.toMatrix(), polys );
   bsp.intersect( polys );

   // Create the new boundary.
   bsp.computeBoundary( polys );
   RCP<Boundary> boundary = create( polys );
   boundary->_type = POLYHEDRON;

   return boundary;
}

//------------------------------------------------------------------------------
//! 
RCP<Boundary>
Boundary::create( const Boundary& b0, const Boundary& b1 )
{
   BSP3 bsp;
   Vector< RCP<BoundaryPolygon> > polys;

   // BSP for initial boundary.
   BoundaryPolygon::create( b0, polys );
   bsp.build( polys );

   // Intersect the BSP with the other boundary.
   BoundaryPolygon::create( b1, polys );
   bsp.intersect( polys );

   // Create the new boundary;
   bsp.computeBoundary( polys );
   RCP<Boundary> boundary = create( polys );
   boundary->_type = POLYHEDRON;

   return boundary;
}

//------------------------------------------------------------------------------
//! 
RCP<Boundary>
Boundary::subtract( Component* comp, const Vector<Component*>& sub )
{
   // For now only supports 3D component.
   if( comp->dimension() < 3 ) return 0;

   Vector< RCP<BoundaryPolygon> > polys;

   // Initial boundary.
   comp->resolveBoundary();
   if( comp->boundary() )
   {
      BoundaryPolygon::create( *comp->boundary(), polys );
   }
   else
   {
      BoundaryPolygon::create( AABBoxf( Vec3f(0.0f), comp->size() ), comp->referential().toMatrix(), polys );
   }

   BSP3 bsp;
   bsp.build( polys );

   // Boundaries to remove.
   for( uint i = 0; i < sub.size(); ++i )
   {
      if( sub[i]->dimension() < 3 ) continue;

      sub[i]->resolveBoundary();
      polys.clear();
      if( sub[i]->boundary() )
      {
         BoundaryPolygon::create( *sub[i]->boundary(), polys );
      }
      else
      {
         BoundaryPolygon::create( AABBoxf( Vec3f(0.0f), sub[i]->size() ), sub[i]->referential().toMatrix(), polys );
      }
      bsp.remove( polys );
   }

   // Create new boundary.
   bsp.computeBoundary( polys );
   RCP<Boundary> boundary = create( polys );
   boundary->_type = POLYHEDRON;

   return boundary;
}

//------------------------------------------------------------------------------
//! 
RCP<Boundary>
Boundary::merge( const Vector<Component*>& comps )
{
   if( comps.empty() ) return 0;

   // For now only supports 3D component.
   if( comps[0]->dimension() < 3 ) return 0;

   Vector< RCP<BoundaryPolygon> > polys;
   BSP3 bsp;

   // Boundaries to merge.
   for( uint i = 0; i < comps.size(); ++i )
   {
      if( comps[i]->dimension() < 3 ) continue;

      comps[i]->resolveBoundary();
      polys.clear();
      if( comps[i]->boundary() )
      {
         BoundaryPolygon::create( *comps[i]->boundary(), polys );
      }
      else
      {
         BoundaryPolygon::create( AABBoxf( Vec3f(0.0f), comps[i]->size() ), comps[i]->referential().toMatrix(), polys );
      }
      if( i == 0 )
      {
         bsp.build( polys );
      }
      else
      {
         bsp.add( polys );
      }
   }

   // Create new boundary.
   bsp.computeBoundary( polys );
   RCP<Boundary> boundary = create( polys );
   boundary->_type = POLYHEDRON;

   return boundary;
}

//------------------------------------------------------------------------------
//! 
RCP<Boundary>
Boundary::intersect( const Vector<Component*>& comps )
{
   if( comps.empty() ) return 0;

   // For now only supports 3D component.
   if( comps[0]->dimension() < 3 ) return 0;

   Vector< RCP<BoundaryPolygon> > polys;
   BSP3 bsp;

   // Boundaries to merge.
   for( uint i = 0; i < comps.size(); ++i )
   {
      if( comps[i]->dimension() < 3 ) continue;

      comps[i]->resolveBoundary();
      polys.clear();
      if( comps[i]->boundary() )
      {
         BoundaryPolygon::create( *comps[i]->boundary(), polys );
      }
      else
      {
         BoundaryPolygon::create( AABBoxf( Vec3f(0.0f), comps[i]->size() ), comps[i]->referential().toMatrix(), polys );
      }
      if( i == 0 )
      {
         bsp.build( polys );
      }
      else
      {
         bsp.intersect( polys );
      }
   }

   // Create new boundary.
   bsp.computeBoundary( polys );
   RCP<Boundary> boundary = create( polys );
   boundary->_type = POLYHEDRON;

   return boundary;
}

//------------------------------------------------------------------------------
//!
Boundary::Boundary() : _type(POLYHEDRON)
{
}

//------------------------------------------------------------------------------
//!
Boundary::~Boundary()
{
}

//------------------------------------------------------------------------------
//!
void
Boundary::computeDerivedData()
{
   DBG_BLOCK( os_b, "Boundary::computeDerivedData" );

   // Compute faces planes.
   for( uint f = 0; f < numFaces(); ++f )
   {
      // Compute face normal using Newell's method (Thanks to Filippo)
      Vec3f n(0.0f);
      uint num = numVertices(f);
      for( uint v = 0; v < num; ++v )
      {
         const Vec3f& v0 = vertex( f, v );
         const Vec3f& v1 = vertex( f, (v+1) % num );

         n.x += (v0.y-v1.y) * (v0.z+v1.z);
         n.y += (v0.z-v1.z) * (v0.x+v1.x);
         n.z += (v0.x-v1.x) * (v0.y+v1.y);
      }
      _faces[f]._plane = Planef( normalize(n), vertex( f, 0 ) );
   }
}

//------------------------------------------------------------------------------
//! 
AABBoxf 
Boundary::boundingBox() const
{
   AABBoxf bbox = AABBoxf::empty();
   for( uint v = 0; v < numVertices(); ++v )
   {
      bbox |= _vertices[v];
   }
   return bbox;
}

//------------------------------------------------------------------------------
//! 
bool 
Boundary::isConvex( uint f ) const
{
   int x, y, z;
   _faces[f].normal().secAxes( x, y, z );

   uint numV  = numVertices(f);
   uint order = 0;
   uint v0    = numV-2;
   uint v1    = numV-1;
   uint v2    = 0;

   for( ; v2 < numV; v0 = v1, v1=v2++ )
   {
      const Vec3f& p0 = vertex(f,v0);
      const Vec3f& p1 = vertex(f,v1);
      const Vec3f& p2 = vertex(f,v2);
      float z = (p1(x) - p0(x)) * (p2(y) - p1(y));
      z      -= (p1(y) - p0(y)) * (p2(x) - p1(x));

      if( z < 0.0f )
      {
         order |= 1;
      }
      else if( z >= 0.0f )
      {
         order |= 2;
      }
   }
   return order != 3;
}

//------------------------------------------------------------------------------
//!
float 
Boundary::perimeter( uint face ) const
{
   uint num = numVertices( face );
   float perimeter  = 0.0f;
   
   for( uint i = 0; i < num; ++i )
   {
      const Vec3f& v0 = vertex( face, i );
      const Vec3f& v1 = vertex( face, (i+1) % num );
      perimeter += (v1-v0).length();
   }
   return perimeter;
}

//------------------------------------------------------------------------------
//! 
int 
Boundary::orientation( const Vec3f& n ) const
{
   int a = n.maxComponent();
   if( n(a) > 0.0f ) return 1<<(a*2+1);
   return 1<<(a*2);
}

//------------------------------------------------------------------------------
//! 
int 
Boundary::orientation( uint face ) const
{
   int ori = (_type == PRISM)&&(face==1) ? EXTRUDED : 0;
   return ori | orientation( _faces[face].normal() );
}

//------------------------------------------------------------------------------
//! 
int 
Boundary::orientation( uint face, const Reff& ref ) const
{
   int ori = (_type == PRISM)&&(face==1) ? EXTRUDED : 0;
   return ori | orientation( ref.orientation().getInversed() * _faces[face].normal() );
}

//------------------------------------------------------------------------------
//! 
void 
Boundary::computeRefAndSize( const Reff& sref, Reff& ref, Vec3f& size ) const
{
   Mat4f mat = sref.globalToLocal();
   AABBoxf box = AABBoxf::empty();
   for( uint i = 0; i < numVertices(); ++i )
   {
      box |= mat * vertex(i);
   }
   size = box.size();
   ref.scale( sref.scale() );
   ref.orientation( sref.orientation() );
   ref.position( sref.toMatrix()*box.corner(0) );
}

//------------------------------------------------------------------------------
//! 
void 
Boundary::computeFaceRefAndSize( uint fi, Reff& ref, Vec3f& size ) const
{
   const Face& f = face(fi);

   // Up vector.
   int maxAxis = f.normal().maxComponent();
   Vec3f up    = (maxAxis == 1) ? Vec3f( 0.0f, 0.0f, 1.0f ) : Vec3f( 0.0f, 1.0f, 0.0f );

   // Compute referential.
   Vec3f x = cross( up, f.normal() ).normalize();
   Vec3f y = cross( f.normal(), x );

   Quatf ori = Quatf::axes( x, y, f.normal() );
   Mat3f mat = ori.getInversed().toMatrix3();

   AABBoxf box = AABBoxf::empty();
   for( uint i = 0; i < f.numVertices(); ++i )
   {
      box |= mat * vertex( fi, i );
   }

   size   = box.size();
   size.z = 0.0f;
   ref.scale(1.0f); 
   ref.orientation( ori );
   ref.position( ori.toMatrix3()*box.corner(0) );
}

//------------------------------------------------------------------------------
//! Rotates the boundary using the specified quaternion.
void
Boundary::rotate( const Quatf& q )
{
   Mat3f m = q.toMatrix3();

   // Update vertices positions.
   for( uint i = 0; i < _vertices.size(); ++i )
   {
      _vertices[i] = m * _vertices[i];
   }

   // Update faces planes.
   for( uint i = 0; i < _faces.size(); ++i )
   {
      _faces[i]._plane = Planef( m * _faces[i].normal(), vertex( i, 0 ) );
   }
}

//------------------------------------------------------------------------------
//! Translates the boundary using the specified vector.
void
Boundary::translate( const Vec3f& v )
{
   for( uint i = 0; i < _vertices.size(); ++i )
   {
      _vertices[i] += v;
   }
}

//------------------------------------------------------------------------------
//! 
void 
Boundary::transform( const Mat4f& m )
{
   // Update vertices positions.
   for( uint i = 0; i < _vertices.size(); ++i )
   {
      _vertices[i] = m * _vertices[i];
   }

   // Update faces planes.
   for( uint i = 0; i < _faces.size(); ++i )
   {
      _faces[i]._plane = Planef( m ^ _faces[i].normal(), vertex( i, 0 ) );
   }
}

//------------------------------------------------------------------------------
//!
void 
Boundary::transform( const Reff& r )
{
   transform( r.toMatrix() );
}

//------------------------------------------------------------------------------
//! 
void
Boundary::shrink( float /*offset*/[] )
{
   // TODO
}

//------------------------------------------------------------------------------
//! 
void
Boundary::clear()
{
   _flags.clear();
   _vertices.clear();
   _hedges.clear();
   _faces.clear();
}

//------------------------------------------------------------------------------
//!
void
Boundary::print( TextStream& os ) const
{
   os << "Boundary type: " << type() << nl;
   os << numVertices() << " vertices:" << nl;
   for( uint vi = 0; vi < numVertices(); ++vi )
   {
      os << "v[" << vi << "]: " << vertex(vi) << nl;
   }
   os << numFaces() << " faces:" << nl;
   for( uint fi = 0; fi < numFaces(); ++fi )
   {
      const Face& f = face(fi);
      os << "Face #" << fi << nl;
      os << "  plane: " << f.plane() << nl;
      os << "  " << f._size << " sides starting at " << f._startID << nl;
      for( uint i = 0; i < f._size; ++i )
      {
         uint vi = vertexID( fi, i );
         os << "  v=" << vi << " " << vertex(vi) << nl;
         CHECK( vertex(vi) == vertex(fi,i) );
      }
   }
}

NAMESPACE_END
