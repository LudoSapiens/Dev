/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/Dbg/UnitTest.h>

#include <Base/MT/Atomic.h>
#include <Base/MT/Semaphore.h>
#include <Base/MT/Task.h>
#include <Base/MT/TaskQueue.h>
#include <Base/MT/Thread.h>
#include <Base/MT/ValueTrigger.h>

#include <Base/Util/Platform.h>
#include <Base/Util/Timer.h>

USING_NAMESPACE

Test::Result*  gRes    = NULL;
Semaphore*     gSem1   = NULL;
Semaphore*     gSem2   = NULL;
ValueTrigger*  gVTrig1 = NULL;
ValueTrigger*  gVTrig2 = NULL;
int32_t        gValue;
AtomicInt32    gAI32;
AtomicInt32*   gAI32p  = NULL;

// Inserts a 4b value 'v' into the specified 'dst' variable.
inline void  atomicInsert4( int32_t& dst, const int32_t v )
{
   int32_t oldV;
   int32_t newV;
   do
   {
      oldV = dst;
      newV = (oldV << 4) | v;
   } while( !atomicCAS(dst, oldV, newV) );
}

class Atomic32Task:
   public Task
{
public:
   Atomic32Task( int32_t id, int32_t n ): _id( id ), _n( n ) { }
protected:
   int32_t  _id;
   int32_t  _n;
   virtual void execute()
   {
      for( int32_t i = 0; i < _n; ++i )
      {
         ++gValue;
         --gValue;
         gValue += _id;
         gValue -= _id;
         ++gValue;

         ++gAI32;
         --gAI32;
         gAI32 += _id;
         gAI32 -= _id;
         ++gAI32;
      }
   }
};


class WaitValueTask:
   public Task
{
public:
   WaitValueTask( int32_t n, int32_t waitedValue ): _n( n ), _v( waitedValue ) { }
protected:
   int32_t  _n;
   int32_t  _v;
   bool  canContinue() const
   {
      if( gValue == _v )
      {
         gAI32p->CAS( 0, _n ); // Must be assigned before waitFor proceeds.
         return true;
      }
      return false;
   }
   virtual void execute()
   {
      waitFor( makeDelegate(this, &WaitValueTask::canContinue) );
      ++gAI32;
      gValue = 1;
   }
};

class AutoFree:
   public Task
{
public:
   AutoFree()
   {
      gValue = 0x01;
      gSem2->post();
   }
   virtual void execute()
   {
      gSem1->wait();
      gValue <<= 4;
      gValue |= 0x02;
      gSem2->post();
   }
   ~AutoFree()
   {
      gValue <<= 4;
      gValue |= 0x03;
      gSem2->post();
   }
};

uint SerialFib( uint n )
{
   if( n < 2 )
   {
      return n;
   }
   else
   {
      return SerialFib( n-1 ) + SerialFib( n-2 );
   }
}

class Fib:
   public Task
{
public:
   static const uint CutOff = 2;
   uint   _n;
   uint&  _res;
   Fib( uint n, uint& res ): _n(n), _res(res) {}
   virtual void execute()
   {
      if( _n < CutOff )
      {
         _res = SerialFib( _n );
      }
      else
      {
         uint resA, resB;
         spawn( new Fib( _n-1, resA ) );
         spawn( new Fib( _n-2, resB ) );
         waitForAll();
         _res = resA + resB;
      }
   }
};

#define SPAWN_FAST 1

class SpawnL3:
   public Task
{
public:
   uint  _n;
   SpawnL3( uint n ): _n(n) {}
   virtual void execute()
   {
      int32_t v = (1 << 16);
#if SPAWN_FAST
      gAI32p[_n] += v;
#else
      for( int32_t i = 0; i < v; ++i )
      {
         ++gAI32p[_n];
      }
#endif
   }
};

class SpawnL2:
   public Task
{
public:
   uint  _n;
   SpawnL2( uint n ): _n(n) {}
   virtual void execute()
   {
      uint n_2 = (_n >> 1);
      for( uint i = 0; i < n_2; ++i )
      {
         spawn( new SpawnL3(_n) );
      }

      int32_t v = (1 << 8);
#if SPAWN_FAST
      gAI32p[_n] += v;
#else
      for( int32_t i = 0; i < v; ++i )
      {
         ++gAI32p[_n];
      }
#endif

      for( uint i = n_2; i < _n; ++i )
      {
         spawn( new SpawnL3(_n) );
      }
   }
};

class SpawnL1:
   public Task
{
public:
   uint  _n;
   SpawnL1( uint n ): _n(n) {}
   virtual void execute()
   {
      uint n_2 = (_n >> 1);
      for( uint i = 0; i < n_2; ++i )
      {
         spawn( new SpawnL2(_n) );
      }

      int32_t v = (1 << 4);
#if SPAWN_FAST
      gAI32p[_n] += v;
#else
      for( int32_t i = 0; i < v; ++i )
      {
         ++gAI32p[_n];
      }
#endif

      for( uint i = n_2; i < _n; ++i )
      {
         spawn( new SpawnL2(_n) );
      }
   }
};

class SpawnL0:
   public Task
{
public:
   uint  _n;
   SpawnL0( uint n ): _n(n) {}
   virtual void execute()
   {
      ++gAI32p[_n];
      for( uint i = 0; i < _n; ++i )
      {
         spawn( new SpawnL1(_n) );
      }
   }
};

class Task1:
   public Task
{
   virtual void execute()
   {
      gValue <<= 4;
      gValue |= 0x01;
      gSem1->post();
      gSem2->wait();
      gValue <<= 4;
      gValue |= 0x01;
      gSem1->post();
      gSem2->wait();
      gValue <<= 4;
      gValue |= 0x01;
      gSem1->post();
   }
};

class Task1Checker:
   public Task
{
   virtual void execute()
   {
      gSem1->wait();
      TEST_ADD( *gRes, gValue == 0x0001 );
      gSem2->post();
      gSem1->wait();
      TEST_ADD( *gRes, gValue == 0x0011 );
      gSem2->post();
      gSem1->wait();
      TEST_ADD( *gRes, gValue == 0x0111 );
   }
};

class VTrigA:
   public BaseTask
{
public:
   virtual void execute()
   {
      ++(*gVTrig2);
      gVTrig2->wait();
      TEST_ADD( *gRes, gValue == 0x0012 );
      atomicInsert4( gValue, 0x3 );
      TEST_ADD( *gRes, gValue == 0x0123 );
      --(*gVTrig1);
   }
};

class VTrigB:
   public BaseTask
{
public:
   virtual void execute()
   {
      // A is blocked.
      TEST_ADD( *gRes, gValue == 0x0000 );
      atomicInsert4( gValue, 0x1 );
      TEST_ADD( *gRes, gValue == 0x0001 );
      atomicInsert4( gValue, 0x2 );
      TEST_ADD( *gRes, gValue == 0x0012 );
      // Release A.
      --(*gVTrig2);
      --(*gVTrig1);
   }
};

class SimpleTask:
   public Task
{
public:

   SimpleTask(): _val(0) {}

   virtual void execute()
   {
      for( uint i = 0; i < 1024; ++i ) ++_val;
   }
   uint _val;
};

class TestWorkerTask:
   public Task
{
public:
   TestWorkerTask( uint id ): _id(id) {}
   virtual void execute()
   {
      //printf("Worker #%d\n", _id);
      int32_t a = 0;
      int32_t b = 1;
      int32_t n = (1 << 20);
      for( int32_t i = 0; i < n; ++i )
      {
         b = b + a;
         a = b - a;
      }
      //printf("Worker #%d ==> %d\n", _id, b);
      gValue += b;
   }
   uint  _id;
};

void mt_atomic32( Test::Result& res )
{
   gValue = 0;
   gAI32  = 0;
   int32_t n = 100;
   int32_t m = 10000;
   Vector<Thread*>  threads;
   threads.reserve( n );
   for( int32_t i = 0; i < n; ++i )
   {
      // Launch n threads in parallel tyring to break gAI32.
      Thread* thread = new Thread( new Atomic32Task(i, m) );
      threads.pushBack( thread );
   }
   while( !threads.empty() )
   {
      Thread* thread = threads.back();
      CHECK( thread );
      threads.popBack();
      thread->wait();
      delete thread;
   }
   int32_t expected = n * m;
   TEST_ADD( res, gValue != expected ); // Guarantee a simple int32_t isn't thread safe.
   TEST_ADD( res, gAI32  == expected ); // Guarantee our class is.
   //fprintf( stderr, "\nDONE: exp=%d gValue=%d gAI32=%d\n", expected, gValue, (int32_t)gAI32 );

   TEST_ADD( res, gAI32.set(0) == expected );
   TEST_ADD( res, gAI32 == 0 );
   TEST_ADD( res, (gAI32 = 1) == 1 );
   TEST_ADD( res, gAI32 == 1 );
}

void mt_atomic32_timed( Test::Result& res )
{
   Timer timer;
   mt_atomic32( res );
   StdErr << nl;
   StdErr << "Time: " << timer.elapsed() << " seconds" << nl;
}

void mt_auto_free( Test::Result& res )
{
   Semaphore sem1( 0 );
   Semaphore sem2( 0 );
   gValue = 0;
   gRes   = &res;
   gSem1  = &sem1;
   gSem2  = &sem2;
   TEST_ADD( res, gValue == 0x0000 );
   Thread* thread = new Thread( new AutoFree(), true );
   gSem2->wait();  // wait for ctor to finish
   TEST_ADD( res, gValue == 0x0001 );
   gSem1->post();
   gSem2->wait();  // wait for execute to finish
   gSem2->wait();  // wait for dtor to finish
   TEST_ADD( res, gValue == 0x0123 );
   unused( thread ); // dangling pointer
}

void mt_fib( Test::Result& res )
{
#if PLAT_ANDROID
   // The code below is rather slow.
   printf(" *** Skipping under Android *** ");
   return;
#endif

   TaskQueue  queue(2);
   uint n = 25;
   uint serial = SerialFib( n );
   uint parallel;
   Task* task = new Fib( n, parallel );
   queue.post( task );
   queue.waitForAll();
   TEST_ADD( res, serial == parallel );
   //StdErr << "res=" << serial << "," << parallel << nl;
}

void mt_simple( Test::Result& res )
{
   Semaphore  sem1( 0 );
   Semaphore  sem2( 0 );
   gValue = 0;
   gRes   = &res;
   gSem1  = &sem1;
   gSem2  = &sem2;
   Thread task1( new Task1() );
   Thread task1checker( new Task1Checker() );
   task1.wait();
   task1checker.wait();
   TEST_ADD( res, gValue == 0x0111 );
}

void mt_spawn( Test::Result& res )
{
   TaskQueue queue(2);
   const uint n = 7;
   AtomicInt32  values[n];
   for( uint i = 0; i < n; ++i )
   {
      values[i] = 0;
   }
   gAI32p = values;
   uint prime = 17;
   for( uint i = 0; i < n; ++i )
   {
      uint v = (i + prime) % n; // Launch them in out-of-order.
      Task* task = new SpawnL0( v );
      queue.post( task );
   }
   queue.waitForAll();
   for( uint i = 0; i < n; ++i )
   {
      int l0 = 1;
      int l1 = i;
      int l2 = l1*i;
      int l3 = l2*i;
      int total = (l3 << 16) + (l2 << 8) + (l1 << 4) + l0;
      TEST_ADD( res, gAI32p[i] == total );
   }
}

void mt_valuetrigger( Test::Result& res )
{
   ValueTrigger app( 2 );
   ValueTrigger tmp( 0 );
   gValue  = 0;
   gRes    = &res;
   gVTrig1 = &app;
   gVTrig2 = &tmp;
   TEST_ADD( res, app == 2 );
   TEST_ADD( res, tmp == 0 );
   new Thread( new VTrigA(), true );
   new Thread( new VTrigB(), true );
   app.wait();
   TEST_ADD( res, gValue == 0x0123 );
   TEST_ADD( res, app == 0 );
   TEST_ADD( res, tmp == 0 );
}

void mt_valuetrigger2( Test::Result& res )
{
   TaskQueue queue(2);

   for( uint i = 0; i < 1024 ; ++i )
   {
      for( uint t = 0; t < 32; ++t )
      {
         queue.post( new SimpleTask() );
      }
      queue.waitForAll();
      int nt = queue.numTasks();
      TEST_ADD( res, nt == 0 );
      if( nt != 0 )
      {
         StdErr << "error: " << queue.numTasks() << " " << nt << nl;
      }
   }
}

void mt_waitfor( Test::Result& res )
{
   // If this test fails, it's most likely through deadlock.
   // It basically creates a lot of tasks, all of them waiting for 1
   // except for one of them, which should unlock all of the other ones.
   // The reason a lost of tasks are created is to make sure the waitFor()
   // calls do not block tasks.
   TaskQueue queue(2);

   AtomicInt32 triggeringID = 0;
   gValue = 0;
   gAI32  = 0;
   gAI32p = &triggeringID;

   uint n = 1024;
   uint t = 7777;
   // Start n blocking tasks (to flood the queue).
   for( uint i = 1; i <= n; ++i )
   {
      queue.post( new WaitValueTask( i, 1 ) );
   }
   Thread::sleep(0.2); // Make sure the queues are all waiting on their semaphores.
   queue.post( new WaitValueTask( t, 0 ) );
   queue.waitForAll();
   TEST_ADD( res, gAI32 == n+1 );
   TEST_ADD( res, triggeringID == t );
}

void mt_worker_threads( Test::Result& res )
{
   TaskQueue  queue(2);
   //uint nThreads = queue.numThreads();
   uint n = 1024;
   //n = 4;
   // Post n tasks.
   for( uint i = 0; i < n; ++i )
   {
      Task* task = new TestWorkerTask(i);
      queue.post( task );
   }
   TEST_ADD( res, queue.numTasks() != 0 );
   // Wait for the tasks to finish.
   queue.waitForAll();
   TEST_ADD( res, queue.numTasks() == 0 );
   //TEST_ADD( res, nThreads == queue.numThreads() ); // Should be identical.
}

void mt_info( Test::Result& /*res*/ )
{
   StdErr << nl;
   StdErr << "Number of logical hardware threads: " << Thread::numHardwareThreads() << nl;
}

void init_mt()
{
   RCP<Test::Collection> col;
   col = new Test::Collection( "mt", "Collection for Base/MT" );
   col->add( new Test::Function("mt_atomic32"     , "Tests the AtomitInt32 class"              , mt_atomic32      ) );
   col->add( new Test::Function("mt_auto_free"    , "Tests auto-freeing of task once completed", mt_auto_free     ) );
   col->add( new Test::Function("mt_simple"       , "Tests simple multi-threading situation"   , mt_simple        ) );
   col->add( new Test::Function("mt_spawn"        , "Tests spawning tasks"                     , mt_spawn         ) );
   col->add( new Test::Function("mt_valuetrigger" , "Tests the ValueTrigger class"             , mt_valuetrigger  ) );
   col->add( new Test::Function("mt_valuetrigger2", "Tests the ValueTrigger class"             , mt_valuetrigger2 ) );
   col->add( new Test::Function("mt_waitfor"      ,  "Tests the waitFor() method"              , mt_waitfor       ) );
   Test::standard().add( col.ptr() );
   col = new Test::Collection( "mt_special", "Collection for Base/MT" );
   col->add( new Test::Function("mt_fib"           , "Tests simple fibonacci example"                              , mt_fib            ) );
   col->add( new Test::Function("mt_worker_threads", "Runs multiple worker threads"                                , mt_worker_threads ) );
   col->add( new Test::Function("mt_atomic32_timed", "Runs mt_atomic32 and gives the time it took"                 , mt_atomic32_timed ) );
   col->add( new Test::Function("mt_info"          , "Reports some info about the number of hardware threads, etc.", mt_info           ) );
   Test::special().add( col.ptr() );
}
