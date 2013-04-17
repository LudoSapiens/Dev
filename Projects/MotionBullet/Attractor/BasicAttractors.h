/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef MOTIONBULLET_BASICATTRACTORS_H
#define MOTIONBULLET_BASICATTRACTORS_H

#include <MotionBullet/StdDefs.h>
#include <MotionBullet/Attractor/Attractor.h>

#include <CGMath/Vec3.h>


NAMESPACE_BEGIN


/*==============================================================================
   CLASS DirectionalAttractor
==============================================================================*/
//!
class DirectionalAttractor
   : public Attractor
{

public: 

   /*----- methods -----*/

   MOTION_DLL_API DirectionalAttractor();
   MOTION_DLL_API DirectionalAttractor( const Vec3f& acceleration );

   MOTION_DLL_API void addForce( const Vector< RCP<RigidBody> >& );

   // Parameters.
   inline void acceleration( const Vec3f& );
   inline const Vec3f& acceleration() const;
   

protected: 
   
   /*----- friends -----*/

   friend class MotionWorld;

   /*----- methods -----*/

   MOTION_DLL_API virtual ~DirectionalAttractor();

private:
   
   /*----- data members -----*/

   Vec3f _acceleration;

};

//------------------------------------------------------------------------------
//!
inline void 
DirectionalAttractor::acceleration( const Vec3f& acceleration )
{
   _acceleration = acceleration;
}
   
//------------------------------------------------------------------------------
//!
inline const Vec3f& 
DirectionalAttractor::acceleration() const
{
   return _acceleration;
}

/*==============================================================================
   CLASS PointAttractor
==============================================================================*/
//!
class PointAttractor
   : public Attractor
{

public: 

   /*----- methods -----*/

   MOTION_DLL_API PointAttractor();

   MOTION_DLL_API void addForce( const Vector< RCP<RigidBody> >& );

protected: 
   
   /*----- friends -----*/

   friend class MotionWorld;

   /*----- methods -----*/

   MOTION_DLL_API virtual ~PointAttractor();

private:
   
   /*----- data members -----*/

   Vec3f _pos;
   float _factor;
   float _radius;

};

/*==============================================================================
   CLASS BeamAttractor
==============================================================================*/
//!
class BeamAttractor
   : public Attractor
{

public: 

   /*----- methods -----*/

   MOTION_DLL_API BeamAttractor();

   MOTION_DLL_API void addForce( const Vector< RCP<RigidBody> >& );

protected: 
   
   /*----- friends -----*/

   friend class MotionWorld;

   /*----- methods -----*/

   MOTION_DLL_API virtual ~BeamAttractor();

private:
   
   /*----- data members -----*/

   Vec3f _pos;
   float _factor;
   float _radius;
   Vec3f _direction;
   float _cutoffAngle;

};

NAMESPACE_END

#endif
