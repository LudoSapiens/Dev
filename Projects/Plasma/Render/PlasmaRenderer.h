/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_PLASMARENDERER_H
#define PLASMA_PLASMARENDERER_H

#include <Plasma/StdDefs.h>

#include <Gfx/Pass/RenderNode.h>
#include <Gfx/FB/Framebuffer.h>

#include <CGMath/Mat4.h>
#include <CGMath/Vec2.h>

#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>
#include <Base/ADT/Set.h>

NAMESPACE_BEGIN

class Viewport;
class World;
class LightRenderer;

/*==============================================================================
  CLASS PlasmaRenderer
==============================================================================*/

//!

class PlasmaRenderer
   : public RCObject
{

public:

   /*----- static methods -----*/

   PLASMA_DLL_API static void initialize();
   PLASMA_DLL_API static const Vector< Set<String> >& constantGroups();

   /*----- methods -----*/

   PLASMA_DLL_API PlasmaRenderer();

   PLASMA_DLL_API void size( const Vec2i& size );
   inline const Vec2i& size() const;

   PLASMA_DLL_API void beginFrame();
   PLASMA_DLL_API void endFrame();

   PLASMA_DLL_API void render( const RCP<Gfx::RenderNode>&, World*, Viewport* );

   PLASMA_DLL_API void renderWorld( Gfx::Pass* );
   PLASMA_DLL_API void renderWorldDepth( Gfx::Pass* );
   PLASMA_DLL_API void renderWorldShadow( Gfx::Pass* );
   PLASMA_DLL_API void renderWorldRefractive( Gfx::Pass* );

protected: 

   /*----- methods -----*/

   virtual ~PlasmaRenderer();

   void renderDepthPass( Viewport* );
   void renderColorPass( Viewport* );
   void renderSelectionPass( Viewport* );
   void renderDistortionPass( Viewport* );
   void blitResult( const RCP<Gfx::Pass>&, Viewport* );

   Gfx::Pass* getPass();
   Gfx::RenderNode* getRenderNode();

private: 

   /*----- data members -----*/

   Mat4f                            _projMat;
   Mat4f                            _viewMat;
   Vec3f                            _camPos;

   Vec2i                            _size;

   World*                           _world;

   Gfx::RenderNode*                 _renderNode;
   RCP<LightRenderer>               _lightRenderer;

   RCP<Gfx::Texture>                _color1Tex;
   RCP<Gfx::Texture>                _color2Tex;
   RCP<Gfx::Texture>                _color3Tex;
   RCP<Gfx::Texture>                _depthStencilTex;
   RCP<Gfx::Texture>                _depthTex;
  
   RCP<Gfx::Framebuffer>            _depthBuffer;
   RCP<Gfx::Framebuffer>            _color1Buffer;
   RCP<Gfx::Framebuffer>            _color3Buffer;

   uint                             _numPasses;
   Vector< RCP<Gfx::Pass> >         _passes;
   uint                             _numRenderNodes;
   Vector< RCP<Gfx::RenderNode> >   _renderNodes;

   uint                             _numLists;
   Vector< RCP<Gfx::ConstantList> > _constants;
   Vector< RCP<Gfx::SamplerList>  > _samplers;

   RCP<Gfx::ConstantBuffer>         _selection1Constants;
   RCP<Gfx::ConstantBuffer>         _selection2Constants;
   RCP<Gfx::SamplerList>            _selectionSamplers;
   RCP<Gfx::Program>                _selectionProg;

   RCP<Gfx::ConstantBuffer>         _distortionConstants;
   RCP<Gfx::SamplerList>            _distortionSamplers;

   RCP<Gfx::SamplerList>            _blitSamplers;
   RCP<Gfx::Program>                _blitProg;

   RCP<Gfx::Program>                _depthProg;
   RCP<Gfx::Program>                _skelDepthProg;

   RCP<Gfx::Program>                _shadowProg;
   RCP<Gfx::Program>                _skelShadowProg;

   RCP<Gfx::Program>                _colorProg;
   RCP<Gfx::Program>                _skelColorProg;
   RCP<Gfx::Sampler>                _depthSampler;
};

//------------------------------------------------------------------------------
//! 
inline const Vec2i& 
PlasmaRenderer::size() const
{
   return _size;
}

NAMESPACE_END

#endif
