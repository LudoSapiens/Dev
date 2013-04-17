/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Action/Action.h>

#include <Plasma/Action/TKAction.h>
#include <Plasma/Action/PuppeteerAction.h>
#include <Plasma/Action/BasicActions.h>
#include <Plasma/Stimulus/EventStimuli.h>
#include <Plasma/World/Brain.h>
#include <Plasma/World/Entity.h>
#include <Plasma/World/ParticleEntity.h>
#include <Plasma/World/World.h>

#include <Fusion/VM/VM.h>
#include <Fusion/VM/VMRegistry.h>

#include <Base/ADT/StringMap.h>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
enum
{
   ATTRIB_AUTO_DELETE,
   ATTRIB_ENABLED,
   ATTRIB_NOTIFY_ON_COMPLETION,
   ATTRIB_TYPE,
   NUM_ATTRIBS
};

//------------------------------------------------------------------------------
//!
StringMap _attributes(
   "autoDelete"        ,  ATTRIB_AUTO_DELETE,
   "enabled"           ,  ATTRIB_ENABLED,
   "notifyOnCompletion",  ATTRIB_NOTIFY_ON_COMPLETION,
   "type"              ,  ATTRIB_TYPE,
   ""
);

//------------------------------------------------------------------------------
//!
enum
{
   WAIT_ATTR_DURATION,
   NUM_WAIT_ATTR
};

//------------------------------------------------------------------------------
//!
StringMap _waitAttr(
   "duration",  WAIT_ATTR_DURATION,
   ""
);

//-----------------------------------------------------------------------------
//!
ConstString  _typeAction;
ConstString  _typeDebugAction;
ConstString  _typeParticleAction;
ConstString  _typeWaitAction;

//-----------------------------------------------------------------------------
//!
Map< ConstString, Action::CtorFunc >  _ctorFuncs;

//-----------------------------------------------------------------------------
//!
void actionSetVM( VMState* vm, int idx, Action& action )
{
   if( VM::isTable(vm, idx) )
   {
      VM::push(vm); // Start iterating at index 0 (nil).
      while( VM::next(vm, idx) )
      {
         action.performSet(vm);
         VM::pop(vm, 1); // Pop the value, keep the key.
      }
   }
}

//-----------------------------------------------------------------------------
//!
int actionVM( VMState* vm )
{
   BrainTaskContext* context = (BrainTaskContext*)VM::userData( vm );
   Entity* e = context->entity();

   const char* type = VM::toCString( vm, 1 );
   if( type == NULL )  return 0;
   ConstString typeStr = type;

   RCP<Action> action;
   bool reuseExisting = VM::toBoolean( vm, 3 );
   if( reuseExisting )
   {
      action = e->brain()->findAction( typeStr );
   }

   if( action.isNull() )
   {
      // Need to allocate a new one, and add it to the brain.
      action = Action::create( typeStr );
      if( action.isNull() )
      {
         StdErr << "ERROR: action() - Failed to create an action of type '" << typeStr << "'." << nl;
         return 0;
      }
      e->brain()->actions().pushBack( action );
   }
   else
   {
      action->enabled( true ); // Re-enable it (in case it was disabled).
   }

   actionSetVM( vm, 2, *action );
   context->entitiesWithActions( action->runAfterPhysics() ).pushBack( e );

   VM::pushProxy( vm, action.ptr() );
   return 1;
}

//-----------------------------------------------------------------------------
//!
int actionGetVM( VMState* vm )
{
   Action* a = (Action*)VM::toProxy( vm, 1 );
   CHECK( a );
   if( a->performGet(vm) )  return 1; // Can only support 1 return value.
   StdErr << "Unknown action attribute: " << VM::toCString( vm, -1 ) << nl;
   return 0;
}

//-----------------------------------------------------------------------------
//!
int actionSetVM( VMState* vm )
{
   Action* a = (Action*)VM::toProxy( vm, 1 );
   CHECK( a );
   if( a->performSet(vm) ) return 0;
   StdErr << "Unknown action attribute: " << VM::toCString( vm, -2 ) << nl;
   return 0;
}

//------------------------------------------------------------------------------
//!
const VM::Reg _actionFuncs[] = {
   { "action", actionVM },
   { 0,0 }
};

//-----------------------------------------------------------------------------
//!
void initVM( VMState* vm, uint /*mask*/ )
{
   VM::registerFunctions( vm, "_G", _actionFuncs );
}

//-----------------------------------------------------------------------------
//!
Action*  createDebugAction()
{
   return new DebugAction();
}

//-----------------------------------------------------------------------------
//!
Action*  createParticleAction()
{
   return new ParticleAction();
}

//-----------------------------------------------------------------------------
//!
Action*  createWaitAction()
{
   return new WaitAction();
}

UNNAMESPACE_END


/*==============================================================================
  CLASS Action
==============================================================================*/

//-----------------------------------------------------------------------------
//!
void
Action::initialize()
{
   _typeAction = "action";
   VMRegistry::add( initVM, VM_CAT_BRAIN );
   VMRegistry::add( _typeAction.cstr(), NULL, actionGetVM, actionSetVM, VM_CAT_BRAIN );

   _typeDebugAction = "debug";
   Action::registerAction( _typeDebugAction, &createDebugAction );

   _typeParticleAction = "particle";
   Action::registerAction( _typeParticleAction, &createParticleAction );

   _typeWaitAction = "wait";
   Action::registerAction( _typeWaitAction, &createWaitAction );

   BasicActions::initialize();
   PuppeteerAction::initialize();
   TKAction::initialize();
}

//-----------------------------------------------------------------------------
//!
void
Action::terminate()
{
   TKAction::terminate();
   PuppeteerAction::terminate();
   BasicActions::terminate();

   _ctorFuncs.clear();
   _typeAction         = ConstString();
   _typeDebugAction    = ConstString();
   _typeParticleAction = ConstString();
   _typeWaitAction     = ConstString();
}

//-----------------------------------------------------------------------------
//!
Action*
Action::create( const ConstString& type )
{
   const CtorFunc& func = _ctorFuncs[type];
   return func ? func() : nullptr;
}

//------------------------------------------------------------------------------
//!
//Action::Action( int state ):
//   _state( state )
//{
//}

//------------------------------------------------------------------------------
//!
Action::~Action()
{
}

//-----------------------------------------------------------------------------
//!
const char*
Action::meta() const
{
   return _typeAction.cstr();
}

#if 0
//------------------------------------------------------------------------------
//!
void
Action::init( VMState* vm )
{
   if( VM::isTable(vm, -1) )
   {
      VM::push(vm); // Start iterating at index 0 (nil).
      while( VM::next(vm, -2) )
      {
         performSet(vm);
         VM::pop(vm, 1); // Pop the value, keep the key.
      }
   }
}
#endif

//------------------------------------------------------------------------------
//!
bool
Action::performGet( VMState* vm )
{
   switch( _attributes[VM::toCString(vm, -1)] )
   {
      case ATTRIB_AUTO_DELETE:
         VM::push( vm, autoDelete() );
         return true;
      case ATTRIB_ENABLED:
         VM::push( vm, enabled() );
         return true;
      case ATTRIB_NOTIFY_ON_COMPLETION:
         VM::push( vm, notifyOnCompletion() );
         return true;
      case ATTRIB_TYPE:
         VM::push( vm, type().cstr() );
         return true;
      default:
         break;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
bool
Action::performSet( VMState* vm )
{
   switch( _attributes[VM::toCString(vm, -2)] )
   {
      case ATTRIB_AUTO_DELETE:
         autoDelete( VM::toBoolean(vm, -1) );
         return true;
      case ATTRIB_ENABLED:
         enabled( VM::toBoolean(vm, -1) );
         return true;
      case ATTRIB_NOTIFY_ON_COMPLETION:
         notifyOnCompletion( VM::toBoolean(vm, -1) );
         return true;
      case ATTRIB_TYPE:
         return true; // Read-only.
      default:
         break;
   }
   return false;
}

//-----------------------------------------------------------------------------
//!
void
Action::registerAction( const ConstString& type, Action::CtorFunc ctorFunc )
{
   CHECK( !_ctorFuncs.has(type) );
   _ctorFuncs[type] = ctorFunc;
}


/*==============================================================================
  CLASS DebugAction
==============================================================================*/

//-----------------------------------------------------------------------------
//!
DebugAction::DebugAction()
{
   _attr = new Table();
}

//-----------------------------------------------------------------------------
//!
const ConstString&
DebugAction::actionType()
{
   return _typeDebugAction;
}

//-----------------------------------------------------------------------------
//!
const ConstString&
DebugAction::type() const
{
   return _typeDebugAction;
}

//-----------------------------------------------------------------------------
//!
bool
DebugAction::execute( Entity& e, double t, double d )
{
   StdErr << "DebugAction: e=" << (void*)&e << " t=" << t << " d=" << d << " attr=";
   _attr->print( StdErr );
   StdErr << nl;
   //return false;
   return CGM::fract(t) > 0.75;
}

//-----------------------------------------------------------------------------
//!
bool
DebugAction::performGet( VMState* vm )
{
   if( Action::performGet(vm) )  return true;
   const char* key = VM::toCString( vm, -1 );
   const Table& t = *_attr; // Force to use const version (which does add entries).
   VM::push( vm, t.get(key) );
   return 1;
}

//-----------------------------------------------------------------------------
//!
bool
DebugAction::performSet( VMState* vm )
{
   if( Action::performSet(vm) )  return true;
   VM::toVariant( vm, *_attr );
   return true;
}

/*==============================================================================
  CLASS PärticleAction
==============================================================================*/

//-----------------------------------------------------------------------------
//!
ParticleAction::ParticleAction()
{
}

//-----------------------------------------------------------------------------
//!
const ConstString&
ParticleAction::actionType()
{
   return _typeParticleAction;
}

//-----------------------------------------------------------------------------
//!
const ConstString&
ParticleAction::type() const
{
   return _typeParticleAction;
}

//-----------------------------------------------------------------------------
//!
bool
ParticleAction::execute( Entity& e, double /*t*/, double d )
{
   if( e.type() == Entity::PARTICLE )
   {
      ParticleEntity& pe = reinterpret_cast<ParticleEntity&>(e);
      pe.simulate( float(d), pe.rng() );
      return false;
   }
   else
   {
      StdErr << "ParticleAction not bound to a ParticleEntity." << nl;
      CHECK( false );
      return true;
   }
}


/*==============================================================================
  CLASS WaitAction
==============================================================================*/

//-----------------------------------------------------------------------------
//!
WaitAction::WaitAction():
   _duration(0.0f)
{
}

//-----------------------------------------------------------------------------
//!
const ConstString&
WaitAction::actionType()
{
   return _typeWaitAction;
}

//-----------------------------------------------------------------------------
//!
const ConstString&
WaitAction::type() const
{
   return _typeWaitAction;
}

//-----------------------------------------------------------------------------
//!
bool
WaitAction::execute( Entity& /*e*/, double /*t*/, double d )
{
   _duration -= float(d);
   return _duration <= 0.0f;
}

//-----------------------------------------------------------------------------
//!
bool
WaitAction::performGet( VMState* vm )
{
   if( Action::performGet(vm) )  return true;
   const char* key = VM::toCString( vm, -1 );
   switch( _waitAttr[key] )
   {
      case WAIT_ATTR_DURATION:
         VM::push( vm, _duration );
         return true;
   }
   return false;
}

//-----------------------------------------------------------------------------
//!
bool
WaitAction::performSet( VMState* vm )
{
   if( Action::performSet(vm) )  return true;
   const char* key = VM::toCString( vm, -2 );
   switch( _waitAttr[key] )
   {
      case WAIT_ATTR_DURATION:
         _duration = (float)VM::toNumber( vm, -1 );
         return true;
   }
   return false;
}


/*==============================================================================
  CLASS ActionTask
==============================================================================*/

//-----------------------------------------------------------------------------
//!
ActionTask::ActionTask( Entity* e, double t, double d, bool afterPhysics ):
   _entity( e ),
   _time( t ),
   _delta( d ),
   _afterPhysics( afterPhysics )
{
}

//-----------------------------------------------------------------------------
//!
void
ActionTask::execute()
{
   if( _entity.isNull() )
   {
      StdErr << "ERROR - ActionTask::execute() called on a NULL entity." << nl;
      return;
   }

   Brain* brain = _entity->brain();
   if( brain == NULL )
   {
      StdErr << "ERROR - ActionTask::execute() called on an entity (" << (void*)_entity.ptr() << ") without a brain." << nl;
      return;
   }

   bool stillRunning = false;
   bool mustClean    = false;
   Brain::ActionContainer& actions = brain->actions();
   //StdErr << "Entity " << (void*)_entity.ptr() << " has " << actions.size() << " actions" << nl;
   CHECK( actions.size() < 100 );
   for( auto cur = actions.begin(); cur != actions.end(); ++cur )
   {
      RCP<Action>& action = (*cur);
      if( action->runAfterPhysics() == _afterPhysics )
      {
         if( action->canExecute() )
         {
            if( action->execute( *_entity, _time, _delta ) )
            {
               action->enabled( false );  // Disable it.
               if( action->notifyOnCompletion() )
               {
                  _entity->stimulate( new ActionCompleted( action.ptr() ) );
               }
               if( action->autoDelete() )
               {
                  action = NULL;
                  mustClean = true;
               }
            }
            else
            {
               stillRunning = true;
            }
         }
      }
   }
   if( mustClean )
   {
      brain->actions().removeAllSwap( NULL );
   }

   if( !stillRunning )
   {
      // Flag yourself to be removed from the World's list of entities running actions.
      if( _afterPhysics )  _entity->runningActionsAfter( false );
      else                 _entity->runningActionsBefore( false );
   }
}
