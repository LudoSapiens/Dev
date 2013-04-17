/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_RESMANAGER_H
#define PLASMA_RESMANAGER_H

#include <Plasma/StdDefs.h>

#include <Fusion/Resource/ResManager.h>

#include <CGMath/Variant.h>

NAMESPACE_BEGIN

class AnimationGraph;
class BrainProgram;
class DFGraph;
class Geometry;
class MeshGeometry;
class MaterialMap;
class MaterialSet;
class Skeleton;
class SkeletalAnimation;
class Sound;
class Task;
class TaskEvent;
class World;

namespace ResManager
{
   //------------------------------------------------------------------------------
   // Private Plasma API
   //------------------------------------------------------------------------------

   void initializePlasma();
   void terminatePlasma();
   void clearPlasma();

   //------------------------------------------------------------------------------
   // Public Plasma API
   //------------------------------------------------------------------------------

   // Data flow graph.
   PLASMA_DLL_API RCP< Resource<DFGraph> > getGraph(
      const String& id,
      Task*         caller
   );
   PLASMA_DLL_API String getGraphName( const Resource<DFGraph>* );
   PLASMA_DLL_API String getGraphName( const DFGraph* );
   PLASMA_DLL_API String getGraphPath( const String& id );
   inline         String getGraphPath( const DFGraph* g ) { return getGraphPath(getGraphName(g)); }

   // Brain.
   PLASMA_DLL_API RCP< Resource<BrainProgram> > getBrainProgram(
      const String& id,
      Task*         caller
   );
   PLASMA_DLL_API String getBrainProgramName( const Resource<BrainProgram>* );
   PLASMA_DLL_API String getBrainProgramName( const BrainProgram* );
   PLASMA_DLL_API void registerBrainProgram( const String& id, BrainProgram* prog );

   // Geometry.
   PLASMA_DLL_API RCP< Resource<Geometry> > getGeometry(
      const String& id,
      Task*         caller,
      bool          compiled = true
   );
   PLASMA_DLL_API RCP< Resource<Geometry> > getGeometry(
      const String& id,
      const Table&  params,
      Task*         caller,
      bool          compiled = true
   );
   PLASMA_DLL_API String getGeometryName( const Resource<Geometry>* );
   PLASMA_DLL_API String getGeometryName( const Geometry* );

   PLASMA_DLL_API void registerGeometry( const String& id, Geometry* );

   PLASMA_DLL_API RCP< Resource<Geometry> > getMeshGeometry(
      const String& id,
      const Table&  params,
      Task*         caller
   );
   PLASMA_DLL_API String getMeshGeometryName( const Resource<Geometry>* );
   PLASMA_DLL_API String getMeshGeometryName( const Geometry* );

   // Material set.
   PLASMA_DLL_API RCP< Resource<MaterialSet> > newMaterialSet(
      const String& id,
      Task*         caller
   );

   PLASMA_DLL_API RCP< Resource<MaterialSet> > newMaterialSet(
      const String& id,
      const Table&  params,
      Task*         caller
   );

   PLASMA_DLL_API RCP< Resource<MaterialSet> > newMaterialSet(
      const String&       id,
      const Table*        params,
      const MaterialMap*  map,
      Task*               caller
   );

   // Skeleton.
   PLASMA_DLL_API RCP< Resource<Skeleton> > getSkeleton(
      const String& id,
      Task*         caller
   );
   PLASMA_DLL_API String getSkeletonName( const Resource<Skeleton>* );
   PLASMA_DLL_API String getSkeletonName( const Skeleton* );
   PLASMA_DLL_API String getSkeletonName( const AnimationGraph* );

   // Skeletal animation.
   PLASMA_DLL_API RCP< Resource<SkeletalAnimation> > getAnimation(
      const String& id,
      Task*         caller
   );
   PLASMA_DLL_API String getAnimationName( const Resource<SkeletalAnimation>* );
   PLASMA_DLL_API String getAnimationName( const SkeletalAnimation* );

   PLASMA_DLL_API RCP< Resource<SkeletalAnimation> > retarget(
      SkeletalAnimation* srcAnim,
      Skeleton*          dstSkel,
      Task*              caller
   );

   // Animation graph.
   PLASMA_DLL_API RCP< Resource<AnimationGraph> > getAnimationGraph(
      const String& id,
      Task*         caller
   );
   PLASMA_DLL_API String getAnimationGraphName( const Resource<AnimationGraph>* );
   PLASMA_DLL_API String getAnimationGraphName( const AnimationGraph* );

   PLASMA_DLL_API RCP< Resource<AnimationGraph> > retarget(
      AnimationGraph* srcGraph,
      Skeleton*       dstSkel,
      Task*           caller
   );

   // World.
   PLASMA_DLL_API RCP< Resource<World> > newWorld( const String& id, Task* caller );
   PLASMA_DLL_API RCP< Resource<World> > newWorld( const String& id, const Table&, Task* caller );

} // namespace ResManager


NAMESPACE_END


#endif //PLASMA_RESMANAGER_H
