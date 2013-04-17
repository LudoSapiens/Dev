/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Widget/RadialMenu.h>
#include <Fusion/VM/VMObjectPool.h>
#include <Fusion/Core/Core.h>
#include <Fusion/Core/Event.h>
#include <Fusion/Core/Key.h>

#include <CGMath/CGMath.h>

#include <Base/ADT/StringMap.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
enum {
   ATTRIB_RADIUS,
   ATTRIB_CENTER,
   ATTRIB_HIGHLIGHT,
   ATTRIB_SELECT,
};

StringMap _attributes(
   "radius",    ATTRIB_RADIUS,
   "center",    ATTRIB_CENTER,
   "highlight", ATTRIB_HIGHLIGHT,
   "select",    ATTRIB_SELECT,
   ""
);

//------------------------------------------------------------------------------
//!
int
highlightVM( VMState* vm )
{
   RadialMenu* radialMenu = (RadialMenu*)VM::thisPtr( vm );
   uint id = VM::toUInt( vm, 1 );
   radialMenu->highlight( id );
   return 0;
}

//------------------------------------------------------------------------------
//!
int
selectVM( VMState* vm )
{
   RadialMenu* radialMenu = (RadialMenu*)VM::thisPtr( vm );
   uint id = VM::toUInt( vm, 1 );
   radialMenu->select( id );
   return 0;
}

//------------------------------------------------------------------------------
//!
Vec2f
computeWidgetPos( float angle, float radius, const Vec2f& size )
{
   float sinAngle = CGM::sin( angle );
   float cosAngle = CGM::cos( angle );
   float y = sinAngle * radius;
   float x = cosAngle * radius;

   Vec2f pos( x, y );

   pos.x += ( cosAngle - 1.0f )*0.5f*size.x;
   //pos.y += ( sinAngle - 1.0f )*0.5f*size.y;
   pos.y +=(CGM::abs(angle*CGConstf::one_pi()-1.5f)-1.0f)*size.y;
   //pos.y -= size.y/2.0f;

   return pos;
}

//------------------------------------------------------------------------------
//!
const char* _radialMenu_str_ = "radialMenu";

UNNAMESPACE_END


NAMESPACE_BEGIN

/*==============================================================================
  CLASS RadialMenu
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
RadialMenu::initialize()
{
   VMObjectPool::registerObject( "UI", _radialMenu_str_, stdCreateVM<RadialMenu>, stdGetVM<RadialMenu>, stdSetVM<RadialMenu> );
}

//------------------------------------------------------------------------------
//!
RadialMenu::RadialMenu()
   : WidgetContainer(),
      _radius( 50.0f ),
      _pressed( false ),
      _currentWidget(NULL)
{
   anchor( ANCHOR_BOTTOM_LEFT );
}

//------------------------------------------------------------------------------
//!
RadialMenu::~RadialMenu()
{}

//------------------------------------------------------------------------------
//!
void
RadialMenu::render( const RCP<Gfx::RenderNode>& rn )
{
   WidgetContainer::render( rn );
}

//------------------------------------------------------------------------------
//!
Vec2f
RadialMenu::performComputeBaseSize()
{
   float angle = CGConstf::pi_2();
   float delta = CGConstf::pi2() / (float)numWidgets();

   Vec2f min( 0.0f, 0.0f );
   Vec2f max( 0.0f, 0.0f );

   for( uint i = 0; i < numWidgets(); ++i, angle += delta )
   {
      Vec2f childSize = widgets()[i]->actualBaseSize();
      Vec2f pos = computeWidgetPos( angle, _radius, childSize );

      CGM::clampMax( min.x, pos.x );
      CGM::clampMax( min.y, pos.y );
      CGM::clampMin( max.x, pos.x + childSize.x );
      CGM::clampMin( max.y, pos.y + childSize.y );
   }
   _center = Vec2f( -min );

   return max-min;
}

//------------------------------------------------------------------------------
//!
void
RadialMenu::performSetGeometry()
{
   float angle = CGConstf::pi_2();
   float delta = CGConstf::pi2() / (float)numWidgets();

   Vec2f min( 0.0f, 0.0f );
   Vec2f max( 0.0f, 0.0f );

   // Compute size.
   for( uint i = 0; i < numWidgets(); ++i, angle += delta )
   {
      Vec2f childSize = widgets()[i]->actualBaseSize();
      Vec2f pos = computeWidgetPos( angle, _radius, childSize );

      CGM::clampMax( min.x, pos.x );
      CGM::clampMax( min.y, pos.y );
      CGM::clampMin( max.x, pos.x + childSize.x );
      CGM::clampMin( max.y, pos.y + childSize.y );
   }

   // Position widgets.
   angle = CGConstf::pi_2();
   for( uint i = 0; i < numWidgets(); ++i, angle += delta )
   {
      Vec2f childSize = widgets()[i]->actualBaseSize();
      Vec2f pos = computeWidgetPos( angle, _radius, childSize );

      widgets()[i]->geometry( globalPosition(), pos-min, childSize );
   }

   _center = -min;
}

//------------------------------------------------------------------------------
//!
void RadialMenu::performSetPosition()
{
   WidgetContainer::performSetPosition();
}

//------------------------------------------------------------------------------
//!
void
RadialMenu::onPointerPress( const Event& ev )
{
   if( _currentWidget ) _currentWidget->onPointerPress( ev );
   _pressed = true;
}

//------------------------------------------------------------------------------
//!
void
RadialMenu::onPointerRelease( const Event& ev )
{
   enableEvents(false);
   Core::releasePointer( this ); // Corresponds to the 'widget.ptr()' in Desktop::modal()
   Core::releaseKeyboard( this );
   _pressed = false;
   enableEvents(true);

   if( _currentWidget )
   {
      _currentWidget->onPointerRelease( ev );
      _currentWidget = 0;
   }

   if( parent() ) parent()->removeWidget( RCP<Widget>( this ) );
}

//------------------------------------------------------------------------------
//!
void
RadialMenu::onPointerMove( const Event& ev )
{
   if( widgets().empty() ) return;

   // Compute position with respect to center.
   Vec2f pos( ev.position() - globalPosition() - _center );
   Widget* currentWidget = 0;

   if( pos.length() > _radius )
   {
      float delta  = CGConstf::pi2()/(float)numWidgets();
      float angle  = CGConstf::pi2() - CGM::atan2( pos.x, pos.y ) + delta*0.5f;
      int widgetNb = int(angle/delta) % numWidgets();
      currentWidget = widgets()[widgetNb].ptr();
   }


   if( currentWidget != _currentWidget )
   {
      if( _currentWidget != 0 )
      {
         _currentWidget->onPointerLeave( Event::PointerLeave(Core::lastTime(), ev.pointerID(), ev.position()) );
      }
      _currentWidget = currentWidget;

      if( _currentWidget != 0 )
      {
         if( _pressed )
         {
            _currentWidget->onPointerEnter( Event::PointerPress(Core::lastTime(), ev.pointerID(), 0, ev.position(), false) );
         }
         else
         {
            _currentWidget->onPointerEnter( Event::PointerEnter(Core::lastTime(), ev.pointerID(), ev.position()) );
         }
      }
   }
}

//------------------------------------------------------------------------------
//!
void
RadialMenu::onKeyPress( const Event& ev )
{
   if( !ev.isRepeated() )
   {
      switch( ev.value() )
      {
         case Key::DIGIT_0:
         case Key::DIGIT_1:
         case Key::DIGIT_2:
         case Key::DIGIT_3:
         case Key::DIGIT_4:
         case Key::DIGIT_5:
         case Key::DIGIT_6:
         case Key::DIGIT_7:
         case Key::DIGIT_8:
         case Key::DIGIT_9:
            highlight( ev.value() - Key::DIGIT_0 );
            break;
         case Key::SPACE:
            select( uint(widgets().size()) ); // deselect
            break;
      }
   }
}

//------------------------------------------------------------------------------
//!
void
RadialMenu::onKeyRelease( const Event& ev )
{
   switch( ev.value() )
   {
      case Key::DIGIT_0:
      case Key::DIGIT_1:
      case Key::DIGIT_2:
      case Key::DIGIT_3:
      case Key::DIGIT_4:
      case Key::DIGIT_5:
      case Key::DIGIT_6:
      case Key::DIGIT_7:
      case Key::DIGIT_8:
      case Key::DIGIT_9:
         select( ev.value() - Key::DIGIT_0 );
         break;
      case Key::ESC:
         select( uint(widgets().size()) );
         break;
   }
}

//------------------------------------------------------------------------------
//!
bool
RadialMenu::isAttribute( const char* name ) const
{
   if( _attributes[ name ] != StringMap::INVALID ) return true;
   return WidgetContainer::isAttribute( name );
}

//------------------------------------------------------------------------------
//!
void
RadialMenu::highlight( const uint widgetIndex )
{
   Vec2f eventPosition;
   if( widgetIndex < (uint)widgets().size() )
   {
      Widget* widgetToSelect = widgets()[widgetIndex].ptr();
      CHECK( widgetToSelect != NULL );
      eventPosition = widgetToSelect->globalPosition() + (widgetToSelect->actualSize() / 2.0f);
   }
   else
   {
      eventPosition = _center + globalPosition();
   }

   // To highlight, simply move the pointer over the button (or center).
   onPointerMove( Event::PointerMove(Core::lastTime(), (uint)-1, eventPosition) );
   onPointerPress( Event::PointerPress(Core::lastTime(), (uint)-1, 0, eventPosition, false) );
}

//------------------------------------------------------------------------------
//!
void
RadialMenu::select( const uint widgetIndex )
{
   Vec2f eventPosition;
   if( widgetIndex < (uint)widgets().size() )
   {
      Widget* widgetToSelect = widgets()[widgetIndex].ptr();
      CHECK( widgetToSelect != NULL );
      eventPosition = widgetToSelect->globalPosition() + (widgetToSelect->actualSize() / 2);
   }
   else
   {
      eventPosition = _center + globalPosition();
   }

   // To highlight, click over the butter (or center).
   onPointerMove( Event::PointerMove(Core::lastTime(), (uint)-1, eventPosition) );
   onPointerPress( Event::PointerPress(Core::lastTime(), (uint)-1, 0, eventPosition, false) );
   onPointerRelease( Event::PointerRelease(Core::lastTime(), (uint)-1, 0, eventPosition, false) );
}

//------------------------------------------------------------------------------
//!
const char*
RadialMenu::meta() const
{
   return _radialMenu_str_;
}

//------------------------------------------------------------------------------
//!
void
RadialMenu::init( VMState* vm )
{
   VM::get( vm, 1, "radius", _radius );

   // Base class init.
   WidgetContainer::init( vm );
}

//------------------------------------------------------------------------------
//!
bool
RadialMenu::performGet( VMState* vm )
{
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_RADIUS:
         VM::push( vm, _radius );
         return true;
      case ATTRIB_CENTER:
         if( !baseSizeCached() )  performComputeBaseSize();
         VM::push( vm, _center );
         return true;
      case ATTRIB_HIGHLIGHT:
         VM::push( vm, this, highlightVM );
         return true;
      case ATTRIB_SELECT:
         VM::push( vm, this, selectVM );
         return true;
      default:
         break;
   }
   return WidgetContainer::performGet( vm );
}

//------------------------------------------------------------------------------
//!
bool
RadialMenu::performSet( VMState* vm )
{
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_RADIUS:
         _radius = VM::toFloat( vm, 3 );
         markForUpdate( true );
         return true;
      case ATTRIB_CENTER:
         return true;
      case ATTRIB_HIGHLIGHT:
         return true;
      case ATTRIB_SELECT:
         return true;
      default: break;
   }
   return WidgetContainer::performSet( vm );
}

NAMESPACE_END
