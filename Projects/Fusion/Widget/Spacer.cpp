/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Widget/Spacer.h>
#include <Fusion/VM/VMObjectPool.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
const char* _spacer_str_ = "spacer";

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Spacer
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
Spacer::initialize()
{
   VMObjectPool::registerObject( "UI", _spacer_str_, stdCreateVM<Spacer>, stdGetVM<Spacer>, stdSetVM<Spacer> );
}

//------------------------------------------------------------------------------
//!
Spacer::Spacer()
   : Widget()
{
   // FIXME Should we have a default flex of 1?
   enableEvents( false );
}

//------------------------------------------------------------------------------
//!
Spacer::~Spacer()
{}

//------------------------------------------------------------------------------
//!
void
Spacer::render( const RCP<Gfx::RenderNode>& )
{}

//------------------------------------------------------------------------------
//!
const char*
Spacer::meta() const
{
   return _spacer_str_;
}

//------------------------------------------------------------------------------
//!
void
Spacer::init( VMState* vm )
{
   Widget::init( vm );
}

NAMESPACE_END
