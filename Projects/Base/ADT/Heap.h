/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_HEAP_H
#define BASE_HEAP_H

#include <Base/StdDefs.h>

#include <Base/ADT/Vector.h>
#include <Base/Dbg/Defs.h>
#include <Base/IO/TextStream.h>

#include <algorithm>
#include <functional>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Heap
==============================================================================*/

//! Heap container class.

template< typename T, typename C = std::less<T> >
class Heap:
   public Vector<T>
{

public:

   /*----- methods -----*/

   Heap();
   Heap( const Vector<T>& );
   Heap( const Heap& );

   void  push( const T& t );
   void  pop();

         T&  root();
   const T&  root() const;

protected:
   
   /*----- data members -----*/

   C  _c;
};

//------------------------------------------------------------------------------
//!
template< typename T, typename C >
Heap<T, C>::Heap()
{
}

//------------------------------------------------------------------------------
//!
template< typename T, typename C >
Heap<T, C>::Heap( const Vector<T>& v ):
   Vector<T>( v )
{
   std::make_heap(Vector<T>::begin(), Vector<T>::end(), _c);
}

//------------------------------------------------------------------------------
//!
template< typename T, typename C >
Heap<T, C>::Heap( const Heap& h ):
   Vector<T>( h )
{
}

//------------------------------------------------------------------------------
//!
template< typename T, typename C > void
Heap<T, C>::push( const T& t )
{
   Vector<T>::pushBack(t);
   std::push_heap(Vector<T>::begin(), Vector<T>::end(), _c);
}

//------------------------------------------------------------------------------
//!
template< typename T, typename C > void
Heap<T, C>::pop()
{
   std::pop_heap(Vector<T>::begin(), Vector<T>::end(), _c);
   Vector<T>::popBack();
}

//------------------------------------------------------------------------------
//!
template< typename T, typename C > T&
Heap<T, C>::root()
{
   return Vector<T>::front();
}

//------------------------------------------------------------------------------
//!
template< typename T, typename C > const T&
Heap<T, C>::root() const
{
   return Vector<T>::front();
}


/*==============================================================================
  CLASS FHeap
==============================================================================*/
//! This heap class implements a more complete interface to also allow to
//! update elements.
//! In order to achieve this, every insertion returns a unique id which can
//! later be used to update the entry in the heap.
template< typename T, typename C = std::less<T> >
class FHeap
{
public:


   /*----- types -----*/
   class Node
   {
   public:
      const T& data() const { return _data; }
      T& data() { return _data; }
      void data( const T& d ) { _data = d; }

      uint  idx() const { return _idx; }

   protected:
      friend class FHeap;
      Node( const T& data, uint idx ): _data(data), _idx(idx) {}
      void  idx( uint i ) { _idx = i; }
      T     _data;
      uint  _idx;
   };

   /*----- methods -----*/

   FHeap( uint reserveSize = 0, const C& c = C() );
   FHeap( const Vector<T>& );
   FHeap( const FHeap& );
   ~FHeap();

         Node*  rootNode()       { return _heap.front(); }
   const Node*  rootNode() const { return _heap.front(); }

   const T&  root() const { return _heap.front()->data(); }

   uint  size() const { return uint(_heap.size()); }
   bool  empty() const { return _heap.empty(); }

   void  reserve( uint size ) { _heap.reserve(size); }
   void  clear();

   Node*  push( const T& t );
   void   pop();
   void   pop( const Node* node ) { pop(node->idx()); }
   void   update( const Node* node ) { update(node->idx()); }

         Node*  node( uint idx )       { return _heap[idx]; }
   const Node*  node( uint idx ) const { return _heap[idx]; }

protected:


   /*----- data members -----*/

   Vector< Node* >  _heap;  //!< The binary tree, by array.
   C                _cmp;   //!< The comparator object (which should be zero size).

   /*----- methods -----*/

   uint  parent( uint idx ) const { return ((idx - 1) >> 1); }
   uint  leftChild( uint idx ) const { return ((idx << 1) + 1); }
   uint  rightChild( uint idx ) const { return ((idx << 1) + 2); }

   bool  valid( uint idx ) const { return idx < size(); }

   void  swap( uint idxA, uint idxB );
   void  percolateUp( uint idx );
   void  percolateDown( uint idx );
   void  update( uint idx );
   void  pop( uint idx );

private:
}; //class FHeap

//------------------------------------------------------------------------------
//!
template< typename T, typename C >
FHeap<T, C>::FHeap( uint reserveSize, const C& c ):
   _cmp( c )
{
   reserve( reserveSize );
}

#if 0
//------------------------------------------------------------------------------
//!
template< typename T, typename C >
FHeap<T, C>::FHeap( const Vector<T>& )
{
   StdErr << "TODO" << nl;
}

//------------------------------------------------------------------------------
//!
template< typename T, typename C >
FHeap<T, C>::FHeap( const FHeap& h )
{
   StdErr << "TODO" << nl;
   clear();
   uint n = h.size();
   reserve( n );
   for( uint i = 0; i < n; ++i )
   {
      // Copy every node, but need to handle Node* properly.
   }
}
#endif

//------------------------------------------------------------------------------
//!
template< typename T, typename C >
FHeap<T, C>::~FHeap()
{
   clear();
}

//------------------------------------------------------------------------------
//!
template< typename T, typename C > void
FHeap<T, C>::clear()
{
   // Deallocate all nodes.
   const uint n = size();
   for( uint i = 0; i < n; ++i )
   {
      delete _heap[i];
   }

   // Clear heap.
   _heap.clear();
}

//------------------------------------------------------------------------------
//!
template< typename T, typename C > typename FHeap<T, C>::Node*
FHeap<T, C>::push( const T& t )
{
   Node* node = new Node( t, size() );
   _heap.pushBack( node );
   percolateUp( node->idx() );
   return node;
}

//------------------------------------------------------------------------------
//!
template< typename T, typename C > void
FHeap<T, C>::pop()
{
   CHECK( !empty() );
   pop( (uint)0 );
}

//------------------------------------------------------------------------------
//!
template< typename T, typename C > void
FHeap<T, C>::pop( uint idx )
{
   CHECK( valid(idx) );

   // Deallocate element.
   delete _heap[idx];

   if( (idx+1) != size() )
   {
      // Put last element in its place.
      _heap[idx] = _heap.back();
      _heap[idx]->idx( idx );
      _heap.popBack();

      // Percolate it down.
      percolateDown( idx );
   }
   else
   {
      // Was already last element.
      _heap.popBack();
   }
}

//------------------------------------------------------------------------------
//! Swaps the two nodes described by the indices.
//! We need to move the pointers since the user uses them to point to elements
//! in the heap.
template< typename T, typename C > void
FHeap<T, C>::swap( uint idxA, uint idxB )
{
   Node* tmp = _heap[idxA];
   _heap[idxA] = _heap[idxB];
   _heap[idxB] = tmp;
   _heap[idxA]->idx( idxA );
   _heap[idxB]->idx( idxB );
}

//------------------------------------------------------------------------------
//!
template< typename T, typename C > void
FHeap<T, C>::percolateUp( uint idx )
{
   while( idx != 0 )
   {
      uint pIdx = parent( idx );
      Node* c = _heap[idx];
      Node* p = _heap[pIdx];
      if( _cmp(c->data(), p->data()) )
      {
         swap( idx, pIdx );
         idx = pIdx;
      }
      else
      {
         return;
      }
   }
}

//------------------------------------------------------------------------------
//!
template< typename T, typename C > void
FHeap<T, C>::percolateDown( uint idx )
{
   uint lIdx = leftChild( idx );
   if( valid(lIdx) )
   {
      Node* l = _heap[lIdx];
      Node* c = _heap[idx];
      uint rIdx = rightChild(idx);
      if( valid(rIdx) )
      {
         // We have 2 children.
         Node* r = _heap[rIdx];
         if( _cmp(l->data(), r->data()) )
         {
            if( _cmp(l->data(), c->data()) )
            {
               swap( lIdx, idx );
               percolateDown( lIdx );
            }
         }
         else
         {
            if( _cmp(r->data(), c->data()) )
            {
               swap( rIdx, idx );
               percolateDown( rIdx );
            }
         }
      }
      else
      {
         // We only have a left child.
         if( _cmp(l->data(), c->data()) )
         {
            swap( lIdx, idx );
            percolateDown( lIdx );
         }
      }
   }
   // else, we have no child.
}

//------------------------------------------------------------------------------
//!
template< typename T, typename C > void
FHeap<T, C>::update( uint idx )
{
   uint pIdx = parent( idx );
   if( valid(pIdx) )
   {
      Node* c = _heap[idx];
      Node* p = _heap[pIdx];
      if( _cmp(c->data(), p->data()) )
      {
         percolateUp( idx );
      }
      else
      {
         percolateDown( idx );
      }
   }
   else
   {
      percolateDown( idx );
   }
}

NAMESPACE_END

#endif //BASE_HEAP_H
