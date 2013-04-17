/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/DataFlow/DFNode.h>
#include <Plasma/DataFlow/DFNodeAttr.h>
#include <Plasma/Manipulator/Manipulator.h>

#include <Fusion/VM/VMFmt.h>

#include <Base/ADT/Map.h>
#include <Base/Dbg/Defs.h>

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

Map< ConstString, RCP<DFNodeSpec> >  _nodeSpecs;

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   CLASS DFSocket
==============================================================================*/

//------------------------------------------------------------------------------
//!
const char*
DFSocket::toStr( Type type )
{
   switch( type )
   {
      case FLOAT     : return "FLOAT";
      case VEC2      : return "VEC2";
      case VEC3      : return "VEC3";
      case VEC4      : return "VEC4";
      case SOUND     : return "SOUND";
      case IMAGE     : return "IMAGE";
      case GEOMETRY  : return "GEOMETRY";
      case ANIMATION : return "ANIMATION";
      case WORLD     : return "WORLD";
      case STROKES   : return "STROKES";
      case POLYGON   : return "POLYGON";
      default        : return "<UNKNOWN>";
   }
}


/*==============================================================================
   CLASS DFNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFNode::DFNode():
   _graph( nullptr ),
   _position( 0, 0 ), _width( 3 )
{
}

//------------------------------------------------------------------------------
//!
const String&
DFNode::label() const
{
   return spec().label();
}

//------------------------------------------------------------------------------
//!
const DFNodeSpec&
DFNode::spec() const
{
   return *DFNodeSpec::get( name() );
}

//------------------------------------------------------------------------------
//!
bool DFNode::isGraph() const
{
   return false;
}

//------------------------------------------------------------------------------
//!
uint DFNode::numInputs() const
{
   return 0;
}

//------------------------------------------------------------------------------
//!
DFOutput* DFNode::output()
{
   return nullptr;
}

//------------------------------------------------------------------------------
//!
DFInput* DFNode::input( uint )
{
   return nullptr;
}

//------------------------------------------------------------------------------
//!
bool DFNode::hasConnectedInput()
{
   uint n = numInputs();
   for( uint i = 0; i < n; ++i )
   {
      if( input(i)->isConnected() )  return true;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
bool DFNode::hasConnectedOutput()
{
   return output()->isConnected();
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeEditor>
DFNode::edit()
{
   return nullptr;
}

//-----------------------------------------------------------------------------
//!
bool
DFNode::dump( TextStream& os, StreamIndent& indent ) const
{
   bool ok = true;
   ok &= dumpBegin( os, indent );
   ok &= dumpCustom( os, indent );
   ok &= dumpEnd( os, indent );
   return ok;
}

//-----------------------------------------------------------------------------
//!
bool
DFNode::dumpBegin( TextStream& os, StreamIndent& indent ) const
{
   os << indent << name() << "{" << nl;
   ++indent;
   os << indent << "position = " << VMFmt( position() ) << "," << nl;
   os << indent << "width = " << width() << "," << nl;
   return os.ok();
}

//-----------------------------------------------------------------------------
//!
bool
DFNode::dumpCustom( TextStream& /*os*/, StreamIndent& /*indent*/ ) const
{
   return true;
}

//-----------------------------------------------------------------------------
//!
bool
DFNode::dumpEnd( TextStream& os, StreamIndent& indent ) const
{
   --indent;
   os << "}" << nl;
   return os.ok();
}


/*==============================================================================
  CLASS DFNodeEditor
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFNodeEditor::DFNodeEditor()
{
}

//------------------------------------------------------------------------------
//!
DFNodeEditor::~DFNodeEditor()
{
}

//------------------------------------------------------------------------------
//!
RCP<Manipulator>
DFNodeEditor::manipulator()
{
   return nullptr;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrList>
DFNodeEditor::attributes() const
{
   return nullptr;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrStates>
DFNodeEditor::attributesStates() const
{
   return nullptr;
}

//------------------------------------------------------------------------------
//!
void DFNodeEditor::updateAttributes( const DFNodeAttrStates& )
{
   // Nothing to do here.
}

/*==============================================================================
  CLASS DFNodeSpec
==============================================================================*/

//-----------------------------------------------------------------------------
//! Returns the DFNodeSpec associated to the specified node name, or nullptr if
//! none was registered for it.
const DFNodeSpec*
DFNodeSpec::get( const ConstString& name )
{
   auto it = _nodeSpecs.find( name );
   if( it != _nodeSpecs.end() )
   {
      return (*it).second.ptr();
   }
   return nullptr;
}

//-----------------------------------------------------------------------------
//! Returns all of the DFNodeSpecs in the destination vector.
void
DFNodeSpec::getAll( Vector<const DFNodeSpec*>& dst )
{
   for( auto cur = _nodeSpecs.begin(); cur != _nodeSpecs.end(); ++cur )
   {
      dst.pushBack( (*cur).second.ptr() );
   }
}

//-----------------------------------------------------------------------------
//! Registers a new DFNodeSpec.
//! Returns true on success, or false if a previous node was registered with the same name.
bool
DFNodeSpec::registerNode(
   DFSocket::Type     type,
   const ConstString& name,
         CreateVMFunc createFunc,
   const      String& label,
   const      String& info,
   const      String& icon
)
{
   if( !_nodeSpecs.has(name) )
   {
      // None registered, just add this one.
      DFNodeSpec*  spec = new DFNodeSpec();
      spec->_type       = type;
      spec->_name       = name;
      spec->_createFunc = createFunc;
      spec->_label      = label;
      spec->_info       = info;
      spec->_icon       = icon;
      _nodeSpecs[name]  = spec;
      return true;
   }
   else
   {
      // Already registered.
      CHECK( false );
      return false;
   }
}

//------------------------------------------------------------------------------
//! Unregisters a DFNodeSpec.
//! Returns true on succes, or false if no previous node was registered with the
//! specified name.
bool
DFNodeSpec::unregisterNode( const ConstString& name )
{
   if( _nodeSpecs.has(name) )
   {
      _nodeSpecs.erase( name );
      return true;
   }
   else
   {
      CHECK( false );
      return false;
   }
}
//------------------------------------------------------------------------------
//! Unregisters all registered DFNodeSpecs.
void
DFNodeSpec::unregisterAll()
{
   _nodeSpecs.clear();
}


NAMESPACE_END
