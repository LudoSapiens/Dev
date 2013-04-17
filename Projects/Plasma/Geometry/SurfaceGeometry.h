/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_SURFACEGEOMETRY_H
#define PLASMA_SURFACEGEOMETRY_H

#include <Plasma/StdDefs.h>
#include <Plasma/Geometry/Geometry.h>

#include <CGMath/BIH.h>
#include <CGMath/Vec4.h>

#include <Base/ADT/Vector.h>
#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>

NAMESPACE_BEGIN

class Skeleton;

/*==============================================================================
   CLASS SurfaceGeometry
==============================================================================*/
//! High level geometry description defining a manifold triangle mesh.
//! If the surface is create by only quads it represents a Catmull-Clark 
//! subdivision surface.
//! An edge is defined by an 32-bits adress: [24:8] ->[edge id, patch id].

class SurfaceGeometry
   : public Geometry
{

public: 

   /*----- classes -----*/

   class Patch;
   struct Weights;
   struct Face;

   /*----- constants -----*/
   
   enum {
      NULL_ID = 0xffffffff
   };

   /*----- types and enumerations ----*/

   typedef Vector< Vec3f >      VecContainer;
   typedef Vector< Vec2f >      UVContainer;
   typedef Vector< Vec4f >      TanContainer;
   typedef Vector< Weights >    WeightsContainer;
   typedef Vector< uint >       EdgeContainer;
   typedef Vector< Face >       FaceContainer;
   typedef Vector< RCP<Patch> > PatchContainer;
   typedef Vector< uchar >      LevelContainer;

   /*----- methods -----*/

   PLASMA_DLL_API SurfaceGeometry();

   // Building methods.
   inline void reserveVertices( uint qty );
   inline void reserveWeights( uint qty );
   inline void reserveMapping( uint qty );
   
   inline uint addVertex( const Vec3f& );
   inline uint addWeights( uchar qty, const Vec4i& bones, const Vec4f& weights );
   inline uint addMapping( const Vec2f& );
   inline RCP<Patch> addPatch();

   inline void rescaleNormal( uint id, float );

   // Accessors. Only read-only.
   inline const PatchContainer& patches() const;
   inline const Vec3f& vertex( uint id ) const;
   inline const Vec2f& mapping( uint id ) const;
   inline const Vec3f& normal( uint id ) const;
   inline const Vec4f& tangent( uint id ) const;
   inline const Weights& weight( uint id ) const;
   
   inline Vec3f wvertex( uint id, const Vector< Mat4f >& transform ) const;
   
   PLASMA_DLL_API uint numFaces() const;
   inline uint numVertices() const;
   inline uint numMappings() const;
   inline uint numNormals() const;
   inline uint numTangents() const;
   inline uint numWeights() const;
   inline uint numPatches() const;
   
   // Edges operators.
   inline uint prevEdge( uint edge );
   inline uint nextEdge( uint edge );
   inline uint neighborEdge( uint edge );
   inline uint orgVertexID( uint edge );
   inline uint dstVertexID( uint edge );
   inline uchar crease( uint edge );
   inline Face& faceFromEdge( uint edge );
   
   inline uint starFirst( uint vID );
   inline uint starNextCCW( uint edge );
   
   inline uint edge( uint p, uint f, uint e );
   
   // TODO: this permits to update derived data.
   // void updateVertex( uint ID, const Vec3f& );
   // void update...
   
   
   // Compute connectivity between vertices, halfs edges and faces.
   PLASMA_DLL_API void computeConnectivity();
   
   // Compute bounding box, normals and tangents.
   PLASMA_DLL_API void computeDerivedData();
   
   // Compute weights for skinning.
   PLASMA_DLL_API void computeWeights( Skeleton* );

   // Translates the surface.
   PLASMA_DLL_API void translate( const Vec3f& v );
   
   // Bounding interval hierarchy.
   PLASMA_DLL_API void computeBIH();
   PLASMA_DLL_API void clearBIH();
   inline const BIH& bih() const;
   
   // Subdivision with an certain precision.
   PLASMA_DLL_API void subdivide( float error, uint maxLevel );

protected: 

   /*----- methods -----*/

   PLASMA_DLL_API virtual ~SurfaceGeometry();
   PLASMA_DLL_API virtual void computeRenderableGeometry();

private: 

   /*----- friends -----*/

   friend class Simplifier;

   /*----- methods -----*/

   void computeBBox();
   void computeNormals();
   void computeTangents();
   void computeDefaultMapping();
   void computeCreases();

   void addEdge( uint v0, uint v1, uint edge );

   void splitFace( uint pID, uint fID );

   float computeError( uint pID, uint fID );
   bool computeFaceCenter( uint edge, uint level, Vec3f& c );
   bool computeCorner( uint v, uint level, Vec3f& c );

   inline uint starNext( uint edge );
   inline void neighbors( uint e0, uint e1 );

   inline void vertex( uint id, const Vec3f& );
   inline void starFirst( uint id, uint edge );

   /*----- data members -----*/

   LevelContainer    _verticesLevel;
   EdgeContainer     _verticesStars;
   VecContainer      _vertices;
   WeightsContainer  _weights;
   UVContainer       _mapping;
   VecContainer      _normals;
   TanContainer      _tangents;
   PatchContainer    _patches;

   BIH               _bih;
};

/*==============================================================================
   STRUCT SurfaceGeometry::Weights
==============================================================================*/

struct SurfaceGeometry::Weights
{
   /*----- methods -----*/
   
   Weights( uchar qty, const Vec4i& bones, const Vec4f& weights ) :
      _qty( qty ), _bones( bones ), _weights( weights )
   {}

   /*----- data members -----*/

   uchar  _qty;
   Vec4uc _bones;
   Vec4f  _weights;
};

/*==============================================================================
   STRUCT SurfaceGeometry::Face
==============================================================================*/

struct SurfaceGeometry::Face
{
   /*----- static methods -----*/
   
   static inline uint id( uint edge ) { return (edge&0xffffff)%3; }
   
   /*----- methods -----*/
   
   Face( uint v0, uint v1, uint v2 ) :
      _level( 0 ), _maxLevel( 255 )
   {
      _vID[0]     = v0;
      _vID[1]     = v1;
      _vID[2]     = v2;
      _mID[0]     = NULL_ID;
      _mID[1]     = NULL_ID;
      _mID[2]     = NULL_ID;
      _dID[0]     = NULL_ID;
      _dID[1]     = NULL_ID;
      _dID[2]     = NULL_ID;
      _creases[0] = 0;
      _creases[1] = 0;
      _creases[2] = 0;
   }
   
   Face( uint v0, uint v1, uint v2, uint m0, uint m1, uint m2 ) :
      _level( 0 ), _maxLevel( 255 )
   {
      _vID[0]     = v0;
      _vID[1]     = v1;
      _vID[2]     = v2;
      _mID[0]     = m0;
      _mID[1]     = m1;
      _mID[2]     = m2;
      _dID[0]     = NULL_ID;
      _dID[1]     = NULL_ID;
      _dID[2]     = NULL_ID;
      _creases[0] = 0;
      _creases[1] = 0;
      _creases[2] = 0;
   }
   
   Face( 
      uint v0, uint v1, uint v2, 
      uint m0, uint m1, uint m2,
      uchar c0, uchar c1, uchar c2
   ) :
      _level( 0 ), _maxLevel( 255 )
   {
      _vID[0]     = v0;
      _vID[1]     = v1;
      _vID[2]     = v2;
      _mID[0]     = m0;
      _mID[1]     = m1;
      _mID[2]     = m2;
      _dID[0]     = NULL_ID;
      _dID[1]     = NULL_ID;
      _dID[2]     = NULL_ID;
      _creases[0] = c0;
      _creases[1] = c1;
      _creases[2] = c2;
   }
   
   /*----- data members -----*/
   
   uint  _vID[3];      // Vertices ID.
   uint  _mID[3];      // Mapping  ID.
   uint  _dID[3];      // Derived data ID.
   uchar _creases[3];  // [0,255] <-> [smooth,hard]
   uchar _level;
   uchar _maxLevel;
};

/*==============================================================================
   CLASS SurfaceGeometry::Patch
==============================================================================*/

class SurfaceGeometry::Patch
   : public RCObject
{

public: 

   /*----- methods -----*/

   PLASMA_DLL_API Patch();

   inline void reserveFaces( uint qty );

   // Add a triangle face.
   inline uint addFace( uint v0, uint v1, uint v2 );
   inline uint addFace( uint v0, uint v1, uint v2, uint m0, uint m1, uint m2 );
   inline uint addFace(
      uint v0, uint v1, uint v2,
      uint m0, uint m1, uint m2,
      uchar c0, uchar c1, uchar c2
   );

   // Add a quad face. The interior edge is always the first one.
   // the returned ID is the ID of the first triangle face created.
   inline uint addFace( uint v0, uint v1, uint v2, uint v3 );
   inline uint addFace(
      uint v0, uint v1, uint v2, uint v3,
      uint m0, uint m1, uint m2, uint m3
   );
   inline uint addFace(
      uint v0, uint v1, uint v2, uint v3,
      uint m0, uint m1, uint m2, uint m3,
      uchar c0, uchar c1, uchar c2, uchar c3
   );

   // Accessors.
   inline const FaceContainer& faces() const;
   inline uint materialID() const;
   inline void materialID( uint id );
   
   // Return the number of triangle faces contained in the patch.
   inline uint numFaces() const;
   
protected: 

   /*----- methods -----*/

   virtual ~Patch();

private:    

   /*----- friends -----*/

   friend class SurfaceGeometry;
   friend class Simplifier;

   /*----- data members -----*/

   EdgeContainer _edgesStars;
   EdgeContainer _edgesNeighbors;
   FaceContainer _faces;
   uint          _materialID;
};

//------------------------------------------------------------------------------
//!
inline void
SurfaceGeometry::Patch::reserveFaces( uint qty )
{
   _faces.reserve( qty );
}

//------------------------------------------------------------------------------
//!
inline uint
SurfaceGeometry::Patch::addFace( uint v0, uint v1, uint v2 )
{
   CHECK( v0 != v1 );
   CHECK( v1 != v2 );
   CHECK( v2 != v0 );
   _faces.pushBack( SurfaceGeometry::Face( v0, v1, v2 ) );
   return (uint)_faces.size() - 1;
}

//------------------------------------------------------------------------------
//!
inline uint
SurfaceGeometry::Patch::addFace( uint v0, uint v1, uint v2, uint m0, uint m1, uint m2 )
{
   CHECK( v0 != v1 );
   CHECK( v1 != v2 );
   CHECK( v2 != v0 );
   _faces.pushBack( SurfaceGeometry::Face( v0, v1, v2, m0, m1, m2 ) );
   return (uint)_faces.size() - 1;
}

//------------------------------------------------------------------------------
//!
inline uint
SurfaceGeometry::Patch::addFace( 
   uint v0, uint v1, uint v2,
   uint m0, uint m1, uint m2,
   uchar c0, uchar c1, uchar c2
)
{
   CHECK( v0 != v1 );
   CHECK( v1 != v2 );
   CHECK( v2 != v0 );
   _faces.pushBack( SurfaceGeometry::Face( v0, v1, v2, m0, m1, m2, c0, c1, c2 ) );
   return (uint)_faces.size() - 1;
}

//------------------------------------------------------------------------------
//!
inline uint
SurfaceGeometry::Patch::addFace( uint v0, uint v1, uint v2, uint v3 )
{
   CHECK( v0 != v1 );
   CHECK( v1 != v2 );
   CHECK( v2 != v3 );
   CHECK( v3 != v0 );
   _faces.pushBack( SurfaceGeometry::Face( v2, v0, v1 ) );
   _faces.pushBack( SurfaceGeometry::Face( v0, v2, v3 ) );
   return (uint)_faces.size() - 2;
}

//------------------------------------------------------------------------------
//!
inline uint
SurfaceGeometry::Patch::addFace( 
   uint v0, uint v1, uint v2, uint v3,
   uint m0, uint m1, uint m2, uint m3
)
{  
   CHECK( v0 != v1 );
   CHECK( v1 != v2 );
   CHECK( v2 != v3 );
   CHECK( v3 != v0 );
   _faces.pushBack( SurfaceGeometry::Face( v2, v0, v1, m2, m0, m1 ) );
   _faces.pushBack( SurfaceGeometry::Face( v0, v2, v3, m0, m2, m3 ) );
   return (uint)_faces.size() - 2;
}

//------------------------------------------------------------------------------
//!
inline uint
SurfaceGeometry::Patch::addFace( 
   uint v0, uint v1, uint v2, uint v3,
   uint m0, uint m1, uint m2, uint m3,
   uchar c0, uchar c1, uchar c2, uchar c3
)
{
   CHECK( v0 != v1 );
   CHECK( v1 != v2 );
   CHECK( v2 != v3 );
   CHECK( v3 != v0 );
   _faces.pushBack( SurfaceGeometry::Face( v2, v0, v1, m2, m0, m1, 0, c0, c1 ) );
   _faces.pushBack( SurfaceGeometry::Face( v0, v2, v3, m0, m2, m3, 0, c2, c3 ) );
   return (uint)_faces.size() - 2;
}

//------------------------------------------------------------------------------
//!
inline const SurfaceGeometry::FaceContainer& 
SurfaceGeometry::Patch::faces() const
{
   return _faces;
}

//------------------------------------------------------------------------------
//!
inline uint
SurfaceGeometry::Patch::materialID() const
{
   return _materialID;
}

//------------------------------------------------------------------------------
//!
inline void
SurfaceGeometry::Patch::materialID( uint id )
{
   _materialID = id;
}

//------------------------------------------------------------------------------
//!
inline uint
SurfaceGeometry::Patch::numFaces() const
{
   return (uint)_faces.size();
}

//------------------------------------------------------------------------------
//!
inline void 
SurfaceGeometry::reserveVertices( uint qty )
{
   _vertices.reserve( qty );
}

//------------------------------------------------------------------------------
//!
inline void
SurfaceGeometry::reserveWeights( uint qty )
{
   _weights.reserve( qty );
}

//------------------------------------------------------------------------------
//!
inline void
SurfaceGeometry::reserveMapping( uint qty )
{
   _mapping.reserve( qty );
}

//------------------------------------------------------------------------------
//!
inline uint
SurfaceGeometry::addVertex( const Vec3f& v )
{
   _vertices.pushBack( v );
   return (uint)_vertices.size() - 1;
}
   
//------------------------------------------------------------------------------
//!
inline uint 
SurfaceGeometry::addWeights( uchar qty, const Vec4i& bones, const Vec4f& weights )
{
   _weights.pushBack( Weights( qty, bones, weights ) );
   return (uint)_weights.size() - 1;
}

//------------------------------------------------------------------------------
//!
inline uint 
SurfaceGeometry::addMapping( const Vec2f& coord )
{
   _mapping.pushBack( coord );
   return (uint)_mapping.size() - 1;
}

//------------------------------------------------------------------------------
//!
inline RCP<SurfaceGeometry::Patch> 
SurfaceGeometry::addPatch()
{
   RCP<Patch> patch( new Patch() );
   _patches.pushBack( patch );
   return patch;
}

//------------------------------------------------------------------------------
//!
inline void 
SurfaceGeometry::rescaleNormal( uint id, float s )
{
   _normals[id] *= s;
}

//------------------------------------------------------------------------------
//!
inline const SurfaceGeometry::PatchContainer& 
SurfaceGeometry::patches() const
{
   return _patches;
}

//------------------------------------------------------------------------------
//!
inline const Vec3f& 
SurfaceGeometry::vertex( uint id ) const
{
   return _vertices[id];
}

//------------------------------------------------------------------------------
//!
inline const Vec2f& 
SurfaceGeometry::mapping( uint id ) const
{
   return _mapping[id];
}

//------------------------------------------------------------------------------
//!
inline const Vec3f& 
SurfaceGeometry::normal( uint id ) const
{
   return _normals[id];
}

//------------------------------------------------------------------------------
//!
inline const Vec4f& 
SurfaceGeometry::tangent( uint id ) const
{
   return _tangents[id];
}

//------------------------------------------------------------------------------
//!
inline const SurfaceGeometry::Weights& 
SurfaceGeometry::weight( uint id ) const
{
   return _weights[id];
}

//------------------------------------------------------------------------------
//!
inline Vec3f 
SurfaceGeometry::wvertex( uint id, const Vector< Mat4f >& transform ) const
{
   Vec3f wv( 0, 0, 0 );
   const Weights& wt = _weights[id];
   const Vec3f& v    = _vertices[id];
   for( int i = 0; i < wt._qty; ++i )
   {
      wv += wt._weights(i) * transform[wt._bones(i)] * v;
   }
   return wv;
}

//------------------------------------------------------------------------------
//!
inline uint 
SurfaceGeometry::numVertices() const
{
   return (uint)_vertices.size();
}

//------------------------------------------------------------------------------
//!
inline uint 
SurfaceGeometry::numMappings() const
{
   return (uint)_mapping.size();
}

//------------------------------------------------------------------------------
//!
inline uint 
SurfaceGeometry::numNormals() const
{
   return (uint)_normals.size();
}

//------------------------------------------------------------------------------
//!
inline uint 
SurfaceGeometry::numTangents() const
{
   return (uint)_tangents.size();
}

//------------------------------------------------------------------------------
//!
inline uint 
SurfaceGeometry::numWeights() const
{
   return (uint)_weights.size();
}

//------------------------------------------------------------------------------
//!
inline uint
SurfaceGeometry::numPatches() const
{
   return (uint)_patches.size();
}

//------------------------------------------------------------------------------
//!
inline uint 
SurfaceGeometry::starFirst( uint vID )
{
   return _verticesStars[vID];
}

//------------------------------------------------------------------------------
//!
inline uint 
SurfaceGeometry::starNextCCW( uint edge )
{
   return neighborEdge( prevEdge(edge) );
}

//------------------------------------------------------------------------------
//!
inline uint 
SurfaceGeometry::starNext( uint edge )
{
   return _patches[edge>>24]->_edgesStars[edge&0xffffff];
}

//------------------------------------------------------------------------------
//!
inline uint 
SurfaceGeometry::prevEdge( uint edge )
{
   return ( (edge&0xffffff) % 3 ) == 0 ? edge + 2 : edge - 1;
}

//------------------------------------------------------------------------------
//!
inline uint 
SurfaceGeometry::nextEdge( uint edge )
{
   return ( (edge&0xffffff) % 3 ) == 2 ? edge - 2 : edge + 1;
}

//------------------------------------------------------------------------------
//!
inline uint 
SurfaceGeometry::neighborEdge( uint edge )
{
   return _patches[edge>>24]->_edgesNeighbors[edge&0xffffff];
}

//------------------------------------------------------------------------------
//!
inline uint 
SurfaceGeometry::orgVertexID( uint edge )
{
   return faceFromEdge( edge )._vID[(edge&0xffffff)%3];
}

//------------------------------------------------------------------------------
//!
inline uint 
SurfaceGeometry::dstVertexID( uint edge )
{
   return faceFromEdge( edge )._vID[((edge&0xffffff)+1)%3];
}

//------------------------------------------------------------------------------
//!
inline uchar 
SurfaceGeometry::crease( uint edge )
{
   return faceFromEdge( edge )._creases[(edge&0xffffff)%3];
}

//------------------------------------------------------------------------------
//!
inline SurfaceGeometry::Face&
SurfaceGeometry::faceFromEdge( uint edge )
{
   return _patches[edge>>24]->_faces[(edge&0xffffff)/3];
}

//------------------------------------------------------------------------------
//!
inline uint 
SurfaceGeometry::edge( uint p, uint f, uint e )
{
   return (p << 24) + f*3 + e;
}

//------------------------------------------------------------------------------
//!
inline void 
SurfaceGeometry::neighbors( uint e0, uint e1 )
{
   // FIXME: Does not work for e0 or e1 null!!!
   _patches[e0>>24]->_edgesNeighbors[e0&0xffffff] = e1;
   _patches[e1>>24]->_edgesNeighbors[e1&0xffffff] = e0;
}

//------------------------------------------------------------------------------
//!
inline const BIH&
SurfaceGeometry::bih() const
{
   return _bih;
}

//------------------------------------------------------------------------------
//!
inline void
SurfaceGeometry::vertex( uint id, const Vec3f& p )
{
   _vertices[id] = p;
}

//------------------------------------------------------------------------------
//!
inline void 
SurfaceGeometry::starFirst( uint id, uint edge )
{
   _verticesStars[id] = edge;
}

NAMESPACE_END

#endif
