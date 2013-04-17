/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_EVENT_DISPATCHER_H
#define PLASMA_EVENT_DISPATCHER_H

#include <Plasma/StdDefs.h>

#include <Fusion/Core/Event.h>

#include <Base/Adt/HashTable.h>
#include <Base/Dbg/Defs.h>
#include <Base/Msg/DelegateList.h>
#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>

NAMESPACE_BEGIN


/*==============================================================================
  CLASS EventCondition
==============================================================================*/
class EventCondition:
   public RCObject
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API EventCondition() {}
   PLASMA_DLL_API virtual ~EventCondition() {}

   PLASMA_DLL_API virtual bool  check( const Event& ev ) = 0;

protected:

private:
}; //class EventCondition


/*==============================================================================
  CLASS KeyCondition
==============================================================================*/
class KeyCondition
{
public:

   /*----- methods -----*/

   KeyCondition( int key ): _key( key ) {}

   PLASMA_DLL_API virtual bool  check( const Event& ev )
   {
      CHECK( ev.isKeyboardEvent() );
      return ev.value() == _key;
   }

protected:

   /*----- data members -----*/
   int  _key;

private:
}; //class KeyCondition


/*==============================================================================
  CLASS EventDispatcher
==============================================================================*/
class EventDispatcher
{
public:

   /*----- types -----*/

   typedef Delegate0<>  Callback;

   /*----- methods -----*/

   PLASMA_DLL_API void  onEvent( EventCondition* cond, Callback callback );

   PLASMA_DLL_API void  onPointer( EventCondition* cond, Callback callback );
   PLASMA_DLL_API void  onPointer( int button, EventCondition* cond, Callback callback );
   PLASMA_DLL_API void  onPointerPress( EventCondition* cond, Callback callback );
   PLASMA_DLL_API void  onPointerPress( int button, EventCondition* cond, Callback callback );
   PLASMA_DLL_API void  onPointerRelease( EventCondition* cond, Callback callback );
   PLASMA_DLL_API void  onPointerRelease( int button, EventCondition* cond, Callback callback );
   PLASMA_DLL_API void  onPointerMove( EventCondition* cond, Callback callback );
   PLASMA_DLL_API void  onPointerScroll( EventCondition* cond, Callback callback );
   PLASMA_DLL_API void  onPointerCancel( EventCondition* cond, Callback callback );

   PLASMA_DLL_API void  onKey( EventCondition* cond, Callback callback );
   PLASMA_DLL_API void  onKey( int key, EventCondition* cond, Callback callback );
   PLASMA_DLL_API void  onKeyPress( EventCondition* cond, Callback callback );
   PLASMA_DLL_API void  onKeyPress( int key, EventCondition* cond, Callback callback );
   PLASMA_DLL_API void  onKeyRelease( EventCondition* cond, Callback callback );
   PLASMA_DLL_API void  onKeyRelease( int key, EventCondition* cond, Callback callback );

   PLASMA_DLL_API void  onChar( EventCondition* cond, Callback callback );
   PLASMA_DLL_API void  onChar( int theChar, EventCondition* cond, Callback callback );

   PLASMA_DLL_API void  onHID( EventCondition* cond, Callback callback );
   //PLASMA_DLL_API void  onHID( int deviceTypeID, EventCondition* cond, Callback callback );
   //PLASMA_DLL_API void  onHID( int deviceTypeID, int deviceID, EventCondition* cond, Callback callback );
   //PLASMA_DLL_API void  onHID( int deviceTypeID, int deviceID, int controlID, EventCondition* cond, Callback callback );
   //PLASMA_DLL_API void  onHID( int deviceTypeID, int deviceID, int controlID, float value, EventCondition* cond, Callback callback );

   PLASMA_DLL_API void  onAccelerate( EventCondition* cond, Callback callback );


   PLASMA_DLL_API bool  dispatch( const Event& ev );

   PLASMA_DLL_API void  clear();
   PLASMA_DLL_API void  clear( Callback callback );

protected:

   /*----- types -----*/

   struct ConditionCallback
   {
      ConditionCallback() {}
      ConditionCallback( EventCondition* cond, Callback call ): _cond( cond ), _call( call ) {}
      bool  check( const Event& ev )
      {
         if( _cond.isValid() )
         {
            if( _cond->check( ev ) )
            {
               _call();
               return true;
            }
            else
            {
               return false; // Try next one.
            }
         }
         else
         {
            // Assume true.
            _call();
            return true;
         }
      }
      RCP<EventCondition>  _cond;
      Callback             _call;
   };

   struct ConditionCallbacks
   {
      typedef Vector<ConditionCallback>::Iterator  Iterator;
      void  add( EventCondition* cond, Callback call )
      {
         _callbacks.pushBack( ConditionCallback(cond, call) );
      }
      Iterator  find( Callback call )
      {
         for( Iterator cur = _callbacks.begin(); cur != _callbacks.end(); ++cur )
         {
            if( (*cur)._call == call )  return cur;
         }
         return _callbacks.end();
      }
      void  remove( Callback call )
      {
         Iterator w = find( call );
         if( w != _callbacks.end() )
         {
            for( Iterator r = w+1; r != _callbacks.end(); ++r )
            {
               if( !((*r)._call == call) )
               {
                  *w = *r;
                  ++w;
               }
            }
            _callbacks.erase( w, _callbacks.end() );
         }
      }
      bool  empty() const { return _callbacks.empty(); }
      bool  check( const Event& ev )
      {
         for( Vector<ConditionCallback>::Iterator cur = _callbacks.begin();
              cur != _callbacks.end();
              ++cur )
         {
            if( (*cur).check( ev ) )  return true;
         }
         return false;
      }
      Vector<ConditionCallback>  _callbacks;
   };

   // Hash[03:00] -  4b  Event type.
   // Hash[31:04] - 28b  Button (pointer event) or key (key event), 0x00FFFFFF for ANY events.
   typedef uint32_t  Hash;

   enum
   {
      ANY_TYPE  = 0x0000000F, //  4b of 1s.
      ANY_VALUE = 0x0FFFFFFF, // 28b of 1s.
   };

   /*----- data members -----*/

   HashTable<Hash, ConditionCallbacks>  _callbacks;  //!< A single hash table to contain all of the callbacks.

   /*----- methods -----*/

   Hash  toHash( uint type, uint32_t id28 = ANY_VALUE ) const
   {
      Hash hash = id28;
      hash <<= 4;
      hash |= type;
      return hash;
   }

   Hash  toAnyType( Hash hash ) const
   {
      return (Hash)(hash | ANY_TYPE);
   }

   Hash  toAnyID( Hash hash ) const
   {
      return (Hash)(hash | (ANY_VALUE<<4));
   }

private:
}; //class EventDispatcher


NAMESPACE_END

#endif //PLASMA_EVENT_DISPATCHER_H
