/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_PROCEDURAL_DATAFLOW_H
#define PLASMA_PROCEDURAL_DATAFLOW_H

#include <Plasma/StdDefs.h>
#include <Plasma/DataFlow/DFGraph.h>

#include <Fusion/Resource/ResourceTask.h>

#include <Base/Util/RCP.h>
#include <Base/ADT/String.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS ProceduralDataFlow
==============================================================================*/

class ProceduralDataFlow:
   public ResourceTask
{
public:

   /*----- static methods -----*/

   static void initialize();

   /*----- methods -----*/

   ProceduralDataFlow( 
      Resource<DFGraph>* res, 
      const String&      path 
   );
 
protected:

   /*----- methods -----*/

   virtual void execute();

   /*----- data members -----*/

   RCP< Resource<DFGraph> > _res;
   RCP<DFGraph>             _graph;
   String                   _path;
};

NAMESPACE_END

#endif
