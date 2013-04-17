/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Core/Event.h>

#include <Fusion/Core/Core.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Event
==============================================================================*/

bool Event::_pointerEvent[NB_EVENT] = {
   true,  //POINTER_PRESS,
   true,  //POINTER_RELEASE,
   true,  //POINTER_MOVE,
   true,  //POINTER_ENTER,
   true,  //POINTER_LEAVE,
   true,  //POINTER_CHANGE,
   true,  //POINTER_SCROLL,
   true,  //POINTER_DELETE,
   true,  //POINTER_CANCEL,
   false, //KEY_PRESS,
   false, //KEY_RELEASE,
   false, //CHAR,
   false, //HID_EVENT,
   false, //ACCELERATE,
};

bool Event::_keyboardEvent[NB_EVENT] = {
   false, //POINTER_PRESS,
   false, //POINTER_RELEASE,
   false, //POINTER_MOVE,
   false, //POINTER_ENTER,
   false, //POINTER_LEAVE,
   false, //POINTER_CHANGE,
   false, //POINTER_SCROLL,
   false, //POINTER_DELETE,
   false, //POINTER_CANCEL,
   true,  //KEY_PRESS,
   true,  //KEY_RELEASE,
   true,  //CHAR,
   false, //HID_EVENT,
   false, //ACCELERATE,
};

//------------------------------------------------------------------------------
//!
const Event& Event::_invalid = Event( -1.0, Event::NB_EVENT, -1, 0u );

//------------------------------------------------------------------------------
//!
Pointer&
Event::pointer() const
{
   // Note:
   // The problem here is that to inline in Event.h, we'd need to include Core.
   // Since Core holds a vector of Events, it is not possible.
   // We could have put the declaration in Event.h, but the definition in Core.h,
   // but we decided against it (too hacky).
   // Hopefully, the compiler will be able to optimize this as an inline function.
   return Core::pointer( pointerID() );
}

//------------------------------------------------------------------------------
//!
String
Event::toStr() const
{
   String tmp;
   tmp += "{";
   tmp += typeToStr(type());
   tmp += ",";
   tmp += String().format("t=%f", timestamp());
   tmp += ",";
   switch( type() )
   {
      case POINTER_PRESS  :
      case POINTER_RELEASE:
      case POINTER_MOVE   :
      case POINTER_ENTER  :
      case POINTER_LEAVE  :
      case POINTER_CHANGE :
      case POINTER_SCROLL :
      case POINTER_DELETE :
      case POINTER_CANCEL :
      {
         tmp += String().format( "ptrID=%d", pointerID() );
         tmp += ",";
         tmp += String().format( "val=%d", value() );
         tmp += ",";
         tmp += String().format( "pos=(%f,%f)", position().x, position().y );
         tmp += ",";
         tmp += String().format( "cnt=%d", count() );
      }  break;
      case KEY_PRESS:
      case KEY_RELEASE:
      case CHAR:
      {
         tmp += String().format( "val=%d", value() );
         tmp += ",";
         tmp += String().format( "cnt=%d", count() );
      }  break;
      case HID_EVENT:
      {
         tmp += String().format( "devTypeID=%d", deviceTypeID() );
         tmp += ",";
         tmp += String().format( "devID=%d", deviceID() );
         tmp += ",";
         tmp += String().format( "ctrlID=%d", controlID() );
         tmp += ",";
         tmp += String().format( "val=%f", valueFloat() );
      }  break;
      case ACCELERATE:
      {
         tmp += String().format( "(%f,%f,%f", dx(), dy(), dz() );
      }  break;
      default:
         break;
   }
   tmp += "}";
   return tmp;
}

//------------------------------------------------------------------------------
//!
bool
Event::isDoubleClick
( const Event& ev1, const Event& ev2 )
{
   if( ev1.isPointerEvent() && ev1.type() == ev2.type() && ev1.value() == ev2.value() )
   {
      Vec2f deltaPos = ev1.position() - ev2.position();
      if( deltaPos.sqrLength() <= (4.0f*4.0f) )
      {
         //Missing the delay
         return true;
      }
   }

   return false;
}

//------------------------------------------------------------------------------
//!
bool
Event::isTripleClick
( const Event& ev1, const Event& ev2, const Event& ev3 )
{
   return isDoubleClick(ev1, ev2) && isDoubleClick(ev2, ev3);
}

NAMESPACE_END
