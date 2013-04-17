/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Widget/DFViewer.h>

#include <Fusion/Drawable/Text.h>
#include <Fusion/Resource/Image.h>
#include <Fusion/Resource/ResManager.h>
#include <Fusion/VM/VMObjectPool.h>
#include <Fusion/Core/Core.h>
#include <Fusion/Core/Key.h>

#include <Base/ADT/StringMap.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

const char* _dfViewer_str = "dfViewer";

enum {
   ATTRIB_GET_VISIBLE_AREA,
   ATTRIB_GRAPH,
   ATTRIB_ON_SELECTION,
   ATTRIB_SELECTION,
};

StringMap _attributes(
   "getVisibleArea", ATTRIB_GET_VISIBLE_AREA,
   "graph"         , ATTRIB_GRAPH,
   "onSelection"   , ATTRIB_ON_SELECTION,
   "selection"     , ATTRIB_SELECTION,
   ""
);

float us = 32.0f;

Vector<uint16_t>  _indices;
Vector<uint16_t>  _selectedIndices;
Vector<uint16_t>  _decoIndices;
Vector<uint16_t>  _selectedDecoIndices;
Vector<Vec4f>     _vertices;
Vector<Vec4f>     _selectedVertices;
Vector<Vec4f>     _decoVertices;
Vector<Vec4f>     _selectedDecoVertices;
Vector<DFNode*>   _nodes;
Gfx::TextureState _texState;

//------------------------------------------------------------------------------
//!
void pushSelectionVM( VMState* vm, const Set< DFNode* >& selection )
{
   VM::newTable( vm );
   uint idx = 1;
   for( auto it = selection.begin(); it != selection.end(); ++it, ++idx )
   {
      DFNode* node = *it;
      VM::push( vm, node );
      VM::seti( vm, -2, idx );
   }
}

//------------------------------------------------------------------------------
//!
int getVisibleAreaVM( VMState* vm )
{
   DFViewer* v = (DFViewer*)VM::thisPtr( vm );
   AARectf   r = v->getVisibleArea();

   VM::newTable( vm );

   VM::push( vm, "position" );
   VM::push( vm, r.position() );
   VM::set( vm, -3 );

   VM::push( vm, "size" );
   VM::push( vm, r.size() );
   VM::set( vm, -3 );

   return 1;
}

//------------------------------------------------------------------------------
//!
void execute( VMRef& ref, DFViewer* viewer )
{
   if( ref.isValid() )
   {
      VMState* vm = ref.vm();
      VM::push( vm, ref );
      VM::pushProxy( vm, viewer );
      VM::ecall( vm, 1, 0 );
   }
}


UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   CLASS DFViewer
==============================================================================*/

//------------------------------------------------------------------------------
//!
void DFViewer::initialize()
{
   VMObjectPool::registerObject(
      "UI",
      _dfViewer_str,
      stdCreateVM<DFViewer>, stdGetVM<DFViewer>, stdSetVM<DFViewer>
   );

   _texState.magFilter( Gfx::TEX_FILTER_LINEAR );
   _texState.minFilter( Gfx::TEX_FILTER_LINEAR );
   _texState.mipFilter( Gfx::TEX_FILTER_NONE );
   _texState.clampX( Gfx::TEX_CLAMP_LAST );
   _texState.clampY( Gfx::TEX_CLAMP_LAST );
}

//------------------------------------------------------------------------------
//!
DFViewer::DFViewer():
   _canvasPos(0.0f), _selectedPos(0.0f), _moveMode( MOVE_NOTHING )
{
   // Allocate geometry buffers.
   _geom = Core::gfx()->createGeometry( Gfx::PRIM_TRIANGLES );
   RCP<Gfx::IndexBuffer> indices   = Core::gfx()->createBuffer( Gfx::INDEX_FMT_16, Gfx::BUFFER_FLAGS_NONE, 0, 0 );
   RCP<Gfx::VertexBuffer> vertices = Core::gfx()->createBuffer( Gfx::BUFFER_FLAGS_NONE, 0, 0 );
   vertices->addAttribute( Gfx::ATTRIB_TYPE_POSITION,  Gfx::ATTRIB_FMT_32F_32F, 0 );
   vertices->addAttribute( Gfx::ATTRIB_TYPE_TEXCOORD0, Gfx::ATTRIB_FMT_32F_32F, 8 );
   _geom->indexBuffer( indices );
   _geom->addBuffer( vertices );

   _selectedGeom = Core::gfx()->createGeometry( Gfx::PRIM_TRIANGLES );
   indices       = Core::gfx()->createBuffer( Gfx::INDEX_FMT_16, Gfx::BUFFER_FLAGS_NONE, 0, 0 );
   vertices      = Core::gfx()->createBuffer( Gfx::BUFFER_FLAGS_NONE, 0, 0 );
   vertices->addAttribute( Gfx::ATTRIB_TYPE_POSITION,  Gfx::ATTRIB_FMT_32F_32F, 0 );
   vertices->addAttribute( Gfx::ATTRIB_TYPE_TEXCOORD0, Gfx::ATTRIB_FMT_32F_32F, 8 );
   _selectedGeom->indexBuffer( indices );
   _selectedGeom->addBuffer( vertices );

   _decoGeom     = Core::gfx()->createGeometry( Gfx::PRIM_TRIANGLES );
   indices       = Core::gfx()->createBuffer( Gfx::INDEX_FMT_16, Gfx::BUFFER_FLAGS_NONE, 0, 0 );
   vertices      = Core::gfx()->createBuffer( Gfx::BUFFER_FLAGS_NONE, 0, 0 );
   vertices->addAttribute( Gfx::ATTRIB_TYPE_POSITION,  Gfx::ATTRIB_FMT_32F_32F, 0 );
   vertices->addAttribute( Gfx::ATTRIB_TYPE_TEXCOORD0, Gfx::ATTRIB_FMT_32F_32F, 8 );
   _decoGeom->indexBuffer( indices );
   _decoGeom->addBuffer( vertices );

   _selectedDecoGeom     = Core::gfx()->createGeometry( Gfx::PRIM_TRIANGLES );
   indices       = Core::gfx()->createBuffer( Gfx::INDEX_FMT_16, Gfx::BUFFER_FLAGS_NONE, 0, 0 );
   vertices      = Core::gfx()->createBuffer( Gfx::BUFFER_FLAGS_NONE, 0, 0 );
   vertices->addAttribute( Gfx::ATTRIB_TYPE_POSITION,  Gfx::ATTRIB_FMT_32F_32F, 0 );
   vertices->addAttribute( Gfx::ATTRIB_TYPE_TEXCOORD0, Gfx::ATTRIB_FMT_32F_32F, 8 );
   _selectedDecoGeom->indexBuffer( indices );
   _selectedDecoGeom->addBuffer( vertices );


   // Texture sampler.
   _img = data( ResManager::getImage( "image/ui/node01" ) );
   _samplers = new Gfx::SamplerList();
   _samplers->addSampler( "colorTex", _img->texture(), _texState );

   _decoImg = data( ResManager::getImage( "image/ui/output" ) );
   _decoSamplers = new Gfx::SamplerList();
   _decoSamplers->addSampler( "colorTex", _decoImg->texture(), _texState );

   // Constants.
   Vec4f color( 0.2f, 0.2f, 0.2f, 1.0f );
   Vec4f decoColor( 0.9f, 0.9f, 0.9f, 1.0f );
   Vec4f selectedColor( 0.40f, 0.40f, 0.9f, 1.0f );

   RCP<Gfx::ConstantBuffer> constants = Core::gfx()->createConstants( 16 );
   constants->addConstant( "color", Gfx::CONST_FLOAT4, 0 );
   constants->setConstant( "color", color.ptr() );
   _cl = Gfx::ConstantList::create( constants );

   constants = Core::gfx()->createConstants( 16 );
   constants->addConstant( "color", Gfx::CONST_FLOAT4, 0 );
   constants->setConstant( "color", selectedColor.ptr() );
   _selectedCl = Gfx::ConstantList::create( constants );

   constants = Core::gfx()->createConstants( 16 );
   constants->addConstant( "color", Gfx::CONST_FLOAT4, 0 );
   constants->setConstant( "color", decoColor.ptr() );
   _decoCl = Gfx::ConstantList::create( constants );
}

//------------------------------------------------------------------------------
//!
DFViewer::~DFViewer()
{
   if( _graph.isValid() ) _graph->msg().removeOnUpdate( makeDelegate( this, &DFViewer::graphUpdated ) );
}

//------------------------------------------------------------------------------
//!
void DFViewer::graph( DFGraph* graph )
{
   if( graph == _graph ) return;

   // Remove update callback.
   if( _graph.isValid() )
   {
      if( !_selectedNodes.empty() )
      {
         _selectedNodes.clear();
         execute( _onSelection, this );
      }
      _graph->msg().removeOnUpdate( makeDelegate( this, &DFViewer::graphUpdated ) );
   }

   _graph = graph;

   // Add update callback.
   if( _graph.isValid() ) _graph->msg().addOnUpdate( makeDelegate( this, &DFViewer::graphUpdated ) );

   updateGraphGeometry( true );
}

//------------------------------------------------------------------------------
//!
AARectf DFViewer::getVisibleArea() const
{
   Vec2f bl = screenToGraph( Vec2f(0.0f)  );
   Vec2f tr = screenToGraph( actualSize() );
   return AARectf( bl, tr-bl );
}

//------------------------------------------------------------------------------
//!
Vec2f DFViewer::screenToGraph( const Vec2f& p ) const
{
   Vec2f off = Vec2f( _mat(0,3), _mat(1,3) );
   Vec2f pos = p - off;
   return pos / us;
}

//------------------------------------------------------------------------------
//!
void DFViewer::graphUpdated( DFGraph* )
{
   updateGraphGeometry( false );
}

//------------------------------------------------------------------------------
//!
void DFViewer::updateGraphGeometry( bool recenter )
{
   // Build graph UI.
   _indices.clear();
   _vertices.clear();
   _texts.clear();
   _decoIndices.clear();
   _decoVertices.clear();
   _selectedDecoVertices.clear();
   _selectedDecoIndices.clear();
   _selectedIndices.clear();
   _selectedVertices.clear();
   _selectedTexts.clear();

   Vector<NodeText>* texts      = nullptr;
   Vector<ushort>* indices      = nullptr;
   Vector<Vec4f>*  vertices     = nullptr;
   Vector<ushort>* decoIndices  = nullptr;
   Vector<Vec4f>*  decoVertices = nullptr;


   Vec2f offset( _mat(0,3), _mat(1,3) );
   Gfx::Texture* tex = _img->texture();
   float txdim = (float)tex->width();
   float du    = 0.25f;
   float dx    = du*txdim;

   AARectf rect = AARectf::empty();
   for( auto it = _graph->nodes().begin(); it != _graph->nodes().end(); ++it )
   {
      DFNode* node = it->ptr();

      // Check if node is selected.
      if( _selectedNodes.has( node ) )
      {
         indices      = &_selectedIndices;
         vertices     = &_selectedVertices;
         decoIndices  = &_selectedDecoIndices;
         decoVertices = &_selectedDecoVertices;
         texts        = &_selectedTexts;
      }
      else
      {
         indices      = &_indices;
         vertices     = &_vertices;
         decoIndices  = &_decoIndices;
         decoVertices = &_decoVertices;
         texts        = &_texts;
      }

      // Size and position.
      uint i    = uint(vertices->size());
      Vec2f pos = Vec2f(node->position())*us;
      float w   = ((float)node->width())*us;
      float h   = us;
      float dv  = node->hasConnectedInput() ? 0.25f : 0.0f;
      dv       += node->hasConnectedOutput() ? 0.5f : 0.0f;

      rect |= pos;
      rect |= pos+Vec2f(w,h);

      // Geometry.
      vertices->pushBack( Vec4f(                     pos,    0.0f,       dv ) );
      vertices->pushBack( Vec4f( pos+Vec2f(   dx, 0.0f ),      du,       dv ) );
      vertices->pushBack( Vec4f( pos+Vec2f( w-dx, 0.0f ), 1.0f-du,       dv ) );
      vertices->pushBack( Vec4f( pos+Vec2f(    w, 0.0f ),    1.0f,       dv ) );
      vertices->pushBack( Vec4f( pos+Vec2f( 0.0f,    h ),    0.0f, dv+0.25f ) );
      vertices->pushBack( Vec4f( pos+Vec2f(   dx,    h ),      du, dv+0.25f ) );
      vertices->pushBack( Vec4f( pos+Vec2f( w-dx,    h ), 1.0f-du, dv+0.25f ) );
      vertices->pushBack( Vec4f( pos+Vec2f(    w,    h ),    1.0f, dv+0.25f ) );
      indices->pushBack( i+0 );
      indices->pushBack( i+1 );
      indices->pushBack( i+5 );
      indices->pushBack( i+0 );
      indices->pushBack( i+5 );
      indices->pushBack( i+4 );

      indices->pushBack( i+1 );
      indices->pushBack( i+2 );
      indices->pushBack( i+6 );
      indices->pushBack( i+1 );
      indices->pushBack( i+6 );
      indices->pushBack( i+5 );

      indices->pushBack( i+2 );
      indices->pushBack( i+3 );
      indices->pushBack( i+7 );
      indices->pushBack( i+2 );
      indices->pushBack( i+7 );
      indices->pushBack( i+6 );

      // Label.
      RCP<Text> text( new Text() );
      text->font( data( ResManager::getFont( "font/Arimo-Bold" ) ), 12, true );
      text->alignment( Text::MIDDLE, Text::MIDDLE );
      text->text( node->label() );
      text->color( Vec4f(0.9f,0.9f,0.9f,1.0f) );
      text->position( pos+offset );
      text->size( Vec2f(w,h) );
      texts->pushBack( NodeText( text, node ) );

      // Decorations.
      if( node == _graph->output() )
      {
         uint16_t first = uint16_t(decoVertices->size());
         decoVertices->pushBack( Vec4f( pos+Vec2f(w-20.0f,  8.0f), 0.0, 0.0f ) );
         decoVertices->pushBack( Vec4f( pos+Vec2f(w-4.0f,  8.0f), 1.0, 0.0f ) );
         decoVertices->pushBack( Vec4f( pos+Vec2f(w-20.0f, 24.0f), 0.0, 1.0f ) );
         decoVertices->pushBack( Vec4f( pos+Vec2f(w-4.0f, 24.0f), 1.0, 1.0f ) );
         decoIndices->pushBack( first+0 );
         decoIndices->pushBack( first+1 );
         decoIndices->pushBack( first+3 );
         decoIndices->pushBack( first+0 );
         decoIndices->pushBack( first+3 );
         decoIndices->pushBack( first+2 );
      }

      if( node->isGraph() )
      {
         // Just a white square, top left.
         uint16_t first = uint16_t(decoVertices->size());
         decoVertices->pushBack( Vec4f( pos+Vec2f( 8.0f, 20.0f), 0.4f, 0.0f ) );
         decoVertices->pushBack( Vec4f( pos+Vec2f(12.0f, 20.0f), 0.4f, 0.0f ) );
         decoVertices->pushBack( Vec4f( pos+Vec2f(12.0f, 24.0f), 0.4f, 0.0f ) );
         decoVertices->pushBack( Vec4f( pos+Vec2f( 8.0f, 24.0f), 0.4f, 0.0f ) );
         decoIndices->pushBack( first+0 );
         decoIndices->pushBack( first+1 );
         decoIndices->pushBack( first+2 );
         decoIndices->pushBack( first+0 );
         decoIndices->pushBack( first+2 );
         decoIndices->pushBack( first+3 );
      }
   }

   // Make the region definition valid if the graph contains no node.
   if( _graph->nodes().empty() ) rect |= Vec2f( 0.0f );

   if( recenter )
   {
      _canvasPos = rect.center();
      updateCanvas();
   }

   // Fill hardware buffer.
   Core::gfx()->setData( _geom->indexBuffer(), _indices.dataSize(), _indices.data() );
   Core::gfx()->setData( _geom->buffer(0), _vertices.dataSize(), _vertices.data() );
   Core::gfx()->setData( _decoGeom->indexBuffer(), _decoIndices.dataSize(), _decoIndices.data() );
   Core::gfx()->setData( _decoGeom->buffer(0), _decoVertices.dataSize(), _decoVertices.data() );
   Core::gfx()->setData( _selectedGeom->indexBuffer(), _selectedIndices.dataSize(), _selectedIndices.data() );
   Core::gfx()->setData( _selectedGeom->buffer(0), _selectedVertices.dataSize(), _selectedVertices.data() );
   Core::gfx()->setData( _selectedDecoGeom->indexBuffer(), _selectedDecoIndices.dataSize(), _selectedDecoIndices.data() );
   Core::gfx()->setData( _selectedDecoGeom->buffer(0), _selectedDecoVertices.dataSize(), _selectedDecoVertices.data() );
}

//------------------------------------------------------------------------------
//!
void DFViewer::resizeLeftSelected( float d )
{
   _selectedVertices.clear();

   Vec2f offset( _mat(0,3), _mat(1,3) );
   Gfx::Texture* tex = _img->texture();
   float txdim = (float)tex->width();
   float du    = 0.25f;
   float dx    = du*txdim;

   for( size_t i = 0; i < _selectedTexts.size(); ++i  )
   {
      DFNode* node = _selectedTexts[i].second;
      Vec2f pos    = Vec2f(node->position())*us;
      float w      = ((float)node->width())*us;
      float h      = us;
      float dv     = node->hasConnectedInput() ? 0.25f : 0.0f;
      dv          += node->hasConnectedOutput() ? 0.5f : 0.0f;

      // Clamp resize.
      float m = CGM::min( d, w-us );

      _selectedVertices.pushBack( Vec4f( pos+Vec2f(    m, 0.0f ),    0.0f,       dv ) );
      _selectedVertices.pushBack( Vec4f( pos+Vec2f( m+dx, 0.0f ),      du,       dv ) );
      _selectedVertices.pushBack( Vec4f( pos+Vec2f( w-dx, 0.0f ), 1.0f-du,       dv ) );
      _selectedVertices.pushBack( Vec4f( pos+Vec2f(    w, 0.0f ),    1.0f,       dv ) );
      _selectedVertices.pushBack( Vec4f( pos+Vec2f(    m,    h ),    0.0f, dv+0.25f ) );
      _selectedVertices.pushBack( Vec4f( pos+Vec2f( m+dx,    h ),      du, dv+0.25f ) );
      _selectedVertices.pushBack( Vec4f( pos+Vec2f( w-dx,    h ), 1.0f-du, dv+0.25f ) );
      _selectedVertices.pushBack( Vec4f( pos+Vec2f(    w,    h ),    1.0f, dv+0.25f ) );

      _selectedTexts[i].first->size( Vec2f(w-m,h) );
      _selectedTexts[i].first->position( offset+pos+Vec2f(m,0.0f) );
   }

   Core::gfx()->setData( _selectedGeom->buffer(0), _selectedVertices.dataSize(), _selectedVertices.data() );
}

//------------------------------------------------------------------------------
//!
void DFViewer::resizeRightSelected( float d )
{
   _selectedVertices.clear();
   _selectedDecoVertices.clear();

   Gfx::Texture* tex = _img->texture();
   float txdim = (float)tex->width();
   float du    = 0.25f;
   float dx    = du*txdim;

   for( size_t i = 0; i < _selectedTexts.size(); ++i  )
   {
      DFNode* node = _selectedTexts[i].second;
      Vec2f pos    = Vec2f(node->position())*us;
      float w      = ((float)node->width())*us;
      float h      = us;
      float dv     = node->hasConnectedInput() ? 0.25f : 0.0f;
      dv          += node->hasConnectedOutput() ? 0.5f : 0.0f;

      // Clamp resize.
      w = CGM::max( us, w+d );

      _selectedVertices.pushBack( Vec4f( pos+Vec2f( 0.0f, 0.0f ),    0.0f,       dv ) );
      _selectedVertices.pushBack( Vec4f( pos+Vec2f(   dx, 0.0f ),      du,       dv ) );
      _selectedVertices.pushBack( Vec4f( pos+Vec2f( w-dx, 0.0f ), 1.0f-du,       dv ) );
      _selectedVertices.pushBack( Vec4f( pos+Vec2f(    w, 0.0f ),    1.0f,       dv ) );
      _selectedVertices.pushBack( Vec4f( pos+Vec2f( 0.0f,    h ),    0.0f, dv+0.25f ) );
      _selectedVertices.pushBack( Vec4f( pos+Vec2f(   dx,    h ),      du, dv+0.25f ) );
      _selectedVertices.pushBack( Vec4f( pos+Vec2f( w-dx,    h ), 1.0f-du, dv+0.25f ) );
      _selectedVertices.pushBack( Vec4f( pos+Vec2f(    w,    h ),    1.0f, dv+0.25f ) );

      _selectedTexts[i].first->size( Vec2f(w,h) );

      // Decorations.
      if( node == _graph->output() )
      {
         _selectedDecoVertices.pushBack( Vec4f( pos+Vec2f(w-20.0f,  8.0f), 0.0, 0.0f ) );
         _selectedDecoVertices.pushBack( Vec4f( pos+Vec2f(w-4.0f,  8.0f), 1.0, 0.0f ) );
         _selectedDecoVertices.pushBack( Vec4f( pos+Vec2f(w-20.0f, 24.0f), 0.0, 1.0f ) );
         _selectedDecoVertices.pushBack( Vec4f( pos+Vec2f(w-4.0f, 24.0f), 1.0, 1.0f ) );
      }
   }

   Core::gfx()->setData( _selectedGeom->buffer(0), _selectedVertices.dataSize(), _selectedVertices.data() );
   Core::gfx()->setData( _selectedDecoGeom->buffer(0), _selectedDecoVertices.dataSize(), _selectedDecoVertices.data() );
}

//------------------------------------------------------------------------------
//!
void DFViewer::updateCanvas()
{
   Vec2f pos = globalPosition() - _canvasPos + actualSize()*0.5f;
   _mat      = Mat4f::translation( pos.x, pos.y, 0.0f );

   // Text.
   for( size_t i = 0; i < _texts.size(); ++i )
   {
      Vec2f tpos = Vec2f(_texts[i].second->position())*us;
      _texts[i].first->position( pos+tpos );
   }

   Vec2f spos = pos + _selectedPos;
   _smat      = Mat4f::translation( spos.x, spos.y, 0.0f );
   for( size_t i = 0; i < _selectedTexts.size(); ++i )
   {
      Vec2f tpos = Vec2f(_selectedTexts[i].second->position())*us;
      _selectedTexts[i].first->position( spos+tpos );
   }
}

//------------------------------------------------------------------------------
//!
void DFViewer::deleteSelected()
{
   DFGraph::UpdateLock lock( _graph.ptr() );
   for( auto it = _selectedNodes.begin(); it != _selectedNodes.end(); ++it )
   {
      _graph->removeNode( *it );
   }
   _selectedNodes.clear();
   execute( _onSelection, this );
}

//------------------------------------------------------------------------------
//!
void DFViewer::onPointerPress( const Event& ev )
{
   if( _graph.isNull() || _moveMode != MOVE_NOTHING ) return;

   // Convert pointer into graph space.
   Vec2f offset( _mat(0,3), _mat(1,3) );
   Vec2f pos  = ev.position() - offset;
   // Convert position into grid space.
   Vec2i gpos(CGM::floor(pos/us));

   DFNode* node = _graph->getNode( gpos );

   if( node )
   {
      // Select output?
      if( ev.value() == 3 )
      {
         _graph->output( node );
         return;
      }

      // Test if resize left, right or move node.
      Vec2f npos = Vec2f(node->position())*us;
      float nw   = node->width()*us;
      _moveMode  = (pos.x < npos.x+12) ? MOVE_LEFT :
         (pos.x > npos.x+nw-12) ? MOVE_RIGHT : MOVE_NODE;

      if( Core::isKeyPressed( Key::META ) )
      {
         if( _selectedNodes.has( node ) && _moveMode == MOVE_NODE )
         {
            _selectedNodes.remove( node );
         }
         else
         {
            _selectedNodes.add( node );
         }
         execute( _onSelection, this );
      }
      else
      {
         if( !_selectedNodes.has( node ) )
         {
            _selectedNodes.clear();
            _selectedNodes.add( node );
            execute( _onSelection, this );
         }
      }
      updateGraphGeometry();
   }
   else
   {
      if( !_selectedNodes.empty() )
      {
         _selectedNodes.clear();
         execute( _onSelection, this );
         updateGraphGeometry();
      }
   }
}

//------------------------------------------------------------------------------
//!
void DFViewer::onPointerRelease( const Event& ev )
{
   Pointer& ptr = ev.pointer();
   Vec2f del    = ptr.position()-ptr.pressPosition();
   Vec2i idel   = CGM::round(del/us);

   // Release of selected nodes.
   if( _moveMode == MOVE_NODE )
   {
      if( idel.x != 0 || idel.y != 0 )
      {
         _nodes.clear();
         for( auto it = _selectedNodes.begin(); it != _selectedNodes.end(); ++it )
         {
            _nodes.pushBack( *it );
         }
         _graph->moveNodes( _nodes, idel );
      }
   }
   else if( _moveMode == MOVE_LEFT )
   {
      _nodes.clear();
      for( auto it = _selectedNodes.begin(); it != _selectedNodes.end(); ++it )
      {
         _nodes.pushBack( *it );
      }
      _graph->leftResizeNodes( _nodes, -idel.x );
   }
   else if( _moveMode == MOVE_RIGHT )
   {
      _nodes.clear();
      for( auto it = _selectedNodes.begin(); it != _selectedNodes.end(); ++it )
      {
         _nodes.pushBack( *it );
      }
      _graph->resizeNodes( _nodes, idel.x );
   }

   _moveMode    = MOVE_NOTHING;
   _selectedPos = Vec2f(0.0f);
   updateCanvas();
}

//------------------------------------------------------------------------------
//!
void DFViewer::onPointerMove( const Event& ev )
{
   Pointer& ptr = ev.pointer();
   if( !ptr.pressed() ) return;

   Vec2f delta = ptr.position()-ptr.pressPosition();

   if( _moveMode == MOVE_NODE )
   {
      _selectedPos = delta;
      updateCanvas();
   }
   else if( _moveMode == MOVE_LEFT )
   {
      resizeLeftSelected( delta.x );
   }
   else if( _moveMode == MOVE_RIGHT )
   {
      resizeRightSelected( delta.x );
   }
}

//------------------------------------------------------------------------------
//!
void DFViewer::onPointerScroll( const Event& ev )
{
   float s = Core::isKeyPressed( Key::SHIFT ) ? 20.0f : 2.0f;
   _canvasPos += ev.scrollValue() * s;
   updateCanvas();
}

//------------------------------------------------------------------------------
//!
void DFViewer::onChar( const Event& ev )
{
   if( ev.value() == Key::BS || ev.value() == Key::DELETE )
   {
      deleteSelected();
   }
}

//------------------------------------------------------------------------------
//!
void DFViewer::onPointerEnter( const Event& )
{
}

//------------------------------------------------------------------------------
//!
void DFViewer::onPointerLeave( const Event& )
{
}

//------------------------------------------------------------------------------
//!
void DFViewer::render( const RCP<Gfx::RenderNode>& rn )
{
   Gfx::Pass& pass = *(rn->current());

   Vec2i scPos   = globalPosition();
   Vec2i scSize  = actualSize();
   const int* sc = pass.addScissor( scPos.x, scPos.y, scSize.x, scSize.y );

   pass.setWorldMatrixPtr( _mat.ptr() );
   pass.setConstants( _cl );
   pass.setSamplers( _samplers );
   pass.execGeometry( _geom );

   pass.setConstants( _decoCl );
   pass.setSamplers( _decoSamplers );
   pass.execGeometry( _decoGeom );

   // Text.
   for( size_t i = 0; i < _texts.size(); ++i )
   {
      _texts[i].first->draw( rn );
   }

   // Selected nodes.
   if( !_selectedNodes.empty() )
   {
      pass.setWorldMatrixPtr( _smat.ptr() );
      pass.setConstants( _selectedCl );
      pass.setSamplers( _samplers );
      pass.execGeometry( _selectedGeom );

      pass.setConstants( _decoCl );
      pass.setSamplers( _decoSamplers );
      pass.execGeometry( _selectedDecoGeom );

      for( size_t i = 0; i < _selectedTexts.size(); ++i )
      {
         _selectedTexts[i].first->draw( rn );
      }
   }

   pass.setScissor( sc[0], sc[1], sc[2], sc[3] );
}

//------------------------------------------------------------------------------
//!
void DFViewer::performSetGeometry()
{
   updateCanvas();
   Widget::performSetGeometry();
}

//------------------------------------------------------------------------------
//!
void DFViewer::performSetPosition()
{
   updateCanvas();
   Widget::performSetPosition();
}

//------------------------------------------------------------------------------
//!
const char* DFViewer::meta() const
{
   return _dfViewer_str;
}

//------------------------------------------------------------------------------
//!
void DFViewer::init( VMState* vm )
{
   Widget::init( vm );
}

//------------------------------------------------------------------------------
//!
bool DFViewer::performGet( VMState* vm )
{
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_GET_VISIBLE_AREA:
         VM::push( vm, this, getVisibleAreaVM );
         return true;
      case ATTRIB_GRAPH:
         VM::pushProxy( vm, _graph.ptr() );
         return true;
      case ATTRIB_ON_SELECTION:
         VM::push( vm, _onSelection );
         return true;
      case ATTRIB_SELECTION:
         pushSelectionVM( vm, _selectedNodes );
         return true;
   }
   return Widget::performGet( vm );
}

//------------------------------------------------------------------------------
//!
bool DFViewer::performSet( VMState* vm )
{
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_GET_VISIBLE_AREA:
         // Read-only.
         return true;
      case ATTRIB_GRAPH:
         graph( (DFGraph*)VM::toProxy( vm, 3 ) );
         return true;
      case ATTRIB_ON_SELECTION:
         VM::toRef( vm, 3, _onSelection );
         return true;
      case ATTRIB_SELECTION:
         StdErr << "DFViewer selection is currently read-only." << nl;
         return false;
   }
   return Widget::performSet( vm );
}

//------------------------------------------------------------------------------
//!
bool DFViewer::isAttribute( const char* name ) const
{
   if( _attributes[ name ] != StringMap::INVALID ) return true;
   return Widget::isAttribute( name );
}

NAMESPACE_END
