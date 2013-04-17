/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/Drawable/Drawable.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Drawable
==============================================================================*/

//------------------------------------------------------------------------------
//!
Drawable::Drawable()
{
}

//------------------------------------------------------------------------------
//!
bool
Drawable::performGet( VMState* vm )
{
   VM::push( vm );
   return false;
}

//------------------------------------------------------------------------------
//!
bool
Drawable::performSet( VMState* )
{
   // Nothing for now.
   return false;
}

NAMESPACE_END
