/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef CGMATH_GRID_H
#define CGMATH_GRID_H

#include <CGMath/StdDefs.h>

#include <Base/ADT/MemoryPool.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS Grid
==============================================================================*/

template< typename T >
class Grid
{
public:

   /*----- classes -----*/

   struct Link {
      T     _obj;
      Link* _next;
   };

   /*----- methods -----*/

   Grid( uint size, float cellSize );

   // Operations.
   void clear();
   void add( uint cell, const T& );
   void remove( uint cell, const T& );

   // Queries.
   Vec3i cellCoord( const Vec3f& pos ) const;
   uint cellID( const Vec3i& coord ) const;
   uint cellID( const Vec3f& pos ) const;
   Link* cell( uint cellID ) const;
   Link* cell( const Vec3f& pos ) const;
   Link* cell( const Vec3i& coord ) const;

protected:

   /*----- members -----*/

   MemoryPool<Link> _links;
   Vector<Link*>    _cells;
   float            _size;
};

//------------------------------------------------------------------------------
//! 
template< typename T >
Grid<T>::Grid( uint size, float cellSize ): 
   _cells( size, 0 ), _size( cellSize )
{
}

//------------------------------------------------------------------------------
//! 
template< typename T > void
Grid<T>::clear()
{
   _links->clear();
   for( uint i = 0 ; i < _cells.size(); ++i )
   {
      _cells[i] = 0;
   }
}

//------------------------------------------------------------------------------
//! 
template< typename T > void
Grid<T>::add( uint cell, const T& obj )
{
   Link* l      = _links.alloc();
   l->_obj      = obj;
   l->_next     = _cells[cell];
   _cells[cell] = l;
}

//------------------------------------------------------------------------------
//! 
template< typename T > void
Grid<T>::remove( uint cell, const T& obj )
{
   Link* l = _cells[cell];

   if( !l ) return;
   if( l->_obj == obj )
   {
      _cells[cell] = l->_next;
      _links.free( l );
      return;
   }
   
   for( ; l->_next; l = l->_next )
   {
      if( l->_next->_obj == obj )
      {
         Link* n = l->_next;
         l->_next = n->_next;
         _links.free( n );
      }
   }
}

//------------------------------------------------------------------------------
//! 
template< typename T > Vec3i
Grid<T>::cellCoord( const Vec3f& pos ) const
{
   return Vec3i( CGM::floori(pos.x/_size), CGM::floori(pos.y/_size), CGM::floori(pos.z/_size) );
}

//------------------------------------------------------------------------------
//! 
template< typename T > uint
Grid<T>::cellID( const Vec3i& c ) const
{
   uint m1 = 0x8da6b343;
   uint m2 = 0xd8163841;
   uint m3 = 0xcb1ab31f;
   return (m1*c.x + m2*c.y + m3*c.z) % _cells.size();
}

//------------------------------------------------------------------------------
//! 
template< typename T > uint
Grid<T>::cellID( const Vec3f& pos ) const
{
   return cellID( cellCoord( pos ) );
}

//------------------------------------------------------------------------------
//! 
template< typename T > typename Grid<T>::Link*
Grid<T>::cell( uint cellID ) const
{
   return _cells[cellID];
}

//------------------------------------------------------------------------------
//! 
template< typename T > typename Grid<T>::Link*
Grid<T>::cell( const Vec3f& pos ) const
{
   return cell( cellID( pos ) );
}

//------------------------------------------------------------------------------
//! 
template< typename T > typename Grid<T>::Link*
Grid<T>::cell( const Vec3i& coord ) const
{
   return cell( cellID( coord ) );
}

NAMESPACE_END

#endif
