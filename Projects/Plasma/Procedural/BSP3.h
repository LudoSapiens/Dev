/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_BSP3_H
#define PLASMA_BSP3_H

#include <Plasma/StdDefs.h>

#include <Plasma/Procedural/BoundaryPolygon.h>

#include <Base/ADT/Vector.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS BSP3
==============================================================================*/
class BSP3Node
{

public: 

   /*----- methods -----*/

   BSP3Node(): _front(0), _back(0) {}
      
   BSP3Node( const Planef& plane ) :
      _plane( plane ), _front(0), _back(0)
   {}
  
   ~BSP3Node()
   {
      clear( _front );
      clear( _back );
   }
   
   inline const Planef& plane() const                  { return _plane; }
   inline void add( const RCP<BoundaryPolygon>& poly ) { _polygons.pushBack( poly ); }
   inline uint numPolygons() const                     { return uint(_polygons.size()); }
   inline BoundaryPolygon* polygon( uint i ) const     { return _polygons[i].ptr(); }
   
   inline BSP3Node* front() const                      { return _front; }
   inline void front( BSP3Node* node )
   {
      if( node != _front )
      {
         clear( _front );
         _front = node;
      }
   }
   
   inline BSP3Node* back() const                       { return _back; }
   inline void back( BSP3Node* node )
   {
      if( node != _back )
      {
         clear( _back );
         _back = node;
      }
   }
   
   inline void frontIn()                           { clear( _front ); _front = &_in; }
   inline void frontOut()                          { clear( _front ); _front = &_out; }
   inline void backIn()                            { clear( _back );  _back = &_in; }
   inline void backOut()                           { clear( _back );  _back = &_out; }
   inline bool isLeaf() const                      { return this == &_in || this == &_out; }
   inline bool in() const                          { return this == &_in; }
   inline bool out() const                         { return this == &_out; }
   
private:   

   friend class BSP3;
      
   /*----- methods -----*/
   
   void clear( BSP3Node* node )
   {
      if( node != &_in && node != &_out ) delete node;
   }
   
   void clearPolygons()
   {
      _polygons.clear();
   }

   /*----- static data members -----*/

   static BSP3Node _in;
   static BSP3Node _out;

   /*----- data members -----*/

   Planef                         _plane;
   BSP3Node*                      _front;
   BSP3Node*                      _back;
   Vector< RCP<BoundaryPolygon> > _polygons;
};

/*==============================================================================
   CLASS BSP3
==============================================================================*/
class BSP3
{

public: 

   /*----- types and enumerations ----*/

   enum Operation {
      UNION,
      INTERSECTION,
      DIFFERENCE
   };

   /*----- methods -----*/

   PLASMA_DLL_API BSP3();
   PLASMA_DLL_API ~BSP3();

   // Creation.
   PLASMA_DLL_API void build( Vector< RCP<BoundaryPolygon> >& );
   PLASMA_DLL_API void add( Vector< RCP<BoundaryPolygon> >& );
   PLASMA_DLL_API void remove( Vector< RCP<BoundaryPolygon> >& );
   PLASMA_DLL_API void intersect( Vector< RCP<BoundaryPolygon> >& );

   // Boundary.
   PLASMA_DLL_API void computeBoundary( Vector< RCP<BoundaryPolygon> >&, bool reduced = true );

   // Queries.
   inline bool isEmpty() const;
   
   // Reset.
   inline void clear();
   
   // Debugging.
   PLASMA_DLL_API void dump() const;

private: 
   
   /*----- methods -----*/

   BSP3Node* buildNode( const Vector< RCP<BoundaryPolygon> >& );
   BSP3Node* incrementalOp( Operation, BSP3Node* node, const Vector< RCP<BoundaryPolygon> >& );
   
   void updateBoundary( BSP3Node* node );
   void retrieveBoundary( BSP3Node* node, Vector< RCP<BoundaryPolygon> >& );
   void clipBoundary( BSP3Node* node, Vector< RCP<BoundaryPolygon> >&, int );
   void reduceBoundary( Vector< RCP<BoundaryPolygon> >& );

   int inclusion( const BSP3Node*, const Vec3f& pt );
   inline void clearOpTree();

   void dump( BSP3Node* node ) const;


   /*----- data members -----*/

   BSP3Node* _root;
   BSP3Node* _oproot;
   float     _precision;
   float     _epsilon;
};

//------------------------------------------------------------------------------
//!
inline bool
BSP3::isEmpty() const
{
   return _root == 0;
}

//------------------------------------------------------------------------------
//!
inline void
BSP3::clear()
{
   if( _root != &BSP3Node::_in && _root != &BSP3Node::_out ) delete _root;
   _root = &BSP3Node::_out;
}

//------------------------------------------------------------------------------
//!
inline void
BSP3::clearOpTree()
{
   if( _oproot != &BSP3Node::_in && _oproot != &BSP3Node::_out ) delete _oproot;
   _oproot = &BSP3Node::_out;
}

NAMESPACE_END

#endif
