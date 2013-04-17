/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Widget/Splitter.h>
#include <Fusion/VM/VMObjectPool.h>
#include <Fusion/Core/Event.h>
#include <Fusion/Core/Core.h>

#include <CGMath/CGMath.h>

#include <Base/ADT/StringMap.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

enum {
   ATTRIB_GAP,
   ATTRIB_ORIENTATION,
   ATTRIB_RATIO,
   ATTRIB_SPLIT_POSITION
};

StringMap _attributes(
   "gap",            ATTRIB_GAP,
   "orientation",    ATTRIB_ORIENTATION,
   "ratio",          ATTRIB_RATIO,
   "splitPosition",  ATTRIB_SPLIT_POSITION,
   ""
);

//------------------------------------------------------------------------------
//!
const char* _splitter_str_ = "splitter";

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Splitter
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
Splitter::initialize()
{
   VMObjectPool::registerObject( "UI", _splitter_str_, stdCreateVM<Splitter>, stdGetVM<Splitter>, stdSetVM<Splitter> );
}

//------------------------------------------------------------------------------
//!
Splitter::Splitter()
   : WidgetContainer(),
     _gap( 0.0f ),
     _orient( HORIZONTAL ),
     _ratio( 0.5f ),
     _lock( false )
{
   // Set the maximum allowable widget in container.
   _maxWidgets = 2;
   intangible( false );
}

//------------------------------------------------------------------------------
//!
Splitter::~Splitter()
{}

//------------------------------------------------------------------------------
//!
void
Splitter::ratio( float val )
{
   if( val == _ratio ) return;

   _ratio = CGM::clamp( val, 0.0f, 1.0f );
   markForUpdate();
}

//------------------------------------------------------------------------------
//!
void
Splitter::render( const RCP<Gfx::RenderNode>& rn )
{
   // Render the contained widgets.
   auto it  = _widgets.begin();
   auto end = _widgets.end();

   for( ; it != end; ++it ) if( !(*it)->hidden() ) (*it)->render( rn );

   // Render the widget container.
   Widget::render( rn );
}

//------------------------------------------------------------------------------
//!
void
Splitter::onPointerMove( const Event& ev )
{
   if( _lock )
   {
      int axis   = _orient == HORIZONTAL ? 0 : 1;

      float size  = actualSize()( axis )  - border()( axis ) - border()( axis + 2 );
      float delta = ev.position()( axis ) - border()( axis ) - globalPosition()( axis );
      float ratio = delta / size;

      if( axis == 0 )
      {
         if( anchor() & INVERT_X ) ratio = 1.0f - ratio;
      }
      else
      {
         if( anchor() & INVERT_Y ) ratio = 1.0f - ratio;
      }

      this->ratio( ratio );
   }
   else
   {
      if( _orient == HORIZONTAL )
      {
         Core::pointerIcon( ev.pointerID(), Pointer::SIZE_L );
      }
      else
      {
         Core::pointerIcon( ev.pointerID(), Pointer::SIZE_T );
      }
   }

   WidgetContainer::onPointerMove( ev );
}

//------------------------------------------------------------------------------
//!
void
Splitter::onPointerPress( const Event& ev )
{
   // Handle left click for dragging value.
   if( ev.value() == 1 )
   {
      int axis = _orient == HORIZONTAL ? 0 : 1;
      float delta = _splitPos( axis ) + globalPosition()(axis) - ev.position()( axis );

      if( delta <= _gap/2.0f && delta > -_gap/2.0f )
      {
         _lock = true;
         Core::grabPointer( ev.pointerID(), this );
         _state |= PRESSED;
         callShader();
      }
   }
   else
   {
      WidgetContainer::onPointerPress( ev );
   }
}

//------------------------------------------------------------------------------
//!
void
Splitter::onPointerRelease( const Event& ev )
{
   if( ev.value() == 1 )
   {
      _lock = false;
      Core::releasePointer( this );
      _state &= ~PRESSED;
      callShader();
   }
   else
   {
      WidgetContainer::onPointerRelease( ev );
   }
}

//------------------------------------------------------------------------------
//!
Vec2f
Splitter::performComputeBaseSize()
{
   WidgetContainer::Container::ConstIterator it  = _widgets.begin();
   WidgetContainer::Container::ConstIterator end = _widgets.end();

   int axis1 = _orient == HORIZONTAL ? 0 : 1;
   int axis2 = _orient == VERTICAL ? 0 : 1;

   Vec2f newBaseSize( 0.0f, 0.0f );

   CHECK( _widgets.size() == 2 );
   for( ; it != end; ++it )
   {
      if( !(*it)->hidden() )
      {
         Vec2f childSize = (*it)->actualBaseSize();

         newBaseSize( axis1 ) += childSize( axis1 );

         CGM::clampMin( newBaseSize( axis2 ), childSize( axis2 ) );
      }
   }

   // Add gap.
   newBaseSize( axis1 ) += _gap;

   // Add border.
   newBaseSize.x += border().x + border().z;
   newBaseSize.y += border().y + border().w;

   return newBaseSize;
}

//------------------------------------------------------------------------------
//!
void
Splitter::performSetGeometry()
{
   // Set first widget geometry.
   if( _widgets.empty()  ) return;

   int axis = _orient == HORIZONTAL ? 0 : 1;

   // Compute the size left to both widgets.
   Vec2f size(
      actualSize() - Vec2f( border().x + border().z, border().y + border().w )
   );
   size( axis ) -= _gap;

   // Position and size of first widget.
   Vec2f newPos( border()( (anchor()&INVERT_X)?2:0 ), border()(  (anchor()&INVERT_Y)?3:1 ) );

   Vec2f newSize = size;
   newSize( axis ) = newSize( axis ) * _ratio ;

   // Compute split position.
   _splitPos( 1-axis ) = 0.0f;
   _splitPos( axis )   = newPos( axis ) + newSize( axis ) + _gap/2.0f;
   if( axis == 0 )
   {
      if( anchor() & INVERT_X ) _splitPos.x = actualSize().x - _splitPos.x;
   }
   else
   {
      if( anchor() & INVERT_Y ) _splitPos.y = actualSize().y - _splitPos.y;
   }

   if( !_widgets[0]->hidden() )
   {
      _widgets[0]->geometry( globalPosition(), newPos, newSize );
   }

   // Set second widget geometry.
   if( _widgets.size() == 1 || _widgets[1]->hidden() ) return;

   newPos( axis ) += newSize( axis ) + _gap;
   newSize( axis ) = size( axis ) - newSize( axis );

    _widgets[1]->geometry( globalPosition(), newPos, newSize );
}

//------------------------------------------------------------------------------
//!
bool
Splitter::isAttribute( const char* name ) const
{
   if( _attributes[ name ] != StringMap::INVALID ) return true;
   return WidgetContainer::isAttribute( name );
}

//------------------------------------------------------------------------------
//!
const char*
Splitter::meta() const
{
   return _splitter_str_;
}

//------------------------------------------------------------------------------
//!
void
Splitter::init( VMState* vm )
{
   VM::get( vm, 1, "gap", _gap );
   VM::get( vm, 1, "orientation", _orient );
   VM::get( vm, 1, "ratio", _ratio );

   _ratio = CGM::clamp( _ratio, 0.0f, 1.0f );

   // Base class init.
   WidgetContainer::init( vm );
}

//------------------------------------------------------------------------------
//!
bool
Splitter::performGet( VMState* vm )
{
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_GAP:
         VM::push( vm, _gap );
         return true;
      case ATTRIB_ORIENTATION:
         VM::push( vm, _orient );
         return true;
      case ATTRIB_RATIO:
         VM::push( vm, _ratio );
         return true;
      case ATTRIB_SPLIT_POSITION:
         VM::push( vm, _splitPos );
         return true;
      default: break;
   }

   return WidgetContainer::performGet( vm );
}

//------------------------------------------------------------------------------
//!
bool
Splitter::performSet( VMState* vm )
{
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_GAP:
         _gap = VM::toFloat( vm, 3 );
         markForUpdate();
         return true;
      case ATTRIB_ORIENTATION:
         _orient = VM::toInt( vm, 3 );
         markForUpdate();
         return true;
      case ATTRIB_RATIO:
         ratio( VM::toFloat( vm, 3 ) );
         markForUpdate();
         return true;
      case ATTRIB_SPLIT_POSITION: // Read only.
         return true;
      default: break;
   }

   return WidgetContainer::performSet( vm );
}

NAMESPACE_END
