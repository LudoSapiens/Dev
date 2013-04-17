/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_METAGEOMETRY_H
#define PLASMA_METAGEOMETRY_H

#include <Plasma/StdDefs.h>
#include <Plasma/Geometry/Geometry.h>
#include <Plasma/Geometry/MetaNode.h>
#include <Plasma/Geometry/MetaSurface.h>

#include <CGMath/HGrid.h>
#include <CGMath/Variant.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS MetaFunction
==============================================================================*/

class MetaFunction:
   public RCObject
{
   public:

   /*----- methods -----*/

   MetaFunction() 
   {
      _params = new Table();
   }

   inline Table& parameters()             { return *_params; }
   inline const Table& parameters() const { return *_params; }

   inline VMByteCode& code()              { return _code; }
   inline const VMByteCode& code() const  { return _code; }

   protected:

   /*----- data members -----*/

   RCP<Table> _params;
   VMByteCode _code;
};

/*==============================================================================
   CLASS MetaBlock
==============================================================================*/

//! Block ordering.
//! Vertex order:    Edge order:    Face order:
//!    2-------3      +---1---+        3
//!   /|      /|     /|      /|        |
//!  / |     / |   11 4    10 7        | 4
//! 6--+----7  |   +--+2---+  |        |/
//! |  0----|--1   |  +--0-|--+      0-+-----1
//! | /     | /    5 8     6 9        /|
//! |/      |/     |/      |/        / 2
//! 4-------5      +---3---+        5
//!
//! Block subdivision.
//! 1x1: 0x0  1x2: 0x4  1x3: 0x8  1x4: 0xc
//! 2x1: 0x1  2x2: 0x5  2x3: 0x9  2x4: 0xd
//! 3x1: 0x2  3x2: 0x6  3x3: 0xa  3x4: 0xe
//! 4x1: 0x3  4x2: 0x7  4x3: 0xb  4x4: 0xf
//!
class MetaBlock
{
public:

   /*----- Structures -----*/

   struct Face;

   struct Vertex
   {
      Vec3f   _position;
      float   _t; // Encode manhattan distance on cube.
      Vertex* _jointVertex;
   };

   struct HEdge
   {
      /*----- methods -----*/

      float t() const                    { return _vertex->_t; }
      const Vec3f& position() const      { return _vertex->_position; }
      const Vec3f& jointPosition() const { return _vertex->_jointVertex->_position; }

      /*----- data members -----*/

      Vertex*             _vertex;
      HEdge*              _next;
      HEdge*              _neighbor;
      HEdge*              _link;
      Face*               _face;
      MetaSurface::Patch* _patch;
      uint                _flags;
   };

   struct Face
   {
      /*----- methods -----*/

      // Computed attributes.
      inline Vec3f center() const
      {
         return _corners[0].position()*0.25f +
                _corners[1].position()*0.25f +
                _corners[2].position()*0.25f +
                _corners[3].position()*0.25f;
      }

      inline float force() const
      {
         Vec3f tanu = (_corners[1].position()-_corners[0].position())*0.5f + 
                      (_corners[2].position()-_corners[3].position())*0.5f;
         Vec3f tanv = (_corners[3].position()-_corners[0].position())*0.5f + 
                      (_corners[2].position()-_corners[1].position())*0.5f;
         return CGM::min( sqrLength( tanu ), sqrLength( tanv ) );
      }

      /*----- data members -----*/

      HEdge      _corners[4];
      MetaBlock* _block;
      Face*      _link;
      uchar      _linkCorner;
      Face*      _next; // Next face on this side.
   };

   /*----- methods -----*/

   MetaBlock() {}
   ~MetaBlock() {}

   // Attributes getters.
   inline const Vec3f& localPosition( uint i ) const { return _localPos[i]; }
   inline Vec3f& position( uint i )                  { return _corners[i]._position; }
   inline const Vec3f& position( uint i ) const      { return _corners[i]._position; }
   inline Vertex& vertex( uint i )                   { return _corners[i]; }
   inline ushort creases() const                     { return _creases; }
   inline int crease( uint i ) const                 { return (_creases>>i)&1; }
   inline ushort groupID() const                     { return _groupID; }
   inline uint id() const                            { return _id; }
   inline int subdivisions() const                   { return _subdivisions; }
   inline int subdivision( uint i ) const            { return (_subdivisions >> (i*4)) & 0xf; }
   inline Face& side( uint i )                       { return _sides[i]; }
   inline MetaBlocks* group() const                  { return _group; }

   inline Vec3f center() const
   {
      Vec3f c(0.0f);
      for( uint i = 0; i < 8; ++i )
      {
         c += position(i);
      }
      return c*0.125f;
   }

protected:

   /*----- friends -----*/

   friend class MetaBuilder;
   friend class MetaCompositeBlocks;

   /*----- data members -----*/

   MetaBlocks* _group;
   Vec3f       _localPos[8];
   Vertex      _corners[8];
   Face        _sides[6];
   uint        _subdivisions;
   ushort      _creases;
   ushort      _groupID;
   uint        _id;
};

/*==============================================================================
   CLASS MetaGeometry
==============================================================================*/

class MetaGeometry:
   public Geometry
{
public:

   /*----- methods -----*/

   MetaGeometry();

   // Intersection.
   inline bool trace( const Rayf& );
   inline bool trace( const Rayf&, Intersector::Hit& );

   PLASMA_DLL_API void triangulate( 
      Vector<float>& vertices, 
      Vector<uint>&  indices,
      Vector<uint>*  faceInfos = 0,
      bool           normals = false
   ) const;
   PLASMA_DLL_API void createMesh( MeshGeometry&, Vector<uint>* faceInfos = 0 ) const;

protected:

   /*----- methods -----*/

   virtual ~MetaGeometry();

   // Update structures.
   void update();
   void updateCSGTree();
   void updateTransforms( MetaNode*, const Mat4f& );
   void updateBoundingBoxes( MetaNode* );
   void updateLinks();
   void updateControlMesh();
   void updateTopology( MetaBlock* );
   void updateCSG();

   virtual void computeRenderableGeometry();

   // Basic topology methods.
   void split( MetaBlock::HEdge*, float frac );
   void split( 
      MetaBlock::HEdge*  startEdge, 
      MetaBlock::HEdge*  endEdge, 
      const float*       fracs,
      MetaBlock::HEdge** edges
   );
   void split( MetaBlock::Face*, int x, int y );

   void connect( MetaBlock::Face* );

   // CSG.
   int classifyCSG( const Vec3f& pos, const Vec3f& dir, MetaBlock* block, int& flipped );
   int evaluateCSG( MetaBlock* block, const Vec3f& pos, int& flipped );
   int evaluateCSG( MetaNode* node, const Vec3f& pos, MetaBlocks*& boundaryNode );

private:

   /*----- friends -----*/

   friend class MetaBuilder;

   /*----- data members -----*/

   bool                          _geomNeedUpdate;
   uint                          _count;
   float                         _gerror;
   float                         _derror;

   // Groups and operations.
   //Vector< MetaNode* >           _roots;
   Vector< RCP<MetaNode> >       _nodes;
   Vector< MetaBlocks* >         _groups;
   Vector< MetaComposite* >      _composites;
   Vector< RCP<MetaFunction> >   _functions;

   // Blocks geometry.
   MemoryPool<MetaBlock>         _bPool;
   MemoryPool<MetaBlock::HEdge>  _hePool;
   MemoryPool<MetaBlock::Face>   _fPool;
   MemoryPool<MetaBlock::Vertex> _vPool;

   // Surfaces/patches Level.
   RCP< MetaSurface >            _surface;

   // Spatial structure.
   HGrid*                        _grid;
};

//------------------------------------------------------------------------------
//! 
inline bool 
MetaGeometry::trace( const Rayf& ray )
{
   return _surface->trace( ray );
}

//------------------------------------------------------------------------------
//! 
inline bool 
MetaGeometry::trace( const Rayf& ray, Intersector::Hit& hit )
{
   return _surface->trace( ray, hit );
}

/*==============================================================================
   CLASS MetaBuilder
==============================================================================*/

class MetaBuilder:
   public RCObject
{
public:

   /*----- methods -----*/

   MetaBuilder( MetaGeometry* );
   virtual ~MetaBuilder();

   // Creation API.
   MetaBlock*           createBlock();
   MetaBlocks*          createBlocks();
   MetaCompositeBlocks* createCompositeBlocks();
   MetaDifference*      createDifference();
   MetaIntersection*    createIntersection();
   MetaTransform*       createTransform();
   MetaUnion*           createUnion();
   MetaComposite*       createComposite();
   MetaInput*           createInput();
   MetaFunction*        createFunction();

   // Modification API.
   void add( MetaBlocks*, MetaBlock* );
   void add( MetaBlocks*, uint, uint );
   void add( MetaOperation*, MetaNode* );
   void add( MetaCompositeBlocks*, MetaBlocks* );
   void set( MetaTransform*, MetaNode* );
   void set( MetaNode*, int layer );
   void set( MetaBlocks*, const Mat4f& );
   void set( MetaBlocks*, MetaFunction* mapping, MetaFunction* displacement );
   void set( MetaTransform*, const Mat4f& );
   void set( MetaComposite*, const Mat4f& );
   void set( MetaComposite*, MetaInput* );
   void set( MetaBlock*, uint id, ushort grp, ushort creases, uint sub );
   void set( MetaBlock*, const Vec3f[8] );

   // Quality metric.
   void setGeometricError( float );
   void setDetailsError( float );

   // Update the MetaGeometry.
   void execute();

protected:

   /*----- data members -----*/

   MetaGeometry* _geom;
};

NAMESPACE_END

#endif
