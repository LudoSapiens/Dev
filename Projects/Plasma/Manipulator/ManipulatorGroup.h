/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_MANIPULATOR_GROUP_H
#define PLASMA_MANIPULATOR_GROUP_H

#include <Plasma/StdDefs.h>
#include <Plasma/Manipulator/Manipulator.h>


NAMESPACE_BEGIN

class RenderableGroup;

/*==============================================================================
  CLASS ManipulatorGroup
==============================================================================*/

class ManipulatorGroup:
   public Manipulator
{

public:

   /*----- methods -----*/

   PLASMA_DLL_API ManipulatorGroup();

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

   PLASMA_DLL_API virtual void camera( Camera* c );
   PLASMA_DLL_API virtual void viewport( Viewport* v );
   PLASMA_DLL_API virtual void widget( Widget* w );

   using Manipulator::camera;
   using Manipulator::viewport;
   using Manipulator::widget;

   // Group.
   PLASMA_DLL_API void remove( Manipulator* );
   PLASMA_DLL_API void clear();
   PLASMA_DLL_API void addFront( Manipulator* );
   PLASMA_DLL_API void addBack( Manipulator* );
   PLASMA_DLL_API void add( Manipulator*, int pos );

   inline const Vector< RCP<Manipulator> >& manipulators() const { return _manips; }

protected:

   /*----- methods -----*/

   virtual ~ManipulatorGroup();

private:

   /*----- data members -----*/

   Vector< RCP<Manipulator> > _manips;
   RCP<RenderableGroup>       _renderable;
};


NAMESPACE_END

#endif
