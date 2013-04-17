/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Stimulus/Orders.h>

#include <Fusion/VM/VMRegistry.h>

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

ConstString  _sType_Aim;
ConstString  _sType_Deselect;
ConstString  _sType_Dolly;
ConstString  _sType_DollyByRatio;
ConstString  _sType_Follow;
ConstString  _sType_Flow;
ConstString  _sType_FlowStop;
ConstString  _sType_GoTo;
ConstString  _sType_Grab;
ConstString  _sType_Jump;
ConstString  _sType_LookAt;
ConstString  _sType_LowerShield;
ConstString  _sType_Move;
ConstString  _sType_MoveObject;
ConstString  _sType_Orbit;
ConstString  _sType_OrbitTo;
ConstString  _sType_Pan;
ConstString  _sType_RaiseShield;
ConstString  _sType_Release;
ConstString  _sType_Roll;
ConstString  _sType_Rotate;
ConstString  _sType_Select;
ConstString  _sType_Shockwave;
ConstString  _sType_Set;
ConstString  _sType_Stop;
ConstString  _sType_Throw;
ConstString  _sType_Tilt;
ConstString  _sType_ToggleShield;
ConstString  _sType_Turn;
ConstString  _sType_Zoom;

//------------------------------------------------------------------------------
//!
bool  VM__get( VMState* vm, int idx, const char* key, RCP<Entity>& entity )
{
   if( VM::get( vm, idx, key ) )
   {
      entity = (Entity*)VM::toProxy( vm, idx );
      return true;
   }
   return false;
}

UNNAMESPACE_END


NAMESPACE_BEGIN

/*==============================================================================
   NAMESPACE Orders
==============================================================================*/

namespace Orders
{

//------------------------------------------------------------------------------
//!
void initialize()
{
#define REGISTER( sTypeVar, str, type ) \
   sTypeVar = str; \
   Stimulus::registerStimulus( sTypeVar, create<Stimulus,type> )

   REGISTER( _sType_Aim         , "aim"         , AimOrder          );
   REGISTER( _sType_Deselect    , "deselect"    , DeselectOrder     );
   REGISTER( _sType_Dolly       , "dolly"       , DollyOrder        );
   REGISTER( _sType_DollyByRatio, "dollyByRatio", DollyByRatioOrder );
   REGISTER( _sType_Follow      , "follow"      , FollowOrder       );
   REGISTER( _sType_Flow        , "flow"        , FlowOrder         );
   REGISTER( _sType_FlowStop    , "flowStop"    , FlowStopOrder     );
   REGISTER( _sType_GoTo        , "goTo"        , GoToOrder         );
   REGISTER( _sType_Grab        , "grab"        , GrabOrder         );
   REGISTER( _sType_Jump        , "jump"        , JumpOrder         );
   REGISTER( _sType_LookAt      , "lookAt"      , LookAtOrder       );
   REGISTER( _sType_LowerShield , "lowerShield" , LowerShieldOrder  );
   REGISTER( _sType_Move        , "move"        , MoveOrder         );
   REGISTER( _sType_MoveObject  , "moveObject"  , MoveObjectOrder   );
   REGISTER( _sType_Orbit       , "orbit"       , OrbitOrder        );
   REGISTER( _sType_OrbitTo     , "orbitTo"     , OrbitToOrder      );
   REGISTER( _sType_Pan         , "pan"         , PanOrder          );
   REGISTER( _sType_RaiseShield , "raiseShield" , RaiseShieldOrder  );
   REGISTER( _sType_Release     , "release"     , ReleaseOrder      );
   REGISTER( _sType_Roll        , "roll"        , RollOrder         );
   REGISTER( _sType_Rotate      , "rotate"      , RotateOrder       );
   REGISTER( _sType_Select      , "select"      , SelectOrder       );
   REGISTER( _sType_Set         , "set"         , SetOrder          );
   REGISTER( _sType_Shockwave   , "shockwave"   , ShockwaveOrder    );
   REGISTER( _sType_Stop        , "stop"        , StopOrder         );
   REGISTER( _sType_Throw       , "throw"       , ThrowOrder        );
   REGISTER( _sType_Tilt        , "tilt"        , TiltOrder         );
   REGISTER( _sType_ToggleShield, "toggleShield", ToggleShieldOrder );
   REGISTER( _sType_Turn        , "turn"        , TurnOrder         );
   REGISTER( _sType_Zoom        , "zoom"        , ZoomOrder         );

#undef REGISTER
}

//------------------------------------------------------------------------------
//!
void terminate()
{
#define UNREGISTER( sTypeVar ) \
   Stimulus::unregisterStimulus( sTypeVar ); \
   sTypeVar = ConstString()

   UNREGISTER( _sType_Aim          );
   UNREGISTER( _sType_Deselect     );
   UNREGISTER( _sType_Dolly        );
   UNREGISTER( _sType_DollyByRatio );
   UNREGISTER( _sType_Follow       );
   UNREGISTER( _sType_Flow         );
   UNREGISTER( _sType_FlowStop     );
   UNREGISTER( _sType_GoTo         );
   UNREGISTER( _sType_Grab         );
   UNREGISTER( _sType_Jump         );
   UNREGISTER( _sType_LookAt       );
   UNREGISTER( _sType_LowerShield  );
   UNREGISTER( _sType_Move         );
   UNREGISTER( _sType_MoveObject   );
   UNREGISTER( _sType_Orbit        );
   UNREGISTER( _sType_OrbitTo      );
   UNREGISTER( _sType_Pan          );
   UNREGISTER( _sType_RaiseShield  );
   UNREGISTER( _sType_Release      );
   UNREGISTER( _sType_Roll         );
   UNREGISTER( _sType_Rotate       );
   UNREGISTER( _sType_Select       );
   UNREGISTER( _sType_Set          );
   UNREGISTER( _sType_Shockwave    );
   UNREGISTER( _sType_Stop         );
   UNREGISTER( _sType_Throw        );
   UNREGISTER( _sType_Tilt         );
   UNREGISTER( _sType_ToggleShield );
   UNREGISTER( _sType_Turn         );
   UNREGISTER( _sType_Zoom         );

#undef UNREGISTER
}

}


/*==============================================================================
  CLASS AimOrder
==============================================================================*/

//------------------------------------------------------------------------------
//!
const ConstString&
AimOrder::type() const
{
   return _sType_Aim;
}

//------------------------------------------------------------------------------
//!
void
AimOrder::push( VMState* vm )
{
   Stimulus::push( vm );

   VM::push( vm, "direction" );
   VM::push( vm, _direction );
   VM::set( vm, -3 );
}

//------------------------------------------------------------------------------
//!
bool
AimOrder::to( VMState* vm, int idx )
{
   return VM::get( vm, idx, "direction", _direction );
}


/*==============================================================================
  CLASS DeselectOrder
==============================================================================*/

//------------------------------------------------------------------------------
//!
const ConstString&
DeselectOrder::type() const
{
   return _sType_Deselect;
}


/*==============================================================================
  CLASS DollyOrder
==============================================================================*/

//------------------------------------------------------------------------------
//!
const ConstString&
DollyOrder::type() const
{
   return _sType_Dolly;
}

//------------------------------------------------------------------------------
//!
void
DollyOrder::push( VMState* vm )
{
   Stimulus::push( vm );

   VM::push( vm, "distance" );
   VM::push( vm, _distance );
   VM::set( vm, -3 );
}

//------------------------------------------------------------------------------
//!
bool
DollyOrder::to( VMState* vm, int idx )
{
   return VM::get( vm, idx, "distance", _distance );
}


/*==============================================================================
  CLASS DollyByRatioOrder
==============================================================================*/

//------------------------------------------------------------------------------
//!
const ConstString&
DollyByRatioOrder::type() const
{
   return _sType_DollyByRatio;
}

//------------------------------------------------------------------------------
//!
void
DollyByRatioOrder::push( VMState* vm )
{
   Stimulus::push( vm );

   VM::push( vm, "ratio" );
   VM::push( vm, _ratio );
   VM::set( vm, -3 );
}

//------------------------------------------------------------------------------
//!
bool
DollyByRatioOrder::to( VMState* vm, int idx )
{
   return VM::get( vm, idx, "ratio", _ratio );
}


/*==============================================================================
  CLASS FollowOrder
==============================================================================*/

//------------------------------------------------------------------------------
//!
const ConstString&
FollowOrder::type() const
{
   return _sType_Follow;
}

//------------------------------------------------------------------------------
//!
void
FollowOrder::push( VMState* vm )
{
   Stimulus::push( vm );

   VM::push( vm, "followID" );
   VM::push( vm, _followID );
   VM::set( vm, -3 );
}

//------------------------------------------------------------------------------
//!
bool
FollowOrder::to( VMState* vm, int idx )
{
   return VM::get( vm, idx, "followID", _followID );
}


/*==============================================================================
  CLASS GoToOrder
==============================================================================*/

//------------------------------------------------------------------------------
//!
const ConstString&
GoToOrder::type() const
{
   return _sType_GoTo;
}

//------------------------------------------------------------------------------
//!
void
GoToOrder::push( VMState* vm )
{
   Stimulus::push( vm );

   VM::push( vm, "position" );
   VM::push( vm, _position );
   VM::set( vm, -3 );
}

//------------------------------------------------------------------------------
//!
bool
GoToOrder::to( VMState* vm, int idx )
{
   return VM::get( vm, idx, "position", _position );
}

/*==============================================================================
  CLASS GrabOrder
==============================================================================*/

//------------------------------------------------------------------------------
//!
const ConstString&
GrabOrder::type() const
{
   return _sType_Grab;
}

//------------------------------------------------------------------------------
//!
void
GrabOrder::push( VMState* vm )
{
   Stimulus::push( vm );

   VM::push( vm, "entity" );
   VM::pushProxy( vm, _entity.ptr() );
   VM::set( vm, -3 );
}

//------------------------------------------------------------------------------
//!
bool
GrabOrder::to( VMState* vm, int idx )
{
   return VM__get( vm, idx, "entity", _entity );
}


/*==============================================================================
  CLASS JumpOrder
==============================================================================*/

//------------------------------------------------------------------------------
//!
const ConstString&
JumpOrder::type() const
{
   return _sType_Jump;
}

//------------------------------------------------------------------------------
//!
void
JumpOrder::push( VMState* vm )
{
   Stimulus::push( vm );

   VM::push( vm, "direction" );
   VM::push( vm, _direction );
   VM::set( vm, -3 );

   VM::push( vm, "force" );
   VM::push( vm, _force );
   VM::set( vm, -3 );
}

//------------------------------------------------------------------------------
//!
bool
JumpOrder::to( VMState* vm, int idx )
{
   bool ok;
   ok  = VM::get( vm, idx, "direction", _direction );
   ok |= VM::get( vm, idx, "force", _force );
   return ok;
}


/*==============================================================================
  CLASS LookAtOrder
==============================================================================*/

//------------------------------------------------------------------------------
//!
const ConstString&
LookAtOrder::type() const
{
   return _sType_LookAt;
}

//------------------------------------------------------------------------------
//!
void
LookAtOrder::push( VMState* vm )
{
   Stimulus::push( vm );

   VM::push( vm, "position" );
   VM::push( vm, _position );
   VM::set( vm, -3 );

   VM::push( vm, "at" );
   VM::push( vm, _at );
   VM::set( vm, -3 );

   VM::push( vm, "up" );
   VM::push( vm, _up );
   VM::set( vm, -3 );

   VM::push( vm, "duration" );
   VM::push( vm, _duration );
   VM::set( vm, -3 );
}

//------------------------------------------------------------------------------
//!
bool
LookAtOrder::to( VMState* vm, int idx )
{
   bool ok;
   ok  = VM::get( vm, idx, "position", _position );
   ok |= VM::get( vm, idx, "at", _at );
   ok |= VM::get( vm, idx, "up", _up );
   ok |= VM::get( vm, idx, "duration", _duration );
   return ok;
}


/*==============================================================================
  CLASS MoveOrder
==============================================================================*/

//------------------------------------------------------------------------------
//!
const ConstString&
MoveOrder::type() const
{
   return _sType_Move;
}

//------------------------------------------------------------------------------
//!
void
MoveOrder::push( VMState* vm )
{
   Stimulus::push( vm );

   VM::push( vm, "direction" );
   VM::push( vm, _dir );
   VM::set( vm, -3 );

   VM::push( vm, "speed" );
   VM::push( vm, _speed );
   VM::set( vm, -3 );

   if( _relative )
   {
      VM::push( vm, "relative" );
      VM::push( vm, _relative );
      VM::set( vm, -3 );
   }
}

//------------------------------------------------------------------------------
//!
bool
MoveOrder::to( VMState* vm, int idx )
{
   bool ok;
   ok  = VM::get( vm, idx, "direction", _dir );
   ok |= VM::get( vm, idx, "speed", _speed );
   ok |= VM::get( vm, idx, "relative", _relative );
   return ok;
}

/*==============================================================================
  CLASS MoveObjectOrder
==============================================================================*/

//------------------------------------------------------------------------------
//!
const ConstString&
MoveObjectOrder::type() const
{
   return _sType_MoveObject;
}

//------------------------------------------------------------------------------
//!
void
MoveObjectOrder::push( VMState* vm )
{
   Stimulus::push( vm );

   VM::push( vm, "entity" );
   VM::pushProxy( vm, _entity.ptr() );
   VM::set( vm, -3 );

   VM::push( vm, "position" );
   VM::push( vm, _position );
   VM::set( vm, -3 );
}

//------------------------------------------------------------------------------
//!
bool
MoveObjectOrder::to( VMState* vm, int idx )
{
   bool ok;
   ok  = VM__get( vm, idx, "entity", _entity );
   ok |= VM::get( vm, idx, "position", _position );
   return ok;
}


/*==============================================================================
  CLASS OrbitOrder
==============================================================================*/

//------------------------------------------------------------------------------
//!
const ConstString&
OrbitOrder::type() const
{
   return _sType_Orbit;
}

//------------------------------------------------------------------------------
//!
void
OrbitOrder::push( VMState* vm )
{
   Stimulus::push( vm );

   VM::push( vm, "angularVelocity" );
   VM::push( vm, _angVel );
   VM::set( vm, -3 );
}

//------------------------------------------------------------------------------
//!
bool
OrbitOrder::to( VMState* vm, int idx )
{
   return VM::get( vm, idx, "angularVelocity", _angVel );
}


/*==============================================================================
  CLASS OrbitOrder
==============================================================================*/

//------------------------------------------------------------------------------
//!
const ConstString&
OrbitToOrder::type() const
{
   return _sType_OrbitTo;
}

//------------------------------------------------------------------------------
//!
void
OrbitToOrder::push( VMState* vm )
{
   Stimulus::push( vm );

   VM::push( vm, "angles" );
   VM::push( vm, _angles );
   VM::set( vm, -3 );

   VM::push( vm, "relative" );
   VM::push( vm, _relative );
   VM::set( vm, -3 );
}

//------------------------------------------------------------------------------
//!
bool
OrbitToOrder::to( VMState* vm, int idx )
{
   bool ok;
   ok  = VM::get( vm, idx, "angles"  , _angles   );
   _relative = false;
   VM::get( vm, idx, "relative", _relative ); // Optional.
   return ok;
}


/*==============================================================================
  CLASS PanOrder
==============================================================================*/

//------------------------------------------------------------------------------
//!
const ConstString&
PanOrder::type() const
{
   return _sType_Pan;
}

//------------------------------------------------------------------------------
//!
void
PanOrder::push( VMState* vm )
{
   Stimulus::push( vm );

   VM::push( vm, "distance" );
   VM::push( vm, _distance );
   VM::set( vm, -3 );

   VM::push( vm, "duration" );
   VM::push( vm, _duration );
   VM::set( vm, -3 );
}

//------------------------------------------------------------------------------
//!
bool
PanOrder::to( VMState* vm, int idx )
{
   bool ok;
   ok  = VM::get( vm, idx, "distance", _distance );
   ok |= VM::get( vm, idx, "duration", _duration );
   return ok;
}


/*==============================================================================
  CLASS LowerShieldOrder
==============================================================================*/

//------------------------------------------------------------------------------
//!
const ConstString&
LowerShieldOrder::type() const
{
   return _sType_LowerShield;
}


/*==============================================================================
  CLASS RaiseShieldOrder
==============================================================================*/

//------------------------------------------------------------------------------
//!
const ConstString&
RaiseShieldOrder::type() const
{
   return _sType_RaiseShield;
}


/*==============================================================================
  CLASS ToggleShieldOrder
==============================================================================*/

//------------------------------------------------------------------------------
//!
const ConstString&
ToggleShieldOrder::type() const
{
   return _sType_ToggleShield;
}


/*==============================================================================
  CLASS ReleaseOrder
==============================================================================*/

//------------------------------------------------------------------------------
//!
const ConstString&
ReleaseOrder::type() const
{
   return _sType_Release;
}

//------------------------------------------------------------------------------
//!
void
ReleaseOrder::push( VMState* vm )
{
   Stimulus::push( vm );

   VM::push( vm, "entity" );
   VM::pushProxy( vm, _entity.ptr() );
   VM::set( vm, -3 );
}

//------------------------------------------------------------------------------
//!
bool
ReleaseOrder::to( VMState* vm, int idx )
{
   return VM__get( vm, idx, "entity", _entity );
}


/*==============================================================================
  CLASS RollOrder
==============================================================================*/

//------------------------------------------------------------------------------
//!
const ConstString&
RollOrder::type() const
{
   return _sType_Roll;
}

//------------------------------------------------------------------------------
//!
void
RollOrder::push( VMState* vm )
{
   Stimulus::push( vm );

   VM::push( vm, "angle" );
   VM::push( vm, _angle );
   VM::set( vm, -3 );

   VM::push( vm, "duration" );
   VM::push( vm, _duration );
   VM::set( vm, -3 );
}

//------------------------------------------------------------------------------
//!
bool
RollOrder::to( VMState* vm, int idx )
{
   bool ok;
   ok  = VM::get( vm, idx, "angle", _angle );
   ok |= VM::get( vm, idx, "duration", _duration );
   return ok;
}


/*==============================================================================
  CLASS RotateOrder
==============================================================================*/

//------------------------------------------------------------------------------
//!
const ConstString&
RotateOrder::type() const
{
   return _sType_Rotate;
}

//------------------------------------------------------------------------------
//!
void
RotateOrder::push( VMState* vm )
{
   Stimulus::push( vm );

   VM::push( vm, "angularVelocity" );
   VM::push( vm, _angularVelocity );
   VM::set( vm, -3 );
}

//------------------------------------------------------------------------------
//!
bool
RotateOrder::to( VMState* vm, int idx )
{
   return VM::get( vm, idx, "angularVelocity", _angularVelocity );
}


/*==============================================================================
  CLASS SelectOrder
==============================================================================*/

//------------------------------------------------------------------------------
//!
const ConstString&
SelectOrder::type() const
{
   return _sType_Select;
}


/*==============================================================================
  CLASS SetOrder
==============================================================================*/

//------------------------------------------------------------------------------
//!
const ConstString&
SetOrder::type() const
{
   return _sType_Set;
}

//------------------------------------------------------------------------------
//!
void
SetOrder::push( VMState* vm )
{
   // Overwrite type (normally set in Stimulus::push()) with the key,
   // and assign the value under the name 'value'.

   //Stimulus::push( vm );
   VM::newTable( vm );

   VM::push( vm, "type" );
   VM::push( vm, _key );
   VM::set( vm, -3 );

   VM::push( vm, "value" );
   VM::push( vm, _val );
   VM::set( vm, -3 );
}

//------------------------------------------------------------------------------
//!
bool
SetOrder::to( VMState* vm, int idx )
{
   bool ok = true;
   idx = VM::absIndex( vm, idx );
   VM::push( vm );
   while( VM::next( vm, idx ) )
   {
      _key = VM::toString( vm, -2 );
      _val = VM::toVariant( vm, -1 );
      VM::pop( vm, 1 );
   }
   return ok;
}


/*==============================================================================
  CLASS StopOrder
==============================================================================*/

//------------------------------------------------------------------------------
//!
const ConstString&
StopOrder::type() const
{
   return _sType_Stop;
}


/*==============================================================================
  CLASS ThrowOrder
==============================================================================*/

//------------------------------------------------------------------------------
//!
const ConstString&
ThrowOrder::type() const
{
   return _sType_Throw;
}

//------------------------------------------------------------------------------
//!
void
ThrowOrder::push( VMState* vm )
{
   Stimulus::push( vm );

   VM::push( vm, "entity" );
   VM::pushProxy( vm, _entity.ptr() );
   VM::set( vm, -3 );

   VM::push( vm, "velocity" );
   VM::push( vm, _velocity );
   VM::set( vm, -3 );
}

//------------------------------------------------------------------------------
//!
bool
ThrowOrder::to( VMState* vm, int idx )
{
   bool ok;
   ok  = VM__get( vm, idx, "entity", _entity );
   ok |= VM::get( vm, idx, "velocity", _velocity );
   return ok;
}


/*==============================================================================
  CLASS TiltOrder
==============================================================================*/

//------------------------------------------------------------------------------
//!
const ConstString&
TiltOrder::type() const
{
   return _sType_Tilt;
}

//------------------------------------------------------------------------------
//!
void
TiltOrder::push( VMState* vm )
{
   Stimulus::push( vm );

   VM::push( vm, "angles" );
   VM::push( vm, _angles );
   VM::set( vm, -3 );

   VM::push( vm, "duration" );
   VM::push( vm, _duration );
   VM::set( vm, -3 );
}

//------------------------------------------------------------------------------
//!
bool
TiltOrder::to( VMState* vm, int idx )
{
   bool ok;
   ok  = VM::get( vm, idx, "angles", _angles );
   ok |= VM::get( vm, idx, "duration", _duration );
   return ok;
}


/*==============================================================================
  CLASS TurnOrder
==============================================================================*/

//------------------------------------------------------------------------------
//!
const ConstString&
TurnOrder::type() const
{
   return _sType_Turn;
}

//------------------------------------------------------------------------------
//!
void
TurnOrder::push( VMState* vm )
{
   Stimulus::push( vm );

   VM::push( vm, "angularVelocity" );
   VM::push( vm, _angularVelocity );
   VM::set( vm, -3 );
}

//------------------------------------------------------------------------------
//!
bool
TurnOrder::to( VMState* vm, int idx )
{
   return VM::get( vm, idx, "angularVelocity", _angularVelocity );
}


/*==============================================================================
  CLASS ZoomOrder
==============================================================================*/

//------------------------------------------------------------------------------
//!
const ConstString&
ZoomOrder::type() const
{
   return _sType_Zoom;
}

//------------------------------------------------------------------------------
//!
void
ZoomOrder::push( VMState* vm )
{
   Stimulus::push( vm );

   VM::push( vm, "fovy" );
   VM::push( vm, _fovy );
   VM::set( vm, -3 );

   VM::push( vm, "duration" );
   VM::push( vm, _duration );
   VM::set( vm, -3 );
}

//------------------------------------------------------------------------------
//!
bool
ZoomOrder::to( VMState* vm, int idx )
{
   bool ok;
   ok  = VM::get( vm, idx, "fovy", _fovy );
   ok |= VM::get( vm, idx, "duration", _duration );
   return ok;
}

/*==============================================================================
  CLASS FlowOrder
==============================================================================*/

//------------------------------------------------------------------------------
//!
const ConstString&
FlowOrder::type() const
{
   return _sType_Flow;
}

//------------------------------------------------------------------------------
//!
void
FlowOrder::push( VMState* vm )
{
   Stimulus::push( vm );

   VM::push( vm, "direction" );
   VM::push( vm, _dir );
   VM::set( vm, -3 );
}

//------------------------------------------------------------------------------
//!
bool
FlowOrder::to( VMState* vm, int idx )
{
   return VM::get( vm, idx, "direction", _dir );
}

/*==============================================================================
  CLASS FlowStopOrder
==============================================================================*/

//------------------------------------------------------------------------------
//!
const ConstString&
FlowStopOrder::type() const
{
   return _sType_FlowStop;
}

/*==============================================================================
  CLASS ShockwaveOrder
==============================================================================*/

//------------------------------------------------------------------------------
//!
const ConstString&
ShockwaveOrder::type() const
{
   return _sType_Shockwave;
}

//------------------------------------------------------------------------------
//!
void
ShockwaveOrder::push( VMState* vm )
{
   Stimulus::push( vm );

   VM::push( vm, "speed" );
   VM::push( vm, _speed );
   VM::set( vm, -3 );

   VM::push( vm, "force" );
   VM::push( vm, _force );
   VM::set( vm, -3 );
}

//------------------------------------------------------------------------------
//!
bool
ShockwaveOrder::to( VMState* vm, int idx )
{
   bool ok;
   ok  = VM::get( vm, idx, "speed", _speed );
   ok |= VM::get( vm, idx, "force", _force );
   return ok;
}


NAMESPACE_END
