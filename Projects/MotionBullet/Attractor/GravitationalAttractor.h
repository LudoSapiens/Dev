/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef MOTIONBULLET_GRAVITATIONALATTRACTORS_H
#define MOTIONBULLET_GRAVITATIONALATTRACTORS_H

#include <MotionBullet/StdDefs.h>
#include <MotionBullet/Attractor/Attractor.h>


NAMESPACE_BEGIN


namespace Constants
{
   //------------------------------------------------------------------------------
   //! Gravitational constant (in m^3 kg^-1 s^-2)
   const float G = 6.67300e-11f;

   //------------------------------------------------------------------------------
   //! Speed of light (in m/s)
   const float c = 299792458.0f;

   //------------------------------------------------------------------------------
   //! One over the speed of light (in s/m)
   const float inv_c = (float)(1.0 / 299792458.0);

   //------------------------------------------------------------------------------
   //! An astronomical unit (in m)
   const float AU = 149597870691.0f;

} // Constants


/*==============================================================================
   CLASS GravitationalAttractor
==============================================================================*/
//!
class GravitationalAttractor
   : public Attractor
{

public:

   /*----- methods -----*/

   MOTION_DLL_API GravitationalAttractor();
   MOTION_DLL_API GravitationalAttractor( const float threshold );

   virtual MOTION_DLL_API void addForce( const Vector< RCP<RigidBody> >& );

   // Parameters.
   inline void threshold( const float threshold ) { _threshold = threshold; }
   inline float threshold() const { return _threshold; }

protected:

   /*----- friends -----*/

   friend class MotionWorld;

   /*----- methods -----*/

   MOTION_DLL_API virtual ~GravitationalAttractor();

private:

   /*----- data members -----*/

   float _threshold;

};


NAMESPACE_END

#endif
