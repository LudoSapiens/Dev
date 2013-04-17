/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_ANIMATIONNODE_H
#define PLASMA_ANIMATIONNODE_H

#include <Plasma/StdDefs.h>
#include <Plasma/Animation/SkeletalAnimation.h>
#include <Plasma/World/Brain.h>
#include <Plasma/World/SkeletalEntity.h>

#include <CGMath/CGConst.h>

#include <Base/ADT/DEQueue.h>
#include <Base/Util/RCObject.h>

NAMESPACE_BEGIN

class AnimationParameter;
class AnimationState;

/*==============================================================================
   CLASS AnimationNode
==============================================================================*/

class AnimationNode :
   public RCObject
{

public:

   /*----- methods -----*/

   virtual RCP<AnimationNode> clone() const = 0;
   virtual RCP<AnimationState> createState( AnimationParameter* p ) const = 0;
   virtual void perform( AnimationState*, double deltaTime ) = 0;

   virtual void getAnimations( Vector< SkeletalAnimation* >& anims );
   virtual void replaceAnimations(
      const Map< SkeletalAnimation*, RCP<SkeletalAnimation> >& anims
   );

   inline void name( const ConstString& name ) { _name = name; }
   inline const ConstString& name() const      { return _name; }

protected:

   /*----- methods -----*/

   AnimationNode();
   virtual ~AnimationNode();

   /*----- data members -----*/

   ConstString _name;
};

/*==============================================================================
   CLASS AnimationParameter
==============================================================================*/

class AnimationParameter
{
   public:

   /*----- methods -----*/

   AnimationParameter():
      _entity( nullptr ),
      _direction(0.0f,0.0f,1.0f),
      _speed(0.0f),
      _arrivingRange(2.0f),
      _arrivedRange(0.05f),
      _decelerateSpeed(1.0f),
      _maxAccel(10.0f),
      _maxAngularSpeed(2.0f),
      _jumpConst(0.0f),
      _jumpScale(1.0f)
   {}

   // Attributes getters.
   inline       SkeletalEntity* entity()       { return _entity; }
   inline const SkeletalEntity* entity() const { return _entity; }

   // Desired values.
   inline const Vec3f& direction() const   { return _direction; }
   inline float speed() const              { return _speed; }

   // Limiting values.
   inline float arrivingRange() const      { return _arrivingRange; }
   inline float arrivedRange() const       { return _arrivedRange; }
   inline float decelerateSpeed() const    { return _decelerateSpeed; }
   inline float maxAcceleration() const    { return _maxAccel;  }
   inline float maxAngularSpeed() const    { return _maxAngularSpeed; }
   inline const Vec3f& jumpConst() const   { return _jumpConst; }
   inline const Vec3f& jumpScale() const   { return _jumpScale; }

   // Attributes setters.
   void entity( SkeletalEntity* v )        { _entity = v; }

   // Desired values.
   void direction( const Vec3f& v )        { _direction = v; }
   void speed( float v )                   { _speed = v; }

   // Waypoints.
   const Vec3f& destination() const        { return _waypoints.front(); }
   bool hasDestination() const             { return !_waypoints.empty(); }
   void destination( const Vec3f& v )      { clearDestination(); _waypoints.pushBack(v); }
   void clearDestination()                 { clearWaypoints(); }

   uint numWaypoints() const               { return uint(_waypoints.size()); }
   const Vec3f& waypoint( uint i ) const   { return _waypoints.peek(i); }
   void popWaypoint()                      { _waypoints.popFront(); }
   void addWaypoint( const Vec3f& v )      { _waypoints.pushBack(v); }
   void clearWaypoints()                   { _waypoints.clear(); }

   // Limiting values.
   void arrivingRange( float v )           { _arrivingRange = v; }
   void arrivedRange( float v )            { _arrivedRange = v; }
   void decelerateSpeed( float v )         { _decelerateSpeed = v; }
   void maxAcceleration( float v )         { _maxAccel = v; }
   void maxAngularSpeed( float s )         { _maxAngularSpeed = s; }
   void jumpConst( const Vec3f& v )        { _jumpConst = v; }
   void jumpScale( const Vec3f& v )        { _jumpScale = v; }

protected:

   /*----- methods -----*/

   SkeletalEntity*      _entity;          //!< The entity being animated.
   // Desired values.
   Vec3f                _direction;       //!< Direction of the movement.
   DEQueue<Vec3f>       _waypoints;       //!< One or multiple destination(s) we want to reach (in order).
   float                _speed;           //!< Speed at which we want to reach the destination.
   // Limiting values.
   float                _arrivingRange;   //!< A threshold distance to consider the entity arriving to destination.
   float                _arrivedRange;    //!< A threshold distance to consider the entity arrived at destination.
   float                _decelerateSpeed; //!< Speed to target when decelerating.
   float                _maxAccel;        //!< Maximum acceleration for the linear velocity.
   float                _maxAngularSpeed;
   Vec3f                _jumpConst;       //!< Jump constants.
   Vec3f                _jumpScale;       //!< Jump scaling factors.
};

/*==============================================================================
   CLASS AnimationState
==============================================================================*/

class AnimationState :
      public RCObject
{

public:

   /*----- methods -----*/

   virtual void reset();

   // Attributes getters.
   inline bool canExit() const                   { return _canExit; }
   inline float speed() const                    { return _speed; }
   inline const Vec3f& direction() const         { return _direction; }
   inline const Quatf& orientation() const       { return _orientation; }
   inline SkeletalPose* pose() const             { return _pose.ptr(); }
   inline AnimationParameter* parameters() const { return _params; }

   inline Vec3f velocity() const                 { return _direction*_speed; }

   // Attributes setters.
   void canExit( bool v )                        { _canExit = v; }
   void speed( float v )                         { _speed = v; }
   void direction( const Vec3f& v )              { _direction = v; }
   void orientation( const Quatf& v )            { _orientation = v; }
   void pose( SkeletalPose* v )                  { _pose = v; }
   void parameters( AnimationParameter* v )      { _params = v; }

protected:

   /*----- methods -----*/

   virtual ~AnimationState();

   /*----- members -----*/

   bool                _canExit;
   float               _speed;
   Vec3f               _direction;
   Quatf               _orientation;
   RCP<SkeletalPose>   _pose;
   AnimationParameter* _params;
};

NAMESPACE_END

#endif
