/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_SEMAPHORE_H
#define BASE_SEMAPHORE_H

#include <Base/StdDefs.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS Semaphore
==============================================================================*/

//! Semaphore primitive for multi-threading.

class BASE_DLL_API Semaphore
{

public: 

   /*----- methods -----*/

   Semaphore( int value );
   ~Semaphore();

   void wait();
   void post();

private: 
   
   /*----- classes -----*/

   class SemaphoreImp;

   /*----- methods -----*/

   //! Disallow copying for this class and its children.
   Semaphore( const Semaphore& );
   Semaphore& operator=( const Semaphore& );

   /*----- data members -----*/
   
   SemaphoreImp* _imp;
};


/*==============================================================================
  CLASS SemLockGuard
==============================================================================*/
//!< Utility routine to lock a section of code using a semaphore.
//!< This class automatically waits on creation, and automatically releases
//!< (a.k.a. posts) to the semaphore when it gets out of scope.
//!< This is the semaphore variant of the LockGuard class.
class SemLockGuard
{
public:
   SemLockGuard( Semaphore& sem ): _sem(sem) { _sem.wait(); }
   ~SemLockGuard() { _sem.post(); }

protected:
   /*----- methods -----*/

   //! Disallow copying for this class and its children.
   SemLockGuard( const SemLockGuard& );
   SemLockGuard& operator=( const SemLockGuard& );

   /*----- data members -----*/
   Semaphore&  _sem;
}; //class SemLock


NAMESPACE_END

#endif
