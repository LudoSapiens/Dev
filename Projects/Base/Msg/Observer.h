/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_OBSERVER_H
#define BASE_OBSERVER_H

#include <Base/StdDefs.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Observer
==============================================================================*/

//! Base class for an observer.

class BASE_DLL_API Observer
{

public:

   /*----- methods -----*/

   virtual void update();
   virtual void destroy();
      
protected:

   /*----- methods -----*/

   Observer();
   virtual ~Observer();
};

NAMESPACE_END

#endif
