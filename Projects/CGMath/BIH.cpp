/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <CGMath/BIH.h>
#include <CGMath/CGMath.h>

#include <Base/Dbg/DebugStream.h>
#include <Base/Util/Timer.h>

/*==============================================================================
   UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

DBG_STREAM( os_bih, "BIH" );
DBG_STREAM( os_bih_trace, "BIH_Trace" );


/*==============================================================================
   CLASS BuildStackNode
==============================================================================*/

class BuildStackNode {

public:

   /*----- methods -----*/

   BuildStackNode() {}

   inline void set(
      uint           nodeID,
      uint           begin,
      uint           end,
      uint           depth,
      const AABBoxf& node,
      const AABBoxf& split
   )
   {
      _nodeID   = nodeID;
      _begin    = begin;
      _end      = end;
      _depth    = depth;
      _nodeBox  = node;
      _splitBox = split;
   }

   inline void get(
      uint&    nodeID,
      uint&    begin,
      uint&    end,
      uint&    depth,
      AABBoxf& node,
      AABBoxf& split
   )
   {
      nodeID = _nodeID;
      begin  = _begin;
      end    = _end;
      depth  = _depth;
      node   = _nodeBox;
      split  = _splitBox;
   }

private:

   /*----- data members -----*/

   uint    _nodeID;
   uint    _begin;
   uint    _end;
   uint    _depth;
   AABBoxf _nodeBox;
   AABBoxf _splitBox;
};

/*==============================================================================
   CLASS TraversalStackNode
==============================================================================*/

class TraversalStackNode {

public:

/*----- methods -----*/

   TraversalStackNode() {}

   inline void set(
      const BIH::Node* node,
      float            tmin,
      float            tmax
   )
   {
      _node = node;
      _tmin = tmin;
      _tmax = tmax;
   }

   inline void get(
      const BIH::Node*& node,
      float&            tmin,
      float&            tmax
   )
   {
      node = _node;
      tmin = _tmin;
      tmax = _tmax;
   }

private:

   /*----- data members -----*/

   const BIH::Node* _node;
   float            _tmin;
   float            _tmax;
};

UNNAMESPACE_END


NAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
inline TextStream&
operator<<
( TextStream& os, const BIH::Node& node )
{
   os << "node[";
   switch( node.flags() )
   {
      case BIH::Node::BIH_NODE_SPLIT_X:
         os << "SplitX";
         break;
      case BIH::Node::BIH_NODE_SPLIT_Y:
         os << "SplitY";
         break;
      case BIH::Node::BIH_NODE_SPLIT_Z:
         os << "SplitZ";
         break;
      case BIH::Node::BIH_NODE_LEAF:
         os << "LEAF";
         break;
      case BIH::Node::BIH_NODE_CLIP_X:
         os << "ClipX";
         break;
      case BIH::Node::BIH_NODE_CLIP_Y:
         os << "ClipY";
         break;
      case BIH::Node::BIH_NODE_CLIP_Z:
         os << "ClipZ";
         break;
   }
   os << " idx=" << node.index();
   if( node.isInteriorNode() )
   {
      os << " pl[0]=" << node._plane[0];
      os << " pl[1]=" << node._plane[1];
   }
   else
   {
      os << " " << node._numElements << " elements";
   }
   os << "]";
   return os;
}

/*==============================================================================
   CLASS BIH
==============================================================================*/

//------------------------------------------------------------------------------
//!
BIH::BIH()
{
}

//------------------------------------------------------------------------------
//!
BIH::BIH( const BIH& bih )
{
   _maxDepth = bih._maxDepth;
   _nodes    = bih._nodes;
   _ids      = bih._ids;
}

//------------------------------------------------------------------------------
//!
BIH::~BIH()
{
}

//------------------------------------------------------------------------------
//!
void
BIH::create(
   const Vector<AABBoxf*>& bboxes,
   const Vector<Vec3f>&    centers,
   Vector<uint>*           ids,
   uint                    leafSize,
   uint                    maxDepth
)
{
   CHECK( bboxes.size() == centers.size() );

   DBG_BLOCK( os_bih, "Start computing BIH..." );
   DBG( Timer timer );

   // Initialization.
   DBG_MSG( os_bih, "# of elements: " << bboxes.size() << " " << centers.size() << "\n" );
   // 1. Init maximum recursion level.
   if( maxDepth == 0 )
   {
      maxDepth = (int)CGM::log2( float(centers.size()) ) * 2 + 1;
   }
   _maxDepth = CGM::min( maxDepth, (uint)100 );

   // 2. Init ids.
   if( ids == 0 )
   {
      _ids.resize( centers.size() );
      for( uint i = 0; i < _ids.size(); ++i )
      {
         _ids[i] = i;
      }
   }
   else
   {
      _ids.swap( *ids );
   }
   
   Vector<uint>  remap( centers.size() );
   for( uint i = 0; i < remap.size(); ++i )
   {
      remap[i] = i;
   }

   // 3. Init nodes and root.
   //_nodes.reserve();
   _nodes.resize(1);

   // 4. Compute bounding box.
   AABBoxf nodeBox  = AABBoxf::empty();
   for( uint i = 0; i < bboxes.size(); ++i )
   {
      nodeBox |= *bboxes[i];
   }
   AABBoxf splitBox = nodeBox;

   // 5. Stack.
   Vector<BuildStackNode> stack( maxDepth );
   uint stackID = 0;

   uint maxPrim = 0;
   uint maxD = 0;

   // Construct BIH.
   uint begin = 0;
   uint end   = uint(_ids.size());
   uint depth = 1;
   uint node  = 0;
   while( 1 )
   {
      // Do we have a root node?
      if( (end - begin <= leafSize) || depth >= maxDepth )
      {
         // Root node.
         _nodes[node]._index = (begin << 3) + 3;
         _nodes[node]._numElements = end-begin;

         maxPrim = CGM::max( maxPrim, end-begin );
         maxD = CGM::max( maxD, depth );

         // Are we done?
         if( stackID == 0 )
         {
            break;
         }
         stack[--stackID].get( node, begin, end, depth, nodeBox, splitBox );
      }
      else
      {
         // Compute split plane and axis.
         uint axis        = splitBox.longestSide();
         float splitPlane = splitBox.center( axis );

         // Partition primitives.
         AABBoxf leftNodeBox  = AABBoxf::empty();
         AABBoxf rightNodeBox = AABBoxf::empty();
         uint pivot = begin;
         for( uint i = begin; i < end; ++i )
         {
            if( centers[remap[i]](axis) < splitPlane )
            {
               leftNodeBox |= (*bboxes[remap[i]]);
               CGM::swap( remap[i], remap[pivot] );
               CGM::swap( _ids[i], _ids[pivot++] );
            }
            else
            {
               rightNodeBox |= (*bboxes[remap[i]]);
            }
         }

         // Construct node.
         ++depth;
         // No left node.
         if( pivot == begin )
         {
            uint childNode = uint(_nodes.size());

            // Current node.
            _nodes[node]._index    = (childNode << 3) + 4 + axis;
            _nodes[node]._plane[0] = rightNodeBox.min( axis );
            _nodes[node]._plane[1] = rightNodeBox.max( axis );

            // Create nodes.
            _nodes.pushBack( Node() );

            // Child node.
            node     = childNode;
            nodeBox  = rightNodeBox;
            splitBox.slab( axis )(0) = CGM::max( splitPlane, rightNodeBox.min( axis ) );
         }
         // No right node.
         else if( pivot == end )
         {
            uint childNode = uint(_nodes.size());

            // Current node.
            _nodes[node]._index    = (childNode << 3) + 4 + axis;
            _nodes[node]._plane[0] = leftNodeBox.min( axis );
            _nodes[node]._plane[1] = leftNodeBox.max( axis );

            // Create nodes.
            _nodes.pushBack( Node() );

            // Child node.
            node     = childNode;
            nodeBox  = leftNodeBox;
            splitBox.slab( axis )(1) = CGM::min( splitPlane, leftNodeBox.max( axis ) );
         }
         // Left and right node.
         else
         {
            uint leftNode = uint(_nodes.size());

            // Current node.
            _nodes[node]._index    = (leftNode << 3) + axis;
            _nodes[node]._plane[0] = leftNodeBox.max( axis );
            _nodes[node]._plane[1] = rightNodeBox.min( axis );

            // Create nodes.
            _nodes.pushBack( Node() );
            _nodes.pushBack( Node() );

            // Right node.
            AABBoxf rsplitBox( splitBox );
            rsplitBox.slab( axis )(0) = splitPlane;
            stack[stackID++].set( leftNode+1, pivot, end, depth, rightNodeBox, rsplitBox );

            // Left node.
            node     = leftNode;
            end      = pivot;
            nodeBox  = leftNodeBox;
            splitBox.slab( axis )(1) = splitPlane;
         }
      }
   }

   DBG( double time = timer.elapsed() );
   DBG_MSG( os_bih, "...finish in " << time << " sec." );
   DBG_MSG( os_bih, "# of nodes: " << _nodes.size() );
   DBG_MSG( os_bih, "# of indices: " << _ids.size() );
   DBG_MSG( os_bih, "max primitives in a leaf: " << maxPrim );
   DBG_MSG( os_bih, "max depth: " << maxD << " " << maxDepth );
}


//------------------------------------------------------------------------------
//!
bool
BIH::trace( const Rayf& ray, Hit& hit, IntersectFunc intersect, void* data ) const
{
   DBG_BLOCK( os_bih_trace, "BIH::trace(" << ray.origin() << ray.direction() << " hit: t=" << hit._t << " id=" << hit._id << ")" );
   if( _nodes.empty() )
   {
      return false;
   }

   Vec3f invDir = ray.direction().getInversed(); 

   // Compute traversal order.
   uint order[3];
   order[0] = invDir(0) >= 0.0f ? 0 : 1;
   order[1] = invDir(1) >= 0.0f ? 0 : 1;
   order[2] = invDir(2) >= 0.0f ? 0 : 1;

   float tmin  = 0.0f;
   float tmax  = hit._t;
   bool impact = false;


   // Traverse tree.
   Vector<TraversalStackNode> stack( _maxDepth );
   uint stackID = 0;
   const Node* node = &_nodes[0];

   while( 1 )
   {
      DBG_MSG( os_bih_trace, "Visiting " << *node );
      if( node->isInteriorNode() )
      {
         uint axis     = node->axis();
         float tplane0 = ( node->_plane[order[axis]] - ray.origin()(axis)) * invDir(axis);
         float tplane1 = ( node->_plane[1-order[axis]] - ray.origin()(axis)) * invDir(axis);

         // Clip node.
         if( node->isClipNode() )
         {
            // FIXME: we could probably do better (not traversing this node).
            node = &_nodes[node->index()];
            tmin = CGM::max( tmin, tplane0 );
            tmax = CGM::min( tmax, tplane1 );
            continue;
         }

         bool traverse0 = tmin < tplane0;
         bool traverse1 = tmax > tplane1;

         if( traverse0 )
         {
            if( traverse1 )
            {
               stack[stackID++].set(
                  &_nodes[node->index() + 1-order[axis]],
                  CGM::max( tmin, tplane1 ),
                  tmax
               );
            }
            node = &_nodes[node->index() + order[axis]];
            tmax = CGM::min( tmax, tplane0 );
         }
         else
         {
            if( traverse1 )
            {
               node = &_nodes[node->index() + 1-order[axis]];
               tmin = CGM::max( tmin, tplane1 );
            }
            else
            {
               // Unstack.
               do
               {
                  if( stackID == 0 )
                  {
                     return impact;
                  }
                  stack[--stackID].get( node, tmin, tmax );
                  tmax = CGM::min( tmax, hit._t );
               } while( tmin > tmax );
            }
         }

      }
      else
      {
         // We are in a leaf node.
         // Intersects all primitives in it.
         uint numElems = node->_numElements;
         uint id       = node->index();
         for( uint i = 0; i < numElems; ++i, ++id )
         {
            if( intersect( ray, _ids[id], hit._t, data ) )
            {
               impact  = true;
               hit._id = _ids[id];
            }
         }

         // Unstack.
         do
         {
            if( stackID == 0 )
            {
               return impact;
            }
            stack[--stackID].get( node, tmin, tmax );
            tmax = CGM::min( tmax, hit._t );
         } while( tmin > tmax );
      }
   }
}

//------------------------------------------------------------------------------
//!
inline void
visitSplitNode( const BIH::Node& node, const Vec2f& range, Vector<uint>& stack )
{
   DBG_BLOCK( os_bih, "visitSplitNode("
                << node << ", "
                << range << ", "
                << stack.size() << " elems in stack"
                << ")" );
   if( range(0) <= node._plane[0] )
   {
      if( node._plane[1] <= range(1) )
      {
         DBG_MSG( os_bih, "Left and right test pass, pushing " << node.index() << " and next" );
         // Visit left node, then right
         stack.pushBack( node.index() + 1 );
         stack.pushBack( node.index() );
      }
      else
      {
         DBG_MSG( os_bih, "Left test passes, pushing " << node.index() );
         // Visit left node only
         stack.pushBack( node.index() );
      }
   }
   else
   {
      if( node._plane[1] <= range(1) )
      {
         DBG_MSG( os_bih, "Right test passes, pushing " << node.index() + 1 );
         // Visit right node only
         stack.pushBack( node.index() + 1 );
      }
   }
}

//------------------------------------------------------------------------------
//!
inline void
visitClipNode( const BIH::Node& node, const Vec2f& range, Vector<uint>& stack )
{
   DBG_BLOCK( os_bih, "visitSplitNode("
                << node << ", "
                << range << ", "
                << stack.size() << " elems in stack"
                << ")" );
   if( node._plane[0] <= range(1) && range(0) <= node._plane[1] )
   {
      DBG_MSG( os_bih, "Test passes" );
      stack.pushBack( node.index() );
   }
}

//------------------------------------------------------------------------------
//!
inline void
visitLeafNode( const BIH::Node& node, const Vector<uint>& ids, Vector<uint>& dst )
{
   DBG_BLOCK( os_bih, "visitLeafNode("
                << node
                << ")" );
   uint index = node.index();
   DBG_MSG( os_bih, "Adding [" << node.index() << ", " << node.index() + node._numElements - 1 << "]" );
   for( uint i = 0; i < node._numElements; ++i )
   {
      dst.pushBack( ids[index + i] );
   }
}

//------------------------------------------------------------------------------
//!
bool
BIH::findElementsInside( const AABBoxf& box, Vector<uint>& dst ) const
{
   DBG_BLOCK( os_bih, "BIH::findElementsInside(" << box << ")" );
   DBG( print( os_bih.stream(), ">> " ) );

   if( _nodes.empty() )
   {
      return false;
   }

   Vector<uint> stack;

   stack.pushBack( 0 );
   while( !stack.empty() )
   {
      DBG_MSG( os_bih, "Node #" << stack.back() );
      const Node& node = _nodes[stack.back()];
      stack.popBack();

      switch( node.flags() )
      {
         case Node::BIH_NODE_SPLIT_X:
            visitSplitNode( node, box.slabX(), stack );
            break;
         case Node::BIH_NODE_SPLIT_Y:
            visitSplitNode( node, box.slabY(), stack );
            break;
         case Node::BIH_NODE_SPLIT_Z:
            visitSplitNode( node, box.slabZ(), stack );
            break;
         case Node::BIH_NODE_LEAF:
            visitLeafNode( node, _ids, dst );
            break;
         case Node::BIH_NODE_CLIP_X:
            visitClipNode( node, box.slabX(), stack );
            break;
         case Node::BIH_NODE_CLIP_Y:
            visitClipNode( node, box.slabY(), stack );
            break;
         case Node::BIH_NODE_CLIP_Z:
            visitClipNode( node, box.slabZ(), stack );
            break;
      }
   }

   return !dst.empty();
}

//------------------------------------------------------------------------------
//!
inline void
visitLeafNode( 
   const BIH::Node&     node, 
   const Vector<uint>&  ids, 
   Vector<uint>&        dst,
   const AABBoxf&       box, 
   BIH::InsideFunc      inside, 
   void*                data
)
{
   DBG_BLOCK( os_bih, "visitLeafNode(" << node << ")" );
   uint index = node.index();
   DBG_MSG( os_bih, "Adding [" << node.index() << ", " << node.index() + node._numElements - 1 << "]" );
   for( uint i = 0; i < node._numElements; ++i )
   {
      const uint id = index + i;
      if( inside(box, ids[id], data) )
      {
         dst.pushBack( ids[id] );
      }
   }
}

//------------------------------------------------------------------------------
//!
bool
BIH::findElementsInside( const AABBoxf& box, Vector<uint>& dst, InsideFunc inside, void* data ) const
{
   DBG_BLOCK( os_bih, "BIH::findElementsInside(" << box << ")" );
   DBG( print( os_bih.stream(), ">> " ) );

   if( _nodes.empty() )
   {
      return false;
   }

   Vector<uint> stack;

   stack.pushBack( 0 );
   while( !stack.empty() )
   {
      DBG_MSG( os_bih, "Node #" << stack.back() );
      const Node& node = _nodes[stack.back()];
      stack.popBack();

      switch( node.flags() )
      {
         case Node::BIH_NODE_SPLIT_X:
            visitSplitNode( node, box.slabX(), stack );
            break;
         case Node::BIH_NODE_SPLIT_Y:
            visitSplitNode( node, box.slabY(), stack );
            break;
         case Node::BIH_NODE_SPLIT_Z:
            visitSplitNode( node, box.slabZ(), stack );
            break;
         case Node::BIH_NODE_LEAF:
            visitLeafNode( node, _ids, dst, box, inside, data );
            break;
         case Node::BIH_NODE_CLIP_X:
            visitClipNode( node, box.slabX(), stack );
            break;
         case Node::BIH_NODE_CLIP_Y:
            visitClipNode( node, box.slabY(), stack );
            break;
         case Node::BIH_NODE_CLIP_Z:
            visitClipNode( node, box.slabZ(), stack );
            break;
      }
   }

   return !dst.empty();
}

//------------------------------------------------------------------------------
//!
void
BIH::searchElement( uint elem ) const
{
   uint n = 0;
   Vector<uint> stack( _maxDepth );
   Vector<uint> pstack;
   uint stackID = 0;

   printf( "Searching for element: %d\n", elem );

   while( 1 )
   {
      pstack.pushBack(n);
      if( _nodes[n].isInteriorNode() )
      {
         if( _nodes[n].isClipNode() )
         {
            n = _nodes[n].index();
            continue;
         }
         stack[stackID++] = _nodes[n].index()+1;
         n = _nodes[n].index();
      }
      else
      {
         uint numElems = _nodes[n]._numElements;
         uint id       = _nodes[n].index();
         for( uint i = 0; i < numElems; ++i, ++id )
         {
            if( _ids[id] == elem )
            {
               for( uint s = 0; s < pstack.size(); ++s )
               {
                  printf( "NODE: %d type: %d 0: %f 1: %f l: %d r: %d\n",
                     pstack[s],
                     _nodes[pstack[s]]._index & 7,
                     _nodes[pstack[s]]._plane[0],
                     _nodes[pstack[s]]._plane[1],
                     _nodes[pstack[s]].index(),
                     _nodes[pstack[s]].index()+1
                  );
               }
               return;
            }
         }
         n = stack[--stackID];
         while( _nodes[pstack.back()].index()+1 != n )
         {
            pstack.popBack();
         }
      }
   }
}

//------------------------------------------------------------------------------
//!
void
BIH::print( TextStream& os, const String& prefix ) const
{
   os << prefix << "BIH{" << '\n';
   os << prefix << "maxDepth=" << _maxDepth << '\n';
   os << prefix << _nodes.size() << " nodes:" << '\n';
   for( uint i = 0; i < _nodes.size(); ++i )
   {
      os << prefix << "[" << i << "]: " << _nodes[i] << '\n';
   }
   os << prefix << _ids.size() << " ids:" << '\n';
   for( uint i = 0; i < _ids.size(); ++i )
   {
      os << prefix << "[" << i << "]: " << _ids[i] << '\n';
   }
   os << prefix << "}" << flush;
}

NAMESPACE_END
