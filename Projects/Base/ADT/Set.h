/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_SET_H
#define BASE_SET_H

#include <Base/StdDefs.h>

#include <Base/ADT/Vector.h>

NAMESPACE_BEGIN


/*==============================================================================
  CLASS Set
==============================================================================*/

template< typename T >
class Set:
   protected Vector<T>
{

public:

   /*----- types and enumerations ----*/

   typedef typename Vector<T>::const_iterator         ConstIterator;
   typedef typename Vector<T>::iterator               Iterator;
   typedef typename Vector<T>::reverse_iterator       ReverseIterator;
   typedef typename Vector<T>::const_reverse_iterator ConstReverseIterator;

   /*----- methods -----*/

   Set();
   Set( const Set<T>& s );
   Set( const Vector<T>& v );

   T&  add( const T& element );
   bool  addCheck( const T& element );
   bool  has( const T& element ) const;
   size_t  size() const { return Vector<T>::size(); }
   void  clear() { return Vector<T>::clear(); }

   template< typename Iter >
   void  insert( Iter c, Iter e )
   {
      for( ; c != e; ++c )
      {
         add(*c);
      }
   }
   
   Iterator remove( const T& element )
   {
      size_t idx = searchIndex(element);
      if( idx < size() && (*this)[idx] == element )
      {
         return Vector<T>::erase( begin() + idx );
      }
      return end();
   }
   bool removeCheck( const T& element )
   {
      size_t idx = searchIndex(element);
      if( idx < size() && (*this)[idx] == element )
      {
         Vector<T>::erase( begin() + idx );
         return true;
      }
      return false;
   }
   Iterator remove( Iterator it )
   {
      return Vector<T>::erase( it );
   }

   void  swap( Set<T>& s ) { Vector<T>::swap(s); }

   // Iterators.
        Iterator  begin()       { return Vector<T>::begin(); }
   ConstIterator  begin() const { return Vector<T>::begin(); }

        Iterator  end()       { return Vector<T>::end(); }
   ConstIterator  end() const { return Vector<T>::end(); }

        ReverseIterator  rbegin()       { return Vector<T>::rbegin(); }
   ConstReverseIterator  rbegin() const { return Vector<T>::rbegin(); }

        ReverseIterator  rend()       { return Vector<T>::rend(); }
   ConstReverseIterator  rend() const { return Vector<T>::rend(); }

   bool  empty() const { return Vector<T>::empty(); }

protected:
   size_t  searchIndex( const T& element ) const;
};

//------------------------------------------------------------------------------
//!
template< typename T >
Set<T>::Set() :
   Vector<T>()
{}

//------------------------------------------------------------------------------
//!
template< typename T >
Set<T>::Set( const Set<T>& s ):
   Vector<T>( s )
{}

//------------------------------------------------------------------------------
//!
template< typename T >
Set<T>::Set( const Vector<T>& v ):
   Vector<T>( v )
{}

//------------------------------------------------------------------------------
//!
template< typename T >
T&
Set<T>::add( const T& element )
{
   // Do a binary insertion sort (keeping order)
   size_t idx = searchIndex(element);
   if( idx >= size() )
   {
      // Need to insert at the end
      Vector<T>::pushBack(element);
      return Vector<T>::back();
   }
   else
   if( !(element == (*this)[idx]) )
   {
      // Insert element (only if it isn't there)
      Vector<T>::insert( begin() + idx, element );
   }
   return (*this)[idx];
}

//------------------------------------------------------------------------------
//! Adds an elements, and returns true if it is a new entry.
template< typename T >
bool
Set<T>::addCheck( const T& element )
{
   // Do a binary insertion sort (keeping order)
   size_t idx = searchIndex(element);
   if( idx >= size() )
   {
      // Need to insert at the end
      Vector<T>::pushBack(element);
      return true;
   }
   else
   if( !(element == (*this)[idx]) )
   {
      // Insert element (only if it isn't there)
      Vector<T>::insert( begin() + idx, element );
      return true;
   }
   // Already there.
   return false;
}

//------------------------------------------------------------------------------
//!
template< typename T >
bool
Set<T>::has( const T& element )
const
{
   // Do a binary insertion sort (keeping order)
   size_t idx = searchIndex(element);
   if( idx >= size() )
   {
      return false;
   }
   else
   {
      return element == (*this)[idx];
   }
}

//------------------------------------------------------------------------------
//! Returns the index at which the specified element either is, or should be
template< typename T >
size_t
Set<T>::searchIndex( const T& element )
const
{
   // Do a binary insertion sort (keeping order)
   size_t s = 0;
   size_t e = size();
   size_t h;
   while( e != s )
   {
      h = (s + e)/2;
      // Check if middle value is greater, or not
      if( element < (*this)[h] )
      {
         // We belong in first half
         e = h;
      }
      else
      if( element == (*this)[h] )
      {
         // Early termination
         return h;
      }
      else
      {
         // We belong in second half
         s = h + 1;
      }
   }
   return s;
}


NAMESPACE_END

#endif //BASE_SET_H
