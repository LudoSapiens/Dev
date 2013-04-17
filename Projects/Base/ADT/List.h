/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_LIST_H
#define BASE_LIST_H

#include <Base/StdDefs.h>

#include <list>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS List
==============================================================================*/

//! Doubly-linked list class

template< typename T >
class List
   : public std::list<T>
{

public:

   /*----- types and enumerations ----*/

   typedef typename std::list<T>::const_iterator   ConstIterator;
   typedef typename std::list<T>::iterator         Iterator;
   
   /*----- methods -----*/

   List();
   List( size_t n );
   List( size_t n, const T& t );
   List( const List& );

   
   void pushFront( const T& );
   void popFront();
   void pushBack( const T& );
   void popBack();
};

//------------------------------------------------------------------------------
//!
template< typename T >
List<T>::List()
{}

//------------------------------------------------------------------------------
//!
template< typename T >
List<T>::List
( size_t n )
   : std::list<T>( n )
{}

//------------------------------------------------------------------------------
//!
template< typename T >
List<T>::List
( size_t n, const T& t )
   : std::list<T>( n, t )
{}

//------------------------------------------------------------------------------
//!
template< typename T >
List<T>::List
( const List& v )
   : std::list<T>( v )
{}

//------------------------------------------------------------------------------
//!
template< typename T >
void
List<T>::pushFront
( const T& t )
{
   std::list<T>::push_front( t );
}

//------------------------------------------------------------------------------
//!
template< typename T >
void
List<T>::popFront()
{
   std::list<T>::pop_front();
}

//------------------------------------------------------------------------------
//!
template< typename T >
void
List<T>::pushBack
( const T& t )
{
   std::list<T>::push_back( t );
}

//------------------------------------------------------------------------------
//!
template< typename T >
void
List<T>::popBack()
{
   std::list<T>::pop_back();
}

NAMESPACE_END

#endif

