/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_RESOURCETASK_H
#define FUSION_RESOURCETASK_H

#include <Fusion/StdDefs.h>
#include <Fusion/Resource/Resource.h>

#include <Base/MT/Task.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS ResourceTask
==============================================================================*/

class ResourceTask:
   public Task
{

protected:

   /*----- methods -----*/

   template< typename T > inline T* waitForData( Resource<T>* res )
   {
      if( !res ) return NULL;
      waitFor( makeDelegate( res, &Resource<T>::isReady ) );
      CHECK( res->data() != NULL );
      return res->data();
   }
};

NAMESPACE_END

#endif
