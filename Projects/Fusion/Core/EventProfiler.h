/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_EVENT_PROFILER_H
#define FUSION_EVENT_PROFILER_H

#include <Fusion/StdDefs.h>

#include <Base/IO/BinaryStream.h>
#include <Base/IO/TextStream.h>
#include <Base/Util/Timer.h>

NAMESPACE_BEGIN

#if !defined( PROFILE_EVENTS )
#  define PROFILE_EVENTS 1
#endif

/*==============================================================================
  CLASS EventProfiler
==============================================================================*/
class EventProfiler:
   public Timer
{
public:

   /*----- types -----*/

   enum EventType
   {
      CORE_BEGIN,
      CORE_END,
      DISPLAY_BEGIN,
      DISPLAY_END,
      EXEC_BEGIN,
      EXEC_END,
      GUI_BEGIN,
      GUI_END,
      LOOP_BEGIN,
      LOOP_END,
      LOOPS_BEGIN,
      LOOPS_END,
      PROCESS_EVENTS_BEGIN,
      PROCESS_EVENTS_END,
      RENDER_BEGIN,
      RENDER_END,
      WORLD_BEGIN,
      WORLD_BEGIN_ANIMATORS,
      WORLD_BEGIN_BRAINS,
      WORLD_BEGIN_COMMANDS_AFTER_BRAINS,
      WORLD_BEGIN_ACTIONS,
      WORLD_BEGIN_COMMANDS_AFTER_ACTIONS,
      WORLD_BEGIN_PHYSICS,
      WORLD_BEGIN_ACTIONS_AFTER_PHYSICS,
      WORLD_BEGIN_COMMANDS_AFTER_PHYSICS,
      WORLD_END,
      NUM_EVENT_TYPES,
   };

   struct Event
   {
      double    _timestamp;
      uint32_t  _event;
      uint32_t  _count;
      Event() {}
      Event( double t, EventType e, uint32_t c ):
         _timestamp(t), _event(e), _count(c) {}

      inline EventType  event() const { return EventType(_event); }

      BinaryStream&  operator<<( BinaryStream& os ) const
      {
         return os << _timestamp << _event << _count;
      }

      BinaryStream&  operator>>( BinaryStream& is )
      {
         return is >> _timestamp >> _event >> _count;
      }
   };

   typedef Vector<Event>::ConstIterator  ConstIterator;

   /*----- methods -----*/

   inline EventProfiler( size_t cap = (1<<16) ) { _events.reserve(cap); }

   inline void  add( EventType event, uint32_t count = -1 );

   inline size_t  numEvents() const { return _events.size(); }

   inline void  clear() { _events.clear(); }

   inline ConstIterator  begin() const { return _events.begin(); }
   inline ConstIterator  end  () const { return _events.end();   }

   inline ConstIterator  findFirst( EventType event ) const;
   inline ConstIterator  findFirst( EventType event, size_t maxCount ) const;
   inline ConstIterator  findLast( EventType event ) const;
   inline ConstIterator  findLast( EventType event, size_t maxCount ) const;
   inline ConstIterator  findLast( EventType event, const ConstIterator& from ) const;

   inline const Event&  last() const { return _events.back(); }

   FUSION_DLL_API void  print( TextStream& os = StdErr ) const;

   FUSION_DLL_API BinaryStream&  operator<<( BinaryStream& os ) const;

   FUSION_DLL_API BinaryStream&  operator>>( BinaryStream& is );

   FUSION_DLL_API bool  save( const char* filename ) const;

   FUSION_DLL_API bool  load( const char* filename );

protected:

   /*----- methods -----*/

   String  dur( ConstIterator from, ConstIterator to ) const;

   /*----- data members -----*/

   Vector<Event>  _events;

private:
}; //class EventProfiler

//------------------------------------------------------------------------------
//!
FUSION_DLL_API const char*  toStr( EventProfiler::EventType e );

//------------------------------------------------------------------------------
//!
FUSION_DLL_API String  toStr( const EventProfiler::Event& e );

//------------------------------------------------------------------------------
//!
inline BinaryStream&  operator<<( BinaryStream& os, const EventProfiler::Event& ev )
{
   return ev.operator<<( os );
}

//------------------------------------------------------------------------------
//!
inline BinaryStream&  operator>>( BinaryStream& is, EventProfiler::Event& ev )
{
   return ev.operator>>( is );
}

//------------------------------------------------------------------------------
//!
inline void
EventProfiler::add( EventType event, uint32_t count )
{
#if PROFILE_EVENTS
   _events.pushBack( Event(elapsed(), event, count) );
#else
   unused( event );
   unused( count );
#endif
}

//------------------------------------------------------------------------------
//!
inline EventProfiler::ConstIterator
EventProfiler::findFirst( EventType event ) const
{
   for( auto cur = begin(); cur < end(); ++cur )
   {
      if( (*cur)._event == event )  return cur;
   }
   return end();
}

//------------------------------------------------------------------------------
//!
inline EventProfiler::ConstIterator
EventProfiler::findFirst( EventType event, size_t maxCount ) const
{
   for( auto cur = begin(); cur < end() && maxCount > 0; ++cur, --maxCount )
   {
      if( (*cur)._event == event )  return cur;
   }
   return end();
}

//------------------------------------------------------------------------------
//!
inline EventProfiler::ConstIterator
EventProfiler::findLast( EventType event ) const
{
   for( auto cur = end()-1; cur >= begin(); --cur )
   {
      if( (*cur)._event == event )  return cur;
   }
   return end();
}

//------------------------------------------------------------------------------
//!
inline EventProfiler::ConstIterator
EventProfiler::findLast( EventType event, size_t maxCount ) const
{
   for( auto cur = end()-1; cur >= begin() && maxCount > 0; --cur, --maxCount )
   {
      if( (*cur)._event == event )  return cur;
   }
   return end();
}

//------------------------------------------------------------------------------
//!
inline EventProfiler::ConstIterator
EventProfiler::findLast( EventType event, const ConstIterator& from ) const
{
   for( auto cur = from-1; cur != begin(); --cur )
   {
      if( (*cur)._event == event )  return cur;
   }
   return end();
}

#if PROFILE_EVENTS
#  define PROFILE_EVENT( e )       Core::profiler().add( e )
#  define PROFILE_EVENT_C( e, c )  Core::profiler().add( e, uint32_t(c) )
#else
#  define PROFILE_EVENT( e )       do {} while(false)
#  define PROFILE_EVENT_C( e, c )  do {} while(false)
#endif

NAMESPACE_END

#endif //FUSION_EVENT_PROFILER_H
