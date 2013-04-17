/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/MT/Lock.h>

NAMESPACE_BEGIN

#ifdef _WIN32

/*==============================================================================
   CLASS LockImp for _WIN32
==============================================================================*/

#include <Base/Util/windows.h>

class Lock::LockImp
{

public: 

   /*----- methods -----*/

   LockImp()
   {
      InitializeCriticalSection( &_section );
   }
   
   ~LockImp()
   {
      DeleteCriticalSection( &_section );
   }

   void lock()
   {
      EnterCriticalSection( &_section );
   }
   
   void unlock()
   {
      LeaveCriticalSection( &_section );
   }

private: 

   /*----- data members -----*/

   CRITICAL_SECTION _section;
};

#else

/*==============================================================================
   CLASS LockImp for POSIX
==============================================================================*/

#include <pthread.h>

class Lock::LockImp
{

public: 

   /*----- methods -----*/

   LockImp()
   {
      pthread_mutex_init( &_mutex, 0 );
   }
   
   ~LockImp()
   {
      pthread_mutex_destroy( &_mutex );
   }

   void lock()
   {
      pthread_mutex_lock( &_mutex );
   }
   
   void unlock()
   {
      pthread_mutex_unlock( &_mutex );
   }

private: 

   /*----- data members -----*/

   pthread_mutex_t _mutex;

};

#endif


/*==============================================================================
   CLASS Lock
==============================================================================*/

//------------------------------------------------------------------------------
//!
Lock::Lock() :
   _imp( new LockImp )
{
}

//------------------------------------------------------------------------------
//!
Lock::~Lock()
{
   delete _imp;
}

//------------------------------------------------------------------------------
//!
void
Lock::lock()
{
   _imp->lock();
}

//------------------------------------------------------------------------------
//!
void
Lock::unlock()
{
   _imp->unlock();
}

NAMESPACE_END
