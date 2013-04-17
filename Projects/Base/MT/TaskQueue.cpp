/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/MT/TaskQueue.h>

#include <Base/Dbg/Defs.h>
#include <Base/MT/Thread.h>
#include <Base/Dbg/DebugStream.h>

NAMESPACE_BEGIN

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_mt, "MT" );
DBG_STREAM( os_mtp, "MTP" );

UNNAMESPACE_END

//------------------------------------------------------------------------------
//!
uint
TaskQueue::defaultNumberOfThreads()
{
   // Automatically determine number of threads based on current architecture.
   uint n = 4; // FIXME: simply use 4 for now.
   const char* env = getenv("BASE_NUM_THREADS");
   if( env )
   {
      n = atoi( env );
      fprintf( stderr, "\nTaskQueue: Overriding number of threads to %d.\n", n );
   }
   return n;
}

//------------------------------------------------------------------------------
//!
TaskQueue::TaskQueue( uint nThreads ):
   _nWaitFor( 0 )
{
   DBG_BLOCK( os_mt, "TaskQueue::TaskQueue(" << nThreads << ")" );
   PROFILE_TASK_OPERATION( _nTasksSubmitted = 0 );
   PROFILE_TASK_OPERATION( _nTasksDeleted   = 0 );
   if( nThreads == 0 )  nThreads = defaultNumberOfThreads();
   createThreads( nThreads );
}

//------------------------------------------------------------------------------
//!
TaskQueue::~TaskQueue()
{
   DBG_BLOCK( os_mt, "TaskQueue::~TaskQueue()" );
   CHECK( _nWaitFor == 0 );
#if BASE_PROFILE_TASKS
   DBG( if( os_mtp.active() ) print(os_mtp.stream()) );
#endif
   deleteThreads();
}

//------------------------------------------------------------------------------
//!
uint
TaskQueue::numRunningThreads()
{
   return _nThreads;
}

//------------------------------------------------------------------------------
//!
uint
TaskQueue::numAllocatedThreads()
{
   return uint(_threads.size());
}

//------------------------------------------------------------------------------
//!
uint
TaskQueue::numTasks()
{
   return _nTasks;
}

//------------------------------------------------------------------------------
//!
void
TaskQueue::setStorage( uint threadID, const String& key, void* data )
{
   LockGuard  lock( _wTasksLock );
   CHECK( threadID < _wTasks.size() );
   //StdErr << "setStorage: TaskQueue=" << (void*)this << " WorkerTask[" << threadID << "]=" << (void*)_wTasks[threadID] << " data[" << key << "]=" << data << nl;
   _wTasks[threadID]->_storage[key] = data;
}

//------------------------------------------------------------------------------
//!
void*&
TaskQueue::getStorage( uint threadID, const String& key )
{
   CHECK( threadID < _wTasks.size() );
   //StdErr << "getStorage: TaskQueue=" << (void*)this << " WorkerTask[" << threadID << "]=" << (void*)_wTasks[threadID] << " data[" << key << "]=" << _wTasks[threadID]->_storage[key] << nl;
   return _wTasks[threadID]->_storage[key];
}

//------------------------------------------------------------------------------
//!
void
TaskQueue::print( TextStream& os )
{
#if BASE_PROFILE_TASKS
   os << nl
      << "TaskQueue:"
      << " ran " << _nTasksSubmitted << " (deleted " << _nTasksDeleted << ") in " << _wTasks.size() << " worker threads."
      << nl;
   CHECK( _nTasksSubmitted == _nTasksDeleted );
#endif
   // Assumes _wTasks cannot change.
   for( WorkerTaskContainer::Iterator cur = _wTasks.begin();
        cur != _wTasks.end();
        ++cur )
   {
      (*cur)->print( os );
   }
}

//------------------------------------------------------------------------------
//!
void
TaskQueue::createThreads( uint n )
{
   LockGuard  lock( _wTasksLock );
   for( uint i = 0; i < n; ++i )
   {
      WorkerTask* task = new WorkerTask( this );
      Thread* thread = new Thread( task, false );
      _wTasks.pushBack( task );
      _threads.pushBack( thread );
   }
}

//------------------------------------------------------------------------------
//!
void
TaskQueue::deleteThreads()
{
   waitForTerm();

   LockGuard  lock( _wTasksLock );
   uint s = uint(_threads.size());
   for( uint i = 0; i < s; ++i )
   {
      Thread* thread = _threads.back();

      _threads.popBack();
      _wTasks.popBack();

      delete thread;
   }
}

//------------------------------------------------------------------------------
//!
void
TaskQueue::post( Task* task )
{
   DBG_BLOCK( os_mt, "TaskQueue::post(" << (void*)task << ")" );

   ++_nTasks;
   PROFILE_TASK_OPERATION( ++_nTasksSubmitted );

   WorkerTask* queue = findBestQueue();
   queue->pushBack( task );
}

//------------------------------------------------------------------------------
//!
void
TaskQueue::post( Task* task, WorkerTask* wt )
{
   DBG_BLOCK( os_mt, "TaskQueue::post(" << (void*)task << ", " << (void*)wt << ")" );

   ++_nTasks;
   PROFILE_TASK_OPERATION( ++_nTasksSubmitted );

   wt->pushFront( task );
}

//------------------------------------------------------------------------------
//! Waits for all outstanding job queues to be empty.
void
TaskQueue::waitForAll()
{
   DBG_BLOCK( os_mt, "TaskQueue::waitForAll()" );
   // Wait for all tasks to complete.
   _nTasks.wait();
}

//------------------------------------------------------------------------------
//! Waits for all outstanding job queues to be empty.
void
TaskQueue::waitForTerm()
{
   DBG_BLOCK( os_mt, "TaskQueue::waitForAll()" );
   // Assumes _wTasks cannot change.
   for( WorkerTaskContainer::Iterator cur = _wTasks.begin();
        cur != _wTasks.end();
        ++cur )
   {
      bool& needsToStop = const_cast<bool&>( (*cur)->_needsToStop );
      needsToStop = true;
      (*cur)->_tasksSema.post(); // Wake a potentially waiting task.
   }

   // Wait for all tasks to finish executing.
   _nThreads.wait();
}

//------------------------------------------------------------------------------
//!
WorkerTask*
TaskQueue::findBestQueue()
{
   DBG_BLOCK( os_mt, "TaskQueue::findBestQueue()" );
   WorkerTask*  bestTask = NULL;
   size_t       bestSize = (size_t)-1; // Or std::limits<size_t>::max() if you prefer.
   // Assumes _wTasks cannot change.
   for( WorkerTaskContainer::Iterator cur = _wTasks.begin();
        cur != _wTasks.end();
        ++cur )
   {
      WorkerTask* t = *cur;
      uint        s = t->size();
      if( s == 0 )
      {
         // Early out: as soon as a queue is empty.
         DBG_MSG( os_mt, "Finding empty queue at slot in " << (void*)t );
         return t;
      }
      else
      if( s < bestSize )
      {
         bestTask = t;
         bestSize = s;
      }
   }
   DBG_MSG( os_mt, "Best queue of size " << bestSize << " (" << (void*)bestTask << ")" );
   return bestTask;
}

//------------------------------------------------------------------------------
//!
WorkerTask*
TaskQueue::findEmptyQueue()
{
   DBG_BLOCK( os_mt, "TaskQueue::findEmptyQueue()" );
   // Assumes _wTasks cannot change.
   for( WorkerTaskContainer::Iterator cur = _wTasks.begin();
        cur != _wTasks.end();
        ++cur )
   {
      WorkerTask* t = *cur;
      if( t->outOfTasks() )  return t;
   }
   return NULL;
}

//-----------------------------------------------------------------------------
//!
void
TaskQueue::awakeAll()
{
   DBG_BLOCK( os_mt, "TaskQueue::awakeAll()" );
   // TODO: Add a condition.
   for( WorkerTaskContainer::Iterator cur = _wTasks.begin();
        cur != _wTasks.end();
        ++cur )
   {
      WorkerTask* t = *cur;
      t->awake();
   }
}


/*==============================================================================
  CLASS WorkerTask
==============================================================================*/

//------------------------------------------------------------------------------
//!
WorkerTask::WorkerTask( TaskQueue* queue ):
   _queue( queue ),
   _tasksSema( 0 ),
   _needsToStop( false )
{
   DBG_BLOCK( os_mt, "WorkerTask::WorkerTask( " << (void*)queue << " ) " << (void*)this );
   //StdErr << "WorkerTask::WorkerTask( " << (void*)queue << " ) " << (void*)this << nl;

   PROFILE_TASK_OPERATION( _timeWaiting = 0.0 );
   PROFILE_TASK_OPERATION( _timeWorking = 0.0 );
   PROFILE_TASK_OPERATION( _timeLocking = 0.0 );
   PROFILE_TASK_OPERATION( _timeOthers  = 0.0 );
   PROFILE_TASK_OPERATION( _nTasksSubmitted = 0 );
}

//------------------------------------------------------------------------------
//!
WorkerTask::~WorkerTask()
{
   DBG_BLOCK( os_mt, "WorkerTask::~WorkerTask() " << (void*)this );
}

//------------------------------------------------------------------------------
//!
void
WorkerTask::print( TextStream& os ) const
{
#if BASE_PROFILE_TASKS
   double timeTotal = _timeWaiting + _timeWorking + _timeLocking + _timeOthers;
   os << (void*)this << ":"
      << " ran " << _nTasksExecuted << "/" << _nTasksSubmitted
      << " W=" << _timeWorking
      << " I=" << _timeWaiting
      << " L=" << _timeLocking
      << " O=" << _timeOthers
      << " Tot=" << timeTotal
      << "(" << 100.0*_timeWorking/timeTotal << "% working " << 100.0*(_timeWaiting+_timeLocking)/timeTotal << "% stalled)"
      << " avg " << _timeWorking/_nTasksExecuted << " s/task"
      << nl;
#endif
}

//------------------------------------------------------------------------------
//!
Task*
WorkerTask::getFrontTask()
{
   PROFILE_TASK_OPERATION( _timeOthers  += _timer.restart() );
   LockGuard lock( _tasksLock );
   PROFILE_TASK_OPERATION( _timeLocking += _timer.restart() );

   if( !_tasks.empty() )
   {
      Task* task = _tasks.front();
      _tasks.popFront();
      return task;
   }

   return NULL;
}

//------------------------------------------------------------------------------
//!
Task*
WorkerTask::getBackTask()
{
   PROFILE_TASK_OPERATION( _timeOthers  += _timer.restart() );
   LockGuard lock( _tasksLock );
   PROFILE_TASK_OPERATION( _timeLocking += _timer.restart() );

   if( !_tasks.empty() )
   {
      Task* task = _tasks.back();
      _tasks.popBack();
      return task;
   }

   return NULL;
}

//------------------------------------------------------------------------------
//! Iterate over all friend WorkerTasks to find a candidate to run.
Task*
WorkerTask::findTaskToOffload()
{
   TaskQueue& q = queue();

   // Assumes tm._wTasks cannot change.
   for( TaskQueue::WorkerTaskContainer::Iterator it = q._wTasks.begin();
        it != q._wTasks.end();
        ++it )
   {
      WorkerTask* wt = *it;

      // Can't offload from ourselves.
      if( wt == this )  continue;

      Task* task = wt->getBackTask();
      if( task )  return task;
   }

   return NULL;
}

//------------------------------------------------------------------------------
//!
void
WorkerTask::waitForAll( Task* parentTask )
{
   DBG_BLOCK( os_mt, "WorkerTask::waitForAll(" << (void*)parentTask << ") " << (void*)this );

   // Do we have task to wait for?
   if( parentTask && parentTask->_count == 1 ) return;

   do
   {
      PROFILE_TASK_OPERATION( _timeOthers += _timer.restart() );

      // Wait for something on the queue.
      _tasksSema.wait();

      PROFILE_TASK_OPERATION( _timeWaiting += _timer.restart() );

      // Check if we indeed have something on the queue (race condition with others offloading).
      Task* task = getFrontTask();
      if( task )  execute( task );

      // Check if we are done waiting for all our children.
      if( parentTask && parentTask->_count == 1 )  return; // End a recursive call.

      // Check if we need to offload a job (rather than waiting on an empty queue).
      while( outOfTasks() )
      {
         // Offload a job.
         if( _needsToStop )  return;

         task = findTaskToOffload();

         if( task )
         {
            execute( task );

            // Check if we are done waiting for all our children.
            if( parentTask && parentTask->_count == 1 )  return; // End a recursive call.
         }
         else
         {
            // The parentTask check above is implicit...
            break;
         }
      }
   } while( !_needsToStop );
}

//------------------------------------------------------------------------------
//!
void
WorkerTask::waitFor( const Condition& cond )
{
   DBG_BLOCK( os_mt, "WorkerTask::waitFor() " << (void*)this );

   ++(queue()._nWaitFor);

   // Is the condition already met?
   if( cond() )
   {
      --(queue()._nWaitFor);
      return;
   }

   do
   {
      PROFILE_TASK_OPERATION( _timeOthers += _timer.restart() );

      // Wait for something on the queue.
      _tasksSema.wait();

      PROFILE_TASK_OPERATION( _timeWaiting += _timer.restart() );

      // Check if we indeed have something on the queue (race condition with others offloading).
      Task* task = getFrontTask();
      if( task )  execute( task );

      // Check if we are done waiting for the condition.
      if( cond() )
      {
         // End a recursive call.
         --(queue()._nWaitFor);
         return;
      }

      // Check if we need to offload a job (rather than waiting on an empty queue).
      while( outOfTasks() )
      {
         // Offload a job.
         if( _needsToStop )  return;

         task = findTaskToOffload();

         if( task )
         {
            execute( task );

            // Check if we are done waiting for the condition.
            if( cond() )
            {
               // End a recursive call.
               --(queue()._nWaitFor);
               return;
            }
         }
         else
         {
            break;
         }
      }
   } while( !_needsToStop );

   --(queue()._nWaitFor);
}

//------------------------------------------------------------------------------
//!
void
WorkerTask::execute()
{
   DBG_BLOCK( os_mt, "WorkerTask::execute() " << (void*)this );
   ++(queue()._nThreads);
   waitForAll( NULL );
   --(queue()._nThreads);
   DBG_MSG( os_mt, "WorkerTask::execute() done " << (void*)this );
}


NAMESPACE_END
