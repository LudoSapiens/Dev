/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef BASE_DEBUG_STREAM_H
#define BASE_DEBUG_STREAM_H

#include <Base/StdDefs.h>

#include <Base/Dbg/Defs.h>
#include <Base/IO/TextStream.h>
#include <Base/Util/Bits.h>
#include <Base/Util/RCObject.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS DebugStream
==============================================================================*/
class DebugStream
{
public:

   /*==============================================================================
     CLASS State
   ==============================================================================*/
   class State
   {
   public:

      /*----- methods -----*/

      State():
         _stream( StdNull ), _depth( NULL ), _fields( 0x0 )
      { }

      inline const String&  group() const { return _group; }

      inline bool  active() const { return getbits(_fields, 0, 1) != 0; }
      inline bool  inactive() const { return getbits(_fields, 0, 1) == 0; }
      inline void  activate() { _fields = setbits(_fields, 0, 1, 0x1); }
      inline void  deactivate() { _fields = setbits(_fields, 0, 1, 0x0); }
      inline void  toggle() { _fields = flipbits(_fields, 0, 1); }
      inline void  toggle( bool active ) { _fields = setbits(_fields, 0, 1, (active?0x1:0x0)); }

      inline TextStream&  stream() { return _stream; }

      inline uint  depth() const { return *_depth; }

      inline void  set( const String& group, IODevice* device, uint* depth, bool active )
      {
         _group = group;
         _stream.device( device );
         _depth = depth;
         toggle( active );
      }

      BASE_DLL_API const char*  pre();
      BASE_DLL_API State&  operator++();
      BASE_DLL_API State&  operator--();

   protected:

      /*----- methods -----*/
      inline uint  mode() const { return getbits(_fields, 1, 2); }
      inline void  mode( uint m ) { _fields = setbits(_fields, 1, 2, m); }

      /*----- data members -----*/

      String      _group;  //!< The name of the group.
      TextStream  _stream; //!< The output stream.
      uint*       _depth;  //!< The current depth (a reference tied to the IODevice).
      //! NAME          SIZE   LOCATION         DESCRIPTION
      //! active         (1)   _fields[ 0: 0]   Set if the stream is active.
      //! mode           (1)   _fields[ 2: 1]   The line mode (0: Opening line, 1: Continuation line, 2: Closing line).
      uint        _fields; //!< The line mode (0: Opening line, 1: Continuation line, 2: Closing line).

   private:
   }; //class State

   /*==============================================================================
     CLASS Block
   ==============================================================================*/
   class Block
   {
   public:
      ~Block() { if( _state ) --(*_state); }
   protected:
      friend class DebugStream;
      Block( State& state ) { if( state.active() ) { _state = &state; ++(*_state); } else { _state = NULL; } }
      State*  _state;
   }; //class Block


   /*----- static methods -----*/
   static BASE_DLL_API void  readConfigFile( const char* filename );

   /*----- methods -----*/

   BASE_DLL_API DebugStream( const String& groupName );
   BASE_DLL_API ~DebugStream();

   BASE_DLL_API void  device( IODevice* device );

   inline const State&  state() const { return *_state; }

   inline const String&  group() const { return _state->group(); }

   inline bool  active() const { return _state->active(); }
   inline bool  inactive() const { return _state->inactive(); }
   inline void  activate() { _state->activate(); }
   inline void  deactivate() { _state->deactivate(); }
   inline void  toggle() { _state->toggle(); }
   inline void  toggle( bool active ) { _state->toggle(active); }

   inline const char*  pre() { return _state->pre(); }
   inline uint  depth() const { return _state->depth(); }
   inline TextStream&  stream() { return _state->stream(); }
   inline Block  block() { return Block(*_state); }
   inline DebugStream&  operator++() { ++(*_state); return *this; }
   inline DebugStream&  operator--() { --(*_state); return *this; }

   // Output routines.
#if 0
   inline DebugStream&  operator<<( const String&  str ) { if( _state->active() ) _state->stream() << str; return *this; }
   inline DebugStream&  operator<<( const char*    str ) { if( _state->active() ) _state->stream() << str; return *this; }
   inline DebugStream&  operator<<( const char     val ) { if( _state->active() ) _state->stream() << val; return *this; }
   //inline DebugStream&  operator<<( const uchar    val ) { if( _state->active() ) _state->stream() << val; return *this; }
   //inline DebugStream&  operator<<( const short    val ) { if( _state->active() ) _state->stream() << val; return *this; }
   //inline DebugStream&  operator<<( const ushort   val ) { if( _state->active() ) _state->stream() << val; return *this; }
#if defined(__GNUC__) && defined(_WIN32)
   inline DebugStream&  operator<<( const int      val ) { if( _state->active() ) _state->stream() << val; return *this; }
   inline DebugStream&  operator<<( const uint     val ) { if( _state->active() ) _state->stream() << val; return *this; }
#endif
   inline DebugStream&  operator<<( const float    val ) { if( _state->active() ) _state->stream() << val; return *this; }
   inline DebugStream&  operator<<( const double   val ) { if( _state->active() ) _state->stream() << val; return *this; }
   inline DebugStream&  operator<<( const int8_t   val ) { if( _state->active() ) _state->stream() << val; return *this; }
   inline DebugStream&  operator<<( const uint8_t  val ) { if( _state->active() ) _state->stream() << val; return *this; }
   inline DebugStream&  operator<<( const int16_t  val ) { if( _state->active() ) _state->stream() << val; return *this; }
   inline DebugStream&  operator<<( const uint16_t val ) { if( _state->active() ) _state->stream() << val; return *this; }
   inline DebugStream&  operator<<( const int32_t  val ) { if( _state->active() ) _state->stream() << val; return *this; }
   inline DebugStream&  operator<<( const uint32_t val ) { if( _state->active() ) _state->stream() << val; return *this; }
   inline DebugStream&  operator<<( const int64_t  val ) { if( _state->active() ) _state->stream() << val; return *this; }
   inline DebugStream&  operator<<( const uint64_t val ) { if( _state->active() ) _state->stream() << val; return *this; }
#if defined(__APPLE__)
   inline DebugStream&  operator<<( const size_t  val ) { if( _state->active() ) _state->stream() << val; return *this; }
#endif
   inline DebugStream&  operator<<( const void*   ptr ) { if( _state->active() ) _state->stream() << ptr; return *this; }

   inline DebugStream&  operator<<( TextStreamFunc fun ) { if( _state->active() ) _state->stream() << fun; return *this; }
#endif

   template< typename T > inline DebugStream&  operator<<( const T& t ) { if( _state->active() ) _state->stream() << t; return *this; }

protected:

   /*----- data members -----*/

   State*  _state;

private:
}; //class DebugStream


//------------------------------------------------------------------------------
//! Various macros to help conciseness.

#if defined(_DEBUG)
// Macros to use in debug builds.

#define DBG_STREAM( var, group ) \
   DebugStream  var( group )

#define DBG_BLOCK( os, msg ) \
   DebugStream::Block _priv_##os = os.block(); \
   os << os.pre() << msg << nl

#define DBG_MSG( os, msg ) \
   os << os.pre() << msg << nl

#define DBG_MSG_BEGIN( os ) \
   os << os.pre()

#define DBG_MSG_END( os ) \
   os << nl

#else
// Macros to use in release builds.

#define DBG_STREAM( var, group ) \
   DebugStream  var( group )  // Required for DBG_BEGIN/DBG_END to work in release builds.

#define DBG_BLOCK( os, msg )

#define DBG_MSG( os, msg )

#define DBG_MSG_BEGIN( os ) \
   if( false ) \
   {

#define DBG_MSG_END( os ) \
   } \
   do { } while( false )

#endif //defined(_DEBUG)

#define REL_BLOCK( os, msg ) \
   DebugStream::Block _priv_##os = os.block(); \
   os << os.pre() << msg << nl

#define REL_MSG( os, msg ) \
   os << os.pre() << msg << nl

#define REL_MSG_BEGIN( os ) \
   os << os.pre()

#define REL_MSG_END( os ) \
   os << nl

NAMESPACE_END

#endif //BASE_DEBUG_STREAM_H
