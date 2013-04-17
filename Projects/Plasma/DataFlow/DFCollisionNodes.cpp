/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/DataFlow/DFCollisionNodes.h>
#include <Plasma/DataFlow/DFGraph.h>
#include <Plasma/DataFlow/DFNodeAttr.h>
#include <Plasma/DataFlow/DFGeometry.h>
#include <Plasma/Manipulator/Manipulator.h>

#if _MSC_VER
// 'this' used in member initializer list.
#pragma warning( disable: 4355 )
#endif

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

ConstString _autoCollisionName;

enum
{
   ID_TYPE,
};

//------------------------------------------------------------------------------
//!
RCP<DFNode> autoCollisionVM( VMState* vm, int idx )
{
   RCP<DFAutoCollisionNode> node = new DFAutoCollisionNode();
   int type;
   if( VM::get( vm, idx, "type", type ) ) node->collisionType( type );
   return node;
}

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   STATIC INITIALIZATION
==============================================================================*/

//-----------------------------------------------------------------------------
//!
void initializeCollisionNodes()
{
   _autoCollisionName = "autoCollision";

   DFNodeSpec::registerNode(
      DFSocket::GEOMETRY,
      _autoCollisionName, autoCollisionVM,
      "Auto Collision", "Set the geometry to use an automatic collision shape (convex hull or triangle mesh).",
      nullptr
   );
}

//-----------------------------------------------------------------------------
//!
void terminateCollisionNodes()
{
   _autoCollisionName = ConstString();
}

/*==============================================================================
   CLASS DFAutoCollisionEditor
==============================================================================*/

class DFAutoCollisionEditor:
   public DFNodeEditor
{
public:

   /*----- methods -----*/

   DFAutoCollisionEditor( DFAutoCollisionNode* n ): _node(n) {}

   PLASMA_DLL_API virtual RCP<Manipulator>  manipulator() { return nullptr; }
   PLASMA_DLL_API virtual RCP<DFNodeAttrList>  attributes() const;
   PLASMA_DLL_API virtual RCP<DFNodeAttrStates>  attributesStates() const;
   PLASMA_DLL_API virtual void updateAttributes( const DFNodeAttrStates& );

protected:

   /*----- members -----*/

   DFAutoCollisionNode* _node;
};

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrList>
DFAutoCollisionEditor::attributes() const
{
   RCP<DFNodeAttrList> atts = new DFNodeAttrList();
   RCP<Table> enums         = new Table();
   enums->pushBack( "Box" );
   enums->pushBack( "Sphere" );
   enums->pushBack( "Convex hull" );
   enums->pushBack( "Triangle mesh" );
   atts->add( DFNodeAttr( "ENUM", ID_TYPE, "Type" ).enums( enums.ptr() ) );
   return atts;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrStates>
DFAutoCollisionEditor::attributesStates() const
{
   RCP<DFNodeAttrStates> states = new DFNodeAttrStates();
   states->set( ID_TYPE, float(_node->collisionType()) );
   return states;
}

//------------------------------------------------------------------------------
//!
void
DFAutoCollisionEditor::updateAttributes( const DFNodeAttrStates& attribs )
{
   for( auto cur = attribs.begin(); cur != attribs.end(); ++cur )
   {
      if( cur->_id == ID_TYPE )
      {
         _node->collisionType( int((*cur)._value.getFloat()) );
      }
   }
   _node->graph()->invalidate( _node );
}

/*==============================================================================
   CLASS DFAutoCollisionNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFAutoCollisionNode::DFAutoCollisionNode():
   _input(this), _type( 0 )
{
   _output.delegate( makeDelegate( this, &DFAutoCollisionNode::process ) );
}

//------------------------------------------------------------------------------
//!
const ConstString&
DFAutoCollisionNode::name() const
{
   return _autoCollisionName;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeEditor>
DFAutoCollisionNode::edit()
{
   if( _editor.isNull() )  _editor = new DFAutoCollisionEditor( this );
   return _editor;
}

//------------------------------------------------------------------------------
//!
uint
DFAutoCollisionNode::numInputs() const
{
   return 1;
}

//------------------------------------------------------------------------------
//!
DFOutput*
DFAutoCollisionNode::output()
{
   return &_output;
}

//------------------------------------------------------------------------------
//!
DFInput*
DFAutoCollisionNode::input( uint id )
{
   if( id == 0 ) return &_input;
   return nullptr;
}

//------------------------------------------------------------------------------
//!
RCP<DFGeometry>
DFAutoCollisionNode::process()
{
   RCP<DFGeometry> geom = _input.getGeometry();
   if( geom.isNull() ) return nullptr;

   // Type 0 is MANUAL so we add +1.
   geom->collisionType( (Geometry::CollisionType)(_type+1) );

   return geom;
}

//-----------------------------------------------------------------------------
//!
bool
DFAutoCollisionNode::dumpCustom( TextStream& os, StreamIndent& indent ) const
{
   os << indent << "type = " << _type << nl;
   return os.ok();
}

NAMESPACE_END
