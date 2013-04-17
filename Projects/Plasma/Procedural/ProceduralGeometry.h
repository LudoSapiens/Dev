/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_PROCEDURALGEOMETRY_H
#define PLASMA_PROCEDURALGEOMETRY_H

#include <Plasma/StdDefs.h>
#include <Plasma/Procedural/Component.h>
#include <Plasma/Procedural/ProceduralContext.h>
#include <Plasma/Geometry/MetaGeometry.h>

#include <Fusion/Resource/ResourceTask.h>

#include <Base/Util/RCP.h>
#include <Base/ADT/String.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS GeometryContext
==============================================================================*/

class GeometryContext:
   public ProceduralContext
{
public:

   /*----- structures -----*/

   struct State
   {
      State(): 
         _trf( Mat4f::identity() ), _ref( Reff::identity() ), 
         _mapping(0), _displacement(0)
      {}
      Mat4f         _trf;
      Reff          _ref;
      MetaFunction* _mapping;
      MetaFunction* _displacement;
   };

   /*----- methods -----*/

   GeometryContext( Task* t );

   /*----- types -----*/

   typedef Pair< RCP<CollisionShape>, Reff >  ShapeReferential;
   typedef Vector< ShapeReferential >         ShapeContainer;

   void pushState()
   {
      _states.pushBack( _state );
   }

   void popState()
   {
      _state = _states.back();
      _states.popBack();
   }

   void pushNode( MetaNode* node )
   {
      _nodeStack.pushBack( node );
      pushState();
   }

   void popNode()
   {
      _nodeStack.popBack();
      popState();
   }

   /*----- members -----*/

   Compositor*                         _compositor;
   MetaBuilder*                        _builder;
   State                               _state;
   MetaBlocks*                         _group;
   uint                                _grpID;
   int                                 _autoCollisionShape;
   Vector<MetaNode*>                   _nodeStack;
   Vector< Pair<MetaComposite*,bool> > _composites;
   Vector<State>                       _states;
   ShapeContainer                      _shapes;
   RCP< Resource<Skeleton> >           _skeletonRes;
   Map<ConstString,Vec3f>              _retarget;
};

/*==============================================================================
   CLASS ProceduralGeometry
==============================================================================*/

class ProceduralGeometry:
   public ResourceTask
{
public:
   
   /*----- static methods -----*/

   static void initialize();

   /*----- methods -----*/

   ProceduralGeometry( Resource<Geometry>* res, const String& path, bool compiled );
   ProceduralGeometry( Resource<Geometry>* res, const String& path, const Table&, bool compiled );

   virtual void execute();

private:

   /*----- data members -----*/

   RCP< Resource<Geometry> > _res;
   RCP<const Table>          _params;
   String                    _path;
   bool                      _compiled;
   float                     _gerror;
};

NAMESPACE_END

#endif
