/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_FLYBYMANIPULATOR_H
#define PLASMA_FLYBYMANIPULATOR_H

#include <Plasma/StdDefs.h>
#include <Plasma/Manipulator/Manipulator.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS FlyByManipulator
==============================================================================*/
//! A simplistic manipulator to move about the scene while flying.
class FlyByManipulator:
   public Manipulator
{

public:

   enum
   {
      MODE_FLYING,
      MODE_WALKING
   };

   /*----- types -----*/
   class CameraAnimator;

   /*----- static methods -----*/
   static void initialize();

   PLASMA_DLL_API FlyByManipulator();

   inline int   mode() const { return _mode; }
   inline void  mode( int m ) { _mode = m; }

   PLASMA_DLL_API void lookAt(
      float px, float py, float pz,
      float ax, float ay, float az,
      float ux, float uy, float uz
   );

   PLASMA_DLL_API virtual void onCameraChange();
   PLASMA_DLL_API virtual bool onPointerPress( const Event& ev );
   PLASMA_DLL_API virtual bool onPointerRelease( const Event& ev );
   PLASMA_DLL_API virtual bool onPointerMove( const Event& ev );
   PLASMA_DLL_API virtual bool onPointerScroll( const Event& ev );
   PLASMA_DLL_API virtual bool onKeyPress( const Event& ev );
   PLASMA_DLL_API virtual bool onKeyRelease( const Event& ev );

   // VM.
   PLASMA_DLL_API virtual const char*  meta() const;
   PLASMA_DLL_API void init( VMState* vm );
   PLASMA_DLL_API virtual bool performGet( VMState* vm );
   PLASMA_DLL_API virtual bool performSet( VMState* vm );

protected:

   friend class CameraAnimator;

   /*----- methods -----*/

   virtual ~FlyByManipulator();

   void  readFrom( const Reff& ref );
   Reff  toRef() const;

private:

   /*----- data members -----*/
   int    _mode;      //!< Current mode (FLYING, WALKING).

   float  _theta;     //!< Azimuth angle (in radians) over the horizon plane (XZ).
   float  _thetaF;    //!< Speed factor for theta.

   float  _phi;       //!< Zenith (elevation) angle (in radians).
   float  _phiF;      //!< Speed factor for phi.
   float  _phiMin;    //!< Minimum allowed phi angle.
   float  _phiMax;    //!< Maximum allowed phi angle.

   Vec3f  _position;  //!< The position in the world.

   Vec3f  _velocity;  //!< Velocity expressed in camera space.
   float  _velScale;  //!< A scaling factor to apply to the velocity.

   RCP<CameraAnimator>  _animator;  //!< An animator used for movement.
   bool                 _animActive;  //!< A flag to indicate that the animator is running.
   uint   _pointer;   //!< Indicates the pointer ID which is used to control the movement.

}; // class FlyByManipulator

NAMESPACE_END

#endif //PLASMA_FLYBYMANIPULATOR_H
