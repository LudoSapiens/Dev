/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_FORWARD_RENDERER_H
#define PLASMA_FORWARD_RENDERER_H

#include <Plasma/StdDefs.h>
#include <Plasma/Render/Renderer.h>

#include <CGMath/Mat4.h>

NAMESPACE_BEGIN

class TQuad;

/*==============================================================================
  CLASS ForwardRenderer
==============================================================================*/

//!

class ForwardRenderer
   : public Renderer
{

public:

   /*----- methods -----*/

   PLASMA_DLL_API ForwardRenderer();

   PLASMA_DLL_API virtual void render( const RCP<Gfx::RenderNode>&, World*, Viewport* );


protected:

   /*----- methods -----*/

   ~ForwardRenderer();
   virtual void performResize();
   virtual void performBeginFrame();
   virtual void performEndFrame();

   void renderLightPass( Gfx::RenderNode&, const Viewport& vp );
   void renderColorPass( Gfx::Pass& pass, const Viewport& vp );
   void renderMirrors( Gfx::Pass& pass );
   void renderMain( Gfx::Pass& pass );
   void renderDepth( Gfx::Pass& pass );
   void renderParticles( Gfx::Pass& pass );
   void renderGhosts( Gfx::Pass& pass );
   void renderSelections( Gfx::Pass& pass, const Viewport& vp );
   void renderRenderable( Gfx::Pass& pass, const Viewport& vp );
   void renderBuffers( const RCP<Gfx::RenderNode>&, const Viewport& vp );

   /*----- members -----*/

   World*                 _world;
   Mat4f                  _projMat;
   Mat4f                  _viewMat;
   Vec3f                  _camPos;

   RCP<Gfx::AlphaState>   _defBlending;
   RCP<Gfx::AlphaState>   _noBlending;
   RCP<Gfx::CullState>    _frontCulling;
   RCP<Gfx::CullState>    _backCulling;
   RCP<Gfx::ColorState>   _noColor;
   RCP<Gfx::ColorState>   _defColor;
   RCP<Gfx::DepthState>   _eqDepth;
   RCP<Gfx::DepthState>   _defDepth;
   RCP<Gfx::DepthState>   _noDepthWrite;
   RCP<Gfx::OffsetState>  _defOffset;
   RCP<Gfx::OffsetState>  _offset;
   Gfx::TextureState      _clampBilinear;
   Gfx::TextureState      _clampNearest;

   // Lights constants and buffers.
   RCP<Gfx::ConstantBuffer> _lightConstants;
   RCP<Gfx::Sampler>        _lightSampler;
   RCP<Gfx::Texture>        _depthTex;
   RCP<Gfx::Texture>        _depthCol[2];
   RCP<Gfx::Framebuffer>    _depthBuffer;

   // Programs.
   RCP<Gfx::Program>        _colorDirProg;
   RCP<Gfx::Program>        _colorDirSkelProg;
   RCP<Gfx::Program>        _colorDirPartProg;
   RCP<Gfx::Program>        _selectionProg;
   RCP<Gfx::Program>        _depthDirProg;
   RCP<Gfx::Program>        _depthDirSkelProg;

   // Blur filter elements.
   RCP<Gfx::RenderNode>     _blur;

   // Debug viewing.
   RCP<TQuad>               _depthQuad;
};


NAMESPACE_END

#endif
