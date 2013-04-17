/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef CGMATH_BIH_H
#define CGMATH_BIH_H

#include <CGMath/StdDefs.h>
#include <CGMath/AABBox.h>
#include <CGMath/Ray.h>

#include <Base/IO/TextStream.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS BIH
==============================================================================*/
class BIH
{

public: 

   /*----- classes -----*/

   struct Node
   {
      // The first 3 bits are flags.
      // 0: Split in X.
      // 1: Split in Y.
      // 2: Split in Z.
      // 3: Leaf node.
      // 4: Clip node in X.
      // 5: Clip node in Y.
      // 6: Clip node in Z.
      enum
      {
         BIH_NODE_SPLIT_X,
         BIH_NODE_SPLIT_Y,
         BIH_NODE_SPLIT_Z,
         BIH_NODE_LEAF,
         BIH_NODE_CLIP_X,
         BIH_NODE_CLIP_Y,
         BIH_NODE_CLIP_Z
      };
      
      // A split node, split a node in 2 where _plane[0] is the new
      // maximum of the "left" bounding box and _plane[1] is the new
      // minimum of the "right" bounding box.
      
      // A clip node has only 1 child where _plane[0] is the new minimum
      // and _plane[1] the new maximum of its bounding box.
      
      bool isLeaf() const
      {
         return (_index & 7) == 3;
      }
      
      bool isSplitNode() const
      {
         return (_index & 7) <= 2;
      }
      
      bool isClipNode() const
      {
         return (_index & 4 ) != 0;
      }
      
      bool isInteriorNode() const
      {
         return (_index & 7) != 3;
      }
      
      uint axis() const
      {
         return _index & 3;
      }
      
      uint index() const
      {
         return _index >> 3;
      }
      
      uint flags() const
      {
         return _index & 7;
      }
      
      uint _index;
      union
      {
         uint  _numElements;
         float _plane[2];
      };
   };
   
   struct Hit
   {
      Hit() :
         _t( CGConstf::infinity() ), // Far away!!
         _id(0)
      {}
         
      float _t;
      uint  _id;
   };
   
   /*----- types and enumerations ----*/

   typedef bool (*IntersectFunc)( const Rayf&, uint, float&, void* );
   typedef bool (*InsideFunc)( const AABBoxf&, uint, void* );

   /*----- methods -----*/

   CGMATH_DLL_API BIH();
   CGMATH_DLL_API BIH( const BIH& );
   CGMATH_DLL_API ~BIH();

   // Creation.
   CGMATH_DLL_API void create(
      const Vector<AABBoxf*>& bboxes,
      const Vector<Vec3f>&    centers,
      Vector<uint>*           ids,
      uint                    leafSize = 1,
      uint                    maxDepth = 0
   );

   // Queries.
   inline bool isEmpty() const;
   
   // Reset.
   inline void clear();
   
   // Queries.
   CGMATH_DLL_API bool trace( const Rayf&, Hit& hit, IntersectFunc, void* data ) const;
   CGMATH_DLL_API bool findElementsInside( const AABBoxf& box, Vector<uint>& dst ) const;
   CGMATH_DLL_API bool findElementsInside( const AABBoxf& box, Vector<uint>& dst, InsideFunc, void* data ) const;
   
   // Debugging
   CGMATH_DLL_API void searchElement( uint elem ) const;
   CGMATH_DLL_API void print( TextStream& os, const String& prefix = String() ) const;
   
private: 

   /*----- data members -----*/

   uint          _maxDepth;
   Vector<Node>  _nodes;
   Vector<uint>  _ids;

};

//------------------------------------------------------------------------------
//!
inline bool
BIH::isEmpty() const
{
   return _nodes.empty();
}

//------------------------------------------------------------------------------
//!
inline void
BIH::clear()
{
   _nodes = Vector<Node>();
   _ids   = Vector<uint>();
}

//------------------------------------------------------------------------------
//!
inline TextStream& operator<<( TextStream& os, const BIH& bih ) { bih.print(os); return os; }

NAMESPACE_END

#endif
