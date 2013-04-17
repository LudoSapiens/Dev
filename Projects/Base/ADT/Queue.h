/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_QUEUE_H
#define BASE_QUEUE_H

#include <Base/StdDefs.h>

#include <Base/ADT/DEQueue.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Queue
==============================================================================*/

//! Queue container class.
//! Elements are pushed at the end, and popped at the front.

template< typename T >
class Queue:
   private DEQueue<T>
{
public:

   /*----- methods -----*/

   Queue( const size_t n = 8 );
   Queue( const Queue& );

   inline void  push( const T& t ) { DEQueue<T>::pushBack(t); }
   inline void  pop()              { DEQueue<T>::popFront();  }

   using DEQueue<T>::front;
   using DEQueue<T>::back;
   using DEQueue<T>::peek;
   using DEQueue<T>::empty;
   using DEQueue<T>::size;
   using DEQueue<T>::capacity;
   using DEQueue<T>::clear;

protected:
};

//------------------------------------------------------------------------------
//!
template< typename T>
Queue<T>::Queue( const size_t n ) :
   DEQueue<T>( n )
{
}

//------------------------------------------------------------------------------
//!
template< typename T >
Queue<T>::Queue( const Queue& q ):
   DEQueue<T>( q )
{
}

NAMESPACE_END

#endif //BASE_QUEUE_H
