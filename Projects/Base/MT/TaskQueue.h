/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_TASK_QUEUE_H
#define BASE_TASK_QUEUE_H

#include <Base/StdDefs.h>

#include <Base/ADT/DEQueue.h>
#include <Base/ADT/Queue.h>
#include <Base/ADT/Vector.h>
#include <Base/IO/TextStream.h>
#include <Base/MT/Lock.h>
#include <Base/MT/Semaphore.h>
#include <Base/MT/Task.h>
#include <Base/MT/ValueTrigger.h>
#include <Base/Msg/Delegate.h>
#include <Base/Msg/DelegateList.h>


#if !defined(BASE_PROFILE_TASKS)
#define BASE_PROFILE_TASKS  1  // Set to 1 to profile the task manager.
#endif


#if BASE_PROFILE_TASKS

#include <Base/Util/Timer.h>
#define PROFILE_TASK_OPERATION( expr )  expr

#else

#define PROFILE_TASK_OPERATION( expr )  /* nothing */

#endif


NAMESPACE_BEGIN

class Thread;
class TaskQueue;

/*==============================================================================
  CLASS WorkerTask
==============================================================================*/
class WorkerTask:
   public BaseTask
{
public:

   /*----- data types -----*/

   typedef Task::Condition    Condition;
   typedef Task::StorageType  StorageType;

   /*----- methods -----*/

   WorkerTask( TaskQueue* queue );

   ~WorkerTask();

   inline       TaskQueue&  queue();

   inline       StorageType&  storage()       { return _storage; }
   inline const StorageType&  storage() const { return _storage; }

   void  waitForAll( Task* parentTask );
   void  waitFor( const Condition& cond );
   void  execute();

   void  print( TextStream& os = StdErr ) const;

protected:

   /*----- data types -----*/
   typedef DEQueue< Task* >  TaskContainer;

   /*----- members -----*/

   TaskQueue*     _queue;      //!< The queue which owns the worker task.
   TaskContainer  _tasks;      //!< The tasks waiting to be executed.
   Lock           _tasksLock;  //!< A lock to access _tasks in a thread-safe manner.
   Semaphore      _tasksSema;  //!< A semaphore to put the worker task to sleep if the queue is empty.
   StorageType    _storage;    //!< User data, similar to thread-local storage.

#if BASE_PROFILE_TASKS
   Timer        _timer;       //!< A timer used to collect the times below;
   double       _timeWaiting; //!< Contains the time where the thread was waiting for work.
   double       _timeWorking; //!< Contains the time used for executing tasks.
   double       _timeLocking; //!< Contains the time used for acquiring locks.
   double       _timeOthers;  //!< Time spent handling the tasks and other stuff.
   AtomicInt32  _nTasksSubmitted; //!< The total number of tasks that passed through this thread.
   AtomicInt32  _nTasksExecuted;  //!< The total number of tasks that got executed by this thread.
#endif

private:

   friend class TaskQueue;
   friend class Task;

   /*----- members -----*/

   const bool  _needsToStop;  //!< A flag set by the TaskQueue indicating we need to terminate the task.

   /*----- methods -----*/

   inline void  execute( Task* task );

   inline bool  outOfTasks();
   inline uint  size();

   Task*  getFrontTask();
   Task*  getBackTask();
   Task*  findTaskToOffload();

   inline void  pushFront( Task* task );
   inline void  pushBack( Task* task );

   inline void  awake() { _tasksSema.post(); }

}; //class WorkerTask


/*==============================================================================
  CLASS TaskQueue
==============================================================================*/
class TaskQueue
{
public:

   /*----- static methods -----*/

   static BASE_DLL_API uint  defaultNumberOfThreads();

   /*----- methods -----*/

   BASE_DLL_API TaskQueue( uint nThreads = 0 );
   BASE_DLL_API ~TaskQueue();

   BASE_DLL_API void    print( TextStream& os = StdErr );

   BASE_DLL_API void    term( bool now = false );

   BASE_DLL_API uint    numRunningThreads();
   BASE_DLL_API uint    numAllocatedThreads();
   BASE_DLL_API uint    numTasks();
   BASE_DLL_API void    setStorage( uint threadID, const String& key, void* data );
   BASE_DLL_API void*&  getStorage( uint threadID, const String& key );

   BASE_DLL_API void    post( Task* task );
   BASE_DLL_API void    waitForAll();

protected:

   /*----- methods -----*/

   void  waitForTerm();

   inline void  addJobCount();
   inline void  removeJobCount();

   /*----- data types -----*/
   typedef Vector< Thread* >      ThreadContainer;
   typedef Vector< WorkerTask* >  WorkerTaskContainer;

   /*----- data members -----*/

   ThreadContainer      _threads;      //!< The lists of threads.
   WorkerTaskContainer  _wTasks;       //!< The working task queue.
   Lock                 _wTasksLock;   //!< A lock to guarantee single-thread access on the jobs queue.
   ValueTrigger         _nTasks;       //!< The total number of tasks currently flowing in the system.
   ValueTrigger         _nThreads;     //!< The total number of active threads.
   AtomicInt32          _nWaitFor;     //!< The total number of outstanding waitFor() calls.

#if BASE_PROFILE_TASKS
   AtomicInt32          _nTasksSubmitted; //!< The total number of submitted tasks.
   AtomicInt32          _nTasksDeleted;   //!< The total number of deleted tasks.
#endif

private:

   /*----- methods -----*/
   void  createThreads( uint n );
   void  deleteThreads();

   friend class Task;
   void  post( Task* task, WorkerTask* wt );

   friend class WorkerTask;
   WorkerTask*  findBestQueue();
   WorkerTask*  findEmptyQueue();

   void  awakeAll();
   void  awakeAllWaitFor() { if( _nWaitFor > 0 )  awakeAll(); }

}; //class TaskQueue

//------------------------------------------------------------------------------
//!
inline TaskQueue&
WorkerTask::queue()
{
   return *_queue;
}

//------------------------------------------------------------------------------
//!
inline void
WorkerTask::execute( Task* task )
{
   CHECK( task );
   CHECK( task->_workerTask == NULL );

   task->incCount();
   task->_workerTask = this;
   PROFILE_TASK_OPERATION( _timeOthers  += _timer.restart() );
   task->execute();
   PROFILE_TASK_OPERATION( _timeWorking += _timer.restart() );
   PROFILE_TASK_OPERATION( ++_nTasksExecuted );
   //task->_workerTask = NULL;
   Task* parent = task->_parent; // Need to keep this since decCount() could delete task.
   task->decCount();

   if( parent )  parent->decCount(); // Register child as completed.
   --(queue()._nTasks);
}

//------------------------------------------------------------------------------
//!
inline bool
WorkerTask::outOfTasks()
{
   // Optional lock?
   PROFILE_TASK_OPERATION( _timeOthers  += _timer.restart() );
   LockGuard lock( _tasksLock );
   PROFILE_TASK_OPERATION( _timeLocking += _timer.restart() );

   return _tasks.empty();
}

//------------------------------------------------------------------------------
//!
inline uint
WorkerTask::size()
{
   // Optional lock?
   PROFILE_TASK_OPERATION( _timeOthers  += _timer.restart() );
   LockGuard lock( _tasksLock );
   PROFILE_TASK_OPERATION( _timeLocking += _timer.restart() );

   return uint(_tasks.size());
}

//------------------------------------------------------------------------------
//!
inline void
WorkerTask::pushBack( Task* task )
{
   // Push to the back of the queue, and advertise.
   PROFILE_TASK_OPERATION( ++_nTasksSubmitted );
   {
      PROFILE_TASK_OPERATION( _timeOthers  += _timer.restart() );
      LockGuard taskLock( _tasksLock );
      PROFILE_TASK_OPERATION( _timeLocking += _timer.restart() );
      _tasks.pushBack( task );
   }
   _tasksSema.post();
}

//------------------------------------------------------------------------------
//!
inline void
WorkerTask::pushFront( Task* task )
{
   // Push to the back of the queue, and advertise.
   PROFILE_TASK_OPERATION( ++_nTasksSubmitted );
   {
      PROFILE_TASK_OPERATION( _timeOthers  += _timer.restart() );
      LockGuard taskLock( _tasksLock );
      PROFILE_TASK_OPERATION( _timeLocking += _timer.restart() );
      _tasks.pushFront( task );
   }
   _tasksSema.post();
}

NAMESPACE_END

#endif //BASE_TASK_QUEUE_H
