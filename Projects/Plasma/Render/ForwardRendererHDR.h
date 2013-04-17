/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_FORWARD_RENDERER_HDR_H
#define PLASMA_FORWARD_RENDERER_HDR_H

#include <Plasma/StdDefs.h>
#include <Plasma/Render/Renderer.h>
#include <Plasma/Geometry/MaterialSet.h>

#include <CGMath/Mat4.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS ForwardRendererHDR
==============================================================================*/

//!

class ForwardRendererHDR
   : public Renderer
{

public:

   /*----- methods -----*/

   PLASMA_DLL_API ForwardRendererHDR();

   PLASMA_DLL_API virtual void render( const RCP<Gfx::RenderNode>&, World*, Viewport* );

protected:

   /*----- methods -----*/

   ~ForwardRendererHDR();
   virtual void performResize();
   virtual void performBeginFrame();
   virtual void performEndFrame();

   void renderColorPass( Gfx::Pass& pass, const Viewport& vp );

   //void renderMirrors( Gfx::Pass& pass );
   void renderMain( Gfx::Pass& pass );
   void renderParticles( Gfx::Pass& pass );
   //void renderGhosts( Gfx::Pass& pass );
   //void renderSelections( Gfx::Pass& pass );
   void renderRenderables( Gfx::Pass& pass, const Viewport& vp );

   void blitResult( Gfx::Pass& pass, Viewport& vp );


   /*----- members -----*/

   World*                 _world;
   Mat4f                  _projMat;
   Mat4f                  _viewMat;
   Vec3f                  _camPos;

   Gfx::RenderNode*       _renderNode;

   RCP<MaterialSet>       _defMatSet;

   RCP<Gfx::Sampler>      _dbrdf;
   RCP<Gfx::Texture>      _color1Tex;
   RCP<Gfx::Texture>      _depthStencilTex;
   RCP<Gfx::Framebuffer>  _color1Buffer;

   RCP<Gfx::AlphaState>   _defBlending;
   RCP<Gfx::AlphaState>   _noBlending;
   RCP<Gfx::DepthState>   _noDepth;
   RCP<Gfx::DepthState>   _noDepthWrite;
   RCP<Gfx::OffsetState>  _defOffset;
   RCP<Gfx::OffsetState>  _offset;

   RCP<Gfx::SamplerList>  _blitSamplers;
   RCP<Gfx::Program>      _blitProg;

   // Programs.
   RCP<Gfx::Program>      _colorDirProg;
   RCP<Gfx::Program>      _colorDirSkelProg;
   RCP<Gfx::Program>      _colorDirPartProg;
   RCP<Gfx::Program>      _wireDirProg;
   RCP<Gfx::Program>      _wireDirSkelProg;
};


NAMESPACE_END

#endif
