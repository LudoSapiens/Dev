/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Widget/Menu.h>
#include <Fusion/VM/VMObjectPool.h>

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
const char* _menu_str_ = "menu";


UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Menu
==============================================================================*/

//------------------------------------------------------------------------------
//!
void Menu::initialize()
{
   VMObjectPool::registerObject( "UI", _menu_str_, stdCreateVM<Menu>, stdGetVM<Menu>, stdSetVM<Menu> );
}

//------------------------------------------------------------------------------
//!
Menu::Menu():
   Box()
{
   // Override default value.
   _orient = VERTICAL;
}

//------------------------------------------------------------------------------
//!
Menu::~Menu()
{}

//------------------------------------------------------------------------------
//!
void Menu::addOnItemSelect( const WidgetDelegate& delegate )
{
   _onItemSelect.addDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void Menu::removeOnItemSelect( const WidgetDelegate& delegate )
{
   _onItemSelect.removeDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void Menu::sendParentMessage( Widget* widget, int message )
{
   if( message == ITEM_SELECTED )
   {
      onItemSelect( widget );
   }
   else
   {
      WidgetContainer::sendParentMessage( widget, message );
   }
}

//------------------------------------------------------------------------------
//!
void Menu::onItemSelect( Widget* item )
{
   _onItemSelect.exec( item );
   execute( _onItemSelectRef, this, item );
}

//------------------------------------------------------------------------------
//!
const char* Menu::meta() const
{
   return _menu_str_;
}

//------------------------------------------------------------------------------
//!
void Menu::init( VMState* vm )
{
   VM::get( vm, 1, "onItemSelect", _onItemSelectRef );

   // Base class init.
   Box::init( vm );
}

//------------------------------------------------------------------------------
//!
bool Menu::performGet( VMState* vm )
{
   const char* param = VM::toCString( vm, 2 );

   if( strcmp( param, "onItemSelect" ) == 0 )
   {
      VM::push( vm, _onItemSelectRef );
      return true;
   }

   return Box::performGet( vm );
}

//------------------------------------------------------------------------------
//!
bool Menu::performSet( VMState* vm )
{
   const char* param = VM::toCString( vm, 2 );

   if( strcmp( param, "onItemSelect" ) == 0 )
   {
      VM::toRef( vm, 3, _onItemSelectRef );
      return true;
   }

   return Box::performSet( vm );
}

//------------------------------------------------------------------------------
//!
bool Menu::isAttribute( const char* name ) const
{
   if( strcmp( name, "onItemSelect" ) == 0 ) return true;

   return Box::isAttribute( name );
}

NAMESPACE_END
