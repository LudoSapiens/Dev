/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_EVENT_H
#define FUSION_EVENT_H

#include <Fusion/StdDefs.h>

#include <CGMath/Vec2.h>

NAMESPACE_BEGIN

// Forward declarations.
class Pointer;

/*==============================================================================
  CLASS Event
==============================================================================*/

//! Base class for event

class Event
{

public:

   /*----- types and enumerations ----*/

   enum EventType {
      POINTER_PRESS,
      POINTER_RELEASE,
      POINTER_MOVE,
      POINTER_ENTER,
      POINTER_LEAVE,
      POINTER_CHANGE,
      POINTER_SCROLL,
      POINTER_DELETE,
      POINTER_CANCEL,
      KEY_PRESS,
      KEY_RELEASE,
      CHAR,
      HID_EVENT,
      ACCELERATE,
      FOCUS_GRAB,
      FOCUS_LOSE,
      NB_EVENT
   };

   /*----- methods -----*/

   Event() {}

   // Static construction routines.
   static inline Event  Char( double timestamp, int theChar, uint count );

   static inline Event  HID( double timestamp, int deviceTypeID, int deviceID, int controlID, float value );

   static inline Event  KeyPress  ( double timestamp, int key );
   static inline Event  KeyRelease( double timestamp, int key );

   static inline Event  PointerEnter  ( double timestamp, uint id, const Vec2f& pos );
   static inline Event  PointerLeave  ( double timestamp, uint id, const Vec2f& pos );
   static inline Event  PointerMove   ( double timestamp, uint id, const Vec2f& pos );
   static inline Event  PointerPress  ( double timestamp, uint id, int button, const Vec2f& pos, uint count );
   static inline Event  PointerRelease( double timestamp, uint id, int button, const Vec2f& pos, uint count );
   static inline Event  PointerChange ( double timestamp, uint id, int iconID ); // TODO: To deprecate.
   static inline Event  PointerScroll ( double timestamp, uint id, float value, const Vec2f& pos );
   static inline Event  PointerScroll ( double timestamp, uint id, const Vec2f& sv, const Vec2f& pos );
   static inline Event  PointerDelete ( double timestamp, uint id );
   static inline Event  PointerCancel ( double timestamp, uint id );

   static inline Event FocusGrab( double timestamp );
   static inline Event FocusLose( double timestamp );

   static inline Event  Accelerate( double timestamp, float dx, float dy, float dz );

   static inline const Event&  Invalid() { return _invalid; }

   inline double timestamp() const;
   inline EventType type() const;
   inline const Vec2f& position() const;
   inline void position( const Vec2f& );
   inline int value() const;
   inline int value2() const;
   inline Vec2f scrollValue() const;
   inline void scrollValue( const Vec2f& );
   inline bool isRepeated() const;
   inline bool isPointerEvent() const;
   inline bool isKeyboardEvent() const;

   inline uint  pointerID() const;
   inline uint  count() const;
   FUSION_DLL_API Pointer&  pointer() const;

   inline int   deviceTypeID() const;
   inline int   deviceID() const;
   inline int   controlID() const;
   inline float valueFloat() const;

   inline float dx() const;
   inline float dy() const;
   inline float dz() const;

   FUSION_DLL_API String  toStr() const;

   /*----- static methods -----*/
   static FUSION_DLL_API bool isDoubleClick( const Event& ev1, const Event& ev2 );
   static FUSION_DLL_API bool isTripleClick( const Event& ev1, const Event& ev2, const Event& ev3 );

   static inline const char*  typeToStr( EventType type );

private:

   /*----- static data members -----*/

   static FUSION_DLL_API bool _pointerEvent[NB_EVENT];
   static FUSION_DLL_API bool _keyboardEvent[NB_EVENT];

   static FUSION_DLL_API const Event& _invalid;

   /*----- data members -----*/

   double    _timestamp;  //!< The time at which the event was submitted.
   EventType _type;
   union
   {
      float     _px;
      uint32_t  _deviceTypeID;
   };
   union
   {
      float     _py;
      uint32_t  _deviceID;
   };
   union
   {
      uint32_t  _pointerID;
      int32_t   _controlID;
      float     _dx;
   };
   union
   {
      int32_t  _value;
      float    _valueFloat;
      float    _dy;
   };
   union
   {
      uint32_t  _count;
      int32_t   _value2;
      float     _valueFloat2;
      float     _dz;
   };

   /*----- methods -----*/

   inline Event( double timestamp, EventType type );
   inline Event( double timestamp, EventType type, uint id, float value, const Vec2f& pos );
   inline Event( double timestamp, EventType type, uint id, int value, const Vec2f& pos );
   inline Event( double timestamp, EventType type, uint id, const Vec2f& pos, const Vec2f& val );
   inline Event( double timestamp, EventType type, uint id, int value, const Vec2f& pos, uint count );
   inline Event( double timestamp, EventType type, uint id, const Vec2f& pos );
   inline Event( double timestamp, EventType type, int value, uint count );
   inline Event( double timestamp, EventType type, uint id, int value );
   inline Event( double timestamp, EventType type, int deviceTypeID, int deviceID, int controlID, float value );
   inline Event( double timestamp, EventType type, float dx, float dy, float dz );
};

//------------------------------------------------------------------------------
//!
Event
Event::Char( double timestamp, int theChar, uint count )
{
   return Event( timestamp, CHAR, theChar, count );
}

//------------------------------------------------------------------------------
//!
Event
Event::HID( double timestamp, int deviceTypeID, int deviceID, int controlID, float value )
{
   return Event( timestamp, HID_EVENT, deviceTypeID, deviceID, controlID, value );
}

//------------------------------------------------------------------------------
//!
Event
Event::KeyPress( double timestamp, int key )
{
   return Event( timestamp, KEY_PRESS, key, 0u );
}
//------------------------------------------------------------------------------
//!
Event
Event::KeyRelease( double timestamp, int key )
{
   return Event( timestamp, KEY_RELEASE, key, 0u );
}

//------------------------------------------------------------------------------
//!
Event
Event::PointerEnter( double timestamp, uint id, const Vec2f& pos )
{
   return Event( timestamp, POINTER_ENTER, id, pos );
}
//------------------------------------------------------------------------------
//!
Event
Event::PointerLeave( double timestamp, uint id, const Vec2f& pos )
{
   return Event( timestamp, POINTER_LEAVE, id, pos );
}
//------------------------------------------------------------------------------
//!
Event
Event::PointerMove( double timestamp, uint id, const Vec2f& pos )
{
   return Event( timestamp, POINTER_MOVE, id, pos );
}
//------------------------------------------------------------------------------
//!
Event
Event::PointerPress( double timestamp, uint id, int button, const Vec2f& pos, uint count )
{
   return Event( timestamp, POINTER_PRESS, id, button, pos, count );
}
//------------------------------------------------------------------------------
//!
Event
Event::PointerRelease( double timestamp, uint id, int button, const Vec2f& pos, uint count )
{
   return Event( timestamp, POINTER_RELEASE, id, button, pos, count );
}
//------------------------------------------------------------------------------
//!
Event
Event::PointerChange( double timestamp, uint pointerID, int iconID )
{
   return Event( timestamp, POINTER_CHANGE, pointerID, iconID );
}
//------------------------------------------------------------------------------
//!
Event
Event::PointerScroll( double timestamp, uint id, float value, const Vec2f& pos )
{
   return Event( timestamp, POINTER_SCROLL, id, value, pos );
}
//------------------------------------------------------------------------------
//!
Event
Event::PointerScroll( double timestamp, uint id, const Vec2f& sv, const Vec2f& pos )
{
   return Event( timestamp, POINTER_SCROLL, id, pos, sv(1, 0) );
}
//------------------------------------------------------------------------------
//!
Event
Event::PointerDelete( double timestamp, uint pointerID )
{
   return Event( timestamp, POINTER_DELETE, pointerID, 0 );
}
//------------------------------------------------------------------------------
//!
Event
Event::PointerCancel( double timestamp, uint pointerID )
{
   return Event( timestamp, POINTER_CANCEL, pointerID, 0 );
}

//------------------------------------------------------------------------------
//!
Event
Event::FocusGrab( double timestamp )
{
   return Event( timestamp, FOCUS_GRAB );
}

//------------------------------------------------------------------------------
//!
Event
Event::FocusLose( double timestamp )
{
   return Event( timestamp, FOCUS_LOSE );
}

//------------------------------------------------------------------------------
//!
Event
Event::Accelerate( double timestamp, float dx, float dy, float dz )
{
   return Event( timestamp, ACCELERATE, dx, dy, dz );
}

//------------------------------------------------------------------------------
//!
inline
Event::Event( double timestamp, EventType type ):
   _timestamp( timestamp ), _type( type )
{
}

//------------------------------------------------------------------------------
//!
inline
Event::Event( double timestamp, EventType type, uint id, float value, const Vec2f& pos )
   : _timestamp( timestamp ), _type( type ), _px( pos.x ), _py( pos.y ), _pointerID( id ), _valueFloat( value ), _count( 0 )
{
}

//------------------------------------------------------------------------------
//!
inline
Event::Event( double timestamp, EventType type, uint id, int value, const Vec2f& pos )
   : _timestamp( timestamp ), _type( type ), _px( pos.x ), _py( pos.y ), _pointerID( id ), _value( value ), _count( 0 )
{
}

//------------------------------------------------------------------------------
//!
inline
Event::Event( double timestamp, EventType type, uint id, const Vec2f& pos, const Vec2f& val )
   : _timestamp( timestamp ), _type( type ), _px( pos.x ), _py( pos.y ), _pointerID( id ), _valueFloat( val.x ), _valueFloat2( val.y )
{
}

//------------------------------------------------------------------------------
//!
inline
Event::Event( double timestamp, EventType type, uint id, int value, const Vec2f& pos, uint count )
   : _timestamp( timestamp ), _type( type ), _px( pos.x ), _py( pos.y ), _pointerID( id ), _value( value ), _count( count )
{
}

//------------------------------------------------------------------------------
//!
inline
Event::Event( double timestamp, EventType type, uint id, const Vec2f& pos )
   : _timestamp( timestamp ), _type( type ), _px( pos.x ), _py( pos.y ), _pointerID( id ), _value( 0 ), _count( 0 )
{
}

//------------------------------------------------------------------------------
//!
inline
Event::Event( double timestamp, EventType type, int value, uint count )
   : _timestamp( timestamp ), _type( type ), _value( value ), _count( count )
{
}

//------------------------------------------------------------------------------
//!
inline
Event::Event( double timestamp, EventType type, uint id, int value )
   : _timestamp( timestamp ), _type( type ), _pointerID( id ), _value( value ), _count( 0 )
{
}

//------------------------------------------------------------------------------
//!
inline
Event::Event( double timestamp, EventType type, int deviceTypeID, int deviceID, int controlID, float value )
   : _timestamp( timestamp ), _type( type ), _deviceTypeID( deviceTypeID ), _deviceID( deviceID ), _controlID( controlID ), _valueFloat( value )
{
}

//------------------------------------------------------------------------------
//!
inline
Event::Event( double timestamp, EventType type, float dx, float dy, float dz )
   : _timestamp( timestamp ), _type( type ), _dx( dx ), _dy( dy ), _dz( dz )
{
}

//------------------------------------------------------------------------------
//!
inline double
Event::timestamp() const
{
   return _timestamp;
}

//------------------------------------------------------------------------------
//!
inline Event::EventType
Event::type() const
{
   return _type;
}

//------------------------------------------------------------------------------
//!
inline const Vec2f&
Event::position() const
{
   return Vec2f::as( &_px );
}

//------------------------------------------------------------------------------
//!
inline void
Event::position( const Vec2f& pos )
{
   _px = pos.x;
   _py = pos.y;
}

//------------------------------------------------------------------------------
//!
inline int
Event::value() const
{
   return _value;
}

//------------------------------------------------------------------------------
//!
inline int
Event::value2() const
{
   return _value2;
}

//------------------------------------------------------------------------------
//!
inline Vec2f
Event::scrollValue() const
{
   return Vec2f( _valueFloat2, _valueFloat ); // Swap back to Vec2( scrollX, scrollY ).
}

//------------------------------------------------------------------------------
//!
inline void
Event::scrollValue( const Vec2f& sv )
{
   _valueFloat  = sv.y;
   _valueFloat2 = sv.x;
}

//------------------------------------------------------------------------------
//!
inline bool
Event::isRepeated() const
{
   return _count > 1;
}

//------------------------------------------------------------------------------
//!
inline bool
Event::isPointerEvent() const
{
   return _pointerEvent[_type];
}

//------------------------------------------------------------------------------
//!
inline bool
Event::isKeyboardEvent() const
{
   return _keyboardEvent[_type];
}

//------------------------------------------------------------------------------
//!
uint
Event::pointerID() const
{
   return _pointerID;
}

//------------------------------------------------------------------------------
//!
uint
Event::count() const
{
   return _count;
}

//------------------------------------------------------------------------------
//!
int
Event::deviceTypeID() const
{
   return _deviceTypeID;
}

//------------------------------------------------------------------------------
//!
int
Event::deviceID() const
{
   return _deviceID;
}

//------------------------------------------------------------------------------
//!
int
Event::controlID() const
{
   return _controlID;
}

//------------------------------------------------------------------------------
//!
float
Event::valueFloat() const
{
   return _valueFloat;
}

//------------------------------------------------------------------------------
//!
float
Event::dx() const
{
   return _dx;
}

//------------------------------------------------------------------------------
//!
float
Event::dy() const
{
   return _dy;
}

//------------------------------------------------------------------------------
//!
float
Event::dz() const
{
   return _dz;
}

//------------------------------------------------------------------------------
//!
const char*
Event::typeToStr( EventType type )
{
   switch( type )
   {
      case POINTER_PRESS  : return "PointerPress";
      case POINTER_RELEASE: return "PointerRelease";
      case POINTER_MOVE   : return "PointerMove";
      case POINTER_ENTER  : return "PointerEnter";
      case POINTER_LEAVE  : return "PointerLeave";
      case POINTER_CHANGE : return "PointerChange";
      case POINTER_SCROLL : return "PointerScroll";
      case POINTER_DELETE : return "PointerDelete";
      case POINTER_CANCEL : return "PointerCancel";
      case KEY_PRESS      : return "KeyPress";
      case KEY_RELEASE    : return "KeyRelease";
      case CHAR           : return "Char";
      case HID_EVENT      : return "HID";
      case ACCELERATE     : return "Accelerate";
      default             : return "<INVALID>";
   }
}

NAMESPACE_END

#endif
