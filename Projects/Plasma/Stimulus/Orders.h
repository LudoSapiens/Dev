/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_ORDERS_H
#define PLASMA_ORDERS_H

#include <Plasma/StdDefs.h>
#include <Plasma/Stimulus/Stimulus.h>
#include <Plasma/World/Entity.h>

#include <Fusion/VM/VM.h>

#include <CGMath/Variant.h>

NAMESPACE_BEGIN

namespace Orders
{

   PLASMA_DLL_API void initialize();
   PLASMA_DLL_API void terminate();

}

/*==============================================================================
  CLASS AimOrder
==============================================================================*/
class AimOrder:
   public Stimulus
{
public:

   /*----- methods -----*/

   AimOrder() {}
   AimOrder( const Vec3f& dir ): _direction(dir) {}

   PLASMA_DLL_API virtual const ConstString&  type() const;

   PLASMA_DLL_API virtual void push( VMState* vm );
   PLASMA_DLL_API virtual bool to( VMState* vm, int idx );

   // Attributes.
   inline void  direction( const Vec3f& d ) { _direction = d; }
   inline const Vec3f&  direction() const { return _direction; }

protected:

   /*----- data members -----*/

   Vec3f  _direction;  //!< The direction towards which to aim (use a Vec4f to allow both points and vectors?).

};


/*==============================================================================
  CLASS DeselectOrder
==============================================================================*/
class DeselectOrder:
   public Stimulus
{
public:

   PLASMA_DLL_API virtual const ConstString&  type() const;

};


/*==============================================================================
  CLASS DollyOrder
==============================================================================*/
class DollyOrder:
   public Stimulus
{
public:

   DollyOrder() {}
   DollyOrder( float d ): _distance( d ) {}

   PLASMA_DLL_API virtual const ConstString&  type() const;

   PLASMA_DLL_API virtual void push( VMState* vm );
   PLASMA_DLL_API virtual bool to( VMState* vm, int idx );

   inline float  distance() const { return _distance; }

protected:

   /*----- data members -----*/

   float  _distance;
};


/*==============================================================================
  CLASS DollyByRatioOrder
==============================================================================*/
class DollyByRatioOrder:
   public Stimulus
{
public:

   DollyByRatioOrder() {}
   DollyByRatioOrder( float v ): _ratio( v ) {}

   PLASMA_DLL_API virtual const ConstString&  type() const;

   PLASMA_DLL_API virtual void push( VMState* vm );
   PLASMA_DLL_API virtual bool to( VMState* vm, int idx );

   inline float  ratio() const { return _ratio; }

protected:

   /*----- data members -----*/

   float  _ratio;
};


/*==============================================================================
  CLASS FollowOrder
==============================================================================*/
class FollowOrder:
   public Stimulus
{
public:

   /*----- methods -----*/

   FollowOrder() {}
   FollowOrder( const ConstString& id ): _followID( id ) {}

   PLASMA_DLL_API virtual const ConstString&  type() const;

   PLASMA_DLL_API virtual void push( VMState* vm );
   PLASMA_DLL_API virtual bool to( VMState* vm, int idx );

   inline const ConstString&  followID() const { return _followID; }

protected:


   /*----- data members -----*/

   ConstString _followID;
};


/*==============================================================================
  CLASS gotoOrder
==============================================================================*/
class GoToOrder:
   public Stimulus
{
public:

   /*----- methods -----*/

   GoToOrder() {}
   GoToOrder( const Vec3f& pos ): _position( pos ) {}

   PLASMA_DLL_API virtual const ConstString&  type() const;

   PLASMA_DLL_API virtual void push( VMState* vm );
   PLASMA_DLL_API virtual bool to( VMState* vm, int idx );

   inline const Vec3f&  position() const { return _position; }
   inline void  position( const Vec3f& v ) { _position = v; }

protected:

   /*----- data members -----*/

   Vec3f  _position;
};


/*==============================================================================
  CLASS GrabOrder
==============================================================================*/
class GrabOrder:
   public Stimulus
{
public:

   /*----- methods -----*/

   GrabOrder() {}
   GrabOrder( Entity* e ): _entity(e) {}

   PLASMA_DLL_API virtual const ConstString&  type() const;

   PLASMA_DLL_API virtual void push( VMState* vm );
   PLASMA_DLL_API virtual bool to( VMState* vm, int idx );


protected:

   /*----- data members -----*/

   RCP<Entity>  _entity;  //!< The entity to grab.

};


/*==============================================================================
  CLASS JumpOrder
==============================================================================*/
class JumpOrder:
   public Stimulus
{
public:

   /*----- methods -----*/

   JumpOrder() {}
   JumpOrder( const Vec3f& d, float f ): _direction(d), _force(f) {}

   PLASMA_DLL_API virtual const ConstString&  type() const;

   PLASMA_DLL_API virtual void push( VMState* vm );
   PLASMA_DLL_API virtual bool to( VMState* vm, int idx );

   // Attributes.
   inline void direction( const Vec3f& dir ) { _direction = dir; }
   inline const Vec3f& direction() const { return _direction; }

   inline void force( float f ) { _force = f; }
   inline float force() const { return _force; }

protected:

   /*----- data members -----*/

   Vec3f _direction;
   float _force;
};


/*==============================================================================
  CLASS LookAtOrder
==============================================================================*/
class LookAtOrder:
   public Stimulus
{
public:

   /*----- methods -----*/

   LookAtOrder() {}
   LookAtOrder(
      const Vec3f& pos,
      const Vec3f& at,
      const Vec3f& up,
      const float  dur
   ): _position( pos ), _at( at ), _up( up ), _duration( dur ) {}

   PLASMA_DLL_API virtual const ConstString&  type() const;

   PLASMA_DLL_API virtual void push( VMState* vm );
   PLASMA_DLL_API virtual bool to( VMState* vm, int idx );

   // Attributes.
   inline void position( const Vec3f& v ) { _position = v; }
   inline const Vec3f& position() const { return _position; }

   inline void at( const Vec3f& v ) { _at = v; }
   inline const Vec3f& at() const { return _at; }

   inline void up( const Vec3f& v ) { _up = v; }
   inline const Vec3f& up() const { return _up; }

   inline void duration( float duration ) { _duration = duration; }
   inline float duration() const { return _duration; }

protected:

   /*----- data members -----*/

   Vec3f  _position;
   Vec3f  _at;
   Vec3f  _up;
   float  _duration;
};


/*==============================================================================
  CLASS MoveOrder
==============================================================================*/
class MoveOrder:
   public Stimulus
{
public:

   /*----- methods -----*/

   MoveOrder() {}
   MoveOrder( const Vec3f& dir, float speed, bool rel = false ):
      _dir(dir), _speed( speed ), _relative(rel) {}

   PLASMA_DLL_API virtual const ConstString&  type() const;

   PLASMA_DLL_API virtual void push( VMState* vm );
   PLASMA_DLL_API virtual bool to( VMState* vm, int idx );

protected:

   /*----- data members -----*/

   Vec3f  _dir;
   float  _speed;
   bool   _relative;
};


/*==============================================================================
  CLASS MoveObjectOrder
==============================================================================*/
class MoveObjectOrder:
   public Stimulus
{
public:

   /*----- methods -----*/

   MoveObjectOrder() {}
   MoveObjectOrder( Entity* e, const Vec3f& pos ): _entity(e), _position(pos) {}

   PLASMA_DLL_API virtual const ConstString&  type() const;

   PLASMA_DLL_API virtual void push( VMState* vm );
   PLASMA_DLL_API virtual bool to( VMState* vm, int idx );

   inline const RCP<Entity>&  entity() const { return _entity; }
   inline void  entity( Entity* e ) { _entity = e; }

   inline const Vec3f&  position() const { return _position; }
   inline void  position( const Vec3f& v ) { _position = v; }

protected:

   /*----- data members -----*/

   RCP<Entity>  _entity;    //!< The entity to move.
   Vec3f        _position;  //!< The position where to move the entity to.
};


/*==============================================================================
  CLASS OrbitOrder
==============================================================================*/
class OrbitOrder:
   public Stimulus
{
public:

   /*----- methods -----*/

   OrbitOrder() {}
   OrbitOrder( const Vec2f& angVel ): _angVel(angVel) {}

   PLASMA_DLL_API virtual const ConstString&  type() const;

   PLASMA_DLL_API virtual void push( VMState* vm );
   PLASMA_DLL_API virtual bool to( VMState* vm, int idx );


   inline void  angularVelocity( const Vec2f& v ) { _angVel = v; }
   inline const Vec2f&  angularVelocity() const { return _angVel; }

protected:

   Vec2f  _angVel;  //!< The angles to reach (typically theta-phi).
};


/*==============================================================================
  CLASS OrbitToOrder
==============================================================================*/
class OrbitToOrder:
   public Stimulus
{
public:

   /*----- methods -----*/

   OrbitToOrder() {}
   OrbitToOrder( const Vec2f& angles, bool rel = false ): _angles(angles), _relative(rel) {}

   PLASMA_DLL_API virtual const ConstString&  type() const;

   PLASMA_DLL_API virtual void push( VMState* vm );
   PLASMA_DLL_API virtual bool to( VMState* vm, int idx );

   inline void  angles( const Vec2f& v ) { _angles = v; }
   inline const Vec2f&  angles() const { return _angles; }

   inline void  relative( bool v ) { _relative = v; }
   inline bool  relative() const { return _relative; }

protected:

   Vec2f  _angles;   //!< The angles to reach (typically theta-phi).
   bool   _relative; //!< A flag indicating if the angles are relative to the current state or absolute.
};


/*==============================================================================
  CLASS PanOrder
==============================================================================*/
class PanOrder:
   public Stimulus
{
public:

   /*----- methods -----*/

   PanOrder() {}
   PanOrder( const Vec2f& dist, float dur ): _distance(dist), _duration(dur) {}

   PLASMA_DLL_API virtual const ConstString&  type() const;

   PLASMA_DLL_API virtual void push( VMState* vm );
   PLASMA_DLL_API virtual bool to( VMState* vm, int idx );

   // Attributes.
   inline void distance( const Vec2f& dist ) { _distance = dist; }
   inline const Vec2f& distance() const { return _distance; }

   inline void duration( float duration ) { _duration = duration; }
   inline float duration() const { return _duration; }

protected:

   /*----- data members -----*/

   Vec2f  _distance;
   float  _duration;
};


/*==============================================================================
  CLASS ReleaseOrder
==============================================================================*/
class ReleaseOrder:
   public Stimulus
{
public:

   /*----- methods -----*/

   ReleaseOrder() {}
   ReleaseOrder( Entity* e ): _entity(e) {}

   PLASMA_DLL_API virtual const ConstString&  type() const;

   PLASMA_DLL_API virtual void push( VMState* vm );
   PLASMA_DLL_API virtual bool to( VMState* vm, int idx );

   // Attributes.
   inline const RCP<Entity>&  entity() const { return _entity; }
   inline void  entity( Entity* e ) { _entity = e; }

protected:

   /*----- data members -----*/

   RCP<Entity>  _entity;  //!< The entity to release.

};


/*==============================================================================
  CLASS RollOrder
==============================================================================*/
class RollOrder:
   public Stimulus
{
public:

   /*----- methods -----*/

   RollOrder() {}
   RollOrder( float angle, float duration ): _angle(angle), _duration(duration) {}

   PLASMA_DLL_API virtual const ConstString&  type() const;

   PLASMA_DLL_API virtual void push( VMState* vm );
   PLASMA_DLL_API virtual bool to( VMState* vm, int idx );

   // Attributes.
   inline void  angle( const float angle ) { _angle = angle; }
   inline float angle() const              { return _angle; }

   inline void  duration( float duration ) { _duration = duration; }
   inline float duration() const           { return _duration; }

protected:

   /*----- data members -----*/

   float  _angle;
   float  _duration;
};


/*==============================================================================
  CLASS RotateOrder
==============================================================================*/
class RotateOrder:
   public Stimulus
{
public:

   /*----- methods -----*/

   RotateOrder() {}
   RotateOrder( const Vec2f& angVel ): _angularVelocity(angVel) {}

   PLASMA_DLL_API virtual const ConstString&  type() const;

   PLASMA_DLL_API virtual void push( VMState* vm );
   PLASMA_DLL_API virtual bool to( VMState* vm, int idx );

   // Attributes.
   inline void angularVelocity( const Vec2f& v ) { _angularVelocity = v; }
   inline const Vec2f& angularVelocity() const { return _angularVelocity; }

protected:

   /*----- data members -----*/

   Vec2f  _angularVelocity;
};


/*==============================================================================
  CLASS SelectOrder
==============================================================================*/
class SelectOrder:
   public Stimulus
{
public:

   PLASMA_DLL_API virtual const ConstString&  type() const;

};


/*==============================================================================
  CLASS SetOrder
==============================================================================*/
class SetOrder:
   public Stimulus
{
public:

   /*----- methods -----*/

   SetOrder() {}
   SetOrder( const String& key, const Variant& val ): _key(key), _val(val) {}

   PLASMA_DLL_API virtual const ConstString&  type() const;

   PLASMA_DLL_API virtual void push( VMState* vm );
   PLASMA_DLL_API virtual bool to( VMState* vm, int idx );

   // Attributes.
   inline void  key( const String& v ) { _key = v; }
   inline const String&  key() const { return _key; }

   inline void  value( const Variant& v ) { _val = v; }
   inline const Variant&  value() const { return _val; }

protected:

   /*----- data members -----*/

   String   _key;
   Variant  _val;

};


/*==============================================================================
  CLASS StopOrder
==============================================================================*/
class StopOrder:
   public Stimulus
{
public:

   PLASMA_DLL_API virtual const ConstString&  type() const;

};


/*==============================================================================
  CLASS ThrowOrder
==============================================================================*/
class ThrowOrder:
   public Stimulus
{
public:

   /*----- methods -----*/

   ThrowOrder() {}
   ThrowOrder( Entity* e, const Vec3f& v ): _entity(e), _velocity(v) {}

   PLASMA_DLL_API virtual const ConstString&  type() const;

   PLASMA_DLL_API virtual void push( VMState* vm );
   PLASMA_DLL_API virtual bool to( VMState* vm, int idx );

   inline const RCP<Entity>&  entity() const { return _entity; }
   inline void  entity( Entity* e ) { _entity = e; }

   inline const Vec3f&  velocity() const { return _velocity; }
   inline void  velocity( const Vec3f& v ) { _velocity = v; }

protected:

   /*----- data members -----*/

   RCP<Entity>  _entity;    //!< The entity to throw.
   Vec3f        _velocity;  //!< The velocity to throw at.
};


/*==============================================================================
  CLASS TiltOrder
==============================================================================*/
class TiltOrder:
   public Stimulus
{
public:

   /*----- methods -----*/

   TiltOrder() {}
   TiltOrder( const Vec2f& angles, float duration ): _angles(angles), _duration(duration) {}

   PLASMA_DLL_API virtual const ConstString&  type() const;

   PLASMA_DLL_API virtual void push( VMState* vm );
   PLASMA_DLL_API virtual bool to( VMState* vm, int idx );

   // Attributes.
   inline void angles( const Vec2f& ang ) { _angles = ang; }
   inline const Vec2f& angles() const { return _angles; }

   inline void duration( float duration ) { _duration = duration; }
   inline float duration() const { return _duration; }

protected:

   /*----- data members -----*/

   Vec2f  _angles;
   float  _duration;
};


/*==============================================================================
  CLASS TurnOrder
==============================================================================*/
class TurnOrder:
   public Stimulus
{
public:

   /*----- methods -----*/

   TurnOrder() {}
   TurnOrder( const Vec2f& angVel ): _angularVelocity(angVel) {}

   PLASMA_DLL_API virtual const ConstString&  type() const;

   PLASMA_DLL_API virtual void push( VMState* vm );
   PLASMA_DLL_API virtual bool to( VMState* vm, int idx );

   // Attributes.
   inline void angularVelocity( const Vec2f& v ) { _angularVelocity = v; }
   inline const Vec2f& angularVelocity() const { return _angularVelocity; }

protected:

   /*----- data members -----*/

   Vec2f  _angularVelocity;
};


/*==============================================================================
  CLASS ZoomOrder
==============================================================================*/
class ZoomOrder:
   public Stimulus
{
public:

   /*----- methods -----*/

   ZoomOrder() {}
   ZoomOrder( float fovy, float duration ): _fovy(fovy), _duration(duration) {}

   PLASMA_DLL_API virtual const ConstString&  type() const;

   PLASMA_DLL_API virtual void push( VMState* vm );
   PLASMA_DLL_API virtual bool to( VMState* vm, int idx );

   // Attributes.
   inline void fovy( float fovy ) { _fovy = fovy; }
   inline float fovy() const { return _fovy; }

   inline void duration( float duration ) { _duration = duration; }
   inline float duration() const { return _duration; }

protected:

   /*----- data members -----*/

   float _fovy;
   float _duration;
};

/*==============================================================================
  CLASS FlowOrder
==============================================================================*/
class FlowOrder:
   public Stimulus
{
public:

   FlowOrder() {}
   FlowOrder( const Vec3f& dir): _dir( dir ) {}

   PLASMA_DLL_API virtual const ConstString&  type() const;

   PLASMA_DLL_API virtual void push( VMState* vm );
   PLASMA_DLL_API virtual bool to( VMState* vm, int idx );

   // Attributes.
   inline void direction( const Vec3f& v ) { _dir = v; }
   inline const Vec3f& direction() const { return _dir; }

protected:

   /*----- data members -----*/

   Vec3f  _dir;
};

/*==============================================================================
  CLASS FlowStopOrder
==============================================================================*/
class FlowStopOrder:
   public Stimulus
{
public:
   PLASMA_DLL_API virtual const ConstString&  type() const;
};

/*==============================================================================
  CLASS LowerShieldOrder
==============================================================================*/
class LowerShieldOrder:
   public Stimulus
{
public:
   PLASMA_DLL_API virtual const ConstString&  type() const;
};

/*==============================================================================
  CLASS RaiseShieldOrder
==============================================================================*/
class RaiseShieldOrder:
   public Stimulus
{
public:
   PLASMA_DLL_API virtual const ConstString&  type() const;
};

/*==============================================================================
  CLASS ToggleShieldOrder
==============================================================================*/
class ToggleShieldOrder:
   public Stimulus
{
public:
   PLASMA_DLL_API virtual const ConstString&  type() const;
};

/*==============================================================================
  CLASS ShockwaveOrder
==============================================================================*/
class ShockwaveOrder:
   public Stimulus
{
public:

   /*----- methods -----*/

   ShockwaveOrder() {}
   ShockwaveOrder( float speed, float force ): _speed( speed ), _force( force ) {}

   PLASMA_DLL_API virtual const ConstString&  type() const;

   PLASMA_DLL_API virtual void push( VMState* vm );
   PLASMA_DLL_API virtual bool to( VMState* vm, int idx );

protected:

   /*----- data members -----*/

   float _speed;
   float _force;
};



NAMESPACE_END

#endif //PLASMA_ORDERS_H
