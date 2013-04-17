/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Action/BasicActions.h>
#include <Plasma/World/World.h>
#include <Plasma/World/Entity.h>
#include <Plasma/World/RigidEntity.h>

#include <Fusion/VM/VMMath.h>

#include <Base/ADT/StringMap.h>

/*==============================================================================
   UNNAMED NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
ConstString _typeFollowEntity_str;
ConstString _typeWaypoint_str;
ConstString _typeMoveTo_str;

enum
{
   ATTRIB_ANGLES,
   ATTRIB_ANGULAR_VELOCITY,
   ATTRIB_DISTANCE,
   ATTRIB_DISTANCE_VELOCITY,
   ATTRIB_FOLLOW_ID,
   ATTRIB_HEIGHT,
   ATTRIB_LOCATION,
   ATTRIB_MAX_ANGULAR_SPEED,
   ATTRIB_MAX_LINEAR_SPEED,
};


//------------------------------------------------------------------------------
//!
enum
{
   MOVETO_POSITION,
   MOVETO_SPEED,
};

//------------------------------------------------------------------------------
//!
StringMap _movetoAttr(
   "position",  MOVETO_POSITION,
   "speed"   ,  MOVETO_SPEED,
   ""
);

StringMap _followAttributes(
   "angles",           ATTRIB_ANGLES,
   "angularVelocity",  ATTRIB_ANGULAR_VELOCITY,
   "distance",         ATTRIB_DISTANCE,
   "distanceVelocity", ATTRIB_DISTANCE_VELOCITY,
   "height",           ATTRIB_HEIGHT,
   "id",               ATTRIB_FOLLOW_ID,
   "maxAngularSpeed",  ATTRIB_MAX_ANGULAR_SPEED,
   "maxLinearSpeed",   ATTRIB_MAX_LINEAR_SPEED,
   ""
);

StringMap _waypointAttributes(
   "distance",  ATTRIB_DISTANCE,
   "location",  ATTRIB_LOCATION,
   ""
);

//------------------------------------------------------------------------------
//!
float clampMove( float v0, float v1, float max )
{
   float d = v1-v0;
   if( CGM::abs(d) <= max ) return v1;
   return v0 + CGM::copySign( max, d );
}

//------------------------------------------------------------------------------
//!
Vec2f clampMove( const Vec2f& v0, const Vec2f& v1, float max )
{
   Vec2f d    = v1-v0;
   float dSqr = sqrLength(d);
   if( dSqr <= max*max ) return v1;
   return v0 + d*(max/CGM::sqrt(dSqr));
}

//------------------------------------------------------------------------------
//!
Vec3f clampMove( const Vec3f& v0, const Vec3f& v1, float max )
{
   Vec3f d    = v1-v0;
   float dSqr = sqrLength(d);
   if( dSqr <= max*max ) return v1;
   return v0 + d*(max/CGM::sqrt(dSqr));
}

#if 0
//------------------------------------------------------------------------------
//!
void clampMove(
   float& d0, float d1, float maxD,
   Vec2f& a0, const Vec2f& a1, float maxA
)
{
   float dd = d1-d0;
   Vec2f da = a1-a0;

   Vec3f dn( da/maxA, dd/maxD );
   float dSqr = sqrLength(dn);
   if( dSqr <= 1.0f )
   {
      d0 = d1;
      a0 = a1;
   }
   else
   {
      dn /= CGM::sqrt(dSqr);
      d0 += dn.z*maxD;
      a0 += dn(0,1)*maxA;
   }
}
#endif

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   NAMESPACE BasicActions
==============================================================================*/

//------------------------------------------------------------------------------
//!
void BasicActions::initialize()
{
   _typeFollowEntity_str = "followEntity";
   _typeWaypoint_str     = "waypoint";
   _typeMoveTo_str       = "moveTo";

   Action::registerAction( _typeMoveTo_str,       &MoveToAction::create );
   Action::registerAction( _typeFollowEntity_str, &FollowEntity::create );
   Action::registerAction( _typeWaypoint_str    , &Waypoint::create     );
}

//------------------------------------------------------------------------------
//!
void BasicActions::terminate()
{
   _typeFollowEntity_str = ConstString();
   _typeWaypoint_str     = ConstString();
   _typeMoveTo_str       = ConstString();
}

/*==============================================================================
  CLASS MoveToAction
==============================================================================*/

//------------------------------------------------------------------------------
//!
Action* MoveToAction::create()
{
   return new MoveToAction;
}

//-----------------------------------------------------------------------------
//!
MoveToAction::MoveToAction():
   _position(0.0f),
   _speed(1.0f)
{
}

//-----------------------------------------------------------------------------
//!
const ConstString&
MoveToAction::actionType()
{
   return _typeMoveTo_str;
}

//-----------------------------------------------------------------------------
//!
const ConstString&
MoveToAction::type() const
{
   return _typeMoveTo_str;
}

//-----------------------------------------------------------------------------
//!
bool
MoveToAction::execute( Entity& e, double /*t*/, double d )
{
   Vec3f pos = e.position();
   Vec3f dir = _position - pos;
   dir.maxLength( _speed * float(d) );
   pos += dir;
   e.position( pos );
   return CGM::equal( pos, _position );
}

//-----------------------------------------------------------------------------
//!
bool
MoveToAction::performGet( VMState* vm )
{
   if( Action::performGet(vm) )  return true;
   const char* key = VM::toCString( vm, -1 );
   switch( _movetoAttr[key] )
   {
      case MOVETO_POSITION:
         VM::push( vm, _position );
         return true;
      case MOVETO_SPEED:
         VM::push( vm, _speed );
         return true;
   }
   return false;
}

//-----------------------------------------------------------------------------
//!
bool
MoveToAction::performSet( VMState* vm )
{
   if( Action::performSet(vm) )  return true;
   const char* key = VM::toCString( vm, -2 );
   switch( _movetoAttr[key] )
   {
      case MOVETO_POSITION:
         _position = VM::toVec3f( vm, -1 );
         return true;
      case MOVETO_SPEED:
         _speed = VM::toFloat( vm, -1 );
         return true;
   }
   return false;
}

/*==============================================================================
   CLASS FollowEntity
==============================================================================*/

//------------------------------------------------------------------------------
//!
Action*
FollowEntity::create()
{
   return new FollowEntity;
}

//------------------------------------------------------------------------------
//!
FollowEntity::FollowEntity():
   Action( STATE_DEFAULT|STATE_RUN_AFTER_PHYSICS ),
   _maxLinearSpeed( 12.0f ),
   _maxAngularSpeed( 0.5f ),
   _maxDistanceSpeed( 4.0f*12.0f ),
   _height( 1.0f ),
   _angularVelocity( 0.0f ), _distanceVelocity( 0.0f ),
   _occluded( false )
{
   _currentState._followPos      = Vec3f(0.0f);
   _currentState._followDistance = 10.0f;
   _currentState._angles         = Vec2f(-0.1f, 0.10f);
   _targetState                  = _currentState;
}

//------------------------------------------------------------------------------
//!
const ConstString&
FollowEntity::type() const
{
   return _typeFollowEntity_str;
}

//------------------------------------------------------------------------------
//!
bool
FollowEntity::execute( Entity& entity, double /*time*/, double delta )
{
   if( _target.isNull() ) return false;

   const float deltaf = float(delta);

   // Update target.
   _targetState._followPos = _target->position();
   distance( _targetState._followDistance + _distanceVelocity*deltaf );
   angles( _targetState._angles + _angularVelocity*deltaf );

   // Compute the new current state.

   // 1. Account for a new followed position.
   float maxLinear   = deltaf * _maxLinearSpeed;
   _currentState._followPos = clampMove( _currentState._followPos, _targetState._followPos, maxLinear );

   // 2. Account for rotation.
   float maxAngular  = deltaf * _maxAngularSpeed;
   _currentState._angles = clampMove( _currentState._angles, _targetState._angles, maxAngular );

   // 3. Account for following distance.
   Reff ref = computeReferential( _currentState );
   // 3.1. Check occlusion.
   Vec3f followerPos = ref.position();
   Vec3f   targetPos = _currentState._followPos;
   targetPos.y += _height;
   float targetFollowDistance = checkOcclusion( targetPos, followerPos, _targetState._followDistance );
   // 3.2. Compute new distance.
   float maxDistance = deltaf * _maxDistanceSpeed;
   float followDistance = clampMove( _currentState._followDistance, targetFollowDistance, maxDistance );
   ref.translateLocal( Vec3f( 0.0f, 0.0f, followDistance - _currentState._followDistance ) );
   _currentState._followDistance = followDistance;

   // 4. Update with the new referential.
   entity.referential( ref );

   return false;
}

//------------------------------------------------------------------------------
//!
Reff
FollowEntity::computeReferential( const State& s )
{
   Quatf rotX = Quatf::axisCir( Vec3f(1.0f, 0.0f, 0.0f), -s._angles.y );
   Quatf rotY = Quatf::axisCir( Vec3f(0.0f, 1.0f, 0.0f),  s._angles.x );
   Reff ref( Vec3f( 0.0f, 0.0f, s._followDistance ) );

   ref.rotate( Vec3f(0.0f), rotY*rotX );
   ref.translate( s._followPos + Vec3f( 0.0f, _height, 0.0f ) );
   return ref;
}

//------------------------------------------------------------------------------
//!
void
FollowEntity::angles( const Vec2f& a )
{
   _targetState._angles.x = a.x;
   _targetState._angles.y = CGM::clamp( a.y, 0.001f, 0.25f );
}

//------------------------------------------------------------------------------
//!
void
FollowEntity::angularVelocity( const Vec2f& v )
{
   _angularVelocity = v;
   _angularVelocity.maxLength( _maxAngularSpeed );
}

//------------------------------------------------------------------------------
//!
void
FollowEntity::distance( float d )
{
   _targetState._followDistance = CGM::clamp( d, 1.0f, 100.0f );
}

//------------------------------------------------------------------------------
//!
void
FollowEntity::distanceVelocity( float v )
{
   _distanceVelocity = CGM::clamp( v, -_maxLinearSpeed, _maxLinearSpeed );
}

//------------------------------------------------------------------------------
//!
void
FollowEntity::followEntity( Entity* e, Entity* target )
{
   // Reset current state the first time?
   if( _target.isNull() )
   {
      Vec3f dir                     = e->position() - (target->position()+Vec3f(0.0f,_height,0.0f));
      _currentState._followPos      = target->position();
      _currentState._followDistance = length( dir );
      _currentState._angles.x       = CGM::radToCir( CGM::atan2( dir.x, dir.z ) );
      _currentState._angles.y       = CGM::radToCir( CGM::asin( dir.y/_currentState._followDistance ) );
   }

   _target = target;
}

//------------------------------------------------------------------------------
//!
float
FollowEntity::checkOcclusion( const Vec3f& targetPos, const Vec3f& followerPos, float targetFollowDistance )
{
   Vec3f dir = followerPos - targetPos;
   dir.rescale( targetFollowDistance ); // Make sure the ray ends at the target distance.
   Vector<Rayf> rays;
   rays.pushBack( Rayf( targetPos, dir ) ); // Doesn't hit the target, apparently.
   MotionWorld::IntersectionData data;
   _target->world()->motionWorld()->raycast( rays, RigidBody::STATIC_MASK, ~0x0, data );
   _occluded = data.hit(0);
   if( _occluded )
   {
      float d = length( data.location(0) - targetPos ) - 1.0f/256.0f;
      //StdErr << "HIT: " << targetPos << " " << followerPos << " + " << dir << " d=" << d << " " << targetFollowDistance << nl;
      return d;
   }
   else
   {
      //StdErr << "MISS: " << targetPos << " " << followerPos << " " << targetFollowDistance << nl;
      return targetFollowDistance;
   }
}

//------------------------------------------------------------------------------
//!
bool
FollowEntity::performGet( VMState* vm )
{
   switch( _followAttributes[VM::toCString( vm, -1 )] )
   {
      case ATTRIB_ANGLES:
         VM::push( vm, _targetState._angles );
         return true;
      case ATTRIB_ANGULAR_VELOCITY:
         VM::push( vm, _angularVelocity );
         return true;
      case ATTRIB_DISTANCE:
         // Use the current state if occluded so as to zoom back in immediately.
         VM::push( vm, _occluded ? _currentState._followDistance : _targetState._followDistance );
         return true;
      case ATTRIB_DISTANCE_VELOCITY:
         VM::push( vm, _distanceVelocity );
         return true;
      case ATTRIB_FOLLOW_ID:
         if( _target.isValid() ) VM::push( vm, _target->id() );
         else VM::push( vm );
         return true;
      case ATTRIB_HEIGHT:
         VM::push( vm, _height );
         return true;
      case ATTRIB_MAX_ANGULAR_SPEED:
         VM::push( vm, _maxAngularSpeed );
         return true;
      case ATTRIB_MAX_LINEAR_SPEED:
         VM::push( vm, _maxLinearSpeed );
         return true;
      default:
         break;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
bool
FollowEntity::performSet( VMState* vm )
{
   // Retrieve entity and world.
   BrainTaskContext* context = (BrainTaskContext*)VM::userData( vm );
   Entity* e = context->entity();
   World*  w = e->world();

   switch( _followAttributes[VM::toCString( vm, -2 )] )
   {
      case ATTRIB_ANGLES:
         angles( VM::toVec2f( vm, -1 ) );
         return true;
      case ATTRIB_ANGULAR_VELOCITY:
         angularVelocity( VM::toVec2f( vm, -1 ) );
         return true;
      case ATTRIB_DISTANCE:
         distance( VM::toFloat( vm, -1 ) );
         return true;
      case ATTRIB_DISTANCE_VELOCITY:
         distanceVelocity( VM::toFloat( vm, -1 ) );
         return true;
      case ATTRIB_FOLLOW_ID:
         followEntity( e, w->entity( VM::toConstString( vm, -1 ) ) );
         return true;
      case ATTRIB_HEIGHT:
         _height = VM::toFloat( vm, -1 );
         return true;
      case ATTRIB_MAX_ANGULAR_SPEED:
         _maxAngularSpeed = VM::toFloat( vm, -1 );
         return true;
      case ATTRIB_MAX_LINEAR_SPEED:
         _maxLinearSpeed = VM::toFloat( vm, -1 );
         return true;
      default:
         break;
   }
   return Action::performSet( vm );
}


/*==============================================================================
   CLASS Waypoint
==============================================================================*/

//------------------------------------------------------------------------------
//!
Action*
Waypoint::create()
{
   return new Waypoint();
}

//------------------------------------------------------------------------------
//!
Waypoint::Waypoint():
   _location( 0.0f ),
   _distance( 1.0f/128.0f )
{
}

//------------------------------------------------------------------------------
//!
const ConstString&
Waypoint::type() const
{
   return _typeWaypoint_str;
}

//------------------------------------------------------------------------------
//!
bool
Waypoint::execute( Entity& entity, double /*time*/, double /*delta*/ )
{
   Vec3f  toLoc = entity.position() - _location;
   if( CGM::sqrLength(toLoc) <= (_distance*_distance) )
   {
      // Send a specific stimulus?
      return true;
   }

   return false;
}

//------------------------------------------------------------------------------
//!
bool
Waypoint::performGet( VMState* vm )
{
   switch( _waypointAttributes[VM::toCString( vm, -1 )] )
   {
      case ATTRIB_DISTANCE:
         VM::push( vm, distance() );
         return true;
      case ATTRIB_LOCATION:
         VM::push( vm, location() );
         return true;
      default:
         break;
   }

   return Action::performGet( vm );
}

//------------------------------------------------------------------------------
//!
bool
Waypoint::performSet( VMState* vm )
{
   switch( _waypointAttributes[VM::toCString( vm, -2 )] )
   {
      case ATTRIB_DISTANCE:
         distance( VM::toFloat( vm, -1 ) );
         return true;
      case ATTRIB_LOCATION:
         location( VMMath::toVec3( vm, -1 ) );
         return true;
      default:
         break;
   }

   return Action::performSet( vm );
}

NAMESPACE_END
