/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Widget/RadialButton.h>
#include <Fusion/VM/VMObjectPool.h>
#include <Fusion/Core/Event.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
const char* _radialButton_str_ = "radialButton";

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
  CLASS RadialButton
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
RadialButton::initialize()
{
   VMObjectPool::registerObject( "UI", _radialButton_str_, stdCreateVM<RadialButton>, stdGetVM<RadialButton>, stdSetVM<RadialButton> );
}

//------------------------------------------------------------------------------
//!
RadialButton::RadialButton()
   : Widget()
{}

//------------------------------------------------------------------------------
//!
RadialButton::~RadialButton()
{}

//------------------------------------------------------------------------------
//!
void
RadialButton::onPointerPress( const Event& /*ev*/ )
{
   _state |= PRESSED;
   callShader();
}

//------------------------------------------------------------------------------
//!
void
RadialButton::onPointerRelease( const Event& ev )
{
   _state &= ~( PRESSED | HOVERED );
   callShader();
   onClick( ev );
}

//------------------------------------------------------------------------------
//!
void
RadialButton::onPointerEnter( const Event& ev )
{
   _state |= HOVERED;
   if( ev.value() ) _state |= PRESSED;
   callShader();
}

//------------------------------------------------------------------------------
//!
void
RadialButton::onPointerLeave( const Event& /*ev*/ )
{
   _state &= ~( HOVERED | PRESSED );
   callShader();
}

//------------------------------------------------------------------------------
//!
const char*
RadialButton::meta() const
{
   return _radialButton_str_;
}

//------------------------------------------------------------------------------
//!
void
RadialButton::init( VMState* vm )
{
   // Base class init.
   Widget::init( vm );
}


NAMESPACE_END
