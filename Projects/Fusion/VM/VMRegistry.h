/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_VM_REGISTRY_H
#define FUSION_VM_REGISTRY_H

#include <Fusion/StdDefs.h>

#include <Fusion/VM/VM.h>

#include <Base/ADT/Vector.h>


NAMESPACE_BEGIN

/*==============================================================================
   ENUM VMCategory
==============================================================================*/

enum VMCategory
{
   VM_CAT_CFG    = 0x0001,
   VM_CAT_APP    = 0x0002,
   VM_CAT_MATH   = 0x0004,
   VM_CAT_GEOM   = 0x0008,
   VM_CAT_WORLD  = 0x0010,
   VM_CAT_MAT    = 0x0020,
   VM_CAT_BRAIN  = 0x0040,
   VM_CAT_MESH   = 0x0080,
   VM_CAT_IMAGE  = 0x0100,
   VM_CAT_SKEL   = 0x0200,
   VM_CAT_ANIM   = 0x0400,
   VM_CAT_AGRAPH = 0x0800,
   VM_CAT_DF     = 0x1000
};

/*==============================================================================
  CLASS VMRegistry
==============================================================================*/
class VMRegistry
{
public:

   /*----- types -----*/

   typedef void (*InitFunction)( VMState*, uint ); 

   /*==============================================================================
     CLASS Entry
   ==============================================================================*/
   class ProxyEntry
   {
   public:
      ProxyEntry( uint mask, const char* name, VM::Function gc, VM::Function get, VM::Function set ):
         _mask( mask ), _name( name ), _gc( gc ), _get( get ), _set( set ) { }

      inline uint          mask() const { return _mask; }
      inline const char*   name() const { return _name; }

      inline VM::Function  gc()   const { return _gc;   }
      inline VM::Function  get()  const { return _get;  }
      inline VM::Function  set()  const { return _set;  }

   protected:
      
      /*----- data members -----*/

      uint          _mask;
      const char*   _name;  //!< The name of the meta-table.
      VM::Function  _gc;    //!< A function called when the VM collects an instance.
      VM::Function  _get;   //!< A function called when retrieving a value (including methods).
      VM::Function  _set;   //!< A function called when assigning a value.

   };

   /*==============================================================================
     CLASS InitEntry
   ==============================================================================*/
   class InitEntry
   {
   public:
      InitEntry( uint mask, InitFunction func ):
         _mask( mask ), _func( func ) { }

      inline uint          mask() const     { return _mask; }
      inline InitFunction  function() const { return _func; }

   protected:

      /*----- data members -----*/

      uint          _mask;
      InitFunction  _func;  //!< A function called when creating a vm.

   };

   /*==============================================================================
     CLASS FunctionEntry
   ==============================================================================*/
   class FunctionEntry
   {
   public:
      FunctionEntry( uint mask, const char* name, VM::Function func ):
         _mask( mask ), _name( name ), _func( func ) { }

      inline uint          mask() const     { return _mask; }
      inline const char*   name() const     { return _name; }
      inline VM::Function  function() const { return _func; }

   protected:

      /*----- data members -----*/

      uint          _mask;
      const char*   _name;
      VM::Function  _func;  //!< A function registered when creating a vm.
   };

   typedef Vector<ProxyEntry>     ProxyEntryContainer;
   typedef Vector<InitEntry>      InitEntryContainer;
   typedef Vector<FunctionEntry>  FunctionEntryContainer;

   /*----- static methods -----*/

   inline static void  add( const ProxyEntry& entry );
   inline static void  add( const char* mtName, VM::Function gc, VM::Function get, VM::Function set, uint mask = ~0 );

   inline static void  add( const InitEntry& entry );
   inline static void  add( InitFunction func, uint mask = ~0 );

   inline static void  add( const FunctionEntry& entry );
   inline static void  add( const char* name, VM::Function func, uint mask = ~0 );

   inline static const ProxyEntryContainer&     proxyEntries()    { return _proxyEntries;    }
   inline static const InitEntryContainer&      initEntries()     { return _initEntries;     }
   inline static const FunctionEntryContainer&  functionEntries() { return _functionEntries; }

   FUSION_DLL_API static void  init( VMState* vm, uint mask = ~0 );

   FUSION_DLL_API static const char*  invalidMetaTableName();

protected:

   /*----- static data members -----*/

   FUSION_DLL_API static ProxyEntryContainer     _proxyEntries;
   FUSION_DLL_API static InitEntryContainer      _initEntries;
   FUSION_DLL_API static FunctionEntryContainer  _functionEntries;

private:
}; //class VMRegistry

//------------------------------------------------------------------------------
//!
inline void
VMRegistry::add( const ProxyEntry& entry )
{
   _proxyEntries.pushBack( entry );
}

//------------------------------------------------------------------------------
//!
inline void
VMRegistry::add( const char* mtName, VM::Function gc, VM::Function get, VM::Function set, uint mask )
{
   add( ProxyEntry( mask, mtName, gc, get, set ) );
}

//------------------------------------------------------------------------------
//!
inline void
VMRegistry::add( const InitEntry& entry )
{
   _initEntries.pushBack( entry );
}

//------------------------------------------------------------------------------
//!
inline void
VMRegistry::add( InitFunction func, uint mask )
{
   add( InitEntry( mask, func ) );
}

//------------------------------------------------------------------------------
//!
inline void
VMRegistry::add( const FunctionEntry& entry )
{
   _functionEntries.pushBack( entry );
}

//------------------------------------------------------------------------------
//!
inline void
VMRegistry::add( const char* name, VM::Function func, uint mask )
{
   add( FunctionEntry( mask, name, func ) );
}

NAMESPACE_END

#endif //FUSION_VM_REGISTRY_H
