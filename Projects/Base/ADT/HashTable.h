/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_HASH_TABLE_H
#define BASE_HASH_TABLE_H

#include <Base/StdDefs.h>

#include <Base/ADT/MemoryPool.h>
#include <Base/ADT/Pair.h>
#include <Base/ADT/Vector.h>
#include <Base/Dbg/Defs.h>
#include <Base/IO/TextStream.h>
#include <Base/Util/Hash.h>

#include <functional>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS HashTable
==============================================================================*/
//!< An implementation of a hash table.
//!< Description of template parameters:
//!<   Key:  The type of the key.
//!<   Data: The type of data contained in the HashTable.
//!<   Hash: The hash function converting the Key into a size_t hash index.
//!<   Pred: A comparison function comparing two Keys (to resolve hash conflicts).
//!< This class implements a chaining hash table.
template< typename Key, typename Data, typename Hash = StdHash<Key>, typename Pred = std::equal_to<Key> >
class HashTable
{
   /*----- types -----*/

protected:
   struct HashTableEntry;

public:

   typedef Pair<Key, Data>  KeyData;

   class Iterator;
   class ConstIterator;

   /*----- methods -----*/

   HashTable( size_t cap = 8, float minLoadFactor = 0.25f, float maxLoadFactor = 0.70f );
   ~HashTable();

   size_t  capacity() const;

   size_t  count() const;

         Data&  operator[]( const Key& key );
   const Data&  operator[]( const Key& key ) const;

   bool  has( const Key& key ) const;

        Iterator  find( const Key& key );
   ConstIterator  find( const Key& key ) const;

   Iterator  add( const Key& key, const Data& data );

   HashTable&  clear();
   HashTable&  merge( const HashTable& t );

   HashTable&  erase( const Key& key );
   HashTable&  erase( const Iterator& it );

        Iterator  begin();
   ConstIterator  begin() const;

        Iterator  end();
   ConstIterator  end() const;

   HashTable&  rehash( size_t cap );

   float  minLoadFactor() const;
   void   minLoadFactor( float f );

   float  maxLoadFactor() const;
   void   maxLoadFactor( float f );

   void   loadFactors( float minF, float maxF );

   float  currentLoadFactor() const;

   void   print( TextStream& os = StdErr, bool compact = true ) const;

   /*==============================================================================
     CLASS Iterator
   ==============================================================================*/
   class Iterator
   {
   public:

      /*----- methods -----*/
      Iterator() { }

      const Key&  key() const { return _entry->_keyData.first; }
            Data&  data()       { return _entry->_keyData.second; }
      const Data&  data() const { return _entry->_keyData.second; }

      const KeyData& operator*() const { return _entry->_keyData; }

      Iterator& operator++()
      {
         if( _entry != NULL )
         {
            _entry = _entry->_next;
            if( _entry == NULL ) nextNonNull();
         }
         // else don't increment end.
         return *this;
      }
      Iterator operator++(int) { Iterator tmp(*this); ++(*this); return tmp; }

      bool  operator==( const Iterator& it ) const { return _entry == it._entry; }
      bool  operator!=( const Iterator& it ) const { return !( this->operator==(it) ); }

      TextStream&  print( TextStream& os ) const
      {
         return os << "{_table=" << _table << ",_index=" << _index << ",_entry=" << _entry << "}";
      }

      TextStream&  operator<<( TextStream& os ) const
      {
         return print(os);
      }

   protected:

      friend class HashTable;

      /*----- data members -----*/

      HashTable*       _table;
      size_t           _index;
      HashTableEntry*  _entry;

      /*----- methods -----*/

      Iterator( HashTable* table, size_t index, HashTableEntry* entry ):
         _table(table), _index(index), _entry(entry) { }

      void nextNonNull()
      {
         // Find next non-NULL table slot.
         const size_t n = _table->_table.size();
         do
         {
            ++_index;
            if( _index >= n )
            {
               _entry = NULL;
               return;
            }
            _entry = _table->_table[_index];
         } while( _entry == NULL );
      }

   private:
   }; //class Iterator

   /*==============================================================================
     CLASS ConstIterator
   ==============================================================================*/
   class ConstIterator
   {
   public:

      /*----- methods -----*/
      ConstIterator() { }

      const Key&  key() const { return _entry->_keyData.first; }
      const Data&  data() const { return _entry->_keyData.second; }

      const KeyData& operator*() const { return _entry->_keyData; }

      ConstIterator& operator++()
      {
         if( _entry != NULL )
         {
            _entry = _entry->_next;
            if( _entry == NULL ) nextNonNull();
         }
         // else don't increment end.
         return *this;
      }
      ConstIterator operator++(int) { ConstIterator tmp(*this); ++(*this); return tmp; }

      bool  operator==( const ConstIterator& it ) const { return _entry == it._entry; }
      bool  operator!=( const ConstIterator& it ) const { return !( this->operator==(it) ); }

      TextStream&  print( TextStream& os ) const
      {
         return os << "{_table=" << _table << ",_index=" << _index << ",_entry=" << _entry << "}";
      }

      TextStream&  operator<<( TextStream& os ) const
      {
         return print(os);
      }

   protected:

      friend class HashTable;

      /*----- data members -----*/

      const HashTable*       _table;
      size_t                 _index;
      const HashTableEntry*  _entry;

      /*----- methods -----*/

      ConstIterator( const HashTable* table, size_t index, const HashTableEntry* entry ):
         _table(table), _index(index), _entry(entry) { }

      void nextNonNull()
      {
         // Find next non-NULL table slot.
         const size_t n = _table->_table.size();
         do
         {
            ++_index;
            if( _index >= n )
            {
               _entry = NULL;
               return;
            }
            _entry = _table->_table[_index];
         } while( _entry == NULL );
      }

   private:
   }; //class ConstIterator

   // DEBUG INTERFACE

   //------------------------------------------------------------------------------
   //! Returns the number of entries for each hash value.
   size_t  getUsageHistogram( Vector<size_t>& histogram ) const;

protected:

   friend class Iterator;
   friend class ConstIterator;

   /*----- types -----*/

   struct HashTableEntry
   {
      KeyData          _keyData;
      HashTableEntry*  _next;
   };

   typedef Vector<HashTableEntry*>  HashTableVector;

   /*----- data members -----*/

   HashTableVector             _table;
   MemoryPool<HashTableEntry>  _pool;
   float                       _minLoadFactor;
   float                       _maxLoadFactor;
   size_t                      _minThreshold;
   size_t                      _maxThreshold;
   size_t                      _count;  //!< The total number of elements currently present.

   /*----- methods -----*/

         HashTableEntry*  findEntry( const Key& key, size_t& index );
   const HashTableEntry*  findEntry( const Key& key, size_t& index ) const;

   HashTable&  erase( size_t index, HashTableEntry* entry );

   inline size_t  keyToIndex( const Key& key ) const { return Hash()(key) % _table.size(); }

   void  clampMinLoadFactor();
   void  refreshThresholds();

private:
}; //class HashTable

//------------------------------------------------------------------------------
//!
template< typename Key, typename Data, typename Hash, typename Pred >
HashTable<Key, Data, Hash, Pred>::HashTable( size_t cap, float minLoadFactor, float maxLoadFactor ):
   _minLoadFactor( minLoadFactor ),
   _maxLoadFactor( maxLoadFactor ),
   _count( 0 )
{
   clampMinLoadFactor();
   _table.resize( cap, NULL );
   refreshThresholds();
}

//------------------------------------------------------------------------------
//!
template< typename Key, typename Data, typename Hash, typename Pred >
HashTable<Key, Data, Hash, Pred>::~HashTable()
{
   clear();
}

//------------------------------------------------------------------------------
//!
template< typename Key, typename Data, typename Hash, typename Pred >
inline size_t
HashTable<Key, Data, Hash, Pred>::capacity() const
{
   return _table.size();
}

//------------------------------------------------------------------------------
//!
template< typename Key, typename Data, typename Hash, typename Pred >
inline size_t
HashTable<Key, Data, Hash, Pred>::count() const
{
   return _count;
}

//------------------------------------------------------------------------------
//!
template< typename Key, typename Data, typename Hash, typename Pred >
inline Data&
HashTable<Key, Data, Hash, Pred>::operator[]( const Key& key )
{
   size_t index;
   HashTableEntry* entry = findEntry( key, index );
   if( entry == NULL )
   {
      // We know it's not there, so simply add it to the table.
      if( ++_count > _maxThreshold )
      {
         rehash( _table.size() << 1 );
         index = keyToIndex( key );
      }
      entry                 = _pool.construct();
      entry->_keyData.first = key;
      entry->_next          = _table[index];
      _table[index]         = entry;
      // Here, the data is uninitialized.
   }
   // else, we're pointing to it already.
   return entry->_keyData.second;
}

//------------------------------------------------------------------------------
//!
template< typename Key, typename Data, typename Hash, typename Pred >
inline typename HashTable<Key, Data, Hash, Pred>::Iterator
HashTable<Key, Data, Hash, Pred>::add( const Key& key, const Data& data )
{
   size_t index;
   HashTableEntry* entry = findEntry( key, index );
   if( entry == NULL )
   {
      // We know it's not there, so simply add it to the table.
      if( ++_count > _maxThreshold )
      {
         rehash( _table.size() << 1 );
         index = keyToIndex( key );
      }
      entry                 = _pool.construct();
      entry->_keyData.first = key;
      entry->_next          = _table[index];
      _table[index]         = entry;
   }
   // else, we're pointing to it already.

   // Assign or update the data.
   entry->_keyData.second = data;

   // Return an iterator pointing to it.
   return Iterator( this, index, entry );
}

//------------------------------------------------------------------------------
//!
template< typename Key, typename Data, typename Hash, typename Pred >
inline bool
HashTable<Key, Data, Hash, Pred>::has( const Key& key ) const
{
   size_t tmp;
   return findEntry( key, tmp ) != NULL;
}

//------------------------------------------------------------------------------
//!
template< typename Key, typename Data, typename Hash, typename Pred >
inline typename HashTable<Key, Data, Hash, Pred>::Iterator
HashTable<Key, Data, Hash, Pred>::find( const Key& key )
{
   size_t index;
   HashTableEntry* entry = findEntry( key, index );
   if( entry != NULL )
   {
      return Iterator( this, index, entry );
   }
   else
   {
      return end();
   }
}

//------------------------------------------------------------------------------
//!
template< typename Key, typename Data, typename Hash, typename Pred >
inline typename HashTable<Key, Data, Hash, Pred>::ConstIterator
HashTable<Key, Data, Hash, Pred>::find( const Key& key ) const
{
   size_t index;
   const HashTableEntry* entry = findEntry( key, index );
   if( entry != NULL )
   {
      return ConstIterator( this, index, entry );
   }
   else
   {
      return end();
   }
}

//------------------------------------------------------------------------------
//!
template< typename Key, typename Data, typename Hash, typename Pred >
inline HashTable<Key, Data, Hash, Pred>&
HashTable<Key, Data, Hash, Pred>::clear( void )
{
   // We need to deallocate memory pool entries.
   size_t n = _table.size();
   for( uint i = 0; i < n; ++i )
   {
      HashTableEntry* entry = _table[i];
      while( entry != NULL )
      {
         HashTableEntry* tmp = entry;
         entry = entry->_next;
         _pool.destroy( tmp );
      }
      _table[i] = NULL;
      _count = 0;
   }
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename Key, typename Data, typename Hash, typename Pred >
inline HashTable<Key, Data, Hash, Pred>&
HashTable<Key, Data, Hash, Pred>::erase( size_t index, HashTableEntry* entry )
{
   if( entry != NULL )
   {
      if( _table[index] == entry )
      {
         // It is the first element.
         _table[index] = entry->_next;
      }
      else
      {
         // Not the first, so need to jump from previous to next.
         HashTableEntry* prev = _table[index];
         while( prev->_next != entry )
         {
            prev = prev->_next;
            CHECK( prev != NULL );
         }
         prev->_next = entry->_next;
      }
      _pool.destroy( entry );
      if( --_count < _minThreshold )  rehash( _table.size() >> 1 );
   }
   // else, it isn't even there in the first place...
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename Key, typename Data, typename Hash, typename Pred >
inline HashTable<Key, Data, Hash, Pred>&
HashTable<Key, Data, Hash, Pred>::erase( const Key& key )
{
   size_t index;
   HashTableEntry* entry = findEntry( key, index );
   return erase( index, entry );
}

//------------------------------------------------------------------------------
//!
template< typename Key, typename Data, typename Hash, typename Pred >
inline HashTable<Key, Data, Hash, Pred>&
HashTable<Key, Data, Hash, Pred>::erase( const Iterator& it )
{
   return erase( it._index, it._entry );
}

//------------------------------------------------------------------------------
//!
template< typename Key, typename Data, typename Hash, typename Pred >
inline HashTable<Key, Data, Hash, Pred>&
HashTable<Key, Data, Hash, Pred>::merge( const HashTable<Key, Data, Hash, Pred>& t )
{
   typename HashTableVector::ConstIterator cur = t._table.begin();
   typename HashTableVector::ConstIterator end = t._table.end();
   while( cur != end )
   {
      HashTableEntry* entry = (*cur);
      while( entry != NULL )
      {
         // Create a new entry (or update the previous one).
         (*this)[entry->_keyData.first] = entry->_keyData.second;
         entry = entry->_next;
      }
      ++cur;
   }
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename Key, typename Data, typename Hash, typename Pred >
inline typename HashTable<Key, Data, Hash, Pred>::Iterator
HashTable<Key, Data, Hash, Pred>::begin()
{
   Iterator it( this, 0, _table[0] );
   if( it._entry == NULL ) it.nextNonNull();
   return it;
}

//------------------------------------------------------------------------------
//!
template< typename Key, typename Data, typename Hash, typename Pred >
inline typename HashTable<Key, Data, Hash, Pred>::ConstIterator
HashTable<Key, Data, Hash, Pred>::begin() const
{
   ConstIterator it( this, 0, _table[0] );
   if( it._entry == NULL ) it.nextNonNull();
   return it;
}

//------------------------------------------------------------------------------
//!
template< typename Key, typename Data, typename Hash, typename Pred >
inline typename HashTable<Key, Data, Hash, Pred>::Iterator
HashTable<Key, Data, Hash, Pred>::end()
{
   return Iterator( this, _table.size(), NULL );
}

//------------------------------------------------------------------------------
//!
template< typename Key, typename Data, typename Hash, typename Pred >
inline typename HashTable<Key, Data, Hash, Pred>::ConstIterator
HashTable<Key, Data, Hash, Pred>::end() const
{
   return ConstIterator( this, _table.size(), NULL );
}

//------------------------------------------------------------------------------
//!
template< typename Key, typename Data, typename Hash, typename Pred >
inline typename HashTable<Key, Data, Hash, Pred>::HashTableEntry*
HashTable<Key, Data, Hash, Pred>::findEntry( const Key& key, size_t& index )
{
   index = keyToIndex( key );
   HashTableEntry* entry = _table[index];
   while( entry != NULL && !Pred()(key, entry->_keyData.first) )
   {
      entry = entry->_next;
   }
   return entry;
}

//------------------------------------------------------------------------------
//!
template< typename Key, typename Data, typename Hash, typename Pred >
inline const typename HashTable<Key, Data, Hash, Pred>::HashTableEntry*
HashTable<Key, Data, Hash, Pred>::findEntry( const Key& key, size_t& index ) const
{
   index = keyToIndex( key );
   const HashTableEntry* entry = _table[index];
   while( entry != NULL && !Pred()(key, entry->_keyData.first) )
   {
      entry = entry->_next;
   }
   return entry;
}

//------------------------------------------------------------------------------
//!
template< typename Key, typename Data, typename Hash, typename Pred >
HashTable<Key, Data, Hash, Pred>&
HashTable<Key, Data, Hash, Pred>::rehash( size_t cap )
{
   if( cap == 0 )  cap = 1;  // Need at least a size of 1.
   HashTableVector  tmp( cap, NULL );
   tmp.swap( _table );  // This makes the current hash table empty, with tmp pointing to old data.
   refreshThresholds();
   typename HashTableVector::ConstIterator cur, end;
   for( cur = tmp.begin(), end = tmp.end(); cur != end; ++cur )
   {
      HashTableEntry* oldEntry = *cur;
      while( oldEntry != NULL )
      {
         // Rehash the key.
         size_t newIdx = keyToIndex( oldEntry->_keyData.first );
         // Save next entry from old table (since we reuse oldEntry in the new table).
         HashTableEntry* tmpEntry  = oldEntry->_next;
         // Store old entry at the beginning of the new table's bucket.
         HashTableEntry*& newEntry = _table[newIdx];
         oldEntry->_next           = newEntry;
         newEntry                  = oldEntry;
         // Prepare next entry from the old table.
         oldEntry                  = tmpEntry;
      }
   }
   return *this;
}

//------------------------------------------------------------------------------
//!
template< typename Key, typename Data, typename Hash, typename Pred >
void
HashTable<Key, Data, Hash, Pred>::print( TextStream& os, bool compact ) const
{
   size_t n = _table.size();
   os << "HashTable (" << (void*)this << ") of size " << n << " with " << _count << " elements." << nl;
   os << "LoadFactors: min=" << _minLoadFactor << " (" << _minThreshold << ") max=" << _maxLoadFactor << " (" << _maxThreshold << ") cur=" << currentLoadFactor() << nl;
   for( uint i = 0; i < n; ++i )
   {
      HashTableEntry* entry = _table[i];
      if( compact && entry == NULL )  continue;
      os << "  [" << i << "]:";
      while( entry != NULL )
      {
         os << " {" << entry->_keyData.first << "}='" << entry->_keyData.second << "'";
         entry = entry->_next;
      }
      os << nl;
   }
}

//------------------------------------------------------------------------------
//! Returns the number of entries for each hash value.
template< typename Key, typename Data, typename Hash, typename Pred >
size_t
HashTable<Key, Data, Hash, Pred>::getUsageHistogram( Vector<size_t>& histogram ) const
{
   histogram.clear();
   size_t total = 0;
   size_t n     = _table.size();
   histogram.resize( n, 0 );
   for( size_t i = 0; i < n; ++i )
   {
      HashTableEntry* entry = _table[i];
      while( entry != NULL )
      {
         ++histogram[i];
         ++total;
         entry = entry->_next;
      }
   }
   return total;
}

//------------------------------------------------------------------------------
//! Returns the minimum load factor.
template< typename Key, typename Data, typename Hash, typename Pred >
float
HashTable<Key, Data, Hash, Pred>::minLoadFactor() const
{
   return _minLoadFactor;
}

//------------------------------------------------------------------------------
//! Sets the minimum load factor.
template< typename Key, typename Data, typename Hash, typename Pred >
void
HashTable<Key, Data, Hash, Pred>::minLoadFactor( float f )
{
   _minLoadFactor = f;
   clampMinLoadFactor();
   refreshThresholds();
}

//------------------------------------------------------------------------------
//! Returns the maximum load factor.
template< typename Key, typename Data, typename Hash, typename Pred >
float
HashTable<Key, Data, Hash, Pred>::maxLoadFactor() const
{
   return _maxLoadFactor;
}

//------------------------------------------------------------------------------
//! Sets the maximum load factor.
template< typename Key, typename Data, typename Hash, typename Pred >
void
HashTable<Key, Data, Hash, Pred>::maxLoadFactor( float f )
{
   _maxLoadFactor = f;
   refreshThresholds();
}

//------------------------------------------------------------------------------
//! Set the minimum and maximum load factors.
template< typename Key, typename Data, typename Hash, typename Pred >
void
HashTable<Key, Data, Hash, Pred>::loadFactors( float minF, float maxF )
{
   _minLoadFactor = minF;
   _maxLoadFactor = maxF;
   clampMinLoadFactor();
   refreshThresholds();
}

//------------------------------------------------------------------------------
//! Computes the current load factor (count/capacity).
template< typename Key, typename Data, typename Hash, typename Pred >
inline float
HashTable<Key, Data, Hash, Pred>::currentLoadFactor() const
{
   return (float)count() / capacity();
}

//------------------------------------------------------------------------------
//! Avoid thrashing the table by guaranteeing some leeway between min and max load factors.
template< typename Key, typename Data, typename Hash, typename Pred >
inline void
HashTable<Key, Data, Hash, Pred>::clampMinLoadFactor()
{
   float maxF_2 = _maxLoadFactor * 0.5f;
   if( _minLoadFactor > maxF_2 )  _minLoadFactor = maxF_2;
}

//------------------------------------------------------------------------------
//! Updates the private threshold values.
template< typename Key, typename Data, typename Hash, typename Pred >
void
HashTable<Key, Data, Hash, Pred>::refreshThresholds()
{
   _minThreshold = (size_t)(_table.size() * _minLoadFactor);
   _maxThreshold = (size_t)(_table.size() * _maxLoadFactor);
}


NAMESPACE_END

#endif //BASE_HASH_TABLE_H
