/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_PROCEDURALMATERIAL_H
#define PLASMA_PROCEDURALMATERIAL_H

#include <Plasma/StdDefs.h>

#include <Fusion/Resource/Resource.h>

#include <CGMath/Variant.h>

#include <Base/MT/Task.h>

NAMESPACE_BEGIN

class MaterialSet;
class MaterialMap;

/*==============================================================================
   CLASS ProceduralMaterial
==============================================================================*/

class ProceduralMaterial:
   public Task
{
public:

   /*----- static methods -----*/

   static void initialize();
   static void terminate();

   /*----- methods -----*/

   ProceduralMaterial(
      Resource<MaterialSet>* res,
      const String&          id,
      const String&          path,
      const Table*           params,
      const MaterialMap* map
   );

protected:

   /*----- methods -----*/

   virtual void execute();

   /*----- data members -----*/

   RCP< Resource<MaterialSet> > _res;
   RCP<const Table>             _params;
   RCP<const MaterialMap>       _map;
   String                       _id;
   String                       _path;
};

NAMESPACE_END

#endif
