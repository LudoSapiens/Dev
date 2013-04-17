/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/MT/Semaphore.h>

#ifndef BASE_USE_NAMED_SEMAPHORES
#if __APPLE__
#define BASE_USE_NAMED_SEMAPHORES
#endif
#endif

#ifdef BASE_USE_NAMED_SEMAPHORES
#include <cstdio>
#include <cstring>
#endif


NAMESPACE_BEGIN

#ifdef _WIN32

/*==============================================================================
   CLASS SemaphoreImp for WIN32
==============================================================================*/

#include <Base/Util/windows.h>

class  Semaphore::SemaphoreImp
{

public: 

   /*----- methods -----*/

   SemaphoreImp( int value )
   {
      _handle = CreateSemaphore( 0, value, 0xffff, 0 );
   }
   
   ~SemaphoreImp()
   {
      CloseHandle( _handle );
   }

   void wait()
   {
      WaitForSingleObject( _handle, INFINITE );
   }
   
   void post()
   {
      ReleaseSemaphore( _handle, 1, 0 );
   }

private: 

   /*----- data members -----*/

   HANDLE _handle;
};

#else

/*==============================================================================
   CLASS SemaphoreImp for POSIX
==============================================================================*/

#include <pthread.h>
#include <semaphore.h>

#ifdef BASE_USE_NAMED_SEMAPHORES

class  Semaphore::SemaphoreImp
{

public: 

   /*----- methods -----*/

   SemaphoreImp( int value )
   {
      //Generate a unique filename
      strncpy( _name, tmpnam(NULL), 256 );
      _name[255] = '\0';
      _handle = sem_open( _name, O_CREAT|O_EXCL, S_IRUSR, value );
   }
   
   ~SemaphoreImp()
   {
      sem_close( _handle );
      sem_unlink( _name );
   }

   void wait()
   {
      sem_wait( _handle );
   }
   
   void post()
   {
      sem_post( _handle );
   }

private: 

   /*----- data members -----*/

   sem_t* _handle;
   char   _name[256];
};

#else

class  Semaphore::SemaphoreImp
{

public: 

   /*----- methods -----*/

   SemaphoreImp( int value )
   {
      sem_init( &_handle, 0, value );
   }
   
   ~SemaphoreImp()
   {
      sem_destroy( &_handle );
   }

   void wait()
   {
      sem_wait( &_handle );
   }
   
   void post()
   {
      sem_post( &_handle );
   }

private: 

   /*----- data members -----*/

   sem_t _handle;
};

#endif //BASE_USE_NAMED_SEMAPHORES..else

#endif //_WIN32..else


/*==============================================================================
   CLASS Semaphore
==============================================================================*/

//------------------------------------------------------------------------------
//!
Semaphore::Semaphore( int value ) :
   _imp( new SemaphoreImp( value ) )
{
}

//------------------------------------------------------------------------------
//!
Semaphore::~Semaphore()
{
   delete _imp;
}

//------------------------------------------------------------------------------
//!
void
Semaphore::wait()
{
   _imp->wait();
}

//------------------------------------------------------------------------------
//!
void
Semaphore::post()
{
   _imp->post();
}

NAMESPACE_END
