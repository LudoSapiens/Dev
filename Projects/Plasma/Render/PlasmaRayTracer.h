/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_RAY_TRACER_H
#define PLASMA_RAY_TRACER_H

#include <Plasma/StdDefs.h>

#include <Plasma/World/Camera.h>
#include <Plasma/World/World.h>

#include <Fusion/Resource/Bitmap.h>
#include <Fusion/VM/GfxProxies.h>
#include <Fusion/VM/VMObject.h>

#include <CGMath/Plane.h>

#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>


NAMESPACE_BEGIN

/*==============================================================================
  CLASS PlasmaRayTracer
==============================================================================*/
//! This class renders blocks using ray-tracing.
//! It is meant as a backup solution instead of a new block system.
class PlasmaRayTracer:
   public RCObject
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API PlasmaRayTracer();
   PLASMA_DLL_API PlasmaRayTracer( const Vec2i& dim );
   PLASMA_DLL_API virtual ~PlasmaRayTracer();

   inline const RCP<Bitmap>&  bitmap() const { return _bmp; }

   inline const RCP<Gfx::Texture>&  texture() const { return _texture->get(); }

   PLASMA_DLL_API void  resize( const Vec2i& dim );

   PLASMA_DLL_API void  render( const RCP<World>& world, const RCP<Camera>& cam );
   PLASMA_DLL_API void  render( const RCP<World>& world, const RCP<Camera>& cam, const Vec2i& yRange );

   PLASMA_DLL_API void  prepareRender( const RCP<World>& );

   PLASMA_DLL_API void  registerTexture( const String& name );

   PLASMA_DLL_API void  registerMaterial( const Material* mat, const Vec4f& color, const String& texName );
   PLASMA_DLL_API void  unregisterMaterial( const Material* mat );

   inline bool save( const String& filename ) const { return bitmap()->saveFile(filename); }

   inline void clippingPlane( const Planef& p ) { _clipPlane = p; }
   inline const Planef& clippingPlane() const { return _clipPlane; }

   inline void samplesPerPixel( const Vec2i& v ) { _samplesPerPixel = v; _samplesPerPixel.clampMin(1); }
   inline const Vec2i& samplesPerPixel() const { return _samplesPerPixel; }

   inline void lodBias( float v ) { _lodBias = v; }
   inline float lodBias() const { return _lodBias; }

   // VM
   PLASMA_DLL_API void init( VMState* );
   PLASMA_DLL_API virtual bool performGet( VMState* );
   PLASMA_DLL_API virtual bool performSet( VMState* );

protected:

   /*----- types -----*/
   class MaterialData
   {
   public:
      MaterialData() {}
      MaterialData( const String& name, const Vec4f& color, const RCP<Bitmap>& baseMap ):
         _name( name ), _color( color )
      {
         _levels.pushBack( baseMap );
         generateMipmapChain();
      }

      void  generateMipmapChain();

      /*----- data members -----*/

      String                 _name;    //!< The texture's name (in the SamplerList).
      Vec4f                  _color;   //!< The material's color.
      Vector< RCP<Bitmap> >  _levels;  //!< The texel data.
   };
   typedef Map< const Material*, MaterialData >   MaterialMap;

   /*----- data members -----*/

   RCP<Bitmap>      _bmp;       //!< Our framebuffer (allows to save as PNG).
   RCP<GfxTexture>  _texture;   //!< A texture (allows to view the result).
   bool             _updateTex; //!< A flag indicating whether or not we want to keep the texture in sync.
   uint             _threads;   //!< The number of threads to launch.
   Planef           _clipPlane; //!< A clipping plane.
   Vec2i            _samplesPerPixel; //!< The number of samples per pixel.
   float            _lodBias;   //!< An lod bias.

   MaterialMap      _materialMap;  //!< A map to retrieve pixel data.

   /*----- methods -----*/

   void  allocate( const Vec2i& dim );

private:
}; //class PlasmaRayTracer


/*==============================================================================
  VM Section
==============================================================================*/

VMOBJECT_TRAITS( PlasmaRayTracer, rayTracer )
typedef VMObject< PlasmaRayTracer > PlasmaRayTracerVM;


NAMESPACE_END

#endif //PLASMA_RAY_TRACER_H
