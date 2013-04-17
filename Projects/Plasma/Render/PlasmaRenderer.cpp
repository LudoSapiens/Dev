/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Render/PlasmaRenderer.h>

#include <Plasma/Particle/ParticleManager.h>
#include <Plasma/Renderable/Renderable.h>
#include <Plasma/World/Entity.h>
#include <Plasma/World/Light.h>
#include <Plasma/World/Material.h>
#include <Plasma/World/Selection.h>
#include <Plasma/World/SkeletalEntity.h>
#include <Plasma/World/Viewport.h>
#include <Plasma/World/World.h>
#include <Plasma/Procedural/MetaGeometry.h>

#include <Fusion/Core/Core.h>
#include <Fusion/Resource/ResManager.h>

#include <Base/Dbg/Defs.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
Vector< Set<String> >  _constantGroups(3);
RCP<Gfx::AlphaState>   _defBlending;
RCP<Gfx::AlphaState>   _noBlending;
RCP<Gfx::AlphaState>   _addBlending;
RCP<Gfx::DepthState>   _noDepth;
RCP<Gfx::DepthState>   _noDepthWrite;
RCP<Gfx::DepthState>   _depthEq;
RCP<Gfx::DepthState>   _defDepth;
RCP<Gfx::ColorState>   _noColor;
Gfx::TextureState      _clampBilinear;
Gfx::TextureState      _clampNearest;


UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   CLASS LightRenderer
==============================================================================*/

class LightRenderer
   : public RCObject
{

public:

   /*----- methods -----*/

   LightRenderer( PlasmaRenderer* renderer, uint size, uint maxLight ) :
      _renderer( renderer ), _size( size ), _maxLight( maxLight ),
      _numLights(0), _numPass(0), _numFilterPass(0),
      _numFilterBuffer(0)
   {
      // Main hires renderable textures.
      _zstencilTex = Core::gfx()->create2DTexture(
         size, size, Gfx::TEX_FMT_24_8, Gfx::TEX_CHANS_ZS, Gfx::TEX_FLAGS_RENDERABLE
      );

      _depthTex = Core::gfx()->create2DTexture(
         size, size, Gfx::TEX_FMT_16F_16F_16F_16F, Gfx::TEX_CHANS_RGBA, Gfx::TEX_FLAGS_RENDERABLE
      );

      // Main buffer.
      _depthBuffer = Core::gfx()->createFramebuffer();
      _depthBuffer->setColorBuffer( _depthTex );
      _depthBuffer->setDepthBuffer( _zstencilTex );

      // Filtering program and constants.
      _filterProg = ResManager::getProgram( "shader/program/f_downsample4x4" );
      RCP<Gfx::ConstantBuffer> constants = Core::gfx()->createConstants( _filterProg );
      constants->setConstant( "texelSize", Vec2f( 1.0f/(float)size ).ptr() );
      _filterConstants = Gfx::ConstantList::create( constants );
   }

   void render( const RCP<Gfx::RenderNode>& rn, Light* light )
   {
      // Shadowmap pass.
      Gfx::Pass* pass = getPass();
      rn->addPass( pass );

      pass->setProjectionMatrixPtr( light->projectionMatrix().ptr() );
      pass->setViewMatrixPtr( light->viewMatrix().ptr() );
      pass->setAlphaState( _noBlending );

      _renderer->renderWorldShadow( pass );

      // Filtering pass.
      rn->addPass( getFilterPass() );

      // Constants.
      _constantBuffer->setConstant( 1+_numLights*3+0, light->position().ptr() );
      _constantBuffer->setConstant( 1+_numLights*3+1, light->color().ptr() );
      _constantBuffer->setConstant( 1+_numLights*3+2, light->lightMatrix().ptr() );

      ++_numLights;
   }

   void setLists( const RCP<Gfx::ConstantList> constants, const RCP<Gfx::SamplerList>& samplers )
   {
      float numLights = (float)_numLights;
      _constantBuffer->setConstant( 0, &numLights );
      constants->addBuffer( _constantBuffer );

      for( uint i = 0; i < _numLights; ++i )
      {
         samplers->addSampler( _samplers[i] );
      }
   }

   void beginFrame()
   {
      _numPass       = 0;
      _numFilterPass = 0;
      _numConstants  = 0;
   }

   void endFrame()
   {
   }

   void beginBatch()
   {
      _numLights       = 0;
      _numFilterBuffer = 0;

      setConstants();
   }

   void endBatch()
   {
   }


protected:

   /*----- methods -----*/

   virtual ~LightRenderer() {}

   Gfx::Pass* getPass()
   {
      if( _numPass >= _passes.size() )
      {
         _passes.pushBack( new Gfx::Pass() );
         _passes.back()->setFramebuffer( _depthBuffer );
         _passes.back()->setClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
      }
      _passes[_numPass]->clear();
      return _passes[_numPass++].ptr();
   }

   Gfx::Pass* getFilterPass()
   {
      if( _numFilterPass >= _filterPasses.size() )
      {
         _filterPasses.pushBack( new Gfx::Pass() );
         Gfx::Pass* pass = _filterPasses.back().ptr();

         // Setting states.
         pass->setAlphaState( _noBlending );
         pass->setDepthState( _noDepth );

         // Set samplers
         RCP<Gfx::SamplerList> sl( new Gfx::SamplerList() );
         sl->addSampler( "tex", _depthTex, _clampBilinear );
         pass->setSamplers(sl);

         // Set program and constants.
         pass->setProgram( _filterProg );
         pass->setConstants( _filterConstants );

         // View.
         int size = _size/2;
         pass->setViewport(0, 0, size, size );

         if( Core::gfx()->oneToOneOffset() != 0 )
         {
            Mat4f view = Mat4f::translate(-1.0f/size, -1.0f/size, 0.0f);
            pass->setViewMatrix( view.ptr() );
         }

         // Set geometry
         pass->execGeometry( Core::gfx()->getOneToOneGeometry() );
      }

       _filterPasses[_numFilterPass]->setFramebuffer( getFilterBuffer() );
      return _filterPasses[_numFilterPass++].ptr();
   }

   RCP<Gfx::Framebuffer>& getFilterBuffer()
   {
      if( _numFilterBuffer >= _filterBuffers.size() )
      {
         // Texture.
         _filterTexture.pushBack(
            Core::gfx()->create2DTexture(
               _size/2, _size/2, Gfx::TEX_FMT_16F_16F_16F_16F,
               Gfx::TEX_CHANS_RGBA, Gfx::TEX_FLAGS_RENDERABLE
            )
         );
         // Buffer.
         _filterBuffers.pushBack( Core::gfx()->createFramebuffer() );
         _filterBuffers.back()->setColorBuffer( _filterTexture.back() );

         // Sampler.
         String name = String( "light" ) + String( _numFilterBuffer ) + String( "_depth" );
         _samplers.pushBack( new Gfx::Sampler( name, _filterTexture.back(), _clampNearest ) );
      }
      return _filterBuffers[_numFilterBuffer++];
   }

   void setConstants()
   {
      if( _numConstants >= _constants.size() )
      {
         _constants.pushBack( Core::gfx()->createConstants( (24+16*4)*_maxLight + 4 ) );
         _constants.back()->addConstant( String( "numLights" ), Gfx::CONST_FLOAT, 0 );
         uint inc = 4;
         for( uint i = 0; i < _maxLight; ++i )
         {
            String name = String( "light" ) + String( i );
            _constants.back()->addConstant( name + String( "_position" ), Gfx::CONST_FLOAT3, inc );
            inc += 12;

            _constants.back()->addConstant( name + String( "_color" ), Gfx::CONST_FLOAT3, inc );
            inc += 12;

            _constants.back()->addConstant( name + String( "_matrix" ), Gfx::CONST_MAT4, inc );
            inc += 16*4;
         }
      }
      _constantBuffer = _constants[_numConstants++].ptr();
   }

private:

   /*----- data members -----*/

   PlasmaRenderer*                    _renderer;
   uint                               _size;
   uint                               _maxLight;
   RCP<Gfx::Texture>                  _zstencilTex;
   RCP<Gfx::Texture>                  _depthTex;
   RCP<Gfx::Framebuffer>              _depthBuffer;
   RCP<Gfx::Program>                  _filterProg;
   RCP<Gfx::ConstantList>             _filterConstants;

   uint                               _numLights;
   uint                               _numConstants;
   Gfx::ConstantBuffer*               _constantBuffer;
   Vector< RCP<Gfx::ConstantBuffer> > _constants;
   Vector< RCP<Gfx::Sampler> >        _samplers;

   uint                               _numPass;
   Vector< RCP<Gfx::Pass> >           _passes;
   uint                               _numFilterPass;
   Vector< RCP<Gfx::Pass> >           _filterPasses;

   uint                               _numFilterBuffer;
   Vector< RCP<Gfx::Texture> >        _filterTexture;
   Vector< RCP<Gfx::Framebuffer> >    _filterBuffers;
};


/*==============================================================================
  CLASS PlasmaRenderer
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
PlasmaRenderer::initialize()
{
   _constantGroups[0].add( String( "boneMatrices" ) );
   _constantGroups[1].add( String( "numLights" ) );
   _constantGroups[1].add( String( "light0_position" ) );
   _constantGroups[1].add( String( "light0_color" ) );
   _constantGroups[1].add( String( "light0_matrix" ) );
   _constantGroups[1].add( String( "light1_position" ) );
   _constantGroups[1].add( String( "light1_color" ) );
   _constantGroups[1].add( String( "light1_matrix" ) );
   _constantGroups[1].add( String( "light2_position" ) );
   _constantGroups[1].add( String( "light2_color" ) );
   _constantGroups[1].add( String( "light2_matrix" ) );
   _constantGroups[1].add( String( "light3_position" ) );
   _constantGroups[1].add( String( "light3_color" ) );
   _constantGroups[1].add( String( "light3_matrix" ) );
   _constantGroups[2].add( String( "texelSize" ) );

   // Create needed states.
   _defBlending = new Gfx::AlphaState();
   _defBlending->alphaTesting( true );
   _defBlending->alphaBlending( true );
   _defBlending->setAlphaBlend( Gfx::ALPHA_BLEND_ONE, Gfx::ALPHA_BLEND_ONE_MINUS_SRC_ALPHA );

   _noBlending = new Gfx::AlphaState();
   _noBlending->alphaBlending( false );
   _noBlending->alphaTesting( false );

   _addBlending = new Gfx::AlphaState();
   _addBlending->alphaTesting( true );
   _addBlending->alphaBlending( true );
   _addBlending->setAlphaBlend( Gfx::ALPHA_BLEND_ONE, Gfx::ALPHA_BLEND_ONE );

   _depthEq = new Gfx::DepthState();
   _depthEq->depthTesting( true );
   _depthEq->depthTestFunc( Gfx::COMPARE_FUNC_EQUAL );
   _depthEq->depthWriting( false );

   _noDepth = new Gfx::DepthState();
   _noDepth->depthTesting( false );
   _noDepth->depthWriting( false );

   _noDepthWrite = new Gfx::DepthState();
   _noDepthWrite->depthTesting( true );
   _noDepthWrite->depthWriting( false );

   _defDepth = new Gfx::DepthState();

   _noColor = new Gfx::ColorState();
   _noColor->colorWriting( false );

   _clampBilinear.setBilinear();
   _clampBilinear.clamp( Gfx::TEX_CLAMP_LAST );

   _clampNearest.setPointSampling();
   _clampNearest.clamp( Gfx::TEX_CLAMP_LAST );
}

//------------------------------------------------------------------------------
//!
const Vector< Set<String> >&
PlasmaRenderer::constantGroups()
{
   return _constantGroups;
}

//------------------------------------------------------------------------------
//!
PlasmaRenderer::PlasmaRenderer() :
   _size( 0 ),
   _numPasses(0),
   _numRenderNodes(0)
{
   _lightRenderer  = new LightRenderer( this, 1024, 4 );

   // Programs.
   _depthProg      = ResManager::getProgram( "shader/program/depth" );
   _skelDepthProg  = ResManager::getProgram( "shader/program/skelDepth" );
   _shadowProg     = ResManager::getProgram( "shader/program/shadow" );
   _skelShadowProg = ResManager::getProgram( "shader/program/skelShadow" );
   _colorProg      = ResManager::getProgram( "shader/program/color" );
   _skelColorProg  = ResManager::getProgram( "shader/program/skelColor" );
   _selectionProg  = ResManager::getProgram( "shader/program/f_selection" );
   _blitProg       = ResManager::getProgram( "shader/program/f_passthrough" );

   _selection1Constants = Core::gfx()->createConstants( _colorProg );
   _selection1Constants->setConstant( "color", Vec4f( 1.0 ).ptr() );

   _selection2Constants = Core::gfx()->createConstants( _selectionProg );

   _distortionConstants = Core::gfx()->createConstants( sizeof(Vec2f) + sizeof(float) );
   _distortionConstants->addConstant( "texelSize", Gfx::CONST_FLOAT2, 0 );
   _distortionConstants->addConstant( "eta", Gfx::CONST_FLOAT, sizeof(Vec2f) );
}

//------------------------------------------------------------------------------
//!
PlasmaRenderer::~PlasmaRenderer()
{
}

//------------------------------------------------------------------------------
//!
void
PlasmaRenderer::size( const Vec2i& size )
{
   if( size == _size )
   {
      return;
   }
   _size = size;


   // Allocate textures.
   _color1Tex = Core::gfx()->create2DTexture(
      _size.x, _size.y,
      Gfx::TEX_FMT_16F_16F_16F_16F, Gfx::TEX_CHANS_RGBA, Gfx::TEX_FLAGS_RENDERABLE
   );
   _color2Tex = Core::gfx()->create2DTexture(
      _size.x, _size.y,
      Gfx::TEX_FMT_16F_16F_16F_16F, Gfx::TEX_CHANS_RGBA, Gfx::TEX_FLAGS_RENDERABLE
   );
   _color3Tex = Core::gfx()->create2DTexture(
      _size.x, _size.y,
      Gfx::TEX_FMT_8_8_8_8, Gfx::TEX_CHANS_RGBA, Gfx::TEX_FLAGS_RENDERABLE
   );
   _depthStencilTex = Core::gfx()->create2DTexture(
      _size.x, _size.y,
      Gfx::TEX_FMT_24_8, Gfx::TEX_CHANS_ZS, Gfx::TEX_FLAGS_RENDERABLE
   );
   _depthTex = Core::gfx()->create2DTexture(
      _size.x, _size.y,
      Gfx::TEX_FMT_16F_16F_16F_16F, Gfx::TEX_CHANS_RGBA, Gfx::TEX_FLAGS_RENDERABLE
      // Don't work on NVIDIA.
      //Gfx::TEX_FMT_16F_16F, Gfx::TEX_CHANS_RG, Gfx::TEX_FLAGS_RENDERABLE
      //Gfx::TEX_FMT_32F, Gfx::TEX_CHANS_R, Gfx::TEX_FLAGS_RENDERABLE
   );

   CHECK( _color1Tex.isValid() );
   CHECK( _color2Tex.isValid() );
   CHECK( _color3Tex.isValid() );
   CHECK( _depthStencilTex.isValid() );
   CHECK( _depthTex.isValid() );

   // Allocate buffers.
   _depthBuffer = Core::gfx()->createFramebuffer();
   _depthBuffer->setColorBuffer( _depthTex );
   _depthBuffer->setDepthBuffer( _depthStencilTex );

   _color1Buffer = Core::gfx()->createFramebuffer();
   _color1Buffer->setColorBuffer( _color1Tex );
   _color1Buffer->setDepthBuffer( _depthStencilTex );

   _color3Buffer = Core::gfx()->createFramebuffer();
   _color3Buffer->setColorBuffer( _color3Tex );
   _color3Buffer->setDepthBuffer( _depthStencilTex );

   // Allocate samplers.
   _distortionSamplers = new Gfx::SamplerList();
   _distortionSamplers->addSampler( "tex", _color2Tex, _clampNearest );

   _selectionSamplers = new Gfx::SamplerList();
   _selectionSamplers->addSampler( "tex", _color3Tex, _clampBilinear );

   _blitSamplers = new Gfx::SamplerList();
   _blitSamplers->addSampler( "tex", _color1Tex, _clampNearest );
   //_blitSamplers->addSampler( "tex", _color1Tex, _clampBilinear );

   // Allocate constants.
   Vec2f const1( 1.0f/size.x, 1.0f/size.y );
   _selection2Constants->setConstant( "texelSize", const1.ptr() );
   _distortionConstants->setConstant( "texelSize", const1.ptr() );

}

//------------------------------------------------------------------------------
//!
void
PlasmaRenderer::render(
   const RCP<Gfx::RenderNode>& rn,
   World*                      world,
   Viewport*                   vp
)
{
   // Is world ready to render?
   if( !world || world->state() != Resource::LOADED ) return;
   if( !vp || !vp->camera() ) return;

   _world      = world;
   _renderNode = getRenderNode();

   _projMat = vp->projectionMatrix();
   _viewMat = vp->viewMatrix();
   _camPos  = vp->camera()->position();

   // Set RenderNode.
   renderDepthPass( vp );
   renderColorPass( vp );
   renderSelectionPass( vp );
   renderDistortionPass( vp );

   const RCP<Gfx::Pass>& pass = rn->current();
   pass->render( _renderNode );
   blitResult( pass, vp );
}

//------------------------------------------------------------------------------
//!
void
PlasmaRenderer::renderDepthPass( Viewport* vp )
{
   Gfx::Pass* pass = getPass();

   _renderNode->addPass( pass );
   pass->setFramebuffer( _depthBuffer );
   pass->setClear( Gfx::CLEAR_ALL );
   pass->setClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
   pass->setAlphaState( _noBlending );
   //pass->setWireframe( true );
   //pass->setColorState( _noColor );

   // Set rendering context.
   pass->setProjectionMatrixPtr( _projMat.ptr() );
   pass->setViewMatrixPtr( _viewMat.ptr() );
   pass->setCamPositionPtr( _camPos.ptr() );

   // Rendering.
   renderWorldDepth( pass );
}

//------------------------------------------------------------------------------
//!
void
PlasmaRenderer::renderColorPass( Viewport* vp )
{
   // Prepare all of the particle buffers once per frame.
   //_world->particleManager()->prepareBuffers();

   // Batch lights by group of maximum 4.
   Vector< Vector<Light*> > lightsBatch;
   lightsBatch.resize( CGM::max( 1, CGM::ceili( _world->numLights() / 4.0f ) ) );
   for( uint i = 0; i < _world->numLights(); ++i )
   {
      lightsBatch[i/4].pushBack( _world->light(i) );
   }

   Gfx::Pass* pass = 0;

   for( uint i = 0; i < lightsBatch.size(); ++i )
   {
      // Render lights shadowmap.
      _lightRenderer->beginBatch();
      for( uint j = 0; j < lightsBatch[i].size(); ++j )
      {
         _lightRenderer->render( _renderNode, lightsBatch[i][j] );
      }
      _lightRenderer->endBatch();

      // Prepare color pass.
      const Vec4f& bgColor = _world->backgroundColor();
      pass = getPass();
      _renderNode->addPass( pass );
      pass->setClearColor( bgColor.x, bgColor.y, bgColor.z, bgColor.w );
      pass->setClear( i == 0 ? Gfx::CLEAR_COLOR : Gfx::CLEAR_NONE );
      pass->setFramebuffer( _color1Buffer );
      //pass->setWireframe( true );
      pass->setAlphaState( i == 0 ? _noBlending : _addBlending );
      pass->setDepthState( _depthEq );

      // Set rendering context.
      pass->setProjectionMatrixPtr( _projMat.ptr() );
      pass->setViewMatrixPtr( _viewMat.ptr() );
      pass->setCamPositionPtr( _camPos.ptr() );

      // Rendering.
      renderWorld( pass );
   }

#if FIXME
   // Particles.
   if( _world->particleManager().isValid() )
   {
      _world->particleManager()->render( *pass, vp->camera(), _depthTex );
   }

   // Renderable.
   if( _world->numRenderables() > 0 )
   {
      pass->setDepthState( _defDepth );
      pass->setAlphaState( _noBlending );

      // Render "renderable".
      World::RenderableContainer::ConstIterator it  = _world->renderables().begin();
      World::RenderableContainer::ConstIterator end = _world->renderables().end();

      for( ; it != end; ++it )
      {
         (*it)->render( pass );
      }
   }
#endif
   // Save color buffer for distortion pass.
   pass->copyColor( _color2Tex );
}

//------------------------------------------------------------------------------
//!
void
PlasmaRenderer::renderSelectionPass( Viewport* vp )
{
#if FIXME
   if( _world->numSelections() == 0 )
   {
      return;
   }

   Gfx::Pass* pass1 = getPass();
   pass1->setFramebuffer( _color3Buffer );
   pass1->setClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
   pass1->setClear( Gfx::CLEAR_COLOR );
   pass1->setAlphaState( _noBlending );
   pass1->setDepthState( _depthEq );
   pass1->setProjectionMatrixPtr( _projMatrix().ptr() );
   pass1->setViewMatrixPtr( _viewMat.ptr() );


   Gfx::Pass* pass2 = getPass();
   pass2->setFramebuffer( _color1Buffer );
   pass2->setClear( Gfx::CLEAR_NONE );
   pass2->setDepthState( _noDepth );
   pass2->setSamplers( _selectionSamplers );
   pass2->setProgram( _selectionProg );
   pass2->setConstants( Gfx::ConstantList::create( _selection2Constants ) );

   // Selection.
   World::SelectionContainer::ConstIterator sit  = _world->selections().begin();
   World::SelectionContainer::ConstIterator send = _world->selections().end();

   Vec2f vs = _size / 2.0f;
   Mat4f viewportMat(
      vs(0), 0.0f, 0.0f, vs(0),
      0.0f, vs(1), 0.0f, vs(1),
      0.0f,  0.0f, 0.5f,  0.5f,
      0.0f,  0.0f, 0.0f,  1.0f
   );
   Mat4f camMat = viewportMat * _projMat * _viewMat;

   AABBoxf box = AABBoxf::empty();
   bool hasSelection = false;
   for( ; sit != send; ++sit )
   {
      Entity* selEntity = (*sit)->entity().ptr();
      if( selEntity && ( (*sit)->camera() == vp->camera() || (*sit)->camera() == 0 ) )
      {
         // Rendering skeletal entity?
         if( selEntity->type() == Entity::SKELETAL )
         {
            // Retreive information for bone matrices.
            SkeletalEntity* skel           = (SkeletalEntity*)selEntity;
            RCP<Gfx::ConstantBuffer> bones = skel->boneConstants();
            RCP<Gfx::ConstantList> cl      = Gfx::ConstantList::create( bones );

            cl->addBuffer( _selection1Constants  );

            pass1->setConstants( cl );
            pass1->setProgram( _skelColorProg );
         }
         else
         {
            pass1->setConstants( Gfx::ConstantList::create( _selection1Constants ) );
            pass1->setProgram( _colorProg );
         }
         pass1->setWorldMatrixPtr( selEntity->transform().ptr() );

         Surface* surface  = selEntity->geometry()->surface();
         if( (*sit)->patches().empty() )
         {
            surface->render( *pass1 );
         }
         else
         {
            const Vector<uint>& patches = (*sit)->patches();
            for( uint pi = 0; pi < patches.size(); ++pi )
            {
               surface->renderPatch( *pass1, patches[pi] );
            }
         }

         // Compute screen bbox.
         hasSelection = true;
         const AABBoxf& selBox = surface->boundingBox();

         Mat4f mat = camMat * selEntity->transform();
         for( uint c = 0; c < 8;  ++c )
         {
            box |= mat | selBox.corner(c);
         }
      }
   }

   // Render selection edges into the color buffer.
   // We could use stencil for multiple selections.
   if( hasSelection )
   {
      _renderNode->addPass( pass1 );
      _renderNode->addPass( pass2 );

      box.grow( 5.0f );
      pass2->addScissor( (int)box.min(0), (int)box.min(1), (int)box.size(0), (int)box.size(1) );
      if( Core::gfx()->oneToOneOffset() != 0 )
      {
         Mat4f view = Mat4f::translate(-1.0f/_size.x, -1.0f/_size.y, 0.0f);
         pass2->setViewMatrix(view.ptr());
      }
      pass2->execGeometry( Core::gfx()->getOneToOneGeometry() );
   }
#endif
}

//------------------------------------------------------------------------------
//!
void
PlasmaRenderer::renderDistortionPass( Viewport* vp )
{
   Gfx::Pass* pass = getPass();

   _renderNode->addPass( pass );
   pass->setFramebuffer( _color1Buffer );
   pass->setClear( Gfx::CLEAR_NONE );

   //pass->copyColor( _distortionColorTex );

   // Render all of objects performing distortion.
   pass->setAlphaState( _noBlending );
   pass->setDepthState( _defDepth );

   // Set rendering context.
   pass->setProjectionMatrixPtr( _projMat.ptr() );
   pass->setViewMatrixPtr( _viewMat.ptr() );
   pass->setCamPositionPtr( _camPos.ptr() );

   // Refractive objects.
   renderWorldRefractive( pass );

   // Refractive particles.
   // FIXME: Send constant list and sampler list.
   //_world->particleManager()->render( *pass, vp->camera(), _depthTex, true );
}

//------------------------------------------------------------------------------
//!
void
PlasmaRenderer::blitResult( const RCP<Gfx::Pass>& pass, Viewport* vp )
{
   // Need to scale geometry from [-1, 1] range to [x, x+w] (or [y, y+h]).
   // We do:  [-1, 1] --(*0.5 + 0.5)--> [0, 1] --(*w + x)--> [x, w+x]
   // which simplfies to [-1, 1] --(*w/2 + w/2 + p)--> [x, w+x]
   const Vec2i& p  = vp->position();
   const Vec2i& s  = vp->size();
   float w_2       = s.x * 0.5f;
   float h_2       = s.y * 0.5f;
   float offx      = Core::gfx()->oneToOneOffset();
   float offy      = Core::gfx()->oneToOneOffset();
   Mat4f mat       = Mat4f::scale( w_2, h_2, 1.0f );
   mat.translate( Vec3f(w_2 + p.x + offx, h_2 + p.y + offy, 0.0f) );

   pass->setAlphaState( _noBlending );
   pass->setSamplers( _blitSamplers );
   pass->setProgram( _blitProg );
   pass->setWorldMatrix( mat.ptr() );
   const int*  savedScissor = pass->setScissor( p.x, p.y, s.x, s.y );
   pass->execGeometry( Core::gfx()->getOneToOneGeometry() );
   // Restore values in opposite order.
   pass->setScissor( savedScissor );
   pass->setProgram( Core::defaultProgram() );
   pass->setAlphaState( _defBlending );
}

//------------------------------------------------------------------------------
//!
void
PlasmaRenderer::renderWorld( Gfx::Pass* pass )
{
   // Render each entity.
   for( uint i = 0; i < _world->numEntities(); ++i )
   {
      Entity* e = _world->entity(i);

      // Temporary handling of MetaObjects.
      MetaGeometry* mgeom = e->geometry()->metaGeometry();

      if( mgeom )
      {
         pass->setWorldMatrixPtr( e->transform().ptr() );
         const Material* material = e->material( 0 ).ptr();
         // Prepare lists.
         if( _numLists >= _constants.size() )
         {
            _constants.pushBack( new Gfx::ConstantList() );
            _samplers.pushBack( new Gfx::SamplerList() );
         }
         _constants[_numLists]->clear();
         _samplers[_numLists]->clear();

         _lightRenderer->setLists( _constants[_numLists], _samplers[_numLists] );
         material->apply( *pass, _constants[_numLists], _samplers[_numLists] );
         mgeom->render( *pass );

         ++_numLists;
      }

      Surface* surface = e->geometry()->surface();
      if( surface == NULL )  continue;

      // Render each surface.
      Surface::PatchContainer::ConstIterator patchIt  = surface->patches().begin();
      Surface::PatchContainer::ConstIterator patchEnd = surface->patches().end();

      pass->setWorldMatrixPtr( e->transform().ptr() );

      // Rendering skeletal entity?
      if( e->type() == Entity::SKELETAL )
      {
         // Retreive information for bone matrices.
         SkeletalEntity* skel           = (SkeletalEntity*)e;
         RCP<Gfx::ConstantBuffer> bones = skel->boneConstants();

         for( uint j = 0; patchIt != patchEnd; ++patchIt, ++j )
         {
            const Material* material = e->material( (*patchIt)->materialID() ).ptr();
            if( material->refractive() ) continue; // Separate pass.

            // Prepare lists.
            if( _numLists >= _constants.size() )
            {
               _constants.pushBack( new Gfx::ConstantList() );
               _samplers.pushBack( new Gfx::SamplerList() );
            }
            _constants[_numLists]->clear();
            _samplers[_numLists]->clear();

            _constants[_numLists]->addBuffer( bones );
            _lightRenderer->setLists( _constants[_numLists], _samplers[_numLists] );
            material->apply( *pass, _constants[_numLists], _samplers[_numLists] );
            surface->renderPatch( *pass, j );

            ++_numLists;
         }
      }
      else
      {
         for( uint j = 0; patchIt != patchEnd; ++patchIt, ++j )
         {
            const Material* material = e->material( (*patchIt)->materialID() ).ptr();
            if( material->refractive() ) continue; // Separate pass.

            // Prepare lists.
            if( _numLists >= _constants.size() )
            {
               _constants.pushBack( new Gfx::ConstantList() );
               _samplers.pushBack( new Gfx::SamplerList() );
            }
            _constants[_numLists]->clear();
            _samplers[_numLists]->clear();

            _lightRenderer->setLists( _constants[_numLists], _samplers[_numLists] );
            material->apply( *pass, _constants[_numLists], _samplers[_numLists] );
            surface->renderPatch( *pass, j );

            ++_numLists;
         }
      }
   }
}

//------------------------------------------------------------------------------
//!
void
PlasmaRenderer::renderWorldShadow( Gfx::Pass* pass )
{
   // Render each entity.
   int config = 0;

   for( uint i = 0; i < _world->numEntities(); ++i )
   {
      Entity* e = _world->entity(i);
      Surface* surface = e->geometry()->surface();
      if( surface == NULL || !e->castShadow() )  continue;

      pass->setWorldMatrixPtr( e->transform().ptr() );

      // Rendering skeletal entity?
      if( e->type() == Entity::SKELETAL )
      {
         // Retreive information for bone matrices.
         SkeletalEntity* skel           = (SkeletalEntity*)e;
         RCP<Gfx::ConstantBuffer> bones = skel->boneConstants();
         RCP<Gfx::ConstantList> cl      = Gfx::ConstantList::create( bones );

         pass->setConstants( cl );
         if( config != 1 )
         {
            config = 1;
            pass->setSamplers(0);
            pass->setProgram( _skelShadowProg );
         }
      }
      else
      {
         if( config != 2 )
         {
            config = 2;
            pass->setSamplers(0);
            pass->setConstants(0);
            pass->setProgram( _shadowProg );
         }
      }

      // Render each patch.
      Surface::PatchContainer::ConstIterator patchIt  = surface->patches().begin();
      Surface::PatchContainer::ConstIterator patchEnd = surface->patches().end();
      for( uint j = 0; patchIt != patchEnd; ++patchIt, ++j )
      {
         const Material* material = e->material( (*patchIt)->materialID() ).ptr();
         if( material->refractive() ) continue; // Separate pass.
         // Do not set the material.
         surface->renderPatch( *pass, j );
      }
   }
}

//------------------------------------------------------------------------------
//!
void
PlasmaRenderer::renderWorldDepth( Gfx::Pass* pass )
{
   // Render each entity.
   int config = 0;

   for( uint i = 0; i < _world->numEntities(); ++i )
   {
      Entity* e = _world->entity(i);
      pass->setWorldMatrixPtr( e->transform().ptr() );

      // Rendering skeletal entity?
      if( e->type() == Entity::SKELETAL )
      {
         // Retreive information for bone matrices.
         SkeletalEntity* skel           = (SkeletalEntity*)e;
         RCP<Gfx::ConstantBuffer> bones = skel->boneConstants();
         RCP<Gfx::ConstantList> cl      = Gfx::ConstantList::create( bones );

         pass->setConstants( cl );
         if( config != 1 )
         {
            config = 1;
            pass->setSamplers(0);
            pass->setProgram( _skelDepthProg );
         }
      }
      else
      {
         if( config != 2 )
         {
            config = 2;
            pass->setSamplers(0);
            pass->setConstants(0);
            pass->setProgram( _depthProg );
         }
      }

      MetaGeometry* mgeom = e->geometry()->metaGeometry();
      if( mgeom ) mgeom->render( *pass );

      Surface* surface = e->geometry()->surface();
      if( surface == NULL )  continue;

      // Render each patch.
      Surface::PatchContainer::ConstIterator patchIt  = surface->patches().begin();
      Surface::PatchContainer::ConstIterator patchEnd = surface->patches().end();
      for( uint j = 0; patchIt != patchEnd; ++patchIt, ++j )
      {
         const Material* material = e->material( (*patchIt)->materialID() ).ptr();
         if( material->refractive() ) continue; // Separate pass.
         // Do not set the material.
         surface->renderPatch( *pass, j );
      }
   }
}

//------------------------------------------------------------------------------
//!
void
PlasmaRenderer::renderWorldRefractive( Gfx::Pass* pass )
{
   // Render each entity.
   for( uint i = 0; i < _world->numEntities(); ++i )
   {
      Entity* e = _world->entity(i);
      Surface* surface = e->geometry()->surface();
      if( surface == NULL )  continue;

      // Render each surface.
      Surface::PatchContainer::ConstIterator patchIt  = surface->patches().begin();
      Surface::PatchContainer::ConstIterator patchEnd = surface->patches().end();

      pass->setWorldMatrixPtr( e->transform().ptr() );

      // Rendering skeletal entity?
      if( e->type() == Entity::SKELETAL )
      {
         // Retreive information for bone matrices.
         SkeletalEntity* skel           = (SkeletalEntity*)e;
         RCP<Gfx::ConstantBuffer> bones = skel->boneConstants();

         for( uint j = 0; patchIt != patchEnd; ++patchIt, ++j )
         {
            const Material* material = e->material( (*patchIt)->materialID() ).ptr();
            if( !material->refractive() ) continue;

            // Prepare lists.
            if( _numLists >= _constants.size() )
            {
               _constants.pushBack( new Gfx::ConstantList() );
               _samplers.pushBack( new Gfx::SamplerList() );
            }

            const RCP<Gfx::ConstantList>& constants = _constants[_numLists];
            constants->clear();
            constants->addBuffer( bones );
            constants->addBuffer( _distortionConstants );

            const RCP<Gfx::SamplerList>& samplers = _samplers[_numLists];
            samplers->clear();
            samplers->addSamplers( _distortionSamplers );

            material->apply( *pass, constants, samplers );

            surface->renderPatch( *pass, j );

            ++_numLists;
         }
      }
      else
      {
         for( uint j = 0; patchIt != patchEnd; ++patchIt, ++j )
         {
            const Material* material = e->material( (*patchIt)->materialID() ).ptr();
            if( !material->refractive() ) continue;

            // Prepare lists.
            if( _numLists >= _constants.size() )
            {
               _constants.pushBack( new Gfx::ConstantList() );
               _samplers.pushBack( new Gfx::SamplerList() );
            }

            const RCP<Gfx::ConstantList>& constants = _constants[_numLists];
            constants->clear();
            constants->addBuffer( _distortionConstants );

            const RCP<Gfx::SamplerList>& samplers = _samplers[_numLists];
            samplers->clear();
            samplers->addSamplers( _distortionSamplers );

            material->apply( *pass, constants, samplers );

            surface->renderPatch( *pass, j );

            ++_numLists;
         }
      }
   }
}

//------------------------------------------------------------------------------
//!
void
PlasmaRenderer::beginFrame()
{
   _numPasses      = 0;
   _numRenderNodes = 0;
   _numLists       = 0;

   _lightRenderer->beginFrame();
}

//------------------------------------------------------------------------------
//!
void
PlasmaRenderer::endFrame()
{
   _lightRenderer->endFrame();
}

//------------------------------------------------------------------------------
//!
Gfx::Pass*
PlasmaRenderer::getPass()
{
   if( _numPasses >= _passes.size() )
   {
      _passes.pushBack( new Gfx::Pass() );
   }
   _passes[_numPasses]->clear();
   return _passes[_numPasses++].ptr();
}

//------------------------------------------------------------------------------
//!
Gfx::RenderNode*
PlasmaRenderer::getRenderNode()
{
   if( _numRenderNodes >= _renderNodes.size() )
   {
      _renderNodes.pushBack( new Gfx::RenderNode() );
   }
   _renderNodes[_numRenderNodes]->clear();
   return _renderNodes[_numRenderNodes++].ptr();
}

NAMESPACE_END
