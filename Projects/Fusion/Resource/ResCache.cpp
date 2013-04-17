/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Resource/ResCache.h>

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN



UNNAMESPACE_END

//------------------------------------------------------------------------------
//!
ResCacheBase::CacheContainer  ResCacheBase::_caches;

//------------------------------------------------------------------------------
//!
Lock  ResCacheBase::_cachesLock;

//------------------------------------------------------------------------------
//!
ResCacheBase::ResCacheBase( const String& name ):
   _name( name )
{
   LockGuard lock( _cachesLock );
   _caches.pushBack( this );
}

//------------------------------------------------------------------------------
//!
ResCacheBase::~ResCacheBase()
{
   LockGuard lock( _cachesLock );
   _caches.removeSwap( this );
}

//------------------------------------------------------------------------------
//!
void
ResCacheBase::cleanAll( double t )
{
   LockGuard lock( _cachesLock );
   for( CacheContainer::Iterator cur = _caches.begin();
        cur != _caches.end();
        ++cur )
   {
      (*cur)->clean( t );
   }
}

//------------------------------------------------------------------------------
//!
void
ResCacheBase::clearAll()
{
   LockGuard lock( _cachesLock );
   for( CacheContainer::Iterator cur = _caches.begin();
        cur != _caches.end();
        ++cur )
   {
      (*cur)->clear();
   }
}
