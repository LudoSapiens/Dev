/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/DataFlow/DFGeomModifier.h>
#include <Plasma/DataFlow/DFGraph.h>
#include <Plasma/DataFlow/DFNodeAttr.h>
#include <Plasma/DataFlow/DFGeometry.h>

#include <Plasma/Manipulator/Manipulator.h>

#include <Fusion/VM/VMFmt.h>

#if _MSC_VER
// 'this' used in member initializer list.
#pragma warning( disable: 4355 )
#endif

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

ConstString _mappingName;
ConstString _matIDName;

enum
{
   ID_OFFSET,
   ID_SCALE,
   ID_MAT_ID
};

//------------------------------------------------------------------------------
//!
RCP<DFNode> mappingVM( VMState* vm, int idx )
{
   RCP<DFMappingNode> node = new DFMappingNode();
   Vec3f off;
   if( VM::get( vm, idx, "offset", off ) ) node->offset( off );
   float scale;
   if( VM::get( vm, idx, "scale", scale ) ) node->scale( scale );
   return node;
}

//------------------------------------------------------------------------------
//!
RCP<DFNode> matIDVM( VMState* vm, int idx )
{
   RCP<DFMaterialIDNode> node = new DFMaterialIDNode();
   uint32_t id;
   if( VM::get( vm, idx, "id", id ) ) node->materialID( id );
   return node;
}

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   STATIC INITIALIZATION
==============================================================================*/

//-----------------------------------------------------------------------------
//!
void initializeGeomModifier()
{
   _mappingName      = "mapping";
   _matIDName        = "materialID";

   DFNodeSpec::registerNode(
      DFSocket::GEOMETRY,
      _mappingName, mappingVM,
      "Mapping", "Change the mapping of the geometry.",
      nullptr
   );
   DFNodeSpec::registerNode(
      DFSocket::GEOMETRY,
      _matIDName, matIDVM,
      "MatID", "Change the materialID of the geometry.",
      nullptr
   );
}

//-----------------------------------------------------------------------------
//!
void terminateGeomModifier()
{
   _mappingName     = ConstString();
   _matIDName       = ConstString();
}

/*==============================================================================
   CLASS DFMappingEditor
==============================================================================*/

class DFMappingEditor:
   public DFNodeEditor
{
public:

   /*----- methods -----*/

   DFMappingEditor( DFMappingNode* n ): _node(n) {}

   PLASMA_DLL_API virtual RCP<Manipulator>  manipulator() { return nullptr; }
   PLASMA_DLL_API virtual RCP<DFNodeAttrList>  attributes() const;
   PLASMA_DLL_API virtual RCP<DFNodeAttrStates>  attributesStates() const;
   PLASMA_DLL_API virtual void updateAttributes( const DFNodeAttrStates& );

protected:

   /*----- members -----*/

   DFMappingNode* _node;
};

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrList>
DFMappingEditor::attributes() const
{
   RCP<DFNodeAttrList> atts = new DFNodeAttrList();
   atts->add( DFNodeAttr( "FLOAT", ID_SCALE, "Scale" ) );
   atts->add( DFNodeAttr( "XYZ", ID_OFFSET, "Offset" ) );
   return atts;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrStates>
DFMappingEditor::attributesStates() const
{
   RCP<DFNodeAttrStates> states = new DFNodeAttrStates();
   states->set( ID_SCALE, _node->scale() );
   states->set( ID_OFFSET, _node->offset() );
   return states;
}

//------------------------------------------------------------------------------
//!
void
DFMappingEditor::updateAttributes( const DFNodeAttrStates& attribs )
{
   for( auto cur = attribs.begin(); cur != attribs.end(); ++cur )
   {
      switch( cur->_id )
      {
         case ID_SCALE:  _node->scale( (*cur)._value.getFloat() ); break;
         case ID_OFFSET: _node->offset( (*cur)._value.getVec3() ); break;
      }
      _node->graph()->invalidate( _node );
   }
}

/*==============================================================================
   CLASS DFMappingNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFMappingNode::DFMappingNode():
   _input(this), _scale(1.0f), _offset(0.0f)
{
   _output.delegate( makeDelegate( this, &DFMappingNode::process ) );
}

//------------------------------------------------------------------------------
//!
const ConstString&
DFMappingNode::name() const
{
   return _mappingName;
}

//------------------------------------------------------------------------------
//!
uint
DFMappingNode::numInputs() const
{
   return 1;
}

//------------------------------------------------------------------------------
//!
DFOutput*
DFMappingNode::output()
{
   return &_output;
}

//------------------------------------------------------------------------------
//!
DFInput*
DFMappingNode::input( uint id )
{
   if( id == 0 ) return &_input;
   return nullptr;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeEditor>
DFMappingNode::edit()
{
   if( _editor.isNull() )  _editor = new DFMappingEditor( this );
   return _editor;
}

//------------------------------------------------------------------------------
//!
bool
DFMappingNode::dumpCustom( TextStream& os, StreamIndent& indent ) const
{
   os << indent << "scale = " << _scale << "," << nl;
   os << indent << "offset = " << VMFmt( _offset ) << "," << nl;
   return os.ok();
}

//------------------------------------------------------------------------------
//!
RCP<DFGeometry>
DFMappingNode::process()
{
   RCP<DFGeometry> geom = _input.getGeometry();
   if( geom.isNull() ) return nullptr;

   // Don't work on the current version.
   geom = geom->clone();

   // Apply mapping.
   for( uint32_t p = 0; p < geom->numPatches(); ++p )
   {
      DFGeometry::Patch& patch = geom->patch(p);

      Vec3f dir = patch._vertices[0]._n +
                  patch._vertices[1]._n +
                  patch._vertices[2]._n +
                  patch._vertices[3]._n;

      uint a = dir.maxComponent();
      uint x, y;
      switch( a )
      {
         case 0: x=1; y=2; break;
         case 1: x=0; y=2; break;
         case 2: x=0; y=1; break;
      }

      for( int i = 0; i < 4; ++i )
         patch._uv[i] = (patch._vertices[i]._pos*_scale+_offset)(x,y);
   }

   return geom;
}

/*==============================================================================
   CLASS DFMatIDEditor
==============================================================================*/

class DFMatIDEditor:
   public DFNodeEditor
{
public:

   /*----- methods -----*/

   DFMatIDEditor( DFMaterialIDNode* n ): _node(n) {}

   PLASMA_DLL_API virtual RCP<Manipulator>  manipulator() { return nullptr; }
   PLASMA_DLL_API virtual RCP<DFNodeAttrList>  attributes() const;
   PLASMA_DLL_API virtual RCP<DFNodeAttrStates>  attributesStates() const;
   PLASMA_DLL_API virtual void updateAttributes( const DFNodeAttrStates& );

protected:

   /*----- members -----*/

   DFMaterialIDNode* _node;
};

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrList>
DFMatIDEditor::attributes() const
{
   RCP<DFNodeAttrList> atts = new DFNodeAttrList();
   atts->add( DFNodeAttr( "INT", ID_MAT_ID, "ID" ).range(0,128) );
   return atts;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrStates>
DFMatIDEditor::attributesStates() const
{
   RCP<DFNodeAttrStates> states = new DFNodeAttrStates();
   states->set( ID_MAT_ID, (float)_node->materialID() );
   return states;
}

//------------------------------------------------------------------------------
//!
void
DFMatIDEditor::updateAttributes( const DFNodeAttrStates& attribs )
{
   for( auto cur = attribs.begin(); cur != attribs.end(); ++cur )
   {
      switch( cur->_id )
      {
         case ID_MAT_ID:  _node->materialID( (uint32_t)(*cur)._value.getFloat() ); break;
      }
      _node->graph()->invalidate( _node );
   }
}

/*==============================================================================
   CLASS DFMaterialIDNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFMaterialIDNode::DFMaterialIDNode():
   _input(this), _matID(0)
{
   _output.delegate( makeDelegate( this, &DFMaterialIDNode::process ) );
}

//------------------------------------------------------------------------------
//!
const ConstString&
DFMaterialIDNode::name() const
{
   return _matIDName;
}

//------------------------------------------------------------------------------
//!
uint
DFMaterialIDNode::numInputs() const
{
   return 1;
}

//------------------------------------------------------------------------------
//!
DFOutput*
DFMaterialIDNode::output()
{
   return &_output;
}

//------------------------------------------------------------------------------
//!
DFInput*
DFMaterialIDNode::input( uint id )
{
   if( id == 0 ) return &_input;
   return nullptr;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeEditor>
DFMaterialIDNode::edit()
{
   if( _editor.isNull() )  _editor = new DFMatIDEditor( this );
   return _editor;
}

//------------------------------------------------------------------------------
//!
bool
DFMaterialIDNode::dumpCustom( TextStream& os, StreamIndent& indent ) const
{
   os << indent << "id = " << _matID << "," << nl;
   return os.ok();
}

//------------------------------------------------------------------------------
//!
RCP<DFGeometry>
DFMaterialIDNode::process()
{
   RCP<DFGeometry> geom = _input.getGeometry();
   if( geom.isNull() ) return nullptr;

   // Don't work on the current version.
   geom = geom->clone();

   // Change material ID.
   for( uint32_t p = 0; p < geom->numPatches(); ++p )
   {
      DFGeometry::Patch& patch = geom->patch(p);
      patch._id = _matID;
   }

   return geom;
}


NAMESPACE_END
