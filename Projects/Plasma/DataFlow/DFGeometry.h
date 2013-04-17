/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_DFGEOMETRY_H
#define PLASMA_DFGEOMETRY_H

#include <Plasma/StdDefs.h>
#include <Plasma/Geometry/Geometry.h>

#include <CGMath/Grid.h>
#include <CGMath/AABBTree.h>
#include <CGMath/Plane.h>

#include <Base/ADT/Pair.h>

NAMESPACE_BEGIN

class DFPatch;
class HGrid;

/*==============================================================================
   CLASS DFGeometry
==============================================================================*/

class DFGeometry:
   public Geometry
{
public:

   enum {
      NULL_ID16 = 0xffff,
      NULL_ID32 = 0xffffffff
   };

   /*----- structures -----*/

   struct Vertex {
      Vec3f    _pos;
      Vec3f    _n;
      Vec2f    _uv;
      uint16_t _next[4];
      uint32_t _id;
      uint16_t _flags;
   };

   struct Subpatch {

      /*----- methods -----*/

      inline bool isHidden() const  { return (_flags & 2) != 0; }
      inline bool isTrimmed() const { return _trimming != NULL_ID32; }
      inline void hide( int v )     { _flags = (_flags & ~2) | (v<<1); }

      /*----- data members -----*/

      uint16_t _corners[4];
      uint32_t _flags;      // 0: Trimming 1: Removed 2..5: non-linear edge.
      uint32_t _trimming;
   };

   struct Patch {

      /*----- methods -----*/

      inline int crease( int edge ) const             { return getbits( _edges, 8+edge,1 ); }
      inline int neighborEdge( int edge ) const       { return getbits( _edges, edge*2, 2 ); }
      inline uint32_t neighborPatch( int edge ) const { return _neighbors[edge]; }
      inline bool isTrimmed() const                   { return (_flags & 1) != 0; }
      inline bool isHidden() const                    { return (_flags & 2) != 0; }
      inline bool isFlipped() const                   { return (_flags & 4) != 0; }

      inline void trim( int v )                       { _flags = (_flags & ~1) | (v); }
      inline void hide( int v )                       { _flags = (_flags & ~2) | (v<<1); }
      inline void flip( int v )                       { _flags = (_flags & ~4) | (v<<2); }

      /*----- data members -----*/

      uint16_t         _flags;         // 0: Trimming 1: Removed 2: Inversed
      uint16_t         _edges;         // 0..7: neighbor edges 8..11: edge creases.
      uint32_t         _id;
      uint32_t         _controlPts[4]; // Control pts ID.
      uint32_t         _neighbors[4];  // Neighbors patches ID.
      Vec2f            _uv[4];
      Vector<Subpatch> _subpatches;
      Vector<Vertex>   _vertices;
   };

   struct Trimming {

      /*----- methods -----*/

      inline bool isHidden( int lp ) const { return (_loops[lp] & 1) != 0; }
      inline bool isInvalid() const        { return (_flags & 2) != 0; }
      inline uint8_t type() const          { return (_flags & 1); }
      inline void hide( int lp, int v )    { _loops[lp] = (_loops[lp] & ~1) | (v); }
      inline void invalidate( int v )      { _flags = (_flags & ~2) | (v<<1); }

      /*----- data members -----*/

      uint8_t                         _x;        // X projection axis.
      uint8_t                         _y;        // Y projection axis.
      uint8_t                         _z;        // Z projection axis.
      uint8_t                         _flags;    // bit 0: triangulation type.
      uint16_t                        _vertices; // First vertex ID.
      Planef                          _plane0;   // first triangle plane.
      Planef                          _plane1;   // second triangle plane.
      Set< Pair<uint16_t, uint16_t> > _edges;
      Vector<uint16_t>                _loops;    // #loops, #vertex, flags, v0, .. vn, #vertex, flags, v0, ...
   };

   /*----- methods -----*/

   PLASMA_DLL_API DFGeometry();

   // Attributes.
   uint32_t numPatches() const                    { return uint32_t(_patches.size()); }
   uint32_t numControlPoints() const              { return uint32_t(_controlPts.size()); }
   uint numSubpatches() const                     { return _numSubpatches; }
   uint32_t numTrimmings() const                  { return uint32_t(_trimmings.size()); }

   // Creation.
   void reservePatches( uint32_t num )            { _patches.reserve( num ); }
   void reserveControlPoints( uint32_t num )      { _controlPts.reserve( num ); }
   void reserveTrimmings( uint32_t num )          { _trimmings.reserve( num ); }
   PLASMA_DLL_API uint32_t addPatch();
   PLASMA_DLL_API uint32_t addPatch( const Patch& );
   PLASMA_DLL_API uint32_t addPatch( uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3, int creases );
   PLASMA_DLL_API uint32_t addControlPoint( const Vec3f& p = Vec3f(0.0f) );
   PLASMA_DLL_API uint32_t addTrimming();
   PLASMA_DLL_API uint32_t addTrimming( const Trimming& );
   PLASMA_DLL_API void neighbors( uint32_t p0, uint e0, uint32_t p1, uint e1, int crease );
   PLASMA_DLL_API void computeNeighbors();


   Patch& patch( uint32_t id )                    { return _patches[id]; }
   const Patch& patch( uint32_t id ) const        { return _patches[id]; }
   Vec3f& controlPoint( uint32_t id )             { return _controlPts[id]; }
   const Vec3f& controlPoint( uint32_t id ) const { return _controlPts[id]; }
   Trimming& trimming( uint32_t id )              { return _trimmings[id]; }
   const Trimming& trimming( uint32_t id ) const  { return _trimmings[id]; }

   // Subdivision.
   PLASMA_DLL_API void subdivide( float error );

   // Trimming.
   PLASMA_DLL_API void trimPatches( uint32_t p0, uint32_t p1 );
   PLASMA_DLL_API void trimSubpatches( uint32_t p0, uint16_t sp0, uint32_t p1, uint16_t sp1 );

   // Operations.
   PLASMA_DLL_API RCP<DFGeometry> clone() const ;
   PLASMA_DLL_API RCP<DFGeometry> transform( const Reff& ) const;
   PLASMA_DLL_API void merge( DFGeometry* );
   PLASMA_DLL_API void subtract( DFGeometry* );
   PLASMA_DLL_API void intersect( DFGeometry* );

   // Update the geometry triangle mesh.
   PLASMA_DLL_API void updateMesh();
   PLASMA_DLL_API RCP<MeshGeometry> createMesh();

   // Queries.
   PLASMA_DLL_API void getSurfacePoint( const Patch&, const Subpatch&, Vec3f& pos, Vec3f& n );
   PLASMA_DLL_API void getLoopPoint(
      const Patch& p,
      uint16_t*    vertices,
      uint         numV,
      uint         x,
      uint         y,
      Vec3f&       pos,
      Vec3f&       n
   );
   PLASMA_DLL_API bool pointInLoop(
      const Patch& p,
      uint16_t*    vertices,
      uint         numV,
      uint         x,
      uint         y,
      const Vec3f& pt
   );

   PLASMA_DLL_API void triangulate(
      Patch&,
      Grid<Vertex*>& grid,
      Vector<float>& vertices,
      Vector<uint>&  indices,
      Vector<uint>*  faceInfos,
      bool normals = false
   ) const;

protected:

   /*----- friends -----*/

   friend class LoopBuilder;

   /*----- methods -----*/

   ~DFGeometry();

   void addEdgeNeighbor( uint32_t v0, uint32_t v1, uint32_t e, Vector<uint32_t>& vs, Vector<uint32_t>& es );

   void subdivide( Patch&, float error );
   bool subdivide( Patch&, uint spID, const DFPatch&, float error );
   void subdivideu( Patch& p, uint spID, const Vec3f pos[], const Vec3f n[], const Vec2f[] );
   void subdividev( Patch& p, uint spID, const Vec3f pos[], const Vec3f n[], const Vec2f[] );
   void subdivideuv( Patch& p, uint spID, const Vec3f pos[], const Vec3f n[], const Vec2f[] );
   uint16_t subdivideEdge( Patch& p, Subpatch& sp, uint edge, const Vec3f& pos, const Vec3f& n, const Vec2f& uv );

   uint16_t insertEdgeVertex(
      Patch&       p,
      uint16_t     pv,
      int          dir,
      const Vec3f& pos,
      const Vec3f& n,
      const Vec2f& uv
   );
   uint16_t insertEdgeVertex( Patch& p, Subpatch& sp, uint edge, const Vec3f& pos, float t );
   uint16_t insertFaceVertex( Patch& p, Trimming& trim, const Vec3f& pos, const Vec3f& n, const Vec2f& uv );
   void addEdge( Patch& p, Trimming& t, uint16_t v0, uint16_t v1 );

   // From Geometry.
   virtual void computeRenderableGeometry();

   uint getTrimming( Patch& p, uint16_t spID );
   void trim(
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
   );

   void mergeAndClips( DFGeometry*, int maskA, int maskB );
   void updateLoops( size_t begin, size_t end );
   void classify( size_t begin, size_t end, int mask, DFGeometry*, HGrid& );
   void cleanTrimming( Trimming& );

   /*----- data members -----*/

   // Patch geometry.
   uint                _numSubpatches;
   Vector< Patch >     _patches;
   Vector< Vec3f >     _controlPts;
   Vector< Trimming >  _trimmings;

   // Mesh geometry.
   Vector<float>       _vertices;
   Vector<uint>        _indices;

   // Collision structure.
   RCP<AABBTree::Pool> _aabbPool;
   Vector<AABBTree*>   _aabbs;
};

NAMESPACE_END

#endif
