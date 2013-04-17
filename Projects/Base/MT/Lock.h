/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_LOCK_H
#define BASE_LOCK_H

#include <Base/StdDefs.h>

NAMESPACE_BEGIN


/*==============================================================================
   CLASS Lock
==============================================================================*/

//! Lock (mutex) primitive for multi-threading.

class BASE_DLL_API Lock
{

public: 

   /*----- methods -----*/

   Lock();
   ~Lock();

protected: 

   /*----- friend classes -----*/

   friend class LockGuard;
      
   /*----- classes -----*/

   class LockImp;
   
   /*----- methods -----*/
   
   void lock();
   void unlock();

   //! Disallow copying for this class and its children.
   Lock( const Lock& );
   Lock& operator=( const Lock& );

   /*----- data members -----*/
   
   LockImp* _imp;
};

/*==============================================================================
   CLASS LockGuard
==============================================================================*/

//! Guard for Lock.

class LockGuard 
{

public: 

   /*----- methods -----*/

   LockGuard( Lock& lock ) :
      _lock( &lock )
   {
      _lock->lock();
   }
   
   ~LockGuard()
   {
      _lock->unlock();
   }

private: 

   /*----- methods -----*/

   //! Disallow copying for this class and its children.
   LockGuard( const LockGuard& );
   LockGuard& operator=( const LockGuard& );

   /*----- data members -----*/

   Lock* _lock;
};

NAMESPACE_END

#endif
