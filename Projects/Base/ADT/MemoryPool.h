/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_MEMORYPOOL_H
#define BASE_MEMORYPOOL_H

#include <Base/StdDefs.h>
#include <Base/ADT/Vector.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS MemoryPool
==============================================================================*/

template< typename T >
class MemoryPool
{
public:

   /*----- methods -----*/

   MemoryPool( size_t chunkSize = 32 );
   ~MemoryPool();

   void clear();

   // Memory allocation without call to constructor/destructor.
   T* alloc();
   void free( T* );

   // Memomry allocation with call to constructor/destructor.
   T* construct();
   void destroy( T* );

   inline size_t  chunkSize() const { return _chunkSize; }

private:

   /*----- typed and enumerations -----*/

   union Data
   {
      Data* _next;
      uchar _data[sizeof(T)];
   };

   /*----- methods -----*/

   void allocateChunk();

   MemoryPool( const MemoryPool& );
   void operator=( const MemoryPool& );

   /*----- data members -----*/

   Data*           _free;
   size_t          _chunkSize;
   Vector< Data* > _chunks;
};

//------------------------------------------------------------------------------
//!
template< typename T >
MemoryPool<T>::MemoryPool( size_t chunkSize )
   : _free(0), _chunkSize( chunkSize )
{
   allocateChunk();
}

//------------------------------------------------------------------------------
//!
template< typename T >
MemoryPool<T>::~MemoryPool()
{
   // Release chunks.
   for( size_t i = 0; i < _chunks.size(); ++i )
   {
      ::free( _chunks[i] );
   }
}

//------------------------------------------------------------------------------
//!
template< typename T > void
MemoryPool<T>::clear()
{
   // Release chunks.
   for( size_t i = 1; i < _chunks.size(); ++i )
   {
      ::free( _chunks[i] );
   }
   _chunks.resize(1);
   // Construct free list.
   _free = _chunks.back();
   for( size_t i = 0; i < _chunkSize; ++i )
   {
      _free[i]._next = &_free[i+1];
   }
   _free[_chunkSize-1]._next = 0;
}

//------------------------------------------------------------------------------
//!
template< typename T > T*
MemoryPool<T>::alloc()
{
   if( !_free )
   {
      allocateChunk();
   }
   T* obj = (T*)_free->_data;
   _free  = _free->_next;
   return obj;
}

//------------------------------------------------------------------------------
//!
template< typename T > void
MemoryPool<T>::free( T* obj )
{
   Data* data  = reinterpret_cast<Data*>(obj);
   data->_next = _free;
   _free       = data;
}

//------------------------------------------------------------------------------
//!
template< typename T > T*
MemoryPool<T>::construct()
{
   return new( alloc() ) T();
}

//------------------------------------------------------------------------------
//!
template< typename T > void
MemoryPool<T>::destroy( T* obj )
{
   obj->~T();
   free( obj );
}

//------------------------------------------------------------------------------
//!
template< typename T > void
MemoryPool<T>::allocateChunk()
{
   // Allocate chunk memory.
   _chunks.push_back( (Data*)malloc( _chunkSize * sizeof(Data) ) );

   // Construct free list.
   _free = _chunks.back();
   for( size_t i = 0; i < _chunkSize; ++i )
   {
      _free[i]._next = &_free[i+1];
   }
   _free[_chunkSize-1]._next = 0;
}


NAMESPACE_END

#endif
