/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_POINTER_H
#define FUSION_POINTER_H

#include <Fusion/StdDefs.h>
#include <Fusion/Widget/Widget.h>

#include <Base/Util/Bits.h>

NAMESPACE_BEGIN

// Forward declarations.
class Event;

/*==============================================================================
  CLASS Pointer
==============================================================================*/
class Pointer
{
public:

   enum Icon
   {
      NONE,
      DEFAULT,
      SIZE_ALL,
      SIZE_T,
      SIZE_B,
      SIZE_BL,
      SIZE_BR,
      SIZE_TL,
      SIZE_TR,
      SIZE_L,
      SIZE_R,
      TEXT,
      HAND,
      WAIT,
      INVALID,
      _COUNT
   };

   enum Mode
   {
      MODE_ASBOLUTE,
      MODE_RELATIVE
   };

   /*----- methods -----*/

   FUSION_DLL_API Pointer();
   FUSION_DLL_API Pointer( const Pointer& p );
   FUSION_DLL_API ~Pointer();

   FUSION_DLL_API Pointer&  operator=( const Pointer& p );

   inline bool isValid() const              { return _id != (uint)-1; }

   inline uint id() const                   { return _id; }

   inline bool pressed() const              { return _state != 0x0; }
   inline bool pressed( uint button ) const { return getbits(_state, button, 1) != 0x0; }
   inline uint pressedState() const         { return _state; }

   inline uint icon() const                 { return _icon; }
   inline void icon( uint i )               { _icon = i; }

   inline bool persistent() const           { return getbits( _fields, 0, 1 ) != 0; }
   inline void persistent( bool val )       { _fields = setbits( _fields, 0, 1, (val?1:0) ); }

   inline Mode mode() const                 { return (Mode)getbits( _fields, 1, 1 ); }

   FUSION_DLL_API const Vec2f&  position() const;
   FUSION_DLL_API const Vec2f&  lastPosition() const;
   FUSION_DLL_API const Vec2f&  pressPosition() const;
   inline Vec2f  deltaPosition() const      { return position() - lastPosition(); }
   inline Vec2f  deltaPressPosition() const { return position() - pressPosition(); }

   void  follow( Widget* w )                { _followingWidget = w; }
   Widget*  following() const               { return _followingWidget.ptr(); }

   bool  grabbed() const                    { return _grabbingWidget.isValid(); }
   Widget*  grabbedWith() const             { return _grabbingWidget.ptr(); }

   Widget*  hoveringOver() const            { return _hoveringWidget.ptr(); }

   FUSION_DLL_API const Event&  curEvent() const;
   FUSION_DLL_API const Event&  lastEvent() const;
   FUSION_DLL_API const Event&  lastPressEvent() const;
   FUSION_DLL_API const Event&  lastReleaseEvent() const;

   FUSION_DLL_API Vec2f  getSpeed() const;

   FUSION_DLL_API bool  withinPress( float radius ) const;
   FUSION_DLL_API bool  withinDelay( double delay ) const;

protected:

   /*----- data types -----*/

   class PrivateData;

   /*----- data members -----*/

   uint    _id;         //!< A unique identification number (assigned by Core).
   uint    _fields;
   //!<    _fields[0:0]  Persistent - Indicates that cancel events should not delete this pointer.
   //!<    _fields[1:1]  Mode       - Indicates whether the pointer is abslute or relative.
   uint    _state;      //!< A bit for every button indicating whether or not it is pressed.
   uint    _icon;       //!< The icon currently representing the pointer.

   RCP<Widget>       _grabbingWidget;   //!< The widget currently grabbing this pointer.
   RCP<Widget>       _hoveringWidget;   //!< The widget currently under this pointer.
   RCP<Widget>       _followingWidget;  //!< The widget currently being followed.
   RCP<PrivateData>  _private;          //!< Some private implementation data.

   inline void  grabWith( Widget* w )  { _grabbingWidget = w; }
   inline void  hoverOver( Widget* w ) { _hoveringWidget = w; }
   FUSION_DLL_API void  curEvent( double time, const Event& event );

   /*----- methods -----*/

   friend class Core;

   inline void id( uint i )                     { _id = i; }
   inline void mode( Mode m )                   { _fields = setbits( _fields, 0, 1, m ); }
   inline void pressed( uint button, bool val ) { _state = setbits( _state, button, 1, (val?0x01:0x00) ); }
   inline void pressedState( uint s )           { _state = s; }
   inline void invalidate()                     { _id = (uint)-1; }

   FUSION_DLL_API void  init( const uint id, const bool persistent = true );
   FUSION_DLL_API void  clear();

private:
}; //class Pointer

NAMESPACE_END

#endif //FUSION_POINTER_H
