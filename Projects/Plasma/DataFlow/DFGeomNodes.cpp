/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/DataFlow/DFGeomNodes.h>
#include <Plasma/DataFlow/DFGraph.h>
#include <Plasma/DataFlow/DFNodeAttr.h>
#include <Plasma/DataFlow/DFGeometry.h>
#include <Plasma/DataFlow/DFGeomOps.h>

#include <Plasma/Manipulator/RefManipulator.h>
#include <Plasma/Resource/ResManager.h>

#include <Fusion/VM/VMFmt.h>

#include <Base/Dbg/Defs.h>
#include <Base/MT/Thread.h>

#if _MSC_VER
// 'this' used in member initializer list.
#pragma warning( disable: 4355 )
#endif

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

ConstString _differenceName;
ConstString _intersectionName;
ConstString _unionName;
ConstString _transformName;
ConstString _cloneName;
ConstString _geomImportName;

enum
{
   ID_NAME,
   ID_REF,
   ID_POS,
   ID_ORI,
   ID_NUM,
   ID_OFFSET
};

//------------------------------------------------------------------------------
//!
RCP<DFNode> differenceVM( VMState*, int )
{
   RCP<DFDifferenceNode> node = new DFDifferenceNode();
   return node;
}

//------------------------------------------------------------------------------
//!
RCP<DFNode> intersectionVM( VMState*, int )
{
   RCP<DFIntersectionNode> node = new DFIntersectionNode();
   return node;
}

//------------------------------------------------------------------------------
//!
RCP<DFNode> unionVM( VMState*, int )
{
   RCP<DFUnionNode> node = new DFUnionNode();
   return node;
}

//------------------------------------------------------------------------------
//!
RCP<DFNode> transformVM( VMState* vm, int idx )
{
   RCP<DFTransformNode> node = new DFTransformNode();
   Reff ref;
   if( VM::get( vm, idx, "ref", ref ) ) node->referential( ref );
   return node;
}

//------------------------------------------------------------------------------
//!
RCP<DFNode> cloneVM( VMState* vm, int idx )
{
   RCP<DFCloneNode> node = new DFCloneNode();
   Vec3f off;
   if( VM::get( vm, idx, "offset", off ) ) node->offset( off );
   Reff ref;
   if( VM::get( vm, idx, "rotation", ref ) ) node->rotation( ref );
   uint num;
   if( VM::get( vm, idx, "num", num ) ) node->numClones( num );
   return node;
}

//------------------------------------------------------------------------------
//!
RCP<DFNode> geomImportVM( VMState* vm, int idx )
{
   RCP<DFGeomImportNode> node = new DFGeomImportNode();
   if( VM::geti( vm, idx, 1 ) )
   {
      node->geomID( VM::toString( vm, -1 ) );
      VM::pop( vm );
   }
   else
   {
      StdErr << "ERROR: Missing geom ID in geomImportVM()." << nl;
   }
   return node;
}

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   STATIC INITIALIZATION
==============================================================================*/

//-----------------------------------------------------------------------------
//!
void initializeGeomNodes()
{
   _differenceName   = "difference";
   _intersectionName = "intersection";
   _unionName        = "union";
   _transformName    = "transform";
   _geomImportName   = "geomImport";
   _cloneName        = "clone";

   DFNodeSpec::registerNode(
      DFSocket::GEOMETRY,
      _differenceName, differenceVM,
      "Difference", "Difference boolean operation.",
      "image/ui/dfnode/difference"
   );
   DFNodeSpec::registerNode(
      DFSocket::GEOMETRY,
      _intersectionName, intersectionVM,
      "Intersection", "Intersection boolean ope::ration.",
      "image/ui/dfnode/intersection"
   );
   DFNodeSpec::registerNode(
      DFSocket::GEOMETRY,
      _unionName, unionVM,
      "Union", "Union boolean operation.",
      "image/ui/dfnode/union"
   );
   DFNodeSpec::registerNode(
      DFSocket::GEOMETRY,
      _transformName, transformVM,
      "Transform", "Apply a referential to a geometry.",
      nullptr
   );
   DFNodeSpec::registerNode(
      DFSocket::GEOMETRY,
      _cloneName, cloneVM,
      "Clone", "Clone geometry.",
      nullptr
   );
   DFNodeSpec::registerNode(
      DFSocket::GEOMETRY,
      _geomImportName, geomImportVM,
      "Import", "Geometry import.",
      nullptr
   );
}

//-----------------------------------------------------------------------------
//!
void terminateGeomNodes()
{
   _differenceName   = ConstString();
   _intersectionName = ConstString();
   _unionName        = ConstString();
   _transformName    = ConstString();
   _cloneName        = ConstString();
   _geomImportName   = ConstString();
}


/*==============================================================================
   CLASS DFGeomOutput
==============================================================================*/

//------------------------------------------------------------------------------
//!
RCP<DFGeometry>
DFGeomOutput::getGeometry()
{
   return _delegate();
}

//------------------------------------------------------------------------------
//!
DFSocket::Type
DFGeomOutput::type() const
{
   return GEOMETRY;
}

/*==============================================================================
   CLASS DFGeomInput
==============================================================================*/

//------------------------------------------------------------------------------
//!
RCP<DFGeometry>
DFGeomInput::getGeometry()
{
   if( !_output ) return nullptr;
   return _output->getGeometry();
}

//------------------------------------------------------------------------------
//!
DFSocket::Type
DFGeomInput::type() const
{
   return GEOMETRY;
}

//------------------------------------------------------------------------------
//!
bool
DFGeomInput::isConnected() const
{
   return _output != nullptr;
}

//------------------------------------------------------------------------------
//!
void
DFGeomInput::connect( DFOutput* output )
{
   CHECK( type() == output->type() );
   if( _output ) disconnectFrom( _output );
   _output = (DFGeomOutput*)output;
}

//------------------------------------------------------------------------------
//!
void
DFGeomInput::disconnect( DFOutput* output )
{
   if( _output == output ) _output = nullptr;
}

//------------------------------------------------------------------------------
//!
void
DFGeomInput::disconnect()
{
   if( !_output ) return;
   disconnectFrom( _output );
   _output = nullptr;
}

/*==============================================================================
   CLASS DFGeomMultiInput
==============================================================================*/

//------------------------------------------------------------------------------
//!
RCP<DFGeometry>
DFGeomMultiInput::getGeometry( uint i )
{
   if( i >= _outputs.size() ) return nullptr;
   return _outputs[i]->getGeometry();
}

//------------------------------------------------------------------------------
//!
DFSocket::Type
DFGeomMultiInput::type() const
{
   return GEOMETRY;
}

//------------------------------------------------------------------------------
//!
bool
DFGeomMultiInput::isConnected() const
{
   return !_outputs.empty();
}

//------------------------------------------------------------------------------
//!
void
DFGeomMultiInput::connect( DFOutput* output )
{
   CHECK( type() == output->type() );
   _outputs.pushBack( (DFGeomOutput*)output );
}

//------------------------------------------------------------------------------
//!
void
DFGeomMultiInput::disconnect( DFOutput* output )
{
   _outputs.removeSwap( (DFGeomOutput*)output );
}

//------------------------------------------------------------------------------
//!
void
DFGeomMultiInput::disconnect()
{
   for( auto it = _outputs.begin(); it != _outputs.end(); ++it )
   {
      disconnectFrom( *it );
   }
   _outputs.clear();
}

/*==============================================================================
   CLASS DFDifferenceNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFDifferenceNode::DFDifferenceNode():
   _minuend(this),
   _subtrahends(this)
{
   _output.delegate( makeDelegate( this, &DFDifferenceNode::process ) );
}

//------------------------------------------------------------------------------
//!
const ConstString&
DFDifferenceNode::name() const
{
   return _differenceName;
}

//------------------------------------------------------------------------------
//!
uint
DFDifferenceNode::numInputs() const
{
   return 2;
}

//------------------------------------------------------------------------------
//!
DFOutput*
DFDifferenceNode::output()
{
   return &_output;
}

//------------------------------------------------------------------------------
//!
DFInput*
DFDifferenceNode::input( uint id )
{
   switch( id )
   {
      case 0: return &_minuend;
      case 1: return &_subtrahends;
   }

   return nullptr;
}

//------------------------------------------------------------------------------
//!
RCP<DFGeometry>
DFDifferenceNode::process()
{
   RCP<DFGeometry> minuend = _minuend.getGeometry();

   // Do we have a valid input?
   if( minuend.isNull() ) return nullptr;

   // Don't work on the current version.
   RCP<DFGeometry> geom = minuend->clone();

   // Subtract every geometry input from the first one.
   const uint n = uint(_subtrahends.size());
   for( uint i = 0; i < n; ++i )
   {
      geom->subtract( _subtrahends.getGeometry(i).ptr() );
   }

   return geom;
}

/*==============================================================================
   CLASS DFIntersectionNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFIntersectionNode::DFIntersectionNode():
   _blocks(this)
{
   _output.delegate( makeDelegate( this, &DFIntersectionNode::process ) );
}

//------------------------------------------------------------------------------
//!
const ConstString&
DFIntersectionNode::name() const
{
   return _intersectionName;
}

//------------------------------------------------------------------------------
//!
uint
DFIntersectionNode::numInputs() const
{
   return 1;
}

//------------------------------------------------------------------------------
//!
DFOutput*
DFIntersectionNode::output()
{
   return &_output;
}

//------------------------------------------------------------------------------
//!
DFInput*
DFIntersectionNode::input( uint id )
{
   if( id == 0 ) return &_blocks;
   return nullptr;
}

//------------------------------------------------------------------------------
//!
RCP<DFGeometry>
DFIntersectionNode::process()
{
   // Do we have a valid input?
   if( _blocks.size() == 0 ) return nullptr;

   // Check whether or not we have a valid geometry to start.
   RCP<DFGeometry> geom = _blocks.getGeometry(0);
   if( geom.isNull() ) return nullptr;

   // Don't work on the current version.
   geom = geom->clone();

   // Intersect every geometry input from the first one.
   const uint n = uint(_blocks.size());
   for( uint i = 1; i < n; ++i )
   {
      geom->intersect( _blocks.getGeometry(i).ptr() ); // Intersect is robust.
   }

   return geom;
}

/*==============================================================================
   CLASS DFUnionNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFUnionNode::DFUnionNode():
   _blocks(this)
{
   _output.delegate( makeDelegate( this, &DFUnionNode::process ) );
}

//------------------------------------------------------------------------------
//!
const ConstString&
DFUnionNode::name() const
{
   return _unionName;
}

//------------------------------------------------------------------------------
//!
uint
DFUnionNode::numInputs() const
{
   return 1;
}

//------------------------------------------------------------------------------
//!
DFOutput*
DFUnionNode::output()
{
   return &_output;
}

//------------------------------------------------------------------------------
//!
DFInput*
DFUnionNode::input( uint id )
{
   if( id == 0 ) return &_blocks;
   return nullptr;
}

//------------------------------------------------------------------------------
//!
RCP<DFGeometry>
DFUnionNode::process()
{
   // Do we have a valid input?
   if( _blocks.size() == 0 ) return nullptr;

   // Check whether or not we have a valid geometry to start.
   RCP<DFGeometry> geom = _blocks.getGeometry(0);
   if( geom.isNull() ) return nullptr;

   // Don't work on the current version.
   geom = geom->clone();

   // Merge every geometry input from the first one.
   const uint n = uint(_blocks.size());
   for( uint i = 1; i < n; ++i )
   {
      geom->merge( _blocks.getGeometry(i).ptr() ); // Merge is robust.
   }

   return geom;
}

/*==============================================================================
   CLASS DFTransformEditor
==============================================================================*/

class DFTransformEditor:
   public DFNodeEditor
{
public:

   /*----- methods -----*/

   DFTransformEditor( DFTransformNode* n ): _node(n) {}

   PLASMA_DLL_API virtual RCP<Manipulator>  manipulator();
   PLASMA_DLL_API virtual RCP<DFNodeAttrList>  attributes() const;
   PLASMA_DLL_API virtual RCP<DFNodeAttrStates>  attributesStates() const;
   PLASMA_DLL_API virtual void updateAttributes( const DFNodeAttrStates& );

   void move( const Vec3f& p );
   void rotate( const Quatf& ori );

protected:

   /*----- methods -----*/

   void updateUI();
   void referentialCb();

   /*----- members -----*/

   DFTransformNode*   _node;
   RCP<RefRenderable> _renderable;
};

//------------------------------------------------------------------------------
//!
RCP<Manipulator>
DFTransformEditor::manipulator()
{
   if( _renderable.isNull() )
   {
      _renderable = new RefRenderable();
      _renderable->addOnModify( makeDelegate( this, &DFTransformEditor::referentialCb ) );
      _renderable->update( _node->referential() );
   }
   return RCP<Manipulator>( new RefManipulator( _renderable.ptr() ) );
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrList>
DFTransformEditor::attributes() const
{
   RCP<DFNodeAttrList> atts = new DFNodeAttrList();
   RCP<DFNodeAttrList> ref  = new DFNodeAttrList();
   ref->add( DFNodeAttr( "XYZ", ID_POS, "Position" ) );
   ref->add( DFNodeAttr( "ORI", ID_ORI, "Orientation" ) );
   atts->add( DFNodeAttr( ID_REF, "", ref.ptr() ) );

   return atts;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrStates>
DFTransformEditor::attributesStates() const
{
   RCP<DFNodeAttrStates> states = new DFNodeAttrStates();
   states->set( ID_POS, _node->referential().position() );
   states->set( ID_ORI, _node->referential().orientation() );
   return states;
}

//------------------------------------------------------------------------------
//!
void
DFTransformEditor::updateAttributes( const DFNodeAttrStates& attribs )
{
   for( auto cur = attribs.begin(); cur != attribs.end(); ++cur )
   {
      switch( cur->_id )
      {
         case ID_POS: move( (*cur)._value.getVec3() );   break;
         case ID_ORI: rotate( (*cur)._value.getQuat() ); break;
         default:;
      }
   }
}

//------------------------------------------------------------------------------
//!
void
DFTransformEditor::move( const Vec3f& p )
{
   _node->position( p );
   _node->graph()->invalidate( _node );
   if( _renderable.isValid() )
      _renderable->update( _node->referential() );
}

//------------------------------------------------------------------------------
//!
void
DFTransformEditor::rotate( const Quatf& ori )
{
   _node->orientation( ori );
   _node->graph()->invalidate( _node );
   if( _renderable.isValid() )
      _renderable->update( _node->referential() );
}

//------------------------------------------------------------------------------
//!
void
DFTransformEditor::updateUI()
{
   _node->graph()->msg().modify( _node, attributesStates().ptr() );
}

//------------------------------------------------------------------------------
//!
void
DFTransformEditor::referentialCb()
{
   _node->referential( _renderable->referential() );
   _node->graph()->invalidate( _node );
   updateUI();
}

/*==============================================================================
   CLASS DFTransformNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFTransformNode::DFTransformNode():
   _input(this), _referential( Reff::identity() )
{
   _output.delegate( makeDelegate( this, &DFTransformNode::process ) );
}

//------------------------------------------------------------------------------
//!
const ConstString&
DFTransformNode::name() const
{
   return _transformName;
}

//------------------------------------------------------------------------------
//!
uint
DFTransformNode::numInputs() const
{
   return 1;
}

//------------------------------------------------------------------------------
//!
DFOutput*
DFTransformNode::output()
{
   return &_output;
}

//------------------------------------------------------------------------------
//!
DFInput*
DFTransformNode::input( uint id )
{
   if( id == 0 ) return &_input;
   return nullptr;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeEditor>
DFTransformNode::edit()
{
   if( _editor.isNull() ) _editor = new DFTransformEditor( this );
   return _editor;
}

//------------------------------------------------------------------------------
//!
bool
DFTransformNode::dumpCustom( TextStream& os, StreamIndent& indent ) const
{
   os << indent << "ref = " << VMFmt( _referential ) << "," << nl;
   return os.ok();
}

//------------------------------------------------------------------------------
//!
RCP<DFGeometry>
DFTransformNode::process()
{
   // Check whether or not we have a valid geometry to start.
   RCP<DFGeometry> geom = _input.getGeometry();
   if( geom.isNull() ) return nullptr;

   return geom->transform( _referential );
}

/*==============================================================================
   CLASS DFCloneEditor
==============================================================================*/

class DFCloneEditor:
   public DFNodeEditor
{
public:

   /*----- methods -----*/

   DFCloneEditor( DFCloneNode* n ): _node(n) {}

   PLASMA_DLL_API virtual RCP<Manipulator>  manipulator() { return nullptr; }
   PLASMA_DLL_API virtual RCP<DFNodeAttrList>  attributes() const;
   PLASMA_DLL_API virtual RCP<DFNodeAttrStates>  attributesStates() const;
   PLASMA_DLL_API virtual void updateAttributes( const DFNodeAttrStates& );

protected:

   /*----- members -----*/

   DFCloneNode*   _node;
};

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrList>
DFCloneEditor::attributes() const
{
   RCP<DFNodeAttrList> atts = new DFNodeAttrList();
   atts->add( DFNodeAttr( "INT", ID_NUM, "#" ) );
   atts->last().range( 0.0f, CGConstf::max() );
   atts->add( DFNodeAttr( "XYZ", ID_OFFSET, "Offset" ) );
   RCP<DFNodeAttrList> rot  = new DFNodeAttrList();
   rot->add( DFNodeAttr( "XYZ", ID_POS, "Origin" ) );
   rot->add( DFNodeAttr( "ORI", ID_ORI, "Rotation" ) );
   atts->add( DFNodeAttr( ID_REF, "", rot.ptr() ) );

   return atts;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrStates>
DFCloneEditor::attributesStates() const
{
   RCP<DFNodeAttrStates> states = new DFNodeAttrStates();
   states->set( ID_NUM, float(_node->numClones()) );
   states->set( ID_OFFSET, _node->offset() );
   states->set( ID_POS, _node->origin() );
   states->set( ID_ORI, _node->rotation() );
   return states;
}

//------------------------------------------------------------------------------
//!
void
DFCloneEditor::updateAttributes( const DFNodeAttrStates& attribs )
{
   for( auto cur = attribs.begin(); cur != attribs.end(); ++cur )
   {
      switch( cur->_id )
      {
         case ID_NUM:    _node->numClones( uint((*cur)._value.getFloat()) ); break;
         case ID_OFFSET: _node->offset( (*cur)._value.getVec3() );           break;
         case ID_POS:    _node->origin( (*cur)._value.getVec3() );           break;
         case ID_ORI:    _node->rotation( (*cur)._value.getQuat() );         break;
         default:;
      }
   }
   _node->graph()->invalidate( _node );
}

/*==============================================================================
   CLASS DFCloneNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFCloneNode::DFCloneNode():
   _input(this), _num( 1 ), _offset( 1.0f, 0.0f, 0.0f ), _rotation( Reff::identity() )
{
   _output.delegate( makeDelegate( this, &DFCloneNode::process ) );
}

//------------------------------------------------------------------------------
//!
const ConstString&
DFCloneNode::name() const
{
   return _cloneName;
}

//------------------------------------------------------------------------------
//!
uint
DFCloneNode::numInputs() const
{
   return 1;
}

//------------------------------------------------------------------------------
//!
DFOutput*
DFCloneNode::output()
{
   return &_output;
}

//------------------------------------------------------------------------------
//!
DFInput*
DFCloneNode::input( uint id )
{
   if( id == 0 ) return &_input;
   return nullptr;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeEditor>
DFCloneNode::edit()
{
   if( _editor.isNull() ) _editor = new DFCloneEditor( this );
   return _editor;
}

//------------------------------------------------------------------------------
//!
bool
DFCloneNode::dumpCustom( TextStream& os, StreamIndent& indent ) const
{
    os << indent << "offset = "   << VMFmt( _offset )   << "," << nl;
    os << indent << "rotation = " << VMFmt( _rotation ) << "," << nl;
    os << indent << "num = "      << _num               << "," << nl;
   return os.ok();
}

//------------------------------------------------------------------------------
//!
RCP<DFGeometry>
DFCloneNode::process()
{
   // Check whether or not we have a valid geometry to start.
   RCP<DFGeometry> geom = _input.getGeometry();
   if( geom.isNull() ) return nullptr;

   return DFGeomOps::clone( *geom, _num, _offset, _rotation );
}

/*==============================================================================
   CLASS DFGeomImportEditor
==============================================================================*/

class DFGeomImportEditor:
   public DFNodeEditor
{
public:

   /*----- methods -----*/

   DFGeomImportEditor( DFGeomImportNode* n ): _node(n) {}

   PLASMA_DLL_API virtual RCP<Manipulator>  manipulator() { return nullptr; }
   PLASMA_DLL_API virtual RCP<DFNodeAttrList>  attributes() const;
   PLASMA_DLL_API virtual RCP<DFNodeAttrStates>  attributesStates() const;
   PLASMA_DLL_API virtual void updateAttributes( const DFNodeAttrStates& );

protected:

   /*----- members -----*/

   DFGeomImportNode* _node;
};

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrList>
DFGeomImportEditor::attributes() const
{
   RCP<DFNodeAttrList> atts = new DFNodeAttrList();
   atts->add( DFNodeAttr( "STRING", ID_NAME, "Name" ) );
   return atts;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrStates>
DFGeomImportEditor::attributesStates() const
{
   RCP<DFNodeAttrStates> states = new DFNodeAttrStates();
   states->set( ID_NAME, _node->geomID().cstr() );
   return states;
}

//------------------------------------------------------------------------------
//!
void
DFGeomImportEditor::updateAttributes( const DFNodeAttrStates& attribs )
{
   for( auto cur = attribs.begin(); cur != attribs.end(); ++cur )
   {
      if( cur->_id == ID_NAME )
      {
         _node->geomID( (*cur)._value.getString().cstr() );
         _node->graph()->invalidate( _node );
      }
   }
}

/*==============================================================================
   CLASS DFGeomImportNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFGeomImportNode::DFGeomImportNode()
{
   _output.delegate( makeDelegate( this, &DFGeomImportNode::process ) );
}

//------------------------------------------------------------------------------
//!
const ConstString&
DFGeomImportNode::name() const
{
   return _geomImportName;
}

//------------------------------------------------------------------------------
//!
bool
DFGeomImportNode::isGraph() const
{
   return true;
}

//------------------------------------------------------------------------------
//!
DFOutput*
DFGeomImportNode::output()
{
   return &_output;
}

//------------------------------------------------------------------------------
//!
void
DFGeomImportNode::geomID( const String& id )
{
   _geomID    = id;
   _geomGraph = nullptr;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeEditor>
DFGeomImportNode::edit()
{
   if( _editor.isNull() )  _editor = new DFGeomImportEditor( this );
   return _editor;
}

//------------------------------------------------------------------------------
//!
bool
DFGeomImportNode::dumpCustom( TextStream& os, StreamIndent& indent ) const
{
   os << indent << "\"" << _geomID << "\"," << nl;
   return os.ok();
}

//------------------------------------------------------------------------------
//!
RCP<DFGeometry>
DFGeomImportNode::process()
{
   // Read geom graph file.
   if( _geomGraph.isNull() )
   {
      RCP< Resource<DFGraph> > res = ResManager::getGraph( _geomID, nullptr );
      // Is it a valid existing resource?
      if( res.isValid() )
      {
         StdErr << "Inefficient resource wait (" << __FILE__ << "@" << __LINE__ << ")" << nl;
         while( !res->isReady() )  Thread::sleep( 0.01f ); // FIXME: more efficient waiting.
         _geomGraph = data( res );
      }
      else
      {
         return nullptr;
      }
   }

   DFNode* outNode = _geomGraph->output();
   if( outNode == nullptr )  return nullptr;

   DFOutput* out = outNode->output();
   if( out == nullptr || out->type() != DFOutput::GEOMETRY )  return nullptr;

   DFGeomOutput* outg = (DFGeomOutput*)out;
   return outg->getGeometry();
}

NAMESPACE_END
