/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef GFX_MANAGER_H
#define GFX_MANAGER_H

#include <Gfx/StdDefs.h>

#include <Gfx/FB/Framebuffer.h>
#include <Gfx/Geom/Buffer.h>
#include <Gfx/Geom/Geometry.h>
#include <Gfx/Pass/Pass.h>
#include <Gfx/Pass/RenderNode.h>
#include <Gfx/Prog/Constants.h>
#include <Gfx/Prog/Program.h>
#include <Gfx/Tex/Texture.h>

#include <Base/ADT/Set.h>
#include <Base/ADT/String.h>
#include <Base/IO/TextStream.h>
#include <Base/Util/RCObject.h>

#include <cstddef>

NAMESPACE_BEGIN

namespace Gfx
{

typedef enum
{
   MAP_READ       = 0x01,
   MAP_WRITE      = 0x02,
   MAP_READ_WRITE = MAP_READ | MAP_WRITE
} MapMode;

class Context;

/*==============================================================================
  CLASS Manager
==============================================================================*/
class Manager:
   public RCObject
{
public:

   /*----- static methods -----*/

   static GFX_DLL_API RCP<Manager>  create( Context* context = NULL );

   static const RCP<Texture>  backBuffer() { return RCP<Texture>(); }

   /*----- methods -----*/

   inline const String&  API() const { return _api; }
   inline Context*  context() const { return _context.ptr(); }

   GFX_DLL_API virtual void  printInfo( TextStream& os ) const;

   GFX_DLL_API virtual void setSize( uint width, uint height );
   GFX_DLL_API void getSize( uint& width, uint& height );

   GFX_DLL_API virtual void  display() = 0;

   GFX_DLL_API virtual float oneToOneOffset() = 0;

   // Screen grab.
   GFX_DLL_API virtual bool  screenGrab( uint x, uint y, uint w, uint h, void* data );

   // Buffer.
   GFX_DLL_API virtual RCP<IndexBuffer>   createBuffer(
      const IndexFormat  format,
      const BufferFlags  flags,
      const size_t       sizeInBytes,
      const void*        data = NULL
   ) = 0;

   GFX_DLL_API virtual RCP<VertexBuffer>  createBuffer(
      const BufferFlags  flags,
      const size_t       sizeInBytes,
      const void*        data = NULL
   ) = 0;

   GFX_DLL_API virtual bool  setData(
      const RCP<IndexBuffer>&  buffer,
      const size_t             sizeInBytes,
      const void*              data
   ) = 0;

   GFX_DLL_API virtual bool  setData(
      const RCP<VertexBuffer>&  buffer,
      const size_t              sizeInBytes,
      const void*               data
   ) = 0;

   GFX_DLL_API virtual void*   map( const RCP<IndexBuffer>& buffer, const MapMode mode, const size_t offsetInBytes = 0, const size_t sizeInBytes = 0 ) = 0;
   GFX_DLL_API virtual bool  unmap( const RCP<IndexBuffer>& buffer ) = 0;

   GFX_DLL_API virtual void*   map( const RCP<VertexBuffer>& buffer, const MapMode mode, const size_t offsetInBytes = 0, const size_t sizeInBytes = 0 ) = 0;
   GFX_DLL_API virtual bool  unmap( const RCP<VertexBuffer>& buffer ) = 0;

   // Framebuffer.
   GFX_DLL_API virtual RCP<Framebuffer>  createFramebuffer() = 0;

   // Geometry.
   GFX_DLL_API virtual RCP<Geometry>  createGeometry( const PrimitiveType pt ) = 0;


   // Program.
   GFX_DLL_API virtual RCP<Shader> createShader( ShaderType type, const String& code ) = 0;
   GFX_DLL_API virtual RCP<Program> createProgram() = 0;


   // Constants.
   GFX_DLL_API virtual size_t getConstants(
      const RCP<Program>&          program,
      ConstantBuffer::Container&   constants,
      const Vector< Set<String> >* ignoreGroups = NULL,
      uint32_t*                    usedIgnoreGroup = NULL
   ) = 0;

   GFX_DLL_API virtual RCP<ConstantBuffer> createConstants( size_t size ) = 0;

   GFX_DLL_API virtual RCP<ConstantBuffer> createConstants(
      const ConstantBuffer::Container& constants,
      size_t                           size = 0
   ) = 0;

   GFX_DLL_API virtual RCP<ConstantBuffer> createConstants(
      const RCP<Program>&          program,
      const Vector< Set<String> >* ignoreGroups = NULL,
      uint32_t*                    usedIgnoreGroup = NULL
   ) = 0;


   // Texture.
   GFX_DLL_API virtual RCP<Texture>  create1DTexture(
      const uint            width,
      const TextureFormat   tfmt,
      const TextureChannels chOrder,
      const TextureFlags    flags
   ) = 0;

   GFX_DLL_API virtual RCP<Texture>  create2DTexture(
      const uint            width,
      const uint            height,
      const TextureFormat   tfmt,
      const TextureChannels chOrder,
      const TextureFlags    flags
   ) = 0;

   GFX_DLL_API virtual RCP<Texture>  create3DTexture(
      const uint            width,
      const uint            height,
      const uint            depth,
      const TextureFormat   tfmt,
      const TextureChannels chOrder,
      const TextureFlags    flags
   ) = 0;

   GFX_DLL_API virtual RCP<Texture>  createCubeTexture(
      const uint            edgeLength,
      const TextureFormat   tfmt,
      const TextureChannels chOrder,
      const TextureFlags    flags
   ) = 0;

   // Sets all of the texture's texels.
   GFX_DLL_API virtual bool  setData(
      const RCP<Texture>& texture,
      const uint          level,
      const void*         data,
      const bool          skipDefinedRegionUpdate = false
   ) = 0;

   // Sets only the specified subset of texels of the 1D texture.
   GFX_DLL_API virtual bool  setData(
      const RCP<Texture>& texture,
      const uint          level,
      const uint          offset_x,
      const uint          width,
      const void*         data,
      const bool          skipDefinedRegionUpdate = false
   ) = 0;

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
   ) = 0;

   // Sets only the specified slice of the 2D texture array or cubemap.
   GFX_DLL_API virtual bool  setData(
      const RCP<Texture>& texture,
      const uint          level,
      const uint          slice,
      const void*         data,
      const bool          skipDefinedRegionUpdate = false
   ) = 0;

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
   ) = 0;

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
   ) = 0;

   // Generate the mipmap chain using the base map
   GFX_DLL_API virtual bool  generateMipmaps( const RCP<Texture>& texture ) = 0;


   // Pass.
   GFX_DLL_API virtual bool  render( const Pass& pass ) = 0;


   // RenderNode.
   virtual bool  render( const RCP<RenderNode>& rn )
   {
      Set< RCP<RenderNode> > j, v;
      return render(rn, j, v);
   }

   virtual bool  render(
      const RCP<RenderNode>&  rn,
      Set< RCP<RenderNode> >& jobsDone
   )
   {
      Set< RCP<RenderNode> > v;
      return render(rn, jobsDone, v);
   }

   GFX_DLL_API virtual bool  render(
      const RCP<RenderNode>&  rn,
      Set< RCP<RenderNode> >& jobsDone,
      Set< RCP<RenderNode> >& nodesVisited
   );


   // Filters.
   GFX_DLL_API RCP<Pass>  createFilterPass(
      const RCP<Texture>& srcTex,
      const RCP<Program>& srcPgm,
      const RCP<Texture>& dstTex,
      const uint          dstLvl = 0,
      const bool          generateMipmaps = false
   );

   GFX_DLL_API RCP<Pass>  createFilterPass(
      const RCP<Texture>&      srcTex,
      const RCP<Program>&      srcPgm,
      const RCP<ConstantList>& srcConst,
      const RCP<Texture>&      dstTex,
      const uint               dstLvl = 0,
      const bool               generateMipmaps = false
   );

   GFX_DLL_API RCP<Pass>  createFilterPass(
      const RCP<SamplerList>& srcSampList,
      const RCP<Program>&     srcPgm,
      const RCP<Texture>&     dstTex,
      const uint              dstLvl = 0,
      const bool              generateMipmaps = false
   );

   GFX_DLL_API RCP<Pass>  createFilterPass(
      const RCP<SamplerList>&  srcSampList,
      const RCP<Program>&      srcPgm,
      const RCP<ConstantList>& srcConst,
      const RCP<Texture>&      dstTex,
      const uint               dstLvl = 0,
      const bool               generateMipmaps = false
   );

   // Have variants replacing dst tex with a Framebuffer object instead (allows ping-pong rendering)
   // Also, could add a boolean to control if we call execGeom or just setGeom (and leave exec to user)

   // A quad with (0,0)-(1,1) coordinates used for filtering passes.
   GFX_DLL_API const RCP<Geometry>&  getOneToOneGeometry();

protected:

   /*----- data members -----*/

   RCP<Context>   _context;       //!< A pointer to the associated context.
   String         _api;           //!< String representing the API ("OpenGL" or "Direct3D" or "Null")
   uint           _width;
   uint           _height;
   uint           _curWidth;
   uint           _curHeight;
   bool           _doingRTT;      //!< A boolean indicating whether or not we are doing render-to-texture
   RCP<Geometry>  _oneToOneGeom;  //!< A simple quad, fully mapping a texture (reused in all filters)

   /*----- methods -----*/

   Manager( Context* context, const String& api );

   virtual ~Manager();

private:
};


}  //namespace Gfx

NAMESPACE_END


#endif //GFX_MANAGER_H
