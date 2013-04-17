/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/World/Entity.h>
#include <Plasma/World/World.h>
#include <Plasma/World/WorldVM.h>

#include <Fusion/VM/VMRegistry.h>

#if MOTION_BULLET
#include <MotionBullet/Collision/BasicShapes.h>
#else
#include <Motion/Collision/BasicShapes.h>
#endif


USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

const char* _entity_str = "entity";

UNNAMESPACE_END


/*==============================================================================
  CLASS Entity
==============================================================================*/

//------------------------------------------------------------------------------
//!
void Entity::initialize()
{
   VMRegistry::add( _entity_str, NULL, entity_get, entity_set, VM_CAT_APP );
   VMRegistry::add( _entity_str, NULL, entity_get_brains, entity_set_brains, VM_CAT_BRAIN );  // Read-only in brain programs.
}

//------------------------------------------------------------------------------
//!
Entity::Entity( Type type )
   : _world( NULL ),
     _type( type ),
     _state( VISIBILE | CASTS_SHADOWS ),
     _referential( Reff::identity() ),
     _transform( Mat4f::identity() )
{
}

//------------------------------------------------------------------------------
//!
Entity::~Entity()
{}

//------------------------------------------------------------------------------
//!
void Entity::referential( const Reff& ref )
{
   _referential = ref;
   _transform   = _referential.localToGlobal();
   _positionSubject.notify();
}

//------------------------------------------------------------------------------
//!
void Entity::position( const Vec3f& pos )
{
   _referential.position( pos );
   _transform = _referential.localToGlobal();
   _positionSubject.notify();
}

//------------------------------------------------------------------------------
//!
void Entity::orientation( const Quatf& orient )
{
   _referential.orientation( orient );
   _transform = _referential.localToGlobal();
   _positionSubject.notify();
}

//------------------------------------------------------------------------------
//!
void Entity::scale( float s )
{
   _referential.scale( s );
   _transform = _referential.localToGlobal();
   _positionSubject.notify();
}

//------------------------------------------------------------------------------
//!
void Entity::lookAt( const Vec3f& at, const Vec3f& up )
{
   _referential = Reff::lookAt( position(), at, up );
   _transform   = _referential.localToGlobal();
   _positionSubject.notify();
}

//------------------------------------------------------------------------------
//!
void Entity::rotate( const Vec3f& pos, const Vec3f axis, float angle )
{
   _referential.rotate( pos, axis, angle );
   _transform = _referential.localToGlobal();
   _positionSubject.notify();
}

//------------------------------------------------------------------------------
//!
void Entity::connect( World* w )
{
   _world = w;
}

//------------------------------------------------------------------------------
//!
void Entity::disconnect()
{
   _positionSubject.detachAll();
   _world = nullptr;
}

//------------------------------------------------------------------------------
//!
void Entity::id( const ConstString& str )
{
   if( str == _id ) return;
   if( _world ) _world->updateID( this, str );
   _id = str;
}

//------------------------------------------------------------------------------
//!
void Entity::geometry( Geometry* geom )
{
   _geometry = geom;
}

//------------------------------------------------------------------------------
//!
void Entity::materialSet( MaterialSet* mat )
{
   _materials = mat;
}

//-----------------------------------------------------------------------------
//!
void Entity::stimulate( Stimulus* s )
{
   if( brain() )
   {
      brain()->_stimuli.post( s );
      world()->scheduleAnalyze( this );
   }
   else
   {
      RCP<Stimulus> rcp = s; // Prevent leak for e->stimulate( new Stimulus() );
   }
}

//------------------------------------------------------------------------------
//!
const char*
Entity::meta() const
{
   return _entity_str;
}
