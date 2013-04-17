/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_OBJECTIMPORTER_H
#define PLASMA_OBJECTIMPORTER_H

#include <Plasma/StdDefs.h>

#include <Fusion/VM/VM.h>

#include <Base/IO/Path.h>
#include <Base/Util/RCP.h>

NAMESPACE_BEGIN

class Light;
class StaticEntity;
class Surface;

/*==============================================================================
  CLASS ObjectImporter
==============================================================================*/

//!

class PLASMA_DLL_API ObjectImporter
{

public:

   /*----- static methods -----*/
   static void initialize( VMState* vm, const char* nameSpace );

   /*----- methods -----*/

   ObjectImporter();
   ~ObjectImporter();

   RCP<StaticEntity> importObject( const Path& name, float scale = 1.0f );

   // FIXME: Temp
   //void exportObj( const Path& name, const RCP<Object>& obj );

private:

   /*----- methods -----*/

   RCP<StaticEntity> importObj( const Path&, float scale );
   RCP<StaticEntity> import3ds( const Path&, float scale );

   void import_ply(const Path& name, float scale, const RCP<StaticEntity>& object);
   void import_fab(const Path& name, float scale, const RCP<StaticEntity>& object);

   void import3ds(
      const Path& name,
      float scale,
      bool compress,
      Vector< RCP<StaticEntity> >& objects,
      Vector< RCP<Light> >& lights
   );
}
;

NAMESPACE_END

#endif //PLASMA_OBJECTIMPORTER_H
