/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/VM/VM.h>
#include <Fusion/VM/VMRegistry.h>
#include <Fusion/VM/VMObjectPool.h>
#include <Fusion/VM/VMMath.h>
#include <Fusion/Core/Event.h>
#include <Fusion/Core/Pointer.h>
#include <Fusion/Widget/Widget.h>

#include <CGMath/Variant.h>

#include <Base/Dbg/Defs.h>
#include <Base/Dbg/DebugStream.h>

extern "C" {
#include <lauxlib.h>
#include <lualib.h>
}

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

DBG_STREAM( io_vmp, "VMProxy" );

struct VMProxyUD
{
   VMProxy*     _proxy;
   const char*  _meta;
};

//------------------------------------------------------------------------------
//!
int errorWritesAreDisabledVM( VMState* vm )
{
   StdErr << "ERROR: Writes are disabled.\n";
   VM::printCallStack( vm );
   return 0;
}

//------------------------------------------------------------------------------
//!
int nullFunctionVM( VMState* /*vm*/ )
{
   return 0;
}

//------------------------------------------------------------------------------
//!
int silentNullReadVM( VMState* vm )
{
   lua_rawget( vm, 1 );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pushcfunction( vm, nullFunctionVM );
   }
   return 1;
}

//------------------------------------------------------------------------------
//!
VM::Reg _libs[] = {
   { "",              luaopen_base   },
   { LUA_TABLIBNAME,  luaopen_table  },
   { LUA_STRLIBNAME,  luaopen_string },
   { LUA_MATHLIBNAME, luaopen_math   },
   { LUA_OSLIBNAME,   luaopen_os     },
   { LUA_IOLIBNAME,   luaopen_io     },
#if defined(_DEBUG)
   { LUA_DBLIBNAME,   luaopen_debug  },
#endif
   {0,0}
};

//------------------------------------------------------------------------------
//! convert a stack index to positive
inline int
absIndex( VMState* vm, int i )
{
   return i > 0 || i <= LUA_REGISTRYINDEX ? i : lua_gettop( vm ) + i + 1;
}

//------------------------------------------------------------------------------
//!
int
readOnlyNewIndex( VMState* /*vm*/ )
{
   printf("Attempt to assign a new value into a read-only table.\n");
   CHECK( false );
   return 0;
}

struct ReaderState
{
   /*----- methods -----*/
   inline ReaderState( const VMByteCode& byteCode ): _byteCode( &byteCode ) {}
   /*----- members -----*/
   const VMByteCode*  _byteCode;
};

//------------------------------------------------------------------------------
//!
const char* readByteCode( VMState* /*vm*/, void* ud, size_t* sz )
{
   ReaderState* state = (ReaderState*)ud;
   const VMByteCode*& byteCodePtr = state->_byteCode;
   if( byteCodePtr != NULL )
   {
      *sz = byteCodePtr->size();
      const char* cur = (const char*)byteCodePtr->data();
      byteCodePtr = NULL; // Flag as done for next call.
      return cur;
   }
   else
   {
      return NULL;
   }
}

//------------------------------------------------------------------------------
//!
int writeByteCode( VMState* /*vm*/, const void* p, size_t sz, void* ud )
{
   VMByteCode* bc = (VMByteCode*)ud;
   CHECK( bc );
   size_t s = bc->size();
   bc->resize( s + sz );
   memcpy( bc->data() + s, p, sz );
   return 0;
}

//------------------------------------------------------------------------------
//!
VMAddress*  getVMAddress( VMState* vm )
{
   lua_pushlightuserdata( vm, (int*)vm+1 ); // Key to VMAddress of vm.
   lua_rawget( vm, LUA_REGISTRYINDEX );
   VMAddress* vma = (VMAddress*)lua_touserdata( vm, -1 );
   lua_pop( vm, 1 );
   return vma;
}

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   CLASS VMAddress
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
VMAddress::increment( void* ptr, int idx )
{
   if( ptr == nullptr )  return;

   VMAddress::CountTable::Iterator it = _counter.find( ptr );
   if( it == _counter.end() )
   {
      // Create a new counter.
      _counter.add( ptr, 1 );
      lua_pushlightuserdata( _vm, ptr );     // [..., ptr]
      lua_pushvalue( _vm, idx );             // [..., ptr, refCopy]
      lua_rawset( _vm, LUA_REGISTRYINDEX );  // [...]
   }
   else
   {
      // Increment the existing counter.
      ++(it.data());
   }
}

//------------------------------------------------------------------------------
//!
void
VMAddress::increment( void* ptr )
{
   if( ptr == nullptr )  return;

   VMAddress::CountTable::Iterator it = _counter.find( ptr );
   if( it != _counter.end() )
   {
      // Increment the existing counter.
      ++(it.data());
   }
   else
   {
      CHECK( false );
   }
}

//------------------------------------------------------------------------------
//!
void
VMAddress::decrement( void* ptr )
{
   if( ptr == nullptr )  return;

   if( --_counter[ptr] == 0 )
   {
      _counter.erase( ptr );
      if( _vm )
      {
         lua_pushlightuserdata( _vm, ptr );     // [..., ptr]
         lua_pushnil( _vm );                    // [..., ptr, nil]
         lua_rawset( _vm, LUA_REGISTRYINDEX );  // [...]
      }
   }
}


/*==============================================================================
   CLASS VMRef
==============================================================================*/

VMRef::~VMRef()
{
   _vma->decrement( _ptr );
}

//------------------------------------------------------------------------------
//!
VMRef&
VMRef::operator=( const VMRef& ref )
{
   _vma->decrement( _ptr );

   _vma = ref._vma;
   _ptr = ref._ptr;

   _vma->increment( _ptr );

   return *this;
}

//------------------------------------------------------------------------------
//!
void
VMRef::set( VMState* vm, int idx )
{
   idx = ::absIndex( vm, idx );

   _vma->decrement( _ptr );

   _vma = getVMAddress( vm );
   _ptr = (void*)lua_topointer( vm, idx );

   _vma->increment( _ptr, idx );
}

//------------------------------------------------------------------------------
//!
VMState*
VMRef::vm() const
{
   return _vma->_vm;
}


/*==============================================================================
  CLASS VM
==============================================================================*/

VM::Function  VM::readsDisabledVM  = silentNullReadVM;
VM::Function  VM::writesDisabledVM = errorWritesAreDisabledVM;
int           VM::_upidx           = lua_upvalueindex(0);

//------------------------------------------------------------------------------
//!
void
VM::printInfo( TextStream& os )
{
   os << "VM: ";
   os << " Lua(" << LUA_RELEASE << ")";
   os << nl;
}

//------------------------------------------------------------------------------
//!
VMState*
VM::open( uint mask, bool localsOnly )
{
   // If cfg is true, use special config file mode where unknown globals return null functions.
   bool cfg = (mask & VM_CAT_CFG) != 0x0;
   Function get = cfg        ? VM::readsDisabledVM  : nullptr;
   Function set = localsOnly ? VM::writesDisabledVM : nullptr;
   return open( mask, get, set );
}

//------------------------------------------------------------------------------
//!
VMState*
VM::open( uint mask, Function get, Function set )
{
   // Just a sanity check to detect if Lua ever changes their lua_upvalueindex() macro,
   // in which case we'll need to modify VM::upvalue() to match it.
   CHECK( lua_upvalueindex(3) == VM::upvalue(3) );

   // Allocate the VMState.
   VMState* vm = lua_open();

   // Register proxy table as a weak table.
   // The proxy table is kept in the registry at the address of the vm.
   lua_pushlightuserdata( vm, vm );   // [key, proxyTable]
   lua_newtable( vm );                // [key, proxyTable]
   lua_newtable( vm );                // [key, proxyTable, metaTable]
   lua_pushstring( vm, "v" );         // [key, proxyTable, metaTable, "v"]
   lua_setfield( vm, -2, "__mode" );  // [key, proxyTable, metaTable]
   lua_setmetatable( vm, -2 );        // [key, proxyTable]
   lua_rawset( vm, LUA_REGISTRYINDEX );

   // Create VMAdress.
   RCP<VMAddress> address( new VMAddress() );
   address->_vm = vm;
   address->addReference();
   lua_pushlightuserdata( vm, (int*)vm+1 );    // [key]
   lua_pushlightuserdata( vm, address.ptr() ); // [key, address]
   lua_rawset( vm, LUA_REGISTRYINDEX );

   // Load standard librairies.
   const Reg* lib = _libs;
   for( ; lib->func; ++lib )
   {
      lua_pushcfunction( vm, lib->func );
      lua_pushstring( vm, lib->name );
      lua_call( vm, 1, 0 );
   }

#ifndef _DEBUG
   // Disable unwanted functions.
   VM::push( vm );
   VM::setGlobal( vm, "collectgarbage" );
   VM::push( vm );
   VM::setGlobal( vm, "dofile" );
   VM::push( vm );
   VM::setGlobal( vm, "error" );
   VM::push( vm );
   VM::setGlobal( vm, "load" );
   VM::push( vm );
   VM::setGlobal( vm, "loadfile" );
   VM::push( vm );
   VM::setGlobal( vm, "setmetatable" );
#endif

   // Add registered routines.
   VMRegistry::init( vm, mask );

   if( get || set )
   {
      // Need a meta-table.
      lua_newtable( vm );
      if( get )
      {
         lua_pushcfunction( vm, get );
         lua_setfield( vm, -2, "__index" );
      }
      if( set )
      {
         lua_pushcfunction( vm, set );
         lua_setfield( vm, -2, "__newindex" );
      }
      lua_setmetatable( vm, LUA_GLOBALSINDEX );
   }

   return vm;
}

//------------------------------------------------------------------------------
//!
void
VM::close( VMState* vm )
{
   if( vm == NULL )  return;

   // Remove VMAddress.
   lua_pushlightuserdata( vm, (int*)vm+1 );
   lua_rawget( vm, LUA_REGISTRYINDEX );
   VMAddress* address = (VMAddress*)lua_touserdata( vm, -1 );
   lua_pop( vm, 1 );
   address->_vm = 0;
   address->removeReference();

   // Deallocate the VMState.
   lua_close( vm );
}

//------------------------------------------------------------------------------
//!
void*
VM::userData( VMState* vm )
{
   lua_pushlightuserdata( vm, (int*)vm+2 );
   lua_rawget( vm, LUA_REGISTRYINDEX );
   void* data = lua_touserdata( vm, -1 );
   lua_pop( vm, 1 );
   return data;
}

//------------------------------------------------------------------------------
//!
void
VM::userData( VMState* vm, void* data )
{
   lua_pushlightuserdata( vm, (int*)vm+2 );
   lua_pushlightuserdata( vm, data );
   lua_rawset( vm, LUA_REGISTRYINDEX );
}

//------------------------------------------------------------------------------
//!
int
VM::loadFile( VMState* vm, const Path& fileName )
{
   return luaL_loadfile( vm, fileName.cstr() );
}

//------------------------------------------------------------------------------
//!
void
VM::doFile( VMState* vm, const String& fileName, int nresults )
{
   // Load script file and execute it.
   if( VM::loadFile( vm, fileName ) != 0  )
   {
      StdErr << "Error when loading: " << VM::toString( vm, -1 ) << nl;
      return;
   }

   VM::ecall( vm, 0, nresults );
}

//------------------------------------------------------------------------------
//!
void
VM::doFile( VMState* vm, const String& fileName, int nargs, int nresults )
{
   // Load script file and execute it.
   if( VM::loadFile( vm, fileName ) != 0  )
   {
      StdErr << "Error when loading: " << VM::toString( vm, -1 ) << nl;
      return;
   }

   // Move the function code right before the arguments already on the stack.
   lua_insert( vm, -nargs-1 );

   VM::ecall( vm, nargs, nresults );
}

//------------------------------------------------------------------------------
//!
int
VM::doString( VMState* vm, const String& string )
{
   return luaL_dostring( vm, string.cstr() );
}

//------------------------------------------------------------------------------
//!
void
VM::setArguments( VMState* vm, const Vector<String>& arguments )
{
   return;
   pushTable( vm, "arg" ); // This routine checks for existence and such
   for( uint i = 0; i < (uint)arguments.size(); ++i )
   {
      lua_pushstring( vm, arguments[i].cstr() );
      lua_rawseti( vm, -2, i+1 );
   }
}

//------------------------------------------------------------------------------
//!
int
VM::getByteCode( VMState* vm, VMByteCode& dst )
{
   return lua_dump( vm, writeByteCode, &dst );
}

//------------------------------------------------------------------------------
//!
int
VM::getByteCode( VMState* vm, const Path& fileName, VMByteCode& dst )
{
   if( luaL_loadfile( vm, fileName.cstr() ) != 0 )
   {
      StdErr << "Error when loading: " << VM::toString( vm, -1 ) << nl;
      return 1;
   }

   return lua_dump( vm, writeByteCode, &dst );
}

//------------------------------------------------------------------------------
//!
int
VM::loadByteCode( VMState* vm, const VMByteCode& src, const char* chunkName )
{
   ReaderState state( src );
   return lua_load( vm, readByteCode, &state, chunkName );
}

//------------------------------------------------------------------------------
//!
int
VM::doByteCode( VMState* vm, const VMByteCode& src, int nresults, const char* chunkName )
{
   return doByteCode( vm, src, 0, nresults, chunkName );
}

//------------------------------------------------------------------------------
//!
int
VM::doByteCode( VMState* vm, const VMByteCode& src, int nargs, int nresults, const char* chunkName )
{
   ReaderState state( src );
   int err = lua_load( vm, readByteCode, &state, chunkName );
   if( err )
   {
      // Lua should already print an error.
      //StdErr << "Error when executing byte code '" << chunkName << "'" << nl;
      return err;
   }

   if( nargs != 0 )
   {
      // Must move function pointer below the arguments.
      lua_insert( vm, lua_gettop( vm ) - nargs );
   }

   err = lua_pcall( vm, nargs, nresults, 0 );
   if( err )
   {
      StdErr << "Error when executing byte code '" << chunkName << "'" << nl;
      StdErr << src.size() << "B, nargs=" << nargs << " nresults=" << nresults << nl;
      // Print top of stack (apparently has an error message).
      int t = lua_type(vm, -1);
      switch( t )
      {
         case LUA_TSTRING:
            printf("%s\n", lua_tostring(vm, -1));
            break;
         case LUA_TBOOLEAN:
            printf(lua_toboolean(vm, -1) ? "true\n" : "false\n");
            break;
         case LUA_TNUMBER:
            printf("%g\n", lua_tonumber(vm, -1));
            break;
         default:
           printf("Some %s\n", lua_typename(vm, t));
           break;
      }
   }

   return err;
}

//------------------------------------------------------------------------------
//!
void
VM::ecall( VMState* vm, int nargs, int nresults )
{
   if( lua_pcall( vm, nargs, nresults, 0 ) != 0 )
   {
      StdErr << "Error when executing: " << lua_tostring( vm, -1 ) << nl;
   }
}

//------------------------------------------------------------------------------
//!
int
VM::getTop( VMState* vm )
{
   return lua_gettop( vm );
}

//------------------------------------------------------------------------------
//!
void
VM::setTop( VMState* vm, int n )
{
   lua_settop( vm, n );
}

//-----------------------------------------------------------------------------
//!
int
VM::absIndex( VMState* vm, int idx )
{
   return ::absIndex( vm, idx );
}

//------------------------------------------------------------------------------
//!
void
VM::pop( VMState* vm, int n )
{
   lua_pop( vm, n );
}

//------------------------------------------------------------------------------
//!
void
VM::insert( VMState* vm, int idx )
{
   lua_insert( vm, idx );
}

//------------------------------------------------------------------------------
//!
void
VM::remove( VMState* vm, int idx )
{
   lua_remove( vm, idx );
}

//------------------------------------------------------------------------------
//!
void
VM::replace( VMState* vm, int idx )
{
   lua_replace( vm, idx );
}

//------------------------------------------------------------------------------
//!
void
VM::getGlobal( VMState* vm, const char* var )
{
   lua_getglobal( vm, var );
}

//------------------------------------------------------------------------------
//!
void
VM::setGlobal( VMState* vm, const char* var )
{
   //lua_setglobal( vm, var );
   lua_pushstring( vm, var );
   lua_insert( vm, -2 );
   lua_rawset( vm, LUA_GLOBALSINDEX );
}

//------------------------------------------------------------------------------
//!
void*
VM::thisPtr( VMState* vm )
{
   return VM::toPtr( vm, upvalue(1) );
}

//------------------------------------------------------------------------------
//!
int
VM::type( VMState* vm, int idx )
{
   return lua_type( vm, idx );
}

//------------------------------------------------------------------------------
//!
bool
VM::isBoolean( VMState* vm, int idx )
{
   return lua_isboolean( vm, idx );
}

//------------------------------------------------------------------------------
//!
bool
VM::isFunction( VMState* vm, int idx )
{
   return lua_isfunction( vm, idx );
}

//------------------------------------------------------------------------------
//!
bool
VM::isNil( VMState* vm, int idx )
{
   return lua_isnil( vm, idx );
}

//------------------------------------------------------------------------------
//!
bool
VM::isNumber( VMState* vm, int idx )
{
   return lua_isnumber( vm, idx ) != 0;
}

//------------------------------------------------------------------------------
//!
bool
VM::isObject( VMState* vm, int idx )
{
   return lua_isuserdata( vm, idx ) != 0;
}

//------------------------------------------------------------------------------
//!
bool
VM::isString( VMState* vm, int idx )
{
   return lua_isstring( vm, idx ) != 0;
}

//------------------------------------------------------------------------------
//!
bool
VM::isTable( VMState* vm, int idx )
{
   return lua_istable( vm, idx );
}

//------------------------------------------------------------------------------
//!
void*
VM::unboxPointer( VMState* vm, int idx, int& flags )
{
   void** ud = (void**)lua_touserdata( vm, idx );
   if( ud != NULL )
   {
      const int* udip = (const int*)(ud + 1);
      flags = *udip;
      return ud[0];
   }
   else
   {
      flags = 0;
      return NULL;
   }
}

//------------------------------------------------------------------------------
//!
void*
VM::unboxPointer( VMState* vm, int idx )
{
   void** ud = (void**)lua_touserdata( vm, idx );
   return (ud != NULL) ? *ud : NULL;
}

//------------------------------------------------------------------------------
//!
void*
VM::newObject( VMState* vm, uint size )
{
   return lua_newuserdata( vm, size );
}

//------------------------------------------------------------------------------
//! Pushes a VMProxy on the stack as a userdata.
void
VM::pushProxy( VMState* vm, VMProxy* proxy )
{
   DBG_BLOCK( io_vmp, "VM::pushProxy( VMProxy=" << (void*)proxy << " )" );
   if( proxy )
   {
      // Check to see if it already exists.
      // Retrieve current proxy (if any).
      lua_pushlightuserdata( vm, vm );                                       // [..., key]
      lua_rawget( vm, LUA_REGISTRYINDEX );                                   // [..., proxiesTable]
      lua_pushlightuserdata( vm, proxy );                                    // [..., proxiesTable, proxyPtr]
      lua_rawget( vm, -2 );                                                  // [..., proxiesTable, proxyUD]
      VMProxyUD* ud = (VMProxyUD*)lua_touserdata( vm, -1 );                  // [..., proxiesTable, proxyUD]
      if( ud == NULL )
      {
         // Doesn't exist, need to create it.
         // 1. Remove the current nil reference.                             // [..., proxiesTable, nil]
         lua_pop( vm, 1 );                                                   // [..., proxiesTable]
         // 2. Push new pointer on stack.
         ud = (VMProxyUD*)lua_newuserdata( vm, sizeof(VMProxyUD) );          // [..., proxiesTable, proxyUD]
         DBG_BLOCK( io_vmp, "Created VMProxyUD*: " << (void*)ud );
         ud->_proxy = proxy;
         ud->_meta  = NULL; // Invalid _meta... solved in the next block.
         // 3. Register the proxyUD using the proxy pointer as key.
         // 3a. Push the key.
         lua_pushlightuserdata( vm, proxy );                                 // [..., proxiesTable, proxyUD, proxyPtr]
         // 3b. Push the value.
         lua_pushvalue( vm, -2 );                                            // [..., proxiesTable, proxyUD, proxyPtr, proxyUD]
         // 3c. Store in the table.
         lua_rawset( vm, -4 );                                               // [..., proxiesTable, proxyUD]
      }

      const char* meta = proxy->meta();
      if( ud->_meta != meta )
      {
         DBG( if( ud->_meta != NULL ) io_vmp << "Changing proxy metatable: " << ud->_meta << " " << proxy << nl );
         // Meta table mismatch: either a new element, or a dangling one.
         // 1. Set metatable.
         luaL_getmetatable( vm, meta );                                      // [..., proxiesTable, proxyUD, mt]
         lua_setmetatable( vm, -2 );                                         // [..., proxiesTable, proxyUD]
         ud->_meta = meta;
      }

      // Current stack content                                               // [..., proxiesTable, proxyUD]
      lua_replace( vm, -2 );                                                 // [..., proxyUD]
   }
   else
   {
      lua_pushnil( vm );                                                     // [..., nil]
   }
}

//------------------------------------------------------------------------------
//!
VMProxy*
VM::toProxy( VMState* vm, int idx )
{
   DBG_BLOCK( io_vmp, "VM::toProxy( idx=" << idx << " )" );
   VMProxy** ud = (VMProxy**)lua_touserdata( vm, idx );
   DBG_MSG( io_vmp, "Userdata: " << (void*)ud );
   DBG_MSG( io_vmp, "Proxy: " << (void*)(ud ? *ud : NULL) );
   return ud ? *ud : NULL;
}

//------------------------------------------------------------------------------
//!
VMProxy*
VM::toProxy( VMState* vm, int idx, const char* meta )
{
   DBG_BLOCK( io_vmp, "VM::toProxy( idx=" << idx << ", meta=" << meta << " )" );
   VMProxy** ud = (VMProxy**)luaL_checkudata( vm, idx, meta );
   DBG_MSG( io_vmp, "Userdata: " << (void*)ud );
   DBG_MSG( io_vmp, "Proxy: " << (void*)(ud ? *ud : NULL) );
   return ud ? *ud : NULL;
}

//------------------------------------------------------------------------------
//!
void
VM::pushValue( VMState* vm, int idx )
{
   lua_pushvalue( vm, idx );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm )
{
   lua_pushnil( vm );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, Function val )
{
   lua_pushcfunction( vm, val );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, Function val, int nbUpvalue )
{
   lua_pushcclosure( vm, val, nbUpvalue );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, void* thisPtr, Function val )
{
   lua_pushlightuserdata( vm, thisPtr );
   lua_pushcclosure( vm, val, 1 );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, void* thisPtr, Function val, int nbUpvalue )
{
   lua_pushlightuserdata( vm, thisPtr );  // [up1, up2, ..., upN, thisPtr]
   lua_insert( vm, 1 );                   // [thisPtr, up1, up2, ..., upN]
   lua_pushcclosure( vm, val, nbUpvalue+1 );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, double val )
{
   lua_pushnumber( vm, val );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, int val )
{
   lua_pushnumber( vm, val );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, uint val )
{
   lua_pushnumber( vm, val );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, bool val )
{
   lua_pushboolean( vm, val );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, void* val )
{
   lua_pushlightuserdata( vm, val );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, const char* val )
{
   lua_pushstring( vm, val );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, const String& val )
{
   lua_pushstring( vm, val.cstr() );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, const ConstString& val )
{
   lua_pushstring( vm, val.cstr() );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, const Vec2i& val )
{
   VMMath::push( vm, val );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, const Vec2f& val )
{
   VMMath::push( vm, val );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, const Vec2d& val )
{
   VMMath::push( vm, val );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, const Vec3i& val )
{
   VMMath::push( vm, val );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, const Vec3f& val )
{
   VMMath::push( vm, val );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, const Vec3d& val )
{
   VMMath::push( vm, val );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, const Vec4i& val )
{
   VMMath::push( vm, val );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, const Vec4f& val )
{
   VMMath::push( vm, val );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, const Vec4d& val )
{
   VMMath::push( vm, val );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, const Quatf& val )
{
   VMMath::push( vm, val );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, const Quatd& val )
{
   VMMath::push( vm, val );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, const Mat2i& val )
{
   VMMath::push( vm, val );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, const Mat2f& val )
{
   VMMath::push( vm, val );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, const Mat2d& val )
{
   VMMath::push( vm, val );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, const Mat3i& val )
{
   VMMath::push( vm, val );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, const Mat3f& val )
{
   VMMath::push( vm, val );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, const Mat3d& val )
{
   VMMath::push( vm, val );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, const Mat4i& val )
{
   VMMath::push( vm, val );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, const Mat4f& val )
{
   VMMath::push( vm, val );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, const Mat4d& val )
{
   VMMath::push( vm, val );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, const Reff& val )
{
   lua_newtable( vm );
   VM::push( vm, val.orientation() );
   lua_rawseti( vm, -2, 1 );
   VM::push( vm, val.position() );
   lua_rawseti( vm, -2, 2 );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, const Refd& val )
{
   lua_newtable( vm );
   VM::push( vm, val.orientation() );
   lua_rawseti( vm, -2, 1 );
   VM::push( vm, val.position() );
   lua_rawseti( vm, -2, 2 );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, const Rayf& val )
{
   lua_newtable( vm );

   lua_pushstring( vm, "origin" );
   push( vm, val.origin() );
   lua_rawset( vm, -3 );

   lua_pushstring( vm, "direction" );
   push( vm, val.direction() );
   lua_rawset( vm, -3 );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, const Rayd& val )
{
   lua_newtable( vm );

   lua_pushstring( vm, "origin" );
   push( vm, val.origin() );
   lua_rawset( vm, -3 );

   lua_pushstring( vm, "direction" );
   push( vm, val.direction() );
   lua_rawset( vm, -3 );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, const AABBoxi& val )
{
   lua_newtable( vm );

   push( vm, val.corner(0, 0, 0) );
   lua_rawseti( vm, -2, 1 );

   push( vm, val.corner(1, 1, 1) );
   lua_rawseti( vm, -2, 2 );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, const AABBoxf& val )
{
   lua_newtable( vm );

   push( vm, val.corner(0, 0, 0) );
   lua_rawseti( vm, -2, 1 );

   push( vm, val.corner(1, 1, 1) );
   lua_rawseti( vm, -2, 2 );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, const AABBoxd& val )
{
   lua_newtable( vm );

   push( vm, val.corner(0, 0, 0) );
   lua_rawseti( vm, -2, 1 );

   push( vm, val.corner(1, 1, 1) );
   lua_rawseti( vm, -2, 2 );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, const VMRef& ref )
{
   if( ref.ptr() )
   {
      CHECK( ref.vm() == vm );
      lua_pushlightuserdata( vm, (void*)ref.ptr() );
      lua_rawget( vm, LUA_REGISTRYINDEX );
   }
   else
   {
      lua_pushnil( vm );
   }
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, const Event& ev )
{
   lua_newtable( vm );

   lua_pushstring( vm, "timestamp" );
   lua_pushnumber( vm, ev.timestamp() );
   lua_rawset( vm, -3 );

   lua_pushstring( vm, "type" );
   lua_pushnumber( vm, ev.type() );
   lua_rawset( vm, -3 );

   switch( ev.type() )
   {
      case Event::HID_EVENT:
      {
         lua_pushstring( vm, "value" );
         lua_pushnumber( vm, ev.valueFloat() );
         lua_rawset( vm, -3 );

         lua_pushstring( vm, "deviceTypeID" );
         VM::push( vm, ev.deviceTypeID() );
         lua_rawset( vm, -3 );

         lua_pushstring( vm, "deviceID" );
         VM::push( vm, ev.deviceID() );
         lua_rawset( vm, -3 );

         lua_pushstring( vm, "controlID" );
         VM::push( vm, ev.controlID() );
         lua_rawset( vm, -3 );
      }  break;
      case Event::ACCELERATE:
      {
         lua_pushstring( vm, "dx" );
         lua_pushnumber( vm, ev.dx() );
         lua_rawset( vm, -3 );

         lua_pushstring( vm, "dy" );
         lua_pushnumber( vm, ev.dy() );
         lua_rawset( vm, -3 );

         lua_pushstring( vm, "dz" );
         lua_pushnumber( vm, ev.dz() );
         lua_rawset( vm, -3 );
      }  break;
      default:
      {
         lua_pushstring( vm, "value" );
         lua_pushnumber( vm, ev.value() );
         lua_rawset( vm, -3 );

         lua_pushstring( vm, "position" );
         VM::push( vm, ev.position() );
         lua_rawset( vm, -3 );

         lua_pushstring( vm, "count" );
         lua_pushboolean( vm, ev.count() );
         lua_rawset( vm, -3 );

         lua_pushstring( vm, "repeated" );
         lua_pushboolean( vm, ev.isRepeated() );
         lua_rawset( vm, -3 );

         if( ev.isPointerEvent() )
         {
            lua_pushstring( vm, "pointerID" );
            lua_pushnumber( vm, ev.pointerID() );
            lua_rawset( vm, -3 );

            Pointer& p = ev.pointer();
            lua_pushstring( vm, "deltaPosition" );
            VM::push( vm, p.deltaPosition() );
            lua_rawset( vm, -3 );

            lua_pushstring( vm, "pressedState" );
            VM::push( vm, p.pressedState() );
            lua_rawset( vm, -3 );

            if( ev.type() == Event::POINTER_SCROLL )
            {
               lua_pushstring( vm, "scrollValue" );
               VM::push( vm, ev.scrollValue() );
               lua_rawset( vm, -3 );
            }
         }
      }  break;
   }
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, const Variant& v )
{
   switch( v.type() )
   {
      case Variant::BOOL:    lua_pushboolean( vm, v.getBoolean() );       break;
      case Variant::FLOAT:   lua_pushnumber( vm, v.getFloat() );          break;
      case Variant::VEC2:    VMMath::push( vm, v.getVec2() );             break;
      case Variant::VEC3:    VMMath::push( vm, v.getVec3() );             break;
      case Variant::VEC4:    VMMath::push( vm, v.getVec4() );             break;
      case Variant::QUAT:    VMMath::push( vm, v.getQuat() );             break;
      case Variant::STRING:  lua_pushstring( vm, v.getString().cstr() );  break;
      case Variant::POINTER: lua_pushlightuserdata( vm, v.getPointer() ); break;
      case Variant::TABLE:   VM::push( vm, *v.getTable() );               break;
      default: lua_pushnil(vm);
   }
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, const Table& table )
{
   // Create table.
   lua_newtable( vm );
   VM::push( vm, -1, table );
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, int idx, const Table& table )
{
   idx = ::absIndex( vm, idx );

   // Add each paramter to the table.
   // 1. integer key.
   for( uint i = 0; i < table.arraySize(); ++i )
   {
      const Variant& v = table[i];
      VM::push( vm, v );
      lua_rawseti( vm, idx, i+1 );
   }

   // 2. String key.
   Table::ConstIterator end = table.end();
   for( Table::ConstIterator iter = table.begin(); iter != end; ++iter )
   {
      lua_pushstring( vm, (*iter).first.cstr() );
      VM::push( vm, (*iter).second );
      lua_rawset( vm, idx );
   }
}

//------------------------------------------------------------------------------
//!
void
VM::push( VMState* vm, const RCP<RCObject>& object, const char* mtName )
{
   if( object.isNull() )
   {
      lua_pushnil( vm );
      return;
   }

   // Push pointer on stack.
   void** ud = (void**)lua_newuserdata( vm, sizeof(void*)+sizeof(int) );
   *ud++     = object.ptr();
   *(int*)ud = RC;

   // Push metatable.
   luaL_getmetatable( vm, mtName );
   lua_setmetatable( vm, -2 );

   // Increase reference count on object.
   object->addReference();
}

//------------------------------------------------------------------------------
//!
void
VM::pushConst( VMState* vm, const RCP<const RCObject>& object, const char* mtName )
{
   if( object.isNull() )
   {
      lua_pushnil( vm );
      return;
   }

   // Push pointer on stack.
   void** ud = (void**)lua_newuserdata( vm, sizeof(void*)+sizeof(int) );
   *ud++     = (void*)object.ptr();
   *(int*)ud = RC | CONSTANT;

   // Push metatable.
   luaL_getmetatable( vm, mtName );
   lua_setmetatable( vm, -2 );

   // Increase reference count on object.
   object->addReference();
}

//------------------------------------------------------------------------------
//!
void
VM::pushTable( VMState* vm, const char* name, const bool create )
{
   Vector<String> nameParts;
   String(name).split(".", nameParts);

   if( !nameParts.empty() )
   {
      lua_pushstring( vm, nameParts[0].cstr() );
      // Check whether lib already exists.
      lua_gettable( vm, LUA_GLOBALSINDEX );

      if( lua_isnil( vm, -1 ) )
      {
         if( create )
         {
            lua_pop( vm, 1 );
            // Create it.
            lua_newtable( vm );
            lua_pushstring( vm, nameParts[0].cstr() );
            lua_pushvalue( vm, -2 ); //copies the table reference
            // Register it with given name (pops the pushstring and pushvalue elements).
            lua_settable( vm, LUA_GLOBALSINDEX );
         }
         else
         {
            // Return nil
            return;
         }
      }
      else
      if( lua_istable( vm, -1 ) == 0 )
      {
         // Name conflict, pop value, return nil.
         lua_pop( vm, 1 );
         lua_pushnil( vm );
         return;
      }

      // Here, the stack contains either the retrieved table (lua_gettable)
      // or the created one (lua_newtable).

      // Iteratively check for remaining (deeper) tables.
      for( uint i = 1; i < nameParts.size(); ++i )
      {
         lua_pushstring( vm, nameParts[i].cstr() );

         // Check whether lib already exists.
         lua_gettable( vm, -2 );

         // Here, the stack contains the previous table, and the result of lua_gettable.

         if( lua_isnil( vm, -1 ) )
         {
            if( create )
            {
               lua_pop( vm, 1 );
               // Create it.
               lua_newtable( vm );
               lua_pushstring( vm, nameParts[i].cstr() );
               lua_pushvalue( vm, -2 ); //copies the table reference
               // Here, the stack is [T0, T1, str, T1]
               // Register it with given name.
               lua_settable( vm, -4 );
            }
            else
            {
               // Remove previous table, keep current nil.
               lua_remove( vm, -2 );
               return;
            }
         }
         else
         if( lua_istable( vm, -1 ) == 0 )
         {
            // Name conflict, pop values, return nil.
            lua_pop( vm, 2 );
            lua_pushnil( vm );
            return;
         }

         // Here, the stack contains the previous table, and the new one.
         // Replace the old one with the new one.
         lua_replace( vm, -2 );
      }
   }
   else
   {
      // Return nil
      lua_pushnil( vm );
   }
}

//------------------------------------------------------------------------------
//!
void*
VM::toPtr( VMState* vm, int idx )
{
   return lua_touserdata( vm, idx );
}

//------------------------------------------------------------------------------
//!
double
VM::toNumber( VMState* vm, int idx )
{
   return lua_tonumber( vm, idx );
}

//------------------------------------------------------------------------------
//!
int
VM::toInt( VMState* vm, int idx )
{
   return (int)lua_tonumber( vm, idx );
}

//------------------------------------------------------------------------------
//!
uint
VM::toUInt( VMState* vm, int idx )
{
   return (uint)lua_tonumber( vm, idx );
}

//------------------------------------------------------------------------------
//!
short
VM::toShort( VMState* vm, int idx )
{
   return (short)lua_tonumber( vm, idx );
}

//------------------------------------------------------------------------------
//!
ushort
VM::toUShort( VMState* vm, int idx )
{
   return (ushort)lua_tonumber( vm, idx );
}

//------------------------------------------------------------------------------
//!
bool
VM::toBoolean( VMState* vm, int idx )
{
   return lua_toboolean( vm, idx ) != 0;
}

//------------------------------------------------------------------------------
//!
const char*
VM::toCString( VMState* vm, int idx )
{
   return lua_tostring( vm, idx );
}

//------------------------------------------------------------------------------
//!
ConstString
VM::toConstString( VMState* vm, int idx )
{
   // TODO: Possibly directly convert Lua string into ours?
   return ConstString( lua_tostring( vm, idx ) );
}

//------------------------------------------------------------------------------
//!
String
VM::toString( VMState* vm, int idx )
{
   return String( lua_tostring( vm, idx ) );
}

//------------------------------------------------------------------------------
//!
const char*
VM::toTypename( VMState* vm, int idx )
{
   return lua_typename( vm, idx );
}

//------------------------------------------------------------------------------
//!
Path
VM::toPath( VMState* vm, int idx )
{
   return Path( lua_tostring( vm, idx ) );
}

//------------------------------------------------------------------------------
//!
Vec2i
VM::toVec2i( VMState* vm, int idx )
{
   switch( VM::type( vm, idx ) )
   {
      case TABLE:
      {
         lua_rawgeti( vm, idx, 1 );
         int x = (int)lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 2 );
         int y = (int)lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         return Vec2i( x, y );
      }  break;
      case OBJECT:
      {
         return VMMath::toVec2( vm, idx );
      }  break;
      default:
         return Vec2i::zero();
   }
}

//------------------------------------------------------------------------------
//!
Vec2f
VM::toVec2f( VMState* vm, int idx )
{
   switch( VM::type( vm, idx ) )
   {
      case TABLE:
      {
         lua_rawgeti( vm, idx, 1 );
         float x = (float)lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 2 );
         float y = (float)lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         return Vec2f( x, y );
      }  break;
      case OBJECT:
      {
         return VMMath::toVec2( vm, idx );
      }  break;
      default:
         return Vec2f::zero();
   }
}

//------------------------------------------------------------------------------
//!
Vec2d
VM::toVec2d( VMState* vm, int idx )
{
   switch( VM::type( vm, idx ) )
   {
      case TABLE:
      {
         lua_rawgeti( vm, idx, 1 );
         double x = lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 2 );
         double y = lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         return Vec2d( x, y );
      }  break;
      case OBJECT:
      {
         return VMMath::toVec2( vm, idx );
      } break;
      default:
         return Vec2d::zero();
   }
}

//------------------------------------------------------------------------------
//!
Vec3i
VM::toVec3i( VMState* vm, int idx )
{
   switch( VM::type( vm, idx ) )
   {
      case TABLE:
      {
         lua_rawgeti( vm, idx, 1 );
         int x = (int)lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 2 );
         int y = (int)lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 3 );
         int z = (int)lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         return Vec3i( x, y, z );
      }  break;
      case OBJECT:
      {
         return VMMath::toVec3( vm, idx );
      }  break;
      default:
         return Vec3i::zero();
   }
}

//------------------------------------------------------------------------------
//!
Vec3f
VM::toVec3f( VMState* vm, int idx )
{
   switch( VM::type( vm, idx ) )
   {
      case TABLE:
      {
         lua_rawgeti( vm, idx, 1 );
         float x = (float)lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 2 );
         float y = (float)lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 3 );
         float z = (float)lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         return Vec3f( x, y, z );
      }  break;
      case OBJECT:
      {
         return VMMath::toVec3( vm, idx );
      }  break;
      default:
         return Vec3f::zero();
   }
}

//------------------------------------------------------------------------------
//!
Vec3d
VM::toVec3d( VMState* vm, int idx )
{
   switch( VM::type( vm, idx ) )
   {
      case TABLE:
      {
         lua_rawgeti( vm, idx, 1 );
         double x = lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 2 );
         double y = lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 3 );
         double z = lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         return Vec3d( x, y, z );
      }  break;
      case OBJECT:
      {
         return VMMath::toVec3( vm, idx );
      }  break;
      default:
         return Vec3d::zero();
   }
}

//------------------------------------------------------------------------------
//!
Vec4i
VM::toVec4i( VMState* vm, int idx )
{
   switch( VM::type( vm, idx ) )
   {
      case TABLE:
      {
         lua_rawgeti( vm, idx, 1 );
         int x = (int)lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 2 );
         int y = (int)lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 3 );
         int z = (int)lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 4 );
         int w = (int)lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         return Vec4i( x, y, z, w );
      }  break;
      case OBJECT:
      {
         return VMMath::toVec4( vm, idx );
      }  break;
      default:
         return Vec4i::zero();
   }
}

//------------------------------------------------------------------------------
//!
Vec4f
VM::toVec4f( VMState* vm, int idx )
{
   switch( VM::type( vm, idx ) )
   {
      case TABLE:
      {
         lua_rawgeti( vm, idx, 1 );
         float x = (float)lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 2 );
         float y = (float)lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 3 );
         float z = (float)lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 4 );
         float w = (float)lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         return Vec4f( x, y, z, w );
      }  break;
      case OBJECT:
      {
         return VMMath::toVec4( vm, idx );
      }  break;
      default:
         return Vec4f::zero();
   }
}

//------------------------------------------------------------------------------
//!
Vec4d
VM::toVec4d( VMState* vm, int idx )
{
   switch( VM::type( vm, idx ) )
   {
      case TABLE:
      {
         lua_rawgeti( vm, idx, 1 );
         double x = lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 2 );
         double y = lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 3 );
         double z = lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 4 );
         double w = lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         return Vec4d( x, y, z, w );
      }  break;
      case OBJECT:
      {
         return VMMath::toVec4( vm, idx );
      }  break;
      default:
         return Vec4d::zero();
   }
}

//------------------------------------------------------------------------------
//!
Quatf
VM::toQuatf( VMState* vm, int idx )
{
   switch( VM::type( vm, idx ) )
   {
      case TABLE:
      {
         switch( lua_objlen( vm, idx ) )
         {
            case 2:
            {
               Vec3f axis;
               float cir;

               lua_rawgeti( vm, idx, 1 );
               if( lua_istable( vm, -1 ) )
               {
                  axis = toVec3f( vm, -1 );
                  lua_pop( vm, 1 );

                  lua_rawgeti( vm, idx, 2 );
                  cir = (float)lua_tonumber( vm, -1 );
                  lua_pop( vm, 1 );
               }
               else
               {
                  cir = (float)lua_tonumber( vm, -1 );
                  lua_pop( vm, 1 );

                  lua_rawgeti( vm, idx, 2 );
                  axis = toVec3f( vm, -1 );
                  lua_pop( vm, 1 );
               }

               return Quatf::axisCir( axis, cir );
            }
            case 4:
            {
               lua_rawgeti( vm, idx, 1 );
               float x = (float)lua_tonumber( vm, -1 );
               lua_pop( vm, 1 );

               lua_rawgeti( vm, idx, 2 );
               float y = (float)lua_tonumber( vm, -1 );
               lua_pop( vm, 1 );

               lua_rawgeti( vm, idx, 3 );
               float z = (float)lua_tonumber( vm, -1 );
               lua_pop( vm, 1 );

               lua_rawgeti( vm, idx, 4 );
               float w = (float)lua_tonumber( vm, -1 );
               lua_pop( vm, 1 );

               return Quatf( x, y, z, w );
            }
            default:
               break; //StdErr << "Invalid Quatf format." << nl;
         }

         return Quatf::identity();
      }  break;
      case OBJECT:
      {
         return VMMath::toQuat( vm, idx );
      }  break;
      default:
         return Quatf::identity();
   }
}

//------------------------------------------------------------------------------
//!
Quatd
VM::toQuatd( VMState* vm, int idx )
{
   switch( lua_objlen( vm, idx ) )
   {
      case 2:
      {
         Vec3d  axis;
         double cir;

         lua_rawgeti( vm, idx, 1 );
         if( lua_istable( vm, -1 ) )
         {
            axis = toVec3f( vm, -1 );
            lua_pop( vm, 1 );

            lua_rawgeti( vm, idx, 2 );
            cir = lua_tonumber( vm, -1 );
            lua_pop( vm, 1 );
         }
         else
         {
            cir = lua_tonumber( vm, -1 );
            lua_pop( vm, 1 );

            lua_rawgeti( vm, idx, 2 );
            axis = toVec3f( vm, -1 );
            lua_pop( vm, 1 );
         }

         return Quatd::axisCir( axis, cir );
      }
      case 4:
      {
         lua_rawgeti( vm, idx, 1 );
         double x = lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 2 );
         double y = lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 3 );
         double z = lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 4 );
         double w = lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         return Quatd( x, y, z, w );
      }
      default:
         break; //StdErr << "Invalid Quatd format." << nl;
   }

   return Quatd::identity();
}

//------------------------------------------------------------------------------
//!
Mat2i
VM::toMat2i( VMState* vm, int idx )
{
   switch( VM::type( vm, idx ) )
   {
      case TABLE:
      {
         Mat2i tmp;
         Vec2i v;

         lua_rawgeti( vm, idx, 1 );
         v = toVec2i( vm, -1 );
         tmp(0, 0) = v(0);
         tmp(0, 1) = v(1);
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 2 );
         v = toVec2i( vm, -1 );
         tmp(1, 0) = v(0);
         tmp(1, 1) = v(1);
         lua_pop( vm, 1 );

         return tmp;
      }  break;
      case OBJECT:
      {
         return VMMath::toMat2( vm, idx );
      }  break;
      default:
         return Mat2i::identity();
   }
}

//------------------------------------------------------------------------------
//!
Mat2f
VM::toMat2f( VMState* vm, int idx )
{
   switch( VM::type( vm, idx ) )
   {
      case TABLE:
      {
         Mat2f tmp;
         Vec2f v;

         lua_rawgeti( vm, idx, 1 );
         v = toVec2f( vm, -1 );
         tmp(0, 0) = v(0);
         tmp(0, 1) = v(1);
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 2 );
         v = toVec2f( vm, -1 );
         tmp(1, 0) = v(0);
         tmp(1, 1) = v(1);
         lua_pop( vm, 1 );

         return tmp;
      }  break;
      case OBJECT:
      {
         return VMMath::toMat2( vm, idx );
      }  break;
      default:
         return Mat2f::identity();
   }
}

//------------------------------------------------------------------------------
//!
Mat2d
VM::toMat2d( VMState* vm, int idx )
{
   switch( VM::type( vm, idx ) )
   {
      case TABLE:
      {
         Mat2d tmp;
         Vec2d v;

         lua_rawgeti( vm, idx, 1 );
         v = toVec2d( vm, -1 );
         tmp(0, 0) = v(0);
         tmp(0, 1) = v(1);
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 2 );
         v = toVec2d( vm, -1 );
         tmp(1, 0) = v(0);
         tmp(1, 1) = v(1);
         lua_pop( vm, 1 );

         return tmp;
      }  break;
      case OBJECT:
      {
         return VMMath::toMat2( vm, idx );
      }  break;
      default:
         return Mat2d::identity();
   }
}

//------------------------------------------------------------------------------
//!
Mat3i
VM::toMat3i( VMState* vm, int idx )
{
   switch( VM::type( vm, idx ) )
   {
      case TABLE:
      {
         Mat3i tmp;
         Vec3i v;

         lua_rawgeti( vm, idx, 1 );
         v = toVec3i( vm, -1 );
         tmp(0, 0) = v(0);
         tmp(0, 1) = v(1);
         tmp(0, 2) = v(2);
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 2 );
         v = toVec3i( vm, -1 );
         tmp(1, 0) = v(0);
         tmp(1, 1) = v(1);
         tmp(1, 2) = v(2);
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 3 );
         v = toVec3i( vm, -1 );
         tmp(2, 0) = v(0);
         tmp(2, 1) = v(1);
         tmp(2, 2) = v(2);
         lua_pop( vm, 1 );

         return tmp;
      }  break;
      case OBJECT:
      {
         return VMMath::toMat3( vm, idx );
      }  break;
      default:
         return Mat3i::identity();
   }
}

//------------------------------------------------------------------------------
//!
Mat3f
VM::toMat3f( VMState* vm, int idx )
{
   switch( VM::type( vm, idx ) )
   {
      case TABLE:
      {
         Mat3f tmp;
         Vec3f v;

         lua_rawgeti( vm, idx, 1 );
         v = toVec3f( vm, -1 );
         tmp(0, 0) = v(0);
         tmp(0, 1) = v(1);
         tmp(0, 2) = v(2);
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 2 );
         v = toVec3f( vm, -1 );
         tmp(1, 0) = v(0);
         tmp(1, 1) = v(1);
         tmp(1, 2) = v(2);
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 3 );
         v = toVec3f( vm, -1 );
         tmp(2, 0) = v(0);
         tmp(2, 1) = v(1);
         tmp(2, 2) = v(2);
         lua_pop( vm, 1 );

         return tmp;
      }  break;
      case OBJECT:
      {
         return VMMath::toMat3( vm, idx );
      }  break;
      default:
         return Mat3f::identity();
   }
}

//------------------------------------------------------------------------------
//!
Mat3d
VM::toMat3d( VMState* vm, int idx )
{
   switch( VM::type( vm, idx ) )
   {
      case TABLE:
      {
         Mat3d tmp;
         Vec3d v;

         lua_rawgeti( vm, idx, 1 );
         v = toVec3d( vm, -1 );
         tmp(0, 0) = v(0);
         tmp(0, 1) = v(1);
         tmp(0, 2) = v(2);
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 2 );
         v = toVec3d( vm, -1 );
         tmp(1, 0) = v(0);
         tmp(1, 1) = v(1);
         tmp(1, 2) = v(2);
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 3 );
         v = toVec3d( vm, -1 );
         tmp(2, 0) = v(0);
         tmp(2, 1) = v(1);
         tmp(2, 2) = v(2);
         lua_pop( vm, 1 );

         return tmp;
      }  break;
      case OBJECT:
      {
         return VMMath::toMat3( vm, idx );
      }  break;
      default:
         return Mat3d::identity();
   }
}

//------------------------------------------------------------------------------
//!
Mat4i
VM::toMat4i( VMState* vm, int idx )
{
   switch( VM::type( vm, idx ) )
   {
      case TABLE:
      {
         Mat4i tmp;
         Vec4i v;

         lua_rawgeti( vm, idx, 1 );
         v = toVec4i( vm, -1 );
         tmp(0, 0) = v(0);
         tmp(0, 1) = v(1);
         tmp(0, 2) = v(2);
         tmp(0, 3) = v(3);
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 2 );
         v = toVec4i( vm, -1 );
         tmp(1, 0) = v(0);
         tmp(1, 1) = v(1);
         tmp(1, 2) = v(2);
         tmp(1, 3) = v(3);
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 3 );
         v = toVec4i( vm, -1 );
         tmp(2, 0) = v(0);
         tmp(2, 1) = v(1);
         tmp(2, 2) = v(2);
         tmp(2, 3) = v(3);
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 4 );
         v = toVec4i( vm, -1 );
         tmp(3, 0) = v(0);
         tmp(3, 1) = v(1);
         tmp(3, 2) = v(2);
         tmp(3, 3) = v(3);
         lua_pop( vm, 1 );

         return tmp;
      }  break;
      case OBJECT:
      {
         return VMMath::toMat4( vm, idx );
      }  break;
      default:
         return Mat4i::identity();
   }
}

//------------------------------------------------------------------------------
//!
Mat4f
VM::toMat4f( VMState* vm, int idx )
{
   switch( VM::type( vm, idx ) )
   {
      case TABLE:
      {
         Mat4f tmp;
         Vec4f v;

         lua_rawgeti( vm, idx, 1 );
         v = toVec4f( vm, -1 );
         tmp(0, 0) = v(0);
         tmp(0, 1) = v(1);
         tmp(0, 2) = v(2);
         tmp(0, 3) = v(3);
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 2 );
         v = toVec4f( vm, -1 );
         tmp(1, 0) = v(0);
         tmp(1, 1) = v(1);
         tmp(1, 2) = v(2);
         tmp(1, 3) = v(3);
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 3 );
         v = toVec4f( vm, -1 );
         tmp(2, 0) = v(0);
         tmp(2, 1) = v(1);
         tmp(2, 2) = v(2);
         tmp(2, 3) = v(3);
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 4 );
         v = toVec4f( vm, -1 );
         tmp(3, 0) = v(0);
         tmp(3, 1) = v(1);
         tmp(3, 2) = v(2);
         tmp(3, 3) = v(3);
         lua_pop( vm, 1 );

         return tmp;
      }  break;
      case OBJECT:
      {
         return VMMath::toMat4( vm, idx );
      }  break;
      default:
         return Mat4f::identity();
   }
}

//------------------------------------------------------------------------------
//!
Mat4d
VM::toMat4d( VMState* vm, int idx )
{
   switch( VM::type( vm, idx ) )
   {
      case TABLE:
      {
         Mat4d tmp;
         Vec4d v;

         lua_rawgeti( vm, idx, 1 );
         v = toVec4d( vm, -1 );
         tmp(0, 0) = v(0);
         tmp(0, 1) = v(1);
         tmp(0, 2) = v(2);
         tmp(0, 3) = v(3);
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 2 );
         v = toVec4d( vm, -1 );
         tmp(1, 0) = v(0);
         tmp(1, 1) = v(1);
         tmp(1, 2) = v(2);
         tmp(1, 3) = v(3);
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 3 );
         v = toVec4d( vm, -1 );
         tmp(2, 0) = v(0);
         tmp(2, 1) = v(1);
         tmp(2, 2) = v(2);
         tmp(2, 3) = v(3);
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 4 );
         v = toVec4d( vm, -1 );
         tmp(3, 0) = v(0);
         tmp(3, 1) = v(1);
         tmp(3, 2) = v(2);
         tmp(3, 3) = v(3);
         lua_pop( vm, 1 );

         return tmp;
      }  break;
      case OBJECT:
      {
         return VMMath::toMat4( vm, idx );
      }  break;
      default:
         return Mat4d::identity();
   }
}

//------------------------------------------------------------------------------
//!
Reff
VM::toReff( VMState* vm, int idx )
{
   Quatf q;
   Vec3f p;
   float s;
   if( VM::isTable( vm, idx ) )
   {
      int ts = VM::getTableSize( vm, idx );
      if( ts == 2 )
      {
         lua_rawgeti( vm, idx, 1 );
         q = VM::toQuatf( vm, -1 );
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 2 );
         p = VM::toVec3f( vm, -1 );
         lua_pop( vm, 1 );

         s = 1.0f;
      }
      else
      if( ts == 7 )
      {
         lua_rawgeti( vm, idx, 1 );
         q(0) = (float)lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 2 );
         q(1) = (float)lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 3 );
         q(2) = (float)lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 4 );
         q(3) = (float)lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 5 );
         p.x = (float)lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 6 );
         p.y = (float)lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 7 );
         p.z = (float)lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         s = 1.0f;
      }
      else
      {
         q = Quatf::identity();
         p = Vec3f::zero();
         s = 1.0f;
         VM::get( vm, idx, "orientation", q );
         VM::get( vm, idx, "position", p );
         VM::get( vm, idx, "scale", s );
      }
   }
   else
   {
      q = Quatf::identity();
      p = Vec3f::zero();
      s = 1.0f;
   }

   return Reff( q, p, s );
}

//------------------------------------------------------------------------------
//!
Refd
VM::toRefd( VMState* vm, int idx )
{
   Quatd  q;
   Vec3d  p;
   double s;
   if( VM::isTable( vm, idx ) )
   {
      int ts = VM::getTableSize( vm, idx );
      if( ts == 2 )
      {
         lua_rawgeti( vm, idx, 1 );
         q = VM::toQuatd( vm, -1 );
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 2 );
         p = VM::toVec3d( vm, -1 );
         lua_pop( vm, 1 );

         s = 1.0;
      }
      else
      if( ts == 7 )
      {
         lua_rawgeti( vm, idx, 1 );
         q(0) = lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 2 );
         q(1) = lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 3 );
         q(2) = lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 4 );
         q(3) = lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 5 );
         p.x = lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 6 );
         p.y = lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 7 );
         p.z = lua_tonumber( vm, -1 );
         lua_pop( vm, 1 );

         s = 1.0;
      }
      else
      {
         q = Quatd::identity();
         p = Vec3d::zero();
         s = 1.0;
         VM::get( vm, idx, "orientation", q );
         VM::get( vm, idx, "position", p );
         VM::get( vm, idx, "scale", s );
      }
   }
   else
   {
      q = Quatd::identity();
      p = Vec3d::zero();
      s = 1.0;
   }

   return Refd( q, p, s );
}

//------------------------------------------------------------------------------
//!
Rayf
VM::toRayf( VMState* vm, int idx )
{
   Vec3f pos, dir;

   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, "origin" );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      pos = Vec3f::zero();
   }
   else
   {
      pos = toVec3f( vm, -1 );
   }
   lua_pop( vm, 1 );

   lua_pushstring( vm, "direction" );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      dir = Vec3f::zero();
   }
   else
   {
      dir = toVec3f( vm, -1 );
   }
   lua_pop( vm, 1 );

   return Rayf( pos, dir );
}

//------------------------------------------------------------------------------
//!
Rayd
VM::toRayd( VMState* vm, int idx )
{
   Vec3d pos, dir;

   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, "origin" );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      pos = Vec3d::zero();
   }
   else
   {
      pos = toVec3d( vm, -1 );
   }
   lua_pop( vm, 1 );

   lua_pushstring( vm, "direction" );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      dir = Vec3d::zero();
   }
   else
   {
      dir = toVec3d( vm, -1 );
   }
   lua_pop( vm, 1 );

   return Rayd( pos, dir );
}

//------------------------------------------------------------------------------
//!
AABBoxi
VM::toAABBoxi( VMState* vm, int idx )
{
   AABBoxi box;

   if( VM::isTable( vm, idx ) )
   {
      int ts = VM::getTableSize( vm, idx );
      if( ts == 2 )
      {
         // box = { {0, 0, 0}, {1, 2, 3} }
         lua_rawgeti( vm, idx, 1 );
         Vec3i c0 = VM::toVec3i( vm, -1 );
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 2 );
         Vec3i c7 = VM::toVec3i( vm, -1 );
         lua_pop( vm, 1 );

         box.set( c0, c7 );
      }
      else
      if( ts == 3 )
      {
         lua_rawgeti( vm, idx, 1 );
         if( VM::isTable(vm, -1 ) )
         {
            // box = { {0,1}, {0,2}, {0,3} }
            Vec2i x = VM::toVec2i( vm, -1 );
            lua_pop( vm, 1 );

            lua_rawgeti( vm, idx, 2 );
            Vec2i y = VM::toVec2i( vm, -1 );
            lua_pop( vm, 1 );

            lua_rawgeti( vm, idx, 3 );
            Vec2i z = VM::toVec2i( vm, -1 );
            lua_pop( vm, 1 );

            box.set( x, y, z );
         }
         else
         {
            // box = { 1, 2, 3 }
            Vec3i pt = VM::toVec3i( vm, idx );
            box.set( pt );
         }
         lua_pop( vm, 1 );
      }
      else
      {
         printf("VM::toAABBoxi - Wrong number of parameters.\n");
         box = AABBoxi::empty();
      }
   }
   else
   {
      // box = 4
      box.set( VM::toInt(vm, idx) );
   }

   return box;
}

//------------------------------------------------------------------------------
//!
AABBoxf
VM::toAABBoxf( VMState* vm, int idx )
{
   AABBoxf box;

   if( VM::isTable( vm, idx ) )
   {
      int ts = VM::getTableSize( vm, idx );
      if( ts == 2 )
      {
         // box = { {0, 0, 0}, {1, 2, 3} }
         lua_rawgeti( vm, idx, 1 );
         Vec3f c0 = VM::toVec3f( vm, -1 );
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 2 );
         Vec3f c7 = VM::toVec3f( vm, -1 );
         lua_pop( vm, 1 );

         box.set( c0, c7 );
      }
      else
      if( ts == 3 )
      {
         lua_rawgeti( vm, idx, 1 );
         if( VM::isTable(vm, -1 ) )
         {
            // box = { {0,1}, {0,2}, {0,3} }
            Vec2f x = VM::toVec2f( vm, -1 );
            lua_pop( vm, 1 );

            lua_rawgeti( vm, idx, 2 );
            Vec2f y = VM::toVec2f( vm, -1 );
            lua_pop( vm, 1 );

            lua_rawgeti( vm, idx, 3 );
            Vec2f z = VM::toVec2f( vm, -1 );
            lua_pop( vm, 1 );

            box.set( x, y, z );
         }
         else
         {
            // box = { 1, 2, 3 }
            Vec3f pt = VM::toVec3f( vm, idx );
            box.set( pt );
         }
         lua_pop( vm, 1 );
      }
      else
      {
         printf("VM::toAABBoxf - Wrong number of parameters.\n");
         box = AABBoxf::empty();
      }
   }
   else
   {
      // box = 4
      box.set( VM::toFloat(vm, idx) );
   }

   return box;
}

//------------------------------------------------------------------------------
//!
AABBoxd
VM::toAABBoxd( VMState* vm, int idx )
{
   AABBoxd box;

   if( VM::isTable( vm, idx ) )
   {
      int ts = VM::getTableSize( vm, idx );
      if( ts == 2 )
      {
         // box = { {0, 0, 0}, {1, 2, 3} }
         lua_rawgeti( vm, idx, 1 );
         Vec3d c0 = VM::toVec3d( vm, -1 );
         lua_pop( vm, 1 );

         lua_rawgeti( vm, idx, 2 );
         Vec3d c7 = VM::toVec3d( vm, -1 );
         lua_pop( vm, 1 );

         box.set( c0, c7 );
      }
      else
      if( ts == 3 )
      {
         lua_rawgeti( vm, idx, 1 );
         if( VM::isTable(vm, -1 ) )
         {
            // box = { {0,1}, {0,2}, {0,3} }
            Vec2d x = VM::toVec2d( vm, -1 );
            lua_pop( vm, 1 );

            lua_rawgeti( vm, idx, 2 );
            Vec2d y = VM::toVec2d( vm, -1 );
            lua_pop( vm, 1 );

            lua_rawgeti( vm, idx, 3 );
            Vec2d z = VM::toVec2d( vm, -1 );
            lua_pop( vm, 1 );

            box.set( x, y, z );
         }
         else
         {
            // box = { 1, 2, 3 }
            Vec3d pt = VM::toVec3d( vm, idx );
            box.set( pt );
         }
         lua_pop( vm, 1 );
      }
      else
      {
         printf("VM::toAABBoxd - Wrong number of parameters.\n");
         box = AABBoxd::empty();
      }
   }
   else
   {
      // box = 4
      box.set( VM::toDouble(vm, idx) );
   }

   return box;
}

//------------------------------------------------------------------------------
//!
void
VM::toRef( VMState* vm, int idx, VMRef& ref )
{
   ref.set( vm, idx );
}

//------------------------------------------------------------------------------
//!
void
VM::toTable( VMState* vm, int idx, Table& table )
{
   idx = ::absIndex( vm, idx );  // Convert to absolute, else the nil below will hose negative idx values.
   VM::push( vm );
   while( VM::next( vm, idx ) )
   {
      toVariant( vm, table );
      VM::pop( vm );
   }
}

//------------------------------------------------------------------------------
//!
Variant
VM::toVariant( VMState* vm, int idx )
{
   switch( VM::type( vm, idx ) )
   {
      case VM::NIL:    return Variant::null();
      case VM::BOOL:   return Variant( VM::toBoolean( vm, idx ) );
      case VM::PTR:    return Variant( VM::toPtr( vm, idx ) );
      case VM::NUMBER: return Variant( VM::toFloat( vm, idx ) );
      case VM::STRING: return Variant( VM::toCString( vm, idx ) );
      case VM::TABLE:
      {
         RCP<Table> subtable = new Table();
         VM::toTable( vm, idx, *subtable );
         return Variant( subtable.ptr() );
      };
      case VM::OBJECT:
         switch( VMMath::type( vm, idx ) )
         {
            case VMMath::VEC2: return Variant( VMMath::toVec2( vm, idx ) );
            case VMMath::VEC3: return Variant( VMMath::toVec3( vm, idx ) );
            case VMMath::VEC4: return Variant( VMMath::toVec4( vm, idx ) );
            case VMMath::QUAT: return Variant( VMMath::toQuat( vm, idx ) );
            case VMMath::COUNTER:
            {
               VMCounter* c = VMMath::toCounter( vm, idx );
               return Variant( (float)c->next() );
            };
            case VMMath::POINTER: return Variant( VMMath::toPointer( vm, idx ) );
            default:              return Variant( VM::toPtr( vm, idx ) );
         }
         break;
      default:;
   }
   return Variant::null();
}


//------------------------------------------------------------------------------
//!
void
VM::toVariant( VMState* vm, Table& table )
{
   // Integer key.
   if( VM::isNumber( vm, -2 ) )
   {
      uint key = VM::toUInt( vm, -2 ) - 1;
      switch( VM::type( vm, -1 ) )
      {
         case VM::NIL:    table.set( key, Variant::null() );         break;
         case VM::BOOL:   table.set( key, VM::toBoolean( vm, -1 ) ); break;
         case VM::PTR:    table.set( key, VM::toPtr( vm, -1 ) );     break;
         case VM::NUMBER: table.set( key, VM::toFloat( vm, -1 ) );   break;
         case VM::STRING: table.set( key, ConstString(VM::toCString( vm, -1 )) ); break;
         case VM::TABLE:
         {
            RCP<Table> subtable = new Table();
            VM::toTable( vm, -1, *subtable );
            table.set( key, subtable.ptr() );
         }  break;
         case VM::OBJECT:
            switch( VMMath::type( vm, -1 ) )
            {
               case VMMath::VEC2: table.set( key, VMMath::toVec2( vm, -1 ) ); break;
               case VMMath::VEC3: table.set( key, VMMath::toVec3( vm, -1 ) ); break;
               case VMMath::VEC4: table.set( key, VMMath::toVec4( vm, -1 ) ); break;
               case VMMath::QUAT: table.set( key, VMMath::toQuat( vm, -1 ) ); break;
               case VMMath::COUNTER:
               {
                  VMCounter* c = VMMath::toCounter( vm, -1 );
                  table.set( key, (float)c->next() );
               }  break;
               case VMMath::POINTER: table.set( key, VMMath::toPointer( vm, -1 ) ); break;
               default:
                  table.set( key, VM::toPtr( vm, -1 ) );
                  break;
            }
            break;
         default:;
      }
      return;
   }

   // String key.
   const char* key = VM::toCString( vm, -2 );
   switch( VM::type( vm, -1 ) )
   {
      case VM::NIL:    table.remove( key );                       break;
      case VM::BOOL:   table.set( key, VM::toBoolean( vm, -1 ) ); break;
      case VM::NUMBER: table.set( key, VM::toFloat( vm, -1 ) );   break;
      case VM::STRING: table.set( key, ConstString(VM::toCString( vm, -1 )) ); break;
      case VM::TABLE:
      {
         RCP<Table> subtable = new Table();
         VM::toTable( vm, -1, *subtable );
         table.set( key, subtable.ptr() );
      }  break;
      case VM::OBJECT:
         switch( VMMath::type( vm, -1 ) )
         {
            case VMMath::VEC2: table.set( key, VMMath::toVec2( vm, -1 ) ); break;
            case VMMath::VEC3: table.set( key, VMMath::toVec3( vm, -1 ) ); break;
            case VMMath::VEC4: table.set( key, VMMath::toVec4( vm, -1 ) ); break;
            case VMMath::QUAT: table.set( key, VMMath::toQuat( vm, -1 ) ); break;
            case VMMath::COUNTER:
            {
               VMCounter* c = VMMath::toCounter( vm, -1 );
               table.set( key, (float)c->next() );
            }  break;
            case VMMath::POINTER: table.set( key, VMMath::toPointer( vm, -1 ) ); break;
            default:
               table.set( key, VM::toPtr( vm, -1 ) );
               break;
         }
         break;
      default:;
   }
}

//------------------------------------------------------------------------------
//!
void
VM::newMetaTable( VMState* vm, const char* name )
{
   int v = luaL_newmetatable( vm, name );
   if( v == 0 )
   {
      //CHECK( false );  //DynamicEntityVM stores its routines in the same table as EntityVM.
   }
}

//------------------------------------------------------------------------------
//!
void
VM::getMetaTable( VMState* vm, const char* name )
{
   luaL_getmetatable( vm, name );
}

//------------------------------------------------------------------------------
//!
void
VM::setMetaTable( VMState* vm, int idx )
{
   lua_setmetatable( vm, idx );
}

//------------------------------------------------------------------------------
//!
bool
VM::checkMetaTable( VMState* vm, int idx, const char* name )
{
   return ( luaL_checkudata( vm, idx, name ) != NULL );
}

//------------------------------------------------------------------------------
//!
int
VM::getTableSize( VMState* vm, int idx )
{
   return int(lua_objlen( vm, idx ));
}

//------------------------------------------------------------------------------
//!
void
VM::newTable( VMState* vm )
{
   lua_newtable( vm );
}

//------------------------------------------------------------------------------
//!
void
VM::getTable( VMState* vm, int idx )
{
   lua_gettable( vm, idx );
}

//------------------------------------------------------------------------------
//!
void
VM::setTable( VMState* vm, int idx )
{
   lua_settable( vm, idx );
}

//------------------------------------------------------------------------------
//!
bool
VM::next( VMState* vm, int idx )
{
   return lua_next( vm, idx ) != 0;
}

//------------------------------------------------------------------------------
//!
void
VM::set( VMState* vm, int idx )
{
   lua_rawset( vm, idx );
}

//------------------------------------------------------------------------------
//!
void
VM::seti( VMState* vm, int idx, int n )
{
   lua_rawseti( vm, idx, n );
}

//------------------------------------------------------------------------------
//!
void
VM::get( VMState* vm, int idx )
{
   lua_rawget( vm, idx );
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key )
{
   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::geti( VMState* vm, int idx, int n )
{
   lua_rawgeti( vm, idx, n );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   return true;
}

//------------------------------------------------------------------------------
//!
void
VM::set( VMState* vm, int idx, const char* key, int value )
{
   idx = ::absIndex( vm, idx );
   lua_pushstring( vm, key );
   lua_pushnumber( vm, value );
   lua_rawset( vm, idx );
}

//------------------------------------------------------------------------------
//!
void
VM::set( VMState* vm, int idx, const char* key, Function func )
{
   idx = ::absIndex( vm, idx );
   lua_pushstring( vm, key );
   if( func )
   {
      lua_pushcfunction( vm, func );
   }
   else
   {
      lua_pushnil( vm );
   }
   lua_rawset( vm, idx );
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key, const char*& result )
{
   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = lua_tostring( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key, Path& result )
{
   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = lua_tostring( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key, String& result )
{
   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = lua_tostring( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key, ConstString& result )
{
   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = lua_tostring( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key, uint8_t& result )
{
   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = (uint8_t)lua_tonumber( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key, int& result )
{
   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = (int)lua_tonumber( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key, uint& result )
{
   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = (uint)lua_tonumber( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key, short& result )
{
   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = (short)lua_tonumber( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key, ushort& result )
{
   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = (ushort)lua_tonumber( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key, double& result )
{
   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = lua_tonumber( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key, float& result )
{
   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = (float)lua_tonumber( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key, bool& result )
{
   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = (lua_toboolean( vm, -1 ) != 0);
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key, Vec2i& result )
{
   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = toVec2i( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key, Vec2f& result )
{
   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = toVec2f( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key, Vec2d& result )
{
   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = toVec2d( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key, Vec3i& result )
{
   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = toVec3i( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key, Vec3f& result )
{
   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = toVec3f( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key, Vec3d& result )
{
   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = toVec3d( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key, Vec4i& result )
{
   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = toVec4i( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key, Vec4f& result )
{
   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = toVec4f( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key, Vec4d& result )
{
   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = toVec4d( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key, Quatf& result )
{
   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = toQuatf( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key, Quatd& result )
{
   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = toQuatd( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key, Mat2i& result )
{
   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = toMat2i( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key, Mat2f& result )
{
   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = toMat2f( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key, Mat2d& result )
{
   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = toMat2d( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key, Mat3i& result )
{
   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = toMat3i( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key, Mat3f& result )
{
   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = toMat3f( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key, Mat3d& result )
{
   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = toMat3d( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key, Mat4i& result )
{
   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = toMat4i( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key, Mat4f& result )
{
   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = toMat4f( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key, Mat4d& result )
{
   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = toMat4d( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key, Reff& result )
{
   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = toReff( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key, Refd& result )
{
   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = toRefd( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key, Rayf& result )
{
   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = toRayf( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key, Rayd& result )
{
   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = toRayd( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key, AABBoxi& result )
{
   idx = ::absIndex( vm, idx );
   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = toAABBoxi( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key, AABBoxf& result )
{
   idx = ::absIndex( vm, idx );
   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = toAABBoxf( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key, AABBoxd& result )
{
   idx = ::absIndex( vm, idx );
   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = toAABBoxd( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::get( VMState* vm, int idx, const char* key, VMRef& result )
{
   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }

   result.set( vm, -1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::geti( VMState* vm, int idx, int n, RCP<RCObject>& result )
{
   lua_rawgeti( vm, idx, n );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = *(RCObject**)lua_touserdata( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::getiWidget( VMState* vm, int idx, int n, RCP<Widget>& result )
{
   lua_rawgeti( vm, idx, n );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = (Widget*)VM::toProxy( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
bool
VM::getWidget( VMState* vm, int idx, const char* key, RCP<Widget>& result )
{
   idx = ::absIndex( vm, idx );

   lua_pushstring( vm, key );
   lua_rawget( vm, idx );
   if( lua_isnil( vm, -1 ) )
   {
      lua_pop( vm, 1 );
      return false;
   }
   result = (Widget*)VM::toProxy( vm, -1 );
   lua_pop( vm, 1 );
   return true;
}

//------------------------------------------------------------------------------
//!
void
VM::registerFunctions( VMState* vm, const Reg* reg )
{
   for( ; reg->name; ++reg )
   {
      lua_pushstring( vm, reg->name );
      lua_pushcfunction( vm, reg->func );
      lua_settable( vm, -3 );
   }
}

//------------------------------------------------------------------------------
//!
void
VM::registerFunctions( VMState* vm, const char* name, const Reg* reg )
{
   pushTable( vm, name );

   if( lua_isnil( vm, -1 ) == 0 )
   {
      for( ; reg->name; ++reg )
      {
         lua_pushstring( vm, reg->name );
         lua_pushcfunction( vm, reg->func );
         lua_settable( vm, -3 );
      }
   }
   else
   {
      printf("Could not register functions - '%s' table could not be pushed to stack\n", String(name).cstr());
   }

   lua_pop( vm, 1 ); // Remove result of pushTable (proxy/nil) from stack.
}

//------------------------------------------------------------------------------
//!
void
VM::registerFunction( VMState* vm, const char* name, Function func )
{
   lua_pushstring( vm, name );
   lua_pushcfunction( vm, func );
   lua_settable( vm, LUA_GLOBALSINDEX );
}

//------------------------------------------------------------------------------
//!
void
VM::registerFunction( VMState* vm, const char* nameSpace, const char* name, Function func )
{
   pushTable( vm, nameSpace );

   if( lua_isnil( vm, -1 ) == 0 )
   {
      lua_pushstring( vm, name );
      lua_pushcfunction( vm, func );
      lua_settable( vm, -3 );
   }
   else
   {
      printf("Could not register function - '%s' table could not be pushed to stack\n", String(nameSpace).cstr());
   }

   lua_pop( vm, 1 ); // Remove result of pushTable (proxy/nil) from stack.
}

//------------------------------------------------------------------------------
//!
void
VM::registerEnum( VMState* vm, const char* name, const EnumReg* reg )
{
   pushTable( vm, name );  // This table will be our enum, stack = [enum].

   if( !VM::isNil( vm, -1 ) )
   {
      // Create a metatable if null.
      if( lua_getmetatable( vm, -1 ) == 0 )
      {
         lua_newtable( vm );  // [enum, metatable]

         // Register error routine as __newindex.
         lua_pushstring( vm, "__newindex" );
         lua_pushcfunction( vm, readOnlyNewIndex );
         lua_settable( vm, -3 );     // [enum, metatable-with-newindex-set]
         lua_setmetatable( vm, -2 ); // [enum]
      }
      else
      {
         lua_pop( vm, 1 );
      }

      // Fill enums.
      for( ; reg->name; ++reg )
      {
         lua_pushstring( vm, reg->name );
         lua_pushnumber( vm, reg->id );
         lua_rawset( vm, -3 );
      }
   }
   else
   {
      printf("Could not register enums - '%s' table could not be pushed to stack\n", String(name).cstr());
   }

   lua_pop( vm, 1 ); // Remove result of pushTable (enum/nil) from stack.
}

//------------------------------------------------------------------------------
//!
void
VM::registerEnumReversed( VMState* vm, const char* name, const EnumReg* reg )
{
   pushTable( vm, name );  // This table will be our proxy, stack = [proxy].

   if( !VM::isNil( vm, -1 ) )
   {
      // The pushed table is the proxy, and should be empty.
      if( lua_getmetatable( vm, -1 ) != 0 )
      {
         luaL_where( vm, 1 );
         const char* where = lua_tostring( vm, -1 );
         printf("VM Error: ");
         if( where != NULL && where[0] != '\0' )
         {
            printf("%s", where);
         }
         printf("Enum '%s' already contains a read-only table.\n", name);
         CHECK( false );
         // In release, just ignore any extra register calls.
         lua_pop( vm, 3 ); // Pop where, metatable, and proxy.
         return;
      }

      lua_newtable( vm );  // [proxy, metatable]

      // Register enum table as __index.
      lua_pushstring( vm, "__index" );
      lua_newtable( vm );  // [proxy, metatable, string, realtable]
      // Fill enums.
      for( ; reg->name; ++reg )
      {
         lua_pushnumber( vm, reg->id );
         lua_pushstring( vm, reg->name );
         lua_settable( vm, -3 );
      }
      lua_settable( vm, -3 ); // [proxy, metatable-with-index-set]

      // Register error routine as __newindex.
      lua_pushstring( vm, "__newindex" );
      lua_pushcfunction( vm, readOnlyNewIndex );
      lua_settable( vm, -3 ); // [proxy, metatable-with-index-and-newindex-set]

      lua_setmetatable( vm, -2 ); // [proxy]
   }
   else
   {
      printf("Could not register enums - '%s' table could not be pushed to stack\n", String(name).cstr());
   }

   lua_pop( vm, 1 ); // Remove result of pushTable (proxy/nil) from stack.
}

//------------------------------------------------------------------------------
//!
int
VM::gcRCObject( VMState* vm )
{
   // Get pointer from stack.
   int flags;
   RCObject* p = (RCObject*)VM::unboxPointer( vm, 1, flags );

   // Decrease reference count.
   if( flags & RC )
   {
      p->removeReference();
   }

   return 0;
}

//------------------------------------------------------------------------------
//!
void
VM::printStack( VMState* vm, int numEntries )
{
   int stackSize = VM::getTop(vm);
   if( numEntries < 0 )
   {
      numEntries = stackSize;
   }
   printf("LUA STACK: printing %d entries out of %d\n", numEntries, stackSize);
   int negIdx = -1;
   int posIdx = stackSize;
   printf("[ abs,  rel]\n");
   printf("             +-----top------------------------------------------------------+\n");
   for( int i = 0; i < numEntries; ++i )
   {
#if 1
      printf("[%4d, %4d] | ", posIdx, negIdx);
      int t = lua_type(vm, posIdx);
      switch( t )
      {
         case LUA_TSTRING:
            printf("'%s'", lua_tostring(vm, posIdx));
            break;
         case LUA_TBOOLEAN:
            printf(lua_toboolean(vm, posIdx) ? "true" : "false");
            break;
         case LUA_TNUMBER:
            printf("%g", lua_tonumber(vm, posIdx));
            break;
         default:
           printf("%s", lua_typename(vm, t));
           break;
      }
      printf(" - ");
      t = lua_type(vm, negIdx);
      switch( t )
      {
         case LUA_TSTRING:
            printf("'%s'", lua_tostring(vm, negIdx));
            break;
         case LUA_TBOOLEAN:
            printf(lua_toboolean(vm, negIdx) ? "true" : "false");
            break;
         case LUA_TNUMBER:
            printf("%g", lua_tonumber(vm, negIdx));
            break;
         default:
           printf("%s", lua_typename(vm, t));
           break;
      }
      printf("\n");
#else
      printf("[%4d, %4d] | %s - %s\n",
             posIdx, negIdx,
             VM::toCString(vm, posIdx), VM::toCString(vm, negIdx));
#endif
      --posIdx;
      --negIdx;
   }
   if( numEntries < stackSize )
   {
      printf("              ...\n");
   }
   printf("             +----bottom----------------------------------------------------+\n");
}


//------------------------------------------------------------------------------
//!
void
VM::printCallStack( VMState* vm, int numEntries )
{
   printf("LUA CALL STACK:\n");
   if( numEntries < 0 )  numEntries = INT_MAX;
   int level = 0;
   int ok;
   lua_Debug ldbg;
   ok = lua_getstack(vm, level, &ldbg);
   if( ok )
   {
      printf("  +----------------top------------------------------------------------------+\n");
      ok = lua_getinfo(vm, "nSlu", &ldbg);
      if( ok )
      {
         printf("%2d| %s (%s) %s@%d [%d-%d] %d ups\n",
                level,
                ldbg.name, ldbg.namewhat,
                ldbg.short_src, ldbg.currentline, ldbg.linedefined, ldbg.lastlinedefined,
                ldbg.nups);
         ++level;
         //printf("+---------------------------------------------------------------------------+\n");
         while( lua_getstack(vm, level, &ldbg) != 0 )
         {
            ok = lua_getinfo(vm, "nSlu", &ldbg);
            if( ok )
            {
               if( level <= numEntries )
               {
                  printf("%2d| %s (%s) %s@%d [%d-%d] %d ups\n",
                         level,
                         ldbg.name, ldbg.namewhat,
                         ldbg.short_src, ldbg.currentline, ldbg.linedefined, ldbg.lastlinedefined,
                         ldbg.nups);
               }
               else
               {
                  printf("(...)\n");
                  break;
               }
            }
            else
            {
               printf("Error retrieving stack info (lua_getinfo return %d)\n", ok);
            }
            ++level;
         }
      }
      else
      {
         printf("Error retrieving stack info (lua_getinfo return %d)\n", ok);
      }
      printf("  +---------------bottom----------------------------------------------------+\n");
   }
   else
   {
      printf("Error retrieving stack (lua_getstack returned %d)\n", ok);
   }
}

NAMESPACE_END
