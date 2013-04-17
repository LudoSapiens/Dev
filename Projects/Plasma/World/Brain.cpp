/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/World/Brain.h>

#include <Plasma/Plasma.h>
#include <Plasma/Resource/ResManager.h>
#include <Plasma/Stimulus/EventStimuli.h>
#include <Plasma/World/Entity.h>
#include <Plasma/World/World.h>

#include <Fusion/VM/VMRegistry.h>

#include <Base/ADT/StringMap.h>
#include <Base/Dbg/DebugStream.h>
#include <Base/MT/TaskQueue.h>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_brain, "Brain" );

//------------------------------------------------------------------------------
//!
int postVM( VMState* vm )
{
   BrainTaskContext* context = (BrainTaskContext*)VM::userData( vm );
   BrainTaskContext::CommandContainer& cmds = context->commands();

   int n = VM::getTop( vm );
   for( int i = 1; i <= n; ++i )
   {
      Command tmp;
      if( Command::to( vm, i, tmp ) )
      {
         cmds.pushBack( tmp );
      }
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int stimulateVM( VMState* vm )
{
   BrainTaskContext* context = (BrainTaskContext*)VM::userData( vm );
   BrainTaskContext::CommandContainer& cmds = context->commands();

   ConstString name = VM::toConstString( vm, 1 );
   RCP<Entity> e = context->entity()->world()->entity( name );
   if( e == nullptr )
   {
      StdErr << "ERROR - Trying to stimulate a non-existing entity '" << name << "'." << nl;
      return 0;
   }

   ConstString type = VM::toConstString( vm, 2 );
   RCP<Stimulus> s = Stimulus::create( type );
   if( s.isNull() )
   {
      StdErr << "ERROR - Unknown stimulus type '" << type << "'" << nl;
      return 0;
   }
   s->to( vm, 3 );

   cmds.pushBack( CmdStimulate( e.ptr(), s.ptr() ) );

   return 0;
}

//------------------------------------------------------------------------------
//!
int getAttributeVM( VMState* vm )
{
   BrainTaskContext* context = (BrainTaskContext*)VM::userData( vm );

   if( VM::getTop( vm ) != 1 )
   {
      StdErr << "Brain::getAttribute() - Wrong number of arguments: expected 1, got " << VM::getTop( vm ) << "." << nl;
      return 0;
   }

   Table* attr = context->entity()->attributes();
   if( attr == NULL )
   {
      VM::push( vm ); // Push nil (return 0 doesn't work).
      return 1;
   }

   switch( VM::type( vm, 1 ) )
   {
      case VM::NUMBER:
      {
         uint idx = VM::toUInt( vm, 1 );
         const Variant& v = attr->get( idx );
         VM::push( vm, v );
         return 1;
      }  break;
      case VM::STRING:
      {
         ConstString key = VM::toCString( vm, 1 );
         const Variant& v = attr->get( key );
         VM::push( vm, v );
         return 1;
      }  break;
      default:
      {
         StdErr << "Brain::getAttribute() - Invalid key type: " << VM::type( vm, 1 ) << "." << nl;
      }  break;
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
int setAttributeVM( VMState* vm )
{
   BrainTaskContext* context = (BrainTaskContext*)VM::userData( vm );

   if( VM::getTop( vm ) != 2 )
   {
      StdErr << "Brain::setAttribute() - Wrong number of arguments: expected 2, got " << VM::getTop( vm ) << "." << nl;
      return 0;
   }

   Entity* e = context->entity();
   if( e->attributes() == NULL )  e->attributes( new Table() );
   VM::toVariant( vm, *(e->attributes()) );

   return 0;
}

//------------------------------------------------------------------------------
//!
const VM::Reg brainFuncs[] = {
   { "getAttribute", getAttributeVM },
   { "post"        , postVM         },
   { "setAttribute", setAttributeVM },
   { "stimulate"   , stimulateVM    },
   { 0,0 }
};

//------------------------------------------------------------------------------
//!
void initVM( VMState* vm, uint /*mask*/ )
{
   VM::registerFunctions( vm, "_G", brainFuncs );
}

UNNAMESPACE_END


/*==============================================================================
  CLASS StimulusQueue
==============================================================================*/

//-----------------------------------------------------------------------------
//!
StimulusQueue::StimulusQueue()
{
}

//-----------------------------------------------------------------------------
//!
StimulusQueue::~StimulusQueue()
{
}

//-----------------------------------------------------------------------------
//!
void
StimulusQueue::post( Stimulus* s )
{
   LockGuard g( _lock );
   _incoming.pushBack( s );
}

//-----------------------------------------------------------------------------
//!
void
StimulusQueue::drain( StimulusQueue::Container& c )
{
   // Set c to the state we want _incoming to end up at.
   Container().swap(c); // We use this trick to guarantee the memory is deallocated; c.clear() doesn't.
   LockGuard g( _lock );
   c.swap( _incoming );
}


/*==============================================================================
  CLASS BrainProgramLoadTask
==============================================================================*/
void
BrainProgramLoadTask::execute()
{
   RCP<BrainProgram> prog = new BrainProgram();
   VMState* vm = (VMState*)storage("brain.vm");
   VM::getByteCode( vm, _path.cstr(), prog->_byteCode );
   _progRes->data( prog.ptr() );
}


/*==============================================================================
  CLASS Brain
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
Brain::initialize()
{
   VMRegistry::add( initVM, VM_CAT_BRAIN );
}

//-----------------------------------------------------------------------------
//!
void
Brain::initVMs()
{
   {
      // Brain VMs.
      TaskQueue* queue = Plasma::dispatchQueue();
      for( uint i = 0; i < queue->numAllocatedThreads(); ++i )
      {
         VMState* vm = VM::open( VM_CAT_BRAIN | VM_CAT_MATH, true );
         VM::userData( vm, new BrainTaskContext() );
         queue->setStorage( i, "brain.vm", vm );
      }
   }

   {
      // BrainProgramLoad VMs.
      TaskQueue* queue = ResManager::dispatchQueue();
      for( uint i = 0; i < queue->numAllocatedThreads(); ++i )
      {
         VMState* vm = VM::open( VM_CAT_BRAIN | VM_CAT_MATH, true );
         queue->setStorage( i, "brain.vm", vm );
      }
   }
}

//-----------------------------------------------------------------------------
//!
void
Brain::termVMs()
{
   {
      // BrainProgramLoad VMs.
      TaskQueue* queue = ResManager::dispatchQueue();
      for( uint i = 0; i < queue->numAllocatedThreads(); ++i )
      {
         VMState*& vm = (VMState*&)queue->getStorage( i, "brain.vm" );
         VM::close( vm );
         vm = NULL;
      }
   }
   {
      // Brain VMs.
      TaskQueue* queue = Plasma::dispatchQueue();
      for( uint i = 0; i < queue->numAllocatedThreads(); ++i )
      {
         VMState*& vm = (VMState*&)queue->getStorage( i, "brain.vm" );
         BrainTaskContext* context = (BrainTaskContext*)VM::userData( vm );
         delete context;
         VM::close( vm );
         vm = NULL;
      }
   }
}

//-----------------------------------------------------------------------------
//!
void
Brain::handlePostedCommands( World* world )
{
   TaskQueue* queue = Plasma::dispatchQueue();
   CHECK( queue->numTasks() == 0 );
   uint n = queue->numAllocatedThreads();
   for( uint i = 0; i < n; ++i )
   {
      VMState*& vm = (VMState*&)queue->getStorage( i, "brain.vm" );
      BrainTaskContext* context = (BrainTaskContext*)VM::userData( vm );
      world->handleCommands( context->commands() );
      context->commands().clear();
   }
}

//-----------------------------------------------------------------------------
//!
void
Brain::markEntitiesForAction( World* world )
{
   TaskQueue* queue = Plasma::dispatchQueue();
   CHECK( queue->numTasks() == 0 );
   uint n = queue->numAllocatedThreads();
   for( uint i = 0; i < n; ++i )
   {
      VMState*& vm              = (VMState*&)queue->getStorage( i, "brain.vm" );
      BrainTaskContext* context = (BrainTaskContext*)VM::userData( vm );
      // Drain entities with actions before pĥysics.
      auto& before              = context->entitiesWithActionsBefore();
      for( auto cur = before.begin(); cur != before.end(); ++cur )
      {
         CHECK( (*cur)->world() == world );
         world->markForActionBefore( (*cur) );
      }
      before.clear();
      // Drain entities with actions after physics.
      auto& after              = context->entitiesWithActionsAfter();
      for( auto cur = after.begin(); cur != after.end(); ++cur )
      {
         CHECK( (*cur)->world() == world );
         world->markForActionAfter( (*cur) );
      }
      after.clear();
   }
}

//------------------------------------------------------------------------------
//!
Brain::~Brain()
{
}

//-----------------------------------------------------------------------------
//!
Action*
Brain::findAction( const ConstString& type ) const
{
   for( auto cur = _actions.begin(); cur != _actions.end(); ++cur )
   {
      Action* action = (*cur).ptr();
      if( action->type() == type ) return action;
   }
   return NULL;
}

//-----------------------------------------------------------------------------
//!
void
Brain::postBegin()
{
   _stimuli.post( new BeginStimulus() );
}


/*==============================================================================
  CLASS BrainTask
==============================================================================*/

//------------------------------------------------------------------------------
//!
BrainTask::BrainTask( Entity* e ):
   _entity( e )
{
}

//------------------------------------------------------------------------------
//!
void
BrainTask::execute()
{
   if( _entity.isNull() )
   {
      StdErr << "ERROR - BrainTask::execute() called on a NULL entity." << nl;
      return;
   }

   Brain* brain = _entity->brain();
   if( brain == NULL )
   {
      StdErr << "ERROR - BrainTask::execute() called on an entity (" << (void*)_entity.ptr() << ") without a brain." << nl;
      return;
   }

   const BrainProgram* prog = brain->program();
   if( !prog )
   {
      StdErr << "ERROR - BrainTask::execute() no brain program." << nl;
      return;
   }

   if( !prog->byteCode().empty() )
   {
      // Run program in a VM.
      VMState* vm = (VMState*)storage( "brain.vm" );

      // Set the entity in the context.
      BrainTaskContext* context = (BrainTaskContext*)VM::userData( vm );
      context->entity( _entity.ptr() );

      // Send parameters.
      // 1. The brain's entity.
      VM::pushProxy( vm, _entity.ptr() );
      // 2. The brain's stimuli.
      StimulusQueue::Container stimuli;
      brain->_stimuli.drain( stimuli );
      VM::newTable( vm );
      uint idx = 1;
      for( auto cur = stimuli.begin(); cur != stimuli.end(); ++cur, ++idx )
      {
         const RCP<Stimulus>& stimulus = (*cur);
         stimulus->push( vm );
         VM::seti( vm, -2, idx );
      }

      // Execute brain bytecode.
      VM::doByteCode( vm, prog->byteCode(), 2, 0, "Brain" );
   }

   if( !prog->func().empty() )
   {
      // Call delegate (C++ program).
      prog->func()( _entity.ptr() );
   }
}
