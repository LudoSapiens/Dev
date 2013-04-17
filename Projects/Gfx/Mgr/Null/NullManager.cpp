/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Gfx/Mgr/Null/NullManager.h>

#include <Gfx/Mgr/Null/NullContext.h>

//Complete forward declarations
#include <Gfx/FB/RenderState.h>
#include <Gfx/Geom/Buffer.h>
#include <Gfx/Geom/Geometry.h>
#include <Gfx/Prog/Program.h>
#include <Gfx/Tex/Sampler.h>
#include <Gfx/Tex/Texture.h>
#include <Gfx/Tex/TextureState.h>

#include <Base/Dbg/DebugStream.h>


/*==============================================================================
   GLOBAL NAMESPACE
==============================================================================*/

//USING_NAMESPACE
//using namespace Gfx;


/*==============================================================================
   UNNAMED NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

DBG_STREAM( os_nm, "NullManager" );

UNNAMESPACE_END


NAMESPACE_BEGIN

namespace Gfx
{

/*==============================================================================
   CLASS NullManager
==============================================================================*/

//------------------------------------------------------------------------------
//!
NullManager::NullManager( NullContext* context ):
   Manager( context, "null" )
{
   DBG_BLOCK( os_nm, "Creating NullManager" );
}

//------------------------------------------------------------------------------
//!
NullManager::~NullManager()
{
   DBG_BLOCK( os_nm, "Destroying NullManager" );
}

//------------------------------------------------------------------------------
//!
void
NullManager::display()
{
   DBG_BLOCK( os_nm, "NullManager::display()" );
}

//------------------------------------------------------------------------------
//!
float
NullManager::oneToOneOffset()
{
   return 0.0f;
}

//------------------------------------------------------------------------------
//!
RCP<IndexBuffer>
NullManager::createBuffer(
   const IndexFormat  format,
   const BufferFlags  flags,
   const size_t       /*bufferSizeInBytes*/,
   const void*        /*data*/
)
{
   return new IndexBuffer( format, flags );
}

//------------------------------------------------------------------------------
//!
RCP<VertexBuffer>
NullManager::createBuffer(
   const BufferFlags  flags,
   const size_t       /*bufferSizeInBytes*/,
   const void*        /*data*/
)
{
   return new VertexBuffer( flags );
}

//------------------------------------------------------------------------------
//!
bool
NullManager::setData(
   const RCP<IndexBuffer>&  /*buffer*/,
   const size_t             /*sizeInBytes*/,
   const void*              /*data*/
)
{
   return true;
}

//------------------------------------------------------------------------------
//!
bool
NullManager::setData(
   const RCP<VertexBuffer>&  /*buffer*/,
   const size_t              /*sizeInBytes*/,
   const void*               /*data*/
)
{
   return true;
}

//------------------------------------------------------------------------------
//!
void*
NullManager::map( const RCP<IndexBuffer>& /*buffer*/, const MapMode /*mode*/, const size_t /*offsetInBytes*/, const size_t /*sizeInBytes*/ )
{
   return NULL;
}

//------------------------------------------------------------------------------
//!
bool
NullManager::unmap( const RCP<IndexBuffer>& /*buffer*/ )
{
   return true;
}

//------------------------------------------------------------------------------
//!
void*
NullManager::map( const RCP<VertexBuffer>& /*buffer*/, const MapMode /*mode*/, const size_t /*offsetInBytes*/, const size_t /*sizeInBytes*/ )
{
   return NULL;
}

//------------------------------------------------------------------------------
//!
bool
NullManager::unmap( const RCP<VertexBuffer>& /*buffer*/ )
{
   return true;
}


//------------------------------------------------------------------------------
//!
RCP<Framebuffer>
NullManager::createFramebuffer()
{
   return new Framebuffer();
}

//------------------------------------------------------------------------------
//!
RCP<Geometry>
NullManager::createGeometry( const PrimitiveType /*pt*/ )
{
   return new Geometry();
}

//------------------------------------------------------------------------------
//!
RCP<Shader>
NullManager::createShader( ShaderType type, const String& /*code*/ )
{
   return new Shader( type );
}

//------------------------------------------------------------------------------
//!
RCP<Program>
NullManager::createProgram()
{
   return new Program();
}

//------------------------------------------------------------------------------
//!
size_t
NullManager::getConstants(
   const RCP<Program>&          /*program*/,
   ConstantBuffer::Container&   /*constants*/,
   const Vector< Set<String> >* /*ignoreGroups*/,
   uint32_t*                    /*usedIgnoreGroup*/
)
{
   return 0;
}

//------------------------------------------------------------------------------
//!
RCP<ConstantBuffer>
NullManager::createConstants( const size_t size )
{
   DBG_BLOCK( os_nm, "NullManager::createConstants(" << size << ")" );
   return new ConstantBuffer( size );
}

//------------------------------------------------------------------------------
//!
RCP<ConstantBuffer>
NullManager::createConstants( const ConstantBuffer::Container& constants, size_t size )
{
   DBG_BLOCK( os_nm, "NullManager::createConstants(" << (void*)&constants << ", " << size << ")" );
   return new ConstantBuffer( constants, size );
}

//------------------------------------------------------------------------------
//! MOVE TO ALL LAYERS
RCP<ConstantBuffer>
NullManager::createConstants(
   const RCP<Program>&          program,
   const Vector< Set<String> >* ignoreGroups,
   uint32_t*                    usedIgnoreGroup
)
{
   DBG_BLOCK( os_nm, "NullManager::createConstants()" );
   ConstantBuffer::Container constants;
   size_t size = getConstants( program, constants, ignoreGroups, usedIgnoreGroup );
   if( !constants.empty() )
   {
      return RCP<ConstantBuffer>( new ConstantBuffer( constants, size ) );
   }
   else
   {
      return RCP<ConstantBuffer>( new ConstantBuffer(0) );
   }
}

//------------------------------------------------------------------------------
//!
RCP<Texture>
NullManager::create1DTexture(
   const uint            width,
   const TextureFormat   tfmt,
   const TextureChannels chOrder,
   const TextureFlags    flags
)
{
   DBG_BLOCK( os_nm, "NullManager::create1DTexture()" );
   RCP<Texture> texture = new Texture();
   texture->set1D(width);
   texture->format(tfmt);
   texture->channelOrder(chOrder);
   texture->flags(flags);
   return texture;
}

//------------------------------------------------------------------------------
//!
RCP<Texture>
NullManager::create2DTexture(
   const uint            width,
   const uint            height,
   const TextureFormat   tfmt,
   const TextureChannels chOrder,
   const TextureFlags    flags
)
{
   DBG_BLOCK( os_nm, "NullManager::create3DTexture()" );
   RCP<Texture> texture = new Texture();
   texture->set2D(width, height);
   texture->format(tfmt);
   texture->channelOrder(chOrder);
   texture->flags(flags);
   return texture;
}

//------------------------------------------------------------------------------
//!
RCP<Texture>
NullManager::create3DTexture(
   const uint            width,
   const uint            height,
   const uint            depth,
   const TextureFormat   tfmt,
   const TextureChannels chOrder,
   const TextureFlags    flags
)
{
   DBG_BLOCK( os_nm, "NullManager::create3DTexture()" );
   RCP<Texture> texture = new Texture();
   texture->set3D(width, height, depth);
   texture->format(tfmt);
   texture->channelOrder(chOrder);
   texture->flags(flags);
   return texture;
}

//------------------------------------------------------------------------------
//!
RCP<Texture>
NullManager::createCubeTexture(
   const uint            edgeLength,
   const TextureFormat   tfmt,
   const TextureChannels chOrder,
   const TextureFlags    flags
)
{
   DBG_BLOCK( os_nm, "NullManager::createCubeTexture()" );
   RCP<Texture> texture = new Texture();
   texture->setCubemap(edgeLength);
   texture->format(tfmt);
   texture->channelOrder(chOrder);
   texture->flags(flags);
   return texture;
}

//------------------------------------------------------------------------------
//!
bool
NullManager::setData(
   const RCP<Texture>& /*texture*/,
   const uint          /*level*/,
   const void*         /*data*/,
   const bool          /*skipDefinedRegionUpdate*/
)
{
   DBG_BLOCK( os_nm, "NullManager::setData( tex )" );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
NullManager::setData(
   const RCP<Texture>& /*texture*/,
   const uint          /*level*/,
   const uint          /*offset_x*/,
   const uint          /*width*/,
   const void*         /*data*/,
   const bool          /*skipDefinedRegionUpdate*/
)
{
   DBG_BLOCK( os_nm, "NullManager::setData( 1D tex )" );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
NullManager::setData(
   const RCP<Texture>& /*texture*/,
   const uint          /*level*/,
   const uint          /*offset_x*/,
   const uint          /*offset_y*/,
   const uint          /*width*/,
   const uint          /*height*/,
   const void*         /*data*/,
   const bool          /*skipDefinedRegionUpdate*/
)
{
   DBG_BLOCK( os_nm, "NullManager::setData( 2D tex )" );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
NullManager::setData(
   const RCP<Texture>& /*texture*/,
   const uint          /*level*/,
   const uint          /*slice*/,
   const void*         /*data*/,
   const bool          /*skipDefinedRegionUpdate*/
)
{
   DBG_BLOCK( os_nm, "NullManager::setData( 2D tex )" );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
NullManager::setData(
   const RCP<Texture>& /*texture*/,
   const uint          /*level*/,
   const uint          /*slice*/,
   const uint          /*offset_x*/,
   const uint          /*offset_y*/,
   const uint          /*width*/,
   const uint          /*height*/,
   const void*         /*data*/,
   const bool          /*skipDefinedRegionUpdate*/
)
{
   DBG_BLOCK( os_nm, "NullManager::setData( 2D tex )" );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
NullManager::setData(
   const RCP<Texture>& /*texture*/,
   const uint          /*level*/,
   const uint          /*offset_x*/,
   const uint          /*offset_y*/,
   const uint          /*offset_z*/,
   const uint          /*width*/,
   const uint          /*height*/,
   const uint          /*depth*/,
   const void*         /*data*/,
   const bool          /*skipDefinedRegionUpdate*/
)
{
   DBG_BLOCK( os_nm, "NullManager::setData( 3D tex )" );
   return true;
}

//------------------------------------------------------------------------------
//! Generate the mipmap chain using the base map
bool
NullManager::generateMipmaps( const RCP<Texture>& /*texture*/ )
{
   DBG_BLOCK( os_nm, "NullManager::generateMipmaps()" );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
NullManager::render( const Pass& /*pass*/ )
{
   DBG_BLOCK( os_nm, "NullManager::render()" );
   return true;
}


}  //namespace Gfx

NAMESPACE_END
