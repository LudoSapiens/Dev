/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Animation/TravelNode.h>

#include <Plasma/Animation/Puppeteer.h>
#include <Plasma/Stimulus/Orders.h>

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
float clampAngle( float a, float minA, float maxA )
{
   float c = (minA+maxA)*0.5f;

   if( a < (c - 0.5f) )
      a += 1.0f;
   else if( a > (c + 0.5f) )
      a -= 1.0f;
   // Clamp.
   return CGM::clamp( a, minA, maxA );
}

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   CLASS TravelNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
TravelNode::TravelNode()
{
}

//------------------------------------------------------------------------------
//!
TravelNode::~TravelNode()
{
}

//------------------------------------------------------------------------------
//!
RCP<AnimationNode>
TravelNode::clone() const
{
   RCP<TravelNode> node( new TravelNode() );
   node->_name      = _name;
   node->_anims     = _anims;
   node->_speeds    = _speeds;
   node->_distances = _distances;
   return node;
}

//------------------------------------------------------------------------------
//!
void TravelNode::getAnimations( Vector< SkeletalAnimation* >& anims )
{
   for( uint i = 0; i < _anims.size(); ++i )
   {
      anims.pushBack( _anims[i].ptr() );
   }
}

//------------------------------------------------------------------------------
//!
void TravelNode::replaceAnimations(
   const Map< SkeletalAnimation*, RCP<SkeletalAnimation> >&  anims
)
{
   for( uint i = 0; i < _anims.size(); ++i )
   {
      auto it = anims.find( _anims[i].ptr() );
      if( it != anims.end() )
      {
         const auto& anim = (*it).second;
         _anims[i]        = anim;
         float speed      = anim->velocity().length();
         _speeds[i]       = speed;
         _distances[i]    = anim->duration()*speed;
      }
   }
}

//------------------------------------------------------------------------------
//!
RCP<AnimationState>
TravelNode::createState( AnimationParameter* p ) const
{
   RCP<TravelState> state = new TravelState();

   state->parameters( p );
   state->canExit( true );
   state->hasArrived( false );

   Vec3f dir = p->entity()->orientation().getAxisZ();
   dir.y     = 0.0f;

   state->speed( 0.0f );
   state->direction( normalize( dir ) );
   state->orientation( p->entity()->orientation() );
   state->orientationAngle( CGM::radToCir(CGM::atan2( dir.x, dir.z )) );

   state->pose( new SkeletalPose( Reff::identity(), _anims[0]->pose(0)->numBones() ) );
   state->time(0);

   return state;
}

//------------------------------------------------------------------------------
//!
void
TravelNode::perform( AnimationState* state, double deltaTime )
{
   TravelState* curState      = (TravelState*)state;
   AnimationParameter& params = *(curState->parameters());

   // Update velocities.
   Vec3f dir   = params.direction();
   float speed = params.speed();

   if( curState->hasArrived() )
   {
      dir   = curState->direction();
      speed = 0.0f;
   }
   else
   if( params.hasDestination() )
   {
      dir     = params.destination() - params.entity()->position();
      dir.y   = 0.0f;
      float d = length( dir );
      if( d <= params.arrivedRange() )
      {
         //StdErr << "Arrived at " << params.destination() << " (" << params.entity()->position() << ")" << nl;
         params.popWaypoint();
         //StdErr << params.numWaypoints() << " TO GO" << nl;
      }

      if( params.hasDestination() )
      {
         // We need to keep moving.
         float f        = (params.numWaypoints() > 1) ? 1.0f : CGM::smoothStep( params.arrivedRange(), params.arrivingRange(), d );
         float maxSpeed = params.speed();
         float minSpeed = CGM::min( params.decelerateSpeed(), maxSpeed );
         speed          = f*(maxSpeed-minSpeed) + minSpeed;
         dir            = dir/d;
      }
      else
      {
         // We've reached our goal.
         //StdErr << "STOPPING" << nl;
         dir   = d < CGConstf::epsilon() ? curState->direction() : dir/d;
         speed = 0.0f;
         params.entity()->stimulate( new StopOrder() );
         curState->hasArrived( true );
      }
   }

   // Clamp speed with maximum acceleration.
   float maxAcc   = params.maxAcceleration()*(float)deltaTime;
   float curSpeed = curState->speed();
   speed          = CGM::clamp( speed, curSpeed-maxAcc, curSpeed+maxAcc );

   // Assign velocities.
   curState->speed( speed );
   curState->direction( dir );

   // Find witch animations to blend and what factor to obtain the desired velocity.
   double time = curState->time();
   if( speed <= _speeds[0] )
   {
      time += deltaTime*speed/_distances[0];

      float t;
      SkeletalPose* p0;
      SkeletalPose* p1;
      _anims[0]->getPoses( (float)time*_anims[0]->duration(), p0, p1, t );

      Puppeteer::blendPoses( p0, p1, t, *curState->pose() );
   }
   else if( speed >= _speeds.back() )
   {
      time += deltaTime*speed/_distances.back();

      float t;
      SkeletalPose* p0;
      SkeletalPose* p1;
      _anims.back()->getPoses( (float)time*_anims.back()->duration(), p0, p1, t );

      Puppeteer::blendPoses( p0, p1, t, *curState->pose() );
   }
   else
   {
      int p = 0;
      while( speed > _speeds[p+1] ) ++p;

      float blend = (speed-_speeds[p])/(_speeds[p+1]-_speeds[p]);
      float dist  = _distances[p]*(1.0f-blend) + _distances[p+1]*blend;

      time += deltaTime*speed/dist;

      float t0;
      float t1;
      SkeletalPose* p0;
      SkeletalPose* p1;
      SkeletalPose* p2;
      SkeletalPose* p3;
      _anims[p]->getPoses( (float)time*_anims[p]->duration(), p0, p1, t0 );
      _anims[p+1]->getPoses( (float)time*_anims[p+1]->duration(), p2, p3, t1 );

      Puppeteer::blendPoses( p0, p1, p2, p3, t0, t1, blend, *curState->pose() );
   }


   // Take into account the direction of the velocity to reorient the movement.

   // Match orientation with velocity.
   if( speed > 0.0f )
   {
      // Target orientation.
      float curAng      = curState->orientationAngle();
      float maxAngSpeed = params.maxAngularSpeed()*(float)deltaTime;
      float ang         = CGM::radToCir( CGM::atan2( dir.x, dir.z ) );
      ang               = clampAngle( ang, curAng - maxAngSpeed, curAng + maxAngSpeed );
      ang               = CGM::fract( ang ); // bring back in [0,1[ range.
      Quatf ori         = Quatf::eulerY( CGM::cirToRad(ang) );

      curState->orientation( ori );
      curState->orientationAngle( ang );
   }

   // Update the current time.
   curState->time( time );
}

//------------------------------------------------------------------------------
//!
void
TravelNode::addAnimation( SkeletalAnimation* anim )
{
   _anims.pushBack( anim );
   _speeds.pushBack( anim->velocity().length() );
   _distances.pushBack( anim->duration()*_speeds.back() );
}

NAMESPACE_END
