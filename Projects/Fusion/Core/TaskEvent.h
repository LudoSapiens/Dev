/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_TASKEVENT_H
#define FUSION_TASKEVENT_H

#include <Fusion/StdDefs.h>

#include <Base/Util/RCObject.h>

NAMESPACE_BEGIN


/*==============================================================================
  CLASS TaskEvent
==============================================================================*/

//! Base class for task event

class TaskEvent:
   public RCObject
{

public:

   /*----- methods -----*/

   virtual void execute() = 0;
};

NAMESPACE_END

#endif


