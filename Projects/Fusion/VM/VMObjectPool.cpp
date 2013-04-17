/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/VM/VMObjectPool.h>
#include <Fusion/VM/VMRegistry.h>
#include <Fusion/Widget/Widget.h>

#include <Base/ADT/StringMap.h>
#include <Base/ADT/Vector.h>
#include <Base/Dbg/DebugStream.h>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_op, "VMObjectPool" );

const char* _objectPoolStr_ = "ObjectPool";

ConstString  _uiStr_;

/*==============================================================================
  COLLECTION CONTAINER.
==============================================================================*/

Map< ConstString, VMObjectPool::CreatorContainer >  _collections;


/*==============================================================================
  POOL CONTAINER.
==============================================================================*/

Vector< RCP<VMObjectPool> >  _pools;  // A container for all of the pools.

//------------------------------------------------------------------------------
//!
void  registerPool( VMObjectPool* p )
{
   _pools.pushBack( p );
}

//------------------------------------------------------------------------------
//!
void  unregisterPool( VMObjectPool* p )
{
   _pools.removeSwap( p );
}

/*==============================================================================
  VM BINDINGS.
==============================================================================*/

//------------------------------------------------------------------------------
//!
enum {
   ATTRIB_CLEAR,
   ATTRIB_REMOVE
};

StringMap _attributes(
   "clear",  ATTRIB_CLEAR,
   "remove", ATTRIB_REMOVE,
   ""
);

//------------------------------------------------------------------------------
//!
int object_create( VMState* vm )
{
   int numArgs = VM::getTop(vm);
   VM::pushValue( vm, VM::upvalue(1) );       // [...,argn,func]
   VM::insert( vm, 1 );                       // [func,...,argn]
   VM::pushValue( vm, VM::upvalue(2) );       // [func,...,argn,pool]
   VM::insert( vm, 2 );                       // [func,pool,...,argn]
   VM::ecall( vm, numArgs+1, 1 );             // [result]
   return 1;
}

//------------------------------------------------------------------------------
//!
int objectPool_create( VMState* vm )
{
   ConstString collection;

   if( VM::getTop( vm ) == 0 )
   {
      collection = _uiStr_;
   }
   else
   {
      collection = ConstString( VM::toCString( vm, 1 ) );
   }

   RCP<VMObjectPool> p = new VMObjectPool( collection );
   DBG_BLOCK( os_op, "objectPool_create " << (void*)p.ptr() );
   VMProxy* proxy = p.ptr();
   VM::pushProxy( vm, proxy );
   return 1;
}

//------------------------------------------------------------------------------
//!
int objectPool_clear( VMState* vm )
{
   VMObjectPool* p = (VMObjectPool*)VM::thisPtr( vm );
   DBG_BLOCK( os_op, "objectPool_clear " << (void*)p );
   CHECK( p );
   p->clear();
   return 0;
}

//------------------------------------------------------------------------------
//! 
int objectPool_remove( VMState* vm )
{
   VMObjectPool* p = (VMObjectPool*)VM::thisPtr( vm );
   VMProxy* pr     = VM::toProxy( vm, 1 );
   p->remove( pr );
   return 0;
}

//------------------------------------------------------------------------------
//!
int objectPool_gc( VMState* vm )
{
   VMObjectPool* p = (VMObjectPool*)VM::toProxy( vm, 1 );
   DBG_BLOCK( os_op, "objectPool_gc " << (void*)p );
   unregisterPool( p );
   return 0;
}

//------------------------------------------------------------------------------
//!
int objectPool_get( VMState* vm )
{
   VMObjectPool* p = (VMObjectPool*)VM::toProxy( vm, 1 );
   DBG_BLOCK( os_op, "objectPool_get " << p );
   CHECK( p );

   const char* cstr = VM::toCString( vm, -1 );

   // Check if it is a registered create routine.
   String str = cstr;
   VM::Function func = p->findCreate( str );
   if( func )
   {
      DBG_MSG( os_op, "Is a registered creator." );
      VM::push( vm, p, func );
      return 1;
   }

   if( p->collection() == _uiStr_ )
   {
      // Check if it is a registered create vm routine.
      VM::getGlobal( vm, "UI" );             // [..., objectName, UI]
      VM::pushValue( vm, -2 );               // [..., objectName, UI, objectName]
      VM::get( vm, -2 );                     // [..., objectName, UI, function]

      if( !VM::isNil( vm, -1 ) )
      {
         DBG_MSG( os_op, "Is a registered vm creator." );
         VM::pushValue( vm, 1 );             // [..., objectName, UI, function, p]
         VM::push( vm, object_create, 2 );
         return 1;
      }
      else
      {
         VM::pop( vm, 2 );                   // [..., objectName]
      }
   }

   DBG_MSG( os_op, "Is not a registered creator." );

   // Check if it is a direct command.
   switch( _attributes[cstr] )
   {
      case ATTRIB_CLEAR:
         VM::push( vm, p, objectPool_clear );
         return 1;
      case ATTRIB_REMOVE:
         VM::push( vm, p, objectPool_remove );
         return 1;
      default:
         DBG_MSG( os_op, "objectPool_get - '" << VM::toCString(vm, -1) << "' unknown." );
         break;
   } //switch..case

   return 0;
}

//------------------------------------------------------------------------------
//!
int
objectPool_set( VMState* vm )
{
   //VMObjectPool* p = (VMObjectPool*)VM::toProxy( vm, 1 );
   unused( vm );
   DBG_BLOCK( os_op, "objectPool_set " << (void*)VM::toProxy( vm, 1 ) );
   return 0;
}

//------------------------------------------------------------------------------
//!
void
initVM( VMState* vm, uint /*mask*/ )
{
   VM::registerFunction( vm, _objectPoolStr_, objectPool_create );
}

UNNAMESPACE_END

/*==============================================================================
  CLASS VMObjectPool
==============================================================================*/

//------------------------------------------------------------------------------
//!
const char*
VMObjectPool::meta() const
{
   return _objectPoolStr_;
}

//------------------------------------------------------------------------------
//! Register all of the object creation callbacks to use.
void
VMObjectPool::initialize()
{
   _uiStr_ = "UI";
   VMRegistry::add( _objectPoolStr_, objectPool_gc, objectPool_get, objectPool_set, VM_CAT_APP );
   VMRegistry::add( initVM, VM_CAT_APP );
}

//------------------------------------------------------------------------------
//!
void
VMObjectPool::terminate()
{
   _uiStr_ = ConstString();
   _collections.clear();
}

//------------------------------------------------------------------------------
//!
void
VMObjectPool::registerObject(
   const ConstString&  collection,
   const char*         name,
   VM::Function        create,
   VM::Function        get,
   VM::Function        set
)
{
   CreatorContainer& creators = _collections[collection];
   CHECK( !creators.has(name) );
   creators[name] = create;
   VMRegistry::add( name, NULL, get, set, VM_CAT_APP );
   DBG_MSG( os_op, "Registered object as '" << name << "' --> " << (void*)create );
}

//------------------------------------------------------------------------------
//!
void
VMObjectPool::registerCreate(
   const ConstString&  collection,
   const char*         name,
   VM::Function        create
)
{
   CreatorContainer& creators = _collections[collection];
   CHECK( !creators.has(name) );
   creators[name] = create;
   DBG_MSG( os_op, "Registered create as '" << name << "' --> " << (void*)create );
}

//------------------------------------------------------------------------------
//!
VM::Function
VMObjectPool::findCreate( const String& name ) const
{
   CreatorContainer::Iterator it = _creators->find( name );
   if( it != _creators->end() )
   {
      VM::Function func = (*it).second;
      DBG_MSG( os_op, "Found '" << name << "' --> " << (void*)func );
      return func;
   }
   DBG_MSG( os_op, "Did not find '" << name << "'" );
   return NULL;
}

//------------------------------------------------------------------------------
//!
VMObjectPool::VMObjectPool( const ConstString& collection ):
   _collection( collection )
{
   DBG_BLOCK( os_op, "VMObjectPool::ctor " << _collection << " " << (void*)this << " " << (void*)(VMProxy*)this );
   _creators = &_collections[collection];
   registerPool( this );
}

//------------------------------------------------------------------------------
//!
VMObjectPool::~VMObjectPool()
{
   DBG_BLOCK( os_op, "VMObjectPool::dtor " << _collection << " " << (void*)this << " " << (void*)(VMProxy*)this );
}

//------------------------------------------------------------------------------
//!
void
VMObjectPool::clear()
{
   _objects.clear();
}

//------------------------------------------------------------------------------
//! 
void
VMObjectPool::dbg()
{
   StdErr << _collection << ":" << nl;
   for( uint i = 0; i < _objects.size(); ++i )
   {
      StdErr << _objects[i].first.ptr() << " " << _objects[i].second << "\n";
   }
}
