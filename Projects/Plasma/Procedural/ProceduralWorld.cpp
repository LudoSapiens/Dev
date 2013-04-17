/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Procedural/ProceduralWorld.h>

#include <Plasma/Action/PuppeteerAction.h>
#include <Plasma/Animation/Puppeteer.h>
#include <Plasma/Particle/ParticleAnimator.h>
#include <Plasma/Particle/ParticleGenerator.h>
#include <Plasma/Resource/ResManager.h>
#include <Plasma/World/Camera.h>
#include <Plasma/World/EntityGroup.h>
#include <Plasma/World/Light.h>
#include <Plasma/World/ParticleEntity.h>
#include <Plasma/World/Probe.h>
#include <Plasma/World/ProxyEntity.h>
#include <Plasma/World/RigidEntity.h>
#include <Plasma/World/SkeletalEntity.h>
#include <Plasma/World/WorldVM.h>

#include <Fusion/VM/VMRegistry.h>
#include <Fusion/VM/VMMath.h>
#include <Fusion/Core/Core.h>


/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
inline WorldContext* getContext( VMState* vm )
{
   return (WorldContext*)VM::userData(vm);
}


/*==============================================================================
   Extern functions
==============================================================================*/

//------------------------------------------------------------------------------
//!
int executeVM( VMState* vm )
{
   int nargs = VM::getTop( vm ) - 1;

   // Read geometry id.
   String id = VM::toString( vm, 1 );

   // Find file name.
   const char* ext[] = { ".world", 0 };
   String file = ResManager::idToPath( id, ext );
   if( file.empty() )
   {
      StdErr << "World file '" << id << "' not found." << nl;
      return 0;
   }

   // Run world script.
   // TODO: restrict arguments to cross-VM-safe types only.
   VM::doFile( vm, file, nargs, 0 );

   return 0;
}

//------------------------------------------------------------------------------
//!
int geometryVM( VMState* vm )
{
   WorldContext* context = getContext(vm);
   String name           = VM::toString( vm, 1 );
   RCP< Resource<Geometry> > res;

   name = ResManager::expand( context->curDir(), name );

   int numParams = VM::getTop(vm);

   if( numParams == 1 || VM::isNil( vm, 2 ) )
   {
      res = ResManager::getGeometry( name, context->task() );
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
      res = ResManager::getGeometry( name, *params, context->task(), compiled );
   }
   else
   {
      StdErr << "geometry: second parameter should be a table.\n";
      return 0;
   }

   // Keep resource alive.
   context->_res.pushBack( res );

   VM::push( vm, res.ptr() );
   return 1;
}

//------------------------------------------------------------------------------
//!
int materialVM( VMState* vm )
{
   WorldContext* context = getContext(vm);

   int n = VM::getTop(vm);

   // First argument.
   String name;
   if( n >= 1 )
   {
      name = VM::toString( vm, 1 );
      name = ResManager::expand( context->curDir(), name );
   }
   else
   {
      StdErr << "materialVM() - Missing material name." << nl;
      return 0;
   }

   // Second argument.
   RCP<Table> params;
   if( n >= 2 )
   {
      if( VM::isTable( vm, 2 ) )
      {
         params = new Table();
         VM::toTable( vm, 2, *params );
      }
      else
      if( !VM::isNil( vm, 2 ) )
      {
         StdErr << "materialVM() - Parameters must be a table, or nil." << nl;
         return 0;
      }
   }

   // Third argument.
   RCP<MaterialMap>  map;
   if( n >= 3 )
   {
      if( VM::isTable( vm, 3 ) )
      {
         map = new MaterialMap();
         MaterialMap::toMap( vm, 3, *map );
         StdErr << "MAP!!!" << nl;
         map->print();
      }
      else
      if( !VM::isNil( vm, 3 ) )
      {
         StdErr << "materialVM() - Material remapping must be done with a table (or nil)." << nl;
         return 0;
      }
   }

   RCP< Resource<MaterialSet> > res =
      ResManager::newMaterialSet(
         name,
         params.ptr(),
         map.ptr(),
         context->task()
      );

   // Keep resource alive.
   context->_res.pushBack( res );

   VM::push( vm, res.ptr() );
   return 1;
}

//------------------------------------------------------------------------------
//!
int animationGraphVM( VMState* vm )
{
   WorldContext* context = getContext(vm);

   int n = VM::getTop(vm);

   // First argument.
   String name;
   if( n >= 1 )
   {
      name = VM::toString( vm, 1 );
      name = ResManager::expand( context->curDir(), name );
   }
   else
   {
      StdErr << "animationGraph() - Missing graph name." << nl;
      return 0;
   }

   RCP< Resource<AnimationGraph> > res = ResManager::getAnimationGraph( name, context->task() );
   context->keepResource( res.ptr() );
   VM::push( vm, res.ptr() );
   return 1;
}

/*==============================================================================
   States.
==============================================================================*/

//------------------------------------------------------------------------------
//!
int gravityVM( VMState* vm )
{
   WorldContext* context = getContext( vm );
   context->_world->gravity( VM::toVec3f( vm, -1 ) );
   return 0;
}

//------------------------------------------------------------------------------
//!
int backgroundVM( VMState* vm )
{
   WorldContext* context = getContext( vm );
   context->_world->backgroundColor( VM::toVec4f( vm, -1 ) );
   return 0;
}


/*==============================================================================
   Objects
==============================================================================*/

//------------------------------------------------------------------------------
//!
int cameraVM( VMState* vm )
{
   WorldContext* context = getContext(vm);
   Camera* e = new Camera( RigidBody::DYNAMIC );
   initCamera( vm, e, context );
   context->_world->addEntity( e );
   VM::push( vm, e );
   return 1;
}

//------------------------------------------------------------------------------
//!
int groupVM( VMState* vm )
{
   WorldContext* context = getContext(vm);
   EntityGroup* group = new EntityGroup();
   context->_world->addGroup( group );

   // Load parameters.
   Reff ref = Reff::identity();
   VM::get( vm, -1, "position", ref.position() );
   Quatf q = Quatf::identity();
   VM::get( vm, -1, "orientation", q );
   ref.orientation( q );

   // Brain.

   // User attributes.

   // Objects (read and transform).
   if( VM::get( vm, -1, "object" ) )
   {
      for( uint i = 1; VM::geti( vm, -1, i ); ++i )
      {
         Entity* e = (Entity*)VM::toPtr( vm, -1 );
         group->addEntity( e );
         e->referential( ref * e->referential() );
         VM::pop( vm, 1 );
      }

      VM::pop( vm, 1 );
   }

   // Groups.
   for( uint i = 1; VM::geti( vm, -1, i ); ++i )
   {
      group->addGroup( (EntityGroup*)VM::toPtr( vm, -1 ) );
      VM::pop( vm, 1 );
   }

   VM::push( vm, group );
   return 1;
}

//------------------------------------------------------------------------------
//!
int kinematicObjectVM( VMState* vm )
{
   WorldContext* context = getContext(vm);
   RigidEntity* e = new RigidEntity( RigidBody::KINEMATIC );
   initRigidEntity( vm, e, context );
   context->_world->addEntity( e );
   VM::push( vm, e );
   return 1;
}

//------------------------------------------------------------------------------
//!
int lightVM( VMState* vm )
{
   WorldContext* context = getContext(vm);
   Light* e = new Light( RigidBody::DYNAMIC );
   initLight( vm, e, context );
   context->_world->addEntity( e );
   VM::push( vm, e );
   return 1;
}

//------------------------------------------------------------------------------
//!
int objectVM( VMState* vm )
{
   WorldContext* context = getContext(vm);
   RigidEntity* e = new RigidEntity( RigidBody::DYNAMIC );
   initRigidEntity( vm, e, context );
   context->_world->addEntity( e );
   VM::push( vm, e );
   return 1;
}

//------------------------------------------------------------------------------
//!
int proxyVM( VMState* vm )
{
   WorldContext* context = getContext(vm);
   ProxyEntity* e = new ProxyEntity();
   initProxyEntity( vm, e, context );
   context->_world->addEntity( e );
   VM::push( vm, e );
   return 1;
}

//------------------------------------------------------------------------------
//!
int skeletalObjectVM( VMState* vm )
{
   WorldContext* context = getContext(vm);
   SkeletalEntity* e = new SkeletalEntity();
   initSkeletalEntity( vm, e, context );
   context->_world->addEntity( e );
   VM::push( vm, e );
   return 1;
}

//------------------------------------------------------------------------------
//!
int staticObjectVM( VMState* vm )
{
   WorldContext* context = getContext(vm);
   RigidEntity* e = new RigidEntity( RigidBody::STATIC );
   initRigidEntity( vm, e, context );
   context->_world->addEntity( e );
   VM::push( vm, e );
   return 1;
}

//------------------------------------------------------------------------------
//!
int particleObjectVM( VMState* vm )
{
   WorldContext* context = getContext(vm);
   ParticleEntity* e = new ParticleEntity();
   initParticleEntity( vm, e, context );
   context->_world->addEntity( e );
   VM::push( vm, e );
   return 1;
}

/*==============================================================================
   Constraints
==============================================================================*/

//------------------------------------------------------------------------------
//!
int ballJointVM( VMState* vm )
{
   WorldContext* context = getContext(vm);

   RigidEntity* entityA = 0;
   RigidEntity* entityB = 0;
   if( VM::geti( vm, -1, 1 ) )
   {
      entityA = (RigidEntity*)VM::toPtr( vm, -1 );
      VM::pop( vm, 1 );
   }
   if( VM::geti( vm, -1, 2 ) )
   {
      entityB = (RigidEntity*)VM::toPtr( vm, -1 );
      VM::pop( vm, 1 );
   }

   BallJoint* joint = context->_world->createBallJoint( entityA, entityB );

   Vec3f anchor(0.0f);
   VM::get( vm, -1, "anchor", anchor );

   joint->anchor( anchor );

   return 0;
}

//------------------------------------------------------------------------------
//!
int hingeJointVM( VMState* vm )
{
   WorldContext* context = getContext(vm);

   RigidEntity* entityA = 0;
   RigidEntity* entityB = 0;
   if( VM::geti( vm, -1, 1 ) )
   {
      entityA = (RigidEntity*)VM::toPtr( vm, -1 );
      VM::pop( vm, 1 );
   }
   if( VM::geti( vm, -1, 2 ) )
   {
      entityB = (RigidEntity*)VM::toPtr( vm, -1 );
      VM::pop( vm, 1 );
   }

   HingeJoint* joint = context->_world->createHingeJoint( entityA, entityB );

   Vec3f anchor(0.0f);
   VM::get( vm, -1, "anchor", anchor );

   Vec3f axis(0.0f, 1.0f, 0.0f );
   VM::get( vm, -1, "axis", axis );

   joint->anchor( anchor );
   joint->axis( axis );
   return 0;
}


/*==============================================================================
   Brains, Receptors, Stimuli
==============================================================================*/

//------------------------------------------------------------------------------
//!
int brainVM( VMState* vm )
{
   WorldContext* context = getContext(vm);

   int n = VM::getTop(vm);

   // We can now create the brain.
   RCP<Brain> brain = new Brain();

   // First argument: brain program.
   if( n >= 1 )
   {
      if( VM::isObject( vm, 1 ) )
      {
         // TODO
      }
      else
      {
         String name = VM::toString( vm, 1 );
         name = ResManager::expand( context->curDir(), name );
         RCP< Resource<BrainProgram> > res = ResManager::getBrainProgram( name, context->task() );
         context->keepResource( res.ptr() );
         context->setBrainProgram( brain.ptr(), res.ptr() );
      }
   }
   else
   {
      StdErr << "brainVM() - Missing brain program." << nl;
      return 0;
   }

   // Second argument: receptors.
   if( n >= 2 )
   {
      if( VM::isTable( vm, 2 ) )
      {
         for( int i = 1; VM::geti( vm, -1, i ); ++i )
         {
            if( VM::isObject( vm, -1 ) )
            {
               Receptor* r = (Receptor*)VM::toPtr( vm, -1 );  // We currently push light user data.
               brain->receptors().pushBack( r );
               //StdErr << "Added receptor " << (void*)r << " count=" << r->count() << nl;
            }
            else
            {
               StdErr << "brainVM() - Invalid receptor received." << nl;
            }
            VM::pop( vm, 1 );
         }
      }
      else
      if( !VM::isNil( vm, 2 ) )
      {
         StdErr << "brainVM() - Parameters must be a table, or nil." << nl;
         return 0;
      }
   }

   // Third argument: initial stimulus.
   if( n >= 3 )
   {
      if( VM::toBoolean( vm, 3 ) )
      {
         brain->postBegin();
      }
   }

   // Keep brain alive in the context (until it is assigned).
   context->_others.pushBack( brain );

   VM::push( vm, brain.ptr() ); // Push a light user data.
   return 1;
}

//------------------------------------------------------------------------------
//!
int contactReceptorVM( VMState* vm )
{
   WorldContext* context = getContext(vm);
   ContactReceptor* r = new ContactReceptor();
   context->_others.pushBack( r );

   initContactReceptor( vm, -1, r );

   VM::push( vm, r ); // Push a light user data.
   return 1;
}

//------------------------------------------------------------------------------
//!
int contactGroupReceptorVM( VMState* vm )
{
   WorldContext* context = getContext(vm);
   ContactGroupReceptor* r = new ContactGroupReceptor();
   context->_others.pushBack( r );

   initContactGroupReceptor( vm, -1, r );

   VM::push( vm, r ); // Push a light user data.
   return 1;
}


/*==============================================================================
   Attributes
==============================================================================*/

//------------------------------------------------------------------------------
//!
int getAttributeVM( VMState* vm )
{
   WorldContext* context = getContext(vm);

   if( VM::getTop( vm ) != 1 )
   {
      StdErr << "ProceduralWorld::getAttributeVM() - Wrong number of arguments: expected 1, got " << VM::getTop( vm ) << "." << nl;
      return 0;
   }

   switch( VM::type( vm, 1 ) )
   {
      case VM::NUMBER:
      {
         uint idx = VM::toUInt( vm, 1 );
         const Variant& v = context->_world->attributes().get( idx );
         VM::push( vm, v );
         return 1;
      }  break;
      case VM::STRING:
      {
         ConstString key = VM::toCString( vm, 1 );
         const Variant& v = context->_world->attributes().get( key );
         VM::push( vm, v );
         return 1;
      }  break;
      default:
      {
         StdErr << "ProceduralWorld::getAttributeVM() - Invalid key type: " << VM::type( vm, 1 ) << "." << nl;
      }  break;
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int setAttributeVM( VMState* vm )
{
   WorldContext* context = getContext(vm);

   if( VM::getTop( vm ) != 2 )
   {
      StdErr << "ProceduralWorld::setAttributeVM() - Wrong number of arguments: expected 2, got " << VM::getTop( vm ) << "." << nl;
      return 0;
   }

   VM::toVariant( vm, context->_world->attributes() );

   return 0;
}

//------------------------------------------------------------------------------
//!
int probeVM( VMState* vm )
{
   WorldContext* context = getContext(vm);

   if( !VM::isTable( vm, 1 ) )
   {
      StdErr << "ProceduralWorld::probeVM() - Expects a table." << nl;
      return 0;
   }

   const char* str;
   if( !VM::get( vm, 1, "type", str ) )
   {
      StdErr << "ProceduralWorld::probeVM() - Missing type." << nl;
      return 0;
   }
   Probe::Type type = toProbeType( str );

   String id;
   if( !VM::get( vm, 1, "id", id ) )
   {
      StdErr << "ProceduralWorld::probeVM() - Missing id." << nl;
      return 0;
   }

   Vec3f pos;
   if( !VM::get( vm, 1, "position", pos ) )
   {
      StdErr << "ProceduralWorld::probeVM() - Missing position." << nl;
      return 0;
   }

   switch( type )
   {
      case Probe::CUBEMAP:
      {
         if( !VM::get( vm, 1, "image" ) )
         {
            StdErr << "ProceduralWorld::probeVM() - Missing cubemap image." << nl;
            return 0;
         }
         RCP<Image> img = (Image*)VM::toProxy( vm, -1 );
         VM::pop( vm );
         context->_world->addProbe( new CubemapProbe( id, pos, img.ptr() ) );
      }  break;
      default:
         CHECK( false );
         break;
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
const VM::Reg funcs[] = {
   // Extern functions.
   { "execute",              executeVM              },
   { "geometry",             geometryVM             },
   { "material",             materialVM             },
   { "animationGraph",       animationGraphVM       },
   // States.
   { "gravity",              gravityVM              },
   { "background",           backgroundVM           },
   // Object.
   { "camera",               cameraVM               },
   { "group",                groupVM                },
   { "kinematicObject",      kinematicObjectVM      },
   { "light",                lightVM                },
   { "object",               objectVM               },
   { "proxy",                proxyVM                },
   { "skeletalObject",       skeletalObjectVM       },
   { "staticObject",         staticObjectVM         },
   { "particleObject",       particleObjectVM       },
   // Constraints.
   { "ballJoint",            ballJointVM            },
   { "hingeJoint",           hingeJointVM           },
   // Brains and Receptors.
   { "brain",                brainVM                },
   { "contactReceptor",      contactReceptorVM      },
   { "contactGroupReceptor", contactGroupReceptorVM },
   // Attributes.
   { "getAttribute",         getAttributeVM         },
   { "setAttribute",         setAttributeVM         },
   // Probes.
   { "probe",                probeVM                },
   { 0,0 }
};

//------------------------------------------------------------------------------
//!
void initVM( VMState* vm, uint /*mask*/ )
{
   VM::registerFunctions( vm, "_G", funcs );
}

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   CLASS WorldContext
==============================================================================*/

WorldContext::~WorldContext()
{
#if 0
   StdErr << "Destroying world context with " << _others.size() << " others." << nl;
   for( uint i = 0; i < _others.size(); ++i )
   {
      StdErr << (void*)_others[i].ptr() << " count=" << _others[i]->count() << nl;
   }
#endif
}


/*==============================================================================
   CLASS ProceduralWorld
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
ProceduralWorld::initialize()
{
   VMRegistry::add( initVM, VM_CAT_WORLD );
}

//------------------------------------------------------------------------------
//!
ProceduralWorld::ProceduralWorld(
   Resource<World>* res,
   const String& id,
   const String& path
):
   _res( res ), _id( id ), _path( path )
{
}

//------------------------------------------------------------------------------
//!
ProceduralWorld::ProceduralWorld(
   Resource<World>* res,
   const String& id,
   const String& path,
   const Table& params
):
   _res( res ), _params( &params ), _id( id ), _path( path )
{
}

//------------------------------------------------------------------------------
//!
void
ProceduralWorld::execute()
{
   RCP<World> world( new World() );

   // Create working context for this vm.
   WorldContext context( this );
   context._world = world.ptr();
   context._ref   = Reff::identity();
   context.curDir( ResManager::dir(_id) );

   // Open vm.
   VMState* vm = VM::open( VM_CAT_WORLD | VM_CAT_MATH, true );

   // keep context pointer into vm.
   VM::userData( vm, &context );

   // Push parameters.
   if( _params.isValid() )
   {
      VM::push( vm, *_params );
      // Execute script.
      VM::doFile( vm, _path, 1, 0 );
   }
   else
   {
      // Execute script.
      VM::doFile( vm, _path, 0 );
   }

   // Wait for all auxilliary task (loading resource) to finish.
   waitForAll();

   // Assign all resources.
   // Geometry.
   for( size_t i = 0; i < context._geomRes.size(); ++i )
   {
      WorldContext::GeomEntityPair& p = context._geomRes[i];
      p.first->geometry( waitForData( p.second ) );
   }
   // MaterialSet.
   for( size_t i = 0; i < context._matRes.size(); ++i )
   {
      WorldContext::MatEntityPair& p = context._matRes[i];
      p.first->materialSet( waitForData( p.second ) );
   }
   // BrainProgram.
   for( size_t i = 0; i < context._brainRes.size(); ++i )
   {
      WorldContext::BrainProgPair& p = context._brainRes[i];
      p.first->program( waitForData( p.second ) );
   }

   // Animation graph.
   for( size_t i = 0; i < context._graphRes.size(); ++i )
   {
      WorldContext::SkelGraphPair& p = context._graphRes[i];
      // Retarget animation graph to skeleton.
      if( p.first->skeleton() )
      {
         RCP< Resource<AnimationGraph> > graph = ResManager::retarget(
            waitForData( p.second ), p.first->skeleton(), this
         );
         context.keepResource( graph.ptr() );
         p.second = graph.ptr();
      }
   }

   // Wait for all auxilliary task (loading resource) to finish.
   waitForAll();

   // At this point, the skeletal entity's animation graph has been retargetted, and the brains are set.
   for( size_t i = 0; i < context._graphRes.size(); ++i )
   {
      WorldContext::SkelGraphPair& p  = context._graphRes[i];
      SkeletalEntity*            skel = p.first;
      Brain*                    brain = skel->brain();
      AnimationGraph*           graph = waitForData( p.second );
      if( brain == NULL )
      {
         StdErr << "Attempted to set an animation graph (" << (void*)graph << ") on a brainless skeletal entity (" << (void*)skel << "); ignoring." << nl;
         continue;
      }
      // Connect the graph into the skeletal entity through a puppeteer action.
      PuppeteerAction* action = (PuppeteerAction*)brain->findAction( PuppeteerAction::actionType() );
      if( action == NULL )
      {
         action = new PuppeteerAction();
         brain->actions().pushBack( action );
      }
      action->entity( skel );
      action->graph( graph );
   }

   // Guarantee a material group in every entity.
   uint n = world->numEntities();
   for( uint i = 0; i < n; ++i )
   {
      Entity* e = world->entity( i );
      if( e->materialSet() == nullptr )
      {
         MaterialSet* ms = new MaterialSet();
         ms->add( Material::white() );
         e->materialSet( ms );
      }
   }

   VM::close( vm );

   _res->data( world.ptr() );
}

NAMESPACE_END
