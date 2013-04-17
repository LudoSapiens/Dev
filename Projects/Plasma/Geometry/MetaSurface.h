/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_METASURFACE_H
#define PLASMA_METASURFACE_H

#include <Plasma/StdDefs.h>
#include <Plasma/Geometry/MaterialSet.h>
#include <Plasma/Intersector.h>

#include <CGMath/Vec3.h>
#include <CGMath/AABBTree.h>
#include <CGMath/HGrid.h>

#include <Base/ADT/MemoryPool.h>
#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>

NAMESPACE_BEGIN

class MetaFunction;

/*==============================================================================
   CLASS MetaSurface
==============================================================================*/

class MetaSurface:
   public RCObject
{
public:

   /*----- static methods -----*/
   static PLASMA_DLL_API uint  debugLevel();
   static PLASMA_DLL_API void  debugLevel( uint v );

   /*----- structures -----*/

   struct Vertex;
   struct Subpatch;
   struct Patch;

   struct Vertex {
      Vec3f     _pos;
      Vec2f     _uv;
      Vertex*   _next[4];
      uint      _id;
      uint      _flags;    // 0..3: Edge i.
   };
   struct Subpatch {
      Vertex*   _corners[4];
      Patch*    _patch;
      Subpatch* _next;
      uint      _flags;    // 0: Trimming 1: Removed 2..5: linear edge.
   };
   struct Patch {
      Vec3f*              _controlPts[4];
      Patch*              _neighbors[4];
      Patch*              _next;
      void*               _data;
      ushort              _flags;    // 0: Trimming 1: Removed 2: Inversed
      ushort              _edges;    // 0..7: neighbor edges 8..11: edge creases.
      uint                _detailsID;
      uint                _id;
      Subpatch            _subpatch;
      Vertex              _corners[4];
      Vec2f               _uv[4];
      AABBTree*           _tree;
      const MetaFunction* _mapping;
      const MetaFunction* _displacement;
   };
   struct LoopLink
   {
      Vertex*   _vertex;
      LoopLink* _next;
   };
   struct Loop
   {
      uint      _flags;    // 0: Removed
      LoopLink* _link;
      Loop*     _next;
   };
   struct Edge
   {
      Vertex*   _v0;
      Vertex*   _v1;
      Edge*     _next;
   };
   struct Trimming
   {
      uchar     _x;        // X projection axis.
      uchar     _y;        // Y projection axis.
      uchar     _z;        // Z projection axis.
      uchar     _flags;    // bit 0: triangulation type.
      Planef    _plane0;   // first triangle plane.
      Planef    _plane1;   // second triangle plane.
      Edge*     _edges;    // Edges constraints.
      Vertex*   _vertices; // Interior vertices.
      Loop*     _loops;    // Simple polygons loops.
   };

   /*----- classes -----*/

   class PatchIter {
      public:
         PatchIter(): _ptr(0)           {}
         PatchIter( Patch* p ): _ptr(p) {}

         operator Patch*()              { return _ptr; }
         Patch& operator*() const       { return *_ptr; }
         Patch* operator->() const      { return _ptr; }
         PatchIter& operator++()        { _ptr = _ptr->_next; return *this; }

         bool isValid() const           { return _ptr != 0; }

      protected:
         Patch* _ptr;
   };

  class SubpatchIter {
      public:
         SubpatchIter(): _ptr(0)              {}
         SubpatchIter( Subpatch* p ): _ptr(p) {}

         operator Subpatch*()                 { return _ptr; }
         Subpatch& operator*() const          { return *_ptr; }
         Subpatch* operator->() const         { return _ptr; }
         SubpatchIter& operator++()           { _ptr = _ptr->_next; return *this; }

         bool isValid() const                 { return _ptr != 0; }

      protected:
         Subpatch* _ptr;
   }; 

   /*----- static methods -----*/
   
   static bool trace( const Subpatch& sp, const Rayf& ray, Intersector::Hit& hit );
   static void computeApproximation( 
      const Subpatch& sp, 
      Vec3f& v0,
      Vec3f& v1,
      Vec3f& v2,
      Vec3f& v3,
      Vec3f& n0,
      Vec3f& n1
   );

   inline static Patch* neighborPatch( Patch&, uint edge );
   inline static uint neighborEdge( Patch&, uint edge );
   inline static int crease( Patch&, uint edge );
   inline static AABBoxf computeAABB( const Subpatch& );

   inline static void flip( Patch&, int );
   inline static void hide( Patch&, int );
   inline static void hide( Subpatch&, int );
   inline static void hide( Loop&, int );
   inline static bool isFlipped( const Patch& );
   inline static bool isTrimmed( const Patch& );
   inline static bool isTrimmed( const Subpatch& );
   inline static bool isHidden( const Patch& );
   inline static bool isHidden( const Subpatch& );
   inline static bool isHidden( const Loop& );

   /*----- methods -----*/

   MetaSurface();

   // Queries.
   uint numPatches() const    { return _numPatches; }
   uint numSubpatches() const { return _numSubpatches; }

   // Iterators.
   PatchIter patches() const  { return PatchIter( _begin ); }
   SubpatchIter subpatches( Patch& p ) const { return SubpatchIter( &p._subpatch ); }

   // Creation API.
   Patch* createPatch();
   Vec3f* createPoint();

   void neighbors( Patch* p0, uint e0, Patch* p1, uint e1, int crease );

   // Subdivision.
   void subdivide( float gerror, float merror );

   // Trimming.
   void trim( Patch&, Patch& );
   void trim( Subpatch&, Subpatch& );
   Trimming* trimming( Subpatch& );
   Loop* loops( Subpatch& );

   // Queries.
   Vec3f computePosition( Patch&, const Vec2f& uv ) const;
   void computeParameters( Patch&, const Vec2f& uv, Vec3f& pos, Vec3f& normal ) const;
   void computePoint( const Subpatch&, const Trimming&, const Loop&, Vec3f& pos ) const;
   void computePoint( const Subpatch&, Vec3f& pos, Vec3f& dir );

   // Geometry.
   void triangulate( 
      Patch&, 
      Vector<float>& vertices, 
      Vector<uint>&  indices,
      Vector<uint>*  faceInfos,
      bool normals = false
   ) const;

   // Collision.
   const HGrid& hgrid() const { return *_grid; }

   // Intersection.
   bool trace( const Rayf& );
   bool trace( const Rayf&, Intersector::Hit& );

   // Debug
   void dump( const Subpatch&, const Trimming& ) const;
   static void dump( const Subpatch& );

protected:

   /*----- friends -----*/

   friend class TrimmingData;

   /*----- classes -----*/

   class CacheData;

   /*----- methods -----*/

   ~MetaSurface();

   void subdivide( Patch&, float gerror );
   bool subdivide( Subpatch&, float gerror );
   bool subdivideDisplaced( Subpatch&, float gerror );
   void subdivide( Subpatch& );

   void resetVertices( Patch& p ) const;
   void computeDetails( float );
   void makeBilinear();
   void subdivideu( Subpatch&, const Vec3f pos[] );
   void subdividev( Subpatch&, const Vec3f pos[] );
   void subdivideuv( Subpatch&, const Vec3f pos[] );
   Vertex* subdivideEdge( Subpatch&, uint edge, float t, const Vec3f* pos );
   Vertex* insertVertex( Subpatch&, uint edge, float t, Vertex* v, int dir, const Vec3f& pos, const Vec2f& uv );
   Vertex* insertVertex( Subpatch&, uint edge, float t );
   Vertex* insertVertex( Subpatch&, Trimming*, const Vec3f& pos );

   Vertex* insertVertex( Subpatch& sp, uint edge, const Vec3f& pos, float t );
   Vertex* insertVertex( Subpatch& sp, Trimming* trim, const Vec3f& pos, const Vec2f& uv );


   void trim( Subpatch&, Subpatch&, Trimming&, Trimming&, Planef&, Planef&, float*, float*, int*, int* );

   void buildSpatialSubdivision();

   /*----- data members -----*/

   HGrid*               _grid;
   RCP<AABBTree::Pool>  _aabbPool;

   RCP<CacheData>       _cache;
   uint                 _numPatches;
   uint                 _numSubpatches;
   Patch*               _begin;
   MemoryPool<Vec3f>    _ptPool;
   MemoryPool<Vertex>   _vPool;
   MemoryPool<Subpatch> _spPool;
   MemoryPool<Patch>    _pPool;
};

//------------------------------------------------------------------------------
//! 
inline MetaSurface::Patch* 
MetaSurface::neighborPatch( Patch& p, uint edge )
{
   return p._neighbors[edge];
}

//------------------------------------------------------------------------------
//! 
inline uint 
MetaSurface::neighborEdge( Patch& p, uint edge )
{
   return getbits( p._edges, edge*2, 2 );
}

//------------------------------------------------------------------------------
//! 
inline int 
MetaSurface::crease( Patch& p, uint edge )
{
   return getbits( p._edges, 8+edge, 1 ); 
}

//------------------------------------------------------------------------------
//! 
inline AABBoxf 
MetaSurface::computeAABB( const Subpatch& sp )
{
   AABBoxf box = AABBoxf::empty();
   Vertex* v   = sp._corners[3];
   for( uint c = 0; c < 4; ++c )
   {
      for( ; v != sp._corners[c]; v = v->_next[c] ) box |= v->_pos;
   }
   box.grow( 1.0f/1024.0f );
   return box;
}

//------------------------------------------------------------------------------
//! 
inline void 
MetaSurface::computePoint( const Subpatch& sp, Vec3f& pos, Vec3f& dir )
{
   Vec3f a, b, c, d;
   Vec3f n0, n1;
   computeApproximation( sp, a, b, c, d, n0, n1 );
   pos = (a*0.25f+b*0.5f+c*0.25f);
   dir = n0;
}

//------------------------------------------------------------------------------
//! 
inline void 
MetaSurface::flip( Patch& p, int v )
{
   p._flags = (p._flags & ~4) | (v<<2);
}

//------------------------------------------------------------------------------
//! 
inline void 
MetaSurface::hide( Patch& p, int v )
{
   p._flags = (p._flags & ~2) | (v<<1);
}

//------------------------------------------------------------------------------
//! 
inline void 
MetaSurface::hide( Subpatch& sp, int v )
{
   sp._flags = (sp._flags & ~2) | (v<<1);
}

//------------------------------------------------------------------------------
//! 
inline void 
MetaSurface::hide( Loop& l, int v )
{
   l._flags = (l._flags & ~1) | v;
}

//------------------------------------------------------------------------------
//!
inline bool 
MetaSurface::isFlipped( const Patch& p )
{
   return (p._flags & 4) != 0;
}

//------------------------------------------------------------------------------
//! 
inline bool 
MetaSurface::isTrimmed( const Patch& p )
{
   return (p._flags & 1) != 0;
}

//------------------------------------------------------------------------------
//! 
inline bool 
MetaSurface::isTrimmed( const Subpatch& sp )
{
   return (sp._flags & 1) != 0;
}

//------------------------------------------------------------------------------
//! 
inline bool 
MetaSurface::isHidden( const Patch& p )
{
   return (p._flags & 2) != 0;
}

//------------------------------------------------------------------------------
//! 
inline bool 
MetaSurface::isHidden( const Subpatch& sp )
{
   return (sp._flags & 2) != 0;
}

//------------------------------------------------------------------------------
//! 
inline bool 
MetaSurface::isHidden( const Loop& l )
{
   return (l._flags & 1) != 0;
}

NAMESPACE_END

#endif
