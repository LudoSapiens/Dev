/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_DFGRAPH_H
#define PLASMA_DFGRAPH_H

#include <Plasma/StdDefs.h>
#include <Plasma/DataFlow/DFMessenger.h>
#include <Plasma/DataFlow/DFNode.h>

#include <Fusion/VM/VM.h>

#include <CGMath/AARect.h>

#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>
#include <Base/ADT/Vector.h>
#include <Base/ADT/Set.h>
#include <Base/ADT/HashTable.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS DFGraph
==============================================================================*/

class DFGraph:
   public RCObject,
   public VMProxy
{
public:

   /*----- classes -----*/

   class UpdateLock;

   struct Vec2iHash
   {
      size_t operator()( const Vec2i& v ) const
      {
         uint m1 = 0x8da6b343;
         uint m2 = 0xd8163841;
         return m1*v.x + m2*v.y;
      }
   };

   /*----- types -----*/

   typedef Vector< RCP<DFNode> >              NodeContainer;
   typedef HashTable<Vec2i,DFNode*,Vec2iHash> Grid;

   /*----- methods -----*/

   PLASMA_DLL_API DFGraph();

   // Nodes.
   PLASMA_DLL_API bool addNode( DFNode*, const Vec2i& pos, int width );
   PLASMA_DLL_API void removeNode( DFNode* );
   PLASMA_DLL_API void output( DFNode* );
   PLASMA_DLL_API bool moveNodes( const Vector<DFNode*>& nodes, const Vec2i& deltaPos );
   PLASMA_DLL_API bool resizeNodes( const Vector<DFNode*>& nodes, int deltaWidth );
   PLASMA_DLL_API bool leftResizeNodes( const Vector<DFNode*>& nodes, int deltaWidth );
   PLASMA_DLL_API Vec2i findSpot( int width, const Vec2i& corner, const Vec2i& range, bool searchVerticalFirst = false ) const;
   PLASMA_DLL_API AARecti  getBoundingBox() const;

   PLASMA_DLL_API DFNode* getNode( const Vec2i& ) const;

   inline uint numNodes() const              { return uint(_nodes.size()); }
   inline const NodeContainer& nodes() const { return _nodes; }
   inline DFNode* node( uint i ) const       { return _nodes[i].ptr(); }
   inline DFNode* output() const             { return _output; }

   PLASMA_DLL_API void invalidate( DFNode* );

   // Messages.
   inline       DFMessenger&  msg()       { return _messenger; }
   inline const DFMessenger&  msg() const { return _messenger; }

   // VM.
   virtual const char* meta() const;
   static  const char* staticMeta();

   // Dumping.
   PLASMA_DLL_API bool  dump( TextStream& os, StreamIndent& indent ) const;

protected:

   /*----- friends -----*/

   friend class DFMessenger;
   friend class UpdateLock;

   /*----- methods -----*/

   void updateConnections();

   /*----- data members -----*/

   int            _lockUpdate;
   NodeContainer  _nodes;
   DFNode*        _output;
   Grid           _grid;
   Set<DFNode*>   _update;
   DFMessenger    _messenger;
};

/*==============================================================================
   CLASS UpdateLock
==============================================================================*/

class DFGraph::UpdateLock
{
public:

   /*----- methods -----*/

   UpdateLock( DFGraph* g ): _graph(g)
   {
      ++_graph->_lockUpdate;
   }

   ~UpdateLock()
   {
      --_graph->_lockUpdate;
      if( _graph->_lockUpdate == 0 ) _graph->updateConnections();
   }

protected:

   /*----- members -----*/

   DFGraph* _graph;
};

NAMESPACE_END

#endif
