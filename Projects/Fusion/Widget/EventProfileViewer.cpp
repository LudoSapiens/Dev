/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Widget/EventProfileViewer.h>

#include <Fusion/Drawable/Text.h>
#include <Fusion/Drawable/TQuad.h>
#include <Fusion/Resource/Image.h>
#include <Fusion/Resource/ResManager.h>
#include <Fusion/VM/VMObjectPool.h>
#include <Fusion/Core/Core.h>
#include <Fusion/Core/Key.h>

#include <Base/ADT/StringMap.h>
#include <Base/MT/Thread.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

const char* _eventProfileViewer_str = "eventProfileViewer";

enum {
   ATTRIB_LOAD,
};

StringMap _attributes(
   "load", ATTRIB_LOAD,
   ""
);

//------------------------------------------------------------------------------
//!
int loadVM( VMState* vm )
{
   EventProfileViewer* epv = (EventProfileViewer*)VM::thisPtr( vm );
   const char* filename = VM::toCString( vm, 1 );
   VM::push( vm, epv->load(filename) );
   return 1;
}

#if 0
//------------------------------------------------------------------------------
//!
void execute( VMRef& ref, EventProfileViewer* viewer )
{
   if( ref.isValid() )
   {
      VMState* vm = ref.vm();
      VM::push( vm, ref );
      VM::pushProxy( vm, viewer );
      VM::ecall( vm, 1, 0 );
   }
}
#endif

Gfx::TextureState  _texState;
Vector<uint16_t>   _indices[EventProfileViewer::NUM_TYPES];  // Reuse common static buffer as scratch data.
Vector<Vec4f>      _vertices[EventProfileViewer::NUM_TYPES]; // Reuse common static buffer as scratch data.

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   CLASS DFViewer
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
EventProfileViewer::initialize()
{
   VMObjectPool::registerObject(
      "UI",
      _eventProfileViewer_str,
      stdCreateVM<EventProfileViewer>, stdGetVM<EventProfileViewer>, stdSetVM<EventProfileViewer>
   );

   _texState.magFilter( Gfx::TEX_FILTER_LINEAR );
   _texState.minFilter( Gfx::TEX_FILTER_LINEAR );
   _texState.mipFilter( Gfx::TEX_FILTER_NONE );
   _texState.clampX( Gfx::TEX_CLAMP_LAST );
   _texState.clampY( Gfx::TEX_CLAMP_LAST );
}

//------------------------------------------------------------------------------
//!
EventProfileViewer::EventProfileViewer():
   _center(0.0f), _zoom(1.0f, 1.0/32.0f),
   _legendVisible( true )
{
   _colors[TYPE_CORE     ] = Vec4f( 0.2f, 0.2f, 0.2f, 0.2f );
   _colors[TYPE_LOOP     ] = Vec4f( 0.2f, 0.2f, 0.8f, 1.0f );
   _colors[TYPE_RENDER   ] = Vec4f( 0.8f, 0.2f, 0.2f, 1.0f );
   _colors[TYPE_DISPLAY  ] = Vec4f( 221.0f, 204.0f, 119.0f, 255.0f ) / 255.0f;
   _colors[TYPE_WORLD    ] = Vec4f( 136.0f, 204.0f, 238.0f, 255.0f ) / 255.0f;
   _colors[TYPE_ANIMATORS] = Vec4f( 136.0f,  34.0f,  85.0f, 255.0f ) / 255.0f;
   _colors[TYPE_BRAINS   ] = Vec4f(  51.0f,  34.0f, 136.0f, 255.0f ) / 255.0f;
   _colors[TYPE_COMMANDS ] = Vec4f( 153.0f, 153.0f,  51.0f, 255.0f ) / 255.0f;
   _colors[TYPE_ACTIONS  ] = Vec4f( 204.0f, 102.0f, 119.0f, 255.0f ) / 255.0f;
   _colors[TYPE_PHYSICS  ] = Vec4f(  17.0f, 119.0f,  51.0f, 255.0f ) / 255.0f;

   _yRange[TYPE_CORE     ] = Vec2f( -1.000f,  0.000f );
   _yRange[TYPE_LOOP     ] = Vec2f( -2.000f, -1.000f );
   _yRange[TYPE_RENDER   ] = Vec2f( -3.000f, -2.000f );
   _yRange[TYPE_DISPLAY  ] = Vec2f( -3.500f, -3.000f );
   _yRange[TYPE_WORLD    ] = Vec2f( -3.500f, -3.000f );
   _yRange[TYPE_ANIMATORS] = Vec2f( -4.000f, -3.500f );
   _yRange[TYPE_BRAINS   ] = Vec2f( -4.000f, -3.500f );
   _yRange[TYPE_COMMANDS ] = Vec2f( -4.000f, -3.500f );
   _yRange[TYPE_ACTIONS  ] = Vec2f( -4.000f, -3.500f );
   _yRange[TYPE_PHYSICS  ] = Vec2f( -4.000f, -3.500f );

   // Allocate geometry buffers with constants.
   for( uint i = 0; i < NUM_TYPES; ++i )
   {
      auto& geometry = _geometry[i];
      geometry = Core::gfx()->createGeometry( Gfx::PRIM_TRIANGLES );
      RCP<Gfx::IndexBuffer>  indices  = Core::gfx()->createBuffer( Gfx::INDEX_FMT_16, Gfx::BUFFER_FLAGS_NONE, 0, 0 );
      RCP<Gfx::VertexBuffer> vertices = Core::gfx()->createBuffer( Gfx::BUFFER_FLAGS_NONE, 0, 0 );
      vertices->addAttribute( Gfx::ATTRIB_TYPE_POSITION,  Gfx::ATTRIB_FMT_32F_32F, vertices->strideInBytes() );
      vertices->addAttribute( Gfx::ATTRIB_TYPE_TEXCOORD0, Gfx::ATTRIB_FMT_32F_32F, vertices->strideInBytes() );
      geometry->indexBuffer( indices );
      geometry->addBuffer( vertices );

      // Constants.
      const Vec4f& color = _colors[i];
      RCP<Gfx::ConstantBuffer> constants = Core::gfx()->createConstants( sizeof(color) );
      constants->addConstant( "color", Gfx::CONST_FLOAT4, 0 );
      constants->setConstant( "color", color.ptr() );
      _constants[i] = Gfx::ConstantList::create( constants );
   }

   // Texture sampler.
   _img = data( ResManager::getImage( "image/gradient1" ) );
   _sampler = new Gfx::SamplerList();
   _sampler->addSampler( "colorTex", _img->texture(), _texState );
}

//------------------------------------------------------------------------------
//!
EventProfileViewer::~EventProfileViewer()
{
}

//------------------------------------------------------------------------------
//!
bool
EventProfileViewer::load( const char* filename )
{
   _loadedProfiler.clear();

   if( filename == nullptr )
   {
      _profiler = &Core::profiler();
      updateGeometry( true );
      return true;
   }
   else
   {
      _profiler = &_loadedProfiler;
      bool ok = _loadedProfiler.load( filename );
      StdErr << ">>> " << _profiler->numEvents() << " events." << nl;
      updateGeometry( true );
      return ok;
   }
}

//------------------------------------------------------------------------------
//!
void
EventProfileViewer::updateGeometry( bool recenterAfter )
{
   // Build graph UI.
   for( uint i = 0; i < NUM_TYPES; ++i )
   {
      _indices[i].clear();
      _vertices[i].clear();
   }

   if( _profiler->numEvents() == 0 )  return;

   double  last[EventProfiler::NUM_EVENT_TYPES];
   for( uint i = 0; i < EventProfiler::NUM_EVENT_TYPES; ++i )
   {
      last[i] = 0.0;
   }

   // Some Core event boxes.
   EventProfiler::ConstIterator lst = _profiler->end()-1;
   EventProfiler::ConstIterator itA, itB;
   itA = _profiler->findFirst(EventProfiler::CORE_BEGIN, 20);
   float y = _yRange[TYPE_CORE].x;
   if( itA != _profiler->end() )
   {
      itB = _profiler->findLast(EventProfiler::CORE_END, 20);
      if( itB == _profiler->end() )  itB = lst;
      makeBox( TYPE_CORE, Vec2f(y, y+1.0f), (*itA)._timestamp, (*itB)._timestamp );
   }
   itA = _profiler->findFirst(EventProfiler::EXEC_BEGIN, 20);
   //y = 0.0f;
   if( itA != _profiler->end() )
   {
      itB = _profiler->findLast(EventProfiler::EXEC_END, 20);
      if( itB == _profiler->end() )  itB = lst;
      makeBox( TYPE_CORE, Vec2f(y+0.50f, y+0.75f), (*itA)._timestamp, (*itB)._timestamp );
   }
   itA = _profiler->findFirst(EventProfiler::GUI_BEGIN, 20);
   if( itA != _profiler->end() )
   {
      itB = _profiler->findLast(EventProfiler::GUI_END, 20);
      if( itB == _profiler->end() )  itB = lst;
      makeBox( TYPE_CORE, Vec2f(y+0.25f, y+0.50f), (*itA)._timestamp, (*itB)._timestamp );
   }
   itA = _profiler->findFirst(EventProfiler::LOOPS_BEGIN, 20);
   if( itA != _profiler->end() )
   {
      itB = _profiler->findLast(EventProfiler::LOOPS_END, 20);
      if( itB == _profiler->end() )  itB = lst;
      makeBox( TYPE_CORE, Vec2f(y+0.00f, y+0.25f), (*itA)._timestamp, (*itB)._timestamp );
   }

   // Skip to the first frame.
   EventProfiler::ConstIterator cur;
   for( cur = _profiler->begin(); cur != _profiler->end(); ++cur )
   {
      const EventProfiler::Event& ev = (*cur);
      last[ev._event] = (*cur)._timestamp;
      if( ev._event == EventProfiler::RENDER_END )  break; // Found the first frame.
   }

   for( ; cur != _profiler->end(); ++cur )
   {
      const EventProfiler::Event& ev = (*cur);
      const double curT = ev._timestamp;

      switch( ev._event )
      {
         case EventProfiler::DISPLAY_END:
            makeBox( TYPE_DISPLAY, _yRange[TYPE_DISPLAY], last[EventProfiler::DISPLAY_BEGIN], curT );
            break;
         case EventProfiler::LOOP_END:
            makeBox( TYPE_LOOP, _yRange[TYPE_LOOP], last[EventProfiler::LOOP_BEGIN], curT );
            break;
         case EventProfiler::RENDER_END:
            makeBox( TYPE_RENDER, _yRange[TYPE_RENDER], last[EventProfiler::RENDER_END], curT );
            break;
         case EventProfiler::WORLD_BEGIN_BRAINS:
            makeBox( TYPE_ANIMATORS, _yRange[TYPE_ANIMATORS], last[EventProfiler::WORLD_BEGIN_ANIMATORS], curT );
            break;
         case EventProfiler::WORLD_BEGIN_COMMANDS_AFTER_BRAINS:
            makeBox( TYPE_BRAINS, _yRange[TYPE_BRAINS], last[EventProfiler::WORLD_BEGIN_BRAINS], curT );
            break;
         case EventProfiler::WORLD_BEGIN_ACTIONS:
            makeBox( TYPE_COMMANDS, _yRange[TYPE_COMMANDS], last[EventProfiler::WORLD_BEGIN_COMMANDS_AFTER_BRAINS], curT );
            break;
         case EventProfiler::WORLD_BEGIN_COMMANDS_AFTER_ACTIONS:
            makeBox( TYPE_ACTIONS, _yRange[TYPE_ACTIONS], last[EventProfiler::WORLD_BEGIN_ACTIONS], curT );
            break;
         case EventProfiler::WORLD_BEGIN_PHYSICS:
            makeBox( TYPE_COMMANDS, _yRange[TYPE_COMMANDS], last[EventProfiler::WORLD_BEGIN_COMMANDS_AFTER_ACTIONS], curT );
            break;
         case EventProfiler::WORLD_BEGIN_ACTIONS_AFTER_PHYSICS:
            makeBox( TYPE_PHYSICS, _yRange[TYPE_PHYSICS], last[EventProfiler::WORLD_BEGIN_PHYSICS], curT );
            break;
         case EventProfiler::WORLD_BEGIN_COMMANDS_AFTER_PHYSICS:
            makeBox( TYPE_ACTIONS, _yRange[TYPE_ACTIONS], last[EventProfiler::WORLD_BEGIN_ACTIONS_AFTER_PHYSICS], curT );
            break;
         case EventProfiler::WORLD_END:
            makeBox( TYPE_COMMANDS, _yRange[TYPE_COMMANDS], last[EventProfiler::WORLD_BEGIN_COMMANDS_AFTER_PHYSICS], curT );
            makeBox( TYPE_WORLD, _yRange[TYPE_WORLD], last[EventProfiler::WORLD_BEGIN], curT );
            break;
         default:
            break;
      }

      last[ev._event] = curT;
   }

   recomputeBoundingBox();

   if( recenterAfter )  recenter();

   // Fill hardware buffer.
   for( uint i = 0; i < NUM_TYPES; ++i )
   {
      auto& geometry = _geometry[i];
      auto& indices  = _indices[i];
      auto& vertices = _vertices[i];
      Core::gfx()->setData( geometry->indexBuffer(), indices.dataSize(), indices.data() );
      Core::gfx()->setData( geometry->buffer(0), vertices.dataSize(), vertices.data() );
   }
}

//------------------------------------------------------------------------------
//!
void
EventProfileViewer::makeBox( Type type, const Vec2f& yRange, double from, double to )
{
   float l = float(from);
   float r = float(to);
   float b = yRange.x;
   float t = yRange.y;
   //StdErr << "makeBox(" << row << "," << from << ":" << to << ")"
   //       << " l=" << l
   //       << " r=" << r
   //       << " t=" << t
   //       << " b=" << b
   //       << nl;
   auto& vertices = _vertices[type];
   auto& indices  = _indices[type];
   uint i = uint( vertices.size() );
   vertices.pushBack( Vec4f( l, b, 0.0f, 0.0f ) );
   vertices.pushBack( Vec4f( r, b, 1.0f, 0.0f ) );
   vertices.pushBack( Vec4f( l, t, 0.0f, 1.0f ) );
   vertices.pushBack( Vec4f( r, t, 1.0f, 1.0f ) );
   indices.pushBack( i+0 );
   indices.pushBack( i+1 );
   indices.pushBack( i+3 );
   indices.pushBack( i+0 );
   indices.pushBack( i+3 );
   indices.pushBack( i+2 );
}

//------------------------------------------------------------------------------
//!
void
EventProfileViewer::recenter()
{
   if( _dataRange.isEmpty() )
   {
      _center = Vec2f( 0.0f );
      _zoom   = Vec2f( 1.0f, 1.0f/32.0f );
   }
   else
   {
      _center = _dataRange.center();
      _zoom   = Vec2f( 1.0f, 1.0f/32.0f );
   }

   updateCanvas();
}

//------------------------------------------------------------------------------
//!
void
EventProfileViewer::recomputeBoundingBox()
{
   _dataRange = AARectf::empty();
   for( size_t i = 0; i < NUM_TYPES; ++i )
   {
      auto& vertices = _vertices[i];
      if( !vertices.empty() )
      {
         _dataRange |= vertices.front()(0,1);
         _dataRange |= vertices.back()(0,1);
      }
   }
   if( _dataRange.isEmpty() )  _dataRange = Vec2f( 0.0f );
}

//------------------------------------------------------------------------------
//!
void
EventProfileViewer::updateCanvas()
{
   Vec2f pos = globalPosition() + actualSize()*0.5f;
   Vec2f s = actualSize() * _zoom;
   s.x /= _dataRange.width();
   //s.y /= 32.0f; //_dataRange.height();
   _mat = Mat4f(
      Mat4d::translation( pos.x, pos.y, 0.0f ) *
      Mat4d::scaling( s.x, s.y, 1.0f ) *
      Mat4d::translation( -_center.x, -_center.y, 0.0f )
   );

#if 0
   // Text.
   for( size_t i = 0; i < _texts.size(); ++i )
   {
      Vec2f tpos = Vec2f(_texts[i].second->position())*us;
      _texts[i].first->position( pos+tpos );
   }
#endif

   // Legend.
   if( _legendTexts[0].isNull() )
   {
      RCP<Font>        font = data( ResManager::getFont("font/Arimo-Bold") );
      int          fontSize = 12;
      FontCharInfo fontInfo = FontCharInfo( *font, fontSize, true );

      const char* labels[NUM_TYPES];
      labels[TYPE_CORE]      = "Core";
      labels[TYPE_LOOP]      = "Loop";
      labels[TYPE_RENDER]    = "Render";
      labels[TYPE_DISPLAY]   = "Display";
      labels[TYPE_WORLD]     = "World";
      labels[TYPE_ANIMATORS] = "Animators";
      labels[TYPE_BRAINS]    = "Brains";
      labels[TYPE_COMMANDS]  = "Commands";
      labels[TYPE_ACTIONS]   = "Actions";
      labels[TYPE_PHYSICS]   = "Physics";
      CHECK( sizeof(labels)/sizeof(labels[0]) == NUM_TYPES );

      Vec2f lo = Vec2f( 16.0f );  // Bottom left offset for the whole legend.
      Vec2f qs = Vec2f( fontInfo.maxHeight() * 0.75f );   // Size of the sample quad.
      Vec2f qo = Vec2f( 0.0f, (fontInfo.maxHeight() - qs.y) * 0.5f );  // Vertical offset for the sample quad.
      Vec2f to = Vec2f( qs.x + 4.0f, 0.0f );
      float adj = fontInfo.getWidth("m");
      for( uint i = 0; i < NUM_TYPES; ++i )
      {
         Vec2f rowPos = lo + Vec2f( 0.0f, fontInfo.maxHeight()*(NUM_TYPES-i) ); // Row offset.

         // Sample quad.
         const Vec4f& color = _colors[i];
         RCP<TQuad> quad = new TQuad();
         quad->image( _img.ptr() );
         quad->color( color );
         quad->position( rowPos + qo );
         quad->size( qs );
         _legendQuads[i] = quad;

         // Sample label.
         const char*  label = labels[i];
         RCP<Text> text = new Text();
         text->font( font, fontSize, true );
         text->text( label );
         text->color( Vec4f(1.0f) );
         text->position( rowPos + to );
         text->size( Vec2f( fontInfo.getWidth(label)+adj, fontInfo.maxHeight() ) );
         _legendTexts[i] = text;
      }
   }
}

//------------------------------------------------------------------------------
//!
void
EventProfileViewer::onPointerPress( const Event& )
{
}

//------------------------------------------------------------------------------
//!
void
EventProfileViewer::onPointerRelease( const Event& )
{
}

//------------------------------------------------------------------------------
//!
void
EventProfileViewer::onPointerMove( const Event& ev )
{
   const Pointer& p = ev.pointer();
   if( p.pressed() )
   {
      Vec2f d = p.deltaPosition();
      _center.x -= (d.x * _dataRange.width()) / (actualSize().x * _zoom.x);
      _center.y -= (d.y * 1.0f) / (actualSize().y * _zoom.y);
      updateCanvas();
   }
}

//------------------------------------------------------------------------------
//!
void
EventProfileViewer::onPointerScroll( const Event& ev )
{
   Vec2f s = ev.scrollValue()(1,0);
   s.y = -s.y;
   Vec2f v = CGM::max( Vec2f(8.0f), CGM::abs(s)+1 );
   Vec2f z = (v - s) / v;
   _zoom *= z;

   updateCanvas();
}

//------------------------------------------------------------------------------
//!
void
EventProfileViewer::onChar( const Event& ev )
{
   switch( ev.value() )
   {
      case Key::_1:
         _zoom.x = 1.0f;
         updateCanvas();
         break;
      case Key::PERIOD:
         recenter();
         updateCanvas();
         break;
      default:
         //StdErr << "OnChar" << ev.value() << nl;
         break;
   }
}

//------------------------------------------------------------------------------
//!
void
EventProfileViewer::onPointerEnter( const Event& )
{
}

//------------------------------------------------------------------------------
//!
void
EventProfileViewer::onPointerLeave( const Event& )
{
}

//------------------------------------------------------------------------------
//!
void
EventProfileViewer::render( const RCP<Gfx::RenderNode>& rn )
{
   Gfx::Pass& pass = *(rn->current());

   Vec2i scPos   = globalPosition();
   Vec2i scSize  = actualSize();
   const int* sc = pass.addScissor( scPos.x, scPos.y, scSize.x, scSize.y );

   pass.setWorldMatrixPtr( _mat.ptr() );
   pass.setSamplers( _sampler );
   for( uint i = 0; i < NUM_TYPES; ++i )
   {
      pass.setConstants( _constants[i] );
      pass.execGeometry( _geometry[i] );
   }

   // Legend.
   if( _legendVisible )
   {
      if( _legendTexts[0].isValid() )
      {
         for( uint i = 0; i < NUM_TYPES; ++i )
         {
            _legendTexts[i]->draw( rn );
         }
      }
      if( _legendQuads[0].isValid() )
      {
         for( uint i = 0; i < NUM_TYPES; ++i )
         {
            _legendQuads[i]->draw( rn );
         }
      }
   }

   pass.setScissor( sc[0], sc[1], sc[2], sc[3] );
}

//------------------------------------------------------------------------------
//!
void
EventProfileViewer::performSetGeometry()
{
   updateCanvas();
   Widget::performSetGeometry();
}

//------------------------------------------------------------------------------
//!
void
EventProfileViewer::performSetPosition()
{
   updateCanvas();
   Widget::performSetPosition();
}

//------------------------------------------------------------------------------
//!
const char*
EventProfileViewer::meta() const
{
   return _eventProfileViewer_str;
}

//------------------------------------------------------------------------------
//!
void
EventProfileViewer::init( VMState* vm )
{
   Widget::init( vm );
}

//------------------------------------------------------------------------------
//!
bool
EventProfileViewer::performGet( VMState* vm )
{
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_LOAD:
         VM::push( vm, this, loadVM );
         return true;
   }
   return Widget::performGet( vm );
}

//------------------------------------------------------------------------------
//!
bool
EventProfileViewer::performSet( VMState* vm )
{
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_LOAD:
         // Read-only.
         return true;
   }
   return Widget::performSet( vm );
}

//------------------------------------------------------------------------------
//!
bool
EventProfileViewer::isAttribute( const char* name ) const
{
   if( _attributes[ name ] != StringMap::INVALID ) return true;
   return Widget::isAttribute( name );
}

NAMESPACE_END
