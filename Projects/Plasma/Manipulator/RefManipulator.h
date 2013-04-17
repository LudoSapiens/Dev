/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_REFMANIPULATOR_H
#define PLASMA_REFMANIPULATOR_H

#include <Plasma/StdDefs.h>
#include <Plasma/Manipulator/Manipulator.h>
#include <Plasma/Renderable/Renderable.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS RefRenderable
==============================================================================*/

class RefRenderable:
   public Renderable
{
public:

   /*----- enumerations -----*/

   enum Mode {
      NONE,
      AXIS_X,
      AXIS_Y,
      AXIS_Z
   };

   /*----- types -----*/

   typedef Delegate0<>      Callback;
   typedef Delegate0List<>  CallbackList;

   /*----- methods -----*/

   RefRenderable();
   virtual void render( Gfx::Pass&, const Viewport& ) const;

   void update();
   void update( const Reff&, bool cb = false );
   void worldSpace( bool v ) { _worldSpace = v; }
   bool worldSpace() const   { return _worldSpace; }
   void mode( Mode m )       { _mode = m; }
   Mode mode() const         { return _mode; }

   inline const Reff& referential() const { return _ref; }

   bool isAxisSelected( int i );

   float scalingFactor( const Viewport& vp ) const;

   Mat4f renderMatrix( const Viewport& vp ) const;

   Vec3f axis( int i );
   Vec3f axis();

   // Callback notification.
   void addOnModify( const Callback& cb )    { _callbacks.addDelegate( cb ); }
   void removeOnModify( const Callback& cb ) { _callbacks.removeDelegate( cb ); }

protected:

   /*----- data members -----*/

   Mode                   _mode;
   bool                   _worldSpace;
   Reff                   _ref;
   RCP<MeshGeometry>      _mesh;
   RCP<Gfx::Program>      _prog;
   RCP<Gfx::ConstantList> _constFront;
   RCP<Gfx::ConstantList> _constBack;
   CallbackList           _callbacks;
};

/*==============================================================================
   CLASS RefManipulator
==============================================================================*/

class RefManipulator:
   public Manipulator
{
public:

   /*----- methods -----*/

   RefManipulator( RefRenderable* r ): _renderable(r) {}

   virtual Renderable* renderable() const { return _renderable.ptr(); }

   inline const RCP<RefRenderable>&  refRenderable() const { return _renderable; }

   // Events.
   virtual bool onPointerPress( const Event& ev );
   virtual bool onPointerRelease( const Event& ev );
   virtual bool onPointerMove( const Event& ev );

protected:

   RCP<RefRenderable> _renderable;
   Vec2f              _axisDir;
   Vec3f              _axis;
   int                _mode;
};


NAMESPACE_END

#endif
