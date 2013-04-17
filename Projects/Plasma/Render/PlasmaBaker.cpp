/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Render/PlasmaBaker.h>
#include <Plasma/Geometry/SurfaceGeometry.h>
#include <Plasma/Intersector.h>
#include <Plasma/Render/ForwardRenderer.h>
#include <Plasma/World/Camera.h>
#include <Plasma/World/Viewport.h>

#include <Fusion/Resource/Image.h>

#include <CGMath/Random.h>
#include <CGMath/Distributions.h>

#include <Base/Util/Timer.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

/*
//------------------------------------------------------------------------------
//!
int
vertexAOVM( VMState* vm )
{
   RCP<Surface> surface = SurfaceVM::to( vm, 1 );
   PlasmaBaker::vertexAO( surface.ptr() );
   return 0;
}

//------------------------------------------------------------------------------
//!
const VM::Reg baker_funcs[] = {
   { "vertexAO",  vertexAOVM },
   { 0, 0 }
};
*/
UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
  CLASS PlasmaBaker
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
PlasmaBaker::initialize( VMState* /*vm*/ )
{
   //VM::registerFunctions( vm, "plasma", baker_funcs );
}

//------------------------------------------------------------------------------
//!
void
PlasmaBaker::vertexAO( World* )
{
   // TODO
}

//------------------------------------------------------------------------------
//!
void
PlasmaBaker::vertexAO( SurfaceGeometry* surface )
{
   uint numRays   = 264;
   float len      = 10.0f;
   //float fraction = 0.3f;

   Vec3f lightDir( 0.6f, 0.4f, 1.0f );
   lightDir.normalize();

   RNG_WELL rng;

   printf( "starting baking AO...\n" );

   // Building acceleration structure.
   surface->computeBIH();

   // Compute AO per vertex.
   Vector<bool> done( surface->numNormals(), false );

   // Precompute normals.
   Vector<Vec3f> normals(numRays);
   for( uint i = 0; i < numRays; ++i )
   {
      normals[i] = rng( UniformSphere<float>() );
   }

   Timer timer;
   for( uint p = 0; p < surface->numPatches(); ++p )
   {
      SurfaceGeometry::Patch* patch = surface->patches()[p].ptr();
      for( uint f = 0; f < patch->numFaces(); ++f )
      {
         for( uint v = 0; v < 3; ++v )
         {
            uint nid  = patch->faces()[f]._dID[v];
            uint vid  = patch->faces()[f]._vID[v];

            // Already computed?
            if( done[nid] )
            {
               continue;
            }

            Vec3f vertex = surface->vertex(vid);
            Vec3f n      = surface->normal(nid);
            uint vid1    = patch->faces()[f]._vID[(v+1)%3];
            uint vid2    = patch->faces()[f]._vID[(v+2)%3];
            Vec3f e0     = surface->vertex(vid1) - vertex;
            Vec3f e1     = surface->vertex(vid2) - vertex;
            Vec3f de     = ( e0.getNormalized() + e1.getNormalized() ) * 0.001f;

            // Compute vertex AO.
            Rayf ray( vertex, n );

            float totalLight = 0.0f;
            float maxLight   = 0.0f;

            for( uint r = 0; r < numRays; ++r )
            {
               float ldotn = normals[r].dot(n);
               if( ldotn > 0.0f )
               {
                  ray.direction( normals[r] );
               }
               else
               {
                  ray.direction( -normals[r] );
                  ldotn = -ldotn;
               }
               ray.origin( vertex + de + ray.direction()*0.005f );
               ray.direction( ray.direction() * len );

               maxLight += ldotn;
               if( !Intersector::trace( surface, ray ) )
               {
                  totalLight += ldotn;
               }
            }

            float ao = CGM::max( 0.1f, totalLight/maxLight );

            // Compute directional lighting.
            ray.direction( lightDir*10000.0f );


            float dl = 1.0f;

            // FIXME: Tangent are wrong!!
            //const Vec4f& t = surface->tangent( vid );
            //Vec3f tan( t.x, t.y, t.z );
            Vec3f tan  = Vec3f::perpendicular( n );
            Vec3f btan = n.cross( tan );


            ray.origin( vertex + 0.005f*n - 0.5f*tan -0.5f*btan );
            if( Intersector::trace( surface, ray ) )
            {
               dl -= 0.25f;
            }
            ray.origin( ray.origin() + tan );
            if( Intersector::trace( surface, ray ) )
            {
               dl -= 0.25f;
            }
            ray.origin( ray.origin() + btan );
            if( Intersector::trace( surface, ray ) )
            {
               dl -= 0.25f;
            }
            ray.origin( ray.origin() - tan );
            if( Intersector::trace( surface, ray ) )
            {
               dl -= 0.25f;
            }

            dl *= CGM::max( 0.0f, lightDir.dot( n ) );

            ao = (ao+dl) * 0.75f;

            // Rescale normal to lighting.
            surface->rescaleNormal( nid, ao );
            done[nid] = true;
         }
      }
   }

   printf( "finish in %f\n", timer.elapsed() );

   // Clear acceleration structure.
   surface->clearBIH();
}


/*==============================================================================
  CLASS CubemapBaker::Job
==============================================================================*/

//------------------------------------------------------------------------------
//!
CubemapBaker::Job::Job( World* w, const Vec3f& p, Image* i ):
   _world(w), _pos(p), _dim(1), _slice(0), _img(i)
{
   if( i )
   {
      Bitmap* b = i->bitmap();
      CHECK( b->dimType() == Bitmap::DIM_CUBEMAP );
      CHECK( b->depth() == 6 );
      CHECK( b->width() == b->height() );
      CHECK( b->pixelType() == Bitmap::BYTE );
      CHECK( b->numChannels() == 4 );
      _dim = b->dimension()(0,1);
   }
}

/*==============================================================================
  CLASS CubemapBaker
==============================================================================*/

//------------------------------------------------------------------------------
//!
CubemapBaker::CubemapBaker()
{
   if( Core::gfxVersion() == 1 )
   {
      //_renderer = new ForwardRendererFixed();
   }
   else
   {
      _renderer = new ForwardRenderer();
   }
}

//------------------------------------------------------------------------------
//!
CubemapBaker::~CubemapBaker()
{
}

//------------------------------------------------------------------------------
//!
void
CubemapBaker::add( World* w, const Vec3f& p, const Vec2i& d, const String& s )
{
   LockGuard g( _jobsLock );
   _jobs.pushBack( Job(w,p,d,s) );
}

//------------------------------------------------------------------------------
//!
void
CubemapBaker::add( World* w, const Vec3f& p, Image* i )
{
   LockGuard g( _jobsLock );
   _jobs.pushBack( Job(w,p,i) );
}

//------------------------------------------------------------------------------
//!
bool
CubemapBaker::exec( double /*time*/, double /*delta*/ )
{
   if( _jobs.empty() )  return true;

   bool done = handle( _jobs.front() );
   if( done ) _jobs.popFront();

   return _jobs.empty();
}

//------------------------------------------------------------------------------
//!
bool
CubemapBaker::handle( Job& job )
{
   //StdErr << "handle: " << job._tmpl << ":" << job._slice << nl;
   Gfx::Manager* gfx = Core::gfx();

   if( job._fbo.isNull() )
   {
      job._fbo = gfx->createFramebuffer();
      job._fbo->setColorBuffer( gfx->create2DTexture( job._dim.x, job._dim.y, Gfx::TEX_FMT_8_8_8_8, Gfx::TEX_CHANS_RGBA, Gfx::TEX_FLAGS_RENDERABLE ) );
      job._fbo->setDepthBuffer( gfx->create2DTexture( job._dim.x, job._dim.y, Gfx::TEX_FMT_32     , Gfx::TEX_CHANS_Z   , Gfx::TEX_FLAGS_RENDERABLE ) );
      _renderer->size( job._dim );
   }

   String path;
   RCP<Camera> cam = new Camera( RigidBody::STATIC );
   cam->position( job._pos );
   // Note: our image origin is bottom-left.
   // Used orientations listed here:
   //   http://msdn.microsoft.com/en-us/library/windows/desktop/bb204881(v=vs.85).aspx
   // They should be identical to:
   //   http://www.opengl.org/registry/specs/ARB/texture_cube_map.txt
   switch( job._slice )
   {
      case Gfx::TEX_SLICE_NEG_X:
         cam->lookAt( cam->position() + Vec3f(-1.0f, 0.0f, 0.0f), Vec3f(0.0f,-1.0f, 0.0f) );
         path.format( job._tmpl.cstr(), "nx" );
         break;
      case Gfx::TEX_SLICE_POS_X:
         cam->lookAt( cam->position() + Vec3f( 1.0f, 0.0f, 0.0f), Vec3f(0.0f,-1.0f, 0.0f) );
         path.format( job._tmpl.cstr(), "px" );
         break;
      case Gfx::TEX_SLICE_NEG_Y:
         cam->lookAt( cam->position() + Vec3f( 0.0f,-1.0f, 0.0f), Vec3f(0.0f, 0.0f,-1.0f) );
         path.format( job._tmpl.cstr(), "ny" );
         break;
      case Gfx::TEX_SLICE_POS_Y:
         cam->lookAt( cam->position() + Vec3f( 0.0f, 1.0f, 0.0f), Vec3f(0.0f, 0.0f,+1.0f) );
         path.format( job._tmpl.cstr(), "py" );
         break;
      case Gfx::TEX_SLICE_NEG_Z:
         cam->lookAt( cam->position() + Vec3f( 0.0f, 0.0f,-1.0f), Vec3f(0.0f,-1.0f, 0.0f) );
         path.format( job._tmpl.cstr(), "nz" );
         break;
      case Gfx::TEX_SLICE_POS_Z:
         cam->lookAt( cam->position() + Vec3f( 0.0f, 0.0f, 1.0f), Vec3f(0.0f,-1.0f, 0.0f) );
         path.format( job._tmpl.cstr(), "pz" );
         break;
   }
   cam->fov( 90.0f );

   Viewport vp;
   vp.camera( cam.ptr() );
   vp.region( Vec2i(0,0), job._dim );
   vp.world( job._world.ptr() );

   // Prepare a render node.
   RCP<Gfx::RenderNode> rn = new Gfx::RenderNode();
   RCP<Gfx::Pass> pass = new Gfx::Pass();
   pass->setFramebuffer( job._fbo );
   rn->addPass( pass );

   // Queue world rendering.
   _renderer->beginFrame();
   _renderer->render( rn, job._world.ptr(), &vp );
   _renderer->endFrame();

   // Perform the actual rendering right now.
   gfx->render( rn );

   if( job._img.isValid() )
   {
      // Render to RAM.
      gfx->screenGrab( 0, 0, job._dim.x, job._dim.y, job._img->bitmap()->pixels(job._slice) );
   }
   else
   {
      // Render to disk.
      RCP<Bitmap> bmp = new Bitmap( job._dim, Bitmap::BYTE, 4 );
      gfx->screenGrab( 0, 0, job._dim.x, job._dim.y, bmp->pixels() );
      bmp->save( path );
   }

   ++job._slice;

   if( job._slice >= 6 )
   {
      StdErr << "Done rendering 6 faces at " << job._pos << nl;
      return true;
   }
   else
   {
      return false;
   }
}

//------------------------------------------------------------------------------
//!
String
CubemapBaker::makeTemplate( const String& path )
{
   String::SizeType pos = path.find( '*' );
   String::SizeType siz = 1;
   if( pos == String::npos )
   {
      // Assume an implicit '*' before the extension.
      siz = 0;
      pos = path.rfind( '.' );
      if( pos == String::npos )
      {
         // Assume an implicit '*' at the end.
         pos = path.size();
      }
   }
   String tmpl = path;
   tmpl.replace( pos, siz, "_%s" );
   return tmpl;
}

NAMESPACE_END
