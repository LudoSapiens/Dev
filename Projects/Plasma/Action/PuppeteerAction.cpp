/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Action/PuppeteerAction.h>
#include <Plasma/Animation/Puppeteer.h>
#include <Plasma/Stimulus/EventStimuli.h>
#include <Plasma/World/World.h>

#include <Base/ADT/StringMap.h>
#include <Base/Dbg/DebugStream.h>

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_pup, "PuppeteerAction" );

ConstString  _typePuppeteerAction;

//------------------------------------------------------------------------------
//!
enum
{
   ATTRIB_DECELERATE_SPEED,
   ATTRIB_DESTINATION,
   ATTRIB_DIRECTION,
   ATTRIB_JUMP_CONST,
   ATTRIB_JUMP_SCALE,
   ATTRIB_MAXACCELERATION,
   ATTRIB_MAX_ANGULAR_SPEED,
   ATTRIB_SET_STATE,
   ATTRIB_SPEED,
   ATTRIB_STATE,
   NUM_ATTRIBS
};

//------------------------------------------------------------------------------
//!
StringMap _attributes(
   "decelerateSpeed", ATTRIB_DECELERATE_SPEED,
   "destination",     ATTRIB_DESTINATION,
   "direction",       ATTRIB_DIRECTION,
   "jumpConst",       ATTRIB_JUMP_CONST,
   "jumpScale",       ATTRIB_JUMP_SCALE,
   "maxAcceleration", ATTRIB_MAXACCELERATION,
   "maxAngularSpeed", ATTRIB_MAX_ANGULAR_SPEED,
   "setState",        ATTRIB_SET_STATE,
   "speed",           ATTRIB_SPEED,
   "state",           ATTRIB_STATE,
   ""
);

/*
String  fmtXZ( const Vec3f& v )
{
   return String().format("(%.2f,%.2f)", v.x, v.z);
}
*/

//------------------------------------------------------------------------------
//!
int
setStateVM( VMState* vm )
{
   PuppeteerAction* puppeteer = (PuppeteerAction*)VM::thisPtr( vm );
   if( VM::getTop( vm ) == 1 )
   {
      VM::push( vm, puppeteer->setState( ConstString(VM::toCString( vm, 1 )) ) );
   }
   else
   {
      VM::push( vm, puppeteer->setState( ConstString(VM::toCString( vm, 1 )), VM::toBoolean( vm, 2 ) ) );
   }
   return 1;
}

//------------------------------------------------------------------------------
//!
inline void
moveEntity(
   const RCP<SkeletalEntity>& entity,
   const Vec3f&               linearVelocity,
   const Quatf&               orientation
)
{
   entity->linearVelocity( linearVelocity );
   entity->orientation( orientation );
}

//------------------------------------------------------------------------------
//! Return true if the first specified path (the backpedal one) takes less time.
inline bool
bestPathIsBackpedal(
   const RCP<AnimationGraph::Path>&   node1ToGoal,
   const RCP<AnimationGraph::Path>&   node2ToGoal,
   const float                        hereToNode1Time,
   const float                        hereToNode2Time,
   const AnimationGraph::Transition*  curTrans
)
{
   if( node2ToGoal->transition(0) == curTrans ) return true;

   // Determine fastest path.
   const float durNode1 = node1ToGoal->computeDuration() + hereToNode1Time;
   const float durNode2 = node2ToGoal->computeDuration() + hereToNode2Time;

   return durNode1 < durNode2;
}

RCP<AnimationGraph::Path> _gEmptyPath( new AnimationGraph::Path() );

//-----------------------------------------------------------------------------
//!
Action*  createPuppeteerAction()
{
   return new PuppeteerAction();
}

UNNAMESPACE_END


NAMESPACE_BEGIN

/*==============================================================================
  CLASS PuppeteerAction
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
PuppeteerAction::initialize()
{
   _typePuppeteerAction = "puppeteer";
   Action::registerAction( _typePuppeteerAction, &createPuppeteerAction );
}

//------------------------------------------------------------------------------
//!
void
PuppeteerAction::terminate()
{
   _typePuppeteerAction = ConstString();
}

//------------------------------------------------------------------------------
//!
PuppeteerAction::PuppeteerAction() :
   _desNode( nullptr ),
   _manNode( nullptr ),
   _curPath( nullptr ),
   _trIdx( 0 ),
   _trTime( 0.0 ),
   _curNode( nullptr ),
   _lastFalling( false ),
   _reallyFalling( false ),
   _fallDistSq( 0.1f ),
   _fallMinDur( 0.3f )
{
}

//------------------------------------------------------------------------------
//!
PuppeteerAction::~PuppeteerAction()
{
}

//------------------------------------------------------------------------------
//!
void
PuppeteerAction::updatePose()
{
   if( entity() != nullptr )
   {
      _pose = new SkeletalPose( Reff::identity(), entity()->skeleton()->numBones() );
   }
   else
      _pose = nullptr;
}

//-----------------------------------------------------------------------------
//!
const ConstString&
PuppeteerAction::actionType()
{
   return _typePuppeteerAction;
}

//-----------------------------------------------------------------------------
//!
const ConstString&
PuppeteerAction::type() const
{
   return _typePuppeteerAction;
}

//------------------------------------------------------------------------------
//!
bool
PuppeteerAction::execute( Entity&, double /*time*/, double delta )
{
   // Handle events.
   checkFalling();

   if( _curPath.isValid() && _node1State->canExit() )
   {
      // Handle transitions.
      const uint numTrans                     = _curPath->numTransitions();
      const AnimationGraph::Transition* trans = _curPath->transition( _trIdx );
      double trDelta                          = trans->duration() - _trTime;
      // 1. Expire completed transitions.
      while( trDelta <= delta )
      {
         trans->node1()->perform( _node1State.ptr(), trDelta );
         trans->node2()->perform( _node2State.ptr(), trDelta );
         if( trans->node2() == _manNode )  _manNode = nullptr;

         _node1State = _node2State;
         _trTime     = 0.0;
         delta      -= trDelta;
         ++_trIdx;
         if( _trIdx != numTrans )
         {
            trans       = _curPath->transition( _trIdx );
            _node2State = trans->node2()->createState( &_params );
            if( _node1State->canExit() )
            {
               trDelta = trans->duration();
            }
            else
            {
               // Handle node (fallback below).
               _curNode = trans->node1();
               trans    = nullptr; // Skip real-time blending.
               break;
            }
         }
         else
         {
            // Handle node (fallback below).
            _node2State = nullptr;
            _curNode    = trans->node2();
            _curPath    = nullptr;
            trans       = nullptr; // Skip real-time blending.
            break;
         }
      }

      // 2. Perform real-time blending.
      if( trans != nullptr )
      {
         trans->node1()->perform( _node1State.ptr(), delta );
         trans->node2()->perform( _node2State.ptr(), delta );
         if( trans->node2() == _manNode )  _manNode = nullptr;

         _trTime += delta;  // Corresponds to the time position in the transition.
         float f  = (float)(_trTime / trans->duration());

         Puppeteer::blendPoses( _node1State->pose(), _node2State->pose(), f, *_pose );
         Puppeteer::setPose( entity()->instance(), _pose.ptr() );

         Vec3f vel = CGM::linear2( _node1State->velocity(), _node2State->velocity(), f );
         Quatf ori = _node1State->orientation().slerp( _node2State->orientation(), f );
         moveEntity( entity(), vel, ori );
         return false;
      }
   }

   // Handle node.
   _curNode->perform( _node1State.ptr(), delta );
   Puppeteer::setPose( entity()->instance(), _node1State->pose() );
   moveEntity( entity(), _node1State->velocity(), _node1State->orientation() );

   // Action never expires.
   return false;
}

//------------------------------------------------------------------------------
//! Defines a new destination animation.
//! If no path can be found from the current point to the desired goal,
//! nothing is done.
//! Important invariances:
//!  - When a transition is active, _curPath is set
//!  - When a node is to be used alone, _curNode is valid
//!  - Both _curPath and _curNode can be valid simultaneously: this means a node cannot exit
//! Mandatory actions block others until they are completed.  Only one such event is honored (the first one).
bool
PuppeteerAction::setState( const ConstString& name, const bool mandatory )
{
   if( _manNode != nullptr ) return false;

   AnimationNode* node = _graph->node( name );
   if( node == nullptr ) return false;

   //state = _graph->followup( state );
   // IMPORTANT: From this point on, we should use 'state->name()' instead of 'name' (because of the followup).
   if( node == _desNode ) return true;

   if( _curPath.isValid() )
   {
      AnimationGraph::Transition* trans = _curPath->transition( _trIdx );

      if( trans->node2() == node )
      {
         // Chopping path to the current transition only.
         _curPath = _curPath->clone( _trIdx, _trIdx );
         _trIdx   = 0;
         _desNode = _curPath->lastNode(); // Register new goal as valid.
         if( mandatory ) _manNode = node;
         // Keep _trTime untouched.
         return true;
      }

      // Determine fastest path: continue until the goal, or backpedal.
      RCP<AnimationGraph::Path> newPath;
      RCP<AnimationGraph::Path> node2ToGoal = _graph->path( trans->node2(), node );

      if( node2ToGoal.isNull() ) return false;

      AnimationGraph::Transition* node2ToNode1 = _graph->transition( trans->node2(), trans->node1() );
      RCP<AnimationGraph::Path>   node1ToGoal  = (trans->node1() == node) ? _gEmptyPath : _graph->path( trans->node1(), node );
      if( !trans->interruptible() || node2ToNode1 == nullptr || node1ToGoal.isNull() || !_node2State->canExit() )
      {
         newPath = _curPath->clone( _trIdx, _trIdx );
         newPath->add( node2ToGoal.ptr() );
      }
      else
      {
         float f = (float)_trTime / trans->duration();
         if( bestPathIsBackpedal(node1ToGoal, node2ToGoal, f*node2ToNode1->duration(), trans->duration()-(float)_trTime, trans) )
         {
            newPath = new AnimationGraph::Path();
            newPath->add( node2ToNode1 );
            newPath->add( node1ToGoal.ptr() );
            // Reverse current transition.
            // Calculate a _trTime yielding the same frame as the forward transition.
            _trTime = (1.0f - f) * node2ToNode1->duration();
            // Reverse the 2 states.
            CGM::swap( _node1State, _node2State );
            // Reset the state of the old state1.
            _node2State->reset();
         }
         else
         {
            newPath = _curPath->clone( _trIdx, _trIdx );
            newPath->add( node2ToGoal.ptr() );
         }
      }

      // Register new valid path (_trTime was already modified).
      _curPath = newPath;
      _trIdx   = 0;
      _desNode = _curPath->lastNode(); // Register new goal as valid.
      if( mandatory ) _manNode = node;

      return true;
   }
   else
   if( _curNode == nullptr )
   {
      // Neither _curPath nor _curNode are set; just set the animation.
      // (happens at startup time)
      if( mandatory ) _manNode = node;
      _desNode    = node; // Register new goal as valid.
      _curNode    = _desNode;
      _node1State = _curNode->createState( &_params );
      return true;
   }

   // We were looping an animation.
   // Simply add the shortest path, and start the transition.
   _curPath = _graph->path( _curNode, node );
   if( _curPath.isValid() )
   {
      if( mandatory ) _manNode = node;
      _desNode    = _curPath->lastNode(); // Register new goal as valid.
      _trIdx      = 0;
      _trTime     = 0.0f; // Mark beginning of transition.
      _node2State = _curPath->transition(0)->node2()->createState( &_params );
      return true;
   }
   return false;
}

//-----------------------------------------------------------------------------
//!
void
PuppeteerAction::checkFalling()
{
   // Retrieve current falling state.
   bool falling = entity()->falling();

   // Reset state when falling toggles.
   if( falling != _lastFalling )
   {
      _reallyFalling = false; // Reset variable.
      if( falling )
      {
         // Determine starting floor height.
         World* w = entity()->world();
         Vec3f  p = entity()->position();
         Vector<Rayf> rays;
         rays.pushBack( Rayf( p, w->gravity().getRescaled(1024.0f) ) ); // TODO: Bounding box for scene?
         MotionWorld::IntersectionData data;
         w->motionWorld()->raycast( rays, RigidBody::ANY_MASK, ~0x0, data );
         float d2 = 1024.0f*1024.0f; // If not hit, this is the minimum distance squared the floor could be at.
         if( data.hit(0) )
         {
            const Vec3f& f = data.location(0);
            d2 = CGM::sqrLength( p - f );
         }
         if( d2 >= _fallDistSq )
         {
            startFalling();
         }
      }
      // else landed (different from landing).
   }

   // Assign latest state.
   _lastFalling = falling;

   // Handle whether or not we are really falling.
   if( _lastFalling )
   {
      if( !_reallyFalling )
      {
         // Check based on duration.
         if( entity()->fallDuration() >= _fallMinDur )
         {
            startFalling();
            return;
         }
         // Check other conditions?
      }
   }
}

//-----------------------------------------------------------------------------
//!
void
PuppeteerAction::startFalling()
{
   _reallyFalling = true;
   entity()->stimulate( new FallStimulus() ); // TODO: avoid sending stimulus on jumps, or let the brains handle this?
   if( !_graph->onFall().isNull() )
   {
      setState( _graph->onFall(), true );
   }
}

//------------------------------------------------------------------------------
//!
bool
PuppeteerAction::performGet( VMState* vm )
{
   switch( _attributes[VM::toCString(vm, -1)] )
   {
      case ATTRIB_DECELERATE_SPEED:
         VM::push( vm, _params.decelerateSpeed() );
         return true;
      case ATTRIB_DESTINATION:
         if( _params.hasDestination() )  VM::push( vm, _params.destination() );
         else                            VM::push( vm ); // Nil.
         return true;
      case ATTRIB_DIRECTION:
         VM::push( vm, _params.direction() );
         return true;
      case ATTRIB_JUMP_CONST:
         VM::push( vm, _params.jumpConst() );
         return true;
      case ATTRIB_JUMP_SCALE:
         VM::push( vm, _params.jumpScale() );
         return true;
      case ATTRIB_MAXACCELERATION:
         VM::push( vm, _params.maxAcceleration() );
         return true;
      case ATTRIB_MAX_ANGULAR_SPEED:
         VM::push( vm, _params.maxAngularSpeed() );
         return true;
      case ATTRIB_SET_STATE:
         VM::push( vm, this, setStateVM );
         return true;
      case ATTRIB_SPEED:
         VM::push( vm, _params.speed() );
         return true;
      case ATTRIB_STATE:
         VM::push(vm);
         return true;
      default:
         break;
   }
   return Action::performGet( vm );
}

//------------------------------------------------------------------------------
//!
bool
PuppeteerAction::performSet( VMState* vm )
{
   switch( _attributes[VM::toCString(vm, -2)] )
   {
      case ATTRIB_DECELERATE_SPEED:
         _params.decelerateSpeed( VM::toFloat( vm, -1 ) );
         return true;
      case ATTRIB_DIRECTION:
         _params.direction( VM::toVec3f( vm, -1 ) );
         return true;
      case ATTRIB_DESTINATION:
         switch( VM::type( vm, -1 ) )
         {
            case VM::TABLE:
            case VM::OBJECT:
               //_params.destination( VM::toVec3f( vm, -1 ) );
               _params.addWaypoint( VM::toVec3f( vm, -1 ) );
               //{
               //   uint n = _params.numWaypoints();
               //   StdErr << "NOW HAS " << n << " waypoints:";
               //   for( uint i = 0; i < n; ++i )
               //   {
               //      StdErr << " " << fmtXZ(_params.waypoint(i));
               //   }
               //   StdErr << nl;
               //}
               break;
            default:
               _params.clearDestination();
               break;
         }
         return true;
      case ATTRIB_JUMP_CONST:
         _params.jumpConst( VM::toVec3f( vm, -1 ) );
         return true;
      case ATTRIB_JUMP_SCALE:
         _params.jumpScale( VM::toVec3f( vm, -1 ) );
         return true;
      case ATTRIB_MAXACCELERATION:
         _params.maxAcceleration( VM::toFloat( vm, -1 ) );
         return true;
      case ATTRIB_MAX_ANGULAR_SPEED:
         _params.maxAngularSpeed( VM::toFloat( vm, -1 ) );
         return true;
      case ATTRIB_SET_STATE:
         // Read-only.
         return true;
      case ATTRIB_SPEED:
         _params.speed( VM::toFloat( vm, -1 ) );
         return true;
      case ATTRIB_STATE:
         setState( ConstString(VM::toCString( vm, -1 )) );
         return true;
      default:
         break;
   }
   return Action::performSet( vm );
}

NAMESPACE_END
