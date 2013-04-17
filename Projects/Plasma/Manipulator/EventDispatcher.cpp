/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Manipulator/EventDispatcher.h>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

UNNAMESPACE_END


/*==============================================================================
  CLASS EventDispatch
==============================================================================*/

//-----------------------------------------------------------------------------
//!
void
EventDispatcher::onEvent( EventCondition* cond, Callback call )
{
   Hash hash = toHash( ANY_TYPE );
   _callbacks[hash].add( cond, call );
}

//-----------------------------------------------------------------------------
//!
void
EventDispatcher::onPointer( EventCondition* cond, Callback call )
{
   static const Event::EventType  _kPointerEventTypes[] = {
      Event::POINTER_PRESS,
      Event::POINTER_RELEASE,
      Event::POINTER_MOVE,
      Event::POINTER_SCROLL,
      Event::POINTER_CANCEL
   };
   Hash hash;
   for( uint i = 0; i < sizeof(_kPointerEventTypes)/sizeof(_kPointerEventTypes[0]); ++i )
   {
      hash = toHash( _kPointerEventTypes[i] );
      _callbacks[hash].add( cond, call );
   }
}

//-----------------------------------------------------------------------------
//!
void
EventDispatcher::onPointer( int button, EventCondition* cond, Callback call )
{
   Hash hash;
   hash = toHash( Event::POINTER_PRESS, button );
   _callbacks[hash].add( cond, call );
   hash = toHash( Event::POINTER_RELEASE, button );
   _callbacks[hash].add( cond, call );
}

//-----------------------------------------------------------------------------
//!
void
EventDispatcher::onPointerPress( EventCondition* cond, Callback call )
{
   Hash hash = toHash( Event::POINTER_PRESS );
   _callbacks[hash].add( cond, call );
}

//-----------------------------------------------------------------------------
//!
void
EventDispatcher::onPointerPress( int button, EventCondition* cond, Callback call )
{
   Hash hash = toHash( Event::POINTER_PRESS, button );
   _callbacks[hash].add( cond, call );
}

//-----------------------------------------------------------------------------
//!
void
EventDispatcher::onPointerRelease( EventCondition* cond, Callback call )
{
   Hash hash = toHash( Event::POINTER_RELEASE );
   _callbacks[hash].add( cond, call );
}

//-----------------------------------------------------------------------------
//!
void
EventDispatcher::onPointerRelease( int button, EventCondition* cond, Callback call )
{
   Hash hash = toHash( Event::POINTER_RELEASE, button );
   _callbacks[hash].add( cond, call );
}

//-----------------------------------------------------------------------------
//!
void
EventDispatcher::onPointerMove( EventCondition* cond, Callback call )
{
   Hash hash = toHash( Event::POINTER_MOVE );
   _callbacks[hash].add( cond, call );
}

//-----------------------------------------------------------------------------
//!
void
EventDispatcher::onPointerScroll( EventCondition* cond, Callback call )
{
   Hash hash = toHash( Event::POINTER_SCROLL );
   _callbacks[hash].add( cond, call );
}

//-----------------------------------------------------------------------------
//!
void
EventDispatcher::onPointerCancel( EventCondition* cond, Callback call )
{
   Hash hash = toHash( Event::POINTER_CANCEL );
   _callbacks[hash].add( cond, call );
}

//-----------------------------------------------------------------------------
//!
void
EventDispatcher::onKey( EventCondition* cond, Callback call )
{
   Hash hash;
   hash = toHash( Event::KEY_PRESS );
   _callbacks[hash].add( cond, call );
   hash = toHash( Event::KEY_RELEASE );
   _callbacks[hash].add( cond, call );
}

//-----------------------------------------------------------------------------
//!
void
EventDispatcher::onKey( int key, EventCondition* cond, Callback call )
{
   Hash hash;
   hash = toHash( Event::KEY_PRESS, key );
   _callbacks[hash].add( cond, call );
   hash = toHash( Event::KEY_RELEASE, key );
   _callbacks[hash].add( cond, call );
}

//-----------------------------------------------------------------------------
//!
void
EventDispatcher::onKeyPress( EventCondition* cond, Callback call )
{
   Hash hash = toHash( Event::KEY_PRESS );
   _callbacks[hash].add( cond, call );
}

//-----------------------------------------------------------------------------
//!
void
EventDispatcher::onKeyPress( int key, EventCondition* cond, Callback call )
{
   Hash hash = toHash( Event::KEY_PRESS, key );
   _callbacks[hash].add( cond, call );
}

//-----------------------------------------------------------------------------
//!
void
EventDispatcher::onKeyRelease( EventCondition* cond, Callback call )
{
   Hash hash = toHash( Event::KEY_RELEASE );
   _callbacks[hash].add( cond, call );
}

//-----------------------------------------------------------------------------
//!
void
EventDispatcher::onKeyRelease( int key, EventCondition* cond, Callback call )
{
   Hash hash = toHash( Event::KEY_RELEASE, key );
   _callbacks[hash].add( cond, call );
}


//-----------------------------------------------------------------------------
//!
void
EventDispatcher::onChar( EventCondition* cond, Callback call )
{
   Hash hash = toHash( Event::CHAR );
   _callbacks[hash].add( cond, call );
}

//-----------------------------------------------------------------------------
//!
void
EventDispatcher::onChar( int theChar, EventCondition* cond, Callback call )
{
   Hash hash = toHash( Event::CHAR, theChar );
   _callbacks[hash].add( cond, call );
}

//-----------------------------------------------------------------------------
//!
void
EventDispatcher::onHID( EventCondition* cond, Callback call )
{
   Hash hash = toHash( Event::HID_EVENT );
   _callbacks[hash].add( cond, call );
}

#if 0
//-----------------------------------------------------------------------------
//!
void
EventDispatcher::onHID( int deviceTypeID, EventCondition* cond, Callback call )
{
}

//-----------------------------------------------------------------------------
//!
void
EventDispatcher::onHID( int deviceTypeID, int deviceID, EventCondition* cond, Callback call )
{
}

//-----------------------------------------------------------------------------
//!
void
EventDispatcher::onHID( int deviceTypeID, int deviceID, int controlID, EventCondition* cond, Callback call )
{
}

//-----------------------------------------------------------------------------
//!
void
EventDispatcher::onHID( int deviceTypeID, int deviceID, int controlID, float value, EventCondition* cond, Callback call )
{
}
#endif

//-----------------------------------------------------------------------------
//!
void
EventDispatcher::onAccelerate( EventCondition* cond, Callback call )
{
   Hash hash = toHash( Event::ACCELERATE );
   _callbacks[hash].add( cond, call );
}

//-----------------------------------------------------------------------------
//!
bool
EventDispatcher::dispatch( const Event& ev )
{
   Hash hash;
   switch( ev.type() )
   {
      case Event::POINTER_PRESS  :  hash = toHash( ev.type(), ev.value() ); break;
      case Event::POINTER_RELEASE:  hash = toHash( ev.type(), ev.value() ); break;
      case Event::POINTER_MOVE   :  hash = toHash( ev.type()             ); break;
      case Event::POINTER_ENTER  :  return false;
      case Event::POINTER_LEAVE  :  return false;
      case Event::POINTER_CHANGE :  return false;
      case Event::POINTER_SCROLL :  hash = toHash( ev.type()             ); break;
      case Event::POINTER_DELETE :  return false;
      case Event::POINTER_CANCEL :  hash = toHash( ev.type()             ); break;
      case Event::KEY_PRESS      :  hash = toHash( ev.type(), ev.value() ); break;
      case Event::KEY_RELEASE    :  hash = toHash( ev.type(), ev.value() ); break;
      case Event::CHAR           :  hash = toHash( ev.type(), ev.value() ); break;
      case Event::HID_EVENT      :  hash = toHash( ev.type()             ); break;
      case Event::ACCELERATE     :  hash = toHash( ev.type()             ); break;
      default:
         CHECK( false );
         return false;
   }

   HashTable<Hash, ConditionCallbacks>::Iterator cur;
   HashTable<Hash, ConditionCallbacks>::Iterator end = _callbacks.end();
   cur = _callbacks.find( hash );
   if( cur != end )
   {
      if( cur.data().check(ev) )  return true;
   }

   // Convert to a generic ID first.
   Hash hashAnyID = toAnyID( hash );
   if( hashAnyID != hash )
   {
      cur = _callbacks.find( hashAnyID );
      if( cur != end )
      {
         if( cur.data().check(ev) )  return true;
      }
   }

   // Convert to a generic type and a generic ID.
   Hash hashAnyAll = toHash( ANY_TYPE, ANY_VALUE );
   if( hashAnyAll != hashAnyID )
   {
      cur = _callbacks.find( hashAnyAll );
      if( cur != end )
      {
         if( cur.data().check(ev) )  return true;
      }
   }

   return false;
}

//-----------------------------------------------------------------------------
//!
void
EventDispatcher::clear()
{
   _callbacks.clear();
}

//-----------------------------------------------------------------------------
//!
void
EventDispatcher::clear( Callback call )
{
   HashTable<Hash, ConditionCallbacks>::Iterator cur = _callbacks.begin();
   while( cur != _callbacks.end() )
   {
      ConditionCallbacks& hashedCallbacks = cur.data();
      hashedCallbacks.remove( call );
      if( hashedCallbacks.empty() )
      {
         _callbacks.erase( cur++ );
      }
      else
      {
         ++cur;
      }
   }
}
