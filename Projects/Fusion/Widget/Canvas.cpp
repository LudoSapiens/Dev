/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Widget/Canvas.h>
#include <Fusion/VM/VMObjectPool.h>

#include <CGMath/CGMath.h>

#include <Base/ADT/StringMap.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN


//------------------------------------------------------------------------------
//!
inline void 
execute( const VMRef& ref, Widget* widget, const RCP<Widget>& item )
{   
   if( ref.isValid() )
   {
      VMState* vm = ref.vm();
      VM::push( vm, ref );
      VM::pushProxy( vm, widget );
      VM::pushProxy( vm, item.ptr() );
      VM::ecall( vm, 2, 0 );
   }
}

//------------------------------------------------------------------------------
//!
int
getCanvasRectVM( VMState* vm )
{
   Canvas* canvas = (Canvas*)VM::thisPtr( vm );

   VM::push( vm, canvas->canvasRect() );
   
   return 1;
}


//------------------------------------------------------------------------------
//!
enum {
   ATTRIB_GET_CANVAS_RECT,
   ATTRIB_OFFSET,
   ATTRIB_ONMODIFY
};


StringMap _attributes(
   "getCanvasRect",   ATTRIB_GET_CANVAS_RECT,
   "offset",          ATTRIB_OFFSET,
   "onModify",        ATTRIB_ONMODIFY,
   ""
);


//------------------------------------------------------------------------------
//!
const char* _canvas_str_ = "canvas";

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Canvas
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
Canvas::initialize()
{
   VMObjectPool::registerObject( "UI", _canvas_str_, stdCreateVM<Canvas>, stdGetVM<Canvas>, stdSetVM<Canvas> );
}


//------------------------------------------------------------------------------
//!
Canvas::Canvas() 
   : WidgetContainer(),
     _offset(0.0f,0.0f),
     _canvasRectCached( false )
{}

//------------------------------------------------------------------------------
//!
Canvas::~Canvas()
{}

//------------------------------------------------------------------------------
//!
void
Canvas::render( const RCP<Gfx::RenderNode>& rn )
{
   if( isScissored() )
   {
      // Render the widget container.
      Widget::render( rn );
      
      // Render the contained widgets.
      const int* sc = rn->current()->addScissor( (int)_scPos.x, (int)_scPos.y, (int)_scSize.x, (int)_scSize.y );
      Container::ConstIterator it  = _widgets.begin();
      Container::ConstIterator end = _widgets.end();
      
      for( ; it != end; ++it )
      {   
         if( !(*it)->hidden() ) (*it)->render( rn );
      }
      rn->current()->setScissor( sc[0], sc[1], sc[2], sc[3] );
   }
   else
   {
      WidgetContainer::render( rn );
   }
}

//------------------------------------------------------------------------------
//!
bool
Canvas::isAttribute
( const char* name ) const
{
   if( _attributes[ name ] != StringMap::INVALID ) return true;
   return WidgetContainer::isAttribute( name );
}


//------------------------------------------------------------------------------
//!
Vec4f
Canvas::canvasRect() const
{
   if( !_canvasRectCached )
   {
      Container::ConstIterator it  = _widgets.begin();
      Container::ConstIterator end = _widgets.end();

       _canvasRect = Vec4f(
          _offset.x,
          _offset.y,
          _offset.x + actualSize().x,
          _offset.y + actualSize().y
       );         
      
      for( ; it != end; ++it )
      {
         if( (*it)->hidden() ) continue;
         CGM::clampMax( _canvasRect.x, (*it)->absPosition().x );
         CGM::clampMax( _canvasRect.y, (*it)->absPosition().y );
         CGM::clampMin( _canvasRect.z, (*it)->absPosition().x + (*it)->actualSize().x );
         CGM::clampMin( _canvasRect.w, (*it)->absPosition().y + (*it)->actualSize().y );
      }

      _canvasRectCached = true;
   }

   return _canvasRect;
}

//------------------------------------------------------------------------------
//!
void
Canvas::offset( const Vec2f& val )
{
   if( val != _offset )
   {  
      _offset = val;
      modified();
      markForUpdate();
   }
}


//------------------------------------------------------------------------------
//!
Vec2f
Canvas::performComputeBaseSize()
{
   return Vec2f( 0.0f, 0.0f );
}


//------------------------------------------------------------------------------
//!
void
Canvas::performSetGeometry()
{
   Container::ConstIterator it  = _widgets.begin();
   Container::ConstIterator end = _widgets.end();

   for( ; it != end; ++it )
   {
      if( (*it)->hidden() ) continue;

      (*it)->geometry(
         globalPosition() - _offset,
         (*it)->localPosition(),
         (*it)->actualBaseSize()
      );
   }
   
   // Ignores flexibility of child      
   _canvasRectCached = false;
   canvasRect();
   modified();
   
   _scPos.x  =  globalPosition().x + border().x;
   _scPos.y  =  globalPosition().y + border().y;
   _scSize.x =  actualSize().x - border().x - border().z;
   _scSize.y =  actualSize().y - border().y - border().w;
}


//------------------------------------------------------------------------------
//!
void
Canvas::performSetPosition()
{
   Container::ConstIterator it  = _widgets.begin();
   Container::ConstIterator end = _widgets.end();
  
   for( ; it != end; ++it )
   {
      if( !(*it)->hidden() )
      {
          (*it)->position( globalPosition() - _offset, (*it)->localPosition() );
      }
   }

   _scPos.x  =  globalPosition().x + border().x;
   _scPos.y  =  globalPosition().y + border().y;
}


//------------------------------------------------------------------------------
//!
bool
Canvas::isScissored() const
{

   Vec4f rect = canvasRect();

   if( _offset.x > rect.x || _offset.y > rect.y ||
       _offset.x + actualSize().x < rect.z || 
       _offset.y + actualSize().y < rect.w )
   {      
      return true;
   }

   return false;
}

//------------------------------------------------------------------------------
//!
void
Canvas::modified()
{
   _onModify.exec( this );
   execute( _onModifyRef, this, this );
}


//------------------------------------------------------------------------------
//!
const char*
Canvas::meta() const
{
   return _canvas_str_;
}

//------------------------------------------------------------------------------
//!
void
Canvas::init( VMState* vm )
{
   VM::get( vm, 1, "offset",   _offset );
   VM::get( vm, 1, "onModify", _onModifyRef );
   
   // Base class init.
   WidgetContainer::init( vm );
}

//------------------------------------------------------------------------------
//!
bool
Canvas::performGet( VMState* vm )
{
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_GET_CANVAS_RECT:
         VM::push( vm, this, getCanvasRectVM );
         return true;
      case ATTRIB_OFFSET:
         VM::push( vm, _offset );
         return true;
      case ATTRIB_ONMODIFY:
         VM::push( vm, _onModifyRef );
         return true;
         
      default: break;
   }
   
   return WidgetContainer::performGet( vm );
}

//------------------------------------------------------------------------------
//!
bool
Canvas::performSet( VMState* vm )
{
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_GET_CANVAS_RECT:
         return true; // read-only
      case ATTRIB_OFFSET:
         offset( VM::toVec2f( vm, 3 ) );
         return true;
      case ATTRIB_ONMODIFY:
         VM::toRef( vm, 3, _onModifyRef );
         return true;
         
      default: break;
   }

   return WidgetContainer::performSet( vm );
}


NAMESPACE_END
