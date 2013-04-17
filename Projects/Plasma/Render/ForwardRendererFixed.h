/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_FORWARD_RENDERER_FIXED_H
#define PLASMA_FORWARD_RENDERER_FIXED_H

#include <Plasma/StdDefs.h>
#include <Plasma/Render/Renderer.h>
#include <Plasma/Geometry/MaterialSet.h>

#include <CGMath/Mat4.h>

NAMESPACE_BEGIN

class Entity;

/*==============================================================================
  CLASS ForwardRendererFixed
==============================================================================*/

//!

class ForwardRendererFixed
   : public Renderer
{

public:

   /*----- methods -----*/

   PLASMA_DLL_API ForwardRendererFixed();

   PLASMA_DLL_API virtual void render( const RCP<Gfx::RenderNode>&, World*, Viewport* );


protected: 

   /*----- classes -----*/

   struct ReflGeom
   {
      ReflGeom( Entity* e, Material* m, int id ): 
         _entity(e), _mat(m), _patchId(id) {}
      Entity*   _entity;
      Material* _mat;
      int       _patchId;
   };

   /*----- methods -----*/

   ~ForwardRendererFixed();
   virtual void performResize();
   virtual void performBeginFrame();
   virtual void performEndFrame();

   void classifyEntities();
   void renderColorPass( Gfx::Pass* pass, Viewport* vp );
   void renderMirrors( Gfx::Pass* pass );
   void renderMain( Gfx::Pass* pass );
   void renderGhosts( Gfx::Pass* pass );
   void renderSelections( Gfx::Pass* pass, Viewport* vp );

   /*----- members -----*/

   World*                 _world;
   Mat4f                  _projMat;
   Mat4f                  _viewMat;
   Vec3f                  _camPos;

   RCP<MaterialSet>       _defMatSet;
   RCP<Gfx::AlphaState>   _defBlending;
   RCP<Gfx::AlphaState>   _noBlending;
   RCP<Gfx::CullState>    _frontCulling;
   RCP<Gfx::CullState>    _backCulling;
   RCP<Gfx::ColorState>   _noColor;
   RCP<Gfx::ColorState>   _defColor;
   RCP<Gfx::DepthState>   _eqDepth;
   RCP<Gfx::DepthState>   _defDepth;
   RCP<Gfx::OffsetState>  _defOffset;
   RCP<Gfx::OffsetState>  _offset;
   Gfx::TextureState      _clampBilinear;
   Gfx::TextureState      _clampNearest;

   RCP<Gfx::Program>      _colorLayerProg;
   RCP<Gfx::Program>      _selectionProg;

   Vector<ReflGeom>       _reflectionGeoms;
   Vector<Entity*>        _visibles;
   Vector<Entity*>        _ghosts;
};


NAMESPACE_END

#endif
