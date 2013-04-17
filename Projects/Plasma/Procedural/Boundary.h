/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_BOUNDARY
#define PLASMA_BOUNDARY


#include <Plasma/StdDefs.h>
#include <Plasma/Procedural/BoundaryPolygon.h>

#include <CGMath/Ref.h>
#include <CGMath/AABBox.h>
#include <CGMath/AARect.h>

#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>   
#include <Base/ADT/Vector.h>
#include <Base/ADT/ConstString.h>


NAMESPACE_BEGIN

class Component;

/*==============================================================================
   CLASS Boundary
==============================================================================*/
//! Define a polytope of one of 3 types: polygon, polyhedron or prism.
//! 
class Boundary :
   public RCObject
{
   public:
      
   /*----- types and enumerations ----*/

   enum {
      POLYGON,
      POLYHEDRON,
      PRISM
   };

   enum {
      NEGX     = 0x01,
      POSX     = 0x02,
      NEGY     = 0x04,
      POSY     = 0x08,
      NEGZ     = 0x10,
      POSZ     = 0x20,
      EXTRUDED = 0x40,
      BOTTOM   = NEGY,
      TOP      = POSY,
      SIDE     = NEGX | POSX | NEGZ | POSZ,
      ALL      = BOTTOM | TOP | SIDE
   };
   
   /*----- classes -----*/
   
   struct Face;
   
   /*----- static methods -----*/

   // 2D.
   static RCP<Boundary> create( const BoundaryPolygon& );
   static RCP<Boundary> create( const Boundary&, const AARectf&, const Reff& );
   static RCP<Boundary> create( const Boundary&, int face, const AARectf&, const Reff& );
   // 3D.
   static RCP<Boundary> create( const AABBoxf& );
   static RCP<Boundary> create( const Vec3f& dim, const Reff& ref );
   static RCP<Boundary> create( const Vector< RCP<BoundaryPolygon> >& );
   static RCP<Boundary> create( const BoundaryPolygon&, float h );
   static RCP<Boundary> create( const BoundaryPolygon&, const Vec3f& dir );
   static RCP<Boundary> create( const BoundaryPolygon& srcPoly, const BoundaryPolygon& dstPoly );
   static RCP<Boundary> create( const Boundary&, float h );
   static RCP<Boundary> create( const Boundary&, int face, float h );
   static RCP<Boundary> create( const Boundary&, const AABBoxf&, const Reff& );
   static RCP<Boundary> create( const Boundary&, const Boundary& );
   
   static RCP<Boundary> subtract( Component*, const Vector<Component*>& );
   static RCP<Boundary> merge( const Vector<Component*>& );
   static RCP<Boundary> intersect( const Vector<Component*>& );

   /*----- methods -----*/

   Boundary();

   // Memory allocation.
   inline void reserveVertices( uint qty );
   inline void reserveHEdges( uint qty );
   inline void reserveFaces( uint qty );
   
   // Creation.
   inline void addVertex( const Vec3f& v );
   inline void addHEdge( uint v );
   inline uint addUniqueVertex( const Vec3f& v, float threshold );
   
   // Vertices should be given in ccw order, normal facing the exterior.
   inline void addFace( uint v0, uint v1, uint v2 );
   inline void addFace( uint v0, uint v1, uint v2, uint v3 );
   inline void addFace( uint numVertices, uint startHEdge );
   
   void computeDerivedData();

   // Accessor.
   inline uint type() const;
   inline uint numVertices() const;
   inline uint numFaces() const;
   inline uint numHEdges() const;
   inline uint numVertices( uint face ) const;
   
   // Vertices.
   inline const Vec3f& vertex( uint v ) const;
   inline const Vec3f& vertex( uint face, uint v ) const;
   inline Vec3f& vertex( uint v );
   inline Vec3f& vertex( uint face, uint v );
   inline uint vertexID( uint face, uint v ) const;

   // Vertices flags.
   inline uchar flag( uint v ) const;
   inline void flag( uint v, uchar f );
   
   // Faces.
   inline const Face& face( uint f ) const;
   inline const ConstString& id( uint f ) const;
   inline void id( uint f, const ConstString& );
   
   AABBoxf boundingBox() const;
   bool isConvex( uint f ) const;
   float perimeter( uint face ) const;
   int orientation( const Vec3f& ) const;
   int orientation( uint face ) const;
   int orientation( uint face, const Reff& ) const;
   void computeRefAndSize( const Reff&, Reff&, Vec3f& size ) const;
   void computeFaceRefAndSize( uint face, Reff&, Vec3f& size ) const;

   // Operations.
   void rotate( const Quatf& q );
   void translate( const Vec3f& v );
   void transform( const Mat4f& );
   void transform( const Reff& );
   void shrink( float offset[] );

   void clear();

   void print( TextStream& os = StdErr ) const;

protected:
      
   /*----- methods -----*/

   virtual ~Boundary();

   /*----- data members -----*/
  
   uint               _type;
   Vector< uchar >    _flags;
   Vector< Vec3f >    _vertices;
   Vector< Face >     _faces;
   Vector< uint16_t > _hedges;
};

/*==============================================================================
   STRUCTURE Face
==============================================================================*/

struct Boundary::Face
{
   /*----- methods -----*/
   
   Face( uint size, uint start ) :
      _size( size ), _startID( start ) {}
            
   uint numVertices() const      { return _size; }
   const Vec3f& normal() const   { return _plane.direction(); }
   const Planef& plane() const   { return _plane; }
   const ConstString& id() const { return _id; }
            
   /*----- data members -----*/
   
   ConstString _id;
   uint16_t    _size;
   uint16_t    _startID;
   Planef      _plane;
};

//------------------------------------------------------------------------------
//!
inline void 
Boundary::reserveVertices( uint qty )
{
   _vertices.reserve( qty );
   _flags.reserve( qty );
}
   
//------------------------------------------------------------------------------
//!
inline void 
Boundary::reserveHEdges( uint qty )
{
   _hedges.reserve( qty );
}
   
//------------------------------------------------------------------------------
//!
inline void 
Boundary::reserveFaces( uint qty )
{
   _faces.reserve( qty );
}
   
//------------------------------------------------------------------------------
//!
inline void 
Boundary::addVertex( const Vec3f& v )
{
   _vertices.pushBack(v);
   _flags.pushBack(0);
}
   
//------------------------------------------------------------------------------
//!
inline void 
Boundary::addHEdge( uint v )
{
   _hedges.pushBack(v);
}
   
//------------------------------------------------------------------------------
//!
inline void 
Boundary::addFace( uint v0, uint v1, uint v2 )
{
   _faces.pushBack( Face( 3, uint(_hedges.size()) ) );
   _hedges.pushBack( v0 );
   _hedges.pushBack( v1 );
   _hedges.pushBack( v2 );
}

//------------------------------------------------------------------------------
//!
inline void 
Boundary::addFace( uint v0, uint v1, uint v2, uint v3 )
{
   _faces.pushBack( Face( 4, uint(_hedges.size()) ) );
   _hedges.pushBack( v0 );
   _hedges.pushBack( v1 );
   _hedges.pushBack( v2 );
   _hedges.pushBack( v3 );
}

//------------------------------------------------------------------------------
//! 
inline void 
Boundary::addFace( uint numVertices, uint startHEdge )
{
   _faces.pushBack( Face( numVertices, startHEdge ) );
}

//------------------------------------------------------------------------------
//! 
inline uint
Boundary::type() const
{
   return _type;
}

//------------------------------------------------------------------------------
//!
inline uint 
Boundary::numVertices() const
{
   return uint(_vertices.size());
}

//------------------------------------------------------------------------------
//!
inline uint 
Boundary::numFaces() const
{
   return uint(_faces.size());
}

//------------------------------------------------------------------------------
//! 
inline uint
Boundary::numHEdges() const
{
   return uint(_hedges.size());
}

//------------------------------------------------------------------------------
//!
inline uint
Boundary::numVertices( uint face ) const
{
   return _faces[face]._size;
}
   
//------------------------------------------------------------------------------
//!
inline const Vec3f&
Boundary::vertex( uint v ) const
{
   return _vertices[v];
}

//------------------------------------------------------------------------------
//!
inline const Vec3f&
Boundary::vertex( uint face, uint v ) const
{
   return _vertices[vertexID(face,v)];
}

//------------------------------------------------------------------------------
//!
inline Vec3f&
Boundary::vertex( uint face, uint v )
{
   return _vertices[vertexID(face,v)];
}

//------------------------------------------------------------------------------
//! 
inline Vec3f&
Boundary::vertex( uint v )
{
   return _vertices[v];
}

//------------------------------------------------------------------------------
//! 
inline uint
Boundary::vertexID( uint face, uint v ) const
{
   return _hedges[_faces[face]._startID+v];
}

//------------------------------------------------------------------------------
//! 
inline uchar 
Boundary::flag( uint v ) const
{
   return _flags[v];
}

//------------------------------------------------------------------------------
//! 
inline void 
Boundary::flag( uint v, uchar f )
{
   _flags[v] = f;
}

//------------------------------------------------------------------------------
//!
inline const Boundary::Face&
Boundary::face( uint f ) const
{
   return _faces[f];
}

//------------------------------------------------------------------------------
//!
inline const ConstString&
Boundary::id( uint f ) const
{
   return _faces[f].id();
}

//------------------------------------------------------------------------------
//!
inline void 
Boundary::id( uint f, const ConstString& str )
{
   _faces[f]._id = str;
}

//------------------------------------------------------------------------------
//!
inline uint
Boundary::addUniqueVertex( const Vec3f& v, float threshold )
{
   for( uint i = 0; i < _vertices.size(); ++i )
   {
      if( _vertices[i].equal( v, threshold ) ) return i;
   }
   _vertices.pushBack( v );
   _flags.pushBack(0);
   return uint(_vertices.size())-1;
}

NAMESPACE_END

#endif
