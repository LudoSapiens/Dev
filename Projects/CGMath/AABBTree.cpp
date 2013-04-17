/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <CGMath/AABBTree.h>

#include <Base/Dbg/Defs.h>

/*==============================================================================
   UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//! 
inline bool
descendA( const AABBoxf& boxA, const AABBoxf& boxB )
{
   return boxA.volume() > boxB.volume();
}

//------------------------------------------------------------------------------
//!
void findCollisionsAB( 
   const AABBoxf&          box, 
   void*                   leaf,
   const AABBoxf&          nbox, 
   const AABBTree::Node*   node, 
   AABBTree::collisionFunc func, 
   void*                   data
)
{
   // TODO: Make it iterative.
   AABBoxf lbox;
   AABBoxf rbox;
   AABBTree::decodeBBs( nbox, *node, lbox, rbox );

   if( box.isOverlapping( lbox ) )
   {
      if( node->isLeftNode() )
      {
         findCollisionsAB( box, leaf, lbox, node->_leftNode, func, data );
      }
      else
      {
         func( data, leaf, node->_leftLeaf );
      }
   }
   if( box.isOverlapping( rbox ) )
   {
      if( node->isRightNode() )
      {
         findCollisionsAB( box, leaf, rbox, node->_rightNode, func, data );
      }
      else
      {
         func( data, leaf, node->_rightLeaf );
      }
   }
}

//------------------------------------------------------------------------------
//!
void findCollisionsBA( 
   const AABBoxf&          box, 
   void*                   leaf,
   const AABBoxf&          nbox, 
   const AABBTree::Node*   node, 
   AABBTree::collisionFunc func, 
   void*                   data
)
{
   // TODO: Make it iterative.
   AABBoxf lbox;
   AABBoxf rbox;
   AABBTree::decodeBBs( nbox, *node, lbox, rbox );

   if( box.isOverlapping( lbox ) )
   {
      if( node->isLeftNode() )
      {
         findCollisionsBA( box, leaf, lbox, node->_leftNode, func, data );
      }
      else
      {
         func( data, node->_leftLeaf, leaf );
      }
   }
   if( box.isOverlapping( rbox ) )
   {
      if( node->isRightNode() )
      {
         findCollisionsBA( box, leaf, rbox, node->_rightNode, func, data );
      }
      else
      {
         func( data, node->_rightLeaf, leaf );
      }
   }
}

//------------------------------------------------------------------------------
//! 
void findCollisions( 
   const AABBoxf&          boxA, 
   const AABBTree::Node*   nodeA, 
   const AABBoxf&          boxB, 
   const AABBTree::Node*   nodeB, 
   AABBTree::collisionFunc func, 
   void*                   data
)
{
   // TODO: Optimize by making it an iterative function.
   if( descendA( boxA, boxB ) )
   {
      AABBoxf lboxA;
      AABBoxf rboxA;
      AABBTree::decodeBBs( boxA, *nodeA, lboxA, rboxA );

      if( lboxA.isOverlapping( boxB ) )
      {
         if( nodeA->isLeftNode() )
         {
            findCollisions( lboxA, nodeA->_leftNode, boxB, nodeB, func, data );
         }
         else
         {
            findCollisionsAB( lboxA, nodeA->_leftLeaf, boxB, nodeB, func, data );
         }
      }
      if( rboxA.isOverlapping( boxB ) )
      {
         if( nodeA->isRightNode() )
         {
            findCollisions( rboxA, nodeA->_rightNode, boxB, nodeB, func, data );
         }
         else
         {
            findCollisionsAB( rboxA, nodeA->_rightLeaf, boxB, nodeB, func, data );
         }
      }
   }
   else
   {
      AABBoxf lboxB;
      AABBoxf rboxB;
      AABBTree::decodeBBs( boxB, *nodeB, lboxB, rboxB );

      if( lboxB.isOverlapping( boxA ) )
      {
         if( nodeB->isLeftNode() )
         {
            findCollisions( boxA, nodeA, lboxB, nodeB->_leftNode, func, data );
         }
         else
         {
            findCollisionsBA( lboxB, nodeB->_leftLeaf, boxA, nodeA, func, data );
         }
      }
      if( rboxB.isOverlapping( boxA ) )
      {
         if( nodeB->isRightNode() )
         {
            findCollisions( boxA, nodeA, rboxB, nodeB->_rightNode, func, data );
         }
         else
         {
            findCollisionsBA( rboxB, nodeB->_rightLeaf, boxA, nodeA, func, data );
         }
      }
   }
}

UNNAMESPACE_END


NAMESPACE_BEGIN

/*==============================================================================
   CLASS AABBTree
==============================================================================*/

//------------------------------------------------------------------------------
//! 
void 
AABBTree::findCollisions( void* data, void* objB, const AABBoxf& box, collisionFunc func ) const
{
   ::findCollisionsBA( box, objB, _box, &_node, func, data );
}

//------------------------------------------------------------------------------
//! 
void 
AABBTree::findCollisions( void* data, const AABBTree& tree, collisionFunc func ) const
{
   ::findCollisions( _box, &_node, tree.region(), tree.root(), func, data );
}

//------------------------------------------------------------------------------
//! 
void 
AABBTree::create( AABBoxf* bboxes, void** elements, uint num )
{
   // Nothing to do.
   if( num == 0 ) return;

   //clear();

   // Compute bounding box.
   for( uint i = 0; i < num; ++i )
   {
      _box |= bboxes[i]; 
   }

   // Fill in the tree.
   create( _box, &_node, bboxes, elements, num );
}

//------------------------------------------------------------------------------
//! 
void
AABBTree::create( 
   const AABBoxf&  box, 
   AABBTree::Node* node, 
   AABBoxf*        bboxes, 
   void**          elements, 
   uint            num
)
{
   AABBoxf lbox;
   AABBoxf rbox;
   uint pivot;

   // Partition elements.
   if( num <= 2 )
   {
      pivot = 1;
      lbox  = bboxes[0];
      rbox  = (num == 2) ? bboxes[1] : box;
   }
   else
   {
      int axis = box.longestSide();
      for( int i = 0; i < 3; ++i, axis = (axis+1)%3 )
      {
         // Compute mean value for splitting plane.
         float mean = 0.0f;
         for( uint e = 0; e < num; ++e )
         {
            mean += bboxes[e].min(axis) + bboxes[e].max(axis);
         }
         mean /= float(num*2);

         // Parition elements.
         lbox  = AABBoxf::empty();
         rbox  = AABBoxf::empty();
         pivot = 0;
         for( uint e = 0; e < num; ++e )
         {
            if( bboxes[e].center(axis) < mean )
            {
               lbox |= bboxes[e];
               CGM::swap( elements[e], elements[pivot] );
               CGM::swap( bboxes[e], bboxes[pivot++] );
            }
            else
            {
               rbox |= bboxes[e];
            }
         }
         // Is splitting good?
         if( pivot > 0 && pivot < num ) break;
      }
   }

   CHECK( pivot > 0 );

   // Dbg
   //AABBoxf clbox = lbox;
   //AABBoxf crbox = rbox;

   encodeBBs( box, *node, lbox, rbox );
/*
   StdErr << "box:   " << box << "\n";
   StdErr << "clbox: " << clbox << "\n";
   StdErr << "lbox:  " << lbox << "\n";
   StdErr << "crbox: " << crbox << "\n";
   StdErr << "rbox:  " << rbox << "\n";
   
   StdErr << "bound: " 
          << node->_bounds[0] << " " << node->_bounds[1] << " " << node->_bounds[2] << " "
          << node->_bounds[3] << " " << node->_bounds[4] << " " << node->_bounds[5] << "\n";
   StdErr << "flags: " << node->_flags0 << "\n";
*/
   // Dbg...
   //CHECK( box.isInside( lbox ) );
   //CHECK( box.isInside( rbox ) );
   //CHECK( lbox.isInside( clbox ) );
   //CHECK( rbox.isInside( crbox ) );
   
   // Left node.
   if( pivot == 1 )
   {
      node->_leftLeaf = elements[0];
   }
   else
   {
      node->_flags0  |= 64;
      node->_leftNode = _pool->alloc();
      node->_leftNode->clear();
      create( lbox, node->_leftNode, bboxes, elements, pivot );
   }

   // Right node.
   if( num-pivot == 1 )
   {
      node->_rightLeaf = elements[pivot];
   }
   else if( num-pivot > 1 )
   {
      node->_flags0   |= 128;
      node->_rightNode = _pool->alloc();
      node->_rightNode->clear();
      create( rbox, node->_rightNode, &bboxes[pivot], &elements[pivot], num-pivot );
   }
}

NAMESPACE_END
