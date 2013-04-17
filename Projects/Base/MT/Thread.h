/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_THREAD_H
#define BASE_THREAD_H

#include <Base/StdDefs.h>
#include <Base/MT/Task.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS Thread
==============================================================================*/

//! Thread primitive for multi-threading.
//! The ownership of the task is given to the Thread and has the responsability
//! to delete it.
//!
class Thread
{

public:

   /*----- static methods -----*/

   static BASE_DLL_API int numHardwareThreads();
   static BASE_DLL_API void sleep( double );
   static BASE_DLL_API void yield();

   /*----- methods -----*/

   BASE_DLL_API Thread( BaseTask*, bool autofree = false );
   BASE_DLL_API ~Thread();

   BASE_DLL_API void wait();

   BASE_DLL_API bool operator==( const Thread& ) const;
   inline bool operator!=( const Thread& ) const;

private:

   /*----- classes -----*/

   class ThreadImp;

   /*----- methods -----*/

   //! Disallow copying for this class and its children.
   Thread( const Thread& );
   Thread& operator=( const Thread& );

   /*----- data members -----*/
   ThreadImp*  _imp;
   BaseTask*   _task;
   bool        _autofree;
};

//------------------------------------------------------------------------------
//!
inline bool
Thread::operator!=( const Thread& thread ) const
{
   return !(*this == thread );
}

NAMESPACE_END

#endif
