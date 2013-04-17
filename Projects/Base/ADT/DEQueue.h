/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_DEQUEUE_H
#define BASE_DEQUEUE_H

#include <Base/StdDefs.h>

#include <Base/ADT/Vector.h>

#include <Base/Dbg/Defs.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS DEQueue
==============================================================================*/

//! DEQueue container class implemented as a circular array.

template< typename T >
class DEQueue:
   private Vector<T>
{

public:

   /*----- methods -----*/

   DEQueue( const size_t n = 8 );
   DEQueue( const DEQueue& );

   void  pushBack( const T& t );
   void  pushFront( const T& t );
   void  popBack();
   void  popFront();

         T&  front();
   const T&  front() const;

         T&  back();
   const T&  back() const;

   bool empty() const;
   size_t size() const;
   size_t capacity() const;

   const T&  peek( size_t i ) const;

   void  clear();

protected:

   size_t  idx( size_t v ) const;

   void  move( size_t from, size_t n, size_t to );
   void  moveRL( size_t from, size_t n, size_t to );
   void  grow();

   /*----- data members -----*/

   size_t _front;
   size_t _size;
};

//------------------------------------------------------------------------------
//!
template< typename T>
DEQueue<T>::DEQueue( const size_t n ) :
   Vector<T>( n ),
   _front(0),
   _size(0)
{
}

//------------------------------------------------------------------------------
//!
template< typename T >
DEQueue<T>::DEQueue( const DEQueue& q ):
   Vector<T>( q ),
   _front( q._front ),
   _size( q._size )
{
}

//------------------------------------------------------------------------------
//!
template< typename T > void
DEQueue<T>::pushBack( const T& t )
{
   // Guarantee we have enough space.
   if( _size == Vector<T>::size() )  grow();

   // Add new element to the back.
   ++_size;  // Arrange back() for next command.
   this->back() = t;
}

//------------------------------------------------------------------------------
//!
template< typename T > void
DEQueue<T>::pushFront( const T& t )
{
   // Guarantee we have enough space.
   if( _size == Vector<T>::size() )  grow();

   // Add new element to the front.
   ++_size;
   const size_t s = Vector<T>::size();
   _front = (_front - 1 + s) % s;  // Slightly different code than idx().
   this->front() = t;
}

//------------------------------------------------------------------------------
//!
template< typename T > void
DEQueue<T>::popBack()
{
   if( !empty() )
   {
      this->back() = T();
      --_size;
   }
}

//------------------------------------------------------------------------------
//!
template< typename T > void
DEQueue<T>::popFront()
{
   if( !empty() )
   {
      this->front() = T();
      _front = idx(1);
      --_size;
   }
}

//------------------------------------------------------------------------------
//!
template< typename T > T&
DEQueue<T>::front()
{
   return (*this)[_front];
}

//------------------------------------------------------------------------------
//!
template< typename T > const T&
DEQueue<T>::front() const
{
   return (*this)[_front];
}

//------------------------------------------------------------------------------
//!
template< typename T > T&
DEQueue<T>::back()
{
   return (*this)[idx(_size-1)];
}

//------------------------------------------------------------------------------
//!
template< typename T > const T&
DEQueue<T>::back() const
{
   return (*this)[idx(_size-1)];
}

//------------------------------------------------------------------------------
//!
template< typename T > bool
DEQueue<T>::empty() const
{
   return _size == 0;
}

//------------------------------------------------------------------------------
//!
template< typename T > size_t
DEQueue<T>::size() const
{
   return _size;
}

//------------------------------------------------------------------------------
//!
template< typename T > size_t
DEQueue<T>::capacity() const
{
   return Vector<T>::capacity();
}

//------------------------------------------------------------------------------
//!
template< typename T > const T&
DEQueue<T>::peek( size_t i ) const
{
   return Vector<T>::operator[]( idx(i) );
}

//------------------------------------------------------------------------------
//!
template< typename T > size_t
DEQueue<T>::idx( size_t v ) const
{
   return (v + _front) % Vector<T>::size();
}

//------------------------------------------------------------------------------
//! Move n elements to the specified location.
//! There must not be any overlap between range[from:from+n] and range[to:to+n]
//! and each range must define a continuous section.
template< typename T > void
DEQueue<T>::move( size_t from, size_t n, size_t to )
{
   CHECK( (from+n) <= to );
   for( size_t i = 0; i < n; ++i )
   {
      (*this)[to++] = (*this)[from++];
   }
}

//------------------------------------------------------------------------------
//! Move n elements to the specified location.
//! Both range[from:from+n] and range[to:to+n] must each be contiguous memory.
//! The copy is done right to left in order to allow potential overlaps in the ranges.
template< typename T > void
DEQueue<T>::moveRL( size_t from, size_t n, size_t to )
{
   // Copy from right to left to allow overlaps.
   from += n;
   to   += n;
   for( size_t i = 0; i < n; ++i )
   {
      (*this)[--to] = (*this)[--from];
   }
}

//------------------------------------------------------------------------------
//!
template< typename T > void
DEQueue<T>::grow()
{
   // Double the space.
   size_t newSize = _size << 1;
   if( newSize == 0 ) newSize = 1; // In case the user forces an intial size of 0.
   Vector<T>::resize( newSize );
   if( _front <= (_size>>1) )
   {
      // Move the tail, currently sitting at the beginning of the storage, to a contiguous state.
      move( 0, _front, _size );
   }
   else
   {
      // Move the head, currently packed right after the last, to the end of the storage.
      move( _front, _size-_front, _size+_front );
      _front += _size;
   }
}

//------------------------------------------------------------------------------
//!
template< typename T > void
DEQueue<T>::clear()
{
   Vector<T>::clear();
   _front = 0;
   _size  = 0;
}

NAMESPACE_END

#endif //BASE_DEQUEUE_H
