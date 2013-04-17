/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Render/ForwardRenderer.h>
#include <Plasma/Renderable/Renderable.h>
#include <Plasma/Render/DebugGeometry.h>
#include <Plasma/World/World.h>
#include <Plasma/World/Light.h>
#include <Plasma/World/Selection.h>
#include <Plasma/World/Viewport.h>
#include <Plasma/World/ParticleEntity.h>
#include <Plasma/World/SkeletalEntity.h>
#include <Plasma/Geometry/ParticleGeometry.h>
#include <Plasma/Procedural/BoundaryPolygon.h>
#include <Plasma/Procedural/BSP3.h>
#include <Plasma/Plasma.h>

#include <Fusion/Core/Core.h>
#include <Fusion/Drawable/TQuad.h>
#include <Fusion/Resource/ResManager.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
AABBoxf computeLightViewBoundingBoxBSP( const Vec3f& cam, const Frustumf& f, const AABBoxf& box, const Mat4f& lm )
{
   // Compute polyhedron from the frustum.
   Vector< RCP<BoundaryPolygon> > polyFrustum;
   Vec3f pts[4];
   for( int i = 0; i < 4; ++i ) pts[i] = f.corner(i+4);

   RCP<BoundaryPolygon> p = new BoundaryPolygon();
   polyFrustum.pushBack( p );
   p->addVertex( cam );
   p->addVertex( pts[1] );
   p->addVertex( pts[3] );
   p->computeDerivedData();

   p = new BoundaryPolygon();
   polyFrustum.pushBack( p );
   p->addVertex( cam );
   p->addVertex( pts[3] );
   p->addVertex( pts[2] );
   p->computeDerivedData();

   p = new BoundaryPolygon();
   polyFrustum.pushBack( p );
   p->addVertex( cam );
   p->addVertex( pts[2] );
   p->addVertex( pts[0] );
   p->computeDerivedData();

   p = new BoundaryPolygon();
   polyFrustum.pushBack( p );
   p->addVertex( cam );
   p->addVertex( pts[0] );
   p->addVertex( pts[1] );
   p->computeDerivedData();

   p = new BoundaryPolygon();
   polyFrustum.pushBack( p );
   p->addVertex( pts[0] );
   p->addVertex( pts[2] );
   p->addVertex( pts[3] );
   p->addVertex( pts[1] );
   p->computeDerivedData();

   // Compute polyhedron from the world bb.
   Vector< RCP<BoundaryPolygon> > polyBox;
   for( int i = 0; i < 6; ++i )
      polyBox.pushBack( BoundaryPolygon::create( box, i, Mat4f::identity() ) );

   // Compute the intersection polyhedron.
   Vector< RCP<BoundaryPolygon> > polyInter;
   BSP3 bsp;
   bsp.build( polyFrustum );
   bsp.intersect( polyBox );
   bsp.computeBoundary( polyInter );

   // Compute the light space bb.
   AABBoxf ibox = AABBoxf::empty();
   for( auto cur = polyInter.begin(); cur != polyInter.end(); ++cur )
   {
      //StdErr << (**cur) << nl;
      for( auto v = (*cur)->begin(); v != (*cur)->end(); ++v )
      {
         ibox |= lm*(*v);
      }
   }

   // Extend volume to contain the min and max scene extend.
   float minz = CGConstf::infinity();
   float maxz = 0.0f;
   for( int i = 0; i < 8; ++i )
   {
      Vec3f p = lm*box.corner(i);
      if( minz > p.z ) minz = p.z;
      if( maxz < p.z ) maxz = p.z;
   }
   ibox.slabZ() = Vec2f( minz, maxz);

   return ibox;
}

#if 0
//------------------------------------------------------------------------------
//!
AABBoxf computeLightViewBoundingBox( const Frustumf& f, const AABBoxf& box, const Mat4f& lm )
{
   AABBoxf fbox     = AABBoxf::empty();
   AABBoxf wbox     = AABBoxf::empty();
   for( int i = 0; i < 8; ++i ) fbox |= lm*f.corner(i);
   for( int i = 0; i < 8; ++i ) wbox |= lm*box.corner(i);
   return fbox & wbox;
}
#endif

//------------------------------------------------------------------------------
//!
void computeLightMatrices(
   const Light&    l,
   const Viewport& vp,
   const AABBoxf&  box,
   Mat4f&          projMat,
   Mat4f&          viewMat
)
{
   if( l.shape() != Light::DIRECTIONAL || box.isEmpty() )
   {
      projMat = l.projectionMatrix();
      viewMat = l.viewMatrix();
      return;
   }

   projMat = l.projectionMatrix();
   viewMat = l.viewMatrix();


   // Compute basic info.
   Vec3f camPos = vp.camera()->position();
   Vec3f ldir   = -l.orientation().getAxisZ();
   Vec3f edir   = -vp.camera()->orientation().getAxisZ();

   // Compute starting light matrices.
   Mat4f view = l.viewMatrix();
   //Mat4f proj = l.projectionMatrix();
   //Reff lref    = Reff::lookAt( camPos, camPos+ldir, edir );
   //Mat4f view   = lref.localToGlobal();
   Mat4f proj   = Mat4f::identity();

   // Switch to lispsm space.
   // Mat4f toLispsm(
   //    1.0f, 0.0f,  0.0f, 0.0f,
   //    0.0f, 0.0f, -1.0f, 0.0f,
   //    0.0f, 1.0f,  0.0f, 0.0f,
   //    0.0f, 0.0f,  0.0f, 1.0f
   // );

   //proj     = toLispsm*proj;
   Mat4f lm = proj*view;

   // Compute fitting bounding box defined by the intersection of the frustum and
   // the world in light space.
   Frustumf frustum = vp.frustum();
   AABBoxf lbox     = computeLightViewBoundingBoxBSP( camPos, frustum, box, lm );
   //AABBoxf lbox0    = computeLightViewBoundingBox( frustum, box, lm );

   //StdErr << "box0=" << lbox0 << nl;
   //StdErr << "box1=" << lbox << nl;

   // LiSPSM...


   // Fitting.
   Mat4f pl  = proj*view;
   Mat4f fit = Mat4f::fitting( lbox.corner(0), lbox.corner(7) );
   proj      = fit*proj;

   // Switch from lispsm space.
   // Mat4f fromLispsm(
   //    1.0f,  0.0f, 0.0f, 0.0f,
   //    0.0f,  0.0f, 1.0f, 0.0f,
   //    0.0f, -1.0f, 0.0f, 0.0f,
   //    0.0f,  0.0f, 0.0f, 1.0f
   // );

   //StdErr << "projMat=" << projMat << nl;
   //StdErr << "proj=   " << proj << nl;

   //proj = fromLispsm*proj;
   proj = Mat4f::scaling( 1.0f, 1.0f, -1.0f )*proj;

   Mat4f lightMat    = projMat*viewMat;
   Mat4f newLightMat = proj*view;

   // Mat4f lmi = lm.getInversed();
   // StdErr << "box=" << box << nl;
   // StdErr << "cam=" << camPos << nl;
   // StdErr << "ldir=" << ldir << nl;
   // for( int i = 0; i < 8; ++i ) StdErr << "f" << i << "=" << frustum.corner(i) << nl;
   // StdErr << "dist=" << frustum.plane(4).distance( frustum.corner(0) ) << nl;
   // for( int i = 0; i < 8; ++i )
   // {
   //    Vec3f c = lmi*ibox.corner(i);
   //    StdErr << i << " c=" << c << " cb=" << lightMat*c << " ca=" << newLightMat*c << nl;
   // }
   // Vec3f c(0.0f);
   // StdErr << c << " cb=" << lightMat*c << " ca=" << newLightMat*c << nl;
   // c = Vec3f(0.0f,1.0f,0.0f);
   // StdErr << c << " cb=" << lightMat*c << " ca=" << newLightMat*c << nl;

   projMat = proj;
   viewMat = view;
}

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
  CLASS ForwardRenderer
==============================================================================*/

//------------------------------------------------------------------------------
//!
ForwardRenderer::ForwardRenderer()
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
   _noDepthWrite  = new Gfx::DepthState();
   _noDepthWrite->depthTesting( true );
   _noDepthWrite->depthWriting( false );

   _defOffset  = new Gfx::OffsetState();
   _offset     = new Gfx::OffsetState();
   _offset->constant( 10.0f );

   _clampBilinear.setBilinear();
   _clampBilinear.clamp( Gfx::TEX_CLAMP_LAST );

   _clampNearest.setPointSampling();
   _clampNearest.clamp( Gfx::TEX_CLAMP_LAST );

   // Allocating programs.
   _colorDirProg     = data( ResManager::getProgram( "shader/program/forward/color1_dir" ) );
   _colorDirSkelProg = data( ResManager::getProgram( "shader/program/forward/color1_dir_skel" ) );
   _colorDirPartProg = data( ResManager::getProgram( "shader/program/particle/psc" ) );
   _selectionProg    = data( ResManager::getProgram( "shader/program/color" ) );
   _depthDirProg     = data( ResManager::getProgram( "shader/program/forward/depth1_dir" ) );
   _depthDirSkelProg = data( ResManager::getProgram( "shader/program/forward/depth1_dir_skel" ) );

   // Allocation constants.
   Vec3f def( 0.0f );
   Mat4f defm = Mat4f::identity();
   _lightConstants = Core::gfx()->createConstants( (6+16)*sizeof(float) );
   _lightConstants->addConstant( "light_intensity", Gfx::CONST_FLOAT3, 0, def.ptr() );
   _lightConstants->addConstant( "light_direction", Gfx::CONST_FLOAT3, 3*sizeof(float), def.ptr() );
   _lightConstants->addConstant( "light_matrix",    Gfx::CONST_MAT4,   6*sizeof(float), defm.ptr() );
}

//------------------------------------------------------------------------------
//!
ForwardRenderer::~ForwardRenderer()
{
}

//------------------------------------------------------------------------------
//!
void
ForwardRenderer::render(
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

   // Classify entities for rendering. We are looking for visible, ghost and
   // reflection.
   classifyEntities( _world );

   // Rendering.
   Gfx::RenderNode* renderNode = getRenderNode();
   Gfx::Pass* curPass          = rn->current().ptr();
   curPass->render( renderNode );

   // Create light pass.
   renderLightPass( *renderNode, *vp );

   // Create main rendering pass.
   Gfx::Pass* pass = getPass();
   pass->setFramebuffer( curPass->framebuffer() );
   renderNode->addPass( pass );

   renderColorPass( *pass, *vp );

   renderBuffers( rn, *vp );
}

//------------------------------------------------------------------------------
//!
void ForwardRenderer::performResize()
{
}

//------------------------------------------------------------------------------
//!
void ForwardRenderer::performBeginFrame()
{
}

//------------------------------------------------------------------------------
//!
void ForwardRenderer::performEndFrame()
{
}

//------------------------------------------------------------------------------
//!
void ForwardRenderer::renderLightPass( Gfx::RenderNode& rn, const Viewport& vp )
{
#define ESM
   // Find main light.
   if( _world->numLights() == 0 ) return;
   Light* l = _world->light(0);

   // Update constants buffer.
   Vec3f dir = l->orientation().getAxisZ();

   _lightConstants->setConstant( 0, l->intensity().ptr() );
   _lightConstants->setConstant( 1, dir.ptr() );

   // Do we have a shadow map to setup/render?
   if( !l->castsShadows() ) return;

   // Allocate buffer if non-existant.
   if( _depthBuffer.isNull() )
   {
      _depthTex = Core::gfx()->create2DTexture(
         //512, 512, Gfx::TEX_FMT_24_8, Gfx::TEX_CHANS_ZS, Gfx::TEX_FLAGS_RENDERABLE
         //512, 512, Gfx::TEX_FMT_16, Gfx::TEX_CHANS_Z, Gfx::TEX_FLAGS_RENDERABLE
         //128, 128, Gfx::TEX_FMT_32, Gfx::TEX_CHANS_Z, Gfx::TEX_FLAGS_RENDERABLE
         //256, 256, Gfx::TEX_FMT_32, Gfx::TEX_CHANS_Z, Gfx::TEX_FLAGS_RENDERABLE
         512, 512, Gfx::TEX_FMT_32, Gfx::TEX_CHANS_Z, Gfx::TEX_FLAGS_RENDERABLE
         //640, 640, Gfx::TEX_FMT_32, Gfx::TEX_CHANS_Z, Gfx::TEX_FLAGS_RENDERABLE
         //1024, 1024, Gfx::TEX_FMT_32, Gfx::TEX_CHANS_Z, Gfx::TEX_FLAGS_RENDERABLE
      );
      for( size_t i = 0; i < sizeof(_depthCol)/sizeof(_depthCol[0]); ++i )
      {
         _depthCol[i] = Core::gfx()->create2DTexture(
            //_depthTex->width(), _depthTex->height(), Gfx::TEX_FMT_16F_16F_16F_16F, Gfx::TEX_CHANS_RGBA, Gfx::TEX_FLAGS_RENDERABLE
            //_depthTex->width(), _depthTex->height(), Gfx::TEX_FMT_16F_16F, Gfx::TEX_CHANS_RG, Gfx::TEX_FLAGS_RENDERABLE
            _depthTex->width(), _depthTex->height(), Gfx::TEX_FMT_16F, Gfx::TEX_CHANS_R, Gfx::TEX_FLAGS_RENDERABLE
         );
         _depthCol[i]->updateDefinedRange( _depthTex->width(), _depthTex->height() );
         _depthCol[i]->updateDefinedRange( 0, 0 );
      }
      _depthBuffer = Core::gfx()->createFramebuffer();
#ifdef ESM
      _depthBuffer->setColorBuffer( _depthCol[0] );
#endif
      _depthBuffer->setDepthBuffer( _depthTex );

      // Sampler for shading.
      const RCP<Gfx::Texture>& tex = _depthBuffer->hasColorBuffer() ? _depthCol[0] : _depthTex;
#ifdef ESM
      _lightSampler = new Gfx::Sampler( "light_tex", tex, _clampBilinear );
#else
      _lightSampler = new Gfx::Sampler( "light_tex", tex, _clampNearest );
#endif

#ifdef ESM
      _blur = new Gfx::RenderNode();

#if 0
      String progName[2] = {
         "shader/program/f_gauss9x9_h",
         "shader/program/f_gauss9x9_v"
      };
      Vec2f disp[2] = {
         Vec2f( 1.386283f, 3.253459f ) / (float)_depthTex->width(),
         Vec2f( 1.386283f, 3.253459f ) / (float)_depthTex->height()
      };
#elif 0
      // Weights: 1 8 28 56 70 56 28 8 1 (=256)
      // Taps   :  x    x    x   x    x
      // These are different than the above.
      String progName[2] = {
         "shader/program/f_gauss9x9b_h",
         "shader/program/f_gauss9x9b_v"
      };
      Vec2f disp[2] = {
         Vec2f( 1.0f+28.0f/(56.0f+28.0f), 3.0f+1.0f/(8.0f+1.0f) ) / (float)_depthTex->width(),
         Vec2f( 1.0f+28.0f/(56.0f+28.0f), 3.0f+1.0f/(8.0f+1.0f) ) / (float)_depthTex->height()
      };
#elif 0
      // Weights: 1 7 21 35 35 21 7 1 (=128)
      // Taps   :  x    x     x    x
      String progName[2] = {
         "shader/program/f_gauss8x8_h",
         "shader/program/f_gauss8x8_v"
      };
      Vec2f disp[2] = {
         Vec2f( 0.5f+21.0f/(35.0f+21.0f), 2.5f+1.0f/(7.0f+1.0f) ) / (float)_depthTex->width(),
         Vec2f( 0.5f+21.0f/(35.0f+21.0f), 2.5f+1.0f/(7.0f+1.0f) ) / (float)_depthTex->height()
      };
#elif 0
      // Weights: 1 6 15 20 15 6 1 (=64)
      // Taps   :  x    x  x    x
      String progName[2] = {
         "shader/program/f_gauss7x7_h",
         "shader/program/f_gauss7x7_v"
      };
      Vec2f disp[2] = {
         Vec2f( 0.0f+(15.0f)/(10.0f+15.0f),  2.0f+(1.0f)/(6.0f+1.0f) ) / (float)_depthTex->width(),
         Vec2f( 0.0f+(15.0f)/(10.0f+15.0f),  2.0f+(1.0f)/(6.0f+1.0f) ) / (float)_depthTex->height()
      };
#elif 0
      // Weights: 1 5 10 10 5 1 (=32)
      // Taps   :  x    x    x
      String progName[2] = {
         "shader/program/f_gauss6x6_h",
         "shader/program/f_gauss6x6_v"
      };
      // Note: Shader handles center sample specially.
      Vec2f disp[2] = {
         Vec2f( 1.5f+(1.0f)/(5.0f+1.0f) ) / (float)_depthTex->width(),
         Vec2f( 1.5f+(1.0f)/(5.0f+1.0f) ) / (float)_depthTex->height()
      };
#elif 1
      // Weights: 1 4 6 4 1 (=16)
      // Taps   :  x  x  x
      String progName[2] = {
         "shader/program/f_gauss5x5_h",
         "shader/program/f_gauss5x5_v"
      };
      Vec2f disp[2] = {
         Vec2f( 1.0f+1.0f/(4.0f+1.0f) ) / (float)_depthTex->width(),
         Vec2f( 1.0f+1.0f/(4.0f+1.0f) ) / (float)_depthTex->height()
      };
#elif 0
      // Weights: 1 3 3 1 (pseudo)
      // Taps   :  x   x
      String progName[2] = {
         "shader/program/f_gauss4x4_h",
         "shader/program/f_gauss4x4_v"
      };
      // Note: Shader has implicit 0.5f weighting of each sample.
      Vec2f disp[2] = {
         Vec2f( 0.5f+1.0f/(1.0f+3.0f) ) / (float)_depthTex->width(),
         Vec2f( 0.5f+1.0f/(1.0f+3.0f) ) / (float)_depthTex->height()
      };
#elif 0
      // Weights: 1 2 1
      // Taps   :  x x
      String progName[2] = {
         "shader/program/f_gauss3x3_h",
         "shader/program/f_gauss3x3_v"
      };
      // Note: Shader has implicit 0.5f weighting of each sample.
      Vec2f disp[2] = {
         Vec2f( 0.5f ) / (float)_depthTex->width(),
         Vec2f( 0.5f ) / (float)_depthTex->height()
      };
#else
      // Weights: 1 3 1
      // Taps   :  x x
      String progName[2] = {
         "shader/program/f_gauss3x3_h",
         "shader/program/f_gauss3x3_v"
      };
      // Note: Shader has implicit 0.5f weighting of each sample.
      Vec2f disp[2] = {
         Vec2f( 0.4f ) / (float)_depthTex->width(),
         Vec2f( 0.4f ) / (float)_depthTex->height()
      };
#endif

      // Horizontal, then vertical.
      for( uint i = 0; i < 2; ++i )
      {
         RCP<Gfx::Program> prog = data( ResManager::getProgram( progName[i] ) );
         RCP<Gfx::SamplerList> smpl( new Gfx::SamplerList() );
         smpl->addSampler( "tex", _depthCol[i], _clampBilinear );
         RCP<Gfx::ConstantBuffer> cnst = Core::gfx()->createConstants( prog );
         cnst->setConstant( "disp", disp[i].ptr() );
         _blur->addPass( Core::gfx()->createFilterPass( smpl, prog, Gfx::ConstantList::create(cnst), _depthCol[1-i] ) );
      }
#endif
   }

   // Render shadow map.

   // 1. Allocate buffer
   Gfx::Pass* pass = getPass();
   rn.addPass( pass );
   pass->setFramebuffer( _depthBuffer );
   int clr = Gfx::CLEAR_DEPTH;
   if( _depthBuffer->hasColorBuffer() ) clr |= Gfx::CLEAR_COLOR;
   pass->setClear( Gfx::ClearType(clr) );

   // 2. Compute projection matrix.
   Mat4f projMat;
   Mat4f viewMat;
   computeLightMatrices( *l, vp, _world->boundingBox(), projMat, viewMat );

   _lightConstants->setConstant( 2, (l->viewportMatrix()*projMat*viewMat).ptr() );

   // 3. Setting default states.
   pass->setProjectionMatrix( projMat.ptr() );
   pass->setViewMatrix( viewMat.ptr() );
   pass->setAlphaState( _noBlending );
   if( !_depthBuffer->hasColorBuffer() ) pass->setColorState( _noColor );

   // 4. Render geometry.
   renderDepth( *pass );

   // 5. Blur the color depth texture.
   if( _blur.isValid() ) pass->render( _blur );
}

//------------------------------------------------------------------------------
//!
void
ForwardRenderer::renderColorPass( Gfx::Pass& pass, const Viewport& vp )
{
   // Setting buffer.
   const Vec4f& bgColor = _world->backgroundColor();
   pass.setClearColor( bgColor.x, bgColor.y, bgColor.z, bgColor.w );

   if( clearDepth() )
      pass.setClear( Gfx::CLEAR_DEPTH );
   else
      pass.setClear( Gfx::CLEAR_NONE );

   // Setting default states.
   pass.setAlphaState( _noBlending );

   _viewMat = vp.viewMatrix();
   _projMat = vp.projectionMatrix();

   pass.setProjectionMatrixPtr( _projMat.ptr() );
   pass.setViewMatrixPtr( _viewMat.ptr() );
   pass.setCamPositionPtr( vp.camera()->position().ptr() );
   pass.setViewport( (int)vp.position().x, (int)vp.position().y, (int)vp.size().x, (int)vp.size().y );
   pass.setScissor( (int)vp.position().x, (int)vp.position().y, (int)vp.size().x, (int)vp.size().y );

   // Setting program.
   pass.setSamplers(0);
   pass.setConstants(0);
   pass.setProgram( _colorDirProg );

   // Rendering passes.
   renderMirrors( pass );
   renderMain( pass );
   renderGhosts( pass );
   renderSelections( pass, vp );
   renderRenderable( pass, vp );
}

//------------------------------------------------------------------------------
//!
void
ForwardRenderer::renderMirrors( Gfx::Pass& pass )
{
   // FIXME: stenciling.

   if( _reflections.empty() ) return;

   // Render reflections.
   pass.setCullState( _frontCulling );
   for( auto cur = _reflections.begin(); cur != _reflections.end(); ++cur )
   {
      ReflectivePlanarMaterial* rmat = (ReflectivePlanarMaterial*)cur->_mat;
      Mat4f rm   = Mat4f::reflection( (*cur->_trf) * rmat->normal(), (*cur->_trf) * rmat->position() );
      pass.setViewMatrix( (_viewMat*rm).ptr() );

      renderMain( pass );
   }

   pass.setViewMatrixPtr( _viewMat.ptr() );
   pass.setCullState( _backCulling );
   pass.setAlphaState( _defBlending );
   // Render reflectors.
   for( auto cur = _reflections.begin(); cur != _reflections.end(); ++cur )
   {
      Geometry* geom  = cur->_geom;
      Material* mat   = cur->_mat;

      // Set position.
      pass.setWorldMatrixPtr( (const float*)cur->_trf );

      pass.setSamplers( mat->samplers() );
      pass.setConstants( mat->constants() );
      geom->renderPatch( pass, cur->_patchId );
   }
   pass.setAlphaState( _noBlending );
}

//------------------------------------------------------------------------------
//!
void
ForwardRenderer::renderMain( Gfx::Pass& pass )
{
   // Render entities.
   for( auto cur = _visibles.begin(); cur != _visibles.end(); ++cur )
   {
      Entity* e      = cur->_entity;
      Geometry* geom = cur->_geom;
      Material* mat  = cur->_mat;

      // Set position.
      pass.setWorldMatrixPtr( (const float*)cur->_trf );

      // Textures.
      if( _lightSampler.isValid() )
      {
         Gfx::SamplerList* samplers = getSamplerList();
         samplers->addSamplers( mat->samplers() );
         samplers->addSampler( _lightSampler );
         pass.setSamplers( samplers );
      }
      else
         pass.setSamplers( mat->samplers() );

      switch( cur->_type )
      {
         case STANDARD:
         {
            Gfx::ConstantList* constants = getConstantList();
            constants->addBuffer( (*mat->constants())[0] );
            constants->addBuffer( _lightConstants );

            pass.setProgram( _colorDirProg );
            pass.setConstants( constants );
         }  break;
         case CUSTOM:
         {
            Gfx::ConstantList* constants = getConstantList();
            constants->addBuffer( (*mat->constants())[0] );
            constants->addBuffer( _lightConstants );

            pass.setProgram( ((CustomMaterial*)mat)->program() );
            pass.setConstants( constants );
            // TEMP
            pass.setAlphaState( _defBlending );
         }  break;
         case STANDARD_SKINNED:
         {
            SkeletalEntity* se           = (SkeletalEntity*)e;
            Gfx::ConstantList* constants = getConstantList();
            constants->addBuffer( (*mat->constants())[0] );
            constants->addBuffer( se->boneConstants() );
            constants->addBuffer( _lightConstants );

            pass.setProgram( _colorDirSkelProg );
            pass.setConstants( constants );
         }  break;
         case CUSTOM_SKINNED:
         {
            SkeletalEntity* se           = (SkeletalEntity*)e;
            Gfx::ConstantList* constants = getConstantList();
            constants->addBuffer( (*mat->constants())[0] );
            constants->addBuffer( se->boneConstants() );
            constants->addBuffer( _lightConstants );

            pass.setProgram( ((CustomMaterial*)mat)->program() );
            pass.setConstants( constants );
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
ForwardRenderer::renderDepth( Gfx::Pass& pass )
{
   // TODO: change program...
   // Render entities.
   for( auto cur = _visibles.begin(); cur != _visibles.end(); ++cur )
   {
      Entity* e      = cur->_entity;
      Geometry* geom = cur->_geom;
      Material* mat  = cur->_mat;

      if( !e->castsShadows() ) return;

      // Set position.
      pass.setWorldMatrixPtr( (const float*)cur->_trf );

      switch( cur->_type )
      {
         case STANDARD:
         {
            pass.setProgram( _depthDirProg );
            pass.setSamplers( nullptr );
            pass.setConstants( nullptr );
         }  break;
         case CUSTOM:
         {
            continue;
            //CHECK( false ); // Unsupported for now.
            //Gfx::ConstantList* constants = getConstantList();
            //constants->addBuffer( (*mat->constants())[0] );
            //
            //pass.setProgram( ((CustomMaterial*)mat)->program() );
            //pass.setSamplers( nullptr );
            //pass.setConstants( constants );
         }  break;
         case STANDARD_SKINNED:
         {
            SkeletalEntity* se           = (SkeletalEntity*)e;
            Gfx::ConstantList* constants = getConstantList();
            constants->addBuffer( se->boneConstants() );

            pass.setProgram( _depthDirSkelProg );
            pass.setConstants( constants );
            pass.setSamplers( nullptr );
         }  break;
         case CUSTOM_SKINNED:
         {
            continue;
            CHECK( false ); // Unsupported for now.
            SkeletalEntity* se           = (SkeletalEntity*)e;
            Gfx::ConstantList* constants = getConstantList();
            constants->addBuffer( (*mat->constants())[0] );
            constants->addBuffer( se->boneConstants() );

            pass.setProgram( ((CustomMaterial*)mat)->program() );
            pass.setConstants( constants );
            pass.setSamplers( nullptr );
         }  break;
         default:;
      }
      // Geometry.
      geom->renderPatch( pass, cur->_patchId );
   }
}

//------------------------------------------------------------------------------
//!
void
ForwardRenderer::renderParticles( Gfx::Pass& pass )
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
ForwardRenderer::renderGhosts( Gfx::Pass& pass )
{
   if( _ghosts.empty() ) return;

   for( int i = 0; i < 2; ++i )
   {
      // Render depth.
      if( i == 0 )
      {
         pass.setColorState( _noColor );
         pass.setOffsetState( _offset );
      }
      // Render color.
      else
      {
         pass.setColorState( _defColor );
         pass.setAlphaState( _defBlending );
         pass.setDepthState( _eqDepth );
      }

      for( auto cur = _ghosts.begin(); cur != _ghosts.end(); ++cur )
      {
         Entity* e      = cur->_entity;
         Geometry* geom = cur->_geom;
         Material* mat  = cur->_mat;

         pass.setWorldMatrixPtr( (const float*)cur->_trf );
         pass.setSamplers( mat->samplers() );

         switch( cur->_type )
         {
            case GHOST:
            {
               Gfx::ConstantList* constants = getConstantList();
               constants->addBuffer( (*mat->constants())[0] );
               constants->addBuffer( _lightConstants );

               pass.setProgram( _colorDirProg );
               pass.setConstants( constants );
            }  break;
            case GHOST_SKINNED:
            {
               SkeletalEntity* se           = (SkeletalEntity*)e;
               Gfx::ConstantList* constants = getConstantList();
               constants->addBuffer( (*mat->constants())[0] );
               constants->addBuffer( se->boneConstants() );
               constants->addBuffer( _lightConstants );

               pass.setProgram( _colorDirSkelProg );
               pass.setConstants( constants );
            }  break;
            default:;
         }
         geom->renderPatch( pass, cur->_patchId );
      }
   }
   pass.setDepthState( _defDepth );
   pass.setOffsetState( _defOffset );
}

//------------------------------------------------------------------------------
//!
void
ForwardRenderer::renderSelections( Gfx::Pass& pass, const Viewport& vp )
{
   // Render selection.
   if( _world->numSelections() > 0 )
   {
      Vec3f camPos = vp.camera()->position();
      // Set selection rendering states.
      pass.setCullState( _frontCulling );
      pass.setProgram( _selectionProg );
      pass.setSamplers(0);
      pass.setConstants(0);

      for( uint i = 0; i < _world->numSelections(); ++i )
      {
         Entity* e      = _world->selection(i)->entity();
         Geometry* geom = e->geometry();

         // Do we have a geometry to render?
         if( !geom ) continue;

         // Set position.
         //pass->setWorldMatrixPtr( e->transform().ptr() );
         Mat4f tm = Mat4f::translation( normalize(camPos-e->position())*0.1f );
         pass.setWorldMatrix( (tm * e->transform()).ptr() );

         // Rendering geometry.
         geom->render( pass );
      }
      pass.setCullState( _backCulling );
   }
}

//------------------------------------------------------------------------------
//!
void ForwardRenderer::renderRenderable( Gfx::Pass& pass, const Viewport& vp )
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
      _world->debugGeometry()->render( pass );
   }
}

//------------------------------------------------------------------------------
//!
void ForwardRenderer::renderBuffers( const RCP<Gfx::RenderNode>& rn, const Viewport& vp )
{
   return;
   if( _depthQuad.isNull() )
   {
      if( _depthCol[0].isNull() ) return;
      RCP<Image> img( new Image(_depthCol[0].ptr()) );
      _depthQuad = new TQuad();
      _depthQuad->position( vp.position() + Vec2f( 10.0f, vp.size().y - 286.0f ) );
      _depthQuad->size( Vec2f(256.0f) );
      _depthQuad->color( Vec4f(2.0f, 2.0f, 2.0f, 1.0f ) );
      _depthQuad->u( Vec4f(0.0f, 1.0f, 0.0f, 0.0f ) );
      _depthQuad->v( Vec4f(0.0f, 1.0f, 0.0f, 0.0f ) );
      _depthQuad->type( TQuad::NORMAL );
      _depthQuad->image( img.ptr() );
   }
   _depthQuad->draw( rn );
}

NAMESPACE_END
