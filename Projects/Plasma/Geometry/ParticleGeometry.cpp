/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Geometry/ParticleGeometry.h>

#include <Plasma/World/ParticleEntity.h>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN



UNNAMESPACE_END


/*==============================================================================
  CLASS ParticleGeometry
==============================================================================*/

//------------------------------------------------------------------------------
//!
ParticleGeometry::ParticleGeometry( ParticleEntity* e ):
   Geometry( Geometry::PARTICLE ),
   _entity( e )
{
   // Fake a patch to avoid culling in Renderer::classifyEntities().
   addPatch( 0, 0, 0 );
}

//------------------------------------------------------------------------------
//!
void
ParticleGeometry::computeRenderableGeometry()
{
   if( _entity == nullptr /*|| _entity->data().empty()*/ )
   {
      _rgeom = nullptr; // Deallocate the previous one, if any.
   }
   else
   {
      const ParticleData& data = _entity->data();
      _rgeom = Core::gfx()->createGeometry( Gfx::PRIM_POINTS );
      size_t s = data.capacity() * data.vertexStride() * sizeof(float);
      RCP<Gfx::VertexBuffer> buffer = Core::gfx()->createBuffer( Gfx::BUFFER_FLAGS_STREAMABLE, s, NULL );
      buffer->addAttribute( Gfx::ATTRIB_TYPE_POSITION, Gfx::ATTRIB_FMT_32F_32F_32F, buffer->strideInBytes() );
      if( data.hasSize()        ) buffer->addAttribute( Gfx::ATTRIB_TYPE_TEXCOORD1, Gfx::ATTRIB_FMT_32F            , buffer->strideInBytes() );
      if( data.hasColor()       ) buffer->addAttribute( Gfx::ATTRIB_TYPE_COLOR    , Gfx::ATTRIB_FMT_32F_32F_32F_32F, buffer->strideInBytes() );
      //if( data.hasOrientation() ) buffer->addAttribute( Gfx::ATTRIB_TYPE_TEXCOORD1, Gfx::ATTRIB_FMT_32F_32F_32F_32F, buffer->strideInBytes() );
      _rgeom->addBuffer( buffer );
   }
}

//------------------------------------------------------------------------------
//!
bool
ParticleGeometry::updateRenderData()
{
   CHECK( _rgeom.isValid() );
   CHECK( _rgeom->numBuffers() >= 1 );
   CHECK( _entity );

   const RCP<Gfx::VertexBuffer>& buffer = _rgeom->buffers()[0];
   const ParticleData& data = _entity->data();

   //data.print();
   //getchar();

   size_t s = data.size() * data.vertexStride() * sizeof(float);

   float* dst = (float*)Core::gfx()->map( buffer, Gfx::MAP_WRITE, 0, s );
   if( dst )
   {
      memcpy( dst, data.vertexData(), s );
      Core::gfx()->unmap( buffer );
      clearPatches();
      addPatch( 0, data.size(), 0 );
      return true;
   }
   else
   {
      CHECK( dst != NULL );
      return false;
   }
}
