/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_VMOBJECT_H
#define FUSION_VMOBJECT_H

#include <Fusion/StdDefs.h>

#include <Fusion/VM/VM.h>

#include <Base/Dbg/Defs.h>

NAMESPACE_BEGIN

/*==============================================================================
  STRUCT VMObjectTraits
==============================================================================*/

template< class T >
struct VMObjectTraits
{
   static const char* getName();
   static const char* getMeta();
   static int create( VMState );
};

/*==============================================================================
  CLASS VMObject
==============================================================================*/

//!
template< class T, typename Traits = VMObjectTraits<T> >
class VMObject
{

public:

   /*----- static methods -----*/

   static int initialize( VMState* vm, const char* nameSpace )
   {
      // Create object metatable.
      VM::Reg meta[] = {
         { "__gc",       VM::gcRCObject },
         { "__index",    get },
         { "__newindex", set },
         { 0, 0 }
      };

      VM::newMetaTable( vm, Traits::getMeta() );
      VM::registerFunctions( vm, meta );
      VM::pop( vm, 1 );

      // Register creation function into the virtual machine.
      VM::Reg func[] = {
         { Traits::getName(), Traits::create }, 
         { 0, 0 }
      };
      VM::registerFunctions( vm, nameSpace, func );
      return 0;
   }

   static RCP<T> to( VMState* vm, int idx )
   {
#if defined(_DEBUG)
      int flags;
      T* result = (T*)VM::unboxPointer( vm, idx, flags );
      CHECK( (flags & VM::CONSTANT) == 0 );
      return RCP<T>( result );
#else
      return RCP<T>( (T*)VM::unboxPointer( vm, idx ) );
#endif
   }

   static RCP<const T> toConst( VMState* vm, int idx )
   {
      return RCP<const T>( (T*)VM::unboxPointer( vm, idx ) );
   }
   
   static void push( VMState* vm, const RCP<T>& object )
   {
      VM::push( vm, object, Traits::getMeta() );
   }
   
   static void pushConst( VMState* vm, const RCP<const T>& object )
   {
      VM::pushConst( vm, object, Traits::getMeta() );
   }

   static bool get( VMState* vm, int idx, const char* key, RCP<T>& result )
   {
      if( !VM::get( vm, idx, key ) )
      {
         return false;
      }      
      
      result = to( vm, -1 );
      VM::pop( vm, 1 );
      return true;
   }

   static bool getConst( VMState* vm, int idx, const char* key, RCP<const T>& result )
   {
      if( !VM::get( vm, idx, key ) )
      {
         return false;
      }      
      
      result = toConst( vm, -1 );
      VM::pop( vm, 1 );
      return true;
   }

   static bool getI( VMState* vm, int idx, int n, RCP<T>& result )
   {
      if( !VM::geti( vm, idx, n ) )
      {
         return false;
      }
      result = to( vm, -1 );
      VM::pop( vm, 1 );
      return true;
   }

   static bool getIConst( VMState* vm, int idx, int n, RCP<const T>& result )
   {
      if( !VM::geti( vm, idx, n ) )
      {
         return false;
      }
      result = toConst( vm, -1 );
      VM::pop( vm, 1 );
      return true;
   }
   
   static int get( VMState* vm )
   {
      RCP<const T> object = toConst( vm, 1 );
      // FIXME: Temporary work-around. performGet should be a const member
      //        everywhere, need to change it soon.
      return ((T*)object.ptr())->performGet( vm ) ? 1 : 0;
   }
   
   static int set( VMState* vm )
   {
      RCP<T> object = to( vm, 1 );
      object->performSet( vm );
      return 0;
   }
   
};

/*==============================================================================
  CLASS VMDObject
==============================================================================*/

//!
template< class T, typename Traits = VMObjectTraits<T> >
class VMDObject
{

public:

   /*----- static methods -----*/

   static int initialize( VMState* vm, const char* nameSpace )
   {
      // Register creation function into the virtual machine.
      VM::Reg func[] = {
         { Traits::getName(), Traits::create }, 
         { 0, 0 }
      };
      VM::registerFunctions( vm, nameSpace, func );
      return 0;
   }

   static RCP<T> to( VMState* vm, int idx )
   {
      return RCP<T>( (T*)VM::unboxPointer( vm, idx ) );
   }

   static void push( VMState* vm, const RCP<T>& object )
   {
      VM::push( vm, object, Traits::getMeta() );
   }   
};

/*==============================================================================
  
==============================================================================*/

#define VMOBJECT_TRAITS( Class, name ) VMDOBJECT_TRAITS( Class, name, name )

#define VMDOBJECT_TRAITS( Class, name, meta )   \
   template<> struct VMObjectTraits< Class >    \
   {                                            \
      static const char* getName()              \
      {                                         \
         return #name;                          \
      }                                         \
                                                \
      static const char* getMeta()              \
      {                                         \
         return #meta;                          \
      }                                         \
                                                \
      static int create( VMState* vm )          \
      {                                         \
         RCP<Class> object( new Class() );      \
         object->init( vm );                    \
         VM::push( vm, object, #meta );         \
         return 1;                              \
      }                                         \
   };

//------------------------------------------------------------------------------
//!
#define VMPUREOBJECT_TRAITS( Class, name ) VMDPUREOBJECT_TRAITS( Class, name, name )

#define VMDPUREOBJECT_TRAITS( Class, name, meta )\
   template<> struct VMObjectTraits< Class >    \
   {                                            \
      static const char* getName()              \
      {                                         \
         return #name;                          \
      }                                         \
                                                \
      static const char* getMeta()              \
      {                                         \
         return #meta;                          \
      }                                         \
                                                \
      static int create( VMState* vm )          \
      {                                         \
         VM::push( vm );                        \
         return 1;                              \
      }                                         \
   };

NAMESPACE_END

#endif
