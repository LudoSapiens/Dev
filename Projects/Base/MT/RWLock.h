/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_RWLOCK_H
#define BASE_RWLOCK_H

#include <Base/StdDefs.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS RWLock
==============================================================================*/

//! Read/Write lock primitive for multi-threading. Allow one writer or many
//! reader to a shared region.

class BASE_DLL_API RWLock
{

public: 

   /*----- methods -----*/

   RWLock();
   ~RWLock();

private: 

   /*----- friend classes -----*/

   friend class ReadGuard;
   friend class WriteGuard;
      
   /*----- classes -----*/
   
   class RWLockImp;

   /*----- methods -----*/
   
   void readLock();
   void writeLock();
   void unlock();

   //! Disallow copying for this class and its children.
   RWLock( const RWLock& );
   RWLock& operator=( const RWLock& );

   /*----- data members -----*/

   RWLockImp* _imp;
};

/*==============================================================================
   CLASS ReadGuard
==============================================================================*/

//! Read guard for RWLock.

class ReadGuard 
{

public: 

   /*----- methods -----*/

   ReadGuard( RWLock& lock ) :
      _lock( &lock )
   {
      _lock->readLock();
   }
   
   ~ReadGuard()
   {
      _lock->unlock();
   }

private: 

   /*----- methods -----*/

   //! Disallow copying for this class and its children.
   ReadGuard( const ReadGuard& );
   ReadGuard& operator=( const ReadGuard& );

   /*----- data members -----*/

   RWLock* _lock;
};

/*==============================================================================
   CLASS WriteGuard
==============================================================================*/

//! Write guard for RWLock.

class WriteGuard 
{

public: 

   /*----- methods -----*/

   WriteGuard( RWLock& lock ) :
      _lock( &lock )
   {
      _lock->writeLock();
   }
   
   ~WriteGuard()
   {
      _lock->unlock();
   }

private: 

   /*----- methods -----*/

   //! Disallow copying for this class and its children.
   WriteGuard( const WriteGuard& );
   WriteGuard& operator=( const WriteGuard& );

   /*----- data members -----*/

   RWLock* _lock;
};


NAMESPACE_END

#endif
