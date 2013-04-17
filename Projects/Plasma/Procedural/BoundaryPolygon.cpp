/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Procedural/BoundaryPolygon.h>
#include <Plasma/Procedural/Boundary.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS BoundaryPolygon
==============================================================================*/

//------------------------------------------------------------------------------
//! 
RCP<BoundaryPolygon>
BoundaryPolygon::create( const AARectf& box, const Mat4f& m )
{
   RCP<BoundaryPolygon> poly( new BoundaryPolygon() );
   poly->reserveVertices(4);
   poly->addVertex( m * Vec3f( box.corner(0), 0.0f ) );
   poly->addVertex( m * Vec3f( box.corner(1), 0.0f ) );
   poly->addVertex( m * Vec3f( box.corner(3), 0.0f ) );
   poly->addVertex( m * Vec3f( box.corner(2), 0.0f ) );
   poly->computeDerivedData();

   return poly;
}

//------------------------------------------------------------------------------
//! 
RCP<BoundaryPolygon> 
BoundaryPolygon::create( const AABBoxf& box, int f, const Mat4f& m )
{
   RCP<BoundaryPolygon> poly( new BoundaryPolygon() );
   int ids[6][4] = { {0,4,6,2}, {4,5,7,6}, {5,1,3,7}, {0,2,3,1}, {0,1,5,4}, {6,7,3,2} };

   int* id = ids[f];
   poly->reserveVertices( 4 );
   poly->addVertex( m*box.corner(id[0]) );
   poly->addVertex( m*box.corner(id[1]) );
   poly->addVertex( m*box.corner(id[2]) );
   poly->addVertex( m*box.corner(id[3]) );
   poly->computeDerivedData();

   return poly;
}

//------------------------------------------------------------------------------
//! 
RCP<BoundaryPolygon> 
BoundaryPolygon::create( const Boundary& b, int f )
{
   RCP<BoundaryPolygon> poly( new BoundaryPolygon() );
   uint numVertices = b.numVertices(f);
   poly->id( b.id(f) );
   poly->reserveVertices( numVertices );
   for( uint v = 0; v < numVertices; ++v )
   {
      poly->addVertex( b.vertex(f,v) );
   }
   poly->computeDerivedData();

   return poly;
}

//------------------------------------------------------------------------------
//! 
RCP<BoundaryPolygon> 
BoundaryPolygon::create( const BoundaryPolygon& p, const Vec3f& scale, const Reff& ref )
{
   RCP<BoundaryPolygon> poly( new BoundaryPolygon() );
   uint numVertices = uint(p.numVertices());
   poly->reserveVertices( numVertices );

   Refd refd = ref;
   Mat4d ml  = refd.globalToLocal();
   Mat4d mg  = refd.localToGlobal();
   Vec3f c   = p.computeCentroid();
   Vec3d cl  = ml * Vec3d(c);
   Mat4f m   = 
      (mg*Mat4d::translation( cl.x, cl.y, scale.z )*
      Mat4d::scaling( scale.x, scale.y, 1.0 )*
      Mat4d::translation( -cl.x, -cl.y, 0.0 )*ml);

   for( uint v = 0; v < numVertices; ++v )
   {
      poly->addVertex( m*p.vertex(v) );
   }
   poly->plane( p.plane() );

   return poly;
}

//------------------------------------------------------------------------------
//! 
void 
BoundaryPolygon::create( 
   const AABBoxf&                  box, 
   const Mat4f&                    m, 
   Vector< RCP<BoundaryPolygon> >& polys
)
{
   polys.clear();
   RCP<BoundaryPolygon> poly;
   int ids[6][4] = { {0,4,6,2}, {4,5,7,6}, {5,1,3,7}, {0,2,3,1}, {0,1,5,4}, {6,7,3,2} };

   Vec3f vertices[8];
   for( uint i = 0; i < 8; ++i ) vertices[i] = m*box.corner(i);
   for( uint i = 0; i < 6; ++i )
   {
      int* id = ids[i];
      poly = new BoundaryPolygon();
      poly->reserveVertices( 4 );
      poly->addVertex( vertices[id[0]] );
      poly->addVertex( vertices[id[1]] );
      poly->addVertex( vertices[id[2]] );
      poly->addVertex( vertices[id[3]] );
      poly->computeDerivedData();
      polys.pushBack( poly );
   }
}

//------------------------------------------------------------------------------
//! 
void 
BoundaryPolygon::create( const Boundary& b, Vector< RCP<BoundaryPolygon> >& polys )
{
   polys.clear();
   RCP<BoundaryPolygon> poly;

   for( uint f = 0; f < b.numFaces(); ++f )
   {
      poly = new BoundaryPolygon();
      poly->id( b.id(f) );
      uint numVertices = b.numVertices(f);
      poly->reserveVertices( numVertices );
      for( uint v = 0; v < numVertices; ++v )
      {
         poly->addVertex( b.vertex(f,v) );
      }
      poly->computeDerivedData();
      polys.pushBack( poly );
   }
}

//------------------------------------------------------------------------------
//! 
BoundaryPolygon::BoundaryPolygon()
{
}

//------------------------------------------------------------------------------
//! 
BoundaryPolygon::~BoundaryPolygon()
{
}

NAMESPACE_END
