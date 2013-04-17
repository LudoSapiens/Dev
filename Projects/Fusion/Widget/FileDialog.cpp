/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Widget/FileDialog.h>

#include <Fusion/Core/Core.h>
#include <Fusion/VM/VMObjectPool.h>

#include <Base/ADT/StringMap.h>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

const char* _meta = "fileDialog";

const VM::EnumReg _enumType[] = {
   { "OPEN",  FileDialog::OPEN },
   { "SAVE",  FileDialog::SAVE },
   { 0, 0 }
};

//------------------------------------------------------------------------------
//!
enum
{
   ATTRIB_ALLOWED_TYPES,
   ATTRIB_ASK,
   ATTRIB_CAN_CREATE_DIRECTORIES,
   ATTRIB_CAN_SELECT_DIRECTORIES,
   ATTRIB_CAN_SELECT_FILES,
   ATTRIB_CAN_SHOW_HIDDEN,
   ATTRIB_MESSAGE,
   ATTRIB_MULTIPLE_SELECT,
   ATTRIB_ON_CANCEL,
   ATTRIB_ON_CONFIRM,
   ATTRIB_PATH,
   ATTRIB_PATHS,
   ATTRIB_PROMPT,
   ATTRIB_RESOLVE_ALIASES,
   ATTRIB_SHOW_HIDDEN,
   ATTRIB_STARTING_LOCATION,
   ATTRIB_TITLE,
   ATTRIB_TYPE,
   NUM_ATTRIBS
};

//------------------------------------------------------------------------------
//!
StringMap _attributes(
   "allowedTypes"        ,  ATTRIB_ALLOWED_TYPES,
   "ask"                 ,  ATTRIB_ASK,
   "canCreateDirectories",  ATTRIB_CAN_CREATE_DIRECTORIES,
   "canSelectDirectories",  ATTRIB_CAN_SELECT_DIRECTORIES,
   "canSelectFiles"      ,  ATTRIB_CAN_SELECT_FILES,
   "canShowHidden"       ,  ATTRIB_CAN_SHOW_HIDDEN,
   "message"             ,  ATTRIB_MESSAGE,
   "multipleSelect"      ,  ATTRIB_MULTIPLE_SELECT,
   "onCancel"            ,  ATTRIB_ON_CANCEL,
   "onConfirm"           ,  ATTRIB_ON_CONFIRM,
   "path"                ,  ATTRIB_PATH,
   "paths"               ,  ATTRIB_PATHS,
   "prompt"              ,  ATTRIB_PROMPT,
   "resolveAliases"      ,  ATTRIB_RESOLVE_ALIASES,
   "showHidden"          ,  ATTRIB_SHOW_HIDDEN,
   "startingLocation"    ,  ATTRIB_STARTING_LOCATION,
   "title"               ,  ATTRIB_TITLE,
   "type"                ,  ATTRIB_TYPE,
   ""
);

//-----------------------------------------------------------------------------
//!
int  askVM( VMState* vm )
{
   FileDialog* d = (FileDialog*)VM::thisPtr( vm );
   d->ask();
   return 0;
}

//-----------------------------------------------------------------------------
//!
void  initVM( VMState* vm, uint /*mask*/ )
{
   VM::registerEnum( vm, "UI.FileDialog", _enumType );
}

//------------------------------------------------------------------------------
//!
void  execute( const VMRef& ref, FileDialog* df )
{
   if( ref.isValid() )
   {
      VMState* vm = ref.vm();
      VM::push( vm, ref );
      VM::pushProxy( vm, df );
      VM::ecall( vm, 1, 0 );
   }
}

//-----------------------------------------------------------------------------
//!
void  pushPaths( VMState* vm, const Vector<Path>& paths )
{
   VM::newTable( vm );
   int i = 1;
   for( auto cur = paths.begin(); cur != paths.end(); ++cur, ++i )
   {
      VM::push( vm, (*cur).string() );
      VM::seti( vm, -2, i );
   }
}

UNNAMESPACE_END


/*==============================================================================
  CLASS FileDialog
==============================================================================*/

//-----------------------------------------------------------------------------
//!
FileDialog::FileDialog():
   _type(OPEN),
   _canSelectDirs(false),
   _canSelectFiles(true),
   _showHidden(false),
   _canShowHidden(true),
   _canCreateDirs(true),
   _resolveAliases(true),
   _multiSelect(false)
{
}

//-----------------------------------------------------------------------------
//!
FileDialog::~FileDialog()
{
}

//-----------------------------------------------------------------------------
//!
void
FileDialog::ask()
{
   Core::ask(*this);
}

//-----------------------------------------------------------------------------
//!
void
FileDialog::onCancel()
{
   _onCancelDel.exec( *this );
   execute( _onCancelRef, this );
}

//-----------------------------------------------------------------------------
//!
void
FileDialog::onConfirm()
{
   _onConfirmDel.exec( *this );
   execute( _onConfirmRef, this );
}

//-----------------------------------------------------------------------------
//!
void
FileDialog::initialize()
{
   VMRegistry::add( initVM, VM_CAT_APP );
   VMObjectPool::registerObject( "UI", _meta, stdCreateVM<FileDialog>, stdGetVM<FileDialog>, stdSetVM<FileDialog> );
}

//-----------------------------------------------------------------------------
//!
const char*
FileDialog::meta() const
{
   return _meta;
}

//------------------------------------------------------------------------------
//!
void
FileDialog::init( VMState* vm )
{
   if( VM::isTable(vm, -1) )
   {
      VM::push(vm); // Start iterating at index 0 (nil).
      while( VM::next(vm, -2) )
      {
         performSet(vm);
         VM::pop(vm, 1); // Pop the value, keep the key.
      }
   }
}

//------------------------------------------------------------------------------
//!
bool
FileDialog::performGet( VMState* vm )
{
   switch( _attributes[VM::toCString(vm, -1)] )
   {
      case ATTRIB_ALLOWED_TYPES:
      {
         VM::newTable( vm );
         int i = 1;
         for( auto cur = _types.begin(); cur != _types.end(); ++cur, ++i )
         {
            VM::push( vm, (*cur) );
            VM::seti( vm, -2, i );
         }
      }  return true;
      case ATTRIB_ASK:
         VM::push( vm, this, askVM );
         return true;
      case ATTRIB_CAN_CREATE_DIRECTORIES:
         VM::push( vm, canCreateDirectories() );
         return true;
      case ATTRIB_CAN_SELECT_DIRECTORIES:
         VM::push( vm, canSelectDirectories() );
         return true;
      case ATTRIB_CAN_SELECT_FILES:
         VM::push( vm, canSelectFiles() );
         return true;
      case ATTRIB_CAN_SHOW_HIDDEN:
         VM::push( vm, canShowHidden() );
         return true;
      case ATTRIB_MESSAGE:
         VM::push( vm, message() );
         return true;
      case ATTRIB_MULTIPLE_SELECT:
         VM::push( vm, multipleSelect() );
         return true;
      case ATTRIB_ON_CANCEL:
         VM::push( vm, _onCancelRef );
         return true;
      case ATTRIB_ON_CONFIRM:
         VM::push( vm, _onConfirmRef );
         return true;
      case ATTRIB_PATH:
         VM::push( vm, path().string() );
         return true;
      case ATTRIB_PATHS:
         pushPaths( vm, _paths );
         return true;
      case ATTRIB_PROMPT:
         VM::push( vm, prompt() );
         return true;
      case ATTRIB_RESOLVE_ALIASES:
         VM::push( vm, resolveAliases() );
         return true;
      case ATTRIB_SHOW_HIDDEN:
         VM::push( vm, showHidden() );
         return true;
      case ATTRIB_STARTING_LOCATION:
         VM::push( vm, startingLocation().string() );
         return true;
      case ATTRIB_TITLE:
         VM::push( vm, title() );
         return true;
      case ATTRIB_TYPE:
         VM::push( vm, uint(type()) );
         return true;
      default:
         break;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
bool
FileDialog::performSet( VMState* vm )
{
   switch( _attributes[VM::toCString(vm, -2)] )
   {
      case ATTRIB_ALLOWED_TYPES:
         _types.clear();
         for( uint i = 1; VM::geti( vm, -1, i ); ++i )
         {
            _types.pushBack( VM::toString(vm, -1) );
            VM::pop( vm );
         }
         return true;
      case ATTRIB_ASK:
         // Read-only.
         return true;
      case ATTRIB_CAN_CREATE_DIRECTORIES:
         canCreateDirectories( VM::toBoolean(vm, -1) );
         return true;
      case ATTRIB_CAN_SELECT_DIRECTORIES:
         canSelectDirectories( VM::toBoolean(vm, -1) );
         return true;
      case ATTRIB_CAN_SELECT_FILES:
         canSelectFiles( VM::toBoolean(vm, -1) );
         return true;
      case ATTRIB_CAN_SHOW_HIDDEN:
         canShowHidden( VM::toBoolean(vm, -1) );
         return true;
      case ATTRIB_MESSAGE:
         message( VM::toString(vm, -1) );
         return true;
      case ATTRIB_MULTIPLE_SELECT:
         multipleSelect( VM::toBoolean(vm, -1) );
         return true;
      case ATTRIB_ON_CANCEL:
         VM::toRef( vm, -1, _onCancelRef );
         return true;
      case ATTRIB_ON_CONFIRM:
         VM::toRef( vm, -1, _onConfirmRef );
         return true;
      case ATTRIB_PATH:
      case ATTRIB_PATHS:
         // Read-only.
         return true;
      case ATTRIB_PROMPT:
         prompt( VM::toString(vm, -1) );
         return true;
      case ATTRIB_RESOLVE_ALIASES:
         resolveAliases( VM::toBoolean(vm, -1) );
         return true;
      case ATTRIB_SHOW_HIDDEN:
         showHidden( VM::toBoolean(vm, -1) );
         return true;
      case ATTRIB_STARTING_LOCATION:
         startingLocation( VM::toString(vm, -1) );
         return true;
      case ATTRIB_TITLE:
         title( VM::toString(vm, -1) );
         return true;
      case ATTRIB_TYPE:
         type( FileDialog::Type(VM::toUInt(vm, -1)) );
         return true;
      default:
         break;
   }
   return false;
}
