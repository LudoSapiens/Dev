/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Widget/Widget.h>

#include <Fusion/Core/Core.h>
#include <Fusion/Core/Event.h>
#include <Fusion/Drawable/Text.h>
#include <Fusion/Drawable/TQuad.h>
#include <Fusion/VM/VMObjectPool.h>
#include <Fusion/VM/VMRegistry.h>
#include <Fusion/Widget/WidgetContainer.h>

#include <Base/ADT/StringMap.h>
#include <Base/Dbg/DebugStream.h>
#include <Base/Msg/Subject.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

DBG_STREAM( os_w, "Widget" );

//------------------------------------------------------------------------------
//!
const char*  _widget_str_ = "widget";

const VM::EnumReg _enumsWidgetAlignment[] = {
   { "FLEX",    Widget::FLEX   },
   { "START",   Widget::START  },
   { "MIDDLE",  Widget::MIDDLE },
   { "END",     Widget::END    },
   { 0, 0 }
};

const VM::EnumReg _enumsWidgetState[] = {
   { "ENABLED",  Widget::ENABLED },
   { "HOVERED",  Widget::HOVERED },
   { "PRESSED",  Widget::PRESSED },
   { "HIDDEN",   Widget::HIDDEN  },
   { 0, 0 }
};

//------------------------------------------------------------------------------
//!
void
initVM( VMState* vm, uint /*mask*/ )
{
   VM::registerEnum( vm, "UI.WidgetAlignment", _enumsWidgetAlignment );
   VM::registerEnum( vm, "UI.WidgetState", _enumsWidgetState );
}

//------------------------------------------------------------------------------
//!
inline void
setDefaultFunction( VMState* vm, VMRef& function, const char* tableName, const char* typeName )
{
   if( !function.isValid() )
   {
      // Get the main ui table.
      VM::getGlobal( vm, "UI" );              //[...,UI]

      // Get the shaders function table.
      VM::push( vm, tableName );              //[...,UI,name]
      VM::get( vm, -2 );                      //[...,UI,function]

      // Get the shader function.
      if( !VM::isNil( vm, -1 ) ) VM::get( vm, -1, typeName, function );

      // Remove the shaders and ui tables.
      VM::pop( vm, 2 );
   }
}

//------------------------------------------------------------------------------
//!
inline void
execute( const VMRef& ref, Widget* widget, const Event& ev )
{
   if( ref.isValid() )
   {
      VMState* vm = ref.vm();
      VM::push( vm, ref );
      VM::pushProxy( vm, widget );
      VM::push( vm, ev );
      VM::ecall( vm, 2, 0 );
   }
}

//------------------------------------------------------------------------------
//!
inline void
execute( const VMRef& ref, Widget* widget )
{
   if( ref.isValid() )
   {
      VMState* vm = ref.vm();
      VM::push( vm, ref );
      VM::pushProxy( vm, widget );
      VM::ecall( vm, 1, 0 );
   }
}

//------------------------------------------------------------------------------
//!
int
toFrontVM( VMState * vm )
{
   Widget* w = (Widget*)VM::thisPtr( vm );
   w->toFront();
   return 0;
}

//------------------------------------------------------------------------------
//!
int
toBackVM( VMState * vm )
{
   Widget* w = (Widget*)VM::thisPtr( vm );
   w->toBack();
   return 0;
}

//------------------------------------------------------------------------------
//!
int
closeVM( VMState * vm )
{
   Widget* w = (Widget*)VM::thisPtr( vm );
   w->close();
   return 0;
}

//------------------------------------------------------------------------------
//!
int
resizeVM( VMState * vm )
{
   Widget* widget = (Widget*)VM::thisPtr( vm );

   widget->resize( Vec4f( VM::toFloat( vm, 1 ),
                          VM::toFloat( vm, 2 ),
                          VM::toFloat( vm, 3 ),
                          VM::toFloat( vm, 4 ) ) );

   return 0;
}

//------------------------------------------------------------------------------
//!
int
resizeWVM( VMState * vm )
{
   Widget* widget = (Widget*)VM::thisPtr( vm );
   widget->resizeW( Vec2f( VM::toFloat( vm, 1 ), VM::toFloat( vm, 2 ) ) );

   return 0;
}

//------------------------------------------------------------------------------
//!
int
resizeHVM( VMState * vm )
{
   Widget* widget = (Widget*)VM::thisPtr( vm );
   widget->resizeH( Vec2f( VM::toFloat( vm, 1 ), VM::toFloat( vm, 2 ) ) );

   return 0;
}

//------------------------------------------------------------------------------
//!
int
findParentVM( VMState* vm )
{
   Widget* widget = (Widget*)VM::thisPtr( vm );
   String id      = VM::toString( vm, 1 );

   VM::pushProxy( vm, widget->findParent( id ) );

   return 1;
}

//------------------------------------------------------------------------------
//!
int
observeVM( VMState* vm )
{
   Widget* widget = (Widget*)VM::thisPtr( vm );
   Subject* subject = (Subject*)VM::toPtr( vm, 1 );
   widget->observe( subject );
   return 0;
}

//------------------------------------------------------------------------------
//!
int
getCenterGlobalVM( VMState* vm )
{
   Widget* widget = (Widget*)VM::thisPtr( vm );
   VM::push( vm, widget->getCenterGlobal() );
   return 1;
}

//------------------------------------------------------------------------------
//!
int
getCenterLocalVM( VMState* vm )
{
   Widget* widget = (Widget*)VM::thisPtr( vm );
   VM::push( vm, widget->getCenterLocal() );
   return 1;
}


//------------------------------------------------------------------------------
//!
enum {
   ATTRIB_STATE,
   ATTRIB_ID,
   ATTRIB_POSITION,
   ATTRIB_ABS_POSITION,
   ATTRIB_SIZE,
   ATTRIB_FLEX,
   ATTRIB_ONDELETE,
   ATTRIB_ONRESIZE,
   ATTRIB_ONCLICK,
   ATTRIB_ONPOINTER_PRESS,
   ATTRIB_ONPOINTER_RELEASE,
   ATTRIB_ONPOINTER_MOVE,
   ATTRIB_ONPOINTER_ENTER,
   ATTRIB_ONPOINTER_LEAVE,
   ATTRIB_ONPOINTER_CANCEL,
   ATTRIB_ONPOINTER_SCROLL,
   ATTRIB_ONKEY_PRESS,
   ATTRIB_ONKEY_RELEASE,
   ATTRIB_ONNOTIFY,
   ATTRIB_ONCHAR,
   ATTRIB_ONACCELERATE,
   ATTRIB_ONFOCUS_GRAB,
   ATTRIB_ONFOCUS_LOSE,
   ATTRIB_SHADER,
   ATTRIB_BASE_SIZE,
   ATTRIB_ACTUAL_BASE_SIZE,
   ATTRIB_ACTUAL_SIZE,
   ATTRIB_GLOBAL_POSITION,
   ATTRIB_GET_CENTER_GLOBAL,
   ATTRIB_GET_CENTER_LOCAL,
   ATTRIB_ENABLED,
   ATTRIB_HOVERED,
   ATTRIB_PRESSED,
   ATTRIB_HIDDEN,
   ATTRIB_ACCELEROMETER,
   ATTRIB_EVENTS_ENABLED,
   ATTRIB_PARENT,
   ATTRIB_HOVER_ICON,
   ATTRIB_TEXTQUAD,
   ATTRIB_TO_FRONT,
   ATTRIB_TO_BACK,
   ATTRIB_TQUAD,
   ATTRIB_CLOSE,
   ATTRIB_RESIZE,
   ATTRIB_RESIZEW,
   ATTRIB_RESIZEH,
   ATTRIB_FINDPARENT,
   ATTRIB_OBSERVE,
   ATTRIB_HALIGN,
   ATTRIB_VALIGN
};

StringMap _attributes(
   "state",              ATTRIB_STATE,
   "id",                 ATTRIB_ID,
   "position",           ATTRIB_POSITION,
   "absPosition",        ATTRIB_ABS_POSITION,
   "size",               ATTRIB_SIZE,
   "flex",               ATTRIB_FLEX,
   "onDelete",           ATTRIB_ONDELETE,
   "onResize",           ATTRIB_ONRESIZE,
   "onClick",            ATTRIB_ONCLICK,
   "onPointerPress",     ATTRIB_ONPOINTER_PRESS,
   "onPointerRelease",   ATTRIB_ONPOINTER_RELEASE,
   "onPointerMove",      ATTRIB_ONPOINTER_MOVE,
   "onPointerEnter",     ATTRIB_ONPOINTER_ENTER,
   "onPointerLeave",     ATTRIB_ONPOINTER_LEAVE,
   "onPointerCancel",    ATTRIB_ONPOINTER_CANCEL,
   "onPointerScroll",    ATTRIB_ONPOINTER_SCROLL,
   "onKeyPress",         ATTRIB_ONKEY_PRESS,
   "onKeyRelease",       ATTRIB_ONKEY_RELEASE,
   "onNotify",           ATTRIB_ONNOTIFY,
   "onChar",             ATTRIB_ONCHAR,
   "onAccelerate",       ATTRIB_ONACCELERATE,
   "onFocusGrab",        ATTRIB_ONFOCUS_GRAB,
   "onFocusLose",        ATTRIB_ONFOCUS_LOSE,
   "shader",             ATTRIB_SHADER,
   "baseSize",           ATTRIB_BASE_SIZE,
   "actualBaseSize",     ATTRIB_ACTUAL_BASE_SIZE,
   "actualSize",         ATTRIB_ACTUAL_SIZE,
   "globalPosition",     ATTRIB_GLOBAL_POSITION,
   "getCenterGlobal",    ATTRIB_GET_CENTER_GLOBAL,
   "getCenterLocal",     ATTRIB_GET_CENTER_LOCAL,
   "enabled",            ATTRIB_ENABLED,
   "hovered",            ATTRIB_HOVERED,
   "pressed",            ATTRIB_PRESSED,
   "hidden",             ATTRIB_HIDDEN,
   "eventsEnabled",      ATTRIB_EVENTS_ENABLED,
   "accelerometer",      ATTRIB_ACCELEROMETER,
   "parent",             ATTRIB_PARENT,
   "hoverIcon",          ATTRIB_HOVER_ICON,
   "textQuad",           ATTRIB_TEXTQUAD,
   "toFront",            ATTRIB_TO_FRONT,
   "toBack",             ATTRIB_TO_BACK,
   "tquad",              ATTRIB_TQUAD,
   "close",              ATTRIB_CLOSE,
   "resize",             ATTRIB_RESIZE,
   "resizeW",            ATTRIB_RESIZEW,
   "resizeH",            ATTRIB_RESIZEH,
   "findParent",         ATTRIB_FINDPARENT,
   "observe",            ATTRIB_OBSERVE,
   "hAlign",             ATTRIB_HALIGN,
   "vAlign",             ATTRIB_VALIGN,
   ""
);

//------------------------------------------------------------------------------
//!
int
tquad_create( VMState* vm )
{
   Widget* w    = (Widget*)VM::thisPtr( vm );
   RCP<TQuad> d = new TQuad();
   d->init( vm );
   w->add( d );
   VM::pushProxy( vm, d.ptr() );
   return 1;
}

//------------------------------------------------------------------------------
//!
int
text_create( VMState* vm )
{
   Widget* w   = (Widget*)VM::thisPtr( vm );
   RCP<Text> d = new Text();
   d->init( vm );
   w->add( d );
   VM::pushProxy( vm, d.ptr() );
   return 1;
}

//------------------------------------------------------------------------------
//!

// Since there is only one widget in init at a time, it is ok to use
// a global variable.
bool inInit = false;

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Widget
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
Widget::initialize()
{
   VMRegistry::add( initVM, VM_CAT_APP );
   VMObjectPool::registerObject( "UI", _widget_str_, nullptr, stdGetVM<Widget>, stdSetVM<Widget> );
}

//------------------------------------------------------------------------------
//!
Widget::Widget()
   : _state( ENABLED ),
     _fields(0),
     _parent( NULL ),
     _globalPos( 0.0f, 0.0f ),
     _localPos( 0.0f, 0.0f ),
     _actualSize( 0.0f, 0.0f ),
     _size( -1.0f, -1.0f ), // Auto
     _actualBaseSize( 0.0f, 0.0f ),
     _flex( 0.0f ),
     _subject( NULL )
{
   // needUpdate
   popup( false, false );
   needUpdate( true );
   baseSizeCached( false );
   clickType( ClickOnUp );
   hoverIcon( Pointer::DEFAULT );
}

//------------------------------------------------------------------------------
//!
Widget::~Widget()
{
   accelerometer( false );
   if( _subject ) _subject->detach( this );
   _onDelete.exec( this );
   execute( _onDeleteRef, this );
   // WARNING: When onDelete is called in Lua, we use the Widget meta-table
   // meaning only Widget-specific things will be available.
   // (the subclass' destructor was called, and the vtable is therefore updated,
   // so this->meta() will now return _widget_str).
}

//------------------------------------------------------------------------------
//!
void
Widget::render( const RCP<Gfx::RenderNode>& rn )
{
   Vector< Drawable* >::ConstIterator it  = _activeDrawables.begin();
   Vector< Drawable* >::ConstIterator end = _activeDrawables.end();

   for( ; it != end; ++it ) (*it)->draw( rn );
}

//------------------------------------------------------------------------------
//!
void
Widget::callShader()
{
   if( _shaderRef.isValid() )
   {
      VMState* vm = _shaderRef.vm();

      // Get the current position in the stack.
      int stackPos = VM::getTop( vm );

      // Call shader function.
      VM::push( vm, _shaderRef );
      VM::pushProxy( vm, this );
      VM::ecall( vm, 1, VM::MULTRET );

      // Clear the current drawable.
      _activeDrawables.clear();

      // Read returned drawables.
      int nbDrawables = VM::getTop( vm ) - stackPos;
      for ( int i = 0; i < nbDrawables; ++i )
      {
         Drawable* d = (Drawable*)VM::toProxy( vm, stackPos + i+1 );
         if( d != NULL )  _activeDrawables.pushBack( d );
      }
      VM::setTop( vm, stackPos );
   }
}

//------------------------------------------------------------------------------
//!
void
Widget::updateLook()
{
   callShader();

   // Compute new base size and call for an update on geometry if the size
   // changed.
   Vec2f oldBaseSize = _actualBaseSize;
   baseSizeCached( false );
   if( actualBaseSize() != oldBaseSize ) markForUpdate();
}

//------------------------------------------------------------------------------
//!
Widget*
Widget::getWidgetAt( const Vec2f& /*pos*/ )
{
   return this;
}

//------------------------------------------------------------------------------
//!
void
Widget::getWidgetsAt( const Vec2f&, Vector<Widget*>& w )
{
   w.pushBack( this );
}

//------------------------------------------------------------------------------
//!
Widget*
Widget::findWidget( const String& id )
{
   if( id == _id ) return this;

   return NULL;
}

//------------------------------------------------------------------------------
//!
void
Widget::sendParentMessage( Widget* widget, int message )
{
   // Default implementation only forward calls the to parent container.
   if( parent() ) parent()->sendParentMessage( widget, message );
}

//------------------------------------------------------------------------------
//!
void
Widget::sendChildMessage( Widget*, int )
{
}

//------------------------------------------------------------------------------
//!
Widget*
Widget::findParent( const String& wid )
{
   if( wid == id() ) return this;
   if( parent() )    return parent()->findParent( wid );

   return NULL;
}

//------------------------------------------------------------------------------
//!
Vec2f
Widget::actualBaseSize()
{
   // Cached?
   if( baseSizeCached() ) return _actualBaseSize;

   if( _size.x >= 0.0f && _size.y >= 0.0f )
   {
      baseSizeCached( true );
      _actualBaseSize = _size;
      return _size;
   }

   _actualBaseSize = performComputeBaseSize();

   // Merge with default base size.
   if( _size.x >= 0.0f ) _actualBaseSize.x = _size.x;
   if( _size.y >= 0.0f ) _actualBaseSize.y = _size.y;

   baseSizeCached( true );

   return _actualBaseSize;
}

//------------------------------------------------------------------------------
//!
Vec2f
Widget::getCenterGlobal() const
{
   Vec2f s = actualSize() * 0.5f;
   return globalPosition() + s;
}

//------------------------------------------------------------------------------
//!
Vec2f
Widget::getCenterLocal() const
{
   Vec2f s = actualSize() * 0.5f;
   return localPosition() + s;
}

//------------------------------------------------------------------------------
//!
Vec2f
Widget::screenToLayer( const Vec2f& pos ) const
{
   if( _parent != NULL )
   {
      return _parent->screenToLayer( pos );
   }
   else
   {
      return pos;
   }
}

//------------------------------------------------------------------------------
//!
Vec2f
Widget::layerToScreen( const Vec2f& pos ) const
{
   if( _parent != NULL )
   {
      return _parent->layerToScreen( pos );
   }
   else
   {
      return pos;
   }
}

//------------------------------------------------------------------------------
//!
void
Widget::geometry( const Vec2f& global, const Vec2f& pos, const Vec2f& size )
{
   // Do we need to update the size?
   if( !needUpdate() && size == _actualSize )
   {
      position( global, pos );
      return;
   }

   bool sizeChanged = (_actualSize != size);

   // Update values.
   _actualSize = size;
   _localPos   = pos;
   _globalPos  = global + absPosition();

   // Call derived class implementation.
   needUpdate( false );
   performSetGeometry();

   // Update the look of the widget.
   callShader();

   if( sizeChanged ) onResize();
}

//------------------------------------------------------------------------------
//!
void
Widget::position( const Vec2f& global, const Vec2f& pos )
{
   // Do we need to update the position?
   if( !needUpdate() && ( pos == _localPos ) && ( global + absPosition() == _globalPos ) )
   {
      return;
   }

   // Update positions.
   _localPos  = pos;
   _globalPos = global + absPosition();

   // Call derived class implementation.
   needUpdate( false );
   performSetPosition();

   // Update the look of the widget.
   callShader();
}

//------------------------------------------------------------------------------
//!
void
Widget::markForUpdate( bool markBaseSize )
{
   needUpdate( true );

   if( markBaseSize ) baseSizeCached( false );
   if( _parent ) _parent->markForUpdate( true  );
}

//------------------------------------------------------------------------------
//!
Vec2f
Widget::absPosition() const
{
   return _parent ? _parent->localToAbsPosition( *this ) : _localPos;
}

//------------------------------------------------------------------------------
//!
void
Widget::absPosition( const Vec2f& pos )
{
   if( !_parent )
   {
      _localPos = pos;
   }
   else
   {
      _localPos = _parent->absToLocalPosition( *this, pos );
   }
   markForUpdate();
}

//------------------------------------------------------------------------------
//!
void
Widget::resize( const Vec4f& delta )
{
   float delx = delta.x + delta.z;
   float dely = delta.y + delta.w;

   if( _size.x < 0.0f ) _size.x = _actualSize.x;
   if( _size.y < 0.0f ) _size.y = _actualSize.y;

   _size.x = _size.x + delx;
   _size.y = _size.y + dely;

   if( !_parent )
   {
      _localPos -= Vec2f( delta.x, delta.y );
   }
   else
   {
      _localPos -= Vec2f( delta( (_parent->anchor() & WidgetContainer::INVERT_X) ? 2 : 0 ),
                          delta( (_parent->anchor() & WidgetContainer::INVERT_Y) ? 3 : 1 ) );
   }
   markForUpdate( true );
}

//------------------------------------------------------------------------------
//!
void
Widget::resizeW( const Vec2f& delta )
{
   float delx = delta.x + delta.y;

   if( _size.x < 0.0f ) _size.x = _actualSize.x;

   _size.x = _size.x + delx;

   if( !_parent )
   {
      _localPos.x -= delta.x;
   }
   else
   {
      _localPos.x -= delta( (_parent->anchor() & WidgetContainer::INVERT_X) ? 1 : 0 );
   }
   markForUpdate( true );
}

//------------------------------------------------------------------------------
//!
void
Widget::resizeH( const Vec2f& delta )
{
   float dely = delta.x + delta.y;

   if( _size.y < 0.0f ) _size.y = _actualSize.y;

   _size.y = _size.y + dely;

   if( !_parent )
   {
      _localPos.y -= delta.y;
   }
   else
   {
      _localPos.y -= delta( (_parent->anchor() & WidgetContainer::INVERT_Y) ? 1 : 0 );
   }
   markForUpdate( true );
}

//------------------------------------------------------------------------------
//!
void
Widget::enable( bool flag )
{
   if( enabled() != flag )
   {
      if( flag )
      {
         _state |= ENABLED;
      }
      else
      {
         // Reset a number of flags
         _state &= ~(ENABLED | HOVERED | PRESSED);

         // Release pointer if we have a grab on it.
         Core::releasePointer( this );
      }
      markForUpdate();
   }
}

//------------------------------------------------------------------------------
//!
void
Widget::hide( bool flag )
{
   if( flag != ( ( _state & HIDDEN ) != 0 ) )
   {
      if( flag )
      {
         // Reset a number of flags
         _state &= ~(HOVERED | PRESSED);

         // Release pointer if we have a grab on it.
         // TODO: One of our child could have a grab on it, though, and will
         //       keep on having it even if after the parent is hidden
         //       This problem can be seen with scrollCanvas's hiding scrollbars
         // TODO: Should we send a POINTER_RELEASE event?
         // TODO: If we release the lock while a mouse button is pressed, that means
         //       a widget could get a POINTER_RELEASE event without a prior POINTER_PRESS
         //       event, and the ClickOnDownUp policy wont work in this specific case.
         Core::releasePointer( this );
      }

      _state ^= HIDDEN;
      markForUpdate();
   }
}

//------------------------------------------------------------------------------
//!
void
Widget::accelerometer( bool val )
{
   if( val != accelerometer() )
   {
      _fields = setbits( _fields, 21, 1, val);
      if( val )
      {
         Core::registerAccelerometer( makeDelegate( this, &Widget::handleEvent ) );
      }
      else
      {
         Core::unregisterAccelerometer( makeDelegate( this, &Widget::handleEvent ) );
      }
   }
}

//------------------------------------------------------------------------------
//!
void
Widget::toFront() const
{
   if( _parent ) _parent->moveToFront( this );
}

//------------------------------------------------------------------------------
//!
void
Widget::toBack() const
{
   if( _parent ) _parent->moveToBack( this );
}

//------------------------------------------------------------------------------
//!
void
Widget::close()
{
   if( _parent ) _parent->removeWidget( this );
}

//------------------------------------------------------------------------------
//!
void
Widget::popup( bool state, bool call )
{
   _fields = setbits( _fields, 0, 1, state );
   if( call ) _onPopup.exec( this );
}


//------------------------------------------------------------------------------
//!
bool
Widget::belongToPopup() const
{
   return isPopup() || ( _parent != 0 && _parent->belongToPopup() );
}

//------------------------------------------------------------------------------
//!
void
Widget::handleEvent( const Event& event )
{
   if( !(_state & ENABLED) || eventsDisabled() ) return;

   //if( needUpdate() )
   //{
   //   return;
   //}

   switch( event.type() )
   {
      case Event::POINTER_PRESS:     onPointerPress( event );     break;
      case Event::POINTER_RELEASE:   onPointerRelease( event );   break;
      case Event::POINTER_MOVE:      onPointerMove( event );      break;
      case Event::POINTER_ENTER:     onPointerEnter( event );     break;
      case Event::POINTER_LEAVE:     onPointerLeave( event );     break;
      case Event::POINTER_CANCEL:    onPointerCancel( event );    break;
      case Event::POINTER_SCROLL:    onPointerScroll( event );    break;
      case Event::KEY_PRESS:         onKeyPress( event );         break;
      case Event::KEY_RELEASE:       onKeyRelease( event );       break;
      case Event::CHAR:              onChar( event );             break;
      case Event::ACCELERATE:        onAccelerate( event );       break;
      case Event::FOCUS_GRAB:        onFocusGrab( event );        break;
      case Event::FOCUS_LOSE:        onFocusLose( event );        break;
      default:;
   }
}

//------------------------------------------------------------------------------
//!
void
Widget::addOnPopup( const WidgetDelegate& delegate )
{
  _onPopup.addDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
Widget::removeOnPopup( const WidgetDelegate& delegate )
{
  _onPopup.removeDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
Widget::addOnDelete( const WidgetDelegate& delegate )
{
  _onDelete.addDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
Widget::removeOnDelete( const WidgetDelegate& delegate )
{
  _onDelete.removeDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
Widget::addOnResize( const WidgetDelegate& delegate )
{
  _onResize.addDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
Widget::removeOnResize( const WidgetDelegate& delegate )
{
  _onResize.removeDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
Widget::addOnClick( const EventDelegate& delegate )
{
  _onClick.addDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
Widget::removeOnClick( const EventDelegate& delegate )
{
  _onClick.removeDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
Widget::addOnPointerPress( const EventDelegate& delegate )
{
  _onPointerPress.addDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
Widget::addOnPointerRelease( const EventDelegate& delegate )
{
  _onPointerRelease.addDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
Widget::addOnPointerMove( const EventDelegate& delegate )
{
  _onPointerMove.addDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
Widget::addOnPointerEnter( const EventDelegate& delegate )
{
  _onPointerEnter.addDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
Widget::addOnPointerLeave( const EventDelegate& delegate )
{
  _onPointerLeave.addDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
Widget::addOnPointerCancel( const EventDelegate& delegate )
{
  _onPointerCancel.addDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
Widget::addOnPointerScroll( const EventDelegate& delegate )
{
  _onPointerScroll.addDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
Widget::removeOnPointerPress( const EventDelegate& delegate )
{
  _onPointerPress.removeDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
Widget::removeOnPointerRelease( const EventDelegate& delegate )
{
  _onPointerRelease.removeDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
Widget::removeOnPointerMove( const EventDelegate& delegate )
{
  _onPointerMove.removeDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
Widget::removeOnPointerEnter( const EventDelegate& delegate )
{
  _onPointerEnter.removeDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
Widget::removeOnPointerLeave( const EventDelegate& delegate )
{
  _onPointerLeave.removeDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
Widget::removeOnPointerCancel( const EventDelegate& delegate )
{
  _onPointerCancel.removeDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
Widget::removeOnPointerScroll( const EventDelegate& delegate )
{
  _onPointerScroll.removeDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
Widget::addOnKeyPress( const EventDelegate& delegate )
{
  _onKeyPress.addDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
Widget::addOnKeyRelease( const EventDelegate& delegate )
{
  _onKeyRelease.addDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
Widget::addOnChar( const EventDelegate& delegate )
{
  _onChar.addDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
Widget::addOnAccelerate( const EventDelegate& delegate )
{
  _onAccelerate.addDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
Widget::removeOnKeyPress( const EventDelegate& delegate )
{
  _onKeyPress.removeDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
Widget::removeOnKeyRelease( const EventDelegate& delegate )
{
  _onKeyRelease.removeDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
Widget::removeOnChar( const EventDelegate& delegate )
{
  _onChar.removeDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
Widget::removeOnAccelerate( const EventDelegate& delegate )
{
  _onAccelerate.removeDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
Widget::addOnFocusGrab( const EventDelegate& delegate )
{
   _onFocusGrab.addDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
Widget::addOnFocusLose( const EventDelegate& delegate )
{
   _onFocusLose.addDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
Widget::removeOnFocusGrab( const EventDelegate& delegate )
{
   _onFocusGrab.removeDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
Widget::removeOnFocusLose( const EventDelegate& delegate )
{
   _onFocusLose.removeDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void
Widget::onPointerPress( const Event& ev )
{
   if( Core::grabPointer( ev.pointerID(), this ) )
   {
      if( !pressed() )
      {
         _state |= PRESSED;
         callShader();

         if( clickType() == ClickOnDown ) onClick( ev );
      }
   }

   _onPointerPress.exec( this, ev );
   execute( _onPointerPressRef, this, ev );
}

//------------------------------------------------------------------------------
//!
void
Widget::onPointerRelease( const Event& ev )
{
   if( !ev.pointer().pressed() )
   {
      Core::releasePointer( ev.pointerID() );

      if( pressed() )
      {
         _state &= ~PRESSED;
         if( clickType() != ClickOnDown && isInside(ev.position()) ) onClick( ev );
         callShader();
      }
   }

   _onPointerRelease.exec( this, ev );
   execute( _onPointerReleaseRef, this, ev );
}

//------------------------------------------------------------------------------
//!
void
Widget::onResize()
{
   _onResize.exec( this );
   execute( _onResizeRef, this );
}

//------------------------------------------------------------------------------
//!
void
Widget::onClick( const Event& ev )
{
   _onClick.exec( this, ev );
   execute( _onClickRef, this, ev );
}

//------------------------------------------------------------------------------
//!
void
Widget::onPointerMove( const Event& ev )
{
   const Pointer& p = ev.pointer();
   if( p.grabbedWith() != NULL )
   {
      // Normal POINTER_ENTER and POINTER_LEAVE generated by normal system events,
      // however we need to handle them ourselves when locked (only for grabbed widget).
      bool curInside = isInside( ev.position() );
      bool wasInside = isInside( p.lastEvent().position() );
      if( curInside != wasInside )
      {
         if( curInside )
         {
            onPointerEnter( Event::PointerEnter(Core::lastTime(), ev.pointerID(), ev.position()) );
         }
         else
         {
            onPointerLeave( Event::PointerLeave(Core::lastTime(), ev.pointerID(), ev.position()) );
         }
      }
   }

   _onPointerMove.exec( this, ev );
   execute( _onPointerMoveRef, this, ev );
}

//------------------------------------------------------------------------------
//!
void
Widget::onPointerEnter( const Event& ev )
{
   if( !hovered() )
   {
      _state |= HOVERED;
      callShader();
   }
   _onPointerEnter.exec( this, ev );
   execute( _onPointerEnterRef, this, ev );
}

//------------------------------------------------------------------------------
//!
void
Widget::onPointerLeave( const Event& ev )
{
   if( !Core::otherPointerHovering(this, ev.pointerID()) )
   {
      _state &= ~HOVERED;
      callShader();
   }
   _onPointerLeave.exec( this, ev );
   execute( _onPointerLeaveRef, this, ev );
}

//------------------------------------------------------------------------------
//!
void
Widget::onPointerCancel( const Event& ev )
{
   // Reset a number of flags
   _state &= ~(HOVERED | PRESSED);

   // Release pointer if we have a grab on it.
   Core::releasePointer( this );

   callShader(); // Necessary?

   _onPointerCancel.exec( this, ev );
   execute( _onPointerCancelRef, this, ev );
}

//------------------------------------------------------------------------------
//!
void
Widget::onPointerScroll( const Event& ev )
{
   _onPointerScroll.exec( this, ev );
   execute( _onPointerScrollRef, this, ev );
   if( _parent ) _parent->onPointerScroll( ev );
}

//------------------------------------------------------------------------------
//!
void
Widget::onKeyPress( const Event& ev )
{
   _onKeyPress.exec( this, ev );
   execute( _onKeyPressRef, this, ev );
}

//------------------------------------------------------------------------------
//!
void
Widget::onKeyRelease( const Event& ev )
{
   _onKeyRelease.exec( this, ev );
   execute( _onKeyReleaseRef, this, ev );
}

//------------------------------------------------------------------------------
//!
void
Widget::onChar( const Event& ev )
{
   _onChar.exec( this, ev );
   execute( _onCharRef, this, ev );
}

//------------------------------------------------------------------------------
//!
void
Widget::onAccelerate( const Event& ev )
{
   _onAccelerate.exec( this, ev );
   execute( _onAccelerateRef, this, ev );
}

//------------------------------------------------------------------------------
//!
void
Widget::onFocusGrab( const Event& ev )
{
   _onFocusGrab.exec( this, ev );
   execute( _onFocusGrabRef, this, ev );
}

//------------------------------------------------------------------------------
//!
void
Widget::onFocusLose( const Event& ev )
{
   callShader();
   _onFocusLose.exec( this, ev );
   execute( _onFocusLoseRef, this, ev );
}

//------------------------------------------------------------------------------
//!
void
Widget::performSetGeometry()
{
   // Nothing to do.
}

//------------------------------------------------------------------------------
//!
void
Widget::performSetPosition()
{
   // Nothing to do.
}

//------------------------------------------------------------------------------
//!
Vec2f
Widget::performComputeBaseSize()
{
   // Execute baseSizeRef function.
   if( _baseSizeRef.isValid() )
   {
      VMState* vm = _baseSizeRef.vm();

      // Call base size function.
      VM::push( vm, _baseSizeRef );
      VM::pushProxy( vm, this );
      VM::ecall( vm, 1, 1 );

      // Read the returned values.
      Vec2f size =  VM::toVec2f( vm, -1 );
      VM::pop( vm, 1 );

      return size;
   }
   return Vec2f( 0.0f, 0.0f );
}

//------------------------------------------------------------------------------
//!
void
Widget::observe( Subject* subject )
{
   if( _subject ) _subject->detach( this );

   _subject = subject;
   _subject->attach( this );
}

//------------------------------------------------------------------------------
//!
void
Widget::update()
{
   execute( _onNotifyRef, this );
}

//------------------------------------------------------------------------------
//!
void
Widget::destroy()
{
   _subject = NULL;
}

//------------------------------------------------------------------------------
//!
void
Widget::add( const RCP<Drawable>& drawable )
{
   if( drawable.isValid() )  _drawables.pushBack( drawable );
}

//------------------------------------------------------------------------------
//!
const char*
Widget::meta() const
{
   return _widget_str_;
}

//------------------------------------------------------------------------------
//!
void
Widget::init( VMState* vm )
{
   const char* name = meta();

   DBG_BLOCK( os_w, "Widget::init: " << name );

   if( !VM::isTable( vm, 1 ) )  return;

   // We are starting the init.
   inInit = true;

   // Read parameters.
   VM::get( vm, 1, "id",                 _id );
   VM::get( vm, 1, "position",           _localPos );
   VM::get( vm, 1, "size",               _size );
   VM::get( vm, 1, "flex",               _flex );
   VM::get( vm, 1, "onDelete",           _onDeleteRef );
   VM::get( vm, 1, "onResize",           _onResizeRef );
   VM::get( vm, 1, "onClick",            _onClickRef );
   VM::get( vm, 1, "onPointerPress",     _onPointerPressRef );
   VM::get( vm, 1, "onPointerRelease",   _onPointerReleaseRef );
   VM::get( vm, 1, "onPointerMove",      _onPointerMoveRef );
   VM::get( vm, 1, "onPointerEnter",     _onPointerEnterRef );
   VM::get( vm, 1, "onPointerLeave",     _onPointerLeaveRef );
   VM::get( vm, 1, "onPointerScroll",    _onPointerScrollRef );
   VM::get( vm, 1, "onKeyPress",         _onKeyPressRef );
   VM::get( vm, 1, "onKeyRelease",       _onKeyReleaseRef );
   VM::get( vm, 1, "onChar",             _onCharRef );
   VM::get( vm, 1, "onAccelerate",       _onAccelerateRef );
   VM::get( vm, 1, "onFocusGrab",        _onFocusGrabRef );
   VM::get( vm, 1, "onFocusLose",        _onFocusLoseRef );
   VM::get( vm, 1, "onNotify",           _onNotifyRef );
   VM::get( vm, 1, "shader",             _shaderRef );
   VM::get( vm, 1, "baseSize",           _baseSizeRef );

   uint icon = hoverIcon();
   VM::get( vm, 1, "hoverIcon",          icon );
   hoverIcon( icon );

   uint click = clickType();
   VM::get( vm, 1, "clickType",          click );
   clickType( click );

   bool enabled = (_state & ENABLED) ? true : false;
   bool hidden = (_state & HIDDEN) ? true : false;
   uint hAlign = alignH();
   uint vAlign = alignV();
   bool accel  = false;

   VM::get( vm, 1, "accelerometer", accel );
   accelerometer( accel );

   _state = 0;

   VM::get( vm, 1, "enabled", enabled );
   _state |= enabled ? ENABLED : 0;

   VM::get( vm, 1, "hidden", hidden );
   _state |= hidden ? HIDDEN : 0;

   VM::get( vm, 1, "hAlign", hAlign );
   alignH( hAlign );

   VM::get( vm, 1, "vAlign", vAlign );
   alignV( vAlign );

   bool b;
   if( VM::get( vm, 1, "eventsEnabled", b ) )  enableEvents( b );

   setDefaultFunction( vm, _shaderRef,   "shaders", name );
   setDefaultFunction( vm, _baseSizeRef, "baseSize", name );

   // Create attributes table.
   VM::newTable( vm );
   VM::push( vm );

   while( VM::next( vm, 1 ) )
   {
      if( VM::isNumber( vm, -2 ) ||
          isAttribute( VM::toCString( vm, -2 ) ) )
      {
         VM::pop( vm, 1 );
      }
      else
      {
         // Duplicate the key.
         VM::pushValue( vm, -2 );
         VM::insert( vm, -3 );

         // Insert attribute into table.
         VM::set( vm, -4 );
      }
   }
   // Keep a reference on user attributes table.
   VM::toRef( vm, -1, _userAttributes );

   // Call shader function.
   if( _shaderRef.isValid() )
   {
      VM::push( vm, _shaderRef );
      VM::pushProxy( vm, this );
      VM::ecall( vm, 1, 0 );
   }
   // We have finish the init.
   inInit = false;
}

//------------------------------------------------------------------------------
//!
bool
Widget::performGet( VMState* vm )
{
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_STATE:
         VM::push( vm, _state );
         return true;
      case ATTRIB_ID:
         VM::push( vm, _id );
         return true;
      case ATTRIB_POSITION:
         VM::push( vm, localPosition() );
         return true;
      case ATTRIB_ABS_POSITION:
         VM::push( vm, absPosition() );
         return true;
      case ATTRIB_SIZE:
         VM::push( vm, size() );
         return true;
      case ATTRIB_FLEX:
         VM::push( vm, _flex );
         return true;
      case ATTRIB_ONDELETE:
         VM::push( vm, _onDeleteRef );
         return true;
      case ATTRIB_ONRESIZE:
         VM::push( vm, _onResizeRef );
         return true;
      case ATTRIB_ONCLICK:
         VM::push( vm, _onClickRef );
         return true;
      case ATTRIB_ONPOINTER_PRESS:
         VM::push( vm, _onPointerPressRef );
         return true;
      case ATTRIB_ONPOINTER_RELEASE:
         VM::push( vm, _onPointerReleaseRef );
         return true;
      case ATTRIB_ONPOINTER_MOVE:
         VM::push( vm, _onPointerMoveRef );
         return true;
      case ATTRIB_ONPOINTER_ENTER:
         VM::push( vm, _onPointerEnterRef );
         return true;
      case ATTRIB_ONPOINTER_LEAVE:
         VM::push( vm, _onPointerLeaveRef );
         return true;
      case ATTRIB_ONPOINTER_SCROLL:
         VM::push( vm, _onPointerScrollRef );
         return true;
      case ATTRIB_ONKEY_PRESS:
         VM::push( vm, _onKeyPressRef );
         return true;
      case ATTRIB_ONKEY_RELEASE:
         VM::push( vm, _onKeyReleaseRef );
         return true;
      case ATTRIB_ONNOTIFY:
         VM::push( vm, _onNotifyRef );
         return true;
      case ATTRIB_ONCHAR:
         VM::push( vm, _onCharRef );
         return true;
      case ATTRIB_ONACCELERATE:
         VM::push( vm, _onAccelerateRef );
         return true;
      case ATTRIB_ONFOCUS_GRAB:
         VM::push( vm, _onFocusGrabRef );
         return true;
      case ATTRIB_ONFOCUS_LOSE:
         VM::push( vm, _onFocusLoseRef );
         return true;
      case ATTRIB_SHADER:
         VM::push( vm, _shaderRef );
         return true;
      case ATTRIB_BASE_SIZE:
         VM::push( vm, _baseSizeRef );
         return true;
      case ATTRIB_ACTUAL_BASE_SIZE:
         VM::push( vm, actualBaseSize() );
         return true;
      case ATTRIB_ACTUAL_SIZE:
         VM::push( vm, _actualSize );
         return true;
      case ATTRIB_GLOBAL_POSITION:
         VM::push( vm, _globalPos );
         return true;
      case ATTRIB_GET_CENTER_GLOBAL:
         VM::push( vm, this, getCenterGlobalVM );
         return true;
      case ATTRIB_GET_CENTER_LOCAL:
         VM::push( vm, this, getCenterLocalVM );
         return true;
      case ATTRIB_ENABLED:
         VM::push( vm, enabled() );
         return true;
      case ATTRIB_HOVERED:
         VM::push( vm, hovered() );
         return true;
      case ATTRIB_PRESSED:
         VM::push( vm, pressed() );
         return true;
      case ATTRIB_HIDDEN:
         VM::push( vm, hidden() );
         return true;
      case ATTRIB_EVENTS_ENABLED:
         VM::push( vm, eventsEnabled() );
         return true;
      case ATTRIB_ACCELEROMETER:
         VM::push( vm, accelerometer() );
         return true;
      case ATTRIB_PARENT:
         VM::pushProxy( vm, _parent );
         return true;
      case ATTRIB_HOVER_ICON:
         VM::push( vm, hoverIcon() );
         return true;
      case ATTRIB_TEXTQUAD:
         VM::push( vm, this, text_create );
         return true;
      case ATTRIB_TO_FRONT:
         VM::push( vm, this, toFrontVM );
         return true;
      case ATTRIB_TO_BACK:
         VM::push( vm, this, toBackVM );
         return true;
      case ATTRIB_TQUAD:
         VM::push( vm, this, tquad_create );
         return true;
      case ATTRIB_CLOSE:
         VM::push( vm, this, closeVM );
         return true;
      case ATTRIB_RESIZE:
         VM::push( vm, this, resizeVM );
         return true;
      case ATTRIB_RESIZEW:
         VM::push( vm, this, resizeWVM );
         return true;
      case ATTRIB_RESIZEH:
         VM::push( vm, this, resizeHVM );
         return true;
      case ATTRIB_FINDPARENT:
         VM::push( vm, this, findParentVM );
         return true;
      case ATTRIB_OBSERVE:
         VM::push( vm, this, observeVM );
         return true;
      case ATTRIB_HALIGN:
         VM::push( vm, alignH() );
         return true;
      case ATTRIB_VALIGN:
         VM::push( vm, alignV() );
         return true;

      default: break;
   }

   // Attribute not found, so its considered a user attribute.
   VM::push( vm, _userAttributes );
   VM::pushValue( vm, 2 );
   VM::get( vm, -2 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
Widget::performSet( VMState* vm )
{
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_STATE: // Read only.
         return true;
      case ATTRIB_ID:
         _id = VM::toString( vm, 3 );
         return true;
      case ATTRIB_POSITION:
         localPosition( VM::toVec2f( vm, 3 ) );
         return true;
      case ATTRIB_ABS_POSITION:
         absPosition( VM::toVec2f( vm, 3 ) );
         return true;
      case ATTRIB_SIZE:
         size( VM::toVec2f( vm, 3 ) );
         return true;
      case ATTRIB_FLEX:
         _flex = VM::toFloat( vm, 3 );
         markForUpdate();
         return true;
      case ATTRIB_ONDELETE:
         VM::toRef( vm, 3, _onDeleteRef );
         return true;
      case ATTRIB_ONRESIZE:
         VM::toRef( vm, 3, _onResizeRef );
         return true;
      case ATTRIB_ONCLICK:
         VM::toRef( vm, 3, _onClickRef );
         return true;
      case ATTRIB_ONPOINTER_PRESS:
         VM::toRef( vm, 3, _onPointerPressRef );
         return true;
      case ATTRIB_ONPOINTER_RELEASE:
         VM::toRef( vm, 3, _onPointerReleaseRef );
         return true;
      case ATTRIB_ONPOINTER_MOVE:
         VM::toRef( vm, 3, _onPointerMoveRef );
         return true;
      case ATTRIB_ONPOINTER_ENTER:
         VM::toRef( vm, 3, _onPointerEnterRef );
         return true;
      case ATTRIB_ONPOINTER_LEAVE:
         VM::toRef( vm, 3, _onPointerLeaveRef );
         return true;
      case ATTRIB_ONPOINTER_SCROLL:
         VM::toRef( vm, 3, _onPointerScrollRef );
         return true;
      case ATTRIB_ONKEY_PRESS:
         VM::toRef( vm, 3, _onKeyPressRef );
         return true;
      case ATTRIB_ONKEY_RELEASE:
         VM::toRef( vm, 3, _onKeyReleaseRef );
         return true;
      case ATTRIB_ONNOTIFY:
         VM::toRef( vm, 3, _onNotifyRef );
         return true;
      case ATTRIB_ONCHAR:
         VM::toRef( vm, 3, _onCharRef );
         return true;
      case ATTRIB_ONACCELERATE:
         VM::toRef( vm, 3, _onAccelerateRef );
         return true;
      case ATTRIB_ONFOCUS_GRAB:
         VM::toRef( vm, 3, _onFocusGrabRef );
         return true;
      case ATTRIB_ONFOCUS_LOSE:
         VM::toRef( vm, 3, _onFocusLoseRef );
         return true;
      case ATTRIB_SHADER:
         VM::toRef( vm, 3, _shaderRef );
         markForUpdate();
         return true;
      case ATTRIB_BASE_SIZE:
         VM::toRef( vm, 3, _baseSizeRef );
         markForUpdate( true );
         return true;
      case ATTRIB_ACTUAL_BASE_SIZE:   // Read only.
      case ATTRIB_ACTUAL_SIZE:        // Read only.
      case ATTRIB_GLOBAL_POSITION:    // Read only.
      case ATTRIB_GET_CENTER_GLOBAL:  // Read only.
      case ATTRIB_GET_CENTER_LOCAL:   // Read only.
         return true;
      case ATTRIB_ENABLED:
         enable( VM::toBoolean( vm, 3 ) );
         return true;
      case ATTRIB_HOVERED:    // Read only.
         return true;
      case ATTRIB_PRESSED:    // Read only.
         return true;
      case ATTRIB_HIDDEN:
         hide( VM::toBoolean( vm, 3 ) );
         return true;
      case ATTRIB_EVENTS_ENABLED:
         enableEvents( VM::toBoolean( vm, 3 ) );
         return true;
      case ATTRIB_ACCELEROMETER:
         accelerometer( VM::toBoolean( vm, 3 ) );
         return true;
      case ATTRIB_PARENT:     // Read only.
         return true;
      case ATTRIB_HOVER_ICON:
         hoverIcon( VM::toUInt( vm, 3 ) );
         return true;
      case ATTRIB_TEXTQUAD:   // Read only.
         return true;
      case ATTRIB_TO_FRONT:   // Read only.
         return true;
      case ATTRIB_TO_BACK:    // Read only.
         return true;
      case ATTRIB_TQUAD:      // Read only.
         return true;
      case ATTRIB_CLOSE:      // Read only.
         return true;
      case ATTRIB_RESIZE:     // Read only.
         return true;
      case ATTRIB_RESIZEW:    // Read only.
         return true;
      case ATTRIB_RESIZEH:    // Read only.
         return true;
      case ATTRIB_FINDPARENT: // Read only.
         return true;
      case ATTRIB_OBSERVE:    // Read only.
         return true;
      case ATTRIB_HALIGN:
         alignH( VM::toInt( vm, 3 ) );
         return true;
      case ATTRIB_VALIGN:
         alignV( VM::toInt( vm, 3 ) );
         return true;
      default: break;
   }

   // Attribute not found, so its considered a user attribute.
   VM::push( vm, _userAttributes );
   VM::insert( vm, 2 );
   VM::set( vm, 2 );

   // Don't update if in init.
   if( inInit ) return true;

   VM::setTop( vm, 0 );

   updateLook();
   return true;
}

//------------------------------------------------------------------------------
//!
bool
Widget::isAttribute( const char* name ) const
{
   return _attributes[ name ] != StringMap::INVALID;
}

NAMESPACE_END
