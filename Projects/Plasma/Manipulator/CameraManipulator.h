/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_CAMERAMANIPULATOR_H
#define PLASMA_CAMERAMANIPULATOR_H

#include <Plasma/StdDefs.h>
#include <Plasma/Manipulator/Manipulator.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS CameraManipulator
==============================================================================*/

//!

class CameraManipulator
   : public Manipulator
{

public:

   /*----- methods -----*/
   static void initialize();

   PLASMA_DLL_API CameraManipulator();

   inline const Vec3f& pointOfInterest() const;
   inline bool isLocked() const;

   PLASMA_DLL_API void lookAt(
      float px, float py, float pz,
      float ax, float ay, float az,
      float ux, float uy, float uz
   );
   PLASMA_DLL_API void pointOfInterest( const Vec3f& pos );
   PLASMA_DLL_API void primaryAxis( const Vec3f& axis );

   PLASMA_DLL_API void lock( bool lock );

   PLASMA_DLL_API virtual void render( const RCP<Gfx::RenderNode>& rn );
   PLASMA_DLL_API virtual void onCameraChange();
   PLASMA_DLL_API virtual bool onPointerPress( const Event& );
   PLASMA_DLL_API virtual bool onPointerRelease( const Event& );
   PLASMA_DLL_API virtual bool onPointerMove( const Event& );
   PLASMA_DLL_API virtual bool onPointerScroll( const Event& );
   PLASMA_DLL_API virtual bool onAccelerate( const Event& );

protected:

   /*----- methods -----*/

   virtual ~CameraManipulator();

   void forward( const Vec2f& pos );
   void orbit( float primAngle, float secAngle );
   void tilt( float primAngle, float secAngle );
   void pan( const Vec2f& pos );

private:

   /*----- data members -----*/

   Vec3f       _primaryAxis;
   Vec3f       _poi;
   bool        _isLocked;
   // Computed at pointer press.
   Reff        _ref;
   Vec3f       _secondaryAxis;
   Vec3f       _grabPoi;
   Vec2f       _grabPos;
   Vec3f       _accel;
   float       _direction;
   float       _scale;
   int         _mode;
};

//------------------------------------------------------------------------------
//!
inline const Vec3f&
CameraManipulator::pointOfInterest() const
{
   return _poi;
}

//------------------------------------------------------------------------------
//!
inline bool
CameraManipulator::isLocked() const
{
   return _isLocked;
}

NAMESPACE_END

#endif
