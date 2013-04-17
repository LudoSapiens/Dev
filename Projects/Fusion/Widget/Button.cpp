/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Widget/Button.h>
#include <Fusion/VM/VMObjectPool.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
const char* _button_str_ = "button";

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Button
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
Button::initialize()
{
   VMObjectPool::registerObject( "UI", _button_str_, stdCreateVM<Button>, stdGetVM<Button>, stdSetVM<Button> );
}

//------------------------------------------------------------------------------
//!
Button::Button()
   : Widget(),
     _toggled( false )
{}

//------------------------------------------------------------------------------
//!
Button::~Button()
{}

//------------------------------------------------------------------------------
//!
void
Button::onClick( const Event& ev )
{
   _toggled = !_toggled;
   callShader();
  Widget::onClick( ev );
}

//------------------------------------------------------------------------------
//!
const char*
Button::meta() const
{
   return _button_str_;
}

//------------------------------------------------------------------------------
//!
void
Button::init( VMState* vm )
{
   VM::get( vm, 1, "toggled", _toggled );
   
   // Base class init.
   Widget::init( vm );
}

//------------------------------------------------------------------------------
//!
bool
Button::performGet( VMState* vm )
{
   const char* param = VM::toCString( vm, 2 );

   if( strcmp( param, "toggled" ) == 0 )
   {
      VM::push( vm, _toggled );
      return true;
   }

   return Widget::performGet( vm );
}

//------------------------------------------------------------------------------
//!
bool
Button::performSet( VMState* vm )
{
   const char* param = VM::toCString( vm, 2 );

   if( strcmp( param, "toggled" ) == 0 )
   {
      _toggled = VM::toBoolean( vm, 3 );
      callShader();
      return true;
   }

   return Widget::performSet( vm );
}

//------------------------------------------------------------------------------
//!
bool
Button::isAttribute( const char* name ) const
{  
   if( strcmp( name, "toggled" ) == 0 ) return true;
   return Widget::isAttribute( name );
}

NAMESPACE_END
