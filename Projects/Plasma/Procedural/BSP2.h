/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_BSP2_H
#define PLASMA_BSP2_H

#include <Plasma/StdDefs.h>

#include <Plasma/Procedural/BoundaryPolygon.h>

#include <Base/ADT/Vector.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS Edgef
==============================================================================*/

class Edgef
{
public:

   /*----- methods -----*/

   Edgef(): _valid(1) {}
   Edgef( const Vec3f& v0, const Vec3f& v1, const Vec3d& normal ) :
      _normal( normal ), _valid(1)
   {
      _vertices[0] = v0;
      _vertices[1] = v1;
   }

   inline void set( const Vec3f& v0, const Vec3f& v1, const Vec3d& normal )
   {
      _normal      = normal;
      _vertices[0] = v0;
      _vertices[1] = v1;
   }

   inline const Vec3f& vertex( uint i ) const { return _vertices[i]; }
   inline const Vec3d& normal() const         { return _normal; }
   
   inline void inverse() 
   {
      CGM::swap( _vertices[0], _vertices[1] );
      _normal = -_normal;
   }

   inline float sqrLength() const { return CGM::sqrLength(_vertices[0]-_vertices[1]); }
   inline Vec3f center() const    { return (_vertices[0]+_vertices[1])*0.5f; }
   inline int valid() const       { return _valid; }
   inline void invalidate()       { _valid = 0; }
   inline bool isInside( const Vec3f& p ) const
   {
      return dot( p-_vertices[0], p-_vertices[1] ) < 0.0f;
   }

private:

   /*----- members -----*/

   Vec3f _vertices[2];
   Vec3d _normal;
   int   _valid;
};

/*==============================================================================
   CLASS BSP2
==============================================================================*/
class BSP2Node
{

public: 

   /*----- methods -----*/

   BSP2Node() :
      _front(0), _back(0)
   {}
      
   BSP2Node( const Planed& plane ) :
      _plane( plane ), _front(0), _back(0)
   {}
  
   ~BSP2Node()
   {
      clear( _front );
      clear( _back );
   }
   
   inline const Planed& plane() const       { return _plane; }
   inline void add( const Edgef& edge )     { _edges.pushBack( edge ); }
   inline uint numEdges() const             { return uint(_edges.size()); }
   inline const Edgef& edge( uint i ) const { return _edges[i]; }
   
   inline BSP2Node* front() const           { return _front; }
   inline void front( BSP2Node* node )
   {
      if( node != _front )
      {
         clear( _front );
         _front = node;
      }
   }
   
   inline BSP2Node* back() const            { return _back; }
   inline void back( BSP2Node* node )
   {
      if( node != _back )
      {
         clear( _back );
         _back = node;
      }
   }
   
   inline void frontIn()      { clear( _front ); _front = &_in; }
   inline void frontOut()     { clear( _front ); _front = &_out; }
   inline void backIn()       { clear( _back ); _back = &_in; }
   inline void backOut()      { clear( _back ); _back = &_out; }
   inline bool isLeaf() const { return this == &_in || this == &_out; }
   inline bool in() const     { return this == &_in; }
   inline bool out() const    { return this == &_out; }

   inline void backID( const ConstString& id )  { _bid = id; }
   inline void frontID( const ConstString& id ) { _fid = id; }
   inline const ConstString& backID() const     { return _bid; }
   inline const ConstString& frontID() const    { return _fid; }
   
private:   

   friend class BSP2;
      
   /*----- methods -----*/
   
   void clear( BSP2Node* node )
   {
      if( node != &_in && node != &_out ) delete node;
   }
   
   void clearEdges() { _edges.clear(); }

   /*----- static data members -----*/

   static BSP2Node _in;
   static BSP2Node _out;

   /*----- data members -----*/

   Planed         _plane;
   BSP2Node*      _front;
   BSP2Node*      _back;
   ConstString    _fid;
   ConstString    _bid;
   Vector<Edgef>  _edges;
};

#ifdef UNION
#undef UNION
#endif
#ifdef INTERSECTION
#undef INTERSECTION
#endif
#ifdef DIFFERENCE
#undef DIFFERENCE
#endif

/*==============================================================================
   CLASS BSP2
==============================================================================*/
class BSP2
{

public: 

   /*----- types and enumerations ----*/

   enum Operation {
      UNION,
      INTERSECTION,
      DIFFERENCE
   };

   /*----- methods -----*/

   PLASMA_DLL_API BSP2();
   PLASMA_DLL_API ~BSP2();

   inline void back( float b ) { _back = b; }

   // Creation.
   PLASMA_DLL_API void build( const BoundaryPolygon& );
   PLASMA_DLL_API void add( const BoundaryPolygon& );
   PLASMA_DLL_API void remove( const BoundaryPolygon& );
   PLASMA_DLL_API void intersect( const BoundaryPolygon& );

   PLASMA_DLL_API void removeProj( BoundaryPolygon& );
   
   // Boundary.
   PLASMA_DLL_API void computePolygons( Vector< RCP<BoundaryPolygon> >& );
   PLASMA_DLL_API void computeConvexPolygons( Vector< RCP<BoundaryPolygon> >& ) const;

   // Queries.
   inline bool isEmpty() const;
   float computeArea() const;
   
   // Reset.
   inline void clear();

   // Debugging.
   PLASMA_DLL_API void dump() const;
   
private: 
   
   /*----- methods -----*/

   BSP2Node* buildNode( 
      const Vector<Edgef>&, 
      const ConstString& inID, 
      const ConstString& outID
   );
   BSP2Node* incrementalOp( 
      Operation            op, 
      BSP2Node*            node, 
      const Vector<Edgef>& edges, 
      const ConstString&   inID, 
      const ConstString&   outID
   );
   
   void updateBoundary( BSP2Node* node );
   void retrieveBoundary( BSP2Node* node, Vector<Edgef>& );
   void clipBoundary( BSP2Node* node, Vector<Edgef>&, int );
   void buildPolygons( Vector<Edgef>& edges, Vector< RCP<BoundaryPolygon> >& );

   void clipPolygon( 
      BSP2Node*, 
      const RCP<BoundaryPolygon>&,
      Vector< RCP<BoundaryPolygon> >&
   ) const;

   void dump( BSP2Node* node ) const;

   /*----- data members -----*/
   
   BSP2Node*   _root;
   ConstString _id;
   float       _back;
   float       _precision;
   double      _epsilon;
   Planed      _plane;
   uint        _mainAxis;
};

//------------------------------------------------------------------------------
//!
inline bool
BSP2::isEmpty() const
{
   return _root == 0;
}

//------------------------------------------------------------------------------
//!
inline void
BSP2::clear()
{
   if( _root != &BSP2Node::_in && _root != &BSP2Node::_out )
   {
      delete _root;
   }
   _root = &BSP2Node::_out;
}

NAMESPACE_END

#endif
