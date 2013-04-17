/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_WIDGET_H
#define FUSION_WIDGET_H

#include <Fusion/StdDefs.h>
#include <Fusion/Drawable/Drawable.h>

#include <CGMath/Vec2.h>

#include <Gfx/Pass/RenderNode.h>

#include <Base/ADT/String.h>
#include <Base/Msg/DelegateList.h>
#include <Base/Msg/Observer.h>
#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>

#if _MSC_VER
#pragma warning(push)
// Normally, we should put the FUSION_DLL_API at every non-inline function.
// but there is a bug in Cygwin's g++ 3.4.4 which cannot find non-virtual thunk
// for the update() and destroy() routines, which are from Observer as well.
#pragma warning(disable:4251) //needs to have dll-interface
#endif //_MSC_VER

NAMESPACE_BEGIN

class Event;
class WidgetContainer;
class Subject;

/*==============================================================================
  CLASS Widget
==============================================================================*/

//! Base class for widget.
//!
//! Attributes list for VM:
//!
//!   read only:
//!     state:  Bit field for current state( 1 = enabled, 2 = hovered,
//!         3 = pressed ).
//!     parent:  Parent Widget.
//!     actualSize:  Current widget size.
//!     globalPos:  Current Widget global position.
//!     findParent( id ): Search and return the first parent name id.
//!
//!   read/write
//!     id:  String id.
//!     pos:  Preferred local position.
//!     size:  Preferred size. A value of -1 indicate automatic computation
//!         with baseSize.
//!     flex:  Flexibility value. Used to distributes left space in
//!        containers. A flex > 0 for widget in Desktop means maximize.
//!     shader:  Scripted shader function.
//!     baseSize:  Scripted function computing the base (preferred) size of a
//!         widget.
//!     enabled:  Enabled/Disabled the widget.
//!     hoverIcon:  Icon used for the pointer when hovering the widget.
//!     hidden:  Visible/hidden state of the widget.
//!     onDelete
//!     onResize
//!     onClick
//!     onPointerPress
//!     onPointerRelease
//!     onPointerMove
//!     onPointerEnter
//!     onPointerCancel
//!     onPointerScroll
//!     onKeyPress
//!     onKeyRelease
//!     onChar
//!     onAccelerate
//!
//! Update mechanism:
//!   markForUpdate: Indicate a change in geometry. Size and position will be
//!       recomputed. The appearance shader will be called and the baseSize
//!       method will be called only if necessary.
//!   updateLook: To call when a attribute that can influence look and base size
//!       change. The shader and the baseSize methods will be called and a
//!       geometry update (markForUpdate) will be called if necessary.
//!   callShader: Only execute the shader program. Call this method if an
//!       attribute that influence look but not size is changed.
//!
class Widget:
   public RCObject,
   public VMProxy,
   private Observer
{

public:

   /*----- types and enumerations ----*/

   enum State {
      ENABLED  = 1,   // 000000001
      HOVERED  = 2,   // 000000010
      PRESSED  = 4,   // 000000100
      HIDDEN   = 8    // 000001000
   };

   enum Alignment {
      FLEX     = 0,   // 00
      START    = 1,   // 01
      MIDDLE   = 2,   // 10
      END      = 3    // 11
   };

   typedef Delegate1< Widget* >                   WidgetDelegate;
   typedef Delegate1List< Widget* >               WidgetDelegateList;
   typedef Delegate2< Widget*, const Event& >     EventDelegate;
   typedef Delegate2List< Widget*, const Event& > EventDelegateList;

   /*----- static methods -----*/

   FUSION_DLL_API static void initialize();

   /*----- methods -----*/

   FUSION_DLL_API Widget();


   inline const String& id() const;

   // Parent.
   inline void parent( WidgetContainer* widget );
   inline WidgetContainer* parent() const;

   // Drawing.
   FUSION_DLL_API virtual void render( const RCP<Gfx::RenderNode>& rn );

   // Searching.
   FUSION_DLL_API virtual Widget* getWidgetAt( const Vec2f& pos );
   FUSION_DLL_API virtual Widget* findWidget( const String& id );
   FUSION_DLL_API virtual void getWidgetsAt( const Vec2f& pos, Vector<Widget*>& );
   FUSION_DLL_API Widget* findParent( const String& id );

   // Messaging.
   FUSION_DLL_API virtual void sendParentMessage( Widget* widget, int message );
   FUSION_DLL_API virtual void sendChildMessage( Widget* widget, int message );


   // Update for geometry.
   inline bool needUpdate() const;
   FUSION_DLL_API void geometry( const Vec2f& global, const Vec2f& pos, const Vec2f& size );
   FUSION_DLL_API void position( const Vec2f& global, const Vec2f& pos );
   FUSION_DLL_API void resize( const Vec4f& delta );
   FUSION_DLL_API void resizeW( const Vec2f& delta );
   FUSION_DLL_API void resizeH( const Vec2f& delta );

   // Position and size.
   inline void  localPosition( const Vec2f& );
   FUSION_DLL_API void   absPosition( const Vec2f& );
   inline const Vec2f&   globalPosition() const;
   inline const Vec2f&   localPosition() const;
   FUSION_DLL_API Vec2f  absPosition() const;

   inline void  size( const Vec2f& );
   inline const Vec2f&   size() const;
   inline const Vec2f&   actualSize() const;
   FUSION_DLL_API Vec2f  actualBaseSize();

   FUSION_DLL_API Vec2f  getCenterGlobal() const;
   FUSION_DLL_API Vec2f  getCenterLocal() const;

   FUSION_DLL_API virtual Vec2f screenToLayer( const Vec2f& ) const;
   FUSION_DLL_API virtual Vec2f layerToScreen( const Vec2f& ) const;

   inline bool isInside( const Vec2f& pos ) const;

   // Flexibility.
   inline float flexibility() const;
   inline float flexibilityH() const;
   inline float flexibilityV() const;
   inline float flexibilityHV( uint axis ) const;

   // State
   inline bool enabled() const;
   FUSION_DLL_API void enable( bool flag );
   inline bool hovered() const;
   inline bool pressed() const;

   // Events.
   inline bool eventsDisabled() const;
   inline bool eventsEnabled() const;
   inline void enableEvents( bool val );
   inline bool accelerometer() const;
   FUSION_DLL_API void accelerometer( bool val );

   // Visibility.
   FUSION_DLL_API void hide( bool flag );
   inline bool hidden() const;
   FUSION_DLL_API void toFront() const;
   FUSION_DLL_API void toBack() const;
   FUSION_DLL_API void close();

   // Pointer icon identifier.
   inline void hoverIcon( uint );
   inline uint hoverIcon() const;

   // Popup.
   inline bool isPopup() const;
   FUSION_DLL_API void popup( bool, bool call = true );
   FUSION_DLL_API bool belongToPopup() const;

   // Alignment.
   inline uint alignH() const;
   inline uint alignV() const;
   inline uint alignHV( uint axis ) const;

   inline void alignH( uint alignment );
   inline void alignV( uint alignment );
   inline void alignHV( uint axis, uint alignment );

   //! Callback
   FUSION_DLL_API void handleEvent( const Event& );

   FUSION_DLL_API void addOnPopup( const WidgetDelegate& );
   FUSION_DLL_API void removeOnPopup( const WidgetDelegate& );

   FUSION_DLL_API void addOnDelete( const WidgetDelegate& );
   FUSION_DLL_API void removeOnDelete( const WidgetDelegate& );

   FUSION_DLL_API void addOnResize( const WidgetDelegate& );
   FUSION_DLL_API void removeOnResize( const WidgetDelegate& );

   FUSION_DLL_API void addOnClick( const EventDelegate& );
   FUSION_DLL_API void removeOnClick( const EventDelegate& );

   FUSION_DLL_API void addOnPointerPress( const EventDelegate& );
   FUSION_DLL_API void addOnPointerRelease( const EventDelegate& );
   FUSION_DLL_API void addOnPointerMove( const EventDelegate& );
   FUSION_DLL_API void addOnPointerEnter( const EventDelegate& );
   FUSION_DLL_API void addOnPointerLeave( const EventDelegate& );
   FUSION_DLL_API void addOnPointerCancel( const EventDelegate& );
   FUSION_DLL_API void addOnPointerScroll( const EventDelegate& );
   FUSION_DLL_API void removeOnPointerPress( const EventDelegate& );
   FUSION_DLL_API void removeOnPointerRelease( const EventDelegate& );
   FUSION_DLL_API void removeOnPointerMove( const EventDelegate& );
   FUSION_DLL_API void removeOnPointerEnter( const EventDelegate& );
   FUSION_DLL_API void removeOnPointerLeave( const EventDelegate& );
   FUSION_DLL_API void removeOnPointerCancel( const EventDelegate& );
   FUSION_DLL_API void removeOnPointerScroll( const EventDelegate& );

   FUSION_DLL_API void addOnKeyPress( const EventDelegate& );
   FUSION_DLL_API void addOnKeyRelease( const EventDelegate& );
   FUSION_DLL_API void addOnChar( const EventDelegate& );
   FUSION_DLL_API void removeOnKeyPress( const EventDelegate& );
   FUSION_DLL_API void removeOnKeyRelease( const EventDelegate& );
   FUSION_DLL_API void removeOnChar( const EventDelegate& );

   FUSION_DLL_API void addOnAccelerate( const EventDelegate& );
   FUSION_DLL_API void removeOnAccelerate( const EventDelegate& );

   FUSION_DLL_API void addOnFocusGrab( const EventDelegate& );
   FUSION_DLL_API void addOnFocusLose( const EventDelegate& );
   FUSION_DLL_API void removeOnFocusGrab( const EventDelegate& );
   FUSION_DLL_API void removeOnFocusLose( const EventDelegate& );

   FUSION_DLL_API void observe( Subject* );

   FUSION_DLL_API virtual void onResize();

   //! Event handlers
   //{
   FUSION_DLL_API virtual void onClick( const Event& );
   FUSION_DLL_API virtual void onPointerPress( const Event& );
   FUSION_DLL_API virtual void onPointerRelease( const Event& );
   FUSION_DLL_API virtual void onPointerMove( const Event& );
   FUSION_DLL_API virtual void onPointerEnter( const Event& );
   FUSION_DLL_API virtual void onPointerLeave( const Event& );
   FUSION_DLL_API virtual void onPointerCancel( const Event& );
   FUSION_DLL_API virtual void onPointerScroll( const Event& );
   FUSION_DLL_API virtual void onKeyPress( const Event& );
   FUSION_DLL_API virtual void onKeyRelease( const Event& );
   FUSION_DLL_API virtual void onChar( const Event& );
   FUSION_DLL_API virtual void onAccelerate( const Event& );
   FUSION_DLL_API virtual void onFocusGrab( const Event& );
   FUSION_DLL_API virtual void onFocusLose( const Event& );
   //}

   // Drawable.
   FUSION_DLL_API void add( const RCP<Drawable>& );

   // VM.
   FUSION_DLL_API virtual const char*  meta() const;
   FUSION_DLL_API void init( VMState* vm );
   FUSION_DLL_API bool performGet( VMState* );
   FUSION_DLL_API bool performSet( VMState* );

protected:

   friend class Core;

   /*----- types and enumerations ----*/

   enum ClickType {
      ClickOnDown   = 0,
      ClickOnUp     = 1
   };

   /*----- methods -----*/

   FUSION_DLL_API virtual ~Widget();

   //! Observer.
   FUSION_DLL_API virtual void update();
   FUSION_DLL_API virtual void destroy();

   FUSION_DLL_API void markForUpdate( bool markBaseSize = false );
   FUSION_DLL_API void callShader();
   FUSION_DLL_API void updateLook();

   FUSION_DLL_API virtual Vec2f performComputeBaseSize();
   FUSION_DLL_API virtual void performSetGeometry();
   FUSION_DLL_API virtual void performSetPosition();

   FUSION_DLL_API virtual bool isAttribute( const char* ) const;

   inline void needUpdate( bool val )     { _fields = setbits( _fields, 1, 1, val ); }
   inline bool baseSizeCached() const     { return getbits( _fields, 2, 1 ) != 0; }
   inline void baseSizeCached( bool val ) { _fields = setbits( _fields, 2, 1, val ); }
   //
   //
   inline ClickType clickType() const     { return (ClickType)getbits( _fields, 4, 2 ); }
   inline void clickType( uint val )      { _fields = setbits( _fields, 4, 2, val ); }

   /*----- data members -----*/

   int       _state;

private:

   /*----- data members -----*/

   //! NAME          SIZE   LOCATION         DESCRIPTION
   //! isPopup        (1)   _fields[ 0: 0]
   //! needUpdate     (1)   _fields[ 1: 1]
   //! baseSizeCached (1)   _fields[ 2: 2]
   //! xxxxxxxxxxxxxx (1)   _fields[ 3: 3]
   //! clickType      (2)   _fields[ 5: 4]
   //! hoverIcon      (5)   _fields[10: 6]
   //! alignH         (2)   _fields[17:16]
   //! alignV         (2)   _fields[19:18]
   //! eventsDisabled (1)   _fields[20:20]
   //! accelerometer  (1)   _fields[21:21]   Set when the widget is registered for accelerometer.
   uint               _fields;

   WidgetContainer*   _parent;
   String             _id;
   Vec2f              _globalPos;
   Vec2f              _localPos;
   Vec2f              _actualSize;
   Vec2f              _size;
   Vec2f              _actualBaseSize;
   float              _flex;

   WidgetDelegateList _onPopup;
   WidgetDelegateList _onDelete;
   WidgetDelegateList _onResize;
   EventDelegateList  _onClick;
   EventDelegateList  _onPointerPress;
   EventDelegateList  _onPointerRelease;
   EventDelegateList  _onPointerMove;
   EventDelegateList  _onPointerEnter;
   EventDelegateList  _onPointerLeave;
   EventDelegateList  _onPointerCancel;
   EventDelegateList  _onPointerScroll;
   EventDelegateList  _onKeyPress;
   EventDelegateList  _onKeyRelease;
   EventDelegateList  _onChar;
   EventDelegateList  _onAccelerate;
   EventDelegateList  _onFocusGrab;
   EventDelegateList  _onFocusLose;

   VMRef              _onDeleteRef;
   VMRef              _onResizeRef;
   VMRef              _onClickRef;
   VMRef              _onPointerPressRef;
   VMRef              _onPointerReleaseRef;
   VMRef              _onPointerMoveRef;
   VMRef              _onPointerEnterRef;
   VMRef              _onPointerLeaveRef;
   VMRef              _onPointerCancelRef;
   VMRef              _onPointerScrollRef;
   VMRef              _onKeyPressRef;
   VMRef              _onKeyReleaseRef;
   VMRef              _onCharRef;
   VMRef              _onAccelerateRef;
   VMRef              _onNotifyRef;
   VMRef              _onFocusGrabRef;
   VMRef              _onFocusLoseRef;

   VMRef              _shaderRef;
   VMRef              _baseSizeRef;
   VMRef              _userAttributes;

   Subject*           _subject;

   Vector< RCP<Drawable> > _drawables;
   Vector< Drawable* >     _activeDrawables;
};

//------------------------------------------------------------------------------
//!
inline bool
Widget::needUpdate() const
{
   return getbits( _fields, 1, 1 ) != 0;
}

//------------------------------------------------------------------------------
//!
inline const String&
Widget::id() const
{
   return _id;
}

//------------------------------------------------------------------------------
//!
inline void
Widget::parent( WidgetContainer* widget )
{
   _parent = widget;
}

//------------------------------------------------------------------------------
//!
inline WidgetContainer*
Widget::parent() const
{
   return _parent;
}

//------------------------------------------------------------------------------
//!
inline const Vec2f&
Widget::globalPosition() const
{
   return _globalPos;
}

//------------------------------------------------------------------------------
//!
inline const Vec2f&
Widget::localPosition() const
{
   return _localPos;
}

//------------------------------------------------------------------------------
//!
inline void
Widget::localPosition( const Vec2f& pos )
{
   _localPos = pos;
   markForUpdate();
}

//------------------------------------------------------------------------------
//!
inline void
Widget::size( const Vec2f& size )
{
   _size = size;
   markForUpdate(true);
}

//------------------------------------------------------------------------------
//!
inline const Vec2f&
Widget::size() const
{
   return _size;
}

//------------------------------------------------------------------------------
//!
inline const Vec2f&
Widget::actualSize() const
{
   return _actualSize;
}

//------------------------------------------------------------------------------
//!
inline float
Widget::flexibility() const
{
   return _flex;
}

//------------------------------------------------------------------------------
//!
inline float
Widget::flexibilityH() const
{
   return ( alignH() == FLEX ) ? _flex : 0;
}

//------------------------------------------------------------------------------
//!
inline float
Widget::flexibilityV() const
{
   return ( alignV() == FLEX ) ? _flex : 0;
}

//------------------------------------------------------------------------------
//!
inline float
Widget::flexibilityHV( uint axis ) const
{
   return ( axis == 0 ) ? flexibilityH() : flexibilityV();
}

//------------------------------------------------------------------------------
//!
inline bool
Widget::enabled() const
{
   return (_state & ENABLED) ? true : false;
}

//------------------------------------------------------------------------------
//!
inline bool
Widget::hovered() const
{
   return (_state & HOVERED) ? true : false;
}

//------------------------------------------------------------------------------
//!
inline bool
Widget::pressed() const
{
   return (_state & PRESSED) ? true : false;
}

//------------------------------------------------------------------------------
//!
inline bool
Widget::hidden() const
{
   return (_state & HIDDEN) ? true : false;
}

//------------------------------------------------------------------------------
//!
inline bool
Widget::eventsDisabled() const
{
   return getbits( _fields, 20, 1 ) != 0;
}

//------------------------------------------------------------------------------
//!
inline bool
Widget::eventsEnabled() const
{
   return !eventsDisabled();
}

//------------------------------------------------------------------------------
//!
inline void
Widget::enableEvents( bool val )
{
   _fields = setbits( _fields, 20, 1, (val?0:1) );
}

//------------------------------------------------------------------------------
//!
inline bool
Widget::accelerometer() const
{
   return getbits( _fields, 21, 1 ) != 0;
}

//------------------------------------------------------------------------------
//!
inline void
Widget::hoverIcon( uint id )
{
   _fields = setbits( _fields, 6, 5, id );
}

//------------------------------------------------------------------------------
//!
inline uint
Widget::hoverIcon() const
{
   return getbits( _fields, 6, 5 );
}

//------------------------------------------------------------------------------
//!
inline bool
Widget::isInside( const Vec2f& pos ) const
{
   return
      pos.x >= _globalPos.x && pos.x < _globalPos.x + _actualSize.x &&
      pos.y >= _globalPos.y && pos.y < _globalPos.y + _actualSize.y;
}

//------------------------------------------------------------------------------
//!
inline bool
Widget::isPopup() const
{
   return getbits( _fields, 0, 1 ) != 0;
}

//------------------------------------------------------------------------------
//!
inline uint
Widget::alignH() const
{
   return getbits( _fields, 16, 2 );
}

//------------------------------------------------------------------------------
//!
inline uint
Widget::alignV() const
{
   return getbits( _fields, 18, 2 );
}

//------------------------------------------------------------------------------
//!
inline uint
Widget::alignHV( uint axis ) const
{
   return (axis==0) ? alignH() : alignV();
}

//------------------------------------------------------------------------------
//!
inline void
Widget::alignH( uint alignment )
{
   _fields = setbits( _fields, 16, 2, alignment );
}

//------------------------------------------------------------------------------
//!
inline void
Widget::alignV( uint alignment )
{
   _fields = setbits( _fields, 18, 2, alignment );
}

//------------------------------------------------------------------------------
//!
inline void
Widget::alignHV( uint axis, uint alignment )
{
   if( axis == 0 )
   {
      alignH( alignment );
   }
   else
   {
      alignV( alignment );
   }
}

NAMESPACE_END

#if _MSC_VER
#pragma warning(pop)
#endif //_MSC_VER

#endif //FUSION_WIDGET_H
