/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Widget/MenuItem.h>
#include <Fusion/Widget/Desktop.h>
#include <Fusion/Widget/Menu.h>
#include <Fusion/VM/VMObjectPool.h>
#include <Fusion/Core/Core.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
const char* _menuItem_str_ = "menuItem";

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
  CLASS MenuItem
==============================================================================*/

//------------------------------------------------------------------------------
//!
void MenuItem::initialize()
{
   VMObjectPool::registerObject( "UI", _menuItem_str_, stdCreateVM<MenuItem>, stdGetVM<MenuItem>, stdSetVM<MenuItem> );
}

//------------------------------------------------------------------------------
//!
MenuItem::MenuItem()
   : Widget()
{}

//------------------------------------------------------------------------------
//!
MenuItem::~MenuItem()
{}

//------------------------------------------------------------------------------
//!
Widget* MenuItem::findWidget( const String& wid )
{
   if( wid == id() ) return this;
   if( _menu.isValid() ) return _menu->findWidget( wid );
   return NULL;
}

//------------------------------------------------------------------------------
//!
void MenuItem::onClick( const Event& ev )
{
   if( _menu.isNull() )
   {
      Core::desktop()->closePopups();

      sendParentMessage( this, Menu::ITEM_SELECTED );
   }

   Widget::onClick( ev );
}

//------------------------------------------------------------------------------
//!
void MenuItem::onPointerEnter( const Event& ev )
{
   // Close other menus that are possibly opened.
   Core::desktop()->closePopupsAfter( RCP<Widget>( this ) );

   // Open our menu.
   if( _menu.isValid() )
   {
      Vec2f pos( globalPosition() + Vec2f( actualSize().x, 0.0f ) );
      Core::desktop()->popup( _menu, pos, this );
   }

   Widget::onPointerEnter( ev );
}

//------------------------------------------------------------------------------
//!
bool MenuItem::isAttribute( const char* name ) const
{
   if( strcmp( name, "menu" ) == 0 ) return true;

   return Widget::isAttribute( name );
}

//------------------------------------------------------------------------------
//!
const char* MenuItem::meta() const
{
   return _menuItem_str_;
}

//------------------------------------------------------------------------------
//!
void MenuItem::itemSelected( Widget* item )
{
   if( parent() )
   {
      parent()->sendParentMessage( item, Menu::ITEM_SELECTED );
   }
}


//------------------------------------------------------------------------------
//!
void MenuItem::init( VMState* vm )
{
   // Get menu.
   RCP<Widget> menu;
   VM::getWidget( vm, 1, "menu", menu );
   _menu = static_cast<Menu*>( menu.ptr() );

   if( _menu.isValid() )
   {
      _menu->addOnItemSelect( makeDelegate( this, &MenuItem::itemSelected ) );
   }

   // Base class init.
   Widget::init( vm );
}

//------------------------------------------------------------------------------
//!
bool MenuItem::performGet( VMState* vm )
{
   const char* param = VM::toCString( vm, 2 );

   if( strcmp( param, "menu" ) == 0 )
   {
      VM::pushProxy( vm, _menu.ptr() );
      return true;
   }

   return Widget::performGet( vm );
}

//------------------------------------------------------------------------------
//!
bool MenuItem::performSet( VMState* vm )
{
   const char* param = VM::toCString( vm, 2 );

   if( strcmp( param, "menu" ) == 0 )
   {
      if( _menu.isValid() )
      {
         _menu->removeOnItemSelect( makeDelegate( this, &MenuItem::itemSelected ) );
      }

      // Get menu.
      _menu = (Menu*)VM::toProxy( vm, 3 );
      if( _menu.isValid() )
      {
         _menu->addOnItemSelect( makeDelegate( this, &MenuItem::itemSelected ) );
      }
      callShader();
      return true;
   }

   return Widget::performSet( vm );
}

NAMESPACE_END
