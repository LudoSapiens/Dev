/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef GFX_GLES1_MANAGER_H
#define GFX_GLES1_MANAGER_H

#include <Gfx/StdDefs.h>

#if GFX_GLES_1_SUPPORT

#include <Gfx/Mgr/Manager.h>
#include <Gfx/Pass/Pass.h>
#include <Gfx/Tex/Texture.h>

NAMESPACE_BEGIN

namespace Gfx
{

class GLESContext;

/*==============================================================================
CLASS GLES1Manager
==============================================================================*/

class GLES1Manager:
   public Manager
{
public:
   /*----- methods -----*/

   GFX_DLL_API virtual void printInfo( TextStream& os ) const;

   GFX_DLL_API virtual void setSize( uint width, uint height );

   GFX_DLL_API virtual void  display();

   GFX_DLL_API virtual float oneToOneOffset();

   // Screen grab.
   GFX_DLL_API virtual bool  screenGrab( uint x, uint y, uint w, uint h, void* data );


   // Buffer.
   GFX_DLL_API virtual RCP<IndexBuffer>   createBuffer(
      const IndexFormat  format,
      const BufferFlags  flags,
      const size_t       sizeInBytes,
      const void*        data = NULL
   );

   GFX_DLL_API virtual RCP<VertexBuffer>  createBuffer(
      const BufferFlags  flags,
      const size_t       sizeInBytes,
      const void*        data = NULL
   );

   GFX_DLL_API virtual bool  setData(
      const RCP<IndexBuffer>&  buffer,
      const size_t             bufferSizeInBytes,
      const void*              data
   );

   GFX_DLL_API virtual bool  setData(
      const RCP<VertexBuffer>&  buffer,
      const size_t              bufferSizeInBytes,
      const void*               data
   );

   GFX_DLL_API virtual void*   map( const RCP<IndexBuffer>& buffer, const MapMode mode, const size_t offsetInBytes = 0, const size_t sizeInBytes = 0 );
   GFX_DLL_API virtual bool  unmap( const RCP<IndexBuffer>& buffer );

   GFX_DLL_API virtual void*   map( const RCP<VertexBuffer>& buffer, const MapMode mode, const size_t offsetInBytes = 0, const size_t sizeInBytes = 0 );
   GFX_DLL_API virtual bool  unmap( const RCP<VertexBuffer>& buffer );

   // Framebuffer.
   GFX_DLL_API virtual RCP<Framebuffer>  createFramebuffer();


   // Geometry.
   GFX_DLL_API virtual RCP<Geometry>  createGeometry( const PrimitiveType pt );


   // Program.
   GFX_DLL_API virtual RCP<Shader> createShader( ShaderType type, const String& code );
   GFX_DLL_API virtual RCP<Program> createProgram();


   // Constants.
   GFX_DLL_API virtual size_t getConstants(
      const RCP<Program>&          program,
      ConstantBuffer::Container&   constants,
      const Vector< Set<String> >* ignoreGroups = NULL,
      uint32_t*                    usedIgnoreGroup = NULL
   );

   GFX_DLL_API virtual RCP<ConstantBuffer> createConstants( size_t size );

   GFX_DLL_API virtual RCP<ConstantBuffer> createConstants(
      const ConstantBuffer::Container& constants,
      size_t                           size = 0
   );

   GFX_DLL_API virtual RCP<ConstantBuffer> createConstants(
      const RCP<Program>&          program,
      const Vector< Set<String> >* ignoreGroups = NULL,
      uint32_t*                    usedIgnoreGroup = NULL
   );


   // Texture.
   GFX_DLL_API virtual RCP<Texture>  create1DTexture(
      const uint            width,
      const TextureFormat   tfmt,
      const TextureChannels chOrder,
      const TextureFlags    flags
   );

   GFX_DLL_API virtual RCP<Texture>  create2DTexture(
      const uint            width,
      const uint            height,
      const TextureFormat   tfmt,
      const TextureChannels chOrder,
      const TextureFlags    flags
   );

   GFX_DLL_API virtual RCP<Texture>  create3DTexture(
      const uint            width,
      const uint            height,
      const uint            depth,
      const TextureFormat   tfmt,
      const TextureChannels chOrder,
      const TextureFlags    flags
   );

   GFX_DLL_API virtual RCP<Texture>  createCubeTexture(
      const uint            edgeLength,
      const TextureFormat   tfmt,
      const TextureChannels chOrder,
      const TextureFlags    flags
   );

   // Sets all of the texture's texels.
   GFX_DLL_API virtual bool  setData(
      const RCP<Texture>& texture,
      const uint          level,
      const void*         data,
      const bool          skipDefinedRegionUpdate = false
   );

   // Sets only the specified subset of texels of the 1D texture.
   GFX_DLL_API virtual bool  setData(
      const RCP<Texture>& texture,
      const uint          level,
      const uint          offset_x,
      const uint          width,
      const void*         data,
      const bool          skipDefinedRegionUpdate = false
   );

   // Sets only the specified subset of texels of the 2D texture.
   GFX_DLL_API virtual bool  setData(
      const RCP<Texture>& texture,
      const uint          level,
      const uint          offset_x,
      const uint          offset_y,
      const uint          width,
      const uint          height,
      const void*         data,
      const bool          skipDefinedRegionUpdate = false
   );

   // Sets only the specified slice of the 2D texture array or cubemap.
   GFX_DLL_API virtual bool  setData(
      const RCP<Texture>& texture,
      const uint          level,
      const uint          slice,
      const void*         data,
      const bool          skipDefinedRegionUpdate = false
   );

   // Sets only the specified subset of texels of the 2D texture array or cubemap.
   GFX_DLL_API virtual bool  setData(
      const RCP<Texture>& texture,
      const uint          level,
      const uint          slice,
      const uint          offset_x,
      const uint          offset_y,
      const uint          width,
      const uint          height,
      const void*         data,
      const bool          skipDefinedRegionUpdate = false
   );

   // Sets only the specified subset of texels of the 3D texture.
   GFX_DLL_API virtual bool  setData(
      const RCP<Texture>& texture,
      const uint          level,
      const uint          offset_x,
      const uint          offset_y,
      const uint          offset_z,
      const uint          width,
      const uint          height,
      const uint          depth,
      const void*         data,
      const bool          skipDefinedRegionUpdate = false
   );

   // Generate the mipmap chain using the base map
   GFX_DLL_API virtual bool  generateMipmaps( const RCP<Texture>& texture );


   // Pass.
   GFX_DLL_API virtual bool  render( const Pass& pass );


protected:

   friend class Manager;
   friend class GLESContext;

   /*----- methods -----*/

   GLES1Manager( GLESContext* context );

   virtual ~GLES1Manager();

   // Framebuffer
   bool  setFramebuffer( const Framebuffer* fb );

   // Geometry
   bool  executeGeometry(
      const Geometry*     geom,
      const Program*      prog,
      const ConstantList* constants,
      const SamplerList*  samp,
      const float*        matrices[],
      const float*        camPosition,
      const uint*         range
   );

private:

   /*----- Classes -----*/

   class Cache;

   /*----- data members -----*/

   RCP<GLESContext>   _context;
   uint               _curVertexAttributes;
   uint               _curTexUnitsUsed;
   mutable Cache*     _cache;
};


}  //namespace Gfx

NAMESPACE_END

#endif //GFX_GLES_1_SUPPORT

#endif //GFX_GLES1_MANAGER_H
