/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_PROCEDURAL_SKELETON_H
#define PLASMA_PROCEDURAL_SKELETON_H

#include <Plasma/StdDefs.h>
#include <Plasma/Animation/Skeleton.h>

#include <Fusion/Resource/Resource.h>

#include <Base/MT/Task.h>
#include <Base/Util/RCP.h>
#include <Base/ADT/String.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS ProceduralSkeleton
==============================================================================*/

class ProceduralSkeleton:
   public Task
{
public:

   /*----- static methods -----*/

   static void initialize();

   /*----- methods -----*/

   ProceduralSkeleton( Resource<Skeleton>* res, const String& path );
 
   inline Skeleton* skeleton() { return _skel.ptr(); }

protected:

   /*----- methods -----*/

   virtual void execute();

   /*----- data members -----*/

   RCP< Resource<Skeleton> > _res;
   RCP<Skeleton>             _skel;
   String                    _path;
};

NAMESPACE_END

#endif
