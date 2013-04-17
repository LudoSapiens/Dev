/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_TRIGGER_H
#define BASE_TRIGGER_H

#include <Base/StdDefs.h>

#include <Base/MT/Atomic.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Trigger
==============================================================================*/
//! A class which controls signaling of triggers.
class Trigger
{
public:

   /*----- methods -----*/

   BASE_DLL_API Trigger( bool startBlocking = true, bool broadcast = true );
   BASE_DLL_API ~Trigger();

   BASE_DLL_API void  wait();
   BASE_DLL_API void  post();
   BASE_DLL_API void  reset();

protected:

   struct TriggerImp;

   /*----- data members -----*/

   TriggerImp*  _imp;

private:
}; //class Trigger

NAMESPACE_END

#endif //BASE_TRIGGER_H
