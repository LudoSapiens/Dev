/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/DataFlow/DFAnimNodes.h>
#include <Plasma/DataFlow/DFNodeAttr.h>
#include <Plasma/DataFlow/DFGraph.h>
#include <Plasma/Resource/ResManager.h>
#include <Plasma/Animation/Puppeteer.h>
#include <Plasma/Manipulator/Manipulator.h>

#include <Fusion/VM/VMFmt.h>

#include <Base/MT/Thread.h>

#if _MSC_VER
// 'this' used in member initializer list.
#pragma warning( disable: 4355 )
#endif

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

ConstString _importName;
ConstString _cycleName;
ConstString _resampleName;
ConstString _trimName;

enum
{
   ID_NAME,
   ID_RATE,
   ID_FRACTION,
   ID_RANGE
};

//------------------------------------------------------------------------------
//!
RCP<DFNode> importVM( VMState* vm, int idx )
{
   RCP<DFImportAnimNode> node = new DFImportAnimNode();
   if( VM::geti( vm, idx, 1 ) )
   {
      node->animationID( VM::toString( vm, -1 ) );
      VM::pop( vm );
   }
   else
   {
      StdErr << "ERROR: Missing animation ID in importAnim()." << nl;
   }
   return node;
}

//------------------------------------------------------------------------------
//!
RCP<DFNode> cycleVM( VMState* vm, int idx )
{
   RCP<DFCycleAnimNode> node = new DFCycleAnimNode();
   float f;
   if( VM::get( vm, idx, "fraction", f ) ) node->fraction( f );

   return node;
}

//------------------------------------------------------------------------------
//!
RCP<DFNode> resampleVM( VMState* vm, int idx )
{
   RCP<DFResampleAnimNode> node = new DFResampleAnimNode();
   float r;
   if( VM::get( vm, idx, "rate", r ) ) node->rate( r );

   return node;
}

//------------------------------------------------------------------------------
//!
RCP<DFNode> trimVM( VMState* vm, int idx )
{
   RCP<DFTrimAnimNode> node = new DFTrimAnimNode();
   Vec2i r;
   if( VM::get( vm, idx, "range", r ) ) node->range( r );

   return node;
}

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   STATIC INITIALIZATION
==============================================================================*/

//------------------------------------------------------------------------------
//!
void initializeAnimNodes()
{
   _importName   = "importAnim";
   _cycleName    = "cycleAnim";
   _resampleName = "resampleAnim";
   _trimName     = "trimAnim";

   DFNodeSpec::registerNode(
      DFSocket::ANIMATION,
      _importName, importVM,
      "Import", "Import an animation.",
      nullptr
   );
   DFNodeSpec::registerNode(
      DFSocket::ANIMATION,
      _cycleName, cycleVM,
      "Cycle", "Make an animation cyclic.",
      nullptr
   );
   DFNodeSpec::registerNode(
      DFSocket::ANIMATION,
      _resampleName, resampleVM,
      "Resample", "Resample an animation.",
      nullptr
   );
   DFNodeSpec::registerNode(
      DFSocket::ANIMATION,
      _trimName, trimVM,
      "Trim", "Trim an animation.",
      nullptr
   );
}

//------------------------------------------------------------------------------
//!
void terminateAnimNodes()
{
   _importName   = ConstString();
   _cycleName    = ConstString();
   _resampleName = ConstString();
   _trimName     = ConstString();
}

/*==============================================================================
   CLASS DFAnimOutput
==============================================================================*/

//------------------------------------------------------------------------------
//!
RCP<SkeletalAnimation>
DFAnimOutput::getAnimation()
{
   return _delegate();
}

//------------------------------------------------------------------------------
//!
DFSocket::Type
DFAnimOutput::type() const
{
   return ANIMATION;
}

/*==============================================================================
   CLASS DFAnimInput
==============================================================================*/

//------------------------------------------------------------------------------
//!
RCP<SkeletalAnimation>
DFAnimInput::getAnimation()
{
   if( !_output ) return nullptr;
   return _output->getAnimation();
}

//------------------------------------------------------------------------------
//!
DFSocket::Type
DFAnimInput::type() const
{
   return ANIMATION;
}

//------------------------------------------------------------------------------
//!
bool
DFAnimInput::isConnected() const
{
   return _output != nullptr;
}

//------------------------------------------------------------------------------
//!
void
DFAnimInput::connect( DFOutput* output )
{
   CHECK( type() == output->type() );
   if( _output ) disconnectFrom( _output );
   _output = (DFAnimOutput*)output;
}

//------------------------------------------------------------------------------
//!
void
DFAnimInput::disconnect( DFOutput* output )
{
   if( _output == output ) _output = nullptr;
}

//------------------------------------------------------------------------------
//!
void
DFAnimInput::disconnect()
{
   if( !_output ) return;
   disconnectFrom( _output );
   _output = nullptr;
}

/*==============================================================================
   CLASS DFAnimMultiInput
==============================================================================*/

//------------------------------------------------------------------------------
//!
RCP<SkeletalAnimation>
DFAnimMultiInput::getAnimation( uint i )
{
   if( i >= _outputs.size() ) return nullptr;
   return _outputs[i]->getAnimation();
}

//------------------------------------------------------------------------------
//!
DFSocket::Type
DFAnimMultiInput::type() const
{
   return ANIMATION;
}

//------------------------------------------------------------------------------
//!
bool
DFAnimMultiInput::isConnected() const
{
   return !_outputs.empty();
}

//------------------------------------------------------------------------------
//!
void
DFAnimMultiInput::connect( DFOutput* output )
{
   CHECK( type() == output->type() );
   _outputs.pushBack( (DFAnimOutput*)output );
}

//------------------------------------------------------------------------------
//!
void
DFAnimMultiInput::disconnect( DFOutput* output )
{
   _outputs.removeSwap( (DFAnimOutput*)output );
}

//------------------------------------------------------------------------------
//!
void
DFAnimMultiInput::disconnect()
{
   for( auto it = _outputs.begin(); it != _outputs.end(); ++it )
   {
      disconnectFrom( *it );
   }
   _outputs.clear();
}

/*==============================================================================
   CLASS DFImportAnimEditor
==============================================================================*/

class DFImportAnimEditor:
   public DFNodeEditor
{
public:

   /*----- methods -----*/

   DFImportAnimEditor( DFImportAnimNode* n ): _node(n) {}

   PLASMA_DLL_API virtual RCP<Manipulator>  manipulator() { return nullptr; }
   PLASMA_DLL_API virtual RCP<DFNodeAttrList>  attributes() const;
   PLASMA_DLL_API virtual RCP<DFNodeAttrStates>  attributesStates() const;
   PLASMA_DLL_API virtual void updateAttributes( const DFNodeAttrStates& );

protected:

   /*----- members -----*/

   DFImportAnimNode* _node;
};

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrList>
DFImportAnimEditor::attributes() const
{
   RCP<DFNodeAttrList> atts = new DFNodeAttrList();
   atts->add( DFNodeAttr( "STRING", ID_NAME, "Name" ) );
   return atts;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrStates>
DFImportAnimEditor::attributesStates() const
{
   RCP<DFNodeAttrStates> states = new DFNodeAttrStates();
   states->set( ID_NAME, _node->animationID().cstr() );
   return states;
}

//------------------------------------------------------------------------------
//!
void
DFImportAnimEditor::updateAttributes( const DFNodeAttrStates& attribs )
{
   for( auto cur = attribs.begin(); cur != attribs.end(); ++cur )
   {
      if( cur->_id == ID_NAME )
      {
         _node->animationID( (*cur)._value.getString().cstr() );
         // Invalidate graph.
         _node->graph()->invalidate( _node );
      }
   }
}

/*==============================================================================
   CLASS DFImportAnimNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFImportAnimNode::DFImportAnimNode()
{
   _output.delegate( makeDelegate( this, &DFImportAnimNode::process ) );
}

//------------------------------------------------------------------------------
//!
const ConstString&
DFImportAnimNode::name() const
{
   return _importName;
}

//------------------------------------------------------------------------------
//!
uint
DFImportAnimNode::numInputs() const
{
   return 0;
}

//------------------------------------------------------------------------------
//!
DFOutput*
DFImportAnimNode::output()
{
   return &_output;
}

//------------------------------------------------------------------------------
//!
DFInput*
DFImportAnimNode::input( uint )
{
   return nullptr;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeEditor>
DFImportAnimNode::edit()
{
   if( _editor.isNull() )  _editor = new DFImportAnimEditor( this );
   return _editor;
}

//------------------------------------------------------------------------------
//!
RCP<SkeletalAnimation>
DFImportAnimNode::process()
{
   if( _anim.isNull() )
   {
      RCP< Resource<SkeletalAnimation> > animRes = ResManager::getAnimation( _animID, nullptr );
      // Is it a valid existing resource?
      if( animRes.isValid() )
      {
         StdErr << "Inefficient resource wait" << nl;
         while( !animRes->isReady() )  Thread::sleep( 0.01f ); // FIXME: more efficient waiting.
         //_anim = waitForData( animRes.ptr() );
         _anim = data( animRes );
      }
   }
   return _anim;
}

//------------------------------------------------------------------------------
//!
void
DFImportAnimNode::animationID( const String& id )
{
   _animID = id;
   _anim   = nullptr;
}

//------------------------------------------------------------------------------
//!
bool
DFImportAnimNode::dumpCustom( TextStream& os, StreamIndent& indent ) const
{
   os << indent << "\"" << _animID << "\"," << nl;
   return os.ok();
}

/*==============================================================================
   CLASS DFCycleAnimEditor
==============================================================================*/

class DFCycleAnimEditor:
   public DFNodeEditor
{
public:

   /*----- methods -----*/

   DFCycleAnimEditor( DFCycleAnimNode* n ): _node(n) {}

   PLASMA_DLL_API virtual RCP<Manipulator>  manipulator() { return nullptr; }
   PLASMA_DLL_API virtual RCP<DFNodeAttrList>  attributes() const;
   PLASMA_DLL_API virtual RCP<DFNodeAttrStates>  attributesStates() const;
   PLASMA_DLL_API virtual void updateAttributes( const DFNodeAttrStates& );

protected:

   /*----- members -----*/

   DFCycleAnimNode* _node;
};

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrList>
DFCycleAnimEditor::attributes() const
{
   RCP<DFNodeAttrList> atts = new DFNodeAttrList();
   RCP<Table> extras        = new Table();
   extras->set( "range", Vec2f( 0.0f, 1.0f ) );
   atts->add( DFNodeAttr( "FLOAT", ID_FRACTION, "Fraction" ).extras( extras.ptr() ) );
   return atts;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrStates>
DFCycleAnimEditor::attributesStates() const
{
   RCP<DFNodeAttrStates> states = new DFNodeAttrStates();
   states->set( ID_FRACTION, _node->fraction() );
   return states;
}

//------------------------------------------------------------------------------
//!
void
DFCycleAnimEditor::updateAttributes( const DFNodeAttrStates& attribs )
{
   for( auto cur = attribs.begin(); cur != attribs.end(); ++cur )
   {
      if( cur->_id == ID_FRACTION )
      {
         _node->fraction( (*cur)._value.getFloat() );
         // Invalidate graph.
         _node->graph()->invalidate( _node );
      }
   }
}

/*==============================================================================
   CLASS DFCycleAnimNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFCycleAnimNode::DFCycleAnimNode(): _input(this), _fraction(0.25f)
{
   _output.delegate( makeDelegate( this, &DFCycleAnimNode::process ) );
}

//------------------------------------------------------------------------------
//!
const ConstString&
DFCycleAnimNode::name() const
{
   return _cycleName;
}

//------------------------------------------------------------------------------
//!
uint
DFCycleAnimNode::numInputs() const
{
   return 1;
}

//------------------------------------------------------------------------------
//!
DFOutput*
DFCycleAnimNode::output()
{
   return &_output;
}

//------------------------------------------------------------------------------
//!
DFInput*
DFCycleAnimNode::input( uint id )
{
   if( id == 0 ) return &_input;
   return nullptr;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeEditor>
DFCycleAnimNode::edit()
{
   if( _editor.isNull() )  _editor = new DFCycleAnimEditor( this );
   return _editor;
}

//------------------------------------------------------------------------------
//!
RCP<SkeletalAnimation>
DFCycleAnimNode::process()
{
   RCP<SkeletalAnimation> anim = _input.getAnimation();
   if( anim.isNull() ) return anim;

   return Puppeteer::cycle( anim.ptr(), _fraction );
}

//------------------------------------------------------------------------------
//!
bool
DFCycleAnimNode::dumpCustom( TextStream& os, StreamIndent& indent ) const
{
   os << indent << "fraction=" << _fraction << "," << nl;
   return os.ok();
}

/*==============================================================================
   CLASS DFResampleAnimEditor
==============================================================================*/

class DFResampleAnimEditor:
   public DFNodeEditor
{
public:

   /*----- methods -----*/

   DFResampleAnimEditor( DFResampleAnimNode* n ): _node(n) {}

   PLASMA_DLL_API virtual RCP<Manipulator>  manipulator() { return nullptr; }
   PLASMA_DLL_API virtual RCP<DFNodeAttrList>  attributes() const;
   PLASMA_DLL_API virtual RCP<DFNodeAttrStates>  attributesStates() const;
   PLASMA_DLL_API virtual void updateAttributes( const DFNodeAttrStates& );

protected:

   /*----- members -----*/

   DFResampleAnimNode* _node;
};

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrList>
DFResampleAnimEditor::attributes() const
{
   RCP<DFNodeAttrList> atts = new DFNodeAttrList();
   atts->add( DFNodeAttr( "FLOAT", ID_RATE, "Rate" ) );
   return atts;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrStates>
DFResampleAnimEditor::attributesStates() const
{
   RCP<DFNodeAttrStates> states = new DFNodeAttrStates();
   states->set( ID_RATE, _node->rate() );
   return states;
}

//------------------------------------------------------------------------------
//!
void
DFResampleAnimEditor::updateAttributes( const DFNodeAttrStates& attribs )
{
   for( auto cur = attribs.begin(); cur != attribs.end(); ++cur )
   {
      if( cur->_id == ID_RATE )
      {
         _node->rate( (*cur)._value.getFloat() );
         // Invalidate graph.
         _node->graph()->invalidate( _node );
      }
   }
}

/*==============================================================================
   CLASS DFResampleAnimNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFResampleAnimNode::DFResampleAnimNode(): _input(this), _rate(10.0f)
{
   _output.delegate( makeDelegate( this, &DFResampleAnimNode::process ) );
}

//------------------------------------------------------------------------------
//!
const ConstString&
DFResampleAnimNode::name() const
{
   return _resampleName;
}

//------------------------------------------------------------------------------
//!
uint
DFResampleAnimNode::numInputs() const
{
   return 1;
}

//------------------------------------------------------------------------------
//!
DFOutput*
DFResampleAnimNode::output()
{
   return &_output;
}

//------------------------------------------------------------------------------
//!
DFInput*
DFResampleAnimNode::input( uint id )
{
   if( id == 0 ) return &_input;
   return nullptr;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeEditor>
DFResampleAnimNode::edit()
{
   if( _editor.isNull() )  _editor = new DFResampleAnimEditor( this );
   return _editor;
}

//------------------------------------------------------------------------------
//!
RCP<SkeletalAnimation>
DFResampleAnimNode::process()
{
   RCP<SkeletalAnimation> anim = _input.getAnimation();
   if( anim.isNull() ) return anim;

   return Puppeteer::resample( anim.ptr(), _rate );
}

//------------------------------------------------------------------------------
//!
bool
DFResampleAnimNode::dumpCustom( TextStream& os, StreamIndent& indent ) const
{
   os << indent << "rate=" << _rate << "," << nl;
   return os.ok();
}

/*==============================================================================
   CLASS DFTrimAnimEditor
==============================================================================*/

class DFTrimAnimEditor:
   public DFNodeEditor
{
public:

   /*----- methods -----*/

   DFTrimAnimEditor( DFTrimAnimNode* n ): _node(n) {}

   PLASMA_DLL_API virtual RCP<Manipulator>  manipulator() { return nullptr; }
   PLASMA_DLL_API virtual RCP<DFNodeAttrList>  attributes() const;
   PLASMA_DLL_API virtual RCP<DFNodeAttrStates>  attributesStates() const;
   PLASMA_DLL_API virtual void updateAttributes( const DFNodeAttrStates& );

protected:

   /*----- members -----*/

   DFTrimAnimNode* _node;
};

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrList>
DFTrimAnimEditor::attributes() const
{
   RCP<DFNodeAttrList> atts = new DFNodeAttrList();
   RCP<Table> extras        = new Table();
   extras->set( "range", Vec2f( 0.0f, 10000.0f ) );
   atts->add( DFNodeAttr( "INT2", ID_RANGE, "Range" ).extras( extras.ptr() ) );
   return atts;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrStates>
DFTrimAnimEditor::attributesStates() const
{
   RCP<DFNodeAttrStates> states = new DFNodeAttrStates();
   states->set( ID_RANGE, _node->range() );
   return states;
}

//------------------------------------------------------------------------------
//!
void DFTrimAnimEditor::updateAttributes( const DFNodeAttrStates& attribs )
{
   for( auto cur = attribs.begin(); cur != attribs.end(); ++cur )
   {
      if( cur->_id == ID_RANGE )
      {
         _node->range( (*cur)._value.getVec2() );
         // Invalidate graph.
         _node->graph()->invalidate( _node );
      }
   }
}


/*==============================================================================
   CLASS DFTrimAnimNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFTrimAnimNode::DFTrimAnimNode(): _input(this), _range(0,1000)
{
   _output.delegate( makeDelegate( this, &DFTrimAnimNode::process ) );
}

//------------------------------------------------------------------------------
//!
const ConstString&
DFTrimAnimNode::name() const
{
   return _trimName;
}

//------------------------------------------------------------------------------
//!
uint
DFTrimAnimNode::numInputs() const
{
   return 1;
}

//------------------------------------------------------------------------------
//!
DFOutput*
DFTrimAnimNode::output()
{
   return &_output;
}

//------------------------------------------------------------------------------
//!
DFInput*
DFTrimAnimNode::input( uint id )
{
   if( id == 0 ) return &_input;
   return nullptr;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeEditor>
DFTrimAnimNode::edit()
{
   if( _editor.isNull() )  _editor = new DFTrimAnimEditor( this );
   return _editor;
}

//------------------------------------------------------------------------------
//!
RCP<SkeletalAnimation>
DFTrimAnimNode::process()
{
   RCP<SkeletalAnimation> anim = _input.getAnimation();
   if( anim.isNull() ) return anim;

   return Puppeteer::cut( anim.ptr(), _range.x, _range.y );
}

//------------------------------------------------------------------------------
//!
bool
DFTrimAnimNode::dumpCustom( TextStream& os, StreamIndent& indent ) const
{
   os << indent << "range=" << VMFmt(_range) << "," << nl;
   return os.ok();
}


NAMESPACE_END
