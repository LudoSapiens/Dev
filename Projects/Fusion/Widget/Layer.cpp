/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Widget/Layer.h>
#include <Fusion/Core/Core.h>
#include <Fusion/VM/VMObjectPool.h>

#include <CGMath/CGMath.h>

#include <Base/ADT/StringMap.h>
#include <Base/Dbg/DebugStream.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

DBG_STREAM( os_layer, "Layer" );

Vector<Vec4f>  _sVertices( 4 );
ushort         _sIndices[] =
{
   0, 1, 2, 3
};

//------------------------------------------------------------------------------
//!
enum {
   ATTRIB_TEX_STATE
};

StringMap _attributes(
   "texState",    ATTRIB_TEX_STATE,
   ""
);

//------------------------------------------------------------------------------
//!
const char* _layer_str_ = "layer";

UNNAMESPACE_END


NAMESPACE_BEGIN

/*==============================================================================
  CLASS Layer
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
Layer::initialize()
{
   VMObjectPool::registerObject( "UI", _layer_str_, stdCreateVM<Layer>, stdGetVM<Layer>, stdSetVM<Layer> );
}

//------------------------------------------------------------------------------
//!
Layer::Layer():
   _color(1.0f, 1.0f, 1.0f, 1.0f)
{
   _maxWidgets = 1;

   _fb = Core::gfx()->createFramebuffer();

   _geom = Core::gfx()->createGeometry( Gfx::PRIM_TRIANGLE_FAN );

   _rtt_rn   = new Gfx::RenderNode();
   _rtt_pass = new Gfx::Pass();

   _constants = Core::gfx()->createConstants( 16 );
   _constants->addConstant( "color", Gfx::CONST_FLOAT4, 0 );
   _constants->setConstant( "color", _color.ptr() );
   _cl = Gfx::ConstantList::create( _constants );

   _ts.magFilter( Gfx::TEX_FILTER_LINEAR );
   _ts.minFilter( Gfx::TEX_FILTER_LINEAR );
   _ts.mipFilter( Gfx::TEX_FILTER_NONE );
   _ts.clampX( Gfx::TEX_CLAMP_LAST );
   _ts.clampY( Gfx::TEX_CLAMP_LAST );
}

//------------------------------------------------------------------------------
//!
Layer::~Layer()
{
}

//------------------------------------------------------------------------------
//!
void
Layer::render( const RCP<Gfx::RenderNode>& rn )
{
   CHECK( _cb.isValid() );
   CHECK( _db.isValid() );
   CHECK( _fb.isValid() );

   // Render widget into an offscreen buffer.
   int wi = int(actualSize()(0));
   int hi = int(actualSize()(1));
   // Avoid "incomplete attachment in framebuffer"
   if( wi*hi == 0 ) return;

   _rtt_rn->clear();
   _rtt_rn->addPass( _rtt_pass ); // to avoid recursive requirement adding by Layers in this Layer
   _rtt_pass->clear();
   _rtt_pass->setClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
   _rtt_pass->setViewport( 0, 0, wi, hi );
   _rtt_pass->setScissor( 0, 0, wi, hi );
   _rtt_pass->setProjectionMatrixPtr( _layerContentsTransform.ptr() );
   _rtt_pass->setFramebuffer( _fb );
   _rtt_pass->setProgram( Core::defaultProgram() );
   WidgetContainer::render( _rtt_rn );

   rn->addRequirement( _rtt_rn );

   // Blit the texture into the current area.
   Gfx::Pass& pass = *(rn->current());
   // 1. Set sampler for the source texture.
   pass.setSamplers( _sl );
   pass.setConstants( _cl );
   // 2. Set matrix to relocate the origin.
   pass.setWorldMatrixPtr( _layerTransform.ptr() );
   // 3. Render quad into current node.
   pass.execGeometry( _geom );
}

//------------------------------------------------------------------------------
//!
void
Layer::performSetGeometry
( void )
{
   _layerTransform = Mat4f::translation( globalPosition()(0), globalPosition()(1), 0.0f );
   updateBuffers();
   
   // Simply forward to the child.
   Container::ConstIterator it  = _widgets.begin();
   Container::ConstIterator end = _widgets.end();

   for( ; it != end; ++it )
   {
      if( !(*it)->hidden() )
      {
         (*it)->geometry( Vec2f::zero(), Vec2f::zero(), Vec2f(float(_cb->width()), float(_cb->height())) );
      }
   }
}

//------------------------------------------------------------------------------
//!
void
Layer::performSetPosition()
{
   _layerTransform = Mat4f::translation( globalPosition()(0), globalPosition()(1), 0.0f );
}

//------------------------------------------------------------------------------
//!
bool
Layer::isAttribute( const char* name ) const
{
   if( _attributes[ name ] != StringMap::INVALID ) return true;

   return WidgetContainer::isAttribute( name );
}

//------------------------------------------------------------------------------
//!
Vec2f
Layer::screenToLayer( const Vec2f& pos ) const
{
   Vec2f tmp;
   if( parent() != NULL )
   {
      tmp = parent()->screenToLayer( pos );
   }
   else
   {
      tmp = pos;
   }
   tmp -= globalPosition();
   return tmp;
}

//------------------------------------------------------------------------------
//!
Vec2f
Layer::layerToScreen( const Vec2f& pos ) const
{
   Vec2f tmp = pos - globalPosition();
   if( parent() != NULL )
   {
      tmp = parent()->screenToLayer( tmp );
   }
   return tmp;
}

//------------------------------------------------------------------------------
//!
Widget*
Layer::getWidgetAt( const Vec2f& pos )
{
   DBG_BLOCK( os_layer, "Layer::getWidgetAt( " << pos << " )" );
   Vec2f posLocal = pos - globalPosition();
   DBG_MSG( os_layer, " --> " << pos );
   return WidgetContainer::getWidgetAt( posLocal );
}

//------------------------------------------------------------------------------
//!
void
Layer::updateBuffers()
{
   // Update various elements if necessary.
   uint w = (uint)actualSize()(0);
   uint h = (uint)actualSize()(1);
   if( _cb.isNull() || (_cb->width() != w || _cb->height() != h) )
   {
      // 1. Set color buffer.
      _cb = Core::gfx()->create2DTexture(
         w, h,
         Gfx::TEX_FMT_16F_16F_16F_16F,
         Gfx::TEX_CHANS_RGBA,
         Gfx::TEX_FLAGS_RENDERABLE
      );
      _fb->setColorBuffer( _cb );

      // 2. Prepare the sampler list.
      _sl = new Gfx::SamplerList();
      _sl->addSampler( "colorTex", _cb, _ts );

      // 3. Set depth buffer.
      _db = Core::gfx()->create2DTexture(
         w, h,
         Gfx::TEX_FMT_24_8,
         Gfx::TEX_CHANS_ZS,
         Gfx::TEX_FLAGS_RENDERABLE
      );
      _fb->setDepthBuffer( _db );

      // 4. Refresh the buffer data.
      RCP<Gfx::VertexBuffer> vertexBuffer;
      if( _geom->numBuffers() == 0 )
      {
         // Create the index buffer (indices are always the same).
         RCP<Gfx::IndexBuffer> indexBuffer = Core::gfx()->createBuffer( Gfx::INDEX_FMT_16, Gfx::BUFFER_FLAGS_NONE, 0, 0 );
         _geom->indexBuffer( indexBuffer );
         Core::gfx()->setData( indexBuffer, 4*sizeof(_sIndices[0]), _sIndices );

         // Create the vertex buffer.
         vertexBuffer = Core::gfx()->createBuffer( Gfx::BUFFER_FLAGS_NONE, 0, 0 );
         vertexBuffer->addAttribute( Gfx::ATTRIB_TYPE_POSITION,  Gfx::ATTRIB_FMT_32F_32F, vertexBuffer->strideInBytes() );
         vertexBuffer->addAttribute( Gfx::ATTRIB_TYPE_TEXCOORD0, Gfx::ATTRIB_FMT_32F_32F, vertexBuffer->strideInBytes() );
         _geom->addBuffer( vertexBuffer );
      }
      else
      {
         vertexBuffer = _geom->buffers()[0];
      }
      // Fill buffer with vertex data (Vec4f is XYST).
      float wf = (float)w;
      float hf = (float)h;
      _sVertices[0] = Vec4f( 0.0f, 0.0f, 0.0f, 0.0f );
      _sVertices[1] = Vec4f(   wf, 0.0f, 1.0f, 0.0f );
      _sVertices[2] = Vec4f(   wf,   hf, 1.0f, 1.0f );
      _sVertices[3] = Vec4f( 0.0f,   hf, 0.0f, 1.0f );
      Core::gfx()->setData( vertexBuffer, _sVertices.dataSize(), _sVertices.data() );

      // 5. Update transform matrix.
      _layerContentsTransform = Mat4f::ortho( 0.0f, (float)w, 0.0f, (float)h, 0.0f, 1.0f );

      // 6. Update constants.
      _constants->setConstant( "color", _color.ptr() );
   }

}


//------------------------------------------------------------------------------
//!
const char*
Layer::meta() const
{
   return _layer_str_;
}

//------------------------------------------------------------------------------
//!
void
Layer::init( VMState* vm )
{
   DBG_BLOCK( os_layer, "Layer::init()" );

   // Base class init.
   WidgetContainer::init( vm );

   if( VM::get( vm, 1, "color", _color ) )
   {
      _constants->setConstant( "color", _color.ptr() );
   }
}

//------------------------------------------------------------------------------
//!
bool
Layer::performGet( VMState* vm )
{
   DBG_BLOCK( os_layer, "Layer::performGet(" << VM::toCString(vm, 2) << ")" );
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_TEX_STATE:
      {
         StdErr << "GfxTextureState was deprecated." << nl;
         CHECK( false );
      }  break;
      default: break;
   }
   DBG_MSG( os_layer, "... not a known attribute, so asking WidgetContainer" );
   return WidgetContainer::performGet( vm );
}

//------------------------------------------------------------------------------
//!
bool
Layer::performSet( VMState* vm )
{
   DBG_BLOCK( os_layer, "Layer::performSet(" << VM::toCString(vm, 2) << ")" );
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_TEX_STATE:
         StdErr << "GfxTextureState was deprecated." << nl;
         CHECK( false );
         break;
      default: break;
   }
   DBG_MSG( os_layer, "... not a known attribute, so calling WidgetContainer" );
   return WidgetContainer::performSet( vm );
}

NAMESPACE_END
