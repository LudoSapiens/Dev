/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Render/Renderer.h>
#include <Plasma/World/World.h>
#include <Plasma/World/Entity.h>
#include <Plasma/World/ProxyEntity.h>
#include <Plasma/Geometry/Geometry.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN


UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Renderer
==============================================================================*/

//------------------------------------------------------------------------------
//!
Renderer::Renderer() :
   _clearDepth( false ),
   _size(0), _numPasses(0), _numRenderNodes(0), _numSamplerLists(0)
{
   // Default material set.
   _defMatSet = new MaterialSet();
   _defMatSet->add( Material::white() );
}

//------------------------------------------------------------------------------
//!
Renderer::~Renderer()
{
}

//------------------------------------------------------------------------------
//!
void
Renderer::size( const Vec2i& size )
{
   if( size == _size ) return;
   _size = CGM::clamp( size, 0, 8192 );
   performResize();
}

//------------------------------------------------------------------------------
//!
void
Renderer::render(
   const RCP<Gfx::RenderNode>& /*rn*/,
   World*                      /*world*/,
   Viewport*                   /*vp*/
)
{
}

//------------------------------------------------------------------------------
//!
void
Renderer::beginFrame()
{
   _numPasses        = 0;
   _numRenderNodes   = 0;
   _numSamplerLists  = 0;
   _numConstantLists = 0;
   performBeginFrame();
}

//------------------------------------------------------------------------------
//!
void
Renderer::endFrame()
{
   performEndFrame();
}

//------------------------------------------------------------------------------
//!
void
Renderer::performResize()
{
}

//------------------------------------------------------------------------------
//!
void
Renderer::performBeginFrame()
{
}

//------------------------------------------------------------------------------
//!
void
Renderer::performEndFrame()
{
}

//------------------------------------------------------------------------------
//!
void Renderer::classifyEntities( World* world )
{
   _reflections.clear();
   _visibles.clear();
   _ghosts.clear();
   _particles.clear();

   uint numEntities = world->numEntities();
   for( uint i = 0; i < numEntities; ++i )
   {
      Entity* e      = world->entity(i);
      Geometry* geom = e->geometry();

      if( e->type() != Entity::PROXY )
      {
         // Do we have a geometry to render?
         if( !geom || !e->visible() ) continue;

         MaterialSet* matSet = e->materialSet();
         if( !matSet ) matSet = _defMatSet.ptr();

         // Do we have a skeletal entity?
         int hasSkin = e->type() == Entity::SKELETAL ? HAS_SKIN : 0;

         // Transform.
         const Mat4f* t = &e->transform();

         // Create a renderable chunk for each patch.
         for( uint p = 0; p < geom->numPatches(); ++p )
         {
            Material* mat = geom->material( matSet, p );

            if( e->ghost() )
            {
               _ghosts.pushBack( Chunk( GHOST | hasSkin, e, t, geom, mat, p ) );
               continue;
            }
            else
            if( e->type() == Entity::PARTICLE )
            {

               switch( mat->type() )
               {
                  case Material::BASE:
                     _particles.pushBack( Chunk( STANDARD, e, t, geom, mat, p ) );
                     break;
                  case Material::CUSTOM:
                     _particles.pushBack( Chunk( CUSTOM, e, t, geom, mat, p ) );
                     break;
                  default:
                     // Unknown material type.
                     CHECK( false );
                     break;
               }
               continue;
            }

            switch( mat->type() )
            {
               case Material::BASE:
                  _visibles.pushBack( Chunk( STANDARD | hasSkin, e, t, geom, mat, p ) );
                  break;
               case Material::REFLECTIVE_PLANAR:
                  _reflections.pushBack( Chunk( REFLECTIVE, e, t, geom, mat, p ) );
                  break;
               case Material::CUSTOM:
                  _visibles.pushBack( Chunk( CUSTOM | hasSkin, e, t, geom, mat, p ) );
                  break;
               default:
                  // Unknown material type.
                  CHECK( false );
                  break;
            }
         }
      }
      else
      {
         ProxyEntity* pe = (ProxyEntity*)e;
         Entity*       e = pe->entity();

         // Do we have a proxy entity?
         if( !e || !pe->visible() ) continue;

         if( !geom ) geom = e->geometry();

         // Do we have a geometry to render?
         if( !geom || !e->visible() ) continue;

         MaterialSet* matSet  = pe->materialSet() ? pe->materialSet() : e->materialSet();
         if( !matSet ) matSet = _defMatSet.ptr();

         // Do we have a skeletal entity?
         int hasSkin = e->type() == Entity::SKELETAL ? HAS_SKIN : 0;

         // Transform.
         // For now, always use the proxy transform.
         const Mat4f* t = &pe->transform();

         // Create a renderable chunk for each patch.
         for( uint p = 0; p < geom->numPatches(); ++p )
         {
            Material* mat = geom->material( matSet, p );

            if( e->ghost() )
            {
               _ghosts.pushBack( Chunk( GHOST | hasSkin, e, t, geom, mat, p ) );
               continue;
            }

            switch( mat->type() )
            {
               case Material::BASE:
                  _visibles.pushBack( Chunk( STANDARD | hasSkin, e, t, geom, mat, p ) );
                  break;
               case Material::REFLECTIVE_PLANAR:
                  _reflections.pushBack( Chunk( REFLECTIVE, e, t, geom, mat, p ) );
                  break;
               case Material::CUSTOM:
                  _visibles.pushBack( Chunk( CUSTOM | hasSkin, e, t, geom, mat, p ) );
                  break;
               default:
                  // Unknown material type.
                  CHECK( false );
                  break;
            }
         }
      }
   }
}

//------------------------------------------------------------------------------
//!
Gfx::Pass*
Renderer::getPass()
{
   if( _numPasses >= _passes.size() ) _passes.pushBack( new Gfx::Pass );
   _passes[_numPasses]->clear();
   return _passes[_numPasses++].ptr();
}

//------------------------------------------------------------------------------
//!
Gfx::RenderNode*
Renderer::getRenderNode()
{
   if( _numRenderNodes >= _renderNodes.size() ) _renderNodes.pushBack( new Gfx::RenderNode() );
   _renderNodes[_numRenderNodes]->clear();
   return _renderNodes[_numRenderNodes++].ptr();
}

//------------------------------------------------------------------------------
//!
Gfx::SamplerList*
Renderer::getSamplerList()
{
   if( _numSamplerLists >= _samplerLists.size() ) _samplerLists.pushBack( new Gfx::SamplerList() );
   _samplerLists[_numSamplerLists]->clear();
   return _samplerLists[_numSamplerLists++].ptr();
}

//------------------------------------------------------------------------------
//!
Gfx::ConstantList*
Renderer::getConstantList()
{
   if( _numConstantLists >= _constantLists.size() ) _constantLists.pushBack( new Gfx::ConstantList() );
   _constantLists[_numConstantLists]->clear();
   return _constantLists[_numConstantLists++].ptr();
}

NAMESPACE_END
