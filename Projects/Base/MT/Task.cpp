/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Base/MT/Task.h>

#include <Base/MT/TaskQueue.h>

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

//DBG_STREAM( os_mt, "MT" );

UNNAMESPACE_END


NAMESPACE_BEGIN

/*==============================================================================
   CLASS BaseTask
==============================================================================*/

//------------------------------------------------------------------------------
//!
BaseTask::BaseTask()
{
}

//------------------------------------------------------------------------------
//!
BaseTask::~BaseTask()
{
}


/*==============================================================================
   CLASS Task
==============================================================================*/

//------------------------------------------------------------------------------
//!
Task::Task():
   _parent( NULL ),
   _workerTask( NULL ),
   _count( 0 )
{
}

//------------------------------------------------------------------------------
//!
Task::~Task()
{
}

//------------------------------------------------------------------------------
//!
void
Task::spawn( Task* childTask )
{
   CHECK( childTask->_parent == NULL );
   childTask->_parent = this;
   incCount();

   if( _workerTask->outOfTasks() )
   {
      queue().post( childTask, _workerTask );
      return;
   }

   // Try to find an empty friend.
   WorkerTask* wt = queue().findEmptyQueue();
   if( wt )
   {
      queue().post( childTask, wt );
      return;
   }

   // No good candidate, just queue on us.
   queue().post( childTask, _workerTask );
}

//------------------------------------------------------------------------------
//!
TaskQueue&
Task::queue()
{
   WorkerTask* wt = _workerTask;
   return wt->queue();
}

//------------------------------------------------------------------------------
//! Wait for all of its children tasks to complete.
void
Task::waitForAll()
{
   _workerTask->waitForAll( this );
}

//------------------------------------------------------------------------------
//! Wait for a specific condition to be met.
void
Task::waitFor( const Condition& cond )
{
   _workerTask->waitFor( cond );
}

//------------------------------------------------------------------------------
//!
Task::StorageType&
Task::storage()
{
   return _workerTask->storage();
}

//------------------------------------------------------------------------------
//!
const Task::StorageType&
Task::storage() const
{
   return _workerTask->storage();
}

//------------------------------------------------------------------------------
//!
void
Task::incCount()
{
   ++_count;
}

//------------------------------------------------------------------------------
//!
void
Task::decCount()
{
   WorkerTask* w = _workerTask;
   int32_t c     = --_count;
   if( c == 1 )
   {
      // A child just decremented its parent count, so wake the parent's queue.
      w->awake();
   }
   else
   if( c == 0 )
   {
      // Need to awake any WorkerTask sleeping inside a waitFor().
      w->queue().awakeAllWaitFor();
#if BASE_PROFILE_TASKS
      ++(queue()._nTasksDeleted);
#endif
      delete this;
   }
}

NAMESPACE_END
