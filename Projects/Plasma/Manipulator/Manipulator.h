/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_MANIPULATOR_H
#define PLASMA_MANIPULATOR_H

#include <Plasma/StdDefs.h>
#include <Plasma/World/Viewport.h>

#include <Fusion/VM/VMObject.h>

#include <Gfx/Pass/RenderNode.h>

#include <Base/Util/RCObject.h>

NAMESPACE_BEGIN

class Event;

/*==============================================================================
  CLASS Manipulator
==============================================================================*/

//! Base class for manipulator.

class Manipulator:
   public RCObject,
   public VMProxy
{

public:

   /*----- static methods -----*/

   static void initialize();

   /*----- methods -----*/

   PLASMA_DLL_API Manipulator();

   // Rendering.
   PLASMA_DLL_API virtual void render( const RCP<Gfx::RenderNode>& );
   PLASMA_DLL_API virtual Renderable* renderable() const;

   // Events.
   PLASMA_DLL_API virtual void onCameraChange();
   PLASMA_DLL_API virtual void onViewportChange();
   PLASMA_DLL_API virtual bool onPointerPress( const Event& ev );
   PLASMA_DLL_API virtual bool onPointerRelease( const Event& ev );
   PLASMA_DLL_API virtual bool onPointerMove( const Event& ev );
   PLASMA_DLL_API virtual bool onPointerCancel( const Event& ev );
   PLASMA_DLL_API virtual bool onPointerScroll( const Event& ev );
   PLASMA_DLL_API virtual bool onKeyPress( const Event& ev );
   PLASMA_DLL_API virtual bool onKeyRelease( const Event& ev );
   PLASMA_DLL_API virtual bool onChar( const Event& ev );
   PLASMA_DLL_API virtual bool onAccelerate( const Event& ev );

   inline Camera*   camera()   const;
   inline Viewport* viewport() const;
   inline Widget*   widget()   const;
   inline World*    world()    const;

   PLASMA_DLL_API virtual void camera( Camera* cam );
   PLASMA_DLL_API virtual void viewport( Viewport* viewport );
   PLASMA_DLL_API virtual void widget( Widget* w );

   // VM.
   PLASMA_DLL_API virtual const char*  meta() const;
   PLASMA_DLL_API void init( VMState* vm );
   PLASMA_DLL_API virtual bool performGet( VMState* vm );
   PLASMA_DLL_API virtual bool performSet( VMState* vm );

protected:

   /*----- methods -----*/

   virtual ~Manipulator();

private:

   /*----- data members -----*/

   Widget*    _widget;
   Viewport*  _viewport;
};

//------------------------------------------------------------------------------
//!
inline Camera*
Manipulator::camera() const
{
   if( _viewport )  return _viewport->camera();
   return nullptr;
}

//------------------------------------------------------------------------------
//!
inline Viewport*
Manipulator::viewport() const
{
   return _viewport;
}

//------------------------------------------------------------------------------
//!
inline Widget*
Manipulator::widget() const
{
   return _widget;
}

//------------------------------------------------------------------------------
//!
inline World*
Manipulator::world() const
{
   if( _viewport ) return _viewport->world();
   return nullptr;
}

NAMESPACE_END

#endif
