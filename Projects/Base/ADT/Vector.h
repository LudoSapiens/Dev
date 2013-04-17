/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_VECTOR_H
#define BASE_VECTOR_H

#include <Base/StdDefs.h>

#include <cstdlib>
#include <cstring>
#include <vector>

NAMESPACE_BEGIN

#if !defined(BASE_VECTOR_USES_STL)
#define BASE_VECTOR_USES_STL  0
#endif

#if BASE_VECTOR_USES_STL
#define _BaseVectorImpl   std::vector
#else

/*==============================================================================
   CLASS _BaseVectorImpl
==============================================================================*/
template<typename T>
class _BaseVectorImpl
{
public:
   
   /*----- classes ----*/
   
   class reverse_iterator
   {
   public:
      reverse_iterator() {}
      reverse_iterator( T* ptr ) : _ptr( ptr ) {}
      T& operator*() const
      {
         T* tmp = _ptr;
         return *--tmp;
      }
      T* operator->() const { return &(operator*()); }
      //Operators.
      reverse_iterator& operator++()
      {
         --_ptr;
         return *this;
      }
      reverse_iterator operator++(int)
      {
         reverse_iterator tmp( this );
         --_ptr;
         return tmp;
      }
      reverse_iterator operator+( size_t n ) const { return reverse_iterator( _ptr - n ); }
      reverse_iterator& operator+=( size_t n )
      {
         _ptr -= n;
         return *this;
      }
      reverse_iterator& operator--()
      {
         ++_ptr;
         return *this;
      }
      reverse_iterator operator--(int)
      {
         reverse_iterator tmp( this );
         ++_ptr;
         return tmp;
      }
      reverse_iterator operator-( size_t n ) const { return reverse_iterator( _ptr + n ); }
      reverse_iterator& operator-=( size_t n )
      {
         _ptr += n;
         return *this;
      }
      size_t operator-( const reverse_iterator& rit ) const
      {
         return rit._ptr - _ptr;
      }
      bool operator==( const reverse_iterator& it ) const { return _ptr == it._ptr; }
      bool operator!=( const reverse_iterator& it ) const { return _ptr != it._ptr; }
      bool operator> ( const reverse_iterator& it ) const { return _ptr  > it._ptr; }
      bool operator>=( const reverse_iterator& it ) const { return _ptr >= it._ptr; }
      bool operator< ( const reverse_iterator& it ) const { return _ptr  < it._ptr; }
      bool operator<=( const reverse_iterator& it ) const { return _ptr <= it._ptr; }
   protected:
      T*  _ptr;
   };

   /*----- types and enumerations ----*/

   typedef const T*               const_iterator;
   typedef T*                     iterator;   
   typedef const reverse_iterator const_reverse_iterator;
   typedef T&                     reference;
   typedef const T&               const_reference;
   
   /*----- methods -----*/
   
   
   // Constructor.
   _BaseVectorImpl(): _begin(0), _end(0), _last(0) { }
   _BaseVectorImpl( size_t n, const T& t = T() ) 
   {
      _begin = n > 0 ? (T*)malloc( n * sizeof(T) ) : 0;
      _end   = _begin + n;
      _last  = _end;
      fill( _begin, _end, t );
   }
   _BaseVectorImpl( const _BaseVectorImpl& v ) 
   {
      size_t n = v.size();
      _begin   = n > 0 ? (T*)malloc( n * sizeof(T) ) : 0;
      _end     = _begin + n;
      _last    = _end;
      copy( v._begin, v._end, _begin );      
   }

   // Destructor.
   ~_BaseVectorImpl() 
   {
      destroy( _begin, _end );
      free( _begin );
   }
   
   // Copy operator.
   _BaseVectorImpl& operator=( const _BaseVectorImpl& v )
   {
      if( &v != this )
      {
         destroy( _begin, _end );
         free( _begin );
         size_t n = v.size();
         _begin   = n > 0 ? (T*)malloc( n * sizeof(T) ) : 0;
         _end     = _begin + n;
         _last    = _end;
         copy( v._begin, v._end, _begin ); 
      }
      return *this;
   }
   
   // Iterators.
         iterator  begin()       { return _begin; }
   const_iterator  begin() const { return _begin; }

         iterator  end()       { return _end; }
   const_iterator  end() const { return _end; }

   
         reverse_iterator rbegin()       { return reverse_iterator( _end ); }
   const_reverse_iterator rbegin() const { return const_reverse_iterator( _end ); }
      
         reverse_iterator rend()       { return reverse_iterator( _begin ); }
   const_reverse_iterator rend() const { return const_reverse_iterator( _begin ); }
   
   // Size.
   size_t  size() const { return _end - _begin; }
   size_t  capacity() const { return _last - _begin; }
   bool empty() const { return _begin == _end; }
   
   void clear() 
   {
      destroy( _begin, _end );
      _end = _begin;
   }
   
   void  resize( size_t n, const T& t = T() )
   {
      if( n < size() )
      {
         destroy( _begin + n, _end );
      }
      else
      {
         reserve( n );
         fill( _end, _last, t );
      }
      _end = _begin + n;
   }
   
   void reserve( size_t n ) 
   {
      if( capacity() < n )
      {
         T* tmp = (T*)realloc( _begin, n * sizeof(T) );
         _end   = tmp + size();
         _last  = tmp + n;
         _begin = tmp;
      }
   }
   
   // Add/remove.
   void  push_back( const T& t )
   {
      if( _end != _last )
      {
         new( _end ) T( t );
         ++_end;
      }
      else
      {
         size_t n = capacity();
         n        = n ? n+n : 4;
         T* tmp   = (T*)realloc( _begin, n * sizeof(T) );
         _end     = tmp + size();
         _last    = tmp + n;
         _begin   = tmp;
         
         new( _end ) T( t );
         ++_end;
      }
   }
   
   void pop_back()
   {
      --_end;
      _end->~T();
   }
   
   iterator insert( iterator pos, const T& t ) 
   {
      if( _end == _last )
      {
         size_t n = capacity();
         n        = n ? n+n : 4;
         T* tmp   = (T*)realloc( _begin, n * sizeof(T) );
         pos      = tmp + (pos - _begin);
         _end     = tmp + size();
         _last    = tmp + n;
         _begin   = tmp;
      }
      memmove( pos + 1, pos, (_end-pos) * sizeof(T) );
      new( pos ) T( t );
      ++_end;
      return pos;
   }
   
   iterator insert( iterator pos, const_iterator f, const_iterator l )
   {
      size_t sizeToCopy = l - f;
      size_t n = capacity();
      if( n < (size() + sizeToCopy) )
      {
         // Take maximum between doubling, and growing tightly.
         n        = n < sizeToCopy ? n+sizeToCopy : n+n;
         T* tmp   = (T*)realloc( _begin, n * sizeof(T) );
         pos      = tmp + (pos - _begin);
         _end     = tmp + size();
         _last    = tmp + n;
         _begin   = tmp;
      }
      memmove( pos + sizeToCopy, pos, (_end-pos) * sizeof(T) );
      while( f < l )
      {
         new( pos++ ) T( *f++ );
      }
      _end += sizeToCopy;
      return pos;
   }
   
   iterator erase( iterator pos ) 
   {
      return erase( pos, pos+1 );
   }
   
   iterator erase( iterator first, iterator last ) 
   {
      destroy( first, last );
      memmove( first, last, (_end-last) * sizeof(T) );
      _end -= (last-first);
      return first;
   }
   
   // Access.
   T& operator[]( size_t pos ) { return _begin[pos]; }
   const T& operator[]( size_t pos ) const { return _begin[pos]; }
   
   T& front() { return *_begin; }
   const T& front() const { return *_begin; }
   T& back() { return _end[-1]; }
   const T& back() const { return _end[-1]; }
   
   void swap( _BaseVectorImpl& v )
   {
      T* tmp   = _begin;
      _begin   = v._begin;
      v._begin = tmp;
      
      tmp      = _end;
      _end     = v._end;
      v._end   = tmp;
      
      tmp      = _last;
      _last    = v._last;
      v._last  = tmp;
   }
      
   
protected:
   
   void destroy( T* begin, T* end )
   {
      for( ; begin != end; ++begin )
      {
         begin->~T();
      }
   }
   
   void fill( T* begin, T* end, const T& t )
   {
      for( ; begin != end; ++begin )
      {
         new( begin ) T( t );
      }
   }
   
   void copy( T* begin, T* end, T* cur )
   {
      for( ; begin != end; ++begin, ++cur )
      {
         new( cur ) T( *begin );
      }
   }
   
   /*----- data members -----*/
   
   T*  _begin;
   T*  _end;
   T*  _last;  //end of allocation

};
#endif //BASE_VECTOR_USES_STL

/*==============================================================================
  CLASS Vector
==============================================================================*/

//! Vector class

template< typename T >
class Vector
   : public _BaseVectorImpl<T>
{

public:

   /*----- types and enumerations ----*/

   typedef typename _BaseVectorImpl<T>::const_iterator   ConstIterator;
   typedef typename _BaseVectorImpl<T>::iterator         Iterator;
   typedef typename _BaseVectorImpl<T>::reverse_iterator ReverseIterator;
   
   
   /*----- methods -----*/

   Vector();
   Vector( size_t n );
   Vector( size_t n, const T& t );
   Vector( const Vector& );

   void eraseSwap( Iterator it );

   Iterator find( const T& );
   bool remove( const T& );
   bool removeAll( const T& );
   bool removeSwap( const T& );
   bool removeAllSwap( const T& );
   bool replace( const T& a, const T& b );
   bool replaceAll( const T& a, const T& b );

   void pushBack( const T& );
   void popBack();

   void append( const Vector& );
   void swap( Vector& );

   void reverse( Iterator first, Iterator last );

   void grow( size_t n, const T& t = T() );
   void shrink( size_t n );

   void move( Iterator src, Iterator dst );
   void move( size_t srcIdx, size_t dstIdx );

         T*  data()       { return &((*this)[0]); }
   const T*  data() const { return &((*this)[0]); }

   size_t  dataSize() const { return this->size() * sizeof(T); }
};

//------------------------------------------------------------------------------
//!
template< typename T >
Vector<T>::Vector()
{}

//------------------------------------------------------------------------------
//!
template< typename T >
Vector<T>::Vector( size_t n )
   : _BaseVectorImpl<T>( n )
{}

//------------------------------------------------------------------------------
//!
template< typename T >
Vector<T>::Vector( size_t n, const T& t )
   : _BaseVectorImpl<T>( n, t )
{}

//------------------------------------------------------------------------------
//!
template< typename T >
Vector<T>::Vector( const Vector& v )
   : _BaseVectorImpl<T>( v )
{}

//------------------------------------------------------------------------------
//!
template< typename T > void
Vector<T>::eraseSwap( Iterator it )
{
   // Put last element in its place.
   *it = this->back();
   popBack();
}

//------------------------------------------------------------------------------
//!
template< typename T > typename Vector<T>::Iterator
Vector<T>::find( const T& t )
{
   for( Iterator it = this->begin(); it != this->end(); ++it )
   {
      if( *it == t )
      {
         return it;
      }
   }
   return this->end();
}

//------------------------------------------------------------------------------
//!
template< typename T > bool
Vector<T>::remove( const T& t )
{
   for( Iterator it = this->begin(); it != this->end(); ++it )
   {
      if( *it == t )
      {
         _BaseVectorImpl<T>::erase( it );
         return true;
      }
   }
   return false;
}

//------------------------------------------------------------------------------
//!
template< typename T > bool
Vector<T>::removeAll( const T& t )
{
   // Find t.
   Iterator it = this->begin();
   for( ; it != this->end(); ++it )
   {
      if( *it == t ) break;
   }
   
   // t not found.
   if( it == this->end() ) return false;

   // Remove all occurences of t.
   Iterator cur = it;
   for( ++it; it != this->end(); ++it )
   {
      if( !(*it == t) )
      {
         *cur = *it;
         ++cur;
      }
   }
   
   // Erase all elements.
   _BaseVectorImpl<T>::erase( cur, this->end() );
   
   return true;
}

//------------------------------------------------------------------------------
//!
template< typename T > bool
Vector<T>::removeSwap( const T& t )
{
   for( Iterator it = this->begin(); it != this->end(); ++it )
   {
      if( *it == t )
      {
         // Put last element in its place.
         *it = this->back();
         popBack();
         return true;
      }
   }
   return false;
}

//------------------------------------------------------------------------------
//!
template< typename T > bool
Vector<T>::removeAllSwap( const T& t )
{
   bool removed = false;

   // Remove all occurences of t.
   Iterator it = this->begin();
   while( it != this->end() )
   {
      if( *it == t )
      {
         *it = this->back();
         popBack();
         removed = true;
         // Keep iterator at current position to check back() just copied.
      }
      else
      {
         ++it;
      }
   }

   return removed;
}

//------------------------------------------------------------------------------
//!
template< typename T > bool
Vector<T>::replace( const T& a, const T& b )
{
   for( Iterator it = this->begin(); it != this->end(); ++it )
   {
      if( *it == a )
      {
         *it = b;
         return true;
      }
   }
   return false;
}

//------------------------------------------------------------------------------
//!
template< typename T > bool
Vector<T>::replaceAll( const T& a, const T& b )
{
   bool found = false;
   for( Iterator it = this->begin(); it != this->end(); ++it )
   {
      if( *it == a )
      {
         *it = b;
         found = true;
      }
   }
   return found;
}

//------------------------------------------------------------------------------
//!
template< typename T > void
Vector<T>::pushBack( const T& t )
{
   _BaseVectorImpl<T>::push_back( t );
}

//------------------------------------------------------------------------------
//!
template< typename T > void
Vector<T>::popBack()
{
   _BaseVectorImpl<T>::pop_back();
}

//------------------------------------------------------------------------------
//!
template< typename T > void
Vector<T>::append( const Vector<T>& v )
{
   _BaseVectorImpl<T>::insert( _BaseVectorImpl<T>::end(), v.begin(), v.end() );
}

//------------------------------------------------------------------------------
//! 
template< typename T > void
Vector<T>::swap( Vector<T>& v )
{
   _BaseVectorImpl<T>::swap( v );
}

//------------------------------------------------------------------------------
//!
template< typename T > void 
Vector<T>::reverse( typename Vector<T>::Iterator first, typename Vector<T>::Iterator last )
{
   while( first < last )
   {
      --last;
      T tmp  = *first;
      *first = *last;
      *last  = tmp;
      first++;
   }
}

//------------------------------------------------------------------------------
//!
template< typename T > void
Vector<T>::grow( size_t n, const T& t )
{
   size_t s = _BaseVectorImpl<T>::size();
   if( s < n )
   {
      size_t c = _BaseVectorImpl<T>::capacity();
      while( c < n )
      {
         c = c ? c+c : 4;
      }
      _BaseVectorImpl<T>::reserve( c );
      _BaseVectorImpl<T>::fill( _BaseVectorImpl<T>::_end, _BaseVectorImpl<T>::_begin + n, t );
      _BaseVectorImpl<T>::_end = _BaseVectorImpl<T>::_begin + n;
   }
}

//------------------------------------------------------------------------------
//!
template< typename T > void
Vector<T>::shrink( size_t n )
{
   if( n < _BaseVectorImpl<T>::size() )
   {
      //_BaseVectorImpl<T>::resize( n );
      _BaseVectorImpl<T>::destroy( _BaseVectorImpl<T>::_begin + n, _BaseVectorImpl<T>::_end );
      _BaseVectorImpl<T>::_end = _BaseVectorImpl<T>::_begin + n;
   }
}

//------------------------------------------------------------------------------
//!
template< typename T > void
Vector<T>::move( typename Vector<T>::Iterator src, typename Vector<T>::Iterator dst )
{
   if( dst < src )
   {
      // Copy the last element (the one being moved).
      T tmp = *src;
      T* dstP = &(*dst);
      // Copy the n-1 first elements up one value.
      memmove( dstP+1, dstP, (src-dst)*sizeof(T) );
      // Copy the tmp in the created slot at the beginning.
      *dst = tmp;
   }
   else
   {
      // Copy the first element (the one being moved).
      T tmp = *src;
      T* dstP = &(*src);
      // Copy the n-1 last elements down one value.
      memmove( dstP, dstP+1, (dst-src)*sizeof(T) );
      // Copy the tmp in the created slot at the end.
      *dst = tmp;
   }
}

//------------------------------------------------------------------------------
//!
template< typename T > void
Vector<T>::move( size_t srcIdx, size_t dstIdx )
{
   move( _BaseVectorImpl<T>::begin()+srcIdx, _BaseVectorImpl<T>::begin()+dstIdx );
}

NAMESPACE_END

#endif

