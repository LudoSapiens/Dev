/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/World/SkeletalEntity.h>
#include <Plasma/Animation/SkeletalAnimation.h>
#include <Plasma/World/World.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN


UNNAMESPACE_END


/*==============================================================================
  CLASS SkeletalEntity
==============================================================================*/


NAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
SkeletalEntity::SkeletalEntity():
   Entity( SKELETAL )
{
   _body = new RigidBody( RigidBody::DYNAMIC, (Entity*)this );
   _body->referential( referential() );
}

//------------------------------------------------------------------------------
//!
SkeletalEntity::~SkeletalEntity()
{
   CHECK( world() == NULL );
}

//------------------------------------------------------------------------------
//!
void
SkeletalEntity::referential( const Reff& ref )
{
   Entity::referential( ref );
   _body->referential( ref );
}

//------------------------------------------------------------------------------
//!
void
SkeletalEntity::position( const Vec3f& pos )
{
   Entity::position( pos );
   _body->referential( _referential );
}

//------------------------------------------------------------------------------
//!
void
SkeletalEntity::orientation( const Quatf& orient )
{
   Entity::orientation( orient );
   _body->referential( _referential );
}

//------------------------------------------------------------------------------
//!
void 
SkeletalEntity::lookAt( const Vec3f& at, const Vec3f& up )
{
   Entity::lookAt( at, up );
   _body->referential( _referential );
}

//------------------------------------------------------------------------------
//! 
void 
SkeletalEntity::rotate( const Vec3f& pos, const Vec3f axis, float angle )
{
   Entity::rotate( pos, axis, angle );
   _body->referential( _referential );
}

//------------------------------------------------------------------------------
//! 
void
SkeletalEntity::geometry( Geometry* geom )
{
   _geometry = geom;
   if( _body.isValid() )
   {
      _body->shape( geom->collisionShape() );
   }
   if( geom )
   {
      _skeleton.skeleton( geom->skeleton() );
   }
}

//------------------------------------------------------------------------------
//!
void
SkeletalEntity::connect( World* w )
{
   Entity::connect( w );
   _constraint = world()->motionWorld()->createCharacterConstraint( _body.ptr() );
   world()->motionWorld()->addBody( _body.ptr() );
}


//------------------------------------------------------------------------------
//!
void
SkeletalEntity::disconnect()
{
   if( world() != NULL )
   {
      world()->motionWorld()->removeBody( _body.ptr() );
      world()->motionWorld()->removeConstraint( _constraint.ptr() );
      _constraint = NULL;
      Entity::disconnect();
   }
   else
   {
      printf("ERROR - Disconnected skeletal entity twice!\n");
   }
}

NAMESPACE_END
