/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_DYN_ARRAY_H
#define BASE_DYN_ARRAY_H

#include <Base/StdDefs.h>

#include <cstdlib>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS DynArray
==============================================================================*/
//!< A simple dynamic array which auto-resize on out-of-bounds accesses.
//!< It is meant to be lighter than Vector (only stores a pointer and a size),
//!< but it does not amortize allocation, and instead is always tight.
//!< Out-of-bounds access also performs a lazy resize as to always return
//!< valid (i.e. allocated) data.
//!< It is meant to be used for cases where resizes are rare (typically only
//!< a few times at the beginning).
template< typename T >
class DynArray
{
public:

   /*----- types and enumerations ----*/

   class ReverseIterator
   {
   public:
      ReverseIterator() {}
      ReverseIterator( T* ptr ) : _ptr( ptr ) {}
      T& operator*() const
      {
         T* tmp = _ptr;
         return *--tmp;
      }
      T* operator->() const
      {
         return &(operator*());
      }
      //Operators.
      ReverseIterator& operator++()
      {
         --_ptr;
         return *this;
      }
      ReverseIterator operator++(int)
      {
         ReverseIterator tmp( this );
         --_ptr;
         return tmp;
      }
      ReverseIterator operator+( size_t n ) const
      {
         return ReverseIterator( _ptr - n );
      }
      ReverseIterator& operator+=( size_t n )
      {
         _ptr -= n;
         return *this;
      }
      ReverseIterator& operator--()
      {
         ++_ptr;
         return *this;
      }
      ReverseIterator operator--(int)
      {
         ReverseIterator tmp( this );
         ++_ptr;
         return tmp;
      }
      ReverseIterator operator-( size_t n ) const
      {
         return ReverseIterator( _ptr + n );
      }
      ReverseIterator& operator-=( size_t n )
      {
         _ptr += n;
         return *this;
      }
      size_t operator-( const ReverseIterator& rit ) const
      {
         return rit._ptr - _ptr;
      }
      bool operator==( const ReverseIterator& it ) const { return _ptr == it._ptr; }
      bool operator!=( const ReverseIterator& it ) const { return _ptr != it._ptr; }
      bool operator> ( const ReverseIterator& it ) const { return _ptr  > it._ptr; }
      bool operator>=( const ReverseIterator& it ) const { return _ptr >= it._ptr; }
      bool operator< ( const ReverseIterator& it ) const { return _ptr  < it._ptr; }
      bool operator<=( const ReverseIterator& it ) const { return _ptr <= it._ptr; }

   protected:
      T*  _ptr;
   }; // class ReverseIterator


   typedef T*                     Iterator;
   typedef const Iterator         ConstIterator;
   typedef const ReverseIterator  ConstReverseIterator;

   /*----- methods -----*/

   DynArray( size_t initialSize = 0, const T& t = T() );
   DynArray( const DynArray& a );
   ~DynArray();

   size_t  size() const { return _size; }
   bool  empty() const { return _size == 0; }

   void  resize( size_t s, const T& t = T() );

   void  clear() { resize(0); }

         T&  operator[]( size_t i );
   const T&  operator[]( size_t i ) const;

   T&  get( size_t i, const T& t = T() );

        Iterator  begin()       { return _data; }
   ConstIterator  begin() const { return _data; }

        Iterator  end()       { return _data+_size; }
   ConstIterator  end() const { return _data+_size; }

        ReverseIterator  rbegin()       { return ReverseIterator( _data+_size ); }
   ConstReverseIterator  rbegin() const { return ConstReverseIterator( _data+_size ); }

        ReverseIterator  rend()       { return ReverseIterator( _data ); }
   ConstReverseIterator  rend() const { return ConstReverseIterator( _data ); }

   void  swap( DynArray<T>& a );

protected:

   /*----- data members -----*/

   T*      _data;
   size_t  _size;

   /*----- methods -----*/

   void destroy( T* begin, T* end );
   void fill( T* begin, T* end, const T& t );
   void copy( T* begin, T* end, T* dst );
   static T*  alloc( size_t s );
   static void  dealloc( T* ptr );

private:
}; //class DynArray

//------------------------------------------------------------------------------
//!
template< typename T >
DynArray<T>::DynArray( size_t initialSize, const T& t )
{
   _data = alloc( initialSize );
   _size = initialSize;
   fill( _data, _data+_size, t );
}

//------------------------------------------------------------------------------
//!
template< typename T >
DynArray<T>::DynArray( const DynArray& a )
{
   if( &a != this )
   {
      _data = alloc( a._size );
      _size = a._size;
      copy( a._data, a._data+_size, _data );
   }
}

//------------------------------------------------------------------------------
//!
template< typename T >
DynArray<T>::~DynArray()
{
   destroy( _data, _data+_size );
   dealloc( _data );
}

//------------------------------------------------------------------------------
//!
template< typename T > void
DynArray<T>::resize( size_t s, const T& t )
{
   if( s == _size )  return;

   T* newData = alloc( s );

   if( s < _size )
   {
      // Truncate to a smaller size.
      // 1. Copy the kept data.
      copy( _data, _data+s, newData );
      // 2. Delete the truncated data.
      destroy( _data+s, _data+_size );
   }
   else
   {
      // Grow.
      // 1. Copy the original data.
      copy( _data, _data+_size, newData );
      // 2. Fill rest with default data.
      fill( newData+_size, newData+s, t );
   }

   // Deallocate old storage.
   dealloc( _data );
   // Use new storage.
   _data = newData;
   _size = s;
}

//------------------------------------------------------------------------------
//!
template< typename T > T&
DynArray<T>::operator[]( size_t i )
{
   if( i >= _size )  resize( i+1 );
   return _data[i];
}

//------------------------------------------------------------------------------
//! If the array is const, then no allocation is possible.
//! So we just return the data (or garbage if it is out-of-bounds).
template< typename T > const T&
DynArray<T>::operator[]( size_t i ) const
{
   return _data[i];
}

//------------------------------------------------------------------------------
//!
template< typename T > T&
DynArray<T>::get( size_t i, const T& t )
{
   if( i >= _size )  resize( i+1, t );
   return _data[i];
}

//------------------------------------------------------------------------------
//!
template< typename T > void
DynArray<T>::swap( DynArray<T>& a )
{
   T* tmpData = _data;
   _data      = a._data;
   a._data    = tmpData;

   size_t tmpSize = _size;
   _size          = a._size;
   a._size        = tmpSize;
}

//------------------------------------------------------------------------------
//!
template< typename T > void
DynArray<T>::destroy( T* begin, T* end )
{
   for( ; begin != end; ++begin )
   {
      begin->~T();
   }
}

//------------------------------------------------------------------------------
//!
template< typename T > void
DynArray<T>::fill( T* begin, T* end, const T& t )
{
   for( ; begin != end; ++begin )
   {
      new( begin ) T( t );
   }
}

//------------------------------------------------------------------------------
//!
template< typename T > void
DynArray<T>::copy( T* begin, T* end, T* dst )
{
   for( ; begin != end; ++begin, ++dst )
   {
      new( dst ) T( *begin );
   }
}

//------------------------------------------------------------------------------
//!
template< typename T > T*
DynArray<T>::alloc( size_t s )
{
   return (s != 0) ? (T*)malloc( s * sizeof(T) ) : NULL;
}

//------------------------------------------------------------------------------
//!
template< typename T > void
DynArray<T>::dealloc( T* ptr )
{
   if( ptr )  free( ptr );
}


NAMESPACE_END

#endif //BASE_DYN_ARRAY_H
