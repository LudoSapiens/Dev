/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_DFBLOCKS_H
#define PLASMA_DFBLOCKS_H

#include <Plasma/StdDefs.h>
#include <Plasma/DataFlow/DFGeometry.h>

#include <CGMath/HGrid.h>

#include <Base/ADT/MemoryPool.h>

NAMESPACE_BEGIN

class DFBlocks;

/*==============================================================================
   CLASS DFBlock
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
class DFBlock
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

      Vertex*   _vertex;
      HEdge*    _next;
      HEdge*    _neighbor;
      HEdge*    _link;
      Face*     _face;
      uint      _patch;
      uint      _flags;
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
      DFBlock*   _block;
      Face*      _link;
      uchar      _linkCorner;
      Face*      _next; // Next face on this side.
   };

   /*----- methods -----*/

   DFBlock() {}
   ~DFBlock() {}

   // Attributes getters.
   inline Vec3f& position( uint i )                  { return _corners[i]._position; }
   inline const Vec3f& position( uint i ) const      { return _corners[i]._position; }
   inline Vertex& vertex( uint i )                   { return _corners[i]; }
   inline ushort creases() const                     { return _creases; }
   inline int crease( uint i ) const                 { return (_creases>>i)&1; }
   inline ushort groupID() const                     { return _groupID; }
   inline uint id() const                            { return _id; }
   inline int subdivisions() const                   { return _subdivisions; }
   inline int subdivision( uint i ) const            { return (_subdivisions >> (i*4)) & 0xf; }
   inline uint attractions() const                   { return _attractions; }
   inline uint attraction( uint i ) const            { return (_attractions >> (i*2)) & 0x3; }
   inline Face& side( uint i )                       { return _sides[i]; }

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

   friend class DFBlocks;

   /*----- data members -----*/

   Vertex      _corners[8];
   Face        _sides[6];
   uint        _subdivisions;
   ushort      _creases;
   ushort      _groupID;
   uint        _id;
   uint        _attractions; // 0: neutral 1: no attraction 2: attraction.
};


/*==============================================================================
   CLASS DFBlocks
==============================================================================*/

class DFBlocks:
   public RCObject
{
public:

   /*----- methods -----*/

   DFBlocks();

   // Creation.
   DFBlock* createBlock();
   void set( DFBlock*, uint id, ushort grp, ushort creases, uint sub, uint attrations );
   void set( DFBlock*, const Vec3f[8] );

   inline           uint  numBlocks() const       { return uint(_blocks.size()); }
   inline       DFBlock*  block( uint idx )       { return _blocks[idx]; }
   inline const DFBlock*  block( uint idx ) const { return _blocks[idx]; }

   // Surface.
   DFGeometry* geometry();

protected:

   /*----- methods -----*/

   virtual ~DFBlocks();

   // Update structures.
   void update();
   void updateBoundingBoxes();
   void updateLinks();
   void updateControlMesh();
   void updateTopology( DFBlock* );

   // Basic topology methods.
   void split( DFBlock::HEdge*, float frac );
   void split( 
      DFBlock::HEdge*  startEdge, 
      DFBlock::HEdge*  endEdge, 
      const float*     fracs,
      DFBlock::HEdge** edges
   );
   void split( DFBlock::Face*, int x, int y );

   void connect( DFBlock::Face* );

private:

   /*----- data members -----*/

   bool                        _needUpdate;
   uint                        _count;
   float                       _gerror;

   Vector< DFBlock* >          _blocks;

   // Blocks geometry.
   MemoryPool<DFBlock>         _bPool;
   MemoryPool<DFBlock::HEdge>  _hePool;
   MemoryPool<DFBlock::Face>   _fPool;
   MemoryPool<DFBlock::Vertex> _vPool;

   // Surfaces/patches Level.
   RCP< DFGeometry >           _surface;

   // Spatial structure.
   HGrid*                      _grid;
};

NAMESPACE_END

#endif
