/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Widget/Label.h>
#include <Fusion/VM/VMObjectPool.h>


/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
const char* _label_str_ = "label";

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Label
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
Label::initialize()
{
   VMObjectPool::registerObject( "UI", _label_str_, stdCreateVM<Label>, stdGetVM<Label>, stdSetVM<Label> );
}

//------------------------------------------------------------------------------
//!
Label::Label() 
   : Widget()
{}

//------------------------------------------------------------------------------
//!
Label::~Label()
{}

//------------------------------------------------------------------------------
//!
void
Label::text
( const String& val )
{
   _text = val;
   updateLook();
}

//------------------------------------------------------------------------------
//!
const char*
Label::meta() const
{
   return _label_str_;
}

//------------------------------------------------------------------------------
//!
void
Label::init( VMState* vm )
{
   VM::get( vm, 1, "text", _text );
   
   // Base class init.
   Widget::init( vm );
}

//------------------------------------------------------------------------------
//!
bool
Label::performGet( VMState* vm )
{
   const char* param = VM::toCString( vm, 2 );

   if( strcmp( param, "text" ) == 0 )
   {
      VM::push( vm, _text );
      return true;
   }

   return Widget::performGet( vm );
}

//------------------------------------------------------------------------------
//!
bool
Label::performSet( VMState* vm )
{
   const char* param = VM::toCString( vm, 2 );

   if( strcmp( param, "text" ) == 0 )
   {
      _text = VM::toString( vm, 3 );
      updateLook();
      return true;
   }

   return Widget::performSet( vm );
}

//------------------------------------------------------------------------------
//!
bool
Label::isAttribute( const char* name ) const
{  
   if( strcmp( name, "text" ) == 0 ) return true;

   return Widget::isAttribute( name );
}

NAMESPACE_END
