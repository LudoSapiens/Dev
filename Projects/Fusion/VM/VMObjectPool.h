/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_OBJECT_POOL_H
#define FUSION_OBJECT_POOL_H

#include <Fusion/StdDefs.h>

#include <Fusion/VM/VM.h>
#include <Fusion/VM/VMRegistry.h>

#include <Base/ADT/ConstString.h>
#include <Base/ADT/Map.h>
#include <Base/ADT/Pair.h>
#include <Base/Util/RCObject.h>
#include <Base/Util/RCP.h>

NAMESPACE_BEGIN


/*==============================================================================
  CLASS VMObjectPool
==============================================================================*/
class VMObjectPool:
   public RCObject,
   public VMProxy
{
public:

   /*----- types -----*/

   typedef Map< String, VM::Function >  CreatorContainer;
   typedef Pair<RCP<RCObject>,VMProxy*> ObjPair;
   typedef Vector< ObjPair >            ObjectContainer;

   /*----- static methods -----*/

   static void  initialize();
   static void  terminate();

   static FUSION_DLL_API void  registerObject( 
      const ConstString&  collection,
      const char*         name,
      VM::Function        create,
      VM::Function        get,
      VM::Function        set
   );

   static FUSION_DLL_API void  registerCreate( 
      const ConstString&  collection,
      const char*         name,
      VM::Function        create
   );

   /*----- methods -----*/

   FUSION_DLL_API VMObjectPool( const ConstString& collection );
   FUSION_DLL_API virtual ~VMObjectPool();

   FUSION_DLL_API virtual const char*  meta() const;

   inline const ConstString&  collection() const { return _collection; }

   inline size_t  size() const                   { return _objects.size(); }
   inline   void  add( RCObject* o, VMProxy* p ) { _objects.pushBack( ObjPair(o,p) ); }
   inline   void  remove( VMProxy* o );

   void clear();

   void dbg();

   // Pseudo-private.
   VM::Function  findCreate( const String& name ) const;


protected:

   /*----- data members -----*/

   ConstString        _collection;  //!< The collection name.
   CreatorContainer*  _creators;    //!< The creator map for this collection.
   ObjectContainer    _objects;     //!< All the objects created by this pool.

private:
}; //class VMObjectPool

//------------------------------------------------------------------------------
//! 
inline void
VMObjectPool::remove( VMProxy* o )
{
   for( ObjectContainer::Iterator it = _objects.begin(); it != _objects.end(); ++it )
   {
      if( (*it).second == o )
      {
         // Put last element in its place.
         *it = _objects.back();
         _objects.popBack();
         return;
      }
   }
}

//------------------------------------------------------------------------------
//! A typedef for easier referencing.
typedef int (*ObjectCreationFunction)( VMState& vm, VMObjectPool& f );

//------------------------------------------------------------------------------
//! A routine to associate a Lua function name with its equivalent C routine.
void  registerFactoryFunction( const String& name, ObjectCreationFunction func );

//------------------------------------------------------------------------------
//! The common routine to register.
//! Sample usage:
//!   VMObjectPool::registerObject( "button", stdCreateVM<Button>, stdGetVM<Button>, stdSetVM<Button> );
//------------------------------------------------------------------------------
//!
template<typename T>
int stdGetVM( VMState* vm )
{
   T* w = (T*)VM::toProxy( vm, 1 );
   CHECK( w );
   return w->performGet( vm ) ? 1 : 0;
}

//------------------------------------------------------------------------------
//!
template<typename T>
int stdSetVM( VMState* vm )
{
   T* w = (T*)VM::toProxy( vm, 1 );
   CHECK( w );
   w->performSet( vm );
   return 0;
}

//------------------------------------------------------------------------------
//!
template<typename T>
int stdCreateVM( VMState* vm )
{
   VMObjectPool* f = (VMObjectPool*)VM::thisPtr( vm );
   T* w = new T();
   w->init( vm );
   f->add( w, w );
   VM::pushProxy( vm, w );
   return 1;
}


NAMESPACE_END

#endif //FUSION_OBJECT_POOL_H
