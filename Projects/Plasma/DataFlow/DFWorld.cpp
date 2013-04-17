/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/DataFlow/DFWorld.h>
#include <Plasma/World/World.h>
#include <Plasma/Resource/ResManager.h>

NAMESPACE_BEGIN

/*==============================================================================
   CLASS DFEntity
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFEntity::DFEntity():
   _bodyType( RigidBody::STATIC ),
   _mass( 1.0f ),
   _friction( 0.5f ),
   _restitution( 0.5f ),
   _exists(1),
   _senses(0),
   _referential( Reff::identity() ),
   _visible( true ),
   _castsShadows( true ),
   _ghost( false )
{
}

//------------------------------------------------------------------------------
//!
RCP<Entity>
DFEntity::createEntity()
{
   // TODO: SkeletalEntity if geom contains a skeleton...
   RCP<RigidEntity> e = new RigidEntity( (RigidBody::Type)_bodyType );

   // Entity parameters.
   initEntity( e.ptr() );

   return e;
}

//------------------------------------------------------------------------------
//!
void
DFEntity::initEntity( RigidEntity* e )
{
   // Id.
   e->id( _id );

   // Geometry.
   if( _geom.isValid() )
   {
      _geom->updateMesh();
      e->geometry( _geom.ptr() );
   }
   // Position and orientation.
   e->referential( _referential );
   // Flags.
   e->visible( visible() );
   e->castsShadows( castsShadows() );
   e->ghost( ghost() );
   // Phyiscal attributes.
   e->mass( _mass );
   e->friction( _friction );
   e->restitution( _restitution );
   e->exists( _exists );
   e->senses( _senses );
   // User attributes.
   e->attributes( _attributes.ptr() );
   // Brain.
   if( !_brainProg.isNull() )
   {
      e->brain( new Brain() );
      RCP< Resource<BrainProgram> > progRes = ResManager::getBrainProgram( _brainProg.cstr(), nullptr );
      RCP< BrainProgram > prog;
      if( progRes.isNull() )
      {
         prog = new BrainProgram();
         ResManager::registerBrainProgram( _brainProg.cstr(), prog.ptr() );
      }
      else
      {
         while( !progRes->isReady() );
         prog = data( progRes );
      }

      e->brain()->program( prog.ptr() );
   }

   // Material.
   if( !_material.isNull() )
   {
      RCP< Resource<MaterialSet> > matRes = ResManager::newMaterialSet( _material.cstr(), nullptr );
      if( matRes.isValid() )
      {
         while( !matRes->isReady() );
         e->materialSet( data( matRes ) );
      }
   }
}

/*==============================================================================
   CLASS DFCamera
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFCamera::DFCamera():
   _focalLength( 2.0f ),
   _fov( 40.0f ),
   _back( 10000.0f ),
   _front( 0.1f ),
   _orthoScale( 10.0f ),
   _shear( 0.0f ),
   _projType( Camera::PERSPECTIVE ),
   _fovMode( Camera::LARGEST )
{
}

//------------------------------------------------------------------------------
//!
RCP<Entity>
DFCamera::createEntity()
{
   RCP<Camera> cam = new Camera( (RigidBody::Type)_bodyType );

   // Entity parameters.
   initEntity( cam.ptr() );

   // Camera parameters.
   cam->projection( _projType );
   cam->focalLength( _focalLength );
   cam->fov( _fov );
   cam->fovMode( _fovMode );
   cam->frontAndBack( _front, _back );
   cam->orthoScale( _orthoScale );
   cam->shear( _shear.x, _shear.y );

   return cam;
}

/*==============================================================================
   CLASS DFLight
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFLight::DFLight():
   _shape( Light::POINT ),
   _intensity( 1.0f ),
   _front( 0.1f ), _back( 500.0f ),
   _fov( 90.0f )
{
}

//------------------------------------------------------------------------------
//!
RCP<Entity>
DFLight::createEntity()
{
   RCP<Light> light = new Light( (RigidBody::Type)_bodyType );

   // Entity parameters.
   initEntity( light.ptr() );

   // Light parameters.
   light->shape( _shape );
   light->intensity( _intensity );
   light->fov( _fov );
   light->frontAndBack( _front, _back );

   return light;
}

/*==============================================================================
   CLASS DFWorld
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFWorld::DFWorld()
{
}

//------------------------------------------------------------------------------
//!
RCP<World>
DFWorld::createWorld()
{
   RCP<World> w = new World();

   for( auto e = _entities.begin(); e != _entities.end(); ++e )
   {
      w->addEntity( (*e)->createEntity().ptr() );
   }

   for( auto p = _probes.begin(); p != _probes.end(); ++p )
   {
      w->addProbe( (*p)->createProbe().ptr() );
   }

   return w;
}

//------------------------------------------------------------------------------
//!
RCP<DFWorld>
DFWorld::clone() const
{
   RCP<DFWorld> w = new DFWorld();
   w->_entities = _entities;
   w->_probes   = _probes;
   return w;
}

NAMESPACE_END
