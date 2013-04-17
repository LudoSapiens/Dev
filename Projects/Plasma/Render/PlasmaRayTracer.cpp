/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Render/PlasmaRayTracer.h>

#include <Plasma/Intersector.h>
#include <Plasma/World/Entity.h>
#include <Plasma/World/Material.h>

#include <Fusion/Core/Core.h>
#include <Fusion/Resource/BitmapManipulator.h>
#include <Fusion/Resource/ResManager.h>

#include <Base/ADT/StringMap.h>
#include <Base/MT/Thread.h>
#include <Base/Util/Timer.h>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
enum
{
   ATTRIB_CLIPPING_PLANE,
   ATTRIB_LOD_BIAS,
   ATTRIB_REGISTER_MATERIAL,
   ATTRIB_REGISTER_TEXTURE,
   ATTRIB_RENDER,
   ATTRIB_RESIZE,
   ATTRIB_SAMPLES_PER_PIXEL,
   ATTRIB_SAVE,
   ATTRIB_TEXTURE,
   ATTRIB_THREADS,
   NUM_ATTRIBS
};

//------------------------------------------------------------------------------
//!
StringMap _attributes(
   "clippingPlane",     ATTRIB_CLIPPING_PLANE,
   "lodBias",           ATTRIB_LOD_BIAS,
   "registerMaterial",  ATTRIB_REGISTER_MATERIAL,
   "registerTexture",   ATTRIB_REGISTER_TEXTURE,
   "render",            ATTRIB_RENDER,
   "resize",            ATTRIB_RESIZE,
   "samplesPerPixel",   ATTRIB_SAMPLES_PER_PIXEL,
   "save",              ATTRIB_SAVE,
   "texture",           ATTRIB_TEXTURE,
   "threads",           ATTRIB_THREADS,
   ""
);

//------------------------------------------------------------------------------
//!
int registerMaterialVM( VMState* vm )
{
   PlasmaRayTracer* prt = (PlasmaRayTracer*)VM::thisPtr(vm);
   CHECK( prt != NULL );

   RCP<Material> mat;
   MaterialVM::get( vm, 1, "mat", mat );

   Vec4f color;
   VM::get( vm, 1, "color", color );

   String tex;
   VM::get( vm, 1, "tex", tex );

   prt->registerMaterial( mat.ptr(), color, tex );

   return 0;
}

//------------------------------------------------------------------------------
//!
int registerTextureVM( VMState* vm )
{
   PlasmaRayTracer* prt = (PlasmaRayTracer*)VM::thisPtr(vm);
   CHECK( prt != NULL );
   prt->registerTexture( VM::toString( vm, 1 ) );
   return 0;
}

//------------------------------------------------------------------------------
//!
int renderVM( VMState* vm )
{
   PlasmaRayTracer* prt = (PlasmaRayTracer*)VM::thisPtr(vm);
   CHECK( prt != NULL );
   prt->render( WorldVM::to( vm, 1 ), CameraVM::to( vm, 2 ) );
   return 0;
}

//------------------------------------------------------------------------------
//!
int resizeVM( VMState* vm )
{
   PlasmaRayTracer* prt = (PlasmaRayTracer*)VM::thisPtr(vm);
   CHECK( prt != NULL );
   prt->resize( VM::toVec2i( vm, 1 ) );
   return 0;
}

//------------------------------------------------------------------------------
//!
int saveVM( VMState* vm )
{
   PlasmaRayTracer* prt = (PlasmaRayTracer*)VM::thisPtr(vm);
   CHECK( prt != NULL );
   VM::push( vm, prt->save( VM::toString( vm, 1 ) ) );
   return 1;
}

//------------------------------------------------------------------------------
//!
void convertPixelRGB( const uchar* src, float* dst )
{
   dst[0] = (float)(src[0])*1.0f/255.0f;
   dst[1] = (float)(src[1])*1.0f/255.0f;
   dst[2] = (float)(src[2])*1.0f/255.0f;
}

//------------------------------------------------------------------------------
//!
Vec4f bilinearFilter( const Bitmap& tex, const Vec2f& st )
{
   // Clamp coordinate to be within [0, 1] range.
   Vec2f texUV( CGM::fmod(st.x, 1.0f), CGM::fmod(st.y, 1.0f) );

   // Range [0, w], [0, h].
   texUV *= Vec2f( tex.dimension() );

   // Bilinear offset.
   texUV -= Vec2f( 0.5f );

   // Extract integer (pixel position) and fractional (weight) parts.
   int xi = CGM::floori(texUV.x);
   int yi = CGM::floori(texUV.y);
   float wh = texUV.x - xi;
   float wv = texUV.y - yi;

   //StdErr << "st=" << st << nl;
   //StdErr << "xi=" << xi << " yi=" << yi << nl;
   //StdErr << "wh=" << wh << " wv=" << wv << nl;
   //StdErr << "dim=" << tex.dimension() << nl;

   xi = CGM::modulo( xi, tex.dimension().x );
   yi = CGM::modulo( yi, tex.dimension().y );
   int xip1 = CGM::modulo( xi + 1, tex.dimension().x );
   int yip1 = CGM::modulo( yi + 1, tex.dimension().y );
   //StdErr << "xi=" << xi << " yi=" << yi << nl;
   //StdErr << "xip1=" << xip1 << " yip1=" << yip1 << nl;

   Vec3f tl, tr, bl, br;
   const uchar* pixel;
   if( tex.pixelType() == Bitmap::BYTE )
   {
      pixel = tex.pixel( Vec2i(xi,   yi  ) );
      convertPixelRGB( pixel, tl.ptr() );
      //StdErr << xi << "," << yi << ": " << String().format("0x%02x%02x%02x%02x", pixel[3], pixel[2], pixel[1], pixel[0]) << " " << tl << nl;

      pixel = tex.pixel( Vec2i(xip1, yi  ) );
      convertPixelRGB( pixel, tr.ptr() );
      //StdErr << xip1 << "," << yi << ": " << String().format("0x%02x%02x%02x%02x", pixel[3], pixel[2], pixel[1], pixel[0]) << " " << tr << nl;

      pixel = tex.pixel( Vec2i(xi,   yip1) );
      convertPixelRGB( pixel, bl.ptr() );
      //StdErr << xi << "," << yip1 << ": " << String().format("0x%02x%02x%02x%02x", pixel[3], pixel[2], pixel[1], pixel[0]) << " " << bl << nl;

      pixel = tex.pixel( Vec2i(xip1, yip1) );
      convertPixelRGB( pixel, br.ptr() );
      //StdErr << xip1 << "," << yip1 << ": " << String().format("0x%02x%02x%02x%02x", pixel[3], pixel[2], pixel[1], pixel[0]) << " " << br << nl;
   }
   else
   {
      StdErr << "Only RGB8 supported" << nl;
      CHECK( false );
   }

   Vec3f v3 = CGM::bilinear(
      tl, tr-tl,
      bl, br-bl,
      wh, wv
   );

   //StdErr << "bilin=" << v3 << nl;
   return Vec4f( v3, 1.0f );
}

//------------------------------------------------------------------------------
//!
Vec4f trilinearFilter( const Vector< RCP<Bitmap> >& texChain, const float lod, const Vec2f& st )
{
   float lodClamped = CGM::clamp( lod, 0.0f, (float)(texChain.size() - 1) );

   int   mipA = (int)lodClamped;
   float mipF = lodClamped - mipA;

   Vec4f color = bilinearFilter( *(texChain[mipA]), st );
   if( mipF != 0.0f )
   {
      color *= 1.0f - mipF;
      color += bilinearFilter( *(texChain[mipA+1]), st ) * mipF;
   }

   return color;
}

/*==============================================================================
  CLASS RayTraceTask
==============================================================================*/
class RayTraceTask:
   public Task
{
public:

   /*----- methods -----*/

   RayTraceTask( PlasmaRayTracer* prt, const RCP<World>& w, const RCP<Camera>& c, const Vec2i& yRange ):
      _plasmaRayTracer( prt ), _world( w ), _camera( c ), _yRange( yRange )
   {}

   virtual void execute()
   {
      _plasmaRayTracer->render( _world, _camera, _yRange );
   }

protected:

   /*----- data members -----*/

   PlasmaRayTracer*    _plasmaRayTracer;
   const RCP<World>&   _world;
   const RCP<Camera>&  _camera;
   const Vec2i         _yRange;

   /*----- methods -----*/

   /* methods... */

private:
}; //class RayTraceTask


UNNAMESPACE_END

//------------------------------------------------------------------------------
//!
PlasmaRayTracer::PlasmaRayTracer():
   _threads(0), _samplesPerPixel(1, 1), _lodBias( 0.0f )
{
}

//------------------------------------------------------------------------------
//!
PlasmaRayTracer::PlasmaRayTracer( const Vec2i& dim )
{
   allocate( dim );
}

//------------------------------------------------------------------------------
//!
PlasmaRayTracer::~PlasmaRayTracer()
{
}

//------------------------------------------------------------------------------
//!
void
PlasmaRayTracer::resize( const Vec2i& dim )
{
   if( _bmp.isNull() || _bmp->dimension() != dim )
   {
      allocate( dim );
   }
}

//------------------------------------------------------------------------------
//!
void
PlasmaRayTracer::allocate( const Vec2i& dim )
{
   _bmp = new Bitmap( dim, Bitmap::BYTE, 4 );

   if( _texture.isNull() ) _texture = new GfxTexture();

   _texture->set(
      Core::gfx()->create2DTexture(
         dim.x,
         dim.y,
         //Gfx::TEX_FMT_32F_32F_32F_32F,
         Gfx::TEX_FMT_8_8_8_8,
         Gfx::TEX_CHANS_RGBA,
         Gfx::TEX_FLAGS_NONE
      )
   );
}

//------------------------------------------------------------------------------
//!
void
PlasmaRayTracer::render( const RCP<World>& world, const RCP<Camera>& cam )
{
   prepareRender( world );

   Timer timer;
   Vector< Thread* >  threads;
   if( _threads < 2 )
   {
      render( world, cam, Vec2i(0, _bmp->dimension().y-1) );
   }
   else
   {
      int h_n      = _bmp->dimension().y / _threads;
      int leftOver = _bmp->dimension().y - ( h_n * (_threads-1) );
      Vec2i yRange( 0, leftOver-1 );
      for( uint t = 0; t < _threads; ++t )
      {
         //StdErr << "Rendering: " << yRange << " " << yRange(1) - yRange(0) + 1 << nl;
         threads.pushBack( new Thread( new RayTraceTask( this, world, cam, yRange) ) );
         yRange(0) = yRange(1) + 1;
         yRange(1) += h_n;
      }
      CHECK( (yRange(1) + 1) == (_bmp->dimension().y + h_n) );
   }

   for( uint i = 0; i < threads.size(); ++i )
   {
      threads[i]->wait();
      delete threads[i];
   }

   StdErr << "Ray-tracing (" << _threads << " threads) took " << timer.elapsed() << " seconds" << nl;
   // Update texture in OpenGL.
   Core::gfx()->setData( _texture->get(), 0, _bmp->pixels() );
}

//------------------------------------------------------------------------------
//!
void
PlasmaRayTracer::render( const RCP<World>& world, const RCP<Camera>& cam, const Vec2i& yRange )
{
   Vec3f lightDir( 0.6f, 0.8f, 1.0f );
   lightDir.normalize();

   uint baseMap = 0;
   if( _lodBias > 0.0f )
   {
      baseMap += (uint)(_lodBias+0.5f);
   }

   Vec2d screenCoord;
   Vec2d sampleCoord;
   Vec2d sampleOffset;
   Vec2d screenLimit( _bmp->dimension().x, yRange(1) + 1 );
   Vec2d halfSample( 0.5/_samplesPerPixel.x, 0.5/_samplesPerPixel.y );
   double sampleScale = 1.0 / (_samplesPerPixel.x * _samplesPerPixel.y);
   Vec4f color;
   Vec4f finalColor;

   uchar* dstPixelPtr = _bmp->pixelRow( yRange(0) );

   Rayf ray;
   ray.origin( cam->position() );

   bool camInFront = _clipPlane.inFront( cam->position() );

   for( screenCoord.y = yRange(0); screenCoord.y < screenLimit.y; ++screenCoord.y )
   {
      for( screenCoord.x = 0.0; screenCoord.x < screenLimit.x; ++screenCoord.x )
      {
#if 0
         if( (screenCoord.x < 773.0f || 774.0f < screenCoord.x) ||
             (screenCoord.y < 492.0f || 492.0f < screenCoord.y) )
         {
            dstPixelPtr += 4;
            continue;
         }
#endif

         finalColor = Vec4f::zero();

         for( sampleOffset.y = halfSample.y; sampleOffset.y < 1.0; sampleOffset.y += 2.0*halfSample.y )
         {
            for( sampleOffset.x = halfSample.x; sampleOffset.x < 1.0; sampleOffset.x += 2.0*halfSample.x )
            {
               sampleCoord = screenCoord + sampleOffset;
               //StdErr << "Sample Coord: " << sampleCoord << " offset=" << sampleOffset << nl;
               //getchar();

               ray.direction( cam->direction( sampleCoord ) );

               Intersector::Hit hit;

               bool doTrace = true;
#if 1
               // Clipping plane.
               if( Intersector::trace( _clipPlane, ray, hit._t ) )
               {
                  if( !camInFront )
                  {
                     hit._tMin = CGM::max( 0.0f, hit._t);
                     hit._t = CGConstf::infinity();
                  }
                  else
                  {
                     // hit._t already contains the max allowed value.
                  }
               }
               else
               {
                  if( !camInFront )
                  {
                     doTrace = false;
                  }
               }
#endif

               if( doTrace && Intersector::traceBlocks( world, ray, hit ) )
               {
                  //StdErr << "Hitting material #" << hit._materialID << " at " << hit._uv << nl;
                  const RCP<Material>& mat = hit._entity->material( hit._materialID );
                  if( mat.isValid() )
                  {
                     MaterialMap::ConstIterator it = _materialMap.find( mat.ptr() );
                     if( it != _materialMap.end() )
                     {
                        const MaterialData& matData = _materialMap[mat.ptr()];
                        Vec2f texCoord = Vec2f(hit._uv.y, hit._uv.x) * 0.125; // The 1/8 factor and flip is the same as demo.glsl
#if 0
                        //StdErr << nl;
                        //StdErr << "tc=" << texCoord;
                        // Throw a new ray on the top-right of the screen pixel.
                        //ray.direction( cam->direction(screenCoord + Vec2d(1.0, 1.0)) );
                        ray.direction( cam->direction(screenCoord + Vec2d(0.35355, 0.35355)) ); // Radial distance of 0.5.
                        hit._entity->blocks()->retrace( ray, hit );
                        Vec2f texCoord2 = Vec2f(hit._uv.y, hit._uv.x) * 0.125; // The 1/8 factor and flip is the same as demo.glsl
                        //StdErr << " ";
                        //StdErr << " tc2=" << texCoord2;
                        //StdErr << " d=" << (texCoord2 - texCoord).length();
                        //StdErr << nl;
                        float lod = (float)matData._levels.size(); // Corresponds to log2( maxDim )
                        lod += 0.5f * CGM::log2( (texCoord2 - texCoord).sqrLength() ); // log2( len ) = log2( (len^2)^1/2 ) = 0.5 * log2( len^2 )
                        lod += lodBias;
                        //StdErr << "lod=" << lod << nl;
                        //StdErr << "baseMap=" << matData._levels[0]->dimension() << nl;
                        color = trilinearFilter( matData._levels, lod, texCoord );
                        //StdErr << "color = " << color << nl;
                        //const uint numLevels = (uint)matData._levels.size();
                        //for( uint i = 0; i < numLevels; ++i )
                        //{
                        //   StdErr << "Material " << hit._materialID << " level " << i << " " << matData._levels[i]->dimension() << nl;
                        //   matData._levels[i]->saveFile( String().format("mat%d_mip%d", hit._materialID, i ) );
                        //}
#else
                        uint mip = CGM::min( baseMap, (uint)matData._levels.size()-1 );
                        color = bilinearFilter( *(matData._levels[mip]), texCoord );
                        //color = bilinearFilter( *(matData._levels.back()), texCoord );
#endif

                        float v = lightDir.dot( hit._normal );
                        v = v * 1.5f;

                        Intersector::Hit hitl;
                        Rayf rayl( hit._pos + hit._normal*0.1f, lightDir );

                        if( Intersector::traceBlocks( world, rayl, hitl ) )
                        {
                           v = 0.5f;
                        }

                        v = CGM::clamp( v, 0.5f, 2.0f );

                        color *= matData._color * v;
                        color.w = 1.0f;
                     }
                     else
                     {
                        StdErr << "Unregistered material id=" << hit._materialID << nl;
                        color = Vec4f( hit._uv, 0.0f, 1.0f );
                     }
                  }
                  else
                  {
                     color = Vec4f( hit._uv, 0.0f, 1.0f );
                  }
               }
               else
               {
                  color = world->backgroundColor();
               }

               finalColor += color;
            } // sampleOffset.x
         } // sampleOffset.y

         finalColor *= (float)sampleScale;
         finalColor.clamp( 0.0f, 1.0f );

         // Force red.
         //finalColor = Vec4f( 1.0f, 0.0f, 0.0f, 1.0f );

         *dstPixelPtr++ = (uchar)(finalColor.x * 255.0f);
         *dstPixelPtr++ = (uchar)(finalColor.y * 255.0f);
         *dstPixelPtr++ = (uchar)(finalColor.z * 255.0f);
         *dstPixelPtr++ = (uchar)(finalColor.w * 255.0f);
      }
   }
}

//------------------------------------------------------------------------------
//!
void
PlasmaRayTracer::prepareRender( const RCP<World>& world )
{
#if FIXME
   World::EntityContainer::ConstIterator curE = world->entities().begin();
   World::EntityContainer::ConstIterator endE = world->entities().end();
   while( curE != endE )
   {
      const RCP<Blocks>& blocks = (*curE)->blocks();

      if( blocks.isValid() )
      {
         blocks->computeBIH();
      }

      ++curE;
   }
#endif
}

//------------------------------------------------------------------------------
//!
void
PlasmaRayTracer::registerTexture( const String& name )
{
#if FIXME
   if( name.empty() )
   {
      // Unregister texture.
      ResManager::setTexture( name, NULL );
      _updateTex = true;
   }
   else
   {
      // Register, and start updating on every render.
      ResManager::setTexture( name, texture() );
      _updateTex = true;
   }
#endif
}

//------------------------------------------------------------------------------
//!
void
PlasmaRayTracer::registerMaterial( const Material* mat, const Vec4f& color, const String& texName )
{
#if FIXME
   String path = ResManager::getTexturePath( texName );
   if( path.empty() )
   {
      StdErr << "ERROR - PlasmaRayTracer::registerMaterial() - Null texture path." << nl;
      return;
   }

   RCP<Bitmap> bmp = new Bitmap( path );
   if( bmp.isNull() )
   {
      StdErr << "ERROR - PlasmaRayTracer::registerMaterial() - Could not load bitmap named '" << texName << "'." << nl;
      return;
   }

   if( bmp->numChannels() == 1 )
   {
      bmp = BitmapManipulator::luminanceToRGB( *bmp );
   }

   _materialMap[mat] = MaterialData( texName, color, bmp );
#endif
}

//------------------------------------------------------------------------------
//!
void
PlasmaRayTracer::unregisterMaterial( const Material* mat )
{
   MaterialMap::Iterator it = _materialMap.find( mat );
   if( it != _materialMap.end() )
   {
      _materialMap.erase( it );
   }
   else
   {
      StdErr << "ERROR - PlasmaRayTracer::unregisterMaterial() - Unregistered inexistent material." << nl;
   }
}

//------------------------------------------------------------------------------
//!
void
PlasmaRayTracer::init( VMState* vm )
{
   if( VM::isTable(vm, -1) )
   {
      VM::push(vm); // Start iterating at index 0 (nil).
      while( VM::next(vm, -2) )
      {
         performSet(vm);
         VM::pop(vm, 1); // Pop the value, keep the key.
      }
   }
}

//------------------------------------------------------------------------------
//!
bool
PlasmaRayTracer::performGet( VMState* vm )
{
   switch( _attributes[VM::toCString(vm, -1)] )
   {
      case ATTRIB_CLIPPING_PLANE:
         VM::push( vm, Vec4f(_clipPlane.a(), _clipPlane.b(), _clipPlane.c(), _clipPlane.d()) );
         return true;
      case ATTRIB_LOD_BIAS:
         VM::push( vm, lodBias() );
         return true;
      case ATTRIB_REGISTER_MATERIAL:
         VM::push( vm, this, registerMaterialVM );
         return true;
      case ATTRIB_REGISTER_TEXTURE:
         VM::push( vm, this, registerTextureVM );
         return true;
      case ATTRIB_RENDER:
         VM::push( vm, this, renderVM );
         return true;
      case ATTRIB_RESIZE:
         VM::push( vm, this, resizeVM );
         return true;
      case ATTRIB_SAMPLES_PER_PIXEL:
         VM::push( vm, samplesPerPixel() );
         return true;
      case ATTRIB_SAVE:
         VM::push( vm, this, saveVM );
         return true;
      case ATTRIB_TEXTURE:
         GfxTextureVM::push( vm, _texture );
         return true;
      case ATTRIB_THREADS:
         VM::push( vm, _threads );
         return true;
      default:
         break;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
bool
PlasmaRayTracer::performSet( VMState* vm )
{
   switch( _attributes[VM::toCString(vm, -2)] )
   {
      case ATTRIB_CLIPPING_PLANE:
      {
         Vec4f abcd = VM::toVec4f( vm, -1 );
         _clipPlane.a( abcd.x );
         _clipPlane.b( abcd.y );
         _clipPlane.c( abcd.z );
         _clipPlane.d( abcd.w );
      }  return true;
      case ATTRIB_LOD_BIAS:
         lodBias( (float)VM::toNumber( vm, -1 ) );
         return true;
      case ATTRIB_SAMPLES_PER_PIXEL:
         samplesPerPixel( VM::toVec2i( vm, -1 ) );
         return true;
      case ATTRIB_REGISTER_MATERIAL:
      case ATTRIB_REGISTER_TEXTURE:
      case ATTRIB_RENDER:
      case ATTRIB_RESIZE:
      case ATTRIB_SAVE:
      case ATTRIB_TEXTURE:
         // Read-only.
         return true;
      case ATTRIB_THREADS:
         _threads = VM::toUInt( vm, -1 );
         return true;
      default:
         break;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
void
PlasmaRayTracer::MaterialData::generateMipmapChain()
{
   if( _levels.empty() )  return;
   CHECK( _levels.size() == 1 );
   uint numLevels = Gfx::Texture::getNumLevelsFromSize( _levels.front()->dimension().x, _levels.front()->dimension().y );
   _levels.reserve( numLevels );
   for( uint i = 1; i < numLevels; ++i )
   {
      _levels.pushBack( BitmapManipulator::downsample( *_levels[i-1] ) );
   }
#if 0
   for( uint i = 0; i < numLevels; ++i )
   {
      StdErr << "Mipmap level " << i << " " << _levels[i]->dimension() << nl;
      //_levels[i]->saveFile( String().format("level_%d", i ) );
   }
#endif
}
