/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_RESOURCE_H
#define FUSION_RESOURCE_H

#include <Fusion/StdDefs.h>
#include <Fusion/Core/TaskEvent.h>
#include <Fusion/Core/Core.h>

#include <Base/MT/Lock.h>
#include <Base/Msg/DelegateList.h>
#include <Base/Util/RCObject.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Resource
==============================================================================*/

//! Only the method state( int ) and data should be called by multiple threads.
template< typename T >
class Resource:
   public RCObject
{
public:

   /*----- types and enumerations ----*/

   enum {
      UNLOADED = 0,
      LOADING  = 1,
      LOADED   = 2,
      CLOSING  = 3
   };

   typedef Delegate1<Resource*>  Callback;

   /*----- methods -----*/

   Resource();

   // Pointer to the real resource.
   void data( T* d )     { _data = d; state( LOADED ); }
   T* data()             { return _data.ptr(); }
   const T* data() const { return _data.ptr(); }

   // State.
   void state( int s );
   inline int state() const { return _state; }

   // Callbacks.
   void callOnLoad( const Callback& );
   void callOnClose( const Callback& );
   void removeOnLoad( const Callback& );
   void removeOnClose( const Callback& );

   bool isReady() const { return _state == LOADED; }

protected:

   Resource( const Resource& );
   Resource& operator=( const Resource& );

   virtual ~Resource();

   /*----- static data members -----*/

   static Lock _cbLock;

   /*----- data members -----*/

   int                       _state;
   RCP<T>                    _data;
   Delegate1List<Resource*>* _loadCbs;
   Delegate1List<Resource*>* _closeCbs;
};

template< typename T > Lock Resource<T>::_cbLock;

//------------------------------------------------------------------------------
//! 
template< typename T > T* data( const RCP< Resource<T> >& res )
{
   return res.isValid() ? res->data() : NULL;
}

//------------------------------------------------------------------------------
//! 
template< typename T > T* data( Resource<T>* res )
{
   return res ? res->data() : NULL;
}

/*==============================================================================
   CLASS ResEvent
==============================================================================*/

template< typename T >
class ResEvent: 
   public TaskEvent
{
public:

   ResEvent( Resource<T>* res,  Delegate1List<Resource<T>*>** cbs ): 
      _res( res ), _cbs( cbs ) {}

   virtual void execute()
   {
      (*_cbs)->exec( _res.ptr() );
      delete (*_cbs);
      *_cbs = 0;
   }

   RCP< Resource<T> >            _res;
   Delegate1List<Resource<T>*>** _cbs;
};

//------------------------------------------------------------------------------
//! 
template< typename T >
Resource<T>::Resource(): 
   _state( UNLOADED ),
   _data( NULL ),
   _loadCbs( NULL ),
   _closeCbs( NULL )
{}

//------------------------------------------------------------------------------
//!
template< typename T >
Resource<T>::~Resource()
{
   delete _loadCbs;
   delete _closeCbs;
}

//------------------------------------------------------------------------------
//! 
template< typename T > void 
Resource<T>::state( int s )
{
   LockGuard lock( _cbLock );
   if( _state == s ) return;
   _state = s;
   switch( _state )
   {
      case LOADED:
         if( _loadCbs ) Core::submitTaskEvent( new ResEvent<T>( this, &_loadCbs ) );
         break;
      case CLOSING:
         if( _closeCbs ) Core::submitTaskEvent( new ResEvent<T>( this, &_closeCbs ) );
         break;
      default:;
   }
}



//------------------------------------------------------------------------------
//! 
template< typename T > void 
Resource<T>::callOnLoad( const Callback& cb )
{
   bool exec = false;

   // Locked region for thread safety.
   {
      LockGuard lock( _cbLock );
      if( _state == LOADED ) exec = true;
      else
      {
         if( !_loadCbs ) _loadCbs = new Delegate1List<Resource*>();
         _loadCbs->addDelegate( cb );
      }
   }

   if( exec ) cb( this );
}
//------------------------------------------------------------------------------
//! 
template< typename T > void
Resource<T>::callOnClose( const Callback& cb )
{
   bool exec = false;

   // Locked region for thread safety.
   {
      LockGuard lock( _cbLock );
      if( _closeCbs ) _closeCbs->addDelegate( cb );
      else
      {
         if( _state == CLOSING ) exec = true;
         else
         {
            _closeCbs = new Delegate1List<Resource*>();
            _closeCbs->addDelegate( cb );
         }
      }
   }

   if( exec ) cb( this );
}

//------------------------------------------------------------------------------
//! 
template< typename T > void 
Resource<T>::removeOnLoad( const Callback& cb )
{
   LockGuard lock( _cbLock );
   if( _loadCbs ) _loadCbs->removeDelegate( cb );
}

//------------------------------------------------------------------------------
//! 
template< typename T > void 
Resource<T>::removeOnClose( const Callback& cb )
{
   LockGuard lock( _cbLock );
   if( _closeCbs ) _closeCbs->removeDelegate( cb );
}

NAMESPACE_END

#endif
