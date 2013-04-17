/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_DFSCREEN_H
#define PLASMA_DFSCREEN_H

#include <Plasma/StdDefs.h>
#include <Plasma/Widget/PlasmaScreen.h>
#include <Plasma/DataFlow/DFGraph.h>

#include <Fusion/VM/DelegateGroup.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS DFScreen
==============================================================================*/

class DFScreen:
   public PlasmaScreen
{
public:
   /*----- types -----*/

   typedef Delegate1Group<DFScreen*>  OnChangeOuputGroup;
   typedef OnChangeOuputGroup::Func   OnChangeOuputFunc;

   /*----- static methods -----*/

   static void initialize();

   /*----- methods -----*/

   PLASMA_DLL_API DFScreen();
   virtual ~DFScreen();

   PLASMA_DLL_API Vec2f timeRange() const;
   PLASMA_DLL_API void time( float );
   inline float time() const { return _time; }

   inline DFGraph* graph() const { return _graph.ptr(); }
   PLASMA_DLL_API void graph( DFGraph* );

   inline void  addOnChangeOutput   ( const OnChangeOuputFunc& d ) { _onChangeOutput.add(d);    }
   inline void  addOnChangeOutput   ( const VMRef& r )             { _onChangeOutput.add(r);    }
   inline void  removeOnChangeOutput( const OnChangeOuputFunc& d ) { _onChangeOutput.remove(d); }
   inline void  removeOnChangeOutput( const VMRef& r )             { _onChangeOutput.remove(r); }
   inline void  removeAllOnChangeOutput()                          { _onChangeOutput.clear();   }

   // VM.
   const char* meta() const;
   PLASMA_DLL_API void init( VMState* vm );
   PLASMA_DLL_API bool performGet( VMState* );
   PLASMA_DLL_API bool performSet( VMState* );
   PLASMA_DLL_API virtual bool isAttribute( const char* ) const;

protected:

   /*----- members -----*/

   void graphUpdated( DFGraph* );
   void emptyOutput();
   void changeOutput( DFNode*, World* );
   void updateImageGeom( const Vec2f& size, const Vec2f& uv );

   /*----- data members -----*/

   float                  _time;
   RCP<DFGraph>           _graph;
   RCP<World>             _world;
   RCP<Light>             _light;
   RCP<Manipulator>       _manip;
   DFNode*                _outputNode;
   OnChangeOuputGroup     _onChangeOutput;

   // Animation output.
   RCP<SkeletalAnimation> _anim;
   RCP<SkeletalEntity>    _puppet;

   // Geometry output.
   RCP<RigidEntity>       _entity;

   // Image output.
   RCP<Image>             _image;
   RCP<RigidEntity>       _imageEntity;
   RCP<MeshGeometry>      _imageMesh;
};

NAMESPACE_END

#endif

