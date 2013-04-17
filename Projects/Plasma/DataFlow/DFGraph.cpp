/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/DataFlow/DFGraph.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

const char* _graph_str = "dfgraph";

//------------------------------------------------------------------------------
//!
void addNodeToGrid( DFNode* node, DFGraph::Grid& grid )
{
   Vec2i p   = node->position();
   int width = node->width();
   for( int i = 0; i < width; ++i, ++p.x )
   {
      grid[p] = node;
   }
}

//------------------------------------------------------------------------------
//!
void removeNodeFromGrid( DFNode* node, DFGraph::Grid& grid )
{
   int w   = node->width();
   Vec2i p = node->position();
   for( int i = 0; i < w; ++i, ++p.x )
   {
      grid.erase(p);
   }
}

//------------------------------------------------------------------------------
//!
bool isOccupied( const Vec2i& pos, int width, const DFGraph::Grid& grid )
{
   Vec2i p = pos;
   for( int i = 0; i < width; ++i, ++p.x )
   {
      if( grid.has(p) ) return true;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
void updateNodes( const Vec2i& pos, int width, DFGraph::Grid& grid, Set<DFNode*>& update )
{
   auto gend     = grid.end();
   DFNode* cnode = nullptr;
   Vec2i p       = pos;
   for( int i = 0; i < width; ++i, ++p.x )
   {
      auto fit      = grid.find( p );
      DFNode* fnode = fit != gend ? fit.data() : nullptr;
      // Have we found a new input node?
      if( fnode && fnode != cnode )
      {
         cnode = fnode;
         update.add( cnode );
      }
   }
}

//------------------------------------------------------------------------------
//!
void updateOutput( DFNode* node, Set<DFNode*>& update )
{
   DFOutput* output = node->output();
   uint n = output->numInputs();
   for( uint i = 0; i < n; ++i )
   {
      update.add( output->inputs()[i]->node() );
   }
}

UNNAMESPACE_END


NAMESPACE_BEGIN

/*==============================================================================
   CLASS DFGraph
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFGraph::DFGraph(): _lockUpdate(0), _output(nullptr)
{
   CHECK( &(_messenger.graph()) == this );
}

//------------------------------------------------------------------------------
//!
bool DFGraph::addNode( DFNode* node, const Vec2i& pos, int width )
{
   // Test if valid.
   if( isOccupied( pos, width, _grid ) ) return false;

   // Node is valid.
   node->_graph    = this;
   node->_position = pos;
   node->_width    = width;
   _nodes.pushBack( node );

   if( _output == nullptr )  _output = node;

   // Add node position to the grid.
   addNodeToGrid( node, _grid );

   // Add this node to update.
   _update.add( node );
   // Add all possible output nodes to update.
   updateNodes( pos+Vec2i(0,-1), width, _grid, _update );

   updateConnections();

   return true;
}

//------------------------------------------------------------------------------
//!
void DFGraph::removeNode( DFNode* node )
{
   // Send remove message.
   msg().remove( node );

   // Remove any remaining messenger callback.
   msg().removeAll( node );

   // Remove from grid.
   removeNodeFromGrid( node, _grid );

   // Disconnect all inputs.
   int numInputs = node->numInputs();
   for( int i = 0; i < numInputs; ++i )
   {
      node->input(i)->disconnect();
   }
   node->_graph = nullptr;

   // Remove this node from update.
   _update.remove( node );
   // Add all outputs to update.
   updateOutput( node, _update );

   _nodes.removeSwap( node );
   if( _output == node ) _output = nullptr;

   updateConnections();
}

//------------------------------------------------------------------------------
//!
bool DFGraph::moveNodes( const Vector<DFNode*>& nodes, const Vec2i& deltaPos )
{
   Vec2i del = deltaPos;

   // Remove nodes from grid.
   for( auto it = nodes.begin(); it != nodes.end(); ++it )
   {
      removeNodeFromGrid( *it, _grid );
   }

   // Check if move is valid.
   for( auto it = nodes.begin(); it != nodes.end(); ++it )
   {
      if( isOccupied( (*it)->position()+deltaPos, (*it)->width() , _grid ) )
      {
         del = Vec2i(0);
         break;
      }
   }

   // Add nodes to the grid.
   for( auto it = nodes.begin(); it != nodes.end(); ++it )
   {
      (*it)->_position += del;
      addNodeToGrid( *it, _grid );
   }

   if( del.x == 0 && del.y == 0 ) return false;

   // Nodes to update.
   auto gend = _grid.end();
   for( auto it = nodes.begin(); it != nodes.end(); ++it )
   {
      // Add the current node to update.
      _update.add( *it );
      // Add all possible output nodes to update.
      updateNodes( (*it)->position()+Vec2i(0,-1), (*it)->width(), _grid, _update );
      // Add all current output to update.
      updateOutput( *it, _update );
   }

   updateConnections();
   return true;
}

//------------------------------------------------------------------------------
//!
bool DFGraph::resizeNodes( const Vector<DFNode*>& nodes, int deltaWidth )
{
   for( auto it = nodes.begin(); it != nodes.end(); ++it )
   {
      int width = CGM::max( (*it)->_width + deltaWidth, 1 );

      if( width > (*it)->width() )
      {
         // Check maximum size.
         for( int w = (*it)->width(); w < width; ++w )
         {
            Vec2i p = (*it)->position() + Vec2i(w,0);
            if( _grid.has(p) ) break;
            _grid[p] = (*it);
            ++(*it)->_width;
         }
      }
      else
      {
         removeNodeFromGrid( *it, _grid );
         (*it)->_width = width;
         addNodeToGrid( *it, _grid );
      }
   }

   // Nodes to update.
   auto gend = _grid.end();
   for( auto it = nodes.begin(); it != nodes.end(); ++it )
   {
      // Add the current node to update.
      _update.add( *it );
      // Add all possible output nodes to update.
      updateNodes( (*it)->position()+Vec2i(0,-1), (*it)->width(), _grid, _update );
      // Add all current output to update.
      updateOutput( *it, _update );
   }

   updateConnections();
   return true;
}

//------------------------------------------------------------------------------
//!
bool DFGraph::leftResizeNodes( const Vector<DFNode*>& nodes, int deltaWidth )
{
   for( auto it = nodes.begin(); it != nodes.end(); ++it )
   {
      int width = CGM::max( (*it)->_width + deltaWidth, 1 );

      if( width > (*it)->width() )
      {
         // Check maximum size.
         for( int w = (*it)->width(); w < width; ++w )
         {
            Vec2i p = (*it)->position() - Vec2i(1,0);
            if( _grid.has(p) ) break;
            _grid[p] = (*it);
            ++(*it)->_width;
            --(*it)->_position.x;
         }
      }
      else
      {
         removeNodeFromGrid( *it, _grid );
         (*it)->_position.x -=  width-(*it)->_width;
         (*it)->_width = width;
         addNodeToGrid( *it, _grid );
      }
   }

   // Nodes to update.
   auto gend = _grid.end();
   for( auto it = nodes.begin(); it != nodes.end(); ++it )
   {
      // Add the current node to update.
      _update.add( *it );
      // Add all possible output nodes to update.
      updateNodes( (*it)->position()+Vec2i(0,-1), (*it)->width(), _grid, _update );
      // Add all current output to update.
      updateOutput( *it, _update );
   }

   updateConnections();
   return true;
}

//-----------------------------------------------------------------------------
//! Searches for an empty spot of size width inside [corner, corner+range] inclusively.
//! The variable corner defines a starting position, while the sign of range
//! defines the direction of the search.
//! The searchVerticalFirst flag defines the direction of the primary search axis.
//! If no empty spot is possible inside the rectangular region, the first empty spot
//! towards the primary axis is returned.
Vec2i
DFGraph::findSpot( int width, const Vec2i& corner, const Vec2i& range, bool searchVerticalFirst ) const
{
   int majAxis = searchVerticalFirst ? 1 : 0;
   int minAxis = 1 - majAxis;
   Vec2i     last = corner + range;
   Vec2i      dir = Vec2i( CGM::copySign(1, range.x), CGM::copySign(1, range.y) );
   Vec2i      pos;
   for( pos(minAxis) = corner(minAxis); pos(minAxis) != last(minAxis); pos(minAxis) += dir(minAxis) )
   {
      for( pos(majAxis) = corner(majAxis); pos(majAxis) != last(majAxis); pos(majAxis) += dir(majAxis) )
      {
         if( !isOccupied( pos, width, _grid ) )  return pos;
      }
   }

   // Fallback to primary axis.
   pos(minAxis) = corner(minAxis);
   int sanity = 0;
   for( pos(majAxis) = corner(majAxis); sanity < (1<<16); pos(majAxis) += dir(majAxis), ++sanity )
   {
      if( !isOccupied( pos, width, _grid ) )  return pos;
   }

   CHECK( false ); // The above loop should always find something (unless we have 2^31 elements in a row).
   return Vec2i( CGConsti::max() );
}

//------------------------------------------------------------------------------
//!
AARecti
DFGraph::getBoundingBox() const
{
   AARecti bbox = AARecti::empty();
   for( auto it = _nodes.begin(); it != _nodes.end(); ++it )
   {
      const DFNode& node = *(*it);
      bbox |= node.position();
      bbox |= node.position() + Vec2i( node.width(), 1 );
   }
   return bbox;
}

//------------------------------------------------------------------------------
//!
void DFGraph::output( DFNode* node )
{
   // New output?
   if( node == _output ) return;

   _output = node;
   msg().update();
}

//------------------------------------------------------------------------------
//!
DFNode* DFGraph::getNode( const Vec2i& pos ) const
{
   auto it = _grid.find( pos );
   return it != _grid.end() ? it.data() : nullptr;
}

//------------------------------------------------------------------------------
//!
void DFGraph::invalidate( DFNode* node )
{
   msg().update( node );
   // FIXME: for now since there is no caching we just call an update.
   msg().update();
}

//------------------------------------------------------------------------------
//!
void DFGraph::updateConnections()
{
   // Can we update?
   if( _lockUpdate > 0 ) return;

   Vector<DFNode*> inputs;
   auto gend = _grid.end();
   for( auto it = _update.begin(); it != _update.end(); ++it )
   {
      DFNode* node = *it;
      // Create list of input nodes for the current node.
      inputs.clear();
      DFNode* cnode = nullptr;
      int w         = node->width();
      Vec2i p       = node->position() + Vec2i(0,1);
      for( int i = 0; i < w; ++i, ++p.x )
      {
         auto fit      = _grid.find( p );
         DFNode* fnode = fit != gend ? fit.data() : nullptr;
         // Have we found a new input node?
         if( fnode != cnode )
         {
            cnode = fnode;
            inputs.pushBack( cnode );
         }
      }
      // Disconnect all inputs.
      int numInputs = node->numInputs();
      for( int i = 0; i < numInputs; ++i )
      {
         node->input(i)->disconnect();
      }

      // Connects sockets (input & output).
      int inputid = 0;
      for( auto iit = inputs.begin(); iit != inputs.end(); ++iit )
      {
         DFInput* input = node->input( inputid );
         // No more valid input?
         if( !input ) break;

         if( (*iit) != nullptr )
         {
            DFOutput* output = (*iit)->output();
            if( input->type() == output->type() )
            {
               input->connect( output );
               output->connect( input );
               if( !input->isMulti() ) ++inputid;
            }
         }
         else
         {
            if( input->isMulti() && input->isConnected() ) ++inputid;
         }
      }
   }
   _update.clear();
   msg().update();
}

//------------------------------------------------------------------------------
//!
const char*
DFGraph::meta() const
{
   return _graph_str;
}

//------------------------------------------------------------------------------
//!
const char*
DFGraph::staticMeta()
{
   return _graph_str;
}

//-----------------------------------------------------------------------------
//!
bool
DFGraph::dump( TextStream& os, StreamIndent& indent ) const
{
   bool      ok = true;
   size_t     n = _nodes.size();
   size_t outID = n;
   for( size_t i = 0; i < n; ++i )
   {
      const DFNode* node = _nodes[i].ptr();
      if( node == output() ) outID = i;
      os << indent << "local n" << i << " = ";
      ok &= node->dump( os, indent );
   }
   if( outID != n )
   {
      os << indent << "output( n" << outID << " )" << nl;
   }
   return ok;
}


NAMESPACE_END
