/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/MT/RWLock.h>
#include <Base/MT/Lock.h>
#include <Base/MT/Atomic.h>

NAMESPACE_BEGIN

#ifdef _WIN32

/*==============================================================================
   CLASS RWLockImp for WIN32
==============================================================================*/

//! Implementation based on PTypes library.

#include <Base/Util/windows.h>

class RWLock::RWLockImp : private Lock
{

public: 

   /*----- methods -----*/

   RWLockImp() : Lock(),
      _readCount( -1 ),
      _writeCount( 0 )
   {
      _reading  = CreateEvent( 0, true, false, 0 );
      _finished = CreateEvent( 0, false, true, 0 );
   }
   
   ~RWLockImp()
   {
      CloseHandle( _reading );
      CloseHandle( _finished );
   }

   void readLock()
   {
      if( atomicInc( _readCount ) == 0 )
      {
         WaitForSingleObject( _finished, INFINITE );
         SetEvent( _reading );
      }
      WaitForSingleObject( _reading, INFINITE );
   }
   
   void writeLock()
   {
      Lock::lock();
      WaitForSingleObject( _finished, INFINITE );
      ++_writeCount;
   }
   
   void unlock()
   {
      if( _writeCount != 0 )
      {
         --_writeCount;
         SetEvent( _finished );
         Lock::unlock();
      }
      else if( atomicDec( _readCount ) < 0 )
      {
         ResetEvent( _reading );
         SetEvent( _finished );
      }
   }

private: 

   /*----- data members -----*/

   HANDLE   _reading;
   HANDLE   _finished;
   int32_t  _readCount;
   int32_t  _writeCount;
};

#else

/*==============================================================================
   CLASS RWLockImp for POSIX
==============================================================================*/

#include <pthread.h>

class RWLock::RWLockImp
{

public: 

   /*----- methods -----*/

   RWLockImp()
   {
      pthread_rwlock_init( &_lock, 0 );
   }
   
   ~RWLockImp()
   {
      pthread_rwlock_destroy( &_lock );
   }

   void readLock()
   {
      pthread_rwlock_rdlock( &_lock );
   }
   
   void writeLock()
   {
      pthread_rwlock_wrlock( &_lock );
   }
   
   void unlock()
   {
      pthread_rwlock_unlock( &_lock );
   }

private: 

   /*----- data members -----*/

   pthread_rwlock_t _lock;
};

#endif


/*==============================================================================
   CLASS RWLock
==============================================================================*/

//------------------------------------------------------------------------------
//!
RWLock::RWLock() :
   _imp( new RWLockImp )
{
}

//------------------------------------------------------------------------------
//!
RWLock::~RWLock()
{
   delete _imp;
}

//------------------------------------------------------------------------------
//!
void
RWLock::readLock()
{
   _imp->readLock();
}

//------------------------------------------------------------------------------
//!
void
RWLock::writeLock()
{
   _imp->writeLock();
}

//------------------------------------------------------------------------------
//!
void
RWLock::unlock()
{
   _imp->unlock();
}

NAMESPACE_END
