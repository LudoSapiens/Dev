/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/DataFlow/DFNodeAttr.h>

#include <Base/ADT/HashTable.h>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

ConstString  _compact_str;
ConstString  _range_str;
ConstString  _step_str;
ConstString  _length_str;

UNNAMESPACE_END


/*==============================================================================
  CLASS DFNodeAttr
==============================================================================*/

const DFNodeAttr  DFNodeAttr::_null = DFNodeAttr( ConstString(), 0, ConstString() );

//------------------------------------------------------------------------------
//!
void
DFNodeAttr::initialize()
{
   _compact_str = "compact";
   _range_str   = "range";
   _step_str    = "step";
   _length_str  = "length";
}

//------------------------------------------------------------------------------
//!
void
DFNodeAttr::terminate()
{
   _compact_str = ConstString();
   _range_str   = ConstString();
   _step_str    = ConstString();
   _length_str  = ConstString();
}

//------------------------------------------------------------------------------
//!
DFNodeAttr&
DFNodeAttr::compact( bool v )
{
   e().set( _compact_str, v );
   return *this;
}

//------------------------------------------------------------------------------
//!
DFNodeAttr&
DFNodeAttr::range( float min, float max )
{
   e().set( _range_str, Vec2f(min, max) );
   return *this;
}

//------------------------------------------------------------------------------
//!
DFNodeAttr&
DFNodeAttr::step( float v )
{
   e().set( _step_str, v );
   return *this;
}

//------------------------------------------------------------------------------
//!
DFNodeAttr&
DFNodeAttr::length( float v )
{
   e().set( _length_str, v );
   return *this;
}

/*==============================================================================
  CLASS DFNodeAttrGroup
==============================================================================*/

