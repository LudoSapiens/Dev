/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_CACHE_H
#define BASE_CACHE_H

#include <Base/StdDefs.h>

#include <Base/ADT/HashTable.h>
#include <Base/ADT/MemoryPool.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS LRUCache
==============================================================================*/
//!< Implements a cache with least-recently-used eviction policy.
//!< The cache uses a circular list of CacheNode, which contain next, prev, and
//!< the data through a HashTable iterator (currently 3ptrs, which is likely
//!< better than having our own copy of key, which can be larger).
template< typename Key, typename Data, typename Hash = StdHash<Key>, typename Pred = std::equal_to<Key> >
class LRUCache
{
public:

   /*----- methods -----*/

   LRUCache( size_t numEntries );
   LRUCache( size_t numEntries, size_t hashTableSize );
   ~LRUCache();

   //------------------------------------------------------------------------------
   //! Returns the total number of elements that the cache can contain.
   size_t  capacity() const;

   //------------------------------------------------------------------------------
   //! Returns the total number of elements that the cache currently contains.
   size_t  size() const;

   //------------------------------------------------------------------------------
   //! Retrieves a valid data memory, and returns whether or not it had to create it.
   //! If the data was already present in the cache, its timestamp will get updated.
   //! If the data was not present, the default constructor is used for the data.
   bool  get( const Key& key, Data*& data );

   //------------------------------------------------------------------------------
   //! Returns a pointer to the cached data (NULL if the data isn't cached).
   Data*  find( const Key& key );

   //------------------------------------------------------------------------------
   //! Caches the specified data with the associated key.
   //! If you can guarantee that the key isn't cached, set safe to false for faster code.
   Data*  set( const Key& key, const Data& data );

   //------------------------------------------------------------------------------
   //! Adds the specified data for the associated key.
   //! This should only be called for keys that are known not to exist in the cache.
   //! For a safer alternative, use @see set(key, data).
   Data*  add( const Key& key, const Data& data );

   //------------------------------------------------------------------------------
   //! Refreshes the timestamp for the specified key.
   void  touch( const Key& key );

   //------------------------------------------------------------------------------
   //! Clear the whole cache.
   LRUCache&  clear();

   //------------------------------------------------------------------------------
   //! Removes a specific element from the cache.
   LRUCache&  erase( const Key& key );

   //------------------------------------------------------------------------------
   //! Prints the keys of all of the elements currently contained.
   void  print( TextStream& os = StdErr ) const;

protected:

   /*----- types -----*/

   struct CacheNode;
   typedef HashTable<Key, CacheNode*, Hash, Pred>  HashTableType;
   typedef typename HashTableType::Iterator        HashTableIterator;
   struct CacheNode
   {
      CacheNode*         _next;  //!< The next node.
      CacheNode*         _prev;  //!< The previous node.
      HashTableIterator  _it;    //!< An iterator pointing to the hash table entry (for the key).
      Data               _data;  //!< The associated data.
   };


   /*----- data members -----*/

   HashTableType          _entries;   //!< The entries in the cache.
   MemoryPool<CacheNode>  _nodePool;  //!< A pool for the cache nodes (better locality).
   CacheNode*             _nodes;     //!< The nodes as a circular list (element to evict is the last).
   size_t                 _nNodes;    //!< The number of elements currently in the cache.


   /*----- methods -----*/

   void  disconnectFast( CacheNode* node );
   void  disconnect( CacheNode* node );
   void  connectFast( CacheNode* node );
   void  connect( CacheNode* node );
   void  touchNode( CacheNode* node );
   void  evict( CacheNode* node );
   void  evictOnFull();
   bool  full() const;

private:
}; //class LRUCache


//------------------------------------------------------------------------------
//!
template< typename K, typename D, typename H, typename P >
LRUCache<K,D,H,P>::LRUCache( size_t numEntries ):
   _entries( numEntries*2 ),
   _nodePool( numEntries ),
   _nodes( NULL ),
   _nNodes( 0 )
{
}

//------------------------------------------------------------------------------
//!
template< typename K, typename D, typename H, typename P >
LRUCache<K,D,H,P>::LRUCache( size_t numEntries, size_t hashTableSize ):
   _entries( hashTableSize ),
   _nodePool( numEntries ),
   _nodes( NULL ),
   _nNodes( 0 )
{
}

//------------------------------------------------------------------------------
//!
template< typename K, typename D, typename H, typename P >
LRUCache<K,D,H,P>::~LRUCache()
{
   clear();
}

//------------------------------------------------------------------------------
//!
template< typename K, typename D, typename H, typename P >
inline size_t
LRUCache<K,D,H,P>::capacity() const
{
   return _nodePool.chunkSize();
}

//------------------------------------------------------------------------------
//!
template< typename K, typename D, typename H, typename P >
inline size_t
LRUCache<K,D,H,P>::size() const
{
   return _nNodes;
}

//------------------------------------------------------------------------------
//!
template< typename K, typename D, typename H, typename P >
inline bool
LRUCache<K,D,H,P>::get( const K& key, D*& data )
{
   HashTableIterator it = _entries.find( key );
   if( it != _entries.end() )
   {
      CacheNode* node = it.data();
      touchNode( node );
      data = &(node->_data);
      return true;
   }
   else
   {
      data = add( key, D() );
      return false;
   }
}

//------------------------------------------------------------------------------
//!
template< typename K, typename D, typename H, typename P >
inline D*
LRUCache<K,D,H,P>::find( const K& key )
{
   HashTableIterator it = _entries.find( key );
   if( it != _entries.end() )
   {
      return &(it.data()->_data);
   }
   else
   {
      return NULL;
   }
}

//------------------------------------------------------------------------------
//!
template< typename K, typename D, typename H, typename P >
inline D*
LRUCache<K,D,H,P>::set( const K& key, const D& data )
{
   // Need to check if the entry was already in the cache or not.
   // (setting safe=false avoids hashing the key twice, once in find() and once add())
   HashTableIterator it = _entries.find( key );
   if( it != _entries.end() )
   {
      // Just update data.
      it.data()->_data = data;
      touchNode( it.data() );
      return &(it.data()->_data);
   }
   // Else create a new entry.
   return add( key, data );
}

//------------------------------------------------------------------------------
//!
template< typename K, typename D, typename H, typename P >
inline D*
LRUCache<K,D,H,P>::add( const K& key, const D& data )
{
   if( full() )
   {
      // Need to make space.
      evict( _nodes );
   }
   CacheNode* node = _nodePool.construct();
   node->_data = data;
   node->_it   = _entries.add( key, node );
   ++_nNodes;
   connect( node );
   return &node->_data;
}

//------------------------------------------------------------------------------
//!
template< typename K, typename D, typename H, typename P >
inline void
LRUCache<K,D,H,P>::touch( const K& key )
{
   CacheNode* node = _entries[key];
   if( node )
   {
      touchNode( node );
   }
   else
   {
      StdErr << "LRUCache::touch(key) called with a non-existent key" << nl;
   }
}

//------------------------------------------------------------------------------
//!
template< typename K, typename D, typename H, typename P >
LRUCache<K,D,H,P>&
LRUCache<K,D,H,P>::clear()
{
   // Need to clear all of the entries.
   if( _nodes != NULL )
   {
      _entries.clear();
      CacheNode* tmp = _nodes;
      do
      {
         tmp = tmp->_prev; // Need to assign before destroying, else memory is unallocated.
         _nodePool.destroy( tmp );
      } while( tmp != _nodes );
      _nodes = NULL;
      _nNodes = 0;
   }
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename K, typename D, typename H, typename P >
LRUCache<K,D,H,P>&
LRUCache<K,D,H,P>::erase( const K& key )
{
   HashTableIterator it = _entries.find( key );
   if( it != _entries.end() )
   {
      CacheNode* node = it.data();
      disconnect( node );
      --_nNodes;
      _entries.erase( node->_it );
      _nodePool.destroy( node );
   }
   else
   {
      StdErr << "LRUCache::erase(key) called with a non-existent key" << nl;
   }
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename K, typename D, typename H, typename P >
void
LRUCache<K,D,H,P>::print( TextStream& os ) const
{
   os << "LRUCache of " << _nNodes << " entries:" << nl;
   os << "--- MRU ---" << nl;
   if( _nodes != NULL )
   {
      CacheNode* tmp = _nodes;
      do
      {
         tmp = tmp->_prev;
         StdErr << "  " << tmp->_it.key() << nl;
      } while( tmp != _nodes );
   }
   os << "--- LRU ---" << nl;
}

//------------------------------------------------------------------------------
//!
template< typename K, typename D, typename H, typename P >
inline void
LRUCache<K,D,H,P>::disconnectFast( CacheNode* node )
{
   node->_prev->_next = node->_next;
   node->_next->_prev = node->_prev;
}

//------------------------------------------------------------------------------
//!
template< typename K, typename D, typename H, typename P >
inline void
LRUCache<K,D,H,P>::disconnect( CacheNode* node )
{
   CHECK( node );
   if( node->_next != node )
   {
      if( _nodes == node )
      {
         // If this is the first, we need a new first.
         _nodes = node->_next;
      }
      disconnectFast( node );
   }
   else
   {
      // There was only one element.
      _nodes = NULL;
   }
}

//------------------------------------------------------------------------------
//!
template< typename K, typename D, typename H, typename P >
inline void
LRUCache<K,D,H,P>::connectFast( CacheNode* node )
{
   // Connect as the last element.
   node->_prev = _nodes->_prev;
   node->_next = _nodes;
   _nodes->_prev->_next = node;
   _nodes->_prev = node;
}

//------------------------------------------------------------------------------
//!
template< typename K, typename D, typename H, typename P >
inline void
LRUCache<K,D,H,P>::connect( CacheNode* node )
{
   if( _nodes != NULL )
   {
      connectFast( node );
   }
   else
   {
      _nodes = node;
      node->_prev = node->_next = node;
   }
}

//------------------------------------------------------------------------------
//!
template< typename K, typename D, typename H, typename P >
inline void
LRUCache<K,D,H,P>::touchNode( CacheNode* node )
{
   CHECK( _nodes );
   if( _nodes->_prev != node )
   {
      // Need to place node as the last element.
      disconnect( node );
      connect( node );
   }
   // Else the element was already the most recent, so don't do anything.
}

//------------------------------------------------------------------------------
//!
template< typename K, typename D, typename H, typename P >
inline void
LRUCache<K,D,H,P>::evict( CacheNode* node )
{
   CHECK( node == _nodes );
   CHECK( node != NULL );
   disconnect( node );
   --_nNodes;
   _entries.erase( node->_it );
   _nodePool.destroy( node );
}

//------------------------------------------------------------------------------
//!
template< typename K, typename D, typename H, typename P >
inline bool
LRUCache<K,D,H,P>::full() const
{
   return _nNodes >= capacity();
}


NAMESPACE_END

#endif //BASE_CACHE_H
