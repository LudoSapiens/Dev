/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef CGMATH_OCTREE_H
#define CGMATH_OCTREE_H

#include <CGMath/StdDefs.h>
#include <CGMath/AABBox.h>

#include <Base/ADT/MemoryPool.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS Octree
 ==============================================================================*/

template< typename T, typename V >
class Octree
{
public:

   /*----- classes -----*/

   struct Data
   {
      T     _data;
      Data* _next;
   };
   
   struct Node
   {
      Node*  _parent;
      Node*  _children;
      Data*  _data;

      uint   _level;
      uint   _loc[3];

      V      _value;
   };

   /*----- methods -----*/

   Octree( uint maxLevel = 10 );

   Node& root() { return _root; }
   const Node& root() const { return _root; }

   // Define octree space.
   void region( const AABBoxf& r );
   const AABBoxf& region() const;

   AABBoxf region( const Node& ) const;
   float center( const Node& node, uint axis ) const;

   // Operations.
   void clear();
   void clear( Node& );
   void split( Node& );
   void merge( Node& );
   
   void add( Node&, const T& );
   void remove( Node&, const T& );
  
   // Queries.
   uint numNodes() const;
   uint numNodes( const Node& ) const;

private:

   /*----- classes -----*/

   struct Level
   {
      Node _nodes[8];
   };

   /*----- methods -----*/

   Octree( const Octree& );
   void operator=( const Octree& );

   void clearRoot();

   /*----- data members -----*/

   AABBoxf           _region;
   Node              _root;
   MemoryPool<Level> _nodes;
   MemoryPool<Data>  _data;
   uint              _maxLevel;
   Vec3f             _scale;
};

//------------------------------------------------------------------------------
//!
template< typename T, typename V >
Octree<T,V>::Octree( uint maxLevel ) :
   _region( AABBoxf::empty() ),
   _nodes(), _data(),
   _maxLevel( maxLevel )
{
   // Init root node.
   clearRoot();
}

//------------------------------------------------------------------------------
//!
template< typename T, typename V > void
Octree<T,V>::region( const AABBoxf& r )
{
   _region         = r;
   float invMaxLoc = 1.0f / (float)(1<<_maxLevel);
   _scale          = _region.size() * invMaxLoc;
}

//------------------------------------------------------------------------------
//!
template< typename T, typename V > const AABBoxf&
Octree<T,V>::region() const
{
   return _region;
}

//------------------------------------------------------------------------------
//!
template< typename T, typename V > AABBoxf
Octree<T,V>::region( const Node& node ) const
{
   uint cellSize = 1<<node._level;

   float x0 = (float)(node._loc[0]         )*_scale(0) + _region.min(0);
   float x1 = (float)(node._loc[0]+cellSize)*_scale(0) + _region.min(0);
   float y0 = (float)(node._loc[1]         )*_scale(1) + _region.min(1);
   float y1 = (float)(node._loc[1]+cellSize)*_scale(1) + _region.min(1);
   float z0 = (float)(node._loc[2]         )*_scale(2) + _region.min(2);
   float z1 = (float)(node._loc[2]+cellSize)*_scale(2) + _region.min(2);
   return AABBoxf( x0, x1, y0, y1, z0, z1 );
}

//------------------------------------------------------------------------------
//!
template< typename T, typename V > float
Octree<T,V>::center( const Node& node, uint axis ) const
{
   uint halfCellSize = 1<<(node._level-1);
   return (float)(node._loc[axis]+halfCellSize)*_scale(axis) + _region.min(axis);
}

//------------------------------------------------------------------------------
//!
template< typename T, typename V > void
Octree<T,V>::clear()
{
   clearRoot();
   _nodes.clear();
   _data.clear();
}

//------------------------------------------------------------------------------
//!
template< typename T, typename V > void
Octree<T,V>::clear( Node& node )
{
   Data* cur  = node._data;
   while( cur )
   {
     Data* prev = cur;
     cur        = cur->_next;
     _data.free( prev );
   }
   node._data = 0;
}

//------------------------------------------------------------------------------
//!
template< typename T, typename V > void
Octree<T,V>::split( Node& node )
{
   if( !node._children && node._level > 0 )
   {
      node._children = reinterpret_cast<Node*>(_nodes.alloc());
      for( uint i = 0; i < 8; ++i )
      {
         Node& child     = node._children[i];
         child._parent   = &node;
         child._children = 0;
         child._data     = 0;
         child._level    = node._level - 1;
         child._loc[0]   = node._loc[0] + ((i & 1)      << child._level);
         child._loc[1]   = node._loc[1] + (((i>>1) & 1) << child._level);
         child._loc[2]   = node._loc[2] + ((i>>2)       << child._level);
      }
   }
}

//------------------------------------------------------------------------------
//!
template< typename T, typename V > void
Octree<T,V>::merge( Node& node )
{
   // Do we have children to merge?
   if( !node._children )
   {
      return;
   }

   for( uint i = 0; i < 8; ++i )
   {
      Node& child = node._children[i];

      // Merge child.
      if( child._children )
      {
         merge( child );
      }
      // Merge child data.
      if( child._data )
      {
         // Find last data.
         Data* data = child._data;
         while( data->_next )
         {
            data = data->_next;
         }
         // Merge to parent list.
         data->_next = node._data;
         node._data  = child._data;
      }
   }

   // Remove children.
   _nodes.free( reinterpret_cast<Level*>(node._children) );
   node._children = 0;
}

//------------------------------------------------------------------------------
//!
template< typename T, typename V > void
Octree<T,V>::add( Node& node, const T& t )
{
   Data* data  = _data.alloc();
   data->_data = t;
   data->_next = node._data;
   node._data  = data;
}

//------------------------------------------------------------------------------
//!
template< typename T, typename V > void
Octree<T,V>::remove( Node& node, const T& t )
{
   // t
   Data* prev = 0;
   Data* cur  = node._data;
   for( ; cur != 0; prev = cur, cur = cur->_next )
   {
      if( cur->_data == t )
      {
         if( prev == 0 ) node._data  = cur->_next;
         else            prev->_next = cur->_next;
         _data.free( cur );
         return;
      }
   }
}

//------------------------------------------------------------------------------
//!
template< typename T, typename V > void
Octree<T,V>::clearRoot()
{
   _root._parent   = 0;
   _root._children = 0;
   _root._data     = 0;
   _root._level    = _maxLevel;
   _root._loc[0]   = 0;
   _root._loc[1]   = 0;
   _root._loc[2]   = 0;
}

//------------------------------------------------------------------------------
//!
template< typename T, typename V > uint
Octree<T,V>::numNodes() const
{
   return numNodes( _root );
}

//------------------------------------------------------------------------------
//!
template< typename T, typename V > uint
Octree<T,V>::numNodes( const Node& n ) const
{
   if( n._children )
   {
      return 1 +
         numNodes( n._children[0] ) +
         numNodes( n._children[1] ) +
         numNodes( n._children[2] ) +
         numNodes( n._children[3] ) +
         numNodes( n._children[4] ) +
         numNodes( n._children[5] ) +
         numNodes( n._children[6] ) +
         numNodes( n._children[7] );
   }
   else
   {
      return 1;
   }
}

NAMESPACE_END

#endif

