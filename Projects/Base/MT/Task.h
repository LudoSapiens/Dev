/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_TASK_H
#define BASE_TASK_H

#include <Base/StdDefs.h>

#include <Base/ADT/Map.h>
#include <Base/ADT/String.h>
#include <Base/Msg/Delegate.h>
#include <Base/MT/Atomic.h>
#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>

NAMESPACE_BEGIN

class TaskQueue;
class WorkerTask;

/*==============================================================================
   CLASS BaseTask
==============================================================================*/

//! A class to execute a single task.
//! Note: If you want to send a Task to the TaskQueue, use a Task instead.

class BaseTask
{
public:

   /*----- methods -----*/

   BASE_DLL_API BaseTask();
   BASE_DLL_API virtual ~BaseTask();

   BASE_DLL_API virtual void execute() = 0;
};


/*==============================================================================
  CLASS Task
==============================================================================*/
class Task:
   public BaseTask
{
public:

   /*----- methods -----*/
   typedef Delegate0<bool>     Condition;
   typedef Map<String, void*>  StorageType;

   /*----- methods -----*/

   BASE_DLL_API Task();
   BASE_DLL_API virtual ~Task();

   BASE_DLL_API void  spawn( Task* childTask );

   BASE_DLL_API TaskQueue&  queue();

protected:

   friend class TaskQueue;
   friend class WorkerTask;

   /*----- methods -----*/

   BASE_DLL_API void  waitForAll();
   BASE_DLL_API void  waitFor( const Condition& cond );

   BASE_DLL_API       StorageType&  storage();
   BASE_DLL_API const StorageType&  storage() const;
   inline void*&  storage( const String& key ) { return storage()[key]; }

   void incCount();
   void decCount();

   /*----- data members -----*/

   Task*        _parent;      //!< The task which spawned this task.
   WorkerTask*  _workerTask;  //!< The worker task associated with the task.
   AtomicInt32  _count;       //!< The number of outstanding child tasks that were spawned.

private:
}; //class Task

NAMESPACE_END

#endif //BASE_TASK_H
