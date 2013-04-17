/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_PROCEDURALWORLD_H
#define PLASMA_PROCEDURALWORLD_H

#include <Plasma/StdDefs.h>
#include <Plasma/Procedural/ProceduralContext.h>
#include <Plasma/World/World.h>
#include <Plasma/World/Brain.h>
#include <Plasma/Geometry/Geometry.h>

#include <Fusion/Resource/ResourceTask.h>

#include <CGMath/Variant.h>

#include <Base/Util/RCP.h>
#include <Base/ADT/String.h>
#include <Base/ADT/Pair.h>

NAMESPACE_BEGIN

class AnimationGraph;

/*==============================================================================
   CLASS WorldContext
==============================================================================*/

class WorldContext:
   public ProceduralContext
{
public:

   /*----- types -----*/

   typedef Pair< Entity*, Resource<Geometry>* >               GeomEntityPair;
   typedef Pair< Entity*, Resource<MaterialSet>* >            MatEntityPair;
   typedef Pair< Brain*, Resource<BrainProgram>* >            BrainProgPair;
   typedef Pair< SkeletalEntity*, Resource<AnimationGraph>* > SkelGraphPair;

   /*----- methods -----*/

   WorldContext( Task* t ) : ProceduralContext( WORLD, t ) {}
   ~WorldContext();

   inline void setGeometry( Entity* e, Resource<Geometry>* res )
   {
      if( res ) _geomRes.pushBack( GeomEntityPair( e, res ) );
   }
   inline void setMaterialSet( Entity* e, Resource<MaterialSet>* res )
   {
      if( res ) _matRes.pushBack( MatEntityPair( e, res ) );
   }
   inline void setBrainProgram( Brain* b, Resource<BrainProgram>* res )
   {
      if( res ) _brainRes.pushBack( BrainProgPair( b, res ) );
   }
   inline void setAnimGraph( SkeletalEntity* e, Resource<AnimationGraph>* res )
   {
      if( res ) _graphRes.pushBack( SkelGraphPair( e, res ) );
   }

   inline void keepResource( RCObject* res ) { _res.pushBack( res ); }

   /*----- data members -----*/

   World*                      _world;
   Reff                        _ref;
   Vector< RCP<RCObject> >     _res;
   Vector< GeomEntityPair >    _geomRes;
   Vector< MatEntityPair >     _matRes;
   Vector< BrainProgPair >     _brainRes;
   Vector< SkelGraphPair >     _graphRes;
   Vector< RCP<RCObject> >     _others;
};

/*==============================================================================
   CLASS ProceduralWorld
==============================================================================*/

class ProceduralWorld:
   public ResourceTask
{
public:

   /*----- static methods -----*/

   static void initialize();

   /*----- methods -----*/

   ProceduralWorld( Resource<World>*, const String& id, const String& path );
   ProceduralWorld( Resource<World>*, const String& id, const String& path, const Table& );

   virtual void execute();

private:

   /*----- data members -----*/

   RCP< Resource<World> > _res;
   RCP<const Table>       _params;
   String                 _id;
   String                 _path;
};

NAMESPACE_END

#endif
