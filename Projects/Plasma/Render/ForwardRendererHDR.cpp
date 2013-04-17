/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Render/ForwardRendererHDR.h>

#include <Plasma/Plasma.h>
#include <Plasma/Geometry/ParticleGeometry.h>
#include <Plasma/Render/DebugGeometry.h>
#include <Plasma/Renderable/Renderable.h>
#include <Plasma/World/World.h>
#include <Plasma/World/Viewport.h>
#include <Plasma/World/ParticleEntity.h>
#include <Plasma/World/SkeletalEntity.h>

#include <Fusion/Core/Core.h>
#include <Fusion/Resource/ResManager.h>
#include <Fusion/Resource/Image.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

Gfx::TextureState clampBilinear;
Gfx::TextureState clampNearest;


UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
  CLASS ForwardRendererHDR
==============================================================================*/

//------------------------------------------------------------------------------
//!
ForwardRendererHDR::ForwardRendererHDR()
{
   // States initialization.
   clampBilinear.setBilinear();
   clampBilinear.clamp( Gfx::TEX_CLAMP_LAST );

   clampNearest.setPointSampling();
   clampNearest.clamp( Gfx::TEX_CLAMP_LAST );

   // Allocating states.
   _defBlending = new Gfx::AlphaState();
   _defBlending->alphaTesting( true );
   _defBlending->alphaBlending( true );
   _defBlending->setAlphaBlend( Gfx::ALPHA_BLEND_ONE, Gfx::ALPHA_BLEND_ONE_MINUS_SRC_ALPHA );

   _noBlending = new Gfx::AlphaState();
   _noBlending->alphaBlending( false );
   _noBlending->alphaTesting( false );

   _noDepth = new Gfx::DepthState();
   _noDepth->depthTesting( false );
   _noDepth->depthWriting( false );

   _noDepthWrite = new Gfx::DepthState();
   _noDepthWrite->depthTesting( true );
   _noDepthWrite->depthWriting( false );

   _defOffset = new Gfx::OffsetState();
   _offset    = new Gfx::OffsetState();
   _offset->constant( -2.0f );
   _offset->factor( -0.0f );

   Image* img = data( ResManager::getImage( "image/dbrdf" ) );
   _dbrdf = new Gfx::Sampler( "dbrdfTex", img->texture(), clampBilinear );

   // Allocating programs.
   _blitProg         = data( ResManager::getProgram( "shader/program/f_passthrough" ) );
   _colorDirProg     = data( ResManager::getProgram( "shader/program/forward/color1_dir" ) );
   _wireDirProg      = data( ResManager::getProgram( "shader/program/forward/color1_dir_wired" ) );
   _colorDirSkelProg = data( ResManager::getProgram( "shader/program/forward/color1_dir_skel" ) );
   _wireDirSkelProg  = data( ResManager::getProgram( "shader/program/forward/color1_dir_skel_wired" ) );
   _colorDirPartProg = data( ResManager::getProgram( "shader/program/particle/psc" ) );

   // Default material set.
   _defMatSet = new MaterialSet();
   _defMatSet->add( Material::white() );
}

//------------------------------------------------------------------------------
//!
ForwardRendererHDR::~ForwardRendererHDR()
{
}

//------------------------------------------------------------------------------
//!
void
ForwardRendererHDR::render(
   const RCP<Gfx::RenderNode>& rn,
   World*                      world,
   Viewport*                   vp
)
{
   // Is world ready to render?
   if( !world ) return;

   // Do we have a valid viewport?
   if( !vp || !vp->camera() ) return;

   // Set internal members.
   _world = world;
   _renderNode = getRenderNode();

   _projMat = vp->projectionMatrix();
   _viewMat = vp->viewMatrix();
   _camPos  = vp->camera()->position();

   // Classify entities for rendering. We are looking for visible, ghost and
   // reflection.
   classifyEntities( _world );

   // Rendering.
   Gfx::Pass* pass = getPass();
   _renderNode->addPass( pass );
   renderColorPass( *pass, *vp );

   // Blitting results.
   pass = rn->current().ptr();
   pass->render( _renderNode );
   blitResult( *pass, *vp );
}

//------------------------------------------------------------------------------
//!
void
ForwardRendererHDR::performResize()
{
   // Allocate renderable texture.
   _color1Tex = Core::gfx()->create2DTexture(
      _size.x, _size.y,
      Gfx::TEX_FMT_16F_16F_16F_16F, Gfx::TEX_CHANS_RGBA, Gfx::TEX_FLAGS_RENDERABLE
   );
   _depthStencilTex = Core::gfx()->create2DTexture(
      _size.x, _size.y,
      Gfx::TEX_FMT_24_8, Gfx::TEX_CHANS_ZS, Gfx::TEX_FLAGS_RENDERABLE
   );

   // Allocate buffers.
   _color1Buffer = Core::gfx()->createFramebuffer();
   _color1Buffer->setColorBuffer( _color1Tex );
   _color1Buffer->setDepthBuffer( _depthStencilTex );

   _blitSamplers = new Gfx::SamplerList();
   _blitSamplers->addSampler( "tex", _color1Tex, clampNearest );
   //_blitSamplers->addSampler( "tex", _color1Tex, clampBilinear );
}

//------------------------------------------------------------------------------
//!
void
ForwardRendererHDR::performBeginFrame()
{
}

//------------------------------------------------------------------------------
//!
void
ForwardRendererHDR::performEndFrame()
{
}

//------------------------------------------------------------------------------
//!
void
ForwardRendererHDR::renderColorPass( Gfx::Pass& pass, const Viewport& vp )
{
   // Setting buffer.
   const Vec4f& bgColor = _world->backgroundColor();
   pass.setClearColor( bgColor.x, bgColor.y, bgColor.z, bgColor.w );
   pass.setClear( Gfx::CLEAR_ALL );
   pass.setFramebuffer( _color1Buffer );

   // Setting default states.
   pass.setAlphaState( _noBlending );

   // Setting matrices and positions.
   pass.setProjectionMatrixPtr( _projMat.ptr() );
   pass.setViewMatrixPtr( _viewMat.ptr() );
   pass.setCamPositionPtr( _camPos.ptr() );

   // Setting program.
   pass.setConstants(0);

   renderMain( pass );
   renderRenderables( pass, vp );
}

//------------------------------------------------------------------------------
//!
void
ForwardRendererHDR::renderMain( Gfx::Pass& pass )
{
   RCP<Gfx::Program> colorDirProg     = Plasma::showWireframe() ? _wireDirProg : _colorDirProg;
   RCP<Gfx::Program> colorDirSkelProg = Plasma::showWireframe() ? _wireDirSkelProg : _colorDirSkelProg;

   // Render entities.
   for( auto cur = _visibles.begin(); cur != _visibles.end(); ++cur )
   {
      Entity* e      = cur->_entity;
      Geometry* geom = cur->_geom;
      Material* mat  = cur->_mat;

      // Set position.
      pass.setWorldMatrixPtr( (const float*)cur->_trf );

      switch( cur->_type )
      {
         case STANDARD:
            pass.setProgram( colorDirProg );
            pass.setSamplers( mat->samplers() );
            pass.setConstants( mat->constants() );
            break;
         case CUSTOM:
            pass.setProgram( ((CustomMaterial*)mat)->program() );
            pass.setSamplers( mat->samplers() );
            pass.setConstants( mat->constants() );
            break;
         case STANDARD_SKINNED:
         {
            SkeletalEntity* se           = (SkeletalEntity*)e;
            Gfx::ConstantList* constants = getConstantList();
            constants->addBuffer( (*mat->constants())[0] );
            constants->addBuffer( se->boneConstants() );

            pass.setProgram( colorDirSkelProg );
            pass.setConstants( constants );
            pass.setSamplers( mat->samplers() );
         }  break;
         case CUSTOM_SKINNED:
         {
            SkeletalEntity* se           = (SkeletalEntity*)e;
            Gfx::ConstantList* constants = getConstantList();
            constants->addBuffer( (*mat->constants())[0] );
            constants->addBuffer( se->boneConstants() );

            pass.setProgram( ((CustomMaterial*)mat)->program() );
            pass.setConstants( constants );
            pass.setSamplers( mat->samplers() );
         }  break;
         default:;
      }
      // Geometry.
      geom->renderPatch( pass, cur->_patchId );
   }

   renderParticles( pass );
}

//------------------------------------------------------------------------------
//!
void
ForwardRendererHDR::renderParticles( Gfx::Pass& pass )
{
   pass.setAlphaState( _defBlending );
   pass.setDepthState( _noDepthWrite );

   // Render particles.
   for( auto cur = _particles.begin(); cur != _particles.end(); ++cur )
   {
      CHECK( cur->_entity->type() == Entity::PARTICLE );
      CHECK( cur->_geom->type() == Geometry::PARTICLE );
      ParticleEntity*      e = (ParticleEntity*)cur->_entity;
      ParticleGeometry* geom = (ParticleGeometry*)cur->_geom;
      Material*          mat = cur->_mat;

      geom->guaranteeRenderableGeometry(); // Needs to exist before updateRenderData() is called.
      geom->updateRenderData();

      // Set position.
      const float* xform = e->isRelative() ? (const float*)cur->_trf : Gfx::Pass::identityMatrix();
      pass.setWorldMatrixPtr( xform );

      switch( cur->_type )
      {
         case STANDARD:
            pass.setProgram( _colorDirPartProg );
            pass.setSamplers( mat->samplers() );
            pass.setConstants( mat->constants() );
            break;
         case CUSTOM:
            pass.setProgram( ((CustomMaterial*)mat)->program() );
            pass.setSamplers( mat->samplers() );
            pass.setConstants( mat->constants() );
            break;
         default:;
      }

      // Geometry.
      geom->renderPatch( pass, cur->_patchId );
   }
}

//------------------------------------------------------------------------------
//!
void
ForwardRendererHDR::renderRenderables( Gfx::Pass& pass, const Viewport& vp )
{
   uint numRenderables = _world->numRenderables();
   for( uint i = 0; i < numRenderables; ++i )
   {
      _world->renderable(i)->render( pass, vp );
   }

   if( _world->debugGeometry().isValid() )
   {
      pass.setWorldMatrix( Mat4f::identity().ptr() );
      pass.setSamplers(0);
      pass.setConstants(0);
      pass.setDepthState( _noDepth );
      _world->debugGeometry()->render( pass );
   }
}

//------------------------------------------------------------------------------
//!
void
ForwardRendererHDR::blitResult( Gfx::Pass& pass, Viewport& vp )
{
   // Need to scale geometry from [-1, 1] range to [x, x+w] (or [y, y+h]).
   // We do:  [-1, 1] --(*0.5 + 0.5)--> [0, 1] --(*w + x)--> [x, w+x]
   // which simplfies to [-1, 1] --(*w/2 + w/2 + p)--> [x, w+x]
   const Vec2i& p  = vp.position();
   const Vec2i& s  = vp.size();
   float w_2       = s.x * 0.5f;
   float h_2       = s.y * 0.5f;
   float offx      = Core::gfx()->oneToOneOffset();
   float offy      = Core::gfx()->oneToOneOffset();
   Mat4f mat       = Mat4f::scaling( w_2, h_2, 1.0f );
   mat.translate( Vec3f(w_2 + p.x + offx, h_2 + p.y + offy, 0.0f) );
   pass.setAlphaState( _noBlending );
   pass.setSamplers( _blitSamplers );
   pass.setProgram( _blitProg );
   pass.setWorldMatrix( mat.ptr() );
   const int*  savedScissor = pass.setScissor( p.x, p.y, s.x, s.y );
   pass.execGeometry( Core::gfx()->getOneToOneGeometry() );
   // Restore values in opposite order.
   pass.setScissor( savedScissor );
   pass.setProgram( Core::defaultProgram() );
   pass.setAlphaState( _defBlending );
}

NAMESPACE_END
