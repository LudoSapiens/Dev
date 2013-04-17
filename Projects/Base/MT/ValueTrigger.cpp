/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/MT/ValueTrigger.h>

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

#include <Base/Util/windows.h>

#include <climits>

// Implementation based on
// http://compgroups.net/comp.programming.threads/yet-another-win32-condvar-implementation

struct ValueTrigger::Impl
{
   void cond_broadcast()
   {
      if( _numWaiters == 0 ) return;
      ReleaseSemaphore( _sema, _numWaiters, NULL );
      WaitForSingleObject( _continueEvent, INFINITE );
   }

   void cond_wait()
   {
      ++_numWaiters;
      LeaveCriticalSection( &_mutex );
      WaitForSingleObject( _sema, INFINITE );
      if( InterlockedDecrement( &_numWaiters ) == 0 )
      {
         SetEvent( _continueEvent );
      }
      EnterCriticalSection( &_mutex );
   }

   //HANDLE  _event;
   LONG             _numWaiters;
   HANDLE           _sema;
   HANDLE           _continueEvent;
   CRITICAL_SECTION _mutex;
};

//------------------------------------------------------------------------------
//!
ValueTrigger::ValueTrigger( int32_t initialValue, int32_t wantedValue ):
   _current( initialValue ),
   _wanted( wantedValue )
{
   _impl = new Impl();
   CHECK( _impl );

   //_impl->_event = CreateEvent( 0, TRUE, initialValue == wantedValue, 0 );
   //CHECK( _impl->_event );

   InitializeCriticalSection( &(_impl->_mutex) );
   _impl->_sema          = CreateSemaphore( NULL, 0, LONG_MAX, NULL );
   _impl->_continueEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
   _impl->_numWaiters    = 0;
}

//------------------------------------------------------------------------------
//!
ValueTrigger::~ValueTrigger()
{
   //CloseHandle( _impl->_event );
   CloseHandle( _impl->_continueEvent );
   CloseHandle( _impl->_sema );
   DeleteCriticalSection( &(_impl->_mutex) );
   delete _impl;
}

//------------------------------------------------------------------------------
//!
void
ValueTrigger::wait()
{
   //while( _current != _wanted )  WaitForSingleObject( _impl->_event, INFINITE );

   EnterCriticalSection( &_impl->_mutex );
   while( _current != _wanted ) _impl->cond_wait();
   LeaveCriticalSection( &_impl->_mutex );
}

//------------------------------------------------------------------------------
//!
void
ValueTrigger::post()
{
   //SetEvent( _impl->_event );
   EnterCriticalSection( &(_impl->_mutex) );
   _impl->cond_broadcast();
   LeaveCriticalSection( &(_impl->_mutex) );
}

#else

//=============================================================================
// POSIX IMPLEMENTATION
//=============================================================================

#include <pthread.h>

struct ValueTrigger::Impl
{
   pthread_mutex_t  _mutex;
   pthread_cond_t   _cond;
};

//------------------------------------------------------------------------------
//!
ValueTrigger::ValueTrigger( int32_t initialValue, int32_t wantedValue ):
   _current( initialValue ),
   _wanted( wantedValue )
{
   _impl = new Impl();
   CHECK( _impl );
   int err = pthread_mutex_init( &(_impl->_mutex), NULL );
   if( err )
   {
      StdErr << "ERROR - pthread_mutex_init failed: " << err << nl;
   }
   err = pthread_cond_init( &(_impl->_cond), NULL );
   if( err )
   {
      StdErr << "ERROR - pthread_cond_init failed: " << err << nl;
   }
}

//------------------------------------------------------------------------------
//!
ValueTrigger::~ValueTrigger()
{
   pthread_cond_destroy( &(_impl->_cond) );
   pthread_mutex_destroy( &(_impl->_mutex) );
   delete _impl;
}

//------------------------------------------------------------------------------
//!
void
ValueTrigger::wait()
{
   pthread_mutex_lock( &(_impl->_mutex) );
   while( _current != _wanted )  pthread_cond_wait( &(_impl->_cond), &(_impl->_mutex) );
   pthread_mutex_unlock( &(_impl->_mutex) );
}

//------------------------------------------------------------------------------
//!
void
ValueTrigger::post()
{
   pthread_mutex_lock( &(_impl->_mutex) );   // Serve as a barrier.
   pthread_mutex_unlock( &(_impl->_mutex) );
   pthread_cond_broadcast( &(_impl->_cond) );
}

#endif


//=============================================================================
// COMMON IMPLEMENTATION
//=============================================================================

//------------------------------------------------------------------------------
//!
void
ValueTrigger::currentValue( int32_t v )
{
   int32_t old;
   do
   {
      old = _current;
   } while( !atomicCAS(_current, old, v) );

   checkTrigger( old, v );
}

#define DEF_OPER( oper, newV_expr ) \
   int32_t \
   ValueTrigger::oper() \
   { \
      int32_t oldV, newV; \
      do \
      { \
         oldV = _current; \
         newV = newV_expr; \
      } while( !atomicCAS(_current, oldV, newV) ); \
      checkTrigger( oldV, newV ); \
      return newV; \
   }

#define DEF_OPER_ARG( oper, newV_expr ) \
   int32_t \
   ValueTrigger::oper( int32_t v ) \
   { \
      int32_t oldV, newV; \
      do \
      { \
         oldV = _current; \
         newV = newV_expr; \
      } while( !atomicCAS(_current, oldV, newV) ); \
      checkTrigger( oldV, newV ); \
      return newV; \
   }

DEF_OPER( operator++    , oldV+1 )
DEF_OPER( operator--    , oldV-1 )
DEF_OPER_ARG( operator+=, oldV+v )
DEF_OPER_ARG( operator-=, oldV-v )
DEF_OPER_ARG( operator&=, oldV&v )
DEF_OPER_ARG( operator|=, oldV|v )
DEF_OPER_ARG( operator^=, oldV^v )

#undef DEF_OPER_ARG
#undef DEF_OPER

//------------------------------------------------------------------------------
//!
void
ValueTrigger::checkTrigger( int32_t oldV, int32_t newV )
{
   if( newV == _wanted )
   {
      if( oldV != _wanted )  post();
   }
}
