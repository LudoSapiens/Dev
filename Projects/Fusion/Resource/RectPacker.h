/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef RECT_PACKER_H
#define RECT_PACKER_H

#include <Fusion/StdDefs.h>

#include <Fusion/Resource/Bitmap.h>

#include <CGMath/AARect.h>

#include <Base/ADT/Heap.h>
#include <Base/ADT/MemoryPool.h>
#include <Base/IO/TextStream.h>
#include <Base/Util/ArrayAdaptor.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS RectPacker
==============================================================================*/
class RectPacker
{
public:

   /*----- types -----*/

   struct RectInfo
   {
      Vec2i      _size;     //!< The size (specified by the user).
      Vec2i      _position; //!< The position (computed by the packer).
      bool       _flipped;  //!< A flag indicating if the rectangle is flipped or not (i.e. if _size.xy == originalSize.yx ).
   };

   typedef Vector< RectInfo >   RectContainer;

   /*----- static methods -----*/

   static FUSION_DLL_API  Vec2i guessStartSize(
      const ConstArrayAdaptor<Vec2i>& sizes,
      const size_t n,
      bool forcePowerOfTwo = true
   );

   /*----- methods -----*/

   FUSION_DLL_API RectPacker( size_t n = 0 );
   FUSION_DLL_API virtual ~RectPacker();

   // ...
   FUSION_DLL_API virtual void  clear();

   // Packing.
   FUSION_DLL_API virtual bool  pack( const Vector<Vec2i>& sizes, const Vec2i& texSize ) = 0;
   FUSION_DLL_API virtual bool  pack(
      const ConstArrayAdaptor<Vec2i>& sizes,
      const size_t n,
      const Vec2i& maxSize,
      bool forcePowerOfTwo = true,
      bool allowRotation = false
   ) = 0;

   // Results.
   inline const RectContainer&  rects() const { return _rects; }
   inline const Vec2i& size() const           { return _size; }

   // Debugging.
   FUSION_DLL_API void  print( TextStream& os = StdOut ) const;
   FUSION_DLL_API RCP<Bitmap>  getBitmap() const;

protected:

   /*----- methods -----*/

   inline void reserve( size_t n ) { _rects.reserve( n ); }
   inline void resize( size_t n ) { _rects.resize( n ); }

   /*----- data members -----*/

   Vector< RectInfo >      _rects;  //!< All of the rectangles the user specified.
   Vec2i                   _size;   //!< The total size required.

}; //class RectPacker

//------------------------------------------------------------------------------
//!
inline TextStream&
operator<<( TextStream& os, const RectPacker::RectInfo& ri )
{
   return os << ri._size << " @ " << ri._position << (ri._flipped?" (flipped)":" (aligned)");
}

/*==============================================================================
  CLASS RectPackerGreedyKdTree
==============================================================================*/
class RectPackerGreedyKdTree:
   public RectPacker
{
public:

   /*----- methods -----*/

   FUSION_DLL_API RectPackerGreedyKdTree( size_t n = 0 );
   FUSION_DLL_API virtual ~RectPackerGreedyKdTree();

   FUSION_DLL_API virtual void  clear();

   // Packing.
   FUSION_DLL_API virtual bool  pack( const Vector<Vec2i>& sizes, const Vec2i& texSize );
   FUSION_DLL_API virtual bool  pack(
      const ConstArrayAdaptor<Vec2i>&  sizes,
      const size_t                     n,
      const Vec2i&                     maxSize,
      bool                             forcePowerOfTwo = true,
      bool                             allowRotation   = false
   );

protected:

   /*----- classes -----*/
public:
   struct Node
   {
      Node*  _child[2]; //!< The child nodes (left/right if _split >= 0, bottom/right otherwise).
      int    _split;    //!< The split value (negative means vertical, otherwise horizontal).
      int    _id;       //!< The rectangle ID being stored in this node (negative means unused).
      enum
      {
         USED_BY_CHILDREN = 0x7FFFFFFF
      };
      Node() { _child[0] = NULL; _child[1] = NULL; _split = 0; _id = -1; }
      void  propagateChildUsage() { if( _child[0]->_id >= 0 && _child[1]->_id >= 0 )  _id = USED_BY_CHILDREN; }
   };

   /*----- methods -----*/

   bool  search( const Vec2i& sizeToInsert, int id, Node* node, AARecti& rect, bool allowRotation );
   bool  visitChildren( const Vec2i& sizeToInsert, int id, Node* node, AARecti& rect, bool allowRotation );
   bool  visitLeaf( const Vec2i& sizeToInsert, int id, Node* node, AARecti& rect, bool allowRotation );
   void  destroy( Node* node );

   /*----- data members -----*/

   MemoryPool<Node>    _pool;  //!< A memory pool for the nodes.

}; //class RectPackerGreedyKdTree


/*==============================================================================
  CLASS RectPackerGreedy
==============================================================================*/
class RectPackerGreedy:
   public RectPacker
{
public:

   /*----- methods -----*/

   FUSION_DLL_API RectPackerGreedy( size_t n = 0 );
   FUSION_DLL_API virtual ~RectPackerGreedy();

   FUSION_DLL_API virtual void  clear();

   // Packing.
   FUSION_DLL_API virtual bool  pack( const Vector<Vec2i>& sizes, const Vec2i& texSize );
   FUSION_DLL_API virtual bool  pack(
      const ConstArrayAdaptor<Vec2i>&  sizes,
      const size_t                     n,
      const Vec2i&                     maxSize,
      bool                             forcePowerOfTwo = true,
      bool                             allowRotation   = true
   );

protected:

   /*----- types -----*/
   struct Region
   {
      Vec2i    _pos;      //!< The position of the lower-left corner of the region.
      bool     _flipped;  //!< Set when the rectangle region is vertical rather than horizontal.
      Region*  _next;     //!< A pointer to the next region of the same size.
      // The region's width and height are store implicitly in the parent group and class.
   };
   struct RegionGroup
   {
      int           _min;    //!< The min of the 2 dimensions (the max is in the class).
      RegionGroup*  _next;   //!< A pointer to the next region group (with a bigger min).
      Region*       _first;  //!< A pointer to the first region of max*min size.
      inline bool bigEnoughFor( int min ) const { return min <= _min; }
   };
   struct RegionClass
   {
      int           _max;   //!< The max of the 2 dimensions (shared by all Regions inside this class).
      RegionClass*  _next;  //!< The next region class (with a bigger max).
      RegionClass*  _prev;  //!< The previous region class (with a smaller max).
      RegionGroup*  _first; //!< The first region group.
      inline bool bigEnoughFor( int max ) const { return max <= _max; }
   };

   /*----- data members -----*/

   MemoryPool<RegionClass>  _clsPool;
   MemoryPool<RegionGroup>  _grpPool;
   MemoryPool<Region>       _regPool;

   RegionClass*  _smallest;  //!< Pointer to the first RegionClass.
   RegionClass*  _largest;   //!< Pointer to the last RegionClass.
   RegionClass*  _latest;    //!< Pointer to the RegionClass used for the previous size candidate.

   Vec2i  _curSize;  //!< The current size.
   Vec2i  _maxSize;  //!< The maximum size allowed.

   /*----- methods -----*/
   
   RegionClass&  getClass( int max );
   RegionClass*  getClassLTE( int max, RegionClass* largestPossible );
   RegionClass*  getClassGTE( int max, RegionClass* smallestPossible );
   RegionClass*  createClass( int max, RegionClass* next, RegionClass* prev );
   inline void  destroyClass( RegionClass* cls );

   RegionGroup&  getGroup( RegionClass& cls, int min );
   RegionGroup*  createGroup( int min, RegionGroup* next );
   inline void  destroyGroup( RegionGroup* grp );

   void  addRegion( Region* reg, const Vec2i& size );
   void  addRegion( RegionGroup& grp, Region* reg );
   bool  createNewRegion( const Vec2i& reqSize, RegionClass*& cls, RegionGroup*& grp, Region*& reg );
   Region*  createRegion();
   Region*  createRegion( const Vec2i& pos, bool flipped, Region* next );
   inline void  destroyRegion( Region* reg );

   bool  findCandidate( const Vec2i& reqSize, RegionClass*&cls, RegionGroup*& grp, Region*& reg );
   void  destroy( RegionClass* cls, RegionGroup* grp, Region* reg );

   void  printUnusedRegions() const;

private:
}; //class RectPackerGreedy

//------------------------------------------------------------------------------
//!
inline RectPackerGreedy::RegionClass*
RectPackerGreedy::createClass( int max, RegionClass* next, RegionClass* prev )
{
   RegionClass* cls = _clsPool.alloc();
   cls->_max   = max;
   cls->_next  = next;
   cls->_prev  = prev;
   cls->_first = NULL;
   return cls;
}

//------------------------------------------------------------------------------
//!
inline void
RectPackerGreedy::destroyClass( RegionClass* cls )
{
   _clsPool.destroy( cls );
}

//------------------------------------------------------------------------------
//!
inline RectPackerGreedy::RegionGroup*
RectPackerGreedy::createGroup( int min, RegionGroup* next )
{
   RegionGroup* grp = _grpPool.alloc();
   grp->_min   = min;
   grp->_next  = next;
   grp->_first = NULL;
   return grp;
}

//------------------------------------------------------------------------------
//!
inline void
RectPackerGreedy::destroyGroup( RegionGroup* grp )
{
   _grpPool.destroy( grp );
}

//------------------------------------------------------------------------------
//!
inline RectPackerGreedy::Region*
RectPackerGreedy::createRegion()
{
   Region* reg = _regPool.alloc();
   return reg;
}

//------------------------------------------------------------------------------
//!
inline RectPackerGreedy::Region*
RectPackerGreedy::createRegion( const Vec2i& pos, bool flipped, Region* next )
{
   Region* reg   = _regPool.alloc();
   reg->_pos     = pos;
   reg->_flipped = flipped;
   reg->_next    = next;
   return reg;
}

//------------------------------------------------------------------------------
//!
inline void
RectPackerGreedy::destroyRegion( Region* reg )
{
   _regPool.destroy( reg );
}


/*==============================================================================
  CLASS IncrementalRectPacker
==============================================================================*/
//! Simply fills rectangles left-to-right, and changes rows when the next
//! rectangle doesn't fit.
class IncrementalRectPacker
{
public:

   /*----- methods -----*/

   IncrementalRectPacker( uint maxWidth, uint border=0 ): _maxWidth( maxWidth ), _border( border ) { reset(); }

   Vec2i  add( const Vec2i& size );

   inline uint curX() const { return _curX; }
   inline uint curY() const { return _curY; }
   inline uint curH() const { return _curH; }

   inline uint width() const { return (_curY != 0 ) ? _maxWidth : _curX+_border; }
   inline uint height() const { return _curY + _curH + _border; }
   inline Vec2i  dimension() const { return Vec2i( width(), height() ); }

   inline uint border() const { return _border; }
   inline void border( uint v ) { _border = v; }

   inline void reset() { _curX = _curY = _curH = 0; }
   inline void reset( uint maxWidth ) { _maxWidth = maxWidth; reset(); }

protected:

   /*----- data members -----*/

   uint  _maxWidth;  //!< The largest width a row can take (including borders).
   uint  _border;    //!< A border to place around each glyph (to avoid bilinear leaking).
   uint  _curX;      //!< The current row's horizontal position.
   uint  _curY;      //!< The current row's vertical position.
   uint  _curH;      //!< The current row's height.

   /*----- methods -----*/

   /* methods... */

private:
}; //class IncrementalRectPacker

NAMESPACE_END

#endif //RECT_PACKER_H
