/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Stimulus/EventStimuli.h>

#include <Plasma/World/Entity.h>
#include <Plasma/World/RigidEntity.h>

#include <Fusion/VM/VMRegistry.h>

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

ConstString  _sType_ActionCompleted;
ConstString  _sType_Begin;
ConstString  _sType_ContactBegin;
ConstString  _sType_ContactEnd;
ConstString  _sType_Fall;
ConstString  _sType_Land;

//------------------------------------------------------------------------------
//!
bool  VM__get( VMState* vm, int idx, const char* key, RCP<Action>& action )
{
   if( VM::get( vm, idx, key ) )
   {
      action = (Action*)VM::toProxy( vm, idx );
      return true;
   }
   return false;
}

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
   NAMESPACE EventStimuli
==============================================================================*/

namespace EventStimuli
{

//------------------------------------------------------------------------------
//!
void initialize()
{
#define REGISTER( sTypeVar, str, type ) \
   sTypeVar = str; \
   Stimulus::registerStimulus( sTypeVar, create<Stimulus,type> )

   REGISTER( _sType_ActionCompleted, "actionCompleted", ActionCompleted      );
   REGISTER( _sType_Begin          , "begin"          , BeginStimulus        );
   REGISTER( _sType_ContactBegin   , "contactBegin"   , ContactBeginStimulus );
   REGISTER( _sType_ContactEnd     , "contactEnd"     , ContactEndStimulus   );
   REGISTER( _sType_Fall           , "fall"           , FallStimulus         );
   REGISTER( _sType_Land           , "land"           , LandStimulus         );

#undef REGISTER
}

//------------------------------------------------------------------------------
//!
void terminate()
{
#define UNREGISTER( sTypeVar ) \
   Stimulus::unregisterStimulus( sTypeVar ); \
   sTypeVar = ConstString()

   UNREGISTER( _sType_ActionCompleted );
   UNREGISTER( _sType_Begin           );
   UNREGISTER( _sType_ContactBegin    );
   UNREGISTER( _sType_ContactEnd      );
   UNREGISTER( _sType_Fall            );
   UNREGISTER( _sType_Land            );

#undef UNREGISTER
}

} //namespace EventStimuli


/*==============================================================================
  CLASS ActionCompleted
==============================================================================*/

//------------------------------------------------------------------------------
//!
const ConstString&
ActionCompleted::type() const
{
   return _sType_ActionCompleted;
}

//------------------------------------------------------------------------------
//!
void
ActionCompleted::push( VMState* vm )
{
   Stimulus::push( vm );

   VM::push( vm, "action" );
   VM::pushProxy( vm, _action.ptr() );
   VM::set( vm, -3 );
}

//------------------------------------------------------------------------------
//!
bool
ActionCompleted::to( VMState* vm, int idx )
{
   return VM__get( vm, idx, "action", _action );
}


/*==============================================================================
  CLASS BeginStimulus
==============================================================================*/

//------------------------------------------------------------------------------
//!
const ConstString&
BeginStimulus::type() const
{
   return _sType_Begin;
}


/*==============================================================================
  CLASS ContactBeginStimulus
==============================================================================*/

//------------------------------------------------------------------------------
//!
const ConstString&
ContactBeginStimulus::type() const
{
   return _sType_ContactBegin;
}

//------------------------------------------------------------------------------
//!
void
ContactBeginStimulus::push( VMState* vm )
{
   Stimulus::push( vm );

   VM::push( vm, "receivingEntity" );
   VM::pushProxy( vm, _receiving.ptr() );
   VM::set( vm, -3 );

   VM::push( vm, "collidingEntity" );
   VM::pushProxy( vm, _colliding.ptr() );
   VM::set( vm, -3 );

   VM::push( vm, "location" );
   VM::push( vm, _location );
   VM::set( vm, -3 );

   VM::push( vm, "force" );
   VM::push( vm, _force );
   VM::set( vm, -3 );
}

//------------------------------------------------------------------------------
//!
bool
ContactBeginStimulus::to( VMState* vm, int idx )
{
   bool ok;
   ok  = VM__get( vm, idx, "receivingEntity", _receiving );
   ok |= VM__get( vm, idx, "collidingEntity", _colliding );
   ok |= VM::get( vm, idx, "location", _location );
   ok |= VM::get( vm, idx, "force", _force );
   return ok;
}


/*==============================================================================
  CLASS ContactEndStimulus
==============================================================================*/

//------------------------------------------------------------------------------
//!
const ConstString&
ContactEndStimulus::type() const
{
   return _sType_ContactEnd;
}

//------------------------------------------------------------------------------
//!
void
ContactEndStimulus::push( VMState* vm )
{
   Stimulus::push( vm );

   VM::push( vm, "receivingEntity" );
   VM::pushProxy( vm, _receiving.ptr() );
   VM::set( vm, -3 );

   VM::push( vm, "collidingEntity" );
   VM::pushProxy( vm, _colliding.ptr() );
   VM::set( vm, -3 );

}

//------------------------------------------------------------------------------
//!
bool
ContactEndStimulus::to( VMState* vm, int idx )
{
   bool ok;
   ok  = VM__get( vm, idx, "receivingEntity", _receiving );
   ok |= VM__get( vm, idx, "collidingEntity", _colliding );
   return ok;
}


/*==============================================================================
  CLASS FallStimulus
==============================================================================*/

//------------------------------------------------------------------------------
//!
const ConstString&
FallStimulus::type() const
{
   return _sType_Fall;
}


/*==============================================================================
  CLASS LandStimulus
==============================================================================*/

//------------------------------------------------------------------------------
//!
const ConstString&
LandStimulus::type() const
{
   return _sType_Land;
}

NAMESPACE_END
