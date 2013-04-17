/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Widget/TreeList.h>
#include <Fusion/VM/VMObjectPool.h>

#include <Base/ADT/StringMap.h>

/*==============================================================================
  UNNAME NAMESPACE
  ==============================================================================*/

UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
inline void
execute( const VMRef& ref, Widget* widget, const RCP<Widget>& item )
{
   if( ref.isValid() )
   {
      VMState* vm = ref.vm();
      VM::push( vm, ref );
      VM::pushProxy( vm, widget );
      VM::pushProxy( vm, item.ptr() );
      VM::ecall( vm, 2, 0 );
   }
}

//------------------------------------------------------------------------------
//!
int
childListModifiedVM( VMState* vm )
{
   TreeList* container = (TreeList*)VM::thisPtr( vm );

   container->childListModified();

   return 0;
}

//------------------------------------------------------------------------------
//!
enum {
   ATTRIB_ONITEM_SELECT,
   ATTRIB_TEXT,
   ATTRIB_SELECTED,
   ATTRIB_OPENED,
   ATTRIB_IS_LAST,
   ATTRIB_HAS_CHILD,
   ATTRIB_IS_ROOT,
   ATTRIB_CHILD_LIST_MODIFIED,
   ATTRIB_ONMODIFY
};

StringMap _attributes(
   "onItemSelect",      ATTRIB_ONITEM_SELECT,
   "text",              ATTRIB_TEXT,
   "selected",          ATTRIB_SELECTED,
   "opened",            ATTRIB_OPENED,
   "isLast",            ATTRIB_IS_LAST,
   "hasChild",          ATTRIB_HAS_CHILD,
   "isRoot",            ATTRIB_IS_ROOT,
   "childListModified", ATTRIB_CHILD_LIST_MODIFIED,
   "onModify",          ATTRIB_ONMODIFY,
   ""
);


//------------------------------------------------------------------------------
//!
const char* _treelist_str_ = "treelist";

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
  CLASS TreeList
  ==============================================================================*/

//------------------------------------------------------------------------------
//!
void
TreeList::initialize()
{
   VMObjectPool::registerObject( "UI", _treelist_str_, stdCreateVM<TreeList>, stdGetVM<TreeList>, stdSetVM<TreeList> );
}

//------------------------------------------------------------------------------
//!
TreeList::TreeList()
   : Box(),
     _opened( true ),
     _selected( false ),
     _isRoot( true ),
     _hasChild( false )
{
}

//------------------------------------------------------------------------------
//!
TreeList::~TreeList()
{}

//------------------------------------------------------------------------------
//!
void
TreeList::text( const String& val )
{
   if( val != _text )
   {
      _text = val;
      modified();
   }
}


//------------------------------------------------------------------------------
//!
void
TreeList::childListModified()
{
   // FIXME: If children are added or removed after init through WidgetContainer's addWidget() or removeWidget()
   //        then the user is responsible for calling the childListModified method.
   //        Failure to do so can make the _hasChild flag invalid.
   //        Also, the FLAG_AS_CHILD and UNHIDE:HIDE mechanisms wont be applied, resulting in erroneous look

   // First check if I have TreeList children
   _hasChild = false;
   Box::sendChildMessage( this, CHECK_CHILD );

   if( _hasChild )
   {
      // Tell them they are not the root
      Box::sendChildMessage( this, FLAG_AS_CHILD );

      // Tell them to hide or unhide
      Box::sendChildMessage( this, (_opened)?UNHIDE:HIDE );
   }

   // Notify the changes
   modified();
}


//------------------------------------------------------------------------------
//!
void
TreeList::addOnItemSelect
( const Delegate1<const RCP<Widget>&>& delegate )
{
   _onItemSelect.addDelegate( delegate );
}

//------------------------------------------------------------------------------
//
void
TreeList::removeOnItemSelect
( const Delegate1<const RCP<Widget>&>& delegate )
{
   _onItemSelect.removeDelegate( delegate );
}


//------------------------------------------------------------------------------
//!
void
TreeList::addOnModify
( const Delegate1<const RCP<Widget>&>& delegate )
{
   _onModify.addDelegate( delegate );
}

//------------------------------------------------------------------------------
//
void
TreeList::removeOnModify
( const Delegate1<const RCP<Widget>&>& delegate )
{
   _onModify.removeDelegate( delegate );
}


//------------------------------------------------------------------------------
//!
void TreeList::sendParentMessage( Widget* widget, int message )
{
   switch( message )
   {
      case CHILD_SELECTED:
      {
         _onItemSelect.exec( widget );
         execute( _onItemSelectRef, this, widget );

         if( _isRoot )
         {
            // Tell all child to unselect. Pass the originating widget, so that it doesn't unselect itself.
            sendChildMessage( widget, CHILD_UNSELECT );
            return; // no propagation
         }
      } break;

      case HAS_CHILD:
      {
         _hasChild = true;
      } return; // no propagation

      default: break;
   }

   Box::sendParentMessage( widget, message );
}

//------------------------------------------------------------------------------
//!
void TreeList::sendChildMessage( Widget* widget, int message )
{
   switch( message )
   {
      case CHILD_UNSELECT:
      {
         // Do not unselect the node that triggered the message
         if( widget != this )
         {
            selected( false );
         }
      }  break;
      case FLAG_AS_CHILD:
      {
         _isRoot = false;
         modified();
      }  return; // no propagation
      case FLAG_AS_ROOT:
      {
         _isRoot = true;
         modified();
      }  return; // no propagation
      case CHECK_CHILD:
      {
         Box::sendParentMessage( this, HAS_CHILD );
      } return; // no propagation
      case UNHIDE:
      {
         hide( false );
      } return; // no propagation
      case HIDE:
      {
         hide( true );
      } return; // no propagation
      default: break;
   }

   // Propagate messages that must
   Box::sendChildMessage( widget, message );
}

//------------------------------------------------------------------------------
//!
void
TreeList::selected( bool val )
{
   if( _selected != val )
   {
      if( val == true ) sendParentMessage( this, CHILD_SELECTED );

      _selected = val;

      modified();
   }
}

//------------------------------------------------------------------------------
//!
void
TreeList::opened( bool val )
{
   if( _opened != val )
   {
      _opened = val;

      if( _hasChild )
      {
         Box::sendChildMessage( this, (_opened)?UNHIDE:HIDE );
         modified();
      }
   }
}

//------------------------------------------------------------------------------
//!
void
TreeList::modified()
{
   _onModify.exec( this );
   execute( _onModifyRef, this, this );
}

//------------------------------------------------------------------------------
//!
const char*
TreeList::meta() const
{
   return _treelist_str_;
}

//------------------------------------------------------------------------------
//!
void
TreeList::init( VMState* vm )
{
   VM::get( vm, 1, "onItemSelect", _onItemSelectRef );
   VM::get( vm, 1, "text", _text );
   VM::get( vm, 1, "selected", _selected );
   VM::get( vm, 1, "opened", _opened );
   VM::get( vm, 1, "onModify", _onModifyRef );
   RCP<Widget> widget;

   // Base class init.
   Box::init( vm );

   // Now child are added, send them their messages

   // First check if I have TreeList children
   _hasChild = false;
   Box::sendChildMessage( this, CHECK_CHILD );

   if( _hasChild )
   {
      // Tell them they are not the root
      Box::sendChildMessage( this, FLAG_AS_CHILD );

      // Tell them to hide or unhide
      Box::sendChildMessage( this, (_opened)?UNHIDE:HIDE );
   }

   // Notify the changes
   modified();
}

//------------------------------------------------------------------------------
//!
bool
TreeList::performGet( VMState* vm )
{
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_ONITEM_SELECT:
         VM::push( vm, _onItemSelectRef );
         return true;
      case ATTRIB_TEXT:
         VM::push( vm, _text );
         return true;
      case ATTRIB_SELECTED:
         VM::push( vm, _selected );
         return true;
      case ATTRIB_OPENED:
         VM::push( vm, _opened );
         return true;
      case ATTRIB_IS_LAST:
         VM::push( vm, isLast() );
         return true;
      case ATTRIB_HAS_CHILD:
         VM::push( vm, hasChild() );
         return true;
      case ATTRIB_IS_ROOT:
         VM::push( vm, isRoot() );
         return true;
      case ATTRIB_CHILD_LIST_MODIFIED:
         VM::push( vm, this, childListModifiedVM );
         return true;
      case ATTRIB_ONMODIFY:
         VM::push( vm, _onModifyRef );
         return true;

      default: break;
   }

   return Box::performGet( vm );
}


//------------------------------------------------------------------------------
//!
bool
TreeList::performSet( VMState* vm )
{
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_ONITEM_SELECT:
         VM::toRef( vm, 3, _onItemSelectRef );
         return true;
      case ATTRIB_TEXT:
         _text = VM::toString( vm, 3 );
         _onModify.exec( this );
         execute( _onModifyRef, this, this );
         return true;
      case ATTRIB_SELECTED:
         selected( VM::toBoolean( vm, 3 ) );
         return true;
      case ATTRIB_OPENED:
         opened( VM::toBoolean( vm, 3 ) );
         return true;
      case ATTRIB_IS_LAST:
         return true;  // read-only
      case ATTRIB_HAS_CHILD:
         return true;  // read-only
      case ATTRIB_IS_ROOT:
         return true;  // read-only
      case ATTRIB_CHILD_LIST_MODIFIED:
         return true;  // read-only
      case ATTRIB_ONMODIFY:
         VM::toRef( vm, 3, _onModifyRef );
         return true;

      default: break;
   }

   bool handled = Box::performSet( vm );

   // TODO: there must be a better way of doing the following?
   // Unrecognized parameter, maybe a useful user-defined parameter? Call onModify
   modified();

   return handled;
}


//------------------------------------------------------------------------------
//!
bool
TreeList::isAttribute
( const char* name ) const
{
   if( _attributes[ name ] != StringMap::INVALID ) return true;
   return Box::isAttribute( name );
}


NAMESPACE_END
