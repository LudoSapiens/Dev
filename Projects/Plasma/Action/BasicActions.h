/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_BASIC_ACTIONS_H
#define PLASMA_BASIC_ACTIONS_H

#include <Plasma/StdDefs.h>
#include <Plasma/Action/Action.h>

NAMESPACE_BEGIN

/*==============================================================================
   NAMESPACE BasicActions
==============================================================================*/

namespace BasicActions
{
   PLASMA_DLL_API void initialize();
   PLASMA_DLL_API void terminate();
}

/*==============================================================================
  CLASS MoveToAction
==============================================================================*/
class MoveToAction:
   public Action
{
public:

   /*----- static methods -----*/

   static Action* create();

   /*----- methods -----*/

   MoveToAction();

   PLASMA_DLL_API static  const ConstString&  actionType();
   PLASMA_DLL_API virtual const ConstString&  type() const;

   PLASMA_DLL_API virtual bool  execute( Entity& entity, double time, double delta );

   PLASMA_DLL_API virtual bool  performGet( VMState* vm );
   PLASMA_DLL_API virtual bool  performSet( VMState* vm );

protected:

   Vec3f    _position;
   float    _speed;

};

/*==============================================================================
  CLASS FollowEntity
==============================================================================*/
class FollowEntity:
   public Action
{
public:

   /*----- static methods -----*/

   static Action* create();

   /*----- methods -----*/

   FollowEntity();

   PLASMA_DLL_API virtual const ConstString&  type() const;

   PLASMA_DLL_API virtual bool  execute( Entity& entity, double time, double delta );

   PLASMA_DLL_API virtual bool  performGet( VMState* vm );
   PLASMA_DLL_API virtual bool  performSet( VMState* vm );

protected:

   /*----- structures -----*/

   struct State
   {
      Vec3f _followPos;
      Vec2f _angles;
      float _followDistance;
   };

   /*----- data members -----*/

   Reff computeReferential( const State& );
   void angles( const Vec2f& );
   void angularVelocity( const Vec2f& );
   void distance( float );
   void distanceVelocity( float );
   void followEntity( Entity* e, Entity* target );
   float checkOcclusion( const Vec3f& targetPos, const Vec3f& followerPos, float targetFollowDistance );

   RCP<Entity> _target;
   float       _maxLinearSpeed;
   float       _maxAngularSpeed;
   float       _maxDistanceSpeed;
   float       _height;

   Vec2f       _angularVelocity;
   float       _distanceVelocity;

   State       _currentState;
   State       _targetState;

   bool        _occluded;
};

/*==============================================================================
  CLASS Waypoint
==============================================================================*/
class Waypoint:
   public Action
{
public:

   /*----- static methods -----*/

   static Action*  create();

   /*----- methods -----*/

   Waypoint();

   inline const Vec3f&  location() const   { return _location; }
   inline void  location( const Vec3f& v ) { _location = v;    }

   inline float  distance() const    { return _distance; }
   inline void   distance( float v ) { _distance = v;    }

   PLASMA_DLL_API virtual const ConstString&  type() const;

   PLASMA_DLL_API virtual bool  execute( Entity& entity, double time, double delta );

   PLASMA_DLL_API virtual bool  performGet( VMState* vm );
   PLASMA_DLL_API virtual bool  performSet( VMState* vm );

protected:

   /*----- data members -----*/

   Vec3f  _location;
   //Vec3f  _mask;
   float  _distance;

}; // Waypoint


NAMESPACE_END

#endif
