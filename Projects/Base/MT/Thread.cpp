/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/MT/Thread.h>

#include <Base/Util/Platform.h>

#if !defined(BASE_USE_SYSCONF)
#if PLAT_ANDROID
#define BASE_USE_SYSCONF 1
#else
#define BASE_USE_SYSCONF 0
#endif
#endif

#if BASE_USE_SYSCONF
#include <sys/sysconf.h>
#endif

#if defined(_WIN32) && !defined(__CYGWIN__)

/*==============================================================================
   CLASS ThreadImp for WIN32
==============================================================================*/

#include <Base/Util/windows.h>
#include <process.h>

NAMESPACE_BEGIN

class Thread::ThreadImp
{

public:

   /*----- static methods -----*/

   static int numHardwareThreads()
   {
      SYSTEM_INFO info={0};
      GetSystemInfo(&info);
      return info.dwNumberOfProcessors;
   }

   static void sleep( double time )
   {
      Sleep( (int)(time * 1000.0) );
   }

   static void yield()
   {
      Sleep(0);
   }

   /*----- methods -----*/

   ThreadImp( Thread* parent )
   {
      _thread = (HANDLE)_beginthreadex( 0, 0, &procedure, parent, 0, &_id );
   }

   ~ThreadImp()
   {
      CloseHandle( _thread );
   }

   void wait()
   {
      WaitForSingleObject( _thread, INFINITE );
      //CloseHandle( _thread );
   }

   bool operator==( const ThreadImp& t ) const
   {
      return _id == t._id;
   }

private:

   /*----- static methods -----*/

   static unsigned __stdcall procedure( void* arg )
   {
      Thread* thread = (Thread*)arg;
      bool autofree = thread->_autofree; // Need to store it before, since thread could be deleted.
      thread->_task->execute();
      if( autofree )
      {
         delete thread;
      }
      return 0;
   }

   /*----- data members -----*/

   HANDLE _thread;
   uint   _id;
};

#else

/*==============================================================================
   CLASS ThreadImp for POSIX
==============================================================================*/

#include <pthread.h>
#include <time.h>

#if defined(__APPLE__) || defined(__CYGWIN__)
//MacOSX/Cygwin don't have pthread_yield, so we use sched_yield() instead
#include <sched.h>
#define pthread_yield  sched_yield
#endif //__APPLE__

#if defined(__APPLE__) || defined(__FreeBSD__)
#include <sys/sysctl.h>
#endif

NAMESPACE_BEGIN

class Thread::ThreadImp
{

public:

   /*----- static methods -----*/

   static int numHardwareThreads()
   {
#if BASE_USE_SYSCONF
      return (int)sysconf(_SC_NPROCESSORS_CONF);
#elif defined(PTW32_VERSION) || defined(__hpux)
      return pthread_num_processors_np();
#elif defined(_GNU_SOURCE)
      return get_nprocs();
#elif defined(__APPLE__) || defined(__FreeBSD__)
      int count;
      size_t size=sizeof(count);
      return sysctlbyname("hw.ncpu",&count,&size,NULL,0)?0:count;
#elif defined(BOOST_HAS_UNISTD_H) && defined(_SC_NPROCESSORS_ONLN)
      int const count=sysconf(_SC_NPROCESSORS_ONLN);
      return (count>0)?count:0;
#else
      return 0;
#endif
   }

   static void sleep( double time )
   {
      timespec ts;
      ts.tv_sec  = (int)time;
      ts.tv_nsec = (long)( ( time - ts.tv_sec ) * 1000000000 );
      nanosleep( &ts, 0 );
   }

   static void yield()
   {
#if PLAT_ANDROID
#     warning Android is missing pthread_yield()
      fprintf( stderr, "Android would call phtread_yield() is it had it...\n" );
#else
      pthread_yield();
#endif
   }

   /*----- methods -----*/

   ThreadImp( Thread* parent )
   {
      pthread_create( &_thread, 0, procedure, parent );
   }

   ~ThreadImp()
   {
      pthread_detach( _thread );
   }

   void wait()
   {
      pthread_join( _thread, 0 );
   }

   bool operator==( const ThreadImp& t ) const
   {
      return pthread_equal( _thread, t._thread ) != 0;
   }

private:

   /*----- static methods -----*/

   static void* procedure( void* arg )
   {
      Thread* thread = (Thread*)arg;
      bool autofree = thread->_autofree; // pthread_detach seems to stop execution of this procedure() call, but better be safe than sorry.
      thread->_task->execute();
      if( autofree )
      {
         delete thread;
      }
      return 0;
   }

   /*----- data members -----*/

   pthread_t _thread;
};

#endif


/*==============================================================================
   CLASS Thread
==============================================================================*/


//------------------------------------------------------------------------------
//! 
int 
Thread::numHardwareThreads()
{
   return ThreadImp::numHardwareThreads();
}

//------------------------------------------------------------------------------
//!
void
Thread::sleep( double seconds )
{
   ThreadImp::sleep( seconds );
}

//------------------------------------------------------------------------------
//!
void
Thread::yield()
{
   ThreadImp::yield();
}

//------------------------------------------------------------------------------
//!
Thread::Thread( BaseTask* task, bool autofree ):
   _task( task ),
   _autofree( autofree )
{
   _imp = new ThreadImp( this );
}

//------------------------------------------------------------------------------
//!
Thread::~Thread()
{
   delete _task;  // This guarantees that auto-freed threads delete their task in the thread domain.
   delete _imp;
}

//------------------------------------------------------------------------------
//!
void
Thread::wait()
{
   _imp->wait();
}

//------------------------------------------------------------------------------
//!
bool
Thread::operator==( const Thread& t ) const
{
   return ( _imp == t._imp );
}


NAMESPACE_END
