/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_TOUCH_CAMERA_MANIPULATOR_H
#define PLASMA_TOUCH_CAMERA_MANIPULATOR_H

#include <Plasma/StdDefs.h>
#include <Plasma/Manipulator/Manipulator.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS TouchCameraManipulator
==============================================================================*/
class TouchCameraManipulator:
   public Manipulator
{
public:

   /*----- static methods -----*/

   static void initialize();

   /*----- methods -----*/

   PLASMA_DLL_API TouchCameraManipulator();

   // Manipulator.
   virtual bool onPointerPress( const Event& );
   virtual bool onPointerRelease( const Event& );
   virtual bool onPointerMove( const Event& );
   virtual bool onPointerScroll( const Event& ev );
   virtual bool onPointerCancel( const Event& );

protected:

   /*----- methods -----*/

   virtual ~TouchCameraManipulator();

   void grabCamera();
   void releaseCamera();
   void updateCamera();
   void orbit( const Vec2f& );
   void translateLocal( const Vec2f& pan, const float zoom );
   void handlePinch( const Pointer& ptr, const Pointer& optr );

   uint getOtherActivePointerID( uint );

   /*----- data members -----*/

   Set<uint> _activePointers;
   Vec3f     _poi;

   bool      _grabbed;
   float     _direction;
   Reff      _ref;
   Vec2f     _angles;
   Vec3f     _offset;
   Vec3f     _primaryAxis;
   Vec3f     _secondaryAxis;
};

NAMESPACE_END

#endif
