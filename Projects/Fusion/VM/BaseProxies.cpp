/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/VM/BaseProxies.h>

#include <Base/ADT/StringMap.h>
#include <Base/Dbg/DebugStream.h>
#include <Base/IO/FileSystem.h>

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_bp, "BaseProxies" );


//------------------------------------------------------------------------------
//!
enum 
{
   ATTRIB_PATH,
   ATTRIB_SIZE,
   ATTRIB_TYPE
};

//------------------------------------------------------------------------------
//!
StringMap _attributes(
   "path",  ATTRIB_PATH,
   "size",  ATTRIB_SIZE,
   "type",  ATTRIB_TYPE,
   ""
);

//------------------------------------------------------------------------------
//!
const VM::EnumReg  _enumsType[] = {
   { "UNKNOWN",     FS::TYPE_UNKNOWN,  },
   { "DIRECTORY",   FS::TYPE_DIRECTORY },
   { "FILE",        FS::TYPE_FILE,     },
   { "DEVICE",      FS::TYPE_DEVICE,   },
   { "ALIAS",       FS::TYPE_ALIAS,    },
   { "BUNDLE",      FS::TYPE_BUNDLE,   },
   { "BLK",         FS::TYPE_BLK,      },
   { "CHR",         FS::TYPE_CHR,      },
   { "FIFO",        FS::TYPE_FIFO,     },
   { "SOCKET",      FS::TYPE_SOCKET,   },
   { "SYMLINK",     FS::TYPE_SYMLINK,  },
   { "SHORTCUT",    FS::TYPE_SHORTCUT  },
   { 0, 0 }
};


//------------------------------------------------------------------------------
//!
int
dirIterVM( VMState* vm )
{
   DBG_BLOCK( os_bp, "dirIterVM" );
   FS::DirIterator* it = (FS::DirIterator*)VM::toPtr( vm, VM::upvalue(1) );
   // No need for a counter.

   if( it == NULL )
   {
      return 0;
   }

   if( !(*it)() )
   {
      delete it;
      return 0;
   }

   // Push iterator value.
   VM::push( vm, *(*it) );

   // Advance iterator.
   ++(*it);

   return 1;
}

int
dirIterCreationVM( VMState* vm )
{
   DBG_BLOCK( os_bp, "Creating a directory iterator" );

   // Initialize a new iterator.
   Path  path( VM::toString(vm, -1) );
   FS::DirIterator* it = new FS::DirIterator( path );

   // Push the iterator (no need for a counter).
   VM::push( vm, it );

   // Push the iterator function as a closure.
   VM::push( vm, dirIterVM, 1 );

   return 1;
}

//------------------------------------------------------------------------------
//!
const VM::Reg _funcs[] = {
   { "DirIterator", dirIterCreationVM },
   { 0, 0 }
};


UNNAMESPACE_END



NAMESPACE_BEGIN

namespace BaseProxies
{

//------------------------------------------------------------------------------
//! Initialize all of the attractors.
void initialize( VMState* vm )
{
   DBG_BLOCK( os_bp, "Initializing all Base proxies" );

   VM::registerEnum( vm, "FS.Type", _enumsType );
   VM::registerFunctions( vm, "FS", _funcs );
   BaseFSEntryVM::initialize( vm, "FS" );
}

} // namespace BaseProxies


/*==============================================================================
  CLASS BaseFSEntry
==============================================================================*/

//------------------------------------------------------------------------------
//!
BaseFSEntry::BaseFSEntry():
   _entry( NULL )
{
}

//------------------------------------------------------------------------------
//!
BaseFSEntry::~BaseFSEntry()
{
   delete _entry;
}

//------------------------------------------------------------------------------
//!
void
BaseFSEntry::init( VMState* vm )
{
   DBG_BLOCK( os_bp, "BaseFSEntry::init" );
   Path  path( VM::toString(vm, -1) );
   CHECK( _entry == NULL );
   _entry = new FS::Entry( path );
}

//------------------------------------------------------------------------------
//!
bool
BaseFSEntry::performGet( VMState* vm )
{
   DBG_BLOCK( os_bp, "BaseFSEntry::performGet" );
   switch( _attributes[VM::toCString(vm, -1)] )
   {
      case ATTRIB_PATH:
      {
         DBG_MSG( os_bp, "Returning path of " << _entry->path().string() );
         VM::push( vm, _entry->path().string() );
      } return true;
      case ATTRIB_SIZE:
      {
         DBG_MSG( os_bp, "Returning size of " << _entry->size() );
         VM::push( vm, (uint)_entry->size() );
      } return true;
      case ATTRIB_TYPE:
      {
         DBG_MSG( os_bp, "Returning defined type of " << _entry->type() );
         VM::push( vm, (uint)_entry->type() );
      } return true;
      default:
      {
         printf("BaseFSEntry::performGet - '%s' unknown\n", VM::toCString(vm, -1));
      }
   } //switch..case
   return false;
}

//------------------------------------------------------------------------------
//!
bool
BaseFSEntry::performSet( VMState* vm )
{
   DBG_BLOCK( os_bp, "BaseFSEntry::performSet" );
   switch( _attributes[VM::toCString(vm, -2)] )
   {
      case ATTRIB_PATH:
      case ATTRIB_SIZE:
      case ATTRIB_TYPE:
      {
         // Read-only.
      } return true;
      default:
      {
         printf("BaseFSEntry::performSet - '%s' unknown\n", VM::toCString(vm, -1));
      }
   }  //switch..case
   return false;
}

NAMESPACE_END
