/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_RES_CACHE_H
#define FUSION_RES_CACHE_H

#include <Fusion/StdDefs.h>

#include <Fusion/Resource/Resource.h>
#include <Fusion/Core/Core.h>

#include <CGMath/CGConst.h>

#include <Base/MT/Lock.h>
#include <Base/ADT/Map.h>
#include <Base/ADT/String.h>
#include <Base/Util/RCP.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS ResCacheBase
==============================================================================*/
class ResCacheBase
{
public:

   /*----- methods -----*/

   FUSION_DLL_API ResCacheBase( const String& name );
   FUSION_DLL_API virtual ~ResCacheBase();

   inline const String&  name() const { return _name; }

   FUSION_DLL_API virtual void  clean( double t ) = 0;
   FUSION_DLL_API virtual void  clear()           = 0;

   static FUSION_DLL_API void  cleanAll( double curTime );
   static FUSION_DLL_API void  clearAll();

protected:

   /*----- types -----*/

   typedef Vector< ResCacheBase* >  CacheContainer;

   /*----- static data members -----*/

   static CacheContainer  _caches;
   static Lock            _cachesLock;

   /*----- data members -----*/

   String  _name;

private:
}; //class ResCacheBase


/*==============================================================================
  CLASS ResCache
==============================================================================*/
template< typename T, typename K = String >
class ResCache:
   public ResCacheBase
{
public:

   /*----- methods -----*/

   ResCache( const String& name, double evictionDelay = 0.5 );
   virtual ~ResCache() {}

   inline double  evictionDelay() const     { return _evictionDelay; }
   inline void    evictionDelay( double d ) { _evictionDelay = d; }

   inline Resource<T>* get( const K& key );
   inline const K& get( const Resource<T>* res ) const;
   inline const K& get( const T* res ) const;

   Resource<T>* add( const K& key );
   void  remove( const K& key );

   void  clean( double t );
   void  clear();

protected:

   /*----- type and classes -----*/

   struct Entry
   {
      Entry() {}
      Entry( Resource<T>* res ): _res( res ), _timestamp( CGConstd::infinity() ) {}

      Resource<T>*  resource() const { return _res.ptr(); }
      bool  isResourceUnique() const { return _res.isUnique(); }
      bool  isDataUnique() const     { return _res->data() && _res->data()->isUnique(); }

      void  markUsed()               { _timestamp = CGConstd::infinity(); }
      void  markUnused( double t )   { _timestamp = t; }
      bool  markedUnused()           { return _timestamp != CGConstd::infinity(); }

      /*----- data members -----*/

      RCP< Resource<T> >  _res;        //!< The resource's pointer to the data.
      double              _timestamp;  //!< Timestamp.  +inf means it is in use, otherwise, it's the earliest unused timestamp.
   };

   typedef Map< K, Entry >                ForwardMap;
   typedef Map< const Resource<T>*, K >  BackwardMap;

   /*----- methods -----*/

   // Disallow copying.
   ResCache( const ResCache& );
   ResCache& operator=( const ResCache& );

   void removeLocked( const typename ForwardMap::Iterator& fit );

   inline bool  shouldEvict( const Entry& e, double t ) const { return (t - e._timestamp) >= _evictionDelay; }

   /*----- data members -----*/

   mutable Lock  _lock;
   ForwardMap    _fmap;
   BackwardMap   _bmap;
   double        _evictionDelay;

private:
   static K  _null;

}; //class ResCache

//------------------------------------------------------------------------------
//!
template< typename T, typename K > K ResCache<T,K>::_null = K();

//------------------------------------------------------------------------------
//!
template< typename T, typename K >
ResCache<T,K>::ResCache( const String& name, double evictionDelay ):
   ResCacheBase( name ),
   _evictionDelay( evictionDelay )
{}

////------------------------------------------------------------------------------
////!
//template< typename T, typename K >
//ResCache<T,K>::~ResCache()
//{}

//------------------------------------------------------------------------------
//!
template< typename T, typename K > Resource<T>*
ResCache<T,K>::get( const K& key )
{
   LockGuard lock( _lock );
   typename ForwardMap::Iterator result = _fmap.find( key );
   if( result != _fmap.end() )
   {
      (*result).second.markUsed();
      return (*result).second.resource();
   }
   return NULL;
}

//------------------------------------------------------------------------------
//!
template< typename T, typename K > const K&
ResCache<T,K>::get( const Resource<T>* res ) const
{
   LockGuard lock( _lock );
   typename BackwardMap::ConstIterator result = _bmap.find( res );
   if( result != _bmap.end() )  return (*result).second;
   return _null;
}

//------------------------------------------------------------------------------
//!
template< typename T, typename K > const K&
ResCache<T,K>::get( const T* data ) const
{
   LockGuard lock( _lock );
   typename BackwardMap::ConstIterator result;

   for( result = _bmap.begin(); result != _bmap.end(); ++result )
   {
      if( (*result).first->data() == data ) return (*result).second;
   }

   return _null;
}

//------------------------------------------------------------------------------
//!
template< typename T, typename K > Resource<T>*
ResCache<T,K>::add( const K& key )
{
   LockGuard lock( _lock );
   Resource<T>* res = new Resource<T>();
   res->state( Resource<T>::LOADING );
   _fmap[ key ] = Entry( res );
   _bmap[ res ] = key;
   return res;
}

//------------------------------------------------------------------------------
//!
template< typename T, typename K > void
ResCache<T,K>::remove( const K& key )
{
   LockGuard lock( _lock );
   typename ForwardMap::Iterator fit = _fmap.find( key );
   if( fit != _fmap.end() )
   {
      removeLocked( fit );
   }
}

//------------------------------------------------------------------------------
//!
template< typename T, typename K > void
ResCache<T,K>::removeLocked( const typename ForwardMap::Iterator& fit )
{
   typename BackwardMap::Iterator bit = _bmap.find( (*fit).second.resource() );
   _bmap.erase( bit );
   _fmap.erase( fit );
}

//------------------------------------------------------------------------------
//!
template< typename T, typename K > void
ResCache<T,K>::clean( double t )
{
   LockGuard lock( _lock );

   typename ForwardMap::Iterator cur = _fmap.begin();
   while( cur != _fmap.end() )
   {
      Entry& entry = (*cur).second;

      if( entry.isResourceUnique() && entry.isDataUnique() )
      {
         if( !entry.markedUnused() )  entry.markUnused(t);
         if( shouldEvict(entry, t) )
         {
            StdErr << "Evicting '" << (*cur).first << nl;
            removeLocked( cur++ );
            continue; // Don't call ++cur below.
         }
      }
      ++cur;
   }
}

//------------------------------------------------------------------------------
//!
template< typename T, typename K > void
ResCache<T,K>::clear()
{
   LockGuard lock( _lock );
   _fmap.clear();
   _bmap.clear();
}

NAMESPACE_END

#endif //FUSION_RES_CACHE_H
