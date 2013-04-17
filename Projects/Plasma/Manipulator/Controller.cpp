/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Manipulator/Controller.h>

#include <Plasma/Intersector.h>
#include <Plasma/Stimulus/Orders.h>
#include <Plasma/World/Entity.h>
#include <Plasma/World/RigidEntity.h>

#include <Fusion/VM/VMObjectPool.h>

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
   ATTRIB_ENTITY,
   NUM_ATTRIBS
};

//------------------------------------------------------------------------------
//!
StringMap _entityAttr(
   "entity",  ATTRIB_ENTITY,
   ""
);

UNNAMESPACE_END


/*==============================================================================
  CLASS Controller
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
Controller::initialize()
{
   VMObjectPool::registerCreate(
      "UI",
      "EntityController",
      stdCreateVM<EntityController>
   );
}

//-----------------------------------------------------------------------------
//!
Controller::Controller()
{
}

//-----------------------------------------------------------------------------
//!
Controller::~Controller()
{
   disableHID();
}

#if 0
//-----------------------------------------------------------------------------
//!
void
Controller::onCameraChange()
{
}

//-----------------------------------------------------------------------------
//!
void
Controller::onViewportChange()
{
}
#endif

//-----------------------------------------------------------------------------
//!
bool
Controller::onPointerPress( const Event& ev )
{
   return _dispatcher.dispatch( ev );
}

//-----------------------------------------------------------------------------
//!
bool
Controller::onPointerRelease( const Event& ev )
{
   return _dispatcher.dispatch( ev );
}

//-----------------------------------------------------------------------------
//!
bool
Controller::onPointerMove( const Event& ev )
{
   return _dispatcher.dispatch( ev );
}

//-----------------------------------------------------------------------------
//!
bool
Controller::onPointerCancel( const Event& ev )
{
   return _dispatcher.dispatch( ev );
}

//-----------------------------------------------------------------------------
//!
bool
Controller::onPointerScroll( const Event& ev )
{
   return _dispatcher.dispatch( ev );
}

//-----------------------------------------------------------------------------
//!
bool
Controller::onKeyPress( const Event& ev )
{
   return _dispatcher.dispatch( ev );
}

//-----------------------------------------------------------------------------
//!
bool
Controller::onKeyRelease( const Event& ev )
{
   return _dispatcher.dispatch( ev );
}

//-----------------------------------------------------------------------------
//!
bool
Controller::onChar( const Event& ev )
{
   return _dispatcher.dispatch( ev );
}

//-----------------------------------------------------------------------------
//!
bool
Controller::onAccelerate( const Event& ev )
{
   return _dispatcher.dispatch( ev );
}

//-----------------------------------------------------------------------------
//!
void
Controller::enableHID()
{
   Core::addOnHID( makeDelegate(this, &EntityController::onHIDEvent) );
}

//-----------------------------------------------------------------------------
//!
void
Controller::disableHID()
{
   Core::removeOnHID( makeDelegate(this, &EntityController::onHIDEvent) );
}

//-----------------------------------------------------------------------------
//!
void
Controller::onHIDEvent( const Event& ev )
{
   _dispatcher.dispatch( ev );
}


/*==============================================================================
  CLASS EntityController
==============================================================================*/

//-----------------------------------------------------------------------------
//!
EntityController::EntityController():
   _entity(NULL)
{
   dispatcher().onPointerPress( NULL, makeDelegate(this, &EntityController::entityPickEvent) );
   dispatcher().onEvent( NULL, makeDelegate(this, &EntityController::anyEvent) );
   enableHID();
}

//-----------------------------------------------------------------------------
//!
EntityController::~EntityController()
{
}

//-----------------------------------------------------------------------------
//!
void
EntityController::entityPickEvent()
{
   const Vec2f& p = Core::currentEvent().position();
   Intersector::Hit  hit;
   Rayf ray;
   ray.origin( camera()->position() );
   ray.direction( viewport()->direction(p) );
   if( Intersector::trace(world(), ray, hit) )
   {
      //StdErr << "Hit: " << (void*)hit._entity << " type=" << hit._entity->type() << nl;
      if( hit._entity && hit._entity->type() == Entity::RIGID )
      {
         RigidEntity* re = (RigidEntity*)hit._entity;
         if( !re->isStatic() )
         {
            if( re == _entity )
            {
               _entity->stimulate( new StopOrder() );
            }
            else
            {
               _entity = re;
            }
            return;
         }
      }

      if( _entity )
      {
         Vec3f p = hit._pos;
         p.y = _entity->position().y;
         _entity->stimulate( new GoToOrder( p ) );
      }
   }
   //else
   //{
   //   StdErr << "Nothing" << nl;
   //}
}

//-----------------------------------------------------------------------------
//!
void
EntityController::anyEvent() const
{
   StdErr << "AnyEvent: " << Core::currentEvent().toStr() << nl;
}

//-----------------------------------------------------------------------------
//!
void
EntityController::onViewportChange()
{
   _entity = NULL;
}

//-----------------------------------------------------------------------------
//!
bool
EntityController::performGet( VMState* vm )
{
   const char* key = VM::toCString( vm, -1 );
   switch( _entityAttr[key] )
   {
      case ATTRIB_ENTITY:
         VM::pushProxy( vm, _entity );
         return true;
      default:
         break;
   }
   return Controller::performGet( vm );
}

//------------------------------------------------------------------------------
//!
bool
EntityController::performSet( VMState* vm )
{
   const char* key = VM::toCString( vm, -2 );
   switch( _entityAttr[key] )
   {
      case ATTRIB_ENTITY:
         entity( (RigidEntity*)VM::toProxy( vm, -1 ) );
         return true;
      default:
         break;
   }
   return Controller::performSet( vm );
}
