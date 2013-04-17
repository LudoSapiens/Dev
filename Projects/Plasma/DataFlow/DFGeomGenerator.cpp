/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/DataFlow/DFGeomGenerator.h>
#include <Plasma/DataFlow/DFGraph.h>
#include <Plasma/DataFlow/DFNodeAttr.h>
#include <Plasma/DataFlow/DFGeomOps.h>

#include <Plasma/Plasma.h>
#include <Plasma/Geometry/MeshGeometry.h>
#include <Plasma/Manipulator/RefManipulator.h>
#include <Plasma/Resource/ResManager.h>

#include <Fusion/VM/VMFmt.h>

#include <CGMath/Variant.h>

#if _MSC_VER
// 'this' used in member initializer list.
#pragma warning( disable: 4355 )
#endif

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

ConstString _blocksName;
ConstString _boxName;
ConstString _sphereName;
ConstString _extrudePolyName;

enum
{
   ID_HALF_SIZE,
   ID_HEIGHT,
   ID_RADIUS,
   ID_REF,
   ID_POS,
   ID_ORI,
};

//------------------------------------------------------------------------------
//!
RCP<DFNode> extrudePolyVM( VMState* vm, int idx )
{
   RCP<DFExtrudePolyNode> node = new DFExtrudePolyNode();
   float h;
   if( VM::get( vm, idx, "height", h ) ) node->height( h );
   return node;
}

//-----------------------------------------------------------------------------
//!
RCP<DFNode> blocksVM( VMState* vm, int idx )
{
   RCP<DFBlocksNode> node = new DFBlocksNode();

   DFBlocks* blocks = node->blocks();

   // Reads blocks.
   for( int i = 1; VM::geti( vm, idx, i ); ++i )
   {
      DFBlock* block    = blocks->createBlock();
      ushort id         = 0;
      ushort groupID    = 0;
      ushort creases    = 0;
      uint subdivisions = 0;
      uint attractions  = 0xAAA;
      VM::get( vm, -1, "id", id );
      if( id > 0 )  --id;
      VM::get( vm, -1, "g", groupID );
      VM::get( vm, -1, "c", creases );
      VM::get( vm, -1, "s", subdivisions );
      VM::get( vm, -1, "a", attractions );
      blocks->set( block, id, groupID, creases, subdivisions, attractions );

      // Vertices.
      Vec3f pos[8];
      switch( VM::getTableSize( vm, -1 ) )
      {
         case 0:
         {
            // No parameters: make a unit block.
            pos[0] = Vec3f( -1.0f, -1.0f, -1.0f );
            pos[1] = Vec3f(  1.0f, -1.0f, -1.0f );
            pos[2] = Vec3f( -1.0f,  1.0f, -1.0f );
            pos[3] = Vec3f(  1.0f,  1.0f, -1.0f );
            pos[4] = Vec3f( -1.0f, -1.0f,  1.0f );
            pos[5] = Vec3f(  1.0f, -1.0f,  1.0f );
            pos[6] = Vec3f( -1.0f,  1.0f,  1.0f );
            pos[7] = Vec3f(  1.0f,  1.0f,  1.0f );
         }  break;
         case 1:
         {
            VM::geti( vm, -1, 1 );
            Vec3f s;
            if( VM::isNumber( vm, -1 ) )
            {
               // Single scalar: radius (i.e. extent in all dimensions).
               s = Vec3f( CGM::abs( VM::toFloat( vm, -1 ) ) );
            }
            else
            {
               // Single vector: specifies extents.
               s = CGM::abs( VM::toVec3f( vm, -1 ) );
            }
            pos[0] = Vec3f( -s.x, -s.y, -s.z );
            pos[1] = Vec3f(  s.x, -s.y, -s.z );
            pos[2] = Vec3f( -s.x,  s.y, -s.z );
            pos[3] = Vec3f(  s.x,  s.y, -s.z );
            pos[4] = Vec3f( -s.x, -s.y,  s.z );
            pos[5] = Vec3f(  s.x, -s.y,  s.z );
            pos[6] = Vec3f( -s.x,  s.y,  s.z );
            pos[7] = Vec3f(  s.x,  s.y,  s.z );
            VM::pop( vm, 1 );
         }  break;
         case 2:
         {
            // Two vectors: min and max values in each axis.
            VM::geti( vm, -1, 1 );
            VM::geti( vm, -2, 2 );
            Vec3f a = VM::toVec3f( vm, -2 );
            Vec3f b = VM::toVec3f( vm, -1 );
            Vec3f min = CGM::min( a, b );
            Vec3f max = CGM::max( a, b );
            pos[0] = Vec3f( min.x, min.y, min.z );
            pos[1] = Vec3f( max.x, min.y, min.z );
            pos[2] = Vec3f( min.x, max.y, min.z );
            pos[3] = Vec3f( max.x, max.y, min.z );
            pos[4] = Vec3f( min.x, min.y, max.z );
            pos[5] = Vec3f( max.x, min.y, max.z );
            pos[6] = Vec3f( min.x, max.y, max.z );
            pos[7] = Vec3f( max.x, max.y, max.z );
            VM::pop( vm, 2 );
         }  break;
         case 5:
         {
            // Five vectors: a base quad, and an extrusion vector.
            for( int v = 1; v <= 4; ++v )
            {
               VM::geti( vm, -1, v );
               pos[v-1] = VM::toVec3f( vm, -1 );
               VM::pop( vm, 1 ); // Pop vertex.
            }
            VM::geti( vm, -1, 5 );
            Vec3f dir = VM::toVec3f( vm, -1 );
            pos[4] = pos[0] + dir;
            pos[5] = pos[1] + dir;
            pos[6] = pos[2] + dir;
            pos[7] = pos[3] + dir;
         }  break;
         case 8:
         {
            for( int v = 1; v <= 8; ++v )
            {
               VM::geti( vm, -1, v );
               pos[v-1] = VM::toVec3f( vm, -1 );
               VM::pop( vm, 1 );
            }
         }  break;
         default:
         {
            // Invalid format.
            StdErr << "Invalid block format; using degenerate block." << nl;
            pos[0] = Vec3f(0.0f);
            pos[1] = Vec3f(0.0f);
            pos[2] = Vec3f(0.0f);
            pos[3] = Vec3f(0.0f);
            pos[4] = Vec3f(0.0f);
            pos[5] = Vec3f(0.0f);
            pos[6] = Vec3f(0.0f);
            pos[7] = Vec3f(0.0f);
         }  break;
      }

      blocks->set( block, pos );

      VM::pop(vm);
   }

   return node;
}

//-----------------------------------------------------------------------------
//!
RCP<DFNode> boxVM( VMState* vm, int idx )
{
   RCP<DFBoxNode> node = new DFBoxNode();
   node->init( vm, idx );
   return node;
}

//-----------------------------------------------------------------------------
//!
RCP<DFNode> sphereVM( VMState* vm, int idx )
{
   RCP<DFBallNode> node = new DFBallNode();
   node->init( vm, idx );
   return node;
}

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   STATIC INITIALIZATION
==============================================================================*/

//-----------------------------------------------------------------------------
//!
void initializeGeomGenerator()
{
   _blocksName       = "blocks";
   _boxName          = "box";
   _sphereName       = "sphere";
   _extrudePolyName  = "extrudePoly";

   DFNodeSpec::registerNode(
      DFSocket::GEOMETRY,
      _blocksName, blocksVM,
      "Blocks", "A block, specified by 8 corners.",
      "image/ui/dfnode/blocks"
   );
   DFNodeSpec::registerNode(
      DFSocket::GEOMETRY,
      _boxName, boxVM,
      "Box", "A simple box.",
      nullptr //"image/ui/dfnode/box"
   );
   DFNodeSpec::registerNode(
      DFSocket::GEOMETRY,
      _sphereName, sphereVM,
      "Sphere", "A simple sphere.",
      nullptr //"image/ui/dfnode/sphere"
   );
   DFNodeSpec::registerNode(
      DFSocket::GEOMETRY,
      _extrudePolyName, extrudePolyVM,
      "Extrude", "Extrude a polygon into a geometry.",
      nullptr
   );
}

//-----------------------------------------------------------------------------
//!
void terminateGeomGenerator()
{
   _blocksName       = ConstString();
   _boxName          = ConstString();
   _sphereName       = ConstString();
   _extrudePolyName  = ConstString();
}

/*==============================================================================
   CLASS DFBlocksNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFBlocksNode::DFBlocksNode()
{
   _output.delegate( makeDelegate( this, &DFBlocksNode::process ) );
   _blocks = new DFBlocks();
}

//------------------------------------------------------------------------------
//!
const ConstString&
DFBlocksNode::name() const
{
   return _blocksName;
}

//------------------------------------------------------------------------------
//!
uint
DFBlocksNode::numInputs() const
{
   return 0;
}

//------------------------------------------------------------------------------
//!
DFOutput*
DFBlocksNode::output()
{
   return &_output;
}

//------------------------------------------------------------------------------
//!
DFInput*
DFBlocksNode::input( uint )
{
   return nullptr;
}

//------------------------------------------------------------------------------
//!
RCP<DFGeometry>
DFBlocksNode::process()
{
   return _blocks->geometry();
}

//-----------------------------------------------------------------------------
//!
bool
DFBlocksNode::dumpCustom( TextStream& os, StreamIndent& indent ) const
{
   uint n = _blocks->numBlocks();
   for( uint i = 0; i < n; ++i )
   {
      const DFBlock* block = _blocks->block(i);
      os << indent << "{"
                   <<  " " << VMFmt(block->position(0))
                   << ", " << VMFmt(block->position(1))
                   << ", " << VMFmt(block->position(2))
                   << ", " << VMFmt(block->position(3))
                   << ", " << VMFmt(block->position(4))
                   << ", " << VMFmt(block->position(5))
                   << ", " << VMFmt(block->position(6))
                   << ", " << VMFmt(block->position(7))
                   << ", " << "id=" << block->id()
                   << ", " << "g=" << block->groupID()
                   << ", " << "c=" << String().format("0x%03x", block->creases())
                   << ", " << "s=" << String().format("0x%06x", block->subdivisions())
                   << " },"
                   << nl;
   }
   return os.ok();
}


/*==============================================================================
   CLASS DFBoxNodeEditor
==============================================================================*/

class DFBoxNodeEditor:
   public DFNodeEditor
{
public:

   /*----- methods -----*/

   DFBoxNodeEditor( DFBoxNode* n ): _node(n) {}

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

   DFBoxNode*         _node;
   RCP<RefRenderable> _renderable;
};

//------------------------------------------------------------------------------
//!
RCP<Manipulator>
DFBoxNodeEditor::manipulator()
{
   if( _renderable.isNull() )
   {
      _renderable = new RefRenderable();
      _renderable->addOnModify( makeDelegate( this, &DFBoxNodeEditor::referentialCb ) );
      _renderable->update( _node->referential() );
   }
   return RCP<Manipulator>( new RefManipulator( _renderable.ptr() ) );
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrList>
DFBoxNodeEditor::attributes() const
{
   RCP<DFNodeAttrList> atts = new DFNodeAttrList();
   RCP<DFNodeAttrList> ref  = new DFNodeAttrList();
   ref->add( DFNodeAttr( "XYZ", ID_POS, "Position" ) );
   ref->add( DFNodeAttr( "ORI", ID_ORI, "Orientation" ) );
   atts->add( DFNodeAttr( ID_REF, "", ref.ptr() ) );
   atts->add( DFNodeAttr( "FLOAT3", ID_HALF_SIZE, "Half size" ) );
   return atts;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrStates>
DFBoxNodeEditor::attributesStates() const
{
   RCP<DFNodeAttrStates> states = new DFNodeAttrStates();
   states->set( ID_POS, _node->referential().position() );
   states->set( ID_ORI, _node->referential().orientation() );
   states->set( ID_HALF_SIZE, _node->halfSize() );
   return states;
}

//------------------------------------------------------------------------------
//!
void
DFBoxNodeEditor::updateAttributes( const DFNodeAttrStates& attribs )
{
   for( auto cur = attribs.begin(); cur != attribs.end(); ++cur )
   {
      switch( cur->_id )
      {
         case ID_HALF_SIZE:
            _node->halfSize( (*cur)._value.getVec3() );
            _node->graph()->invalidate( _node );
            break;
         case ID_POS: move( (*cur)._value.getVec3() );   break;
         case ID_ORI: rotate( (*cur)._value.getQuat() ); break;
         default:;
      }
   }
}

//------------------------------------------------------------------------------
//!
void
DFBoxNodeEditor::move( const Vec3f& p )
{
   _node->position( p );
   _node->graph()->invalidate( _node );
   if( _renderable.isValid() )
      _renderable->update( _node->referential() );
}

//------------------------------------------------------------------------------
//!
void
DFBoxNodeEditor::rotate( const Quatf& ori )
{
   _node->orientation( ori );
   _node->graph()->invalidate( _node );
   if( _renderable.isValid() )
      _renderable->update( _node->referential() );
}

//------------------------------------------------------------------------------
//!
void
DFBoxNodeEditor::updateUI()
{
   _node->graph()->msg().modify( _node, attributesStates().ptr() );
}

//------------------------------------------------------------------------------
//!
void
DFBoxNodeEditor::referentialCb()
{
   _node->referential( _renderable->referential() );
   _node->graph()->invalidate( _node );
   updateUI();
}

/*==============================================================================
   CLASS DFBoxNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFBoxNode::DFBoxNode():
   _halfSize( 0.5f ), _referential( Reff::identity() )
{
   _output.delegate( makeDelegate( this, &DFBoxNode::process ) );
}

//------------------------------------------------------------------------------
//!
const ConstString&
DFBoxNode::name() const
{
   return _boxName;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeEditor>
DFBoxNode::edit()
{
   if( _editor.isNull() )  _editor = new DFBoxNodeEditor( this );
   return _editor;
}

//------------------------------------------------------------------------------
//!
DFOutput*
DFBoxNode::output()
{
   return &_output;
}

//------------------------------------------------------------------------------
//!
RCP<DFGeometry>
DFBoxNode::process()
{
   RCP<DFGeometry> geom = new DFGeometry();

   Mat4f m = _referential.toMatrix();

   // Set 8 corners.
   geom->addControlPoint( m * Vec3f( -_halfSize.x,-_halfSize.y,-_halfSize.z ) );
   geom->addControlPoint( m * Vec3f(  _halfSize.x,-_halfSize.y,-_halfSize.z ) );
   geom->addControlPoint( m * Vec3f( -_halfSize.x, _halfSize.y,-_halfSize.z ) );
   geom->addControlPoint( m * Vec3f(  _halfSize.x, _halfSize.y,-_halfSize.z ) );
   geom->addControlPoint( m * Vec3f( -_halfSize.x,-_halfSize.y, _halfSize.z ) );
   geom->addControlPoint( m * Vec3f(  _halfSize.x,-_halfSize.y, _halfSize.z ) );
   geom->addControlPoint( m * Vec3f( -_halfSize.x, _halfSize.y, _halfSize.z ) );
   geom->addControlPoint( m * Vec3f(  _halfSize.x, _halfSize.y, _halfSize.z ) );

   // Set 6 faces.
   geom->addPatch( 0, 4, 6, 2, 0xF );
   geom->addPatch( 1, 3, 7, 5, 0xF );
   geom->addPatch( 0, 1, 5, 4, 0xF );
   geom->addPatch( 2, 6, 7, 3, 0xF );
   geom->addPatch( 0, 2, 3, 1, 0xF );
   geom->addPatch( 4, 5, 7, 6, 0xF );

   // Automatically compute neighbors.
   geom->computeNeighbors();

   // Create geometry.
   geom->subdivide( Plasma::geometricError() );

   return geom;
}

//-----------------------------------------------------------------------------
//!
bool
DFBoxNode::dumpCustom( TextStream& os, StreamIndent& indent ) const
{
   os << indent << "hsize = "  << VMFmt( halfSize() )   << "," << nl;
   os << indent << "ref = "    << VMFmt( _referential ) << "," << nl;
   return os.ok();
}

//------------------------------------------------------------------------------
//!
void
DFBoxNode::init( VMState* vm, int idx )
{
   VM::get( vm, idx, "hsize", _halfSize );
   VM::get( vm, idx, "ref", _referential );
}

/*==============================================================================
   CLASS DFBallNodeEditor
==============================================================================*/

class DFBallNodeEditor:
   public DFNodeEditor
{
public:

   /*----- methods -----*/

   DFBallNodeEditor( DFBallNode* n ): _node(n) {}

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

   DFBallNode*        _node;
   RCP<RefRenderable> _renderable;
};

//------------------------------------------------------------------------------
//!
RCP<Manipulator>
DFBallNodeEditor::manipulator()
{
   if( _renderable.isNull() )
   {
      _renderable = new RefRenderable();
      _renderable->addOnModify( makeDelegate( this, &DFBallNodeEditor::referentialCb ) );
      _renderable->update( _node->referential() );
   }
   return RCP<Manipulator>( new RefManipulator( _renderable.ptr() ) );
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrList>
DFBallNodeEditor::attributes() const
{
   RCP<DFNodeAttrList> atts = new DFNodeAttrList();
   RCP<DFNodeAttrList> ref  = new DFNodeAttrList();
   ref->add( DFNodeAttr( "XYZ", ID_POS, "Position" ) );
   ref->add( DFNodeAttr( "ORI", ID_ORI, "Orientation" ) );
   atts->add( DFNodeAttr( ID_REF, "", ref.ptr() ) );
   atts->add( DFNodeAttr( "FLOAT", ID_RADIUS, "Radius" ) );
   return atts;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrStates>
DFBallNodeEditor::attributesStates() const
{
   RCP<DFNodeAttrStates> states = new DFNodeAttrStates();
   states->set( ID_POS, _node->referential().position() );
   states->set( ID_ORI, _node->referential().orientation() );
   states->set( ID_RADIUS, _node->radius() );
   return states;
}

//------------------------------------------------------------------------------
//!
void
DFBallNodeEditor::updateAttributes( const DFNodeAttrStates& attribs )
{
   for( auto cur = attribs.begin(); cur != attribs.end(); ++cur )
   {
      switch( cur->_id )
      {
         case ID_RADIUS:
            _node->radius( (*cur)._value.getFloat() );
            _node->graph()->invalidate( _node );
            break;
         case ID_POS: move( (*cur)._value.getVec3() );   break;
         case ID_ORI: rotate( (*cur)._value.getQuat() ); break;
         default:;
      }
   }
}

//------------------------------------------------------------------------------
//!
void
DFBallNodeEditor::move( const Vec3f& p )
{
   _node->position( p );
   _node->graph()->invalidate( _node );
   if( _renderable.isValid() )
      _renderable->update( _node->referential() );
}

//------------------------------------------------------------------------------
//!
void
DFBallNodeEditor::rotate( const Quatf& ori )
{
   _node->orientation( ori );
   _node->graph()->invalidate( _node );
   if( _renderable.isValid() )
      _renderable->update( _node->referential() );
}

//------------------------------------------------------------------------------
//!
void
DFBallNodeEditor::updateUI()
{
   _node->graph()->msg().modify( _node, attributesStates().ptr() );
}

//------------------------------------------------------------------------------
//!
void
DFBallNodeEditor::referentialCb()
{
   _node->referential( _renderable->referential() );
   _node->graph()->invalidate( _node );
   updateUI();
}

/*==============================================================================
   CLASS DFBallNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFBallNode::DFBallNode():
   _radius( 0.5f ), _referential( Reff::identity() )
{
   _output.delegate( makeDelegate( this, &DFBallNode::process ) );
}

//------------------------------------------------------------------------------
//!
const ConstString&
DFBallNode::name() const
{
   return _sphereName;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeEditor>
DFBallNode::edit()
{
   if( _editor.isNull() )  _editor = new DFBallNodeEditor( this );
   return _editor;
}

//------------------------------------------------------------------------------
//!
DFOutput*
DFBallNode::output()
{
   return &_output;
}

//------------------------------------------------------------------------------
//!
RCP<DFGeometry>
DFBallNode::process()
{
   RCP<DFGeometry> geom = new DFGeometry();

   Mat4f m = _referential.toMatrix();

   // Set 8 corners.
   float r = _radius * 0.5681818181f * 2.0f; // Special factor to get closest to a sphere.
   geom->addControlPoint( m * Vec3f( -r,-r,-r ) );
   geom->addControlPoint( m * Vec3f(  r,-r,-r ) );
   geom->addControlPoint( m * Vec3f( -r, r,-r ) );
   geom->addControlPoint( m * Vec3f(  r, r,-r ) );
   geom->addControlPoint( m * Vec3f( -r,-r, r ) );
   geom->addControlPoint( m * Vec3f(  r,-r, r ) );
   geom->addControlPoint( m * Vec3f( -r, r, r ) );
   geom->addControlPoint( m * Vec3f(  r, r, r ) );

   // Set 6 faces.
   geom->addPatch( 0, 4, 6, 2, 0x0 );
   geom->addPatch( 1, 3, 7, 5, 0x0 );
   geom->addPatch( 0, 1, 5, 4, 0x0 );
   geom->addPatch( 2, 6, 7, 3, 0x0 );
   geom->addPatch( 0, 2, 3, 1, 0x0 );
   geom->addPatch( 4, 5, 7, 6, 0x0 );

   // Automatically compute neighbors.
   geom->computeNeighbors();

   // Create geometry.
   geom->subdivide( Plasma::geometricError() );

   // Fix to have perfect sphere (Catmull-ROM only approximates).
   uint32_t n = geom->numPatches();
   for( uint32_t i = 0; i < n; ++i )
   {
      auto& patch = geom->patch( i );
      for( auto cur = patch._vertices.begin(); cur != patch._vertices.end(); ++cur )
      {
         auto& v = *cur;
         v._n = normalize( v._pos );
         v._pos = v._n * _radius;
      }
   }

   return geom;
}

//-----------------------------------------------------------------------------
//!
bool
DFBallNode::dumpCustom( TextStream& os, StreamIndent& indent ) const
{
   os << indent << "radius = " << _radius               << "," << nl;
   os << indent << "ref = "    << VMFmt( _referential ) << "," << nl;
   return os.ok();
}

//------------------------------------------------------------------------------
//!
void
DFBallNode::init( VMState* vm, int idx )
{
   VM::get( vm, idx, "radius", _radius );
   VM::get( vm, idx, "ref", _referential );
}

/*==============================================================================
   CLASS DFExtrudePolyEditor
==============================================================================*/

class DFExtrudePolyEditor:
   public DFNodeEditor
{
public:

   /*----- methods -----*/

   DFExtrudePolyEditor( DFExtrudePolyNode* n ): _node(n) {}

   PLASMA_DLL_API virtual RCP<Manipulator>  manipulator() { return nullptr; }
   PLASMA_DLL_API virtual RCP<DFNodeAttrList>  attributes() const;
   PLASMA_DLL_API virtual RCP<DFNodeAttrStates>  attributesStates() const;
   PLASMA_DLL_API virtual void updateAttributes( const DFNodeAttrStates& );

protected:

   /*----- members -----*/

   DFExtrudePolyNode* _node;
};

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrList>
DFExtrudePolyEditor::attributes() const
{
   RCP<DFNodeAttrList> atts = new DFNodeAttrList();
   atts->add( DFNodeAttr( "FLOAT", ID_HEIGHT, "Height" ) );
   return atts;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrStates>
DFExtrudePolyEditor::attributesStates() const
{
   RCP<DFNodeAttrStates> states = new DFNodeAttrStates();
   states->set( ID_HEIGHT, _node->height() );
   return states;
}

//------------------------------------------------------------------------------
//!
void
DFExtrudePolyEditor::updateAttributes( const DFNodeAttrStates& attribs )
{
   for( auto cur = attribs.begin(); cur != attribs.end(); ++cur )
   {
      if( cur->_id == ID_HEIGHT )
      {
         _node->height( (*cur)._value.getFloat() );
         _node->graph()->invalidate( _node );
      }
   }
}

/*==============================================================================
   CLASS DFExtrudePolyNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFExtrudePolyNode::DFExtrudePolyNode():
   _input(this), _height( 1.0f )
{
   _output.delegate( makeDelegate( this, &DFExtrudePolyNode::process ) );
}

//------------------------------------------------------------------------------
//!
const ConstString&
DFExtrudePolyNode::name() const
{
   return _extrudePolyName;
}

//------------------------------------------------------------------------------
//!
uint
DFExtrudePolyNode::numInputs() const
{
   return 1;
}

//------------------------------------------------------------------------------
//!
DFOutput*
DFExtrudePolyNode::output()
{
   return &_output;
}

//------------------------------------------------------------------------------
//!
DFInput*
DFExtrudePolyNode::input( uint id )
{
   if( id == 0 ) return &_input;
   return nullptr;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeEditor>
DFExtrudePolyNode::edit()
{
   if( _editor.isNull() )  _editor = new DFExtrudePolyEditor( this );
   return _editor;
}

//------------------------------------------------------------------------------
//!
bool
DFExtrudePolyNode::dumpCustom( TextStream& os, StreamIndent& indent ) const
{
   os << indent << "height=" << _height << "," << nl;
   return os.ok();
}

//------------------------------------------------------------------------------
//!
RCP<DFGeometry>
DFExtrudePolyNode::process()
{
   RCP<DFPolygon> poly = _input.getPolygon();

   // Do we have a valid input?
   if( poly.isNull() ) return nullptr;

   poly->computeDerivedData();
   return DFGeomOps::extrude( *poly, _height );
}

NAMESPACE_END

