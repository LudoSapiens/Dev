/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_RCVECTOR_H
#define BASE_RCVECTOR_H

#include <Base/StdDefs.h>

#include <Base/ADT/Vector.h>
#include <Base/Util/RCObject.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS RCVector
==============================================================================*/
//!< An RCVector is effectively an RCObject of a Vector<T>.
template< typename T >
class RCVector:
   public RCObject
{
public:

   /*----- types -----*/
   typedef typename Vector<T>::ConstIterator    ConstIterator;
   typedef typename Vector<T>::Iterator         Iterator;
   typedef typename Vector<T>::ReverseIterator  ReverseIterator;

   /*----- methods -----*/

   RCVector(): _vector() {}
   RCVector( size_t n ): _vector(n) {}
   RCVector( size_t n, const T& t ): _vector(n, t) {}
   RCVector( const RCVector& b ): _vector(b._vector) {}

        Iterator  begin()       { return _vector.begin(); }
   ConstIterator  begin() const { return _vector.begin(); }

        Iterator  end()       { return _vector.end(); }
   ConstIterator  end() const { return _vector.end(); }

        Iterator  rbegin()       { return _vector.rbegin(); }
   ConstIterator  rbegin() const { return _vector.rbegin(); }

        Iterator  rend()       { return _vector.rend(); }
   ConstIterator  rend() const { return _vector.rend(); }

   Iterator find( const T& e ) { return _vector.find(e); }

   bool remove( const T& e ) { return _vector.remove(e); }
   bool removeAll( const T& e ) { return _vector.removeAll(e); }

   void pushBack( const T& e ) { _vector.pushBack(e); }
   void popBack() { return _vector.popBack(); }

   Iterator insert( Iterator pos, const T& t )                       { return _vector.insert(pos, t);    }
   Iterator insert( Iterator pos, ConstIterator f, ConstIterator l ) { return _vector.insert(pos, f, l); }

   Iterator erase( Iterator pos )                  { return _vector.erase(pos);         }
   Iterator erase( Iterator first, Iterator last ) { return _vector.erase(first, last); }

   void append( const RCVector& b ) { return _vector.append(b._vector); }
   void append( const Vector<T>& v ) { return _vector.append(v); }

   void reverse( Iterator first, Iterator last ) { _vector.reverse(first, last); }

   void swap( RCVector& b ) { _vector.swap(b._vector); }

         T& operator[]( size_t pos )       { return _vector[pos]; }
   const T& operator[]( size_t pos ) const { return _vector[pos]; }

         T& front()       { return _vector.front(); }
   const T& front() const { return _vector.front(); }

         T& back()       { return _vector.back(); }
   const T& back() const { return _vector.back(); }

         T*  data()       { return _vector.data(); }
   const T*  data() const { return _vector.data(); }

   size_t  dataSize() const { return _vector.dataSize(); }

   size_t  size()     const { return _vector.size();     }
   size_t  capacity() const { return _vector.capacity(); }
   bool    empty()    const { return _vector.empty();    }

   void  clear() { _vector.clear(); }

   void  resize( size_t n, const T& t = T() ) { _vector.resize(n, t); }
   void  reserve( size_t n ) { _vector.reserve(n); }

   // Typecast operators to retrieve the vector directly.
   operator       Vector<T>&()       { return _vector; }
   operator const Vector<T>&() const { return _vector; }

protected:

   /*----- data members -----*/

   Vector<T>  _vector;  //!< The vector containing the actual data.

}; //class RCVector


NAMESPACE_END

#endif //BASE_RCVECTOR_H
