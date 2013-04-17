/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Widget/ComboBox.h>
#include <Fusion/VM/VMObjectPool.h>

#include <Fusion/Core/Core.h>
#include <Fusion/Widget/Desktop.h>
#include <Fusion/Widget/Menu.h>

#include <Base/ADT/StringMap.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
inline void
execute( const VMRef& ref, Widget* widget, const String& string )
{
   if( ref.isValid() )
   {
      VMState* vm = ref.vm();
      VM::push( vm, ref );
      VM::pushProxy( vm, widget );
      VM::push( vm, string );
      VM::ecall( vm, 2, 0 );
   }
}

//------------------------------------------------------------------------------
//!
enum {
   ATTRIB_ITEM_ID,
   ATTRIB_ITEM,
   ATTRIB_ONITEM_CHANGED,
   ATTRIB_MENU_ABOVE,
   ATTRIB_MENU_OFFSET,
   ATTRIB_OPENED
};

StringMap _attributes(
   "itemId",        ATTRIB_ITEM_ID,
   "item",          ATTRIB_ITEM,
   "onItemChanged", ATTRIB_ONITEM_CHANGED,
   "menuAbove",     ATTRIB_MENU_ABOVE,
   "menuOffset",    ATTRIB_MENU_OFFSET,
   "opened",        ATTRIB_OPENED,
   ""
);

//------------------------------------------------------------------------------
//!
const char* _comboBox_str_ = "comboBox";

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
  CLASS ComboBox
==============================================================================*/

//------------------------------------------------------------------------------
//!
void ComboBox::initialize()
{
   VMObjectPool::registerObject( "UI", _comboBox_str_, stdCreateVM<ComboBox>, stdGetVM<ComboBox>, stdSetVM<ComboBox> );
}

//------------------------------------------------------------------------------
//!
ComboBox::ComboBox():
   Widget(), _menuAbove( false ), _offset(0.0f)
{
}

//------------------------------------------------------------------------------
//!
ComboBox::~ComboBox()
{}

//------------------------------------------------------------------------------
//!
bool ComboBox::opened() const
{
   return _menu->isPopup();
}

//------------------------------------------------------------------------------
//!
void ComboBox::itemId( const String& str )
{
   _itemId = str;
   _item   = _menu->findWidget( _itemId );
   callShader();

   onItemChanged( _itemId );
}

//------------------------------------------------------------------------------
//!
void ComboBox::item( Widget* widget )
{
   _item   = widget;
   _itemId = widget->id();
   callShader();

   onItemChanged( _itemId );
}

//------------------------------------------------------------------------------
//!
void ComboBox::addOnItemChanged( const Delegate1<const String&>& delegate )
{
   _onItemChanged.addDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void ComboBox::removeOnItemChanged( const Delegate1<const String&>& delegate )
{
   _onItemChanged.removeDelegate( delegate );
}

//------------------------------------------------------------------------------
//!
void ComboBox::onClick( const Event& ev )
{
   // Set minimum width of menu to the current combobox.
   Vec2f nsize = Vec2f( CGM::max( actualSize().x - 2.0f*_offset.x, _menu->actualBaseSize().x ), _menu->size().y );
   _menu->size( nsize );

   Vec2f pos = _menuAbove ? Vec2f( 0.0f, actualSize().y ) :
                           -Vec2f( 0.0f, _menu->actualBaseSize().y );

   Core::desktop()->popup( _menu, globalPosition() + pos + _offset, nullptr );
   Widget::onClick( ev );
}

//------------------------------------------------------------------------------
//!
void ComboBox::onItemChanged( const String& itemId )
{
   _onItemChanged.exec( itemId );
   execute( _onItemChangedRef, this, itemId );
}

//------------------------------------------------------------------------------
//!
const char* ComboBox::meta() const
{
   return _comboBox_str_;
}

//------------------------------------------------------------------------------
//!
void ComboBox::init( VMState* vm )
{
   VM::get( vm, 1, "itemId", _itemId );
   VM::get( vm, 1, "onItemChanged", _onItemChangedRef );
   VM::get( vm, 1, "menuOffset", _offset );
   VM::get( vm, 1, "menuAbove", _menuAbove );

   // Get menu.
   RCP<Widget> menu;
   VM::getWidget( vm, 1, "menu", menu );
   _menu = static_cast<Menu*>( menu.ptr() );

   _menu->addOnItemSelect( makeDelegate( this, &ComboBox::itemSelected ) );
   _menu->addOnPopup( makeDelegate( this, &ComboBox::popupped ) );
   _item = _menu->findWidget( _itemId );

   // Base class init.
   Widget::init( vm );
}

//------------------------------------------------------------------------------
//!
bool ComboBox::performGet( VMState* vm )
{
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_ITEM_ID:
         VM::push( vm, _itemId );
         return true;
      case ATTRIB_ITEM:
         VM::pushProxy( vm, _item.ptr() );
         return true;
      case ATTRIB_ONITEM_CHANGED:
         VM::push( vm, _onItemChangedRef );
         return true;
      case ATTRIB_MENU_ABOVE:
         VM::push( vm, _menuAbove );
         return true;
      case ATTRIB_MENU_OFFSET:
         VM::push( vm, _offset );
         return true;
      case ATTRIB_OPENED:
         VM::push( vm, opened() );
         return true;
      default: break;
   }

   return Widget::performGet( vm );
}

//------------------------------------------------------------------------------
//!
bool ComboBox::performSet( VMState* vm )
{
   switch( _attributes[ VM::toCString( vm, 2 ) ] )
   {
      case ATTRIB_ITEM_ID:
         itemId( VM::toCString( vm, 3 ) );
         return true;
      case ATTRIB_ITEM: // Read only.
         return true;
      case ATTRIB_ONITEM_CHANGED:
         VM::toRef( vm, 3, _onItemChangedRef );
         return true;
      case ATTRIB_MENU_ABOVE:
         _menuAbove = VM::toBoolean( vm, 3 );
         return true;
      case ATTRIB_MENU_OFFSET:
         _offset = VM::toVec2f( vm, 3 );
         return true;
      case ATTRIB_OPENED:
         return false;
      default: break;
   }

   return Widget::performSet( vm );
}

//------------------------------------------------------------------------------
//!
bool ComboBox::isAttribute( const char* name ) const
{
   if( _attributes[ name ] != StringMap::INVALID ) return true;

   return Widget::isAttribute( name );
}

//------------------------------------------------------------------------------
//!
void ComboBox::itemSelected( Widget* widget )
{
   item( widget );
}

//------------------------------------------------------------------------------
//!
void ComboBox::popupped( Widget* w )
{
   // State "opened" changed, call an update.
   if( !w->isPopup() ) callShader();
}

NAMESPACE_END
