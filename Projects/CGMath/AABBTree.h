/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef CGMATH_AABBTREE_H
#define CGMATH_AABBTREE_H

#include <CGMath/StdDefs.h>
#include <CGMath/AABBox.h>

#include <Base/ADT/MemoryPool.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS AABBTree
 ==============================================================================*/

class AABBTree
{
public:

   /*----- classes -----*/

   struct Node {
      
      /*----- methods -----*/

      void clear() 
      {
         _flags0    = 0;
         _flags1    = 0;
         _leftNode  = 0;
         _rightNode = 0;
      }

      bool isLeftNode() const  { return (_flags0 & 64) != 0; }
      bool isRightNode() const { return (_flags0 & 128) != 0; }

      /*----- data members -----*/
      
      uint8_t  _flags0;
      uint8_t  _flags1;
      uint8_t  _bounds[6];
      union
      {
         Node* _leftNode;
         void* _leftLeaf;
      };
      union
      {
         Node* _rightNode;
         void* _rightLeaf;
      };
   };

   class Pool: public RCObject
   {
   public:

      /*----- methods -----*/

      Pool( uint chunkSize = 32 ) : _pool( chunkSize ) {}

      void clear()           { _pool.clear(); }
      Node* alloc()          { return _pool.alloc(); }
      void free( Node* n )   { _pool.free( n ); }
      uint chunkSize() const { return uint(_pool.chunkSize()); }

   private:

      /*----- data members -----*/

      MemoryPool<Node> _pool;
   };

   /*----- types -----*/

   typedef void (*collisionFunc)( void* data, void* objA, void* objB );

   /*----- static methods -----*/
   
   static void decodeBBs( const AABBoxf& pbox, const Node& n, AABBoxf& lbox, AABBoxf& rbox );
   static void encodeBBs( const AABBoxf& pbox, Node& n, AABBoxf& lbox, AABBoxf& rbox );

   /*----- methods -----*/

   inline AABBTree( const RCP<AABBTree::Pool>& );
   inline ~AABBTree();

   // Operations.
   inline void clear();

   CGMATH_DLL_API void create( AABBoxf* bboxes, void** elements, uint num );

   // Attributes.
   inline const AABBoxf& region() const { return _box; }
   inline const Node* root() const      { return &_node; }

   // Queries.
   CGMATH_DLL_API void findCollisions( void* data, void* objB, const AABBoxf&, collisionFunc ) const;
   CGMATH_DLL_API void findCollisions( void* data, const AABBTree&, collisionFunc ) const;
   // ray casting...

private:
   
   /*----- methods -----*/

   void clear( Node* );
   void create( const AABBoxf& box, Node*, AABBoxf* bboxes, void** elements, uint num );

   AABBTree( const AABBTree& );
   void operator=( const AABBTree& );

   /*----- data members -----*/

   AABBoxf   _box;
   Node      _node;
   RCP<Pool> _pool;
};

//------------------------------------------------------------------------------
//!
inline 
AABBTree::AABBTree( const RCP<AABBTree::Pool>& pool ) :
   _box( AABBoxf::empty() ), _pool( pool )
{
   _node.clear();
}

//------------------------------------------------------------------------------
//! 
inline
AABBTree::~AABBTree()
{
   if( !_pool.isUnique() ) clear();
}

//------------------------------------------------------------------------------
//!
inline void
AABBTree::clear()
{
   if( _node.isLeftNode() )  clear( _node._leftNode );
   if( _node.isRightNode() ) clear( _node._rightNode );
   _node.clear();
   _box = AABBoxf::empty();
}

//------------------------------------------------------------------------------
//! 
inline void
AABBTree::clear( AABBTree::Node* node )
{
   if( node->isLeftNode() )  clear( node->_leftNode );
   if( node->isRightNode() ) clear( node->_rightNode );
   _pool->free( node );
}

//------------------------------------------------------------------------------
//! 
inline void 
AABBTree::decodeBBs( const AABBoxf& pbox, const Node& n, AABBoxf& lbox, AABBoxf& rbox )
{
   for( uint i = 0; i < 3; ++i )
   {
      float t0 = (float)n._bounds[i]/255.0f;
      float t1 = (float)n._bounds[i+3]/255.0f;
      float b0 = pbox.min(i)*(1.0f-t0) + pbox.max(i)*t0;
      float b1 = pbox.min(i)*(1.0f-t1) + pbox.max(i)*t1;

      // Minimum boundaries.
      if( n._flags0 & (1<<i) )
      {
         lbox.min(i) = pbox.min(i);
         rbox.min(i) = b0;
      }
      else
      {
         lbox.min(i) = b0;
         rbox.min(i) = pbox.min(i);
      }
      // Maximum boundaries.
      if( n._flags0 & (1<<(i+3)) )
      {
         lbox.max(i) = pbox.max(i);
         rbox.max(i) = b1;
      }
      else
      {
         lbox.max(i) = b1;
         rbox.max(i) = pbox.max(i);
      }
   }
}

//------------------------------------------------------------------------------
//! 
inline void 
AABBTree::encodeBBs( const AABBoxf& pbox, Node& n, AABBoxf& lbox, AABBoxf& rbox )
{
   uint8_t flag = 0;
   for( uint i = 0; i < 3; ++i )
   {
      float size = pbox.size(i);
      // Minimum boundaries.
      if( lbox.min(i) < rbox.min(i) )
      {
         flag |= 1<<i;
         lbox.min(i)  = pbox.min(i);
         n._bounds[i] = CGM::floori( 255.0f*((rbox.min(i)-pbox.min(i)) / size) );
         float t      = (float)n._bounds[i]/255.0f;
         rbox.min(i)  = pbox.min(i)*(1.0f-t) + pbox.max(i)*t;
      }
      else
      {
         rbox.min(i)  = pbox.min(i);
         n._bounds[i] = CGM::floori( 255.0f*((lbox.min(i)-pbox.min(i)) / size) );
         float t      = (float)n._bounds[i]/255.0f;
         lbox.min(i)  = pbox.min(i)*(1.0f-t) + pbox.max(i)*t;
      }
      // Maximum boundaries.
      if( lbox.max(i) > rbox.max(i) )
      {
         flag |= 1<<(i+3);
         lbox.max(i)    = pbox.max(i);
         n._bounds[i+3] = CGM::ceili( 255.0f*((rbox.max(i)-pbox.min(i)) / size) );
         float t        = (float)n._bounds[i+3]/255.0f;
         rbox.max(i)    = pbox.min(i)*(1.0f-t) + pbox.max(i)*t;
      }
      else
      {
         rbox.max(i)    = pbox.max(i);
         n._bounds[i+3] = CGM::ceili( 255.0f*((lbox.max(i)-pbox.min(i)) / size) );
         float t        = (float)n._bounds[i+3]/255.0f;
         lbox.max(i)    = pbox.min(i)*(1.0f-t) + pbox.max(i)*t;
      }
   }
   n._flags0 = flag | (n._flags0 & 192);
}

NAMESPACE_END

#endif

