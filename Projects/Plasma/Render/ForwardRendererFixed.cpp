/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Render/ForwardRendererFixed.h>
#include <Plasma/World/World.h>
#include <Plasma/World/Viewport.h>
#include <Plasma/World/Selection.h>

#include <Fusion/Core/Core.h>
#include <Fusion/Resource/ResManager.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN


UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
  CLASS ForwardRendererFixed
==============================================================================*/

//------------------------------------------------------------------------------
//!
ForwardRendererFixed::ForwardRendererFixed()
{
   // Allocating states.
   _defBlending = new Gfx::AlphaState();
   //_defBlending->alphaTesting( true );
   _defBlending->alphaTesting( false );
   _defBlending->alphaBlending( true );
   _defBlending->setAlphaBlend( Gfx::ALPHA_BLEND_ONE, Gfx::ALPHA_BLEND_ONE_MINUS_SRC_ALPHA );

   _noBlending = new Gfx::AlphaState();
   _noBlending->alphaBlending( false );
   _noBlending->alphaTesting( false );

   _frontCulling = new Gfx::CullState();
   _frontCulling->side( Gfx::FACE_FRONT );
   _backCulling  = new Gfx::CullState();
   _backCulling->side( Gfx::FACE_BACK );


   _defColor = new Gfx::ColorState();
   _noColor  = new Gfx::ColorState();
   _noColor->colorWriting( false );

   _defDepth = new Gfx::DepthState();
   _eqDepth  = new Gfx::DepthState();
   _eqDepth->depthTesting( true );
   _eqDepth->depthWriting( false );
   _eqDepth->depthTestFunc( Gfx::COMPARE_FUNC_EQUAL );

   _defOffset = new Gfx::OffsetState();
   _offset    = new Gfx::OffsetState();
   _offset->constant( 10.0f );

   _clampBilinear.setBilinear();
   _clampBilinear.clamp( Gfx::TEX_CLAMP_LAST );

   _clampNearest.setPointSampling();
   _clampNearest.clamp( Gfx::TEX_CLAMP_LAST );


   // Allocating programs.
   _colorLayerProg = data( ResManager::getProgram( "shader/program/forwardColorFixed" ) );
   _selectionProg  = data( ResManager::getProgram( "shader/program/colorFixed" ) );

   // Default material set.
   _defMatSet = new MaterialSet();
   _defMatSet->add( Material::white() );
}

//------------------------------------------------------------------------------
//!
ForwardRendererFixed::~ForwardRendererFixed()
{
}

//------------------------------------------------------------------------------
//!
void
ForwardRendererFixed::render(
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

   // Rendering.
   RCP<Gfx::Pass> pass = rn->current();
   renderColorPass( pass.ptr(), vp );

   // TODO: Add support for multiple viewports.
}

//------------------------------------------------------------------------------
//!
void
ForwardRendererFixed::performResize()
{
   // TODO: Allocate buffers if viewport isn't full screen.
}

//------------------------------------------------------------------------------
//!
void
ForwardRendererFixed::performBeginFrame()
{
}

//------------------------------------------------------------------------------
//!
void
ForwardRendererFixed::performEndFrame()
{
}

//------------------------------------------------------------------------------
//!
void
ForwardRendererFixed::renderColorPass( Gfx::Pass* pass, Viewport* vp )
{
   // Classify entities for rendering. We are looking for visible, ghost and
   // reflection.
   classifyEntities();

   // Setting buffer.
   const Vec4f& bgColor = _world->backgroundColor();
   pass->setClearColor( bgColor.x, bgColor.y, bgColor.z, bgColor.w );
   pass->setClear( Gfx::CLEAR_ALL );

   // Setting default states.
   pass->setAlphaState( _noBlending );

   _viewMat = vp->viewMatrix();
   _projMat = vp->projectionMatrix();

   const float* matProj = pass->setProjectionMatrixPtr( _projMat.ptr() );
   const float* matView = pass->setViewMatrixPtr( _viewMat.ptr() );
   const float* posCam  = pass->setCamPositionPtr( vp->camera()->position().ptr() );
   const int* viewp     = pass->setViewport( (int)vp->position().x, (int)vp->position().y, (int)vp->size().x, (int)vp->size().y );
   const int* scissor   = pass->setScissor( (int)vp->position().x, (int)vp->position().y, (int)vp->size().x, (int)vp->size().y );

   // Setting program.
   pass->setSamplers(0);
   pass->setConstants(0);
   pass->setProgram( _colorLayerProg );

   // Rendering passes.
   renderMirrors( pass );
   renderMain( pass );
   renderGhosts( pass );
   renderSelections( pass, vp );

   // Setting matrices and positions.
   pass->setProjectionMatrixPtr( matProj );
   pass->setViewMatrixPtr( matView );
   pass->setCamPositionPtr( posCam );
   pass->setViewportPtr( viewp );
   pass->setScissorPtr( scissor );

   pass->setProgram( Core::defaultProgram() );
   pass->setAlphaState( _defBlending );
}

//------------------------------------------------------------------------------
//!
void
ForwardRendererFixed::classifyEntities()
{
   _reflectionGeoms.clear();
   _visibles.clear();
   _ghosts.clear();

   uint numEntities = _world->numEntities();
   for( uint i = 0; i < numEntities; ++i )
   {
      Entity* e      = _world->entity(i);
      Geometry* geom = e->geometry();

      // Do we have a geometry to render?
      if( !geom || !e->visible() ) continue;

      if( e->ghost() )
         _ghosts.pushBack( e );
      else
         _visibles.pushBack( e );

      MaterialSet* matSet = e->materialSet();
      if( !matSet ) continue;

      for( uint p = 0; p < geom->numPatches(); ++p )
      {
         Material* mat = geom->material( matSet, p );
         if( mat->type() == Material::REFLECTIVE_PLANAR )
            _reflectionGeoms.pushBack( ReflGeom( e, mat, p ) );
      }
   }
}

//------------------------------------------------------------------------------
//!
void
ForwardRendererFixed::renderMirrors( Gfx::Pass* pass )
{
   // FIXME:
   //  stenciling.

   // Render reflections.
   if( !_reflectionGeoms.empty() )
   {
      pass->setCullState( _frontCulling );
      for( uint r = 0; r < _reflectionGeoms.size(); ++r )
      {
         ReflectivePlanarMaterial* rmaterial = (ReflectivePlanarMaterial*)_reflectionGeoms[r]._mat;
         Entity* re = _reflectionGeoms[r]._entity;
         Mat4f rmat = Mat4f::reflection( re->transform() * rmaterial->normal(), re->transform() * rmaterial->position() );
         pass->setViewMatrix( (_viewMat*rmat).ptr() );

         // Render entities.
         for( uint i = 0; i < _visibles.size(); ++i )
         {
            Entity* e      = _visibles[i];
            Geometry* geom = e->geometry();

            MaterialSet* matSet = e->materialSet();
            if( !matSet ) matSet = _defMatSet.ptr();

            // Set position.
            pass->setWorldMatrixPtr( e->transform().ptr() );

            //pass->setProgram( _colorLayerProg );
            for( uint p = 0; p < geom->numPatches(); ++p )
            {
               Material* mat = geom->material( matSet, p );
               if( mat == rmaterial ) continue;
               pass->setSamplers( mat->samplers() );
               pass->setConstants( mat->constants() );
               geom->renderPatch( *pass, p );
            }
         }
      }
      pass->setViewMatrixPtr( _viewMat.ptr() );
      pass->setCullState( _backCulling );
      pass->setAlphaState( _defBlending );
      // Render reflectors.
      for( uint r = 0; r < _reflectionGeoms.size(); ++r )
      {
         Entity* e       = _reflectionGeoms[r]._entity;
         Geometry* geom  = e->geometry();
         Material* mat   = _reflectionGeoms[r]._mat;

         // Set position.
         pass->setWorldMatrixPtr( e->transform().ptr() );

         pass->setSamplers( mat->samplers() );
         pass->setConstants( mat->constants() );
         geom->renderPatch( *pass, _reflectionGeoms[r]._patchId );
      }
      pass->setAlphaState( _noBlending );
   }
}

//------------------------------------------------------------------------------
//!
void
ForwardRendererFixed::renderMain( Gfx::Pass* pass )
{
   // Render entities.
   for( uint i = 0; i < _visibles.size(); ++i )
   {
      Entity* e      = _visibles[i];
      Geometry* geom = e->geometry();

      MaterialSet* matSet = e->materialSet();
      if( !matSet ) matSet = _defMatSet.ptr();

      // Set position.
      pass->setWorldMatrixPtr( e->transform().ptr() );

      for( uint p = 0; p < geom->numPatches(); ++p )
      {
         Material* mat = geom->material( matSet, p );
         if( mat->type() == Material::REFLECTIVE_PLANAR ) continue;
         pass->setSamplers( mat->samplers() );
         pass->setConstants( mat->constants() );
         geom->renderPatch( *pass, p );
      }
   }
}

//------------------------------------------------------------------------------
//!
void
ForwardRendererFixed::renderGhosts( Gfx::Pass* pass )
{
   if( !_ghosts.empty() )
   {
      // Render depth.
      pass->setColorState( _noColor );
      pass->setOffsetState( _offset );
      for( uint i = 0; i < _ghosts.size(); ++i )
      {
         Entity* e = _ghosts[i];
         pass->setWorldMatrixPtr( e->transform().ptr() );
         e->geometry()->render( *pass );
      }
      // Render material.
      pass->setColorState( _defColor );
      pass->setAlphaState( _defBlending );
      pass->setDepthState( _eqDepth );
      for( uint i = 0; i < _ghosts.size(); ++i )
      {
         Entity* e      = _ghosts[i];
         Geometry* geom = e->geometry();
         pass->setWorldMatrixPtr( e->transform().ptr() );
         MaterialSet* matSet = e->materialSet();
         for( uint p = 0; p < geom->numPatches(); ++p )
         {
            Material* mat = geom->material( matSet, p );
            pass->setSamplers( mat->samplers() );
            pass->setConstants( mat->constants() );
            geom->renderPatch( *pass, p );
         }
      }
      pass->setDepthState( _defDepth );
      pass->setOffsetState( _defOffset );
   }
}

//------------------------------------------------------------------------------
//!
void
ForwardRendererFixed::renderSelections( Gfx::Pass* pass, Viewport* vp )
{
   // Render selection.
   if( _world->numSelections() > 0 )
   {
      Vec3f camPos = vp->camera()->position();
      // Set selection rendering states.
      pass->setCullState( _frontCulling );
      pass->setProgram( _selectionProg );
      pass->setSamplers(0);
      pass->setConstants(0);

      for( uint i = 0; i < _world->numSelections(); ++i )
      {
         Entity* e      = _world->selection(i)->entity();
         Geometry* geom = e->geometry();

         // Do we have a geometry to render?
         if( !geom ) continue;

         // Set position.
         //pass->setWorldMatrixPtr( e->transform().ptr() );
         Mat4f tm = Mat4f::translation( normalize(camPos-e->position())*0.1f );
         pass->setWorldMatrix( (tm * e->transform()).ptr() );

         // Rendering geometry.
         geom->render( *pass );
      }
      pass->setCullState( _backCulling );
   }
}

NAMESPACE_END
