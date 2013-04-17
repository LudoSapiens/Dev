/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Gfx/Mgr/Manager.h>
#include <Gfx/Mgr/Context.h>

#include <Base/Dbg/DebugStream.h>

#include <CGMath/Mat4.h>

#include <cassert>


USING_NAMESPACE

using namespace Gfx;


UNNAMESPACE_BEGIN

DBG_STREAM( os_mgr, "Manager" );

UNNAMESPACE_END

//------------------------------------------------------------------------------
//!
Manager::Manager( Context* context, const String& api ):
   _context( context ), _api( api ), _width(0), _height(0), _curWidth(0), _curHeight(0), _doingRTT(false)
{
   DBG_BLOCK( os_mgr, "Manager::Manager" );
}

//------------------------------------------------------------------------------
//!
Manager::~Manager
( void )
{
   DBG_BLOCK( os_mgr, "Manager::~Manager" );
}

//------------------------------------------------------------------------------
//!
void
Manager::printInfo( TextStream& os ) const
{
   os << "API: " << API() << nl;
}

//------------------------------------------------------------------------------
//!
void 
Manager::setSize( uint width, uint height )
{
   _width  = width;
   _height = height;
}

//------------------------------------------------------------------------------
//!
void 
Manager::getSize( uint& width, uint& height )
{
   width  = _width;
   height = _height;
}

//------------------------------------------------------------------------------
//!
RCP<Manager>
Manager::create( Context* context )
{
   if( context )  return context->createManager();
   else           return Context::getDefaultContext()->createManager();
}

//------------------------------------------------------------------------------
//!
bool
Manager::screenGrab( uint /*x*/, uint /*y*/, uint /*w*/, uint /*h*/, void* /*data*/ )
{
   return false;
}

// RenderNode.
//------------------------------------------------------------------------------
//!
bool
Manager::render( 
   const RCP<RenderNode>&  rn,
   Set< RCP<RenderNode> >& jobsDone,
   Set< RCP<RenderNode> >& nodesVisited
)
{
   bool ok = true;

   // Check for infinite loops
   if( nodesVisited.has(rn) )
   {
      printf("Infinite loop detected in render tree.  Aborting.\n");
      return false;
   }
   else
   {
      nodesVisited.add(rn);
   }

   // Do all requirements first
   uint n = rn->nbRequirements();
   for( uint i = 0; i < n; ++i )
   {
      const RCP<RenderNode>& req = rn->getRequirement(i);
      if( !jobsDone.has(req) )
      {
         ok &= render(req, jobsDone, nodesVisited);
         if( ok )
         {
            jobsDone.add(req);
         }
         else
         {
            return false;  // early termination on error
         }
      }
      // else, we already did the job
   }

   // Execute every pass, now that requirements are done
   n = rn->numPasses();
   for( uint i = 0; i < n; ++i )
   {
      const RCP<Pass>& pass = rn->getPass(i);
      ok &= render( *(pass.ptr()) );
   }

   return ok;
}


// Filters.
//------------------------------------------------------------------------------
//! This routine is basically the same as the other setFilterPass routine except
//! that it will generate a point-sampled base-map-only sampler list using the
//! specified texture.  The shader program must reference it as 'tex'.
//! This routine is meant to simplify coding, and is simply a wrapper over the
//! other setFilterPass routine.
RCP<Pass>
Manager::createFilterPass( 
   const RCP<Texture>& srcTex, 
   const RCP<Program>& srcPgm,
   const RCP<Texture>& dstTex, 
   const uint          dstLvl, 
   const bool          genMipmaps
)
{
   // Set basemap point-sampling texture called 'tex'
   RCP<SamplerList> sl( new SamplerList() );
   TextureState tState;
   tState.setPointSampling();
   
   //OSX fallbacks in software renderer if unsupported clamp policy is used
   tState.clampX( TEX_CLAMP_LAST );
   tState.clampY( TEX_CLAMP_LAST );
   tState.mipFilter( TEX_FILTER_NONE );
   sl->addSampler( "tex", srcTex, tState );

   return  createFilterPass( sl, srcPgm, NULL, dstTex, dstLvl, genMipmaps );
}

//------------------------------------------------------------------------------
//!
RCP<Pass>
Manager::createFilterPass(
   const RCP<Texture>&      srcTex,
   const RCP<Program>&      srcPgm,
   const RCP<ConstantList>& srcConst,
   const RCP<Texture>&      dstTex,
   const uint               dstLvl,
   const bool               genMipmaps
)
{
   // Set basemap point-sampling texture called 'tex'
   RCP<SamplerList> sl( new SamplerList() );
   TextureState tState;
   tState.setPointSampling();
   
   //OSX fallbacks in software renderer if unsupported clamp policy is used
   tState.clampX( TEX_CLAMP_LAST );
   tState.clampY( TEX_CLAMP_LAST );
   tState.mipFilter( TEX_FILTER_NONE );
   sl->addSampler( "tex", srcTex, tState );

   return  createFilterPass( sl, srcPgm, srcConst, dstTex, dstLvl, genMipmaps );
}

//------------------------------------------------------------------------------
//!
RCP<Pass>
Manager::createFilterPass( 
   const RCP<SamplerList>& srcSampList,
   const RCP<Program>&     srcPgm,
   const RCP<Texture>&     dstTex,
   const uint              dstLvl,
   const bool              genMipmaps
)
{
   return  createFilterPass( srcSampList, srcPgm, NULL, dstTex, dstLvl, genMipmaps );
}

//------------------------------------------------------------------------------
//! Utility routine setting various typical values for post-processing passes.
//! The specified paramaters all share the fact that they are required for
//! proper execution, granted that:
//!  - The sampler list must be consistent with the program used
//!  - A NULL sampler list indicates a generation pass (pixels are created from nothing)
//!  - The source program must always be valid
//!  - A NULL destination texture indicates to store the result in the back buffer
//!  - A NULL destination pass means that the routine will perform the required allocation
//! Result:
//!  - The destination pass will contain all of the rendering command except for
//!    the 'exec' command, which allow the user to add extra stuff (such as constants).
RCP<Pass>
Manager::createFilterPass( 
   const RCP<SamplerList>&  srcSampList, 
   const RCP<Program>&      srcPgm, 
   const RCP<ConstantList>& srcConst,
   const RCP<Texture>&      dstTex, 
   const uint               dstLvl,
   const bool               genMipmaps
)
{
   DBG_BLOCK( os_mgr, "Manager::createFilterPass()" );

   // Allocate destination storage
   RCP<Pass> dstPass( new Pass() );

   // Disable alpha-blending
   RCP<AlphaState> aState( new AlphaState() );
   aState->alphaBlending(false);
   aState->alphaTesting(false); //should be default anyways
   dstPass->setAlphaState(aState);
   
   // Disable depth testing
   RCP<DepthState> dState( new DepthState() );
   dState->depthTesting(false);
   dState->depthWriting(false);
   dstPass->setDepthState(dState);

   // Set destination framebuffer
   uint w, h;
   if( dstTex.isValid() )
   {
      RCP<Framebuffer> fb = createFramebuffer();
      fb->setColorBuffer( dstTex, dstLvl );
      dstPass->setFramebuffer( fb );
      dstPass->mipmaps( genMipmaps );
      w = fb->width();
      h = fb->height();
   }
   else
   {
      // Use back buffer
      getSize(w, h);
   }

   // Set samplers
   if( srcSampList.isValid() )
   {
      dstPass->setSamplers(srcSampList);
   }

   // Set program
   if( srcPgm.isValid() )
   {
      dstPass->setProgram(srcPgm);
   }
   else
   {
      DBG_MSG( os_mgr, "NULL program." );
      return NULL;
   }

   // Set constants
   if( srcConst.isValid() )
   {
      dstPass->setConstants(srcConst);
   }

   // Set viewport
   dstPass->setViewport(0, 0, w, h);

   // Set view matrix
   if( oneToOneOffset() != 0 )
   {
      Mat4f view = Mat4f::translation(-1.0f/w, -1.0f/h, 0.0f);
      dstPass->setViewMatrix(view.ptr());
   }

   // Set geometry
   dstPass->execGeometry( getOneToOneGeometry() );

   return dstPass;
}

//------------------------------------------------------------------------------
//!
const RCP<Geometry>&
Manager::getOneToOneGeometry()
{
   if( _oneToOneGeom.isValid() )
   {
      // Simply return (at the end)
   }
   else
   {
#if 0
      /**
       *  (0, 1)   (1, 1)
       *       D---C
       *       |  /|
       *       | / |
       *       |/  |
       *       A---B
       *  (0, 0)   (1, 0)
       */

      _oneToOneGeom = createGeometry(PRIM_TRIANGLE_FAN);

      // Index buffer
      ushort indices[] = {
         0, 1, 2, 3
      };
      RCP<IndexBuffer> iBuffer = createBuffer( INDEX_FMT_16, BUFFER_FLAGS_NONE, 4*sizeof(indices[0]), indices );
      _oneToOneGeom->indexBuffer(iBuffer);

      // Vertex buffer
      float vertices[4*2];
      vertices[0] = -1.0f; // X
      vertices[1] = -1.0f; // Y

      vertices[2] =  1.0f; // X
      vertices[3] = -1.0f; // Y

      vertices[4] =  1.0f; // X
      vertices[5] =  1.0f; // Y

      vertices[6] = -1.0f; // X
      vertices[7] =  1.0f; // Y

      RCP<VertexBuffer> vBuffer = createBuffer( BUFFER_FLAGS_NONE, 4*2*sizeof(float), vertices );
      vBuffer->addAttribute(ATTRIB_TYPE_POSITION , ATTRIB_FMT_32F_32F, vBuffer->strideInBytes());
      _oneToOneGeom->addBuffer(vBuffer);
#else

      /**
       *   (0,1)  [1,1]   (2,1)
       *       C----+----B
       *       |    |  /
       *       |    |/
       *  [0,0]|----+[1,0]
       *       |  /
       *       |/
       *       A
       *  (0,-1)
       */

      _oneToOneGeom = createGeometry(PRIM_TRIANGLES);

      // Index buffer
      ushort indices[] = {
         0, 1, 2
      };
      RCP<IndexBuffer> iBuffer = createBuffer( INDEX_FMT_16, BUFFER_FLAGS_NONE, 4*sizeof(indices[0]), indices );
      _oneToOneGeom->indexBuffer(iBuffer);

      // Vertex buffer
      float vertices[3*2];
      vertices[0] = -1.0f; // X
      vertices[1] = -3.0f; // Y

      vertices[2] =  3.0f; // X
      vertices[3] =  1.0f; // Y

      vertices[4] = -1.0f; // X
      vertices[5] =  1.0f; // Y

      RCP<VertexBuffer> vBuffer = createBuffer( BUFFER_FLAGS_NONE, 3*2*sizeof(float), vertices );
      vBuffer->addAttribute(ATTRIB_TYPE_POSITION , ATTRIB_FMT_32F_32F, vBuffer->strideInBytes());
      _oneToOneGeom->addBuffer(vBuffer);
#endif
   }

   return  _oneToOneGeom;
}
