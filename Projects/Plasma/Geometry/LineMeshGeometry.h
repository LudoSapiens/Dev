/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_LINE_MESH_GEOMETRY_H
#define PLASMA_LINE_MESH_GEOMETRY_H

#include <Plasma/StdDefs.h>

#include <Plasma/Geometry/MeshGeometry.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS LineMeshGeometry
==============================================================================*/
class LineMeshGeometry:
   public MeshGeometry
{
public:

   /*==============================================================================
     CLASS Vertex
   ==============================================================================*/
   class Vertex
   {
   public:
      Vertex( float* v0, float* v1 ): _v0( v0 ), _v1( v1 ) {}

      inline const Vec3f& position() const;
      inline         void position( const Vec3f& v );

      inline const Vec3f& previous() const;
      inline         void previous( const Vec3f& v );

      inline const Vec3f& next() const;
      inline         void next( const Vec3f& v );

      inline        float t() const;
      inline         void t( float v );

      inline        float radius() const;
      inline         void radius( float v );

      inline const Vec4f& color() const;
      inline         void color( const Vec4f& v );

      inline         bool operator==( const Vertex& v ) const;
      inline         bool operator< ( const Vertex& v ) const;
      inline         bool operator<=( const Vertex& v ) const;

      inline  Vertex& operator++();

   protected:
      float*  _v0;
      float*  _v1;
   }; //class Vertex

   /*==============================================================================
     CLASS Segment
   ==============================================================================*/
   class Segment
   {
   public:
      Segment( uint32_t* data ): _data( data ) {}

      inline uint32_t v0() const;
      inline     void v0( uint32_t v );

      inline uint32_t v1() const;
      inline     void v1( uint32_t v );

      inline     void set( uint32_t v0, uint32_t v1 );

      inline         bool operator==( const Segment& v ) const;
      inline         bool operator< ( const Segment& v ) const;
      inline         bool operator<=( const Segment& v ) const;

      inline  Segment& operator++();

   protected:
      uint32_t*  _data;
   }; //class Segment


   /*----- methods -----*/

   LineMeshGeometry();
   virtual ~LineMeshGeometry();

   inline void  allocateVertices( uint maxVertices );
   inline void  allocateSegments( uint maxSegments );

   inline uint  numVertices() const;
   inline uint  numSegments() const;

   inline const Vertex  vertex( uint vertexID ) const;
   inline Vertex  vertex( uint vertexID );
   inline Vertex  vertexBegin();
   inline Vertex  vertexEnd();

   inline const Segment  segment( uint segID ) const;
   inline Segment  segment( uint segID );

   PLASMA_DLL_API void  updateAdjacency();

   PLASMA_DLL_API void  print( TextStream& os = StdErr ) const;

protected:

   // Sample data layout:
   //
   // Lines (2 lines, first of 3 segments, second of only 1):
   //
   //   +=====+=====+=====+     +=====+
   //
   //   <-------(0)------->     <-(1)->
   //
   // Line vertices:
   //   +-----+-----+-----+     +-----+
   //   |     |     |     |     |     |
   //  (0)   (1)   (2)   (3)   (4)   (5)
   //   |     |     |     |     |     |
   //   +-----+-----+-----+     +-----+
   //
   // Line segments:
   //   +-----+-----+-----+     +-----+
   //   |     |     |     |     |     |
   //   | (0) | (1) | (2) |     | (3) |
   //   |     |     |     |     |     |
   //   +-----+-----+-----+     +-----+
   //
   // Geom vertices:
   //  (1)---(3)---(5)---(7)   (9)---(11)
   //   |     |     |     |     |     |
   //   |     |     |     |     |     |
   //   |     |     |     |     |     |
   //  (0)---(2)---(4)---(6)   (8)---(10)
   //
   // Geom indices (quads):
   //   Line #0:
   //     (0,2,3) (0,3,1)
   //     (2,4,5) (2,5,3)
   //     (4,6,7) (4,7,5)
   //   Line #1:
   //     (8,10,11) (8,11,9)

private:
}; //class LineMeshGeometry

//------------------------------------------------------------------------------
//!
inline void
LineMeshGeometry::allocateVertices( uint maxVertices )
{
   // Every line vertex has 2 vertices (we render quads).
   MeshGeometry::allocateVertices( maxVertices*2 );
}

//------------------------------------------------------------------------------
//!
inline void
LineMeshGeometry::allocateSegments( uint maxSegments )
{
   // Every segment renders as a quad.
   MeshGeometry::allocateIndices( maxSegments*6 );
}

//------------------------------------------------------------------------------
//!
inline uint
LineMeshGeometry::numVertices() const
{
   return _numVertices/2;
}

//------------------------------------------------------------------------------
//!
inline uint
LineMeshGeometry::numSegments() const
{
   return _numIndices/6;
}

//------------------------------------------------------------------------------
//!
inline const LineMeshGeometry::Vertex
LineMeshGeometry::vertex( uint vertexID ) const
{
   float* v0 = _vData + vertexID*_vStride*2;
   return Vertex( v0, v0 + _vStride );
}

//------------------------------------------------------------------------------
//!
inline LineMeshGeometry::Vertex
LineMeshGeometry::vertex( uint vertexID )
{
   float* v0 = _vData + vertexID*_vStride*2;
   return Vertex( v0, v0 + _vStride );
}

//------------------------------------------------------------------------------
//!
inline LineMeshGeometry::Vertex
LineMeshGeometry::vertexBegin()
{
   return Vertex( _vData, _vData + _vStride );
}

//------------------------------------------------------------------------------
//!
inline LineMeshGeometry::Vertex
LineMeshGeometry::vertexEnd()
{
   float* v0 = _vData + _numVertices*_vStride;
   return Vertex( v0, v0 + _vStride );
}

//------------------------------------------------------------------------------
//!
inline const LineMeshGeometry::Segment
LineMeshGeometry::segment( uint segID ) const
{
   return Segment( _iData + segID*6 );
}

//------------------------------------------------------------------------------
//!
inline LineMeshGeometry::Segment
LineMeshGeometry::segment( uint segID )
{
   return Segment( _iData + segID*6 );
}


/*==============================================================================
  CLASS LineMeshGeometry::Vertex
==============================================================================*/

//------------------------------------------------------------------------------
//!
inline const Vec3f&
LineMeshGeometry::Vertex::position() const
{
   return Vec3f::as( _v0 );
}

//------------------------------------------------------------------------------
//!
inline void
LineMeshGeometry::Vertex::position( const Vec3f& v )
{
   Vec3f::as( _v0 ) = v;
   Vec3f::as( _v1 ) = v;
}

//------------------------------------------------------------------------------
//!
inline const Vec3f&
LineMeshGeometry::Vertex::previous() const
{
   return Vec3f::as( _v0+5 );
}

//------------------------------------------------------------------------------
//!
inline void
LineMeshGeometry::Vertex::previous( const Vec3f& v )
{
   Vec3f::as( _v0+5 ) = v;
   Vec3f::as( _v1+5 ) = v;
}

//------------------------------------------------------------------------------
//!
inline const Vec3f&
LineMeshGeometry::Vertex::next() const
{
   return Vec3f::as( _v0+8 );
}

//------------------------------------------------------------------------------
//!
inline void
LineMeshGeometry::Vertex::next( const Vec3f& v )
{
   Vec3f::as( _v0+8 ) = v;
   Vec3f::as( _v1+8 ) = v;
}

//------------------------------------------------------------------------------
//!
inline float
LineMeshGeometry::Vertex::t() const
{
   return _v0[3];
}

//------------------------------------------------------------------------------
//!
inline void
LineMeshGeometry::Vertex::t( float v )
{
   _v0[3] = v;
   _v1[3] = v;
}

//------------------------------------------------------------------------------
//!
inline float
LineMeshGeometry::Vertex::radius() const
{
   return _v1[4]; // Return the positive one.
}

//------------------------------------------------------------------------------
//!
inline void
LineMeshGeometry::Vertex::radius( float v )
{
   _v0[4] = -v;  // Place negative at the bottom of the quad.
   _v1[4] =  v;
}

//------------------------------------------------------------------------------
//!
inline const Vec4f&
LineMeshGeometry::Vertex::color() const
{
   return Vec4f::as( _v0+11 );
}

//------------------------------------------------------------------------------
//!
inline void
LineMeshGeometry::Vertex::color( const Vec4f& v )
{
   Vec4f::as( _v0+11 ) = v;
   Vec4f::as( _v1+11 ) = v;
}

//------------------------------------------------------------------------------
//!
inline bool
LineMeshGeometry::Vertex::operator==( const LineMeshGeometry::Vertex& v ) const
{
   return _v0 == v._v0;
}

//------------------------------------------------------------------------------
//!
inline bool
LineMeshGeometry::Vertex::operator< ( const LineMeshGeometry::Vertex& v ) const
{
   return _v0 <  v._v0;
}

//------------------------------------------------------------------------------
//!
inline bool
LineMeshGeometry::Vertex::operator<=( const LineMeshGeometry::Vertex& v ) const
{
   return _v0 <= v._v0;
}

//------------------------------------------------------------------------------
//!
inline LineMeshGeometry::Vertex&
LineMeshGeometry::Vertex::operator++()
{
   size_t d = 2*(_v1 - _v0);
   _v0 += d;
   _v1 += d;
   return *this;
}


/*==============================================================================
  CLASS LineMeshGeometry::Segment
==============================================================================*/

//------------------------------------------------------------------------------
//!
inline uint32_t
LineMeshGeometry::Segment::v0() const
{
   return _data[0]/2;
}

//------------------------------------------------------------------------------
//!
inline void
LineMeshGeometry::Segment::v0( uint32_t v )
{
   // Set 3 indices out of the quad's 6.
   _data[0] = v*2;
   _data[3] = _data[0];
   _data[5] = _data[0]+1;
}
//------------------------------------------------------------------------------
//!
inline uint32_t
LineMeshGeometry::Segment::v1() const
{
   return _data[1]/2;
}

//------------------------------------------------------------------------------
//!
inline void
LineMeshGeometry::Segment::v1( uint32_t v )
{
   // Set 3 vertices out of the quad's 6.
   _data[1] = v*2;
   _data[2] = _data[1]+1;
   _data[4] = _data[2];
}

//------------------------------------------------------------------------------
//!
inline void
LineMeshGeometry::Segment::set( uint32_t nv0, uint32_t nv1 )
{
   v0( nv0 );
   v1( nv1 );
}

//------------------------------------------------------------------------------
//!
inline bool
LineMeshGeometry::Segment::operator==( const LineMeshGeometry::Segment& v ) const
{
   return _data == v._data;
}

//------------------------------------------------------------------------------
//!
inline bool
LineMeshGeometry::Segment::operator< ( const LineMeshGeometry::Segment& v ) const
{
   return _data <  v._data;
}

//------------------------------------------------------------------------------
//!
inline bool
LineMeshGeometry::Segment::operator<=( const LineMeshGeometry::Segment& v ) const
{
   return _data <= v._data;
}

//------------------------------------------------------------------------------
//!
inline LineMeshGeometry::Segment&
LineMeshGeometry::Segment::operator++()
{
   _data += 6;
   return *this;
}


NAMESPACE_END

#endif //PLASMA_LINE_MESH_GEOMETRY_H
