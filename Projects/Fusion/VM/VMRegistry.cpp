/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/VM/VMRegistry.h>

#include <Base/Dbg/DebugStream.h>

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( io_reg, "VMRegistry" );

const char* _VMProxyStr_ = "~VMProxy";

UNNAMESPACE_END

//------------------------------------------------------------------------------
//!
int
invalidVMProxy_get( VMState* vm )
{
   VMProxy* proxy = VM::toProxy( vm, 1 );
   StdErr << "ERROR - Get on deallocated proxy " << proxy << "\n";
   return 0;
}

//------------------------------------------------------------------------------
//!
int
invalidVMProxy_set( VMState* vm )
{
   VMProxy* proxy = VM::toProxy( vm, 1 );
   StdErr << "ERROR - Set on deallocated proxy " << proxy << "\n";
   return 0;
}

NAMESPACE_BEGIN

/*==============================================================================
   CLASS VMRegistry
==============================================================================*/

VMRegistry::ProxyEntryContainer     VMRegistry::_proxyEntries;
VMRegistry::InitEntryContainer      VMRegistry::_initEntries;
VMRegistry::FunctionEntryContainer  VMRegistry::_functionEntries;

//------------------------------------------------------------------------------
//!
void
VMRegistry::init( VMState* vm, uint mask )
{
   DBG_BLOCK( io_reg, "VMRegistry::init()" );

   DBG_MSG( io_reg, "Registering proxy meta-tables" );
   // Meta-table used to when a proxy is deleted in C++, but references still exist in Lua.
   VM::newMetaTable( vm, invalidMetaTableName() );       // [..., mt]
   VM::set( vm, -1, "__index"   , invalidVMProxy_get );  // [..., mt]
   VM::set( vm, -1, "__newindex", invalidVMProxy_set );  // [..., mt]
   VM::pop( vm, 1 );                                     // [...]

   for( ProxyEntryContainer::Iterator cur = _proxyEntries.begin(), end = _proxyEntries.end(); cur != end; ++cur )
   {
      const ProxyEntry& entry = *cur;

      if( mask & entry.mask() )
      {
         DBG_MSG_BEGIN( io_reg );
         io_reg << "Registering " << entry.name();
         io_reg << " (";
         io_reg <<  "gc="  << (void*)entry.gc();
         io_reg << ",get=" << (void*)entry.get();
         io_reg << ",set=" << (void*)entry.set();
         io_reg << ")";
         DBG_MSG_END( io_reg );

         VM::newMetaTable( vm, entry.name() );          // [..., mt]

         VM::set( vm, -1, "__gc"      , entry.gc()  );  // [..., mt]
         VM::set( vm, -1, "__index"   , entry.get() );  // [..., mt]
         VM::set( vm, -1, "__newindex", entry.set() );  // [..., mt]

         VM::pop( vm, 1 );                              // [...]
      }
   }

   DBG_MSG( io_reg, "Initialization routines" );
   for( InitEntryContainer::Iterator cur = _initEntries.begin(), end = _initEntries.end(); cur != end; ++cur )
   {
      const InitEntry& entry = *cur;

      if( mask & entry.mask() )
      {
         entry.function()( vm, mask );
      }
   }

   DBG_MSG( io_reg, "Function registrations" );
   for( FunctionEntryContainer::Iterator cur = _functionEntries.begin(), end = _functionEntries.end(); cur != end; ++cur )
   {
      const FunctionEntry& entry = *cur;

      if( mask & entry.mask() )
      {
         VM::registerFunction( vm, entry.name(), entry.function() );
      }
   }
}

//------------------------------------------------------------------------------
//!
const char*
VMRegistry::invalidMetaTableName()
{
   return _VMProxyStr_;
}


NAMESPACE_END
