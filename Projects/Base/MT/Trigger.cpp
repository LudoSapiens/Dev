/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/MT/Trigger.h>

#include <Base/Dbg/Defs.h>
#include <Base/IO/TextStream.h>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN



UNNAMESPACE_END


#ifdef _WIN32

//=============================================================================
// WIN32 IMPLEMENTATION
//=============================================================================

//! Implementation based on PTypes library.

#include <Base/Util/windows.h>

struct Trigger::TriggerImp
{
   HANDLE  _event;
};

//------------------------------------------------------------------------------
//!
Trigger::Trigger( bool startBlocking, bool broadcast )
{
   _imp = new TriggerImp();
   CHECK( _imp );
   _imp->_event = CreateEvent( 0, broadcast, startBlocking, 0 );
   CHECK( _imp->_event );
}

//------------------------------------------------------------------------------
//!
Trigger::~Trigger()
{
   CloseHandle( _imp->_event );
   delete _imp;
}

//------------------------------------------------------------------------------
//!
void
Trigger::wait()
{
   WaitForSingleObject( _imp->_event, INFINITE );
}

//------------------------------------------------------------------------------
//!
void
Trigger::post()
{
   SetEvent( _imp->_event );
}

//------------------------------------------------------------------------------
//!
void
Trigger::reset()
{
   ResetEvent( _imp->_event );
}

#else

//=============================================================================
// POSIX IMPLEMENTATION
//=============================================================================

//! Implementation based on PTypes library.

#include <pthread.h>

struct Trigger::TriggerImp
{
   pthread_mutex_t  _mutex;
   pthread_cond_t   _cond;
   bool             _blocking;
   bool             _broadcast;
};

//------------------------------------------------------------------------------
//!
Trigger::Trigger( bool startBlocking, bool broadcast )
{
   _imp = new TriggerImp();
   CHECK( _imp );
   int err = pthread_mutex_init( &(_imp->_mutex), NULL );
   if( err )
   {
      StdErr << "ERROR - pthread_mutex_init failed: " << err << nl;
   }
   err = pthread_cond_init( &(_imp->_cond), NULL );
   if( err )
   {
      StdErr << "ERROR - pthread_cond_init failed: " << err << nl;
   }
   _imp->_blocking  = startBlocking;
   _imp->_broadcast = broadcast;
}

//------------------------------------------------------------------------------
//!
Trigger::~Trigger()
{
   pthread_cond_destroy( &(_imp->_cond) );
   pthread_mutex_destroy( &(_imp->_mutex) );
   delete _imp;
}

//------------------------------------------------------------------------------
//!
void
Trigger::wait()
{
   pthread_mutex_lock( &(_imp->_mutex) );
   while( _imp->_blocking )  pthread_cond_wait( &(_imp->_cond), &(_imp->_mutex) );
   if( !_imp->_broadcast )  _imp->_blocking = true;
   pthread_mutex_unlock( &(_imp->_mutex) );
}

//------------------------------------------------------------------------------
//!
void
Trigger::post()
{
   pthread_mutex_lock( &(_imp->_mutex) );
   _imp->_blocking = false;
   if( _imp->_broadcast )  pthread_cond_broadcast( &(_imp->_cond) );
   else                    pthread_cond_signal( &(_imp->_cond) );
   pthread_mutex_unlock( &(_imp->_mutex) );
}

//------------------------------------------------------------------------------
//!
void
Trigger::reset()
{
   _imp->_blocking = true;
}

#endif
