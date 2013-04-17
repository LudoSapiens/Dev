/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/World/WorldVM.h>

#include <Plasma/Animation/AnimationGraph.h>
#include <Plasma/Particle/BaseParticles.h>
#include <Plasma/Particle/ParticleAnimator.h>
#include <Plasma/Particle/ParticleGenerator.h>
#include <Plasma/Particle/Spark.h>
#include <Plasma/Stimulus/Orders.h>
#include <Plasma/World/World.h>
#include <Plasma/World/Entity.h>
#include <Plasma/World/Camera.h>
#include <Plasma/World/Light.h>
#include <Plasma/World/ParticleEntity.h>
#include <Plasma/World/ProxyEntity.h>
#include <Plasma/World/Receptor.h>
#include <Plasma/World/SkeletalEntity.h>
#include <Plasma/Resource/ResManager.h>
#include <Plasma/Procedural/ProceduralWorld.h>

#include <Base/ADT/StringMap.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

// World attributes.
enum {
   ATTRIB_ADD_CAMERA,
   ATTRIB_ADD_LIGHT,
   ATTRIB_ADD_OBJECT,
   ATTRIB_ADD_PROXY,
   ATTRIB_ADD_SKELETAL_OBJECT,
   ATTRIB_ADD_STATIC_OBJECT,
   ATTRIB_BACKGROUND_COLOR,
   ATTRIB_CAMERA,
   ATTRIB_ENTITY,
   ATTRIB_GET_ATTRIBUTE,
   ATTRIB_GRAVITY,
   ATTRIB_LIGHT,
   ATTRIB_NUM_CAMERAS,
   ATTRIB_NUM_ENTITIES,
   ATTRIB_NUM_LIGHTS,
   ATTRIB_SET_ATTRIBUTE,
   ATTRIB_SIMULATION_SPEED,
   ATTRIB_START_SIMULATION,
   ATTRIB_STOP_SIMULATION,
   ATTRIB_TIME
};

StringMap _worldAttributes(
   "addCamera",         ATTRIB_ADD_CAMERA,
   "addLight",          ATTRIB_ADD_LIGHT,
   "addObject",         ATTRIB_ADD_OBJECT,
   "addProxy",          ATTRIB_ADD_PROXY,
   "addSkeletalObject", ATTRIB_ADD_SKELETAL_OBJECT,
   "addStaticObject",   ATTRIB_ADD_STATIC_OBJECT,
   "backgroundColor",   ATTRIB_BACKGROUND_COLOR,
   "camera",            ATTRIB_CAMERA,
   "entity",            ATTRIB_ENTITY,
   "getAttribute",      ATTRIB_GET_ATTRIBUTE,
   "gravity",           ATTRIB_GRAVITY,
   "light",             ATTRIB_LIGHT,
   "numCameras",        ATTRIB_NUM_CAMERAS,
   "numEntities",       ATTRIB_NUM_ENTITIES,
   "numLights",         ATTRIB_NUM_LIGHTS,
   "setAttribute",      ATTRIB_SET_ATTRIBUTE,
   "simulationSpeed",   ATTRIB_SIMULATION_SPEED,
   "startSimulation",   ATTRIB_START_SIMULATION,
   "stopSimulation",    ATTRIB_STOP_SIMULATION,
   "time",              ATTRIB_TIME,
   ""
);

// Entity attributes.
enum {
   ATTRIB_ATTRIBUTE,
   ATTRIB_BRAIN,
   ATTRIB_POSITION,
   ATTRIB_ORIENTATION,
   ATTRIB_SCALE,
   ATTRIB_VISIBLE,
   ATTRIB_CASTS_SHADOWS,
   ATTRIB_GHOST,
   ATTRIB_GEOMETRY,
   ATTRIB_SET_GEOMETRY,
   ATTRIB_MATERIAL,
   ATTRIB_SET_MATERIAL,
   ATTRIB_STIMULATE,
};

StringMap _entityAttributes(
   "attribute",    ATTRIB_ATTRIBUTE,
   "brain",        ATTRIB_BRAIN,
   "position",     ATTRIB_POSITION,
   "orientation",  ATTRIB_ORIENTATION,
   "scale",        ATTRIB_SCALE,
   "visible",      ATTRIB_VISIBLE,
   "castsShadows", ATTRIB_CASTS_SHADOWS,
   "ghost",        ATTRIB_GHOST,
   "geometry",     ATTRIB_GEOMETRY,
   "setGeometry",  ATTRIB_SET_GEOMETRY,
   "material",     ATTRIB_MATERIAL,
   "setMaterial",  ATTRIB_SET_MATERIAL,
   "stimulate",    ATTRIB_STIMULATE,
   ""
);

// RigidEntity attributes.
enum {
   ATTRIB_MASS_R,
   ATTRIB_FRICTION_R,
   ATTRIB_RESTITUTION_R,
   ATTRIB_EXISTS_R,
   ATTRIB_SENSES_R,
   ATTRIB_ATTRACTION_CATEGORIES_R,
   ATTRIB_LINEAR_VELOCITY_R,
   ATTRIB_ANGULAR_VELOCITY_R
};

StringMap _rigidAttributes(
   "mass",                    ATTRIB_MASS_R,
   "friction",                ATTRIB_FRICTION_R,
   "restitution",             ATTRIB_RESTITUTION_R,
   "exists",                  ATTRIB_EXISTS_R,
   "senses",                  ATTRIB_SENSES_R,
   "attractionCategories",    ATTRIB_ATTRACTION_CATEGORIES_R,
   "linearVelocity",          ATTRIB_LINEAR_VELOCITY_R,
   "angularVelocity",         ATTRIB_ANGULAR_VELOCITY_R,
   ""
);

// Camera attributes.
enum {
   ATTRIB_BACK_C,
   ATTRIB_FOCAL_C,
   ATTRIB_FOV_C,
   ATTRIB_FOV_MODE_C,
   ATTRIB_FRONT_C,
   ATTRIB_ORTHO_SCALE_C,
   ATTRIB_PROJECTION_C
};

StringMap _camAttributes(
   "back",       ATTRIB_BACK_C,
   "focal",      ATTRIB_FOCAL_C,
   "fov",        ATTRIB_FOV_C,
   "fovMode",    ATTRIB_FOV_MODE_C,
   "front",      ATTRIB_FRONT_C,
   "orthoScale", ATTRIB_ORTHO_SCALE_C,
   "projection", ATTRIB_PROJECTION_C,
   ""
);

// Light attributes.
enum {
   ATTRIB_BACK_L,
   ATTRIB_FOV_L,
   ATTRIB_FRONT_L,
   ATTRIB_INTENSITY_L,
   ATTRIB_SHAPE_L
};

StringMap _lightAttributes(
   "back",      ATTRIB_BACK_L,
   "fov",       ATTRIB_FOV_L,
   "front",     ATTRIB_FRONT_L,
   "intensity", ATTRIB_INTENSITY_L,
   "shape",     ATTRIB_SHAPE_L,
   ""
);

// ProxyEntity attributes.
enum{
   ATTRIB_ENTITY_PROXY
};

StringMap _proxyAttributes(
   "entity", ATTRIB_ENTITY_PROXY,
   ""
);

// Geometry attributes.
enum {
   ATTRIB_STATE_G
};

StringMap _geomAttributes(
   "state", ATTRIB_STATE_G,
   ""
);

// Receptor attributes.
enum {
   RECEPTOR_ATTRIB_COLLIDING_GROUPS,
   RECEPTOR_ATTRIB_RECEIVING_ENTITY,
   RECEPTOR_ATTRIB_RECEIVING_GROUPS,
   RECEPTOR_ATTRIB_TYPE
};

StringMap _receptorAttributes(
   "collidingGroups",  RECEPTOR_ATTRIB_COLLIDING_GROUPS,
   "receivingEntity",  RECEPTOR_ATTRIB_RECEIVING_ENTITY,
   "receivingGroups",  RECEPTOR_ATTRIB_RECEIVING_GROUPS,
   "type"           ,  RECEPTOR_ATTRIB_TYPE,
   ""
);

//------------------------------------------------------------------------------
//!
void initEntity( VMState* vm, Entity* e, WorldContext* context )
{
   Reff ref = Reff::identity();

   // ID.
   const char* idstr = nullptr;
   if( VM::get( vm, -1, "id", idstr ) ) e->id( ConstString( idstr ) );

   // Position.
   VM::get( vm, -1, "position", ref.position() );

   // Orientation.
   Quatf q = Quatf::identity();
   VM::get( vm, -1, "orientation", q );
   ref.orientation( q );

   // Look at.
   if( VM::get( vm, -1, "lookAt" ) )
   {
      VM::geti( vm, -1, 1 );
      Vec3f at = VM::toVec3f( vm, -1 );
      VM::geti( vm, -2, 2 );
      Vec3f up = VM::toVec3f( vm, -1 );
      VM::pop( vm, 3 );
      ref = Reff::lookAt( ref.position(), at, up );
   }

   // Scaling.
   VM::get( vm, -1, "scale", ref.scale() );

   // Visibility states.
   bool state;
   if( VM::get( vm, -1, "visible",      state ) ) e->visible( state );
   if( VM::get( vm, -1, "castsShadows", state ) ) e->castsShadows( state );
   if( VM::get( vm, -1, "ghost",        state ) ) e->ghost( state );

   // Initializing the resources.
   if( context )
   {
      // Update entity referential.
      e->referential( context->_ref * ref );

      // Geometry.
      if( VM::get( vm, -1, "geometry" ) )
      {
         if( VM::isObject( vm, -1 ) )
         {
            context->setGeometry( e, (Resource<Geometry>*)VM::toPtr( vm, -1 ) );
         }
         else
         {
            RCP< Resource<Geometry> > res = ResManager::getGeometry( VM::toString( vm, -1 ), context->task() );
            context->keepResource( res.ptr() );
            context->setGeometry( e, res.ptr() );
         }
         VM::pop( vm );
      }

      // Material.
      if( VM::get( vm, -1, "material" ) )
      {
         if( VM::isObject( vm, -1 ) )
         {
            context->setMaterialSet( e, (Resource<MaterialSet>*)VM::toPtr( vm, -1 ) );
         }
         else
         {
            RCP< Resource<MaterialSet> > res = ResManager::newMaterialSet( VM::toString( vm, -1 ), context->task() );
            context->keepResource( res.ptr() );
            context->setMaterialSet( e, res.ptr() );
         }
         VM::pop( vm );
      }

      // Brain.
      if( VM::get( vm, -1, "brain" ) )
      {
         if( VM::isObject( vm, -1 ) )
         {
            e->brain( (Brain*)VM::toPtr( vm, -1 ) );
         }
         else
         {
            StdErr << "WorldVM::initEntity() - Brains should now call the brain() function." << nl;
         }
         VM::pop( vm, 1 );
      }
   }
   else
   {
      // Update entity referential.
      e->referential( ref );

      // Geometry.
      if( VM::get( vm, -1, "geometry" ) )
      {
         if( VM::isObject( vm, -1 ) )
         {
            e->geometry( (Geometry*)VM::toProxy( vm, -1 ) );
         }
         //else
         //{
         //   e->geometry( ResManager::getGeometry( VM::toString( vm, -1 ), NULL ) );
         //}
         VM::pop( vm );
      }

      // Material.
      if( VM::get( vm, -1, "material" ) )
      {
         if( VM::isObject( vm, -1 ) )
         {
            // TODO
         }
         //else
         //{
         //   e->materialSet( ResManager::newMaterialSet( VM::toString( vm, -1 ), NULL ).ptr() );
         //}
         VM::pop( vm );
      }

      // Brain.
      if( VM::get( vm, -1, "brain" ) )
      {
         /*
         if( VM::isObject( vm, -1 ) )
         {
            e->brain( (Brain*)VM::toPtr( vm, -1 ) );
         }
         else
         {
            StdErr << "WorldVM::initEntity() - Brains should now call the brain() function." << nl;
         }

         if( e->brain() != NULL )
         {
            e->world()->scheduleAnalyze( e ); // TEMP.
         }
         */
         VM::pop( vm, 1 );
      }
   }

   // User attributes.
   if( VM::get( vm, -1, "attributes" ) )
   {
      Table* attr = new Table();
      VM::toTable( vm, -1, *attr );
      VM::pop( vm );
      e->attributes( attr );
   }
}

//------------------------------------------------------------------------------
//!
void initRigid( VMState* vm, RigidEntity* e )
{
   // Physics.
   float f;
   if( VM::get( vm, -1, "mass"       , f ) )  e->mass( f );
   if( VM::get( vm, -1, "friction"   , f ) )  e->friction( f );
   if( VM::get( vm, -1, "restitution", f ) )  e->restitution( f );

   uint u;
   if( VM::get( vm, -1, "exists", u ) )                e->exists( u );
   if( VM::get( vm, -1, "senses", u ) )                e->senses( u );
   if( VM::get( vm, -1, "attractionCategories", u ) )  e->attractionCategories( u );

   Vec3f v;
   if( VM::get( vm, -1, "linearVelocity" , v ) )  e->linearVelocity( v );
   if( VM::get( vm, -1, "angularVelocity", v ) )  e->angularVelocity( v );
}

//------------------------------------------------------------------------------
//!
void initSkeletal( VMState* vm, SkeletalEntity* e, WorldContext* context )
{
   // Physics.
   float f;
   if( VM::get( vm, -1, "mass"       , f ) )  e->mass( f );
   if( VM::get( vm, -1, "friction"   , f ) )  e->friction( f );
   if( VM::get( vm, -1, "restitution", f ) )  e->restitution( f );

   uint u;
   if( VM::get( vm, -1, "exists", u ) )                e->exists( u );
   if( VM::get( vm, -1, "senses", u ) )                e->senses( u );
   if( VM::get( vm, -1, "attractionCategories", u ) )  e->attractionCategories( u );

   Vec3f v;
   if( VM::get( vm, -1, "linearVelocity" , v ) )  e->linearVelocity( v );
   if( VM::get( vm, -1, "angularVelocity", v ) )  e->angularVelocity( v );

   bool  b;
   if( VM::get( vm, -1, "constrainLinearVelocity", b ) )  e->constrainLinearVelocity( b );

   // Initializing the resources.
   if( context )
   {
      // Animation graph.
      if( VM::get( vm, -1, "graph" ) )
      {
         if( VM::isObject( vm, -1 ) )
         {
            context->setAnimGraph( e, (Resource<AnimationGraph>*)VM::toPtr( vm, -1 ) );
         }
         else
         {
            RCP< Resource<AnimationGraph> > res = ResManager::getAnimationGraph( VM::toString( vm, -1 ), context->task() );
            context->keepResource( res.ptr() );
            context->setAnimGraph( e, res.ptr() );
         }
         VM::pop( vm );
      }
   }
}

//------------------------------------------------------------------------------
//!
void initParticle( VMState* vm, ParticleEntity* e, WorldContext* context )
{
   // Initializing the resources.
   if( context )
   {
      // Generator and animator.
      if( VM::get( vm, -1, "generator" ) )
      {
         String name = VM::toString( vm, -1 );
         if( name == "spark" )
         {
            e->generator( new SparkGenerator() );
         }
         else
         if( name == "line" )
         {
            e->generator( new LineGenerator() );
         }
         else
         {
            StdErr << "ERROR: Unknown particle generator '" << name << "'." << nl;
         }

         VM::pop( vm );
      }
      if( VM::get( vm, -1, "animator" ) )
      {
         String name = VM::toString( vm, -1 );
         if( name == "spark" )
         {
            e->animator( new SparkAnimator() );
         }
         else
         if( name == "line" )
         {
            e->animator( new LineAnimator() );
         }
         else
         {
            StdErr << "ERROR: Unknown particle animator '" << name << "'." << nl;
         }
         VM::pop( vm );
      }
   }

   uint32_t u = 0;
   if( !VM::get( vm, -1, "capacity", u ) )
   {
      StdErr << "ERROR: Missing capacity in ParticleEntity." << nl;
   }

   e->init( u );
}

//------------------------------------------------------------------------------
//!
int startSimulationVM( VMState* vm )
{
   World* w = (World*)VM::thisPtr(vm);
   w->startSimulation();
   return 0;
}

//------------------------------------------------------------------------------
//!
int stopSimulationVM( VMState* vm )
{
   World* w = (World*)VM::thisPtr(vm);
   w->stopSimulation();
   return 0;
}

//------------------------------------------------------------------------------
//!
int addCameraVM( VMState* vm )
{
   World* w  = (World*)VM::thisPtr(vm);
   Camera* e = new Camera( RigidBody::DYNAMIC );
   initCamera( vm, e, NULL );
   w->addEntity( e );
   VM::pushProxy( vm, e );
   return 1;
}

//------------------------------------------------------------------------------
//!
int addLightVM( VMState* vm )
{
   World* w  = (World*)VM::thisPtr(vm);
   Light* e = new Light( RigidBody::DYNAMIC );
   initLight( vm, e, NULL );
   w->addEntity( e );
   VM::pushProxy( vm, e );
   return 1;
}

//------------------------------------------------------------------------------
//!
int addObjectVM( VMState* vm )
{
   World* w       = (World*)VM::thisPtr(vm);
   RigidEntity* e = new RigidEntity( RigidBody::DYNAMIC );
   initRigidEntity( vm, e, NULL );
   w->addEntity( e );
   VM::pushProxy( vm, e );
   return 1;
}

//------------------------------------------------------------------------------
//!
int addProxyVM( VMState* vm )
{
   World* w       = (World*)VM::thisPtr(vm);
   ProxyEntity* e = new ProxyEntity();
   initProxyEntity( vm, e, NULL );
   w->addEntity( e );
   VM::pushProxy( vm, e );
   return 1;
}

//------------------------------------------------------------------------------
//!
int addSkeletalObjectVM( VMState* vm )
{
   World* w          = (World*)VM::thisPtr(vm);
   SkeletalEntity* e = new SkeletalEntity();
   initSkeletalEntity( vm, e, NULL );
   w->addEntity( e );
   VM::pushProxy( vm, e );
   return 1;
}

//------------------------------------------------------------------------------
//!
int addStaticObjectVM( VMState* vm )
{
   World* w       = (World*)VM::thisPtr(vm);
   RigidEntity* e = new RigidEntity( RigidBody::STATIC );
   initRigidEntity( vm, e, NULL );
   w->addEntity( e );
   VM::pushProxy( vm, e );
   return 1;
}

//------------------------------------------------------------------------------
//!
int cameraVM( VMState* vm )
{
   World* w = (World*)VM::thisPtr(vm);
   switch( VM::type( vm, 1 ) )
   {
      case VM::NUMBER:
      {
         int idx = VM::toInt( vm, 1 ) - 1;
         VM::pushProxy( vm, w->camera(idx) );
         return 1;
      }
      case VM::STRING:
      {
         ConstString id = VM::toConstString( vm, 1 );
         VM::pushProxy( vm, w->camera(id) );
         return 1;
      }
      default:
         break;
   }
   return 0;
}

//------------------------------------------------------------------------------
//!
int entityVM( VMState* vm )
{
   World* w = (World*)VM::thisPtr(vm);
   switch( VM::type( vm, 1 ) )
   {
      case VM::NUMBER:
      {
         int idx = VM::toInt( vm, 1 ) - 1;
         VM::pushProxy( vm, w->entity(idx) );
         return 1;
      }
      case VM::STRING:
      {
         ConstString id = VM::toConstString( vm, 1 );
         VM::pushProxy( vm, w->entity(id) );
         return 1;
      }
      default:
         break;
   }
   return 0;
}

//------------------------------------------------------------------------------
//!
int lightVM( VMState* vm )
{
   World* w = (World*)VM::thisPtr(vm);
   switch( VM::type( vm, 1 ) )
   {
      case VM::NUMBER:
      {
         int idx = VM::toInt( vm, 1 ) - 1;
         VM::pushProxy( vm, w->light(idx) );
         return 1;
      }
      case VM::STRING:
      {
         ConstString id = VM::toConstString( vm, 1 );
         VM::pushProxy( vm, w->light(id) );
         return 1;
      }
      default:
         break;
   }
   return 0;
}

//------------------------------------------------------------------------------
//!
int getAttributeVM( VMState* vm )
{
   World* w = (World*)VM::thisPtr(vm);

   if( VM::getTop( vm ) != 1 )
   {
      StdErr << "WorldVM::getAttributeVM() - Wrong number of arguments: expected 1, got " << VM::getTop( vm ) << "." << nl;
      return 0;
   }

   switch( VM::type( vm, 1 ) )
   {
      case VM::NUMBER:
      {
         uint idx = VM::toUInt( vm, 1 );
         const Variant& v = w->attributes().get( idx );
         VM::push( vm, v );
         return 1;
      }  break;
      case VM::STRING:
      {
         ConstString key = VM::toCString( vm, 1 );
         const Variant& v = w->attributes().get( key );
         VM::push( vm, v );
         return 1;
      }  break;
      default:
      {
         StdErr << "WorldVM::getAttributeVM() - Invalid key type: " << VM::type( vm, 1 ) << "." << nl;
      }  break;
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int setAttributeVM( VMState* vm )
{
   World* w = (World*)VM::thisPtr(vm);

   if( VM::getTop( vm ) != 2 )
   {
      StdErr << "WorldVM::setAttributeVM() - Wrong number of arguments: expected 2, got " << VM::getTop( vm ) << "." << nl;
      return 0;
   }

   VM::toVariant( vm, w->attributes() );

   return 0;
}

//------------------------------------------------------------------------------
//!
int entityAttributeVM( VMState* vm )
{
   Entity* e = (Entity*)VM::thisPtr(vm);

   switch( VM::getTop( vm ) )
   {
      case 1:
      {
         // Get.
         if( e->attributes() != NULL )
         {
            switch( VM::type( vm, 1 ) )
            {
               case VM::NUMBER:
               {
                  uint idx = VM::toUInt( vm, 1 );
                  const Variant& v = e->attributes()->get( idx );
                  VM::push( vm, v );
                  return 1;
               }  break;
               case VM::STRING:
               {
                  ConstString key = VM::toCString( vm, 1 );
                  const Variant& v = e->attributes()->get( key );
                  VM::push( vm, v );
                  return 1;
               }  break;
               default:
               {
                  StdErr << "WorldVM::entityAttributeVM() - Invalid key type: " << VM::type( vm, 1 ) << "." << nl;
               }  break;
            }
         }
      }  break;
      case 2:
      {
         // Set.
         if( e->attributes() == NULL )  e->attributes( new Table() );
         VM::toVariant( vm, *(e->attributes()) );
      }  break;
      default:
         StdErr << "WorldVM::entityAttributeVM() - Wrong number of arguments: expected 1, got " << VM::getTop( vm ) << "." << nl;
         break;
   }

   return 0;
}

//-----------------------------------------------------------------------------
//!
int entityAttributeBrainVM( VMState* vm )
{
   Entity* e = (Entity*)VM::thisPtr(vm);

   switch( VM::getTop( vm ) )
   {
      case 1:
      {
         // Get.
         if( e->attributes() != NULL )
         {
            switch( VM::type( vm, 1 ) )
            {
               case VM::NUMBER:
               {
                  uint idx = VM::toUInt( vm, 1 );
                  const Variant& v = e->attributes()->get( idx );
                  VM::push( vm, v );
                  return 1;
               }  break;
               case VM::STRING:
               {
                  ConstString key = VM::toCString( vm, 1 );
                  const Variant& v = e->attributes()->get( key );
                  VM::push( vm, v );
                  return 1;
               }  break;
               default:
               {
                  StdErr << "WorldVM::entityAttributeVM() - Invalid key type: " << VM::type( vm, 1 ) << "." << nl;
               }  break;
            }
         }
      }  break;
      case 2:
      {
         // Set.
         BrainTaskContext* context = (BrainTaskContext*)VM::userData( vm );
         if( e == context->entity() )
         {
            // Only allowed to set the current entity.
            if( e->attributes() == NULL )  e->attributes( new Table() );
            VM::toVariant( vm, *(e->attributes()) );
            return 1;
         }
         else
         {
            StdErr << "WorldVM::entityAttributeVM() - Attempted to set attribute on the non-current entity." << nl;
         }
      }  break;
      default:
         StdErr << "WorldVM::entityAttributeVM() - Wrong number of arguments: expected 1 or 2, got " << VM::getTop( vm ) << "." << nl;
         break;
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int setGeometryVM( VMState* /*vm*/ )
{
/*
   Entity* e   = (Entity*)VM::thisPtr(vm);
   String name = VM::toString( vm, 1 );
   Geometry* geom;

   int numParams = VM::getTop(vm);

   if( numParams == 1 || VM::isNil( vm, 2 ) )
   {
      geom = ResManager::getGeometry( name, NULL );
   }
   else
   if( VM::isTable( vm, 2 ) )
   {
      // Compiled geometry?
      bool compiled = true;
      if( numParams > 2 ) compiled = VM::toBoolean( vm, 3 );

      // Read paramaters.
      RCP<Table> params( new Table );
      VM::toTable( vm, 2, *params );
      geom = ResManager::getGeometry( name, *params, NULL, compiled );
   }
   else
   {
      StdErr << "geometry: second parameter should be a table.\n";
      return 0;
   }

   e->geometry( geom );
*/
   return 0;
}

//------------------------------------------------------------------------------
//!
int setMaterialVM( VMState* /*vm*/ )
{
/*
   Entity* e   = (Entity*)VM::thisPtr(vm);
   String name = VM::toString( vm, 1 );
   RCP<MaterialSet> mat;

   int numParams = VM::getTop(vm);

   if( numParams == 1 || VM::isNil( vm, 2 ) )
   {
      mat = ResManager::newMaterialSet( name, NULL );
   }
   else
   if( VM::isTable( vm, 2 ) )
   {
      // Read paramaters.
      RCP<Table> params( new Table );
      VM::toTable( vm, 2, *params );
      mat = ResManager::newMaterialSet( name, *params, NULL );
   }
   else
   {
      StdErr << "material: second parameter should be a table.\n";
      return 0;
   }

   e->materialSet( mat.ptr() );
*/
   return 0;
}

//------------------------------------------------------------------------------
//!
int stimulateVM( VMState* vm )
{
   Entity* e = (Entity*)VM::thisPtr( vm );

   ConstString type = VM::toConstString( vm, 1 );
   if( type.isNull() )  return 0;

   RCP<Stimulus> s = Stimulus::create( type );
   if( s.isValid() )
   {
      if( !s->to( vm, 2 ) )
      {
         StdErr << "EROR stimulus() - Parameters parse error for stimulus of type '" << type << "'." << nl;
      }
      e->stimulate( s.ptr() );
   }
   else
   {
      StdErr << "ERROR: stimulus() - Failed to create a stimulus of type '" << type << "'." << nl;
   }

   return 0;
}

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   CLASS World
==============================================================================*/

//------------------------------------------------------------------------------
//!
int world_create( VMState* vm )
{
   World* w = new World();

   // Initialize.
   Vec4f bg;
   if( VM::get( vm, -1, "backgroundColor", bg ) ) w->backgroundColor( bg );
   Vec3f gravity;
   if( VM::get( vm, -1, "gravity", gravity ) ) w->gravity( gravity );

   VM::pushProxy( vm, w );
   return 1;
}

//------------------------------------------------------------------------------
//!
int world_get( VMState* vm )
{
   World* w = (World*)VM::toProxy( vm, 1 );

   switch( _worldAttributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_ADD_CAMERA:
         VM::push( vm, w, addCameraVM );
         return 1;
      case ATTRIB_ADD_LIGHT:
         VM::push( vm, w, addLightVM );
         return 1;
      case ATTRIB_ADD_OBJECT:
         VM::push( vm, w, addObjectVM );
         return 1;
      case ATTRIB_ADD_PROXY:
         VM::push( vm, w, addProxyVM );
         return 1;
      case ATTRIB_ADD_SKELETAL_OBJECT:
         VM::push( vm, w, addSkeletalObjectVM );
         return 1;
      case ATTRIB_ADD_STATIC_OBJECT:
         VM::push( vm, w, addStaticObjectVM );
         return 1;
      case ATTRIB_BACKGROUND_COLOR:
         VM::push( vm, w->backgroundColor() );
         return 1;
      case ATTRIB_CAMERA:
         VM::push( vm, w, cameraVM );
         return 1;
      case ATTRIB_ENTITY:
         VM::push( vm, w, entityVM );
         return 1;
      case ATTRIB_GET_ATTRIBUTE:
         VM::push( vm, w, getAttributeVM );
         return 1;
      case ATTRIB_GRAVITY:
         VM::push( vm, w->gravity() );
         return 1;
      case ATTRIB_LIGHT:
         VM::push( vm, w, lightVM );
         return 1;
      case ATTRIB_NUM_CAMERAS:
         VM::push( vm, w->numCameras() );
         return 1;
      case ATTRIB_NUM_ENTITIES:
         VM::push( vm, w->numEntities() );
         return 1;
      case ATTRIB_NUM_LIGHTS:
         VM::push( vm, w->numLights() );
         return 1;
      case ATTRIB_SET_ATTRIBUTE:
         VM::push( vm, w, setAttributeVM );
         return 1;
      case ATTRIB_SIMULATION_SPEED:
         VM::push( vm, w->simulationSpeed() );
         return 1;
      case ATTRIB_START_SIMULATION:
         VM::push( vm, w, startSimulationVM );
         return 1;
      case ATTRIB_STOP_SIMULATION:
         VM::push( vm, w, stopSimulationVM );
         return 1;
      case ATTRIB_TIME:
         VM::push( vm, w->time() );
         return 1;
   }
   return 0;
}

//------------------------------------------------------------------------------
//!
int world_set( VMState* vm )
{
   World* w = (World*)VM::toProxy( vm, 1 );
   switch( _worldAttributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_ADD_CAMERA:      // Read-only.
      case ATTRIB_ADD_LIGHT:
      case ATTRIB_ADD_OBJECT:
      case ATTRIB_ADD_PROXY:
      case ATTRIB_ADD_SKELETAL_OBJECT:
      case ATTRIB_ADD_STATIC_OBJECT:
         return 0;
      case ATTRIB_BACKGROUND_COLOR:
         w->backgroundColor( VM::toVec4f( vm, 3 ) );
         return 0;
      case ATTRIB_CAMERA:          // Read-only.
      case ATTRIB_ENTITY:          // Read-only.
      case ATTRIB_GET_ATTRIBUTE:   // Read-only.
         return 0;
      case ATTRIB_GRAVITY:
         w->gravity( VM::toVec3f( vm, 3 ) );
         return 0;
      case ATTRIB_LIGHT:           // Read-only.
         return 0;
      case ATTRIB_NUM_CAMERAS:
      case ATTRIB_NUM_ENTITIES:
      case ATTRIB_NUM_LIGHTS:
         return 0;
      case ATTRIB_SET_ATTRIBUTE:   // Read-only.
         return 0;
      case ATTRIB_SIMULATION_SPEED:
         w->simulationSpeed( VM::toFloat( vm, 3 ) );
         return 0;
      case ATTRIB_START_SIMULATION:
      case ATTRIB_STOP_SIMULATION: // Read-only.
      case ATTRIB_TIME:            // Read-only.
         return 0;
   }
   return 0;
}

/*==============================================================================
   CLASS Entity
==============================================================================*/

//------------------------------------------------------------------------------
//!
void initCamera( VMState* vm, Camera* e, WorldContext* wc )
{
   initEntity( vm, e, wc );
   initRigid( vm, e );

   // Focal length.
   float fl;
   if( VM::get( vm, -1, "focal", fl ) ) e->focalLength( fl );

   // Clipping planes.
   float f, b;
   if( VM::get( vm, -1, "front", f ) ) e->front( f );
   if( VM::get( vm, -1, "back", b ) )  e->back( b );

   // Shearing.
   Vec2f shear;
   if( VM::get( vm, -1, "shear", shear ) ) e->shear( shear.x, shear.y );

   // Field of view.
   float fov;
   if( VM::get( vm, -1, "fov", fov ) ) e->fov( fov );
   const char* mode;
   if( VM::get( vm, -1, "fovMode", mode ) )
   {
      switch( mode[0] )
      {
         case 'x': e->fovMode( Camera::FOV_X );    break;
         case 'y': e->fovMode( Camera::FOV_Y );    break;
         case 's': e->fovMode( Camera::SMALLEST ); break;
         case 'l': e->fovMode( Camera::LARGEST );  break;
         default:
            StdErr << "Invalid camera fovMode string: '" << mode << "'." << nl;
            break;
      }
   }

   // Orthographic scaling factor.
   float scale;
   if( VM::get( vm, -1, "orthoScale", scale ) ) e->orthoScale( scale );

   // Projection type.
   const char* type;
   if( VM::get( vm, -1, "projection", type ) )
   {
      if( type[0] == 'p' )
         e->projection( Camera::PERSPECTIVE );
      else
         e->projection( Camera::ORTHO );
   }
}

//------------------------------------------------------------------------------
//!
void initLight( VMState* vm, Light* l, WorldContext* wc )
{
   initEntity( vm, l, wc );
   initRigid( vm, l );

   // Shape.
   const char* shape;
   if( VM::get( vm, -1, "shape", shape ) )
   {
      switch( shape[0] )
      {
         case 'd': l->shape( Light::DIRECTIONAL ); break;
         case 'g': l->shape( Light::GEOMETRY );    break;
         case 'p': l->shape( Light::POINT );       break;
         case 's': l->shape( Light::SPOT );        break;
      }
   }

   // Intensity.
   Vec3f intensity( 1.0f );
   if( VM::get( vm, -1, "intensity", intensity ) ) l->intensity( intensity );

   // Clipping planes.
   float f;
   float b;
   if( VM::get( vm, -1, "front", f ) ) l->front( f );
   if( VM::get( vm, -1, "back", b ) )  l->back( b );

   // Field of view.
   float fov;
   if( VM::get( vm, -1, "fov", fov ) ) l->fov( fov );
}

//------------------------------------------------------------------------------
//!
void initProxyEntity( VMState* vm, ProxyEntity* e, WorldContext* wc )
{
   initEntity( vm, e, wc );

   if( wc )
   {
      if( VM::get( vm, -1, "entity" ) )
      {
         e->entity( (ProxyEntity*)VM::toPtr( vm, -1 ) );
         VM::pop( vm, 1 );
      }
   }
   else
   {
      e->entity( (ProxyEntity*)VM::toProxy( vm, -1 ) );
   }
}

//------------------------------------------------------------------------------
//!
void initRigidEntity( VMState* vm, RigidEntity* e, WorldContext* wc )
{
   initEntity( vm, e, wc );
   initRigid( vm, e );
}

//------------------------------------------------------------------------------
//!
void initSkeletalEntity( VMState* vm, SkeletalEntity* e, WorldContext* wc )
{
   initEntity( vm, e, wc );
   initSkeletal( vm, e, wc );
}

//------------------------------------------------------------------------------
//!
void initParticleEntity( VMState* vm, ParticleEntity* e, WorldContext* wc )
{
   initEntity( vm, e, wc );
   initParticle( vm, e, wc );
}

//------------------------------------------------------------------------------
//!
int entity_get( VMState* vm )
{
   Entity* e       = (Entity*)VM::toProxy( vm, 1 );
   const char* key = VM::toCString( vm, 2 );
   switch( _entityAttributes[key] )
   {
      case ATTRIB_ATTRIBUTE:
         VM::push( vm, e, entityAttributeVM );
         return 1;
      case ATTRIB_BRAIN:
         // TODO.
         return 0;
      case ATTRIB_POSITION:
         VM::push( vm, e->position() );
         return 1;
      case ATTRIB_ORIENTATION:
         VM::push( vm, e->orientation() );
         return 1;
      case ATTRIB_SCALE:
         VM::push( vm, e->scale() );
         return 1;
      case ATTRIB_VISIBLE:
         VM::push( vm, e->visible() );
         return 1;
      case ATTRIB_CASTS_SHADOWS:
         VM::push( vm, e->castsShadows() );
         return 1;
      case ATTRIB_GHOST:
         VM::push( vm, e->ghost() );
         return 1;
      case ATTRIB_GEOMETRY:
         VM::pushProxy( vm, e->geometry() );
         return 1;
      case ATTRIB_SET_GEOMETRY:
         VM::push( vm, e, setGeometryVM );
         return 1;
      case ATTRIB_MATERIAL:
         // TODO.
         return 0;
      case ATTRIB_SET_MATERIAL:
         VM::push( vm, e, setMaterialVM );
         return 1;
      case ATTRIB_STIMULATE:
         VM::push( vm, e, stimulateVM );
         return 1;
   }

   // RigidEntity.
   if( e->type() <= Entity::LIGHT )
   {
      RigidEntity* re = (RigidEntity*)e;
      switch( _rigidAttributes[key] )
      {
         case ATTRIB_MASS_R:
            VM::push( vm, re->mass() );
            return 1;
         case ATTRIB_FRICTION_R:
            VM::push( vm, re->friction() );
            return 1;
         case ATTRIB_RESTITUTION_R:
            VM::push( vm, re->restitution() );
            return 1;
         case ATTRIB_EXISTS_R:
            VM::push( vm, re->exists() );
            return 1;
         case ATTRIB_SENSES_R:
            VM::push( vm, re->senses() );
            return 1;
         case ATTRIB_ATTRACTION_CATEGORIES_R:
            VM::push( vm, re->attractionCategories() );
            return 1;
         case ATTRIB_LINEAR_VELOCITY_R:
            VM::push( vm, re->linearVelocity() );
            return 1;
         case ATTRIB_ANGULAR_VELOCITY_R:
            VM::push( vm, re->angularVelocity() );
            return 1;
      }
   }

   switch( e->type() )
   {
      case Entity::CAMERA:
      {
         Camera* c = (Camera*)e;
         switch( _camAttributes[key] )
         {
            case ATTRIB_BACK_C:
               VM::push( vm, c->back() );
               return 1;
            case ATTRIB_FOCAL_C:
               VM::push( vm, c->focalLength() );
               return 1;
            case ATTRIB_FOV_C:
               VM::push( vm, c->fov() );
               return 1;
            case ATTRIB_FOV_MODE_C:
               VM::push( vm, c->fovMode() );
               return 1;
            case ATTRIB_FRONT_C:
               VM::push( vm, c->front() );
               return 1;
            case ATTRIB_ORTHO_SCALE_C:
               VM::push( vm, c->orthoScale() );
               return 1;
            case ATTRIB_PROJECTION_C:
               VM::push( vm, c->projection() == Camera::ORTHO ? "ortho" : "perspective" );
               return 1;
         }
      }  break;
      case Entity::LIGHT:
      {
         Light* l = (Light*)e;
         switch( _lightAttributes[key] )
         {
            case ATTRIB_BACK_L:
               VM::push( vm, l->back() );
               return 1;
            case ATTRIB_INTENSITY_L:
               VM::push( vm, l->intensity() );
               return 1;
            case ATTRIB_FOV_L:
               VM::push( vm, l->fov() );
               return 1;
            case ATTRIB_FRONT_L:
               VM::push( vm, l->front() );
               return 1;
            case ATTRIB_SHAPE_L:
               switch( l->shape() )
               {
                  case Light::DIRECTIONAL: VM::push( vm, "directional" ); break;
                  case Light::GEOMETRY:    VM::push( vm, "geometry" );    break;
                  case Light::POINT:       VM::push( vm, "point" );       break;
                  case Light::SPOT:        VM::push( vm, "spot" );        break;
               }
               return 1;
         }
      }  break;
      case Entity::PROXY:
      {
         ProxyEntity* p = (ProxyEntity*)e;
         switch( _proxyAttributes[key] )
         {
            case ATTRIB_ENTITY_PROXY:
               VM::pushProxy( vm, p->entity() );
               return 1;
         }
      }  break;
      default:;
   }

   Table* attr = e->attributes();
   if( attr )
   {
      ConstString s = key;
      if( attr->has( s ) )
      {
         VM::push( vm, attr->get( s ) );
         return 1;
      }
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int entity_get_brains( VMState* vm )
{
   Entity* e       = (Entity*)VM::toProxy( vm, 1 );
   const char* key = VM::toCString( vm, 2 );

   switch( _entityAttributes[key] )
   {
      case ATTRIB_ATTRIBUTE:
         VM::push( vm, e, entityAttributeBrainVM ); // Only possible for the current entity.
         return 1;
      case ATTRIB_BRAIN:
         // TODO.
         return 0;
      case ATTRIB_POSITION:
         VM::push( vm, e->position() );
         return 1;
      case ATTRIB_ORIENTATION:
         VM::push( vm, e->orientation() );
         return 1;
      case ATTRIB_SCALE:
         VM::push( vm, e->scale() );
         return 1;
      case ATTRIB_VISIBLE:
         VM::push( vm, e->visible() );
         return 1;
      case ATTRIB_CASTS_SHADOWS:
         VM::push( vm, e->castsShadows() );
         return 1;
      case ATTRIB_GHOST:
         VM::push( vm, e->ghost() );
         return 1;
      case ATTRIB_GEOMETRY:
         VM::push( vm, (void*)e->geometry() );
         return 1;
      case ATTRIB_SET_GEOMETRY:
         //Read-only.
         return 0;
      case ATTRIB_MATERIAL:
         VM::push( vm, (void*)e->materialSet() );
         return 1;
      case ATTRIB_SET_MATERIAL:
         // Read-only.
         return 0;
      case ATTRIB_STIMULATE:
         VM::push( vm, e, stimulateVM );
         return 1;
   }

   // RigidEntity.
   if( e->type() <= Entity::LIGHT )
   {
      RigidEntity* re = (RigidEntity*)e;
      switch( _rigidAttributes[key] )
      {
         case ATTRIB_MASS_R:
            VM::push( vm, re->mass() );
            return 1;
         case ATTRIB_FRICTION_R:
            VM::push( vm, re->friction() );
            return 1;
         case ATTRIB_RESTITUTION_R:
            VM::push( vm, re->restitution() );
            return 1;
         case ATTRIB_EXISTS_R:
            VM::push( vm, re->exists() );
            return 1;
         case ATTRIB_SENSES_R:
            VM::push( vm, re->senses() );
            return 1;
         case ATTRIB_ATTRACTION_CATEGORIES_R:
            VM::push( vm, re->attractionCategories() );
            return 1;
         case ATTRIB_LINEAR_VELOCITY_R:
            VM::push( vm, re->linearVelocity() );
            return 1;
         case ATTRIB_ANGULAR_VELOCITY_R:
            VM::push( vm, re->angularVelocity() );
            return 1;
      }
   }

   switch( e->type() )
   {
      case Entity::CAMERA:
      {
         Camera* c = (Camera*)e;
         switch( _camAttributes[key] )
         {
            case ATTRIB_BACK_C:
               VM::push( vm, c->back() );
               return 1;
            case ATTRIB_FOCAL_C:
               VM::push( vm, c->focalLength() );
               return 1;
            case ATTRIB_FOV_C:
               VM::push( vm, c->fov() );
               return 1;
            case ATTRIB_FOV_MODE_C:
               VM::push( vm, c->fovMode() );
               return 1;
            case ATTRIB_FRONT_C:
               VM::push( vm, c->front() );
               return 1;
            case ATTRIB_ORTHO_SCALE_C:
               VM::push( vm, c->orthoScale() );
               return 1;
            case ATTRIB_PROJECTION_C:
               VM::push( vm, c->projection() == Camera::ORTHO ? "ortho" : "perspective" );
               return 1;
         }
      }  break;
      case Entity::LIGHT:
      {
         Light* l = (Light*)e;
         switch( _lightAttributes[key] )
         {
            case ATTRIB_BACK_L:
               VM::push( vm, l->back() );
               return 1;
            case ATTRIB_INTENSITY_L:
               VM::push( vm, l->intensity() );
               return 1;
            case ATTRIB_FOV_L:
               VM::push( vm, l->fov() );
               return 1;
            case ATTRIB_FRONT_L:
               VM::push( vm, l->front() );
               return 1;
            case ATTRIB_SHAPE_L:
               switch( l->shape() )
               {
                  case Light::DIRECTIONAL: VM::push( vm, "directional" ); break;
                  case Light::GEOMETRY:    VM::push( vm, "geometry" );    break;
                  case Light::POINT:       VM::push( vm, "point" );       break;
                  case Light::SPOT:        VM::push( vm, "spot" );        break;
               }
               return 1;
         }
      }  break;
      default:;
   }

   Table* attr = e->attributes();
   if( attr )
   {
      ConstString s = key;
      if( attr->has( s ) )
      {
         VM::push( vm, attr->get( s ) );
         return 1;
      }
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int entity_set( VMState* vm )
{
   Entity* e       = (Entity*)VM::toProxy( vm, 1 );
   const char* key = VM::toCString( vm, 2 );

   switch( _entityAttributes[key] )
   {
      case ATTRIB_ATTRIBUTE:
         return 0; // Read-only.
      case ATTRIB_BRAIN:
         // TODO
         return 0;
      case ATTRIB_POSITION:
         e->position( VM::toVec3f( vm, 3 ) );
         return 0;
      case ATTRIB_ORIENTATION:
         e->orientation( VM::toQuatf( vm, 3 ) );
         return 0;
      case ATTRIB_SCALE:
         e->scale( VM::toFloat( vm, 3 ) );
         return 0;
      case ATTRIB_VISIBLE:
         e->visible( VM::toBoolean( vm, 3 ) );
         return 0;
      case ATTRIB_CASTS_SHADOWS:
         e->castsShadows( VM::toBoolean( vm, 3 ) );
         return 0;
      case ATTRIB_GHOST:
         e->ghost( VM::toBoolean( vm, 3 ) );
         return 0;
      case ATTRIB_GEOMETRY:
         e->geometry( (Geometry*)VM::toProxy( vm, 3 ) );
         return 0;
      case ATTRIB_SET_GEOMETRY:
         return 0; // Read-only.
      case ATTRIB_MATERIAL:
         // TODO
         return 0;
      case ATTRIB_SET_MATERIAL:
      case ATTRIB_STIMULATE:
         return 0; // Read-only.
   }

   // RigidEntity.
   if( e->type() <= Entity::LIGHT )
   {
      RigidEntity* re = (RigidEntity*)e;
      switch( _rigidAttributes[key] )
      {
         case ATTRIB_MASS_R:
            re->mass( VM::toFloat( vm, 3 ) );
            return 0;
         case ATTRIB_FRICTION_R:
            re->friction( VM::toFloat( vm, 3 ) );
            return 0;
         case ATTRIB_RESTITUTION_R:
            re->restitution( VM::toFloat( vm, 3 ) );
            return 0;
         case ATTRIB_EXISTS_R:
            re->exists( VM::toUInt( vm, 3 ) );
            return 0;
         case ATTRIB_SENSES_R:
            re->senses( VM::toUInt( vm, 3 ) );
            return 0;
         case ATTRIB_ATTRACTION_CATEGORIES_R:
            re->attractionCategories( VM::toUInt( vm, 3 ) );
            return 0;
         case ATTRIB_LINEAR_VELOCITY_R:
            re->linearVelocity( VM::toVec3f( vm, 3 ) );
            return 0;
         case ATTRIB_ANGULAR_VELOCITY_R:
            re->angularVelocity( VM::toVec3f( vm, 3 ) );
            return 0;
      }
   }

   switch( e->type() )
   {
      case Entity::CAMERA:
      {
         Camera* c = (Camera*)e;
         switch( _camAttributes[key] )
         {
            case ATTRIB_BACK_C:
               c->back( VM::toFloat( vm, 3 ) );
               return 0;
            case ATTRIB_FOCAL_C:
               c->focalLength( VM::toFloat( vm, 3 ) );
               return 0;
            case ATTRIB_FOV_C:
               c->fov( VM::toFloat( vm, 3 ) );
               return 0;
            case ATTRIB_FOV_MODE_C:
            {
               const char* str = VM::toCString( vm, 3 );
               if( str != NULL )
               {
                  switch( str[0] )
                  {
                     case 'x':
                        c->fovMode( Camera::FOV_X );
                        break;
                     case 'y':
                        c->fovMode( Camera::FOV_Y );
                        break;
                     case 's':
                        c->fovMode( Camera::SMALLEST );
                        break;
                     case 'l':
                        c->fovMode( Camera::LARGEST );
                        break;
                     default:
                        StdErr << "Invalid camera field of view mode string: '" << str << "'." << nl;
                        break;
                  }
               }
               else
               {
                  StdErr << "Null camera field of view mode string." << nl;
               }
            }  return 0;
            case ATTRIB_FRONT_C:
               c->front( VM::toFloat( vm, 3 ) );
               return 0;
            case ATTRIB_ORTHO_SCALE_C:
               c->orthoScale( VM::toFloat( vm, 3 ) );
               return 0;
            case ATTRIB_PROJECTION_C:
               if( VM::toCString( vm, 3 )[0] == 'p' )
               {
                  c->projection( Camera::PERSPECTIVE );
               }
               else
               {
                  c->projection( Camera::ORTHO );
               }
               return 0;
         }
      }  break;
      case Entity::LIGHT:
      {
         Light* l = (Light*)e;
         switch( _lightAttributes[key] )
         {
            case ATTRIB_BACK_L:
               l->back( VM::toFloat( vm, 3 ) );
               return 0;
            case ATTRIB_INTENSITY_L:
               l->intensity( VM::toVec3f( vm, 3 ) );
               return 0;
            case ATTRIB_FOV_L:
               l->fov( VM::toFloat( vm, 3 ) );
               return 0;
            case ATTRIB_FRONT_L:
               l->front( VM::toFloat( vm, 3 ) );
               return 0;
            case ATTRIB_SHAPE_L:
               switch( VM::toCString( vm, 3 )[0] )
               {
                  case 'd': l->shape( Light::DIRECTIONAL ); break;
                  case 'g': l->shape( Light::GEOMETRY );    break;
                  case 'p': l->shape( Light::POINT );       break;
                  case 's': l->shape( Light::SPOT );        break;
               }
               return 0;
         }
      }  break;
      case Entity::PROXY:
      {
         ProxyEntity* p = (ProxyEntity*)e;
         switch( _proxyAttributes[key] )
         {
            case ATTRIB_ENTITY_PROXY:
               p->entity( (Entity*)VM::toProxy( vm, 3 ) );
               return 0;
         }
      }  break;
      default:;
   }

   if( e->attributes() == NULL )  e->attributes( new Table() );
   VM::toVariant( vm, *(e->attributes()) );

   return 0;
}

//------------------------------------------------------------------------------
//!
int entity_set_brains( VMState* vm )
{
   BrainTaskContext* context = (BrainTaskContext*)VM::userData( vm );
   Entity* e       = (Entity*)VM::toProxy( vm, 1 );
   const char* key = VM::toCString( vm, 2 );

#define IF_RET0( cond, msg ) \
   if( cond ) \
   { \
      StdErr << "WorldVM::entity_set_brains() - " << msg << nl; \
      return 0; \
   }

   IF_RET0( e != context->entity(), "Attempted to set attribute on the non-current entity." );

   IF_RET0( _entityAttributes[key] != StringMap::INVALID, "Cannot assign '" << key << "' - reserved for entities." );

   // RigidEntity.
   if( e->type() <= Entity::LIGHT )
   {
      IF_RET0( _rigidAttributes[key] != StringMap::INVALID, "Cannot assign '" << key << "' - reserved for rigid entities." );
   }

   switch( e->type() )
   {
      case Entity::CAMERA:
      {
         IF_RET0( _camAttributes[key] != StringMap::INVALID, "Cannot assign '" << key << "' - reserved for cameras." );
      }  break;
      case Entity::LIGHT:
      {
         IF_RET0( _lightAttributes[key] != StringMap::INVALID, "Cannot assign '" << key << "' - reserved for lights." );
      }  break;
      default:;
   }

#undef IF_RET0

   if( e->attributes() == NULL )  e->attributes( new Table() );
   VM::toVariant( vm, *(e->attributes()) );

   return 0;
}


/*==============================================================================
   CLASS Geometry
==============================================================================*/

//------------------------------------------------------------------------------
//!
int geometry_get( VMState* /*vm*/ )
{
/*
   Geometry* g = (Geometry*)VM::toProxy( vm, 1 );

   switch( _geomAttributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_STATE_G:
         VM::push( vm, g->state() );
         return 1;
   }
*/
   return 0;
}

//------------------------------------------------------------------------------
//!
int geometry_set( VMState* vm )
{
   //Geometry* g = (Geometry*)VM::toProxy( vm, 1 );

   switch( _geomAttributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_STATE_G:
         return 0;
   }
   return 0;
}


/*==============================================================================
   CLASS Receptor
==============================================================================*/

//------------------------------------------------------------------------------
//!
int receptor_get( VMState* vm )
{
   Receptor* r = (Receptor*)VM::toProxy( vm, 1 );

   const char* k = VM::toCString( vm, 2 );
   int    attrib = _receptorAttributes[k];

   switch( attrib )
   {
      case RECEPTOR_ATTRIB_TYPE:
         VM::push( vm, r->type() );
         return 1;
   }

   switch( r->type() )
   {
      case Receptor::TYPE_CONTACT:
      {
         ContactReceptor* cr = (ContactReceptor*)r;
         switch( attrib )
         {
            case RECEPTOR_ATTRIB_RECEIVING_ENTITY:
               VM::pushProxy( vm, cr->receivingEntity() );
               return 1;
            default:
               StdErr << "Unknown contact receptor attribute: '" << k << "'" << nl;
               break;
         }
      }

      case Receptor::TYPE_CONTACT_GROUP:
      {
         ContactGroupReceptor* cgr = (ContactGroupReceptor*)r;
         switch( attrib )
         {
            case RECEPTOR_ATTRIB_RECEIVING_GROUPS:
               VM::push( vm, cgr->receivingGroups() );
               return 1;
            case RECEPTOR_ATTRIB_COLLIDING_GROUPS:
               VM::push( vm, cgr->collidingGroups() );
               return 1;
            default:
               StdErr << "Unknown contact group receptor attribute: '" << k << "'" << nl;
               break;
         }
      }
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int receptor_set( VMState* vm )
{
   Receptor* r = (Receptor*)VM::toProxy( vm, 1 );

   const char* k = VM::toCString( vm, 2 );
   int    attrib = _receptorAttributes[k];

   switch( attrib )
   {
      case RECEPTOR_ATTRIB_TYPE:  // Read-only.
         return 0;
   }

   switch( r->type() )
   {
      case Receptor::TYPE_CONTACT:
      {
         ContactReceptor* cr = (ContactReceptor*)r;
         switch( attrib )
         {
            case RECEPTOR_ATTRIB_RECEIVING_ENTITY:
               cr->receivingEntity( (RigidEntity*)VM::toProxy( vm, 3 ) );
               return 0;
            default:
               StdErr << "Unknown contact receptor attribute: '" << k << "'" << nl;
               break;
         }
      }

      case Receptor::TYPE_CONTACT_GROUP:
      {
         ContactGroupReceptor* cgr = (ContactGroupReceptor*)r;
         switch( attrib )
         {
            case RECEPTOR_ATTRIB_RECEIVING_GROUPS:
               cgr->receivingGroups( VM::toUInt( vm, 3 ) );
               return 1;
            case RECEPTOR_ATTRIB_COLLIDING_GROUPS:
               cgr->collidingGroups( VM::toUInt( vm, 3 ) );
               return 1;
            default:
               StdErr << "Unknown contact group receptor attribute: '" << k << "'" << nl;
               break;
         }
      }
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int receptor_set( VMState* vm, int, Receptor* r )
{
   const char* k = VM::toCString( vm, 2 );
   int    attrib = _receptorAttributes[k];

   switch( attrib )
   {
      case RECEPTOR_ATTRIB_TYPE:  // Read-only.
         return 0;
   }

   switch( r->type() )
   {
      case Receptor::TYPE_CONTACT:
      {
         ContactReceptor* cr = (ContactReceptor*)r;
         switch( attrib )
         {
            case RECEPTOR_ATTRIB_RECEIVING_ENTITY:
               cr->receivingEntity( (RigidEntity*)VM::toProxy( vm, 3 ) );
               return 0;
            default:
               StdErr << "Unknown contact receptor attribute: '" << k << "'" << nl;
               break;
         }
      }

      case Receptor::TYPE_CONTACT_GROUP:
      {
         ContactGroupReceptor* cgr = (ContactGroupReceptor*)r;
         switch( attrib )
         {
            case RECEPTOR_ATTRIB_RECEIVING_GROUPS:
               cgr->receivingGroups( VM::toUInt( vm, 3 ) );
               return 1;
            case RECEPTOR_ATTRIB_COLLIDING_GROUPS:
               cgr->collidingGroups( VM::toUInt( vm, 3 ) );
               return 1;
            default:
               StdErr << "Unknown contact group receptor attribute: '" << k << "'" << nl;
               break;
         }
      }
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
void initReceptor( VMState* vm, int idx, Receptor* r )
{
   switch( r->type() )
   {
      case Receptor::TYPE_CONTACT:
         initContactReceptor( vm, idx, (ContactReceptor*)r );
         break;
      case Receptor::TYPE_CONTACT_GROUP:
         initContactGroupReceptor( vm, idx, (ContactGroupReceptor*)r );
         break;
      default:
         StdErr << "initReceptor() - Unknown receptor type: " << r->type() << "." << nl;
         CHECK( false );
         break;
   }
}

//------------------------------------------------------------------------------
//!
void initContactReceptor( VMState* vm, int idx, ContactReceptor* r )
{
   idx = VM::absIndex( vm, idx );
   if( VM::isTable( vm, idx ) )
   {
      // Start iterating at index 0 (nil, pushed below).
      VM::push( vm );
      while( VM::next( vm, idx ) )
      {
         //if( VM::isNumber( vm, -2 ) )  continue;
         const char* k = VM::toCString( vm, -2 );
         switch( _receptorAttributes[k] )
         {
            case RECEPTOR_ATTRIB_RECEIVING_ENTITY:
            {
               RigidEntity* e = nullptr;
               switch( VM::type( vm, -1 ) )
               {
                  case VM::PTR:
                     e = (RigidEntity*)VM::toPtr( vm, -1 );
                     break;
                  case VM::OBJECT:
                     e = (RigidEntity*)VM::toProxy( vm, -1 );
                     break;
               }
               r->receivingEntity( e );
            }  break;
            default:
               StdErr << "Unknown contact receptor attribute: '" << k << "'" << nl;
               break;
         }
         VM::pop( vm, 1 ); // Pop the value but keep the key.
      }
   }
}

//------------------------------------------------------------------------------
//!
void initContactGroupReceptor( VMState* vm, int idx, ContactGroupReceptor* r )
{
   idx = VM::absIndex( vm, idx );
   if( VM::isTable( vm, idx ) )
   {
      // Start iterating at index 0 (nil, pushed below).
      VM::push( vm );
      while( VM::next( vm, idx ) )
      {
         //if( VM::isNumber( vm, -2 ) )  continue;
         const char* k = VM::toCString( vm, -2 );
         switch( _receptorAttributes[k] )
         {
            case RECEPTOR_ATTRIB_RECEIVING_GROUPS:
               r->receivingGroups( VM::toUInt( vm, -1 ) );
               break;
            case RECEPTOR_ATTRIB_COLLIDING_GROUPS:
               r->collidingGroups( VM::toUInt( vm, -1 ) );
               break;
            default:
               StdErr << "Unknown contact group receptor attribute: '" << k << "'" << nl;
               break;
         }
         VM::pop( vm, 1 ); // Pop the value but keep the key.
      }
   }
}

NAMESPACE_END
