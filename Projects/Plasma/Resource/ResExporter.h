/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_RESEXPORTER_H
#define PLASMA_RESEXPORTER_H

#include <Plasma/StdDefs.h>
#include <Plasma/Geometry/Geometry.h>

#include <CGMath/Ref.h>
#include <CGMath/Variant.h>

#include <Base/Msg/Delegate.h>
#include <Base/IO/Path.h>
#include <Base/IO/StreamIndent.h>
#include <Base/IO/TextStream.h>

NAMESPACE_BEGIN

class CollisionShape;
class Material;
class SkeletalAnimation;
class SkeletalPose;
class Skeleton;
class World;
class DFGraph;

/*==============================================================================
   NAMEPACE ResExporter
==============================================================================*/

namespace ResExporter
{

   /*----- types and enumerations -----*/

   typedef Delegate2<const World&, const Path&, bool>              WorldHandler;
   typedef Delegate2<const Geometry&, const Path&, bool>           GeomHandler;
   typedef Delegate2<const SkeletalAnimation&, const Path&, bool>  AnimHandler;

   /*----- methods -----*/

   void  initialize();

   template< typename T > PLASMA_DLL_API const String&  defaultExtension();

   // Dispatching/default format.
   PLASMA_DLL_API void  addSupport( const String& ext, const WorldHandler& cb );
   PLASMA_DLL_API void  addSupport( const String& ext, const GeomHandler& cb );
   PLASMA_DLL_API void  addSupport( const String& ext, const AnimHandler& cb );

   PLASMA_DLL_API bool  save( const World& world, const Path& path );
   PLASMA_DLL_API bool  save( const Geometry& geom, const Path& path );
   PLASMA_DLL_API bool  save( const SkeletalAnimation& anim, const Path& path );
   PLASMA_DLL_API bool  save( DFGraph&, const Path& path, const Table* params = nullptr );

   // Lua format.
   namespace Lua
   {
      PLASMA_DLL_API bool  save( const World& world, const Path& path );
      PLASMA_DLL_API bool  save( const World& world, const Path& localPath, TextStream& os, StreamIndent& indent );
      PLASMA_DLL_API bool  save( const Geometry& geom, const Path& path );
      PLASMA_DLL_API bool  save( const Geometry& geom, TextStream& os, StreamIndent& indent );
      PLASMA_DLL_API bool  save( const MaterialSet&, const Path& path );
      PLASMA_DLL_API bool  save( const MaterialSet&, TextStream& os, StreamIndent& indent );

      PLASMA_DLL_API bool  save(
         Geometry::CollisionType type,
         const CollisionShape*   shape,
         TextStream&             os,
         StreamIndent&           indent
      );
      PLASMA_DLL_API bool  save(
         const CollisionShape* shape,
         const Reff*           ref,
         TextStream&           os,
         StreamIndent&         indent
      );
      PLASMA_DLL_API bool  save( const SkeletalAnimation& anim, const Path& path );
      PLASMA_DLL_API bool  save( const SkeletalAnimation& anim, TextStream& os, StreamIndent& indent );
      PLASMA_DLL_API bool  save( const SkeletalPose& pose, TextStream& os, StreamIndent& indent );
   }

   // Collada.
   namespace Collada
   {
      PLASMA_DLL_API bool  save( const World& world, const Path& path );
      PLASMA_DLL_API bool  save( const World& world, TextStream& os, StreamIndent& indent );
   }

   // Binary.
   namespace Bin
   {
      PLASMA_DLL_API bool  save( const Geometry& geom, const Path& path );
      PLASMA_DLL_API bool  saveCompressed( const Geometry& geom, const Path& path );
      PLASMA_DLL_API bool  save( const Geometry& geom, IODevice* dev );
   }

   // Obj
   namespace Obj
   {
      PLASMA_DLL_API bool  save( const Geometry& geom, const Path& path );
      PLASMA_DLL_API bool  save( const Geometry& geom, TextStream& os, StreamIndent& indent );
   }

} // namespace ResExporter

NAMESPACE_END

#endif
