/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef MOTION_SPRINGS_INCLUDED
#define MOTION_SPRINGS_INCLUDED

#include <Motion/StdDefs.h>
#include <Motion/Constraint/Constraint.h>

#include <CGMath/Mat3.h>
#include <CGMath/Ref.h>
#include <CGMath/Vec3.h>

NAMESPACE_BEGIN

class RigidBody;

//------------------------------------------------------------------------------
//! Notes about the various springs.
//! Springs can be either anchored or not.  An anchored spring is tied to a fixed
//! position/orientation on one end, and ties a RigidBody on the other.
//! When it is not anchored, a spring ties two RigidBodies together.
//! A spring can contraint either the position, the orientation, or both.
//!
//! We have elected to implement springs as contraints, as it felt more natural
//! to specify them as such. But they are actually more akin to attractors, and
//! therefore only implement their prePositionStep() routines.

/*==============================================================================
  CLASS PosSpring
==============================================================================*/
class PosSpring:
   public Constraint
{
public:

   /*----- methods -----*/

   MOTION_DLL_API PosSpring( RigidBody*, RigidBody* );
   MOTION_DLL_API virtual ~PosSpring();

   // Attributes.
   inline float restLength() const { return _restLength; }
   inline float stiffness() const { return _stiffness; }
   inline float damping() const { return _damping; }
   inline float angularVelocityDamping() const { return _angVelDamp; }

   inline void restLength( const float l ) { _restLength = l; }
   inline void stiffness( const float k ) { _stiffness = k; }
   inline void damping( const float d ) { _damping = d; }
   inline void angularVelocityDamping( const float d ) { _angVelDamp = d; }

   MOTION_DLL_API void anchor( const Vec3f& posA, const Vec3f& posB, float restLength );
   MOTION_DLL_API void anchor( const Vec3f& posA, const Vec3f& posB );

   // Solving constraint.
   MOTION_DLL_API virtual void prePositionStep( double step );
   MOTION_DLL_API virtual bool solvePosition( double step );

   MOTION_DLL_API virtual void preVelocitiesStep();
   MOTION_DLL_API virtual bool solveVelocities();


protected:
   /*----- friends -----*/

   friend class MotionWorld;

   /*----- methods -----*/

   /*----- data members -----*/

   RCP<RigidBody>  _bodyA;
   RCP<RigidBody>  _bodyB;
   float           _restLength; //!< The length of the spring, at rest.
   float           _stiffness;  //!< The spring's stiffness constant.
   float           _damping;    //!< The spring's damping (friction) constant.
   float           _angVelDamp; //!< An angular velocity damping factor [0.0, 1.0].

   Vec3f           _anchorA;
   Vec3f           _anchorB;
}; //class PosSpring


/*==============================================================================
  CLASS AnchoredPosSpring
==============================================================================*/
class AnchoredPosSpring:
   public Constraint
{
public:

   /*----- methods -----*/

   MOTION_DLL_API AnchoredPosSpring( RigidBody* );
   MOTION_DLL_API virtual ~AnchoredPosSpring();

   // Attributes.
   inline float restLength() const { return _restLength; }
   inline float stiffness() const { return _stiffness; }
   inline float damping() const { return _damping; }
   inline float angularVelocityDamping() const { return _angVelDamp; }

   inline void restLength( const float l ) { _restLength = l; }
   inline void stiffness( const float k ) { _stiffness = k; }
   inline void damping( const float d ) { _damping = d; }
   inline void angularVelocityDamping( const float d ) { _angVelDamp = d; }

   MOTION_DLL_API void anchor( const Vec3f& posA, const Vec3f& posB, float restLength );
   MOTION_DLL_API void anchor( const Vec3f& posA, const Vec3f& posB );
   MOTION_DLL_API void anchor( const Vec3f& posB );

   // Solving constraint.
   MOTION_DLL_API virtual void prePositionStep( double step );
   MOTION_DLL_API virtual bool solvePosition( double step );

   MOTION_DLL_API virtual void preVelocitiesStep();
   MOTION_DLL_API virtual bool solveVelocities();


protected:
   /*----- friends -----*/

   friend class MotionWorld;

   /*----- methods -----*/

   /*----- data members -----*/

   RCP<RigidBody>  _bodyA;
   float           _restLength; //!< The length of the spring, at rest.
   float           _stiffness;  //!< The spring's stiffness constant.
   float           _damping;    //!< The spring's damping (friction) constant.
   float           _angVelDamp; //!< An angular velocity damping factor [0.0, 1.0].

   Vec3f           _anchorA;
   Vec3f           _anchorB;
}; //class AnchoredPosSpring


/*==============================================================================
  CLASS AnchoredSpring
==============================================================================*/
class AnchoredSpring:
   public Constraint
{
public:

   /*----- methods -----*/

   MOTION_DLL_API AnchoredSpring( RigidBody* );
   MOTION_DLL_API virtual ~AnchoredSpring();

   // Attributes.
   inline float restLength() const { return _restLength; }
   inline float stiffness() const { return _stiffness; }
   inline float damping() const { return _damping; }
   inline float maxTorque() const { return _maxTorque; }
   inline float angularVelocityDamping() const { return _angVelDamp; }

   inline void restLength( const float l ) { _restLength = l; }
   inline void stiffness( const float k ) { _stiffness = k; }
   inline void damping( const float d ) { _damping = d; }
   inline void maxTorque( const float t ) { _maxTorque = t; }
   inline void angularVelocityDamping( const float d ) { _angVelDamp = d; }

   MOTION_DLL_API void anchor( const Vec3f& posA, const Reff& refB, float restLength );
   MOTION_DLL_API void anchor( const Vec3f& posA, const Reff& refB );
   MOTION_DLL_API void anchor( const Reff& refB );

   // Solving constraint.
   MOTION_DLL_API virtual void prePositionStep( double step );
   MOTION_DLL_API virtual bool solvePosition( double step );

   MOTION_DLL_API virtual void preVelocitiesStep();
   MOTION_DLL_API virtual bool solveVelocities();


protected:
   /*----- friends -----*/

   friend class MotionWorld;

   /*----- methods -----*/

   /*----- data members -----*/

   RCP<RigidBody>  _bodyA;
   float           _restLength; //!< The length of the spring, at rest.
   float           _stiffness;  //!< The spring's stiffness constant.
   float           _damping;    //!< The spring's damping (friction) constant.
   float           _maxTorque;  //!< The maximum torque which the spring can apply.
   float           _angVelDamp; //!< An angular velocity damping factor [0.0, 1.0].

   Vec3f           _anchorA;
   Reff            _anchorB;
}; //class AnchoredSpring


NAMESPACE_END

#endif //MOTION_SPRINGS_INCLUDED
