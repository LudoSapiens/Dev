/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/DataFlow/DFPolygonNodes.h>
#include <Plasma/DataFlow/DFNodeAttr.h>
#include <Plasma/DataFlow/DFGraph.h>
#include <Plasma/Manipulator/Manipulator.h>
#include <Plasma/Renderable/Renderable.h>
#include <Plasma/Geometry/MeshGeometry.h>
#include <Plasma/Intersector.h>

#include <Fusion/Core/Key.h>
#include <Fusion/VM/VMFmt.h>

#if _MSC_VER
// 'this' used in member initializer list.
#pragma warning( disable: 4355 )
#endif

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

ConstString _polygonName;

RCP<Gfx::DepthState> _defDepth    = new Gfx::DepthState();
RCP<Gfx::AlphaState> _defBlending = new Gfx::AlphaState();

// MODE_STROKES
Vec3f  _col_v_default    = Vec3f( 2.0f, 2.0f, 2.0f );
Vec3f  _col_v_selected_p = Vec3f( 4.0f, 0.0f, 0.0f );
Vec3f  _col_v_selected_s = Vec3f( 0.0f, 1.0f, 0.0f );
Vec3f  _col_e_default    = Vec3f( 0.0f, 0.0f, 0.0f );
Vec3f  _col_e_selected   = _col_v_selected_p;

float  _siz_v_default  =  8.0f;
float  _siz_v_selected = 13.0f;

//------------------------------------------------------------------------------
//!
RCP<DFNode> polygonVM( VMState* vm, int idx )
{
   RCP<DFPolygonNode> node = new DFPolygonNode();
   DFPolygon* poly         = node->polygon();

   // Reading multiple polygon/loops?.
   for( int p = 1; VM::geti( vm, idx, p ); ++p )
   {
      // Read one vertex.
      poly->reserveVertices( VM::getTableSize( vm, -1 ) );
      for( int i = 1; VM::geti( vm, -1, i ); ++i )
      {
         poly->addVertex( VM::toVec3f( vm, -1 ) );
         VM::pop(vm);
      }
      VM::pop(vm);
   }
   poly->computeDerivedData();
   return node;
}

//------------------------------------------------------------------------------
//!
inline void pushBack( Vector<float>& vec, const Vec3f& val )
{
   vec.pushBack( val.x );
   vec.pushBack( val.y );
   vec.pushBack( val.z );
}

//------------------------------------------------------------------------------
//!
inline void pushBack( Vector<float>& vec, float val )
{
   vec.pushBack( val );
}

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   STATIC INITIALIZATION
==============================================================================*/

//-----------------------------------------------------------------------------
//!
void initializePolygonNodes()
{
   _polygonName = "Polygon";

   DFNodeSpec::registerNode(
      DFSocket::POLYGON,
      _polygonName, polygonVM,
      "Polygon", "2D polygon generator.",
      nullptr
   );
}

//-----------------------------------------------------------------------------
//!
void terminatePolygonNodes()
{
   _polygonName = ConstString();
}

/*==============================================================================
   CLASS DFPolygonOutput
==============================================================================*/

//------------------------------------------------------------------------------
//!
RCP<DFPolygon>
DFPolygonOutput::getPolygon()
{
   return _delegate();
}

//------------------------------------------------------------------------------
//!
DFSocket::Type
DFPolygonOutput::type() const
{
   return POLYGON;
}

/*==============================================================================
   CLASS DFPolygonInput
==============================================================================*/

//------------------------------------------------------------------------------
//!
RCP<DFPolygon>
DFPolygonInput::getPolygon()
{
   if( !_output ) return nullptr;
   return _output->getPolygon();
}

//------------------------------------------------------------------------------
//!
DFSocket::Type
DFPolygonInput::type() const
{
   return POLYGON;
}

//------------------------------------------------------------------------------
//!
bool
DFPolygonInput::isConnected() const
{
   return _output != nullptr;
}

//------------------------------------------------------------------------------
//!
void
DFPolygonInput::connect( DFOutput* output )
{
   CHECK( type() == output->type() );
   if( _output ) disconnectFrom( _output );
   _output = (DFPolygonOutput*)output;
}

//------------------------------------------------------------------------------
//!
void
DFPolygonInput::disconnect( DFOutput* output )
{
   if( _output == output ) _output = nullptr;
}

//------------------------------------------------------------------------------
//!
void
DFPolygonInput::disconnect()
{
   if( !_output ) return;
   disconnectFrom( _output );
   _output = nullptr;
}

/*==============================================================================
   CLASS DFPolygonMultiInput
==============================================================================*/

//------------------------------------------------------------------------------
//!
RCP<DFPolygon>
DFPolygonMultiInput::getPolygon( uint i )
{
   if( i >= _outputs.size() ) return nullptr;
   return _outputs[i]->getPolygon();
}

//------------------------------------------------------------------------------
//!
DFSocket::Type
DFPolygonMultiInput::type() const
{
   return POLYGON;
}

//------------------------------------------------------------------------------
//!
bool
DFPolygonMultiInput::isConnected() const
{
   return !_outputs.empty();
}

//------------------------------------------------------------------------------
//!
void
DFPolygonMultiInput::connect( DFOutput* output )
{
   CHECK( type() == output->type() );
   _outputs.pushBack( (DFPolygonOutput*)output );
}

//------------------------------------------------------------------------------
//!
void
DFPolygonMultiInput::disconnect( DFOutput* output )
{
   _outputs.removeSwap( (DFPolygonOutput*)output );
}

//------------------------------------------------------------------------------
//!
void
DFPolygonMultiInput::disconnect()
{
   for( auto it = _outputs.begin(); it != _outputs.end(); ++it )
   {
      disconnectFrom( *it );
   }
   _outputs.clear();
}

/*==============================================================================
   CLASS DFPolygonRenderable
==============================================================================*/

class DFPolygonRenderable:
   public Renderable
{
public:

   /*----- methods -----*/

   DFPolygonRenderable( DFPolygonEditor* e );
   virtual void render( Gfx::Pass&, const Viewport& ) const;

   void update();

   MeshGeometry*   edgeMesh() { return _meshEdge.ptr(); }
   MeshGeometry* vertexMesh() { return _meshVert.ptr(); }

   bool pick( const Viewport&, const Vec2f& pos, float r, DFPolygonEditor::Pick& );

protected:

   //friend class DFStrokesEditor::Pick;

   /*----- data members -----*/

   DFPolygonEditor*       _editor;
   RCP<MeshGeometry>      _meshEdge;
   RCP<MeshGeometry>      _meshVert;
   RCP<Gfx::Program>      _progEdge;
   RCP<Gfx::Program>      _progVert;
   RCP<Gfx::SamplerList>  _texVert;
   RCP<Gfx::ConstantList> _constFront;
};

//------------------------------------------------------------------------------
//!
DFPolygonRenderable::DFPolygonRenderable( DFPolygonEditor* e ):
   _editor( e )
{
   // Material.
   Vec4f color(1.0f);
   RCP<Gfx::ConstantBuffer> constant = Core::gfx()->createConstants( 4*sizeof(float) );
   constant->addConstant( "color", Gfx::CONST_FLOAT4, 0, color.ptr() );
   _constFront = Gfx::ConstantList::create( constant );

   _progEdge = data( ResManager::getProgram( "shader/program/editor/l_standard" ) );
   _progVert = data( ResManager::getProgram( "shader/program/editor/p_standard" ) );

   Gfx::TextureState state;
   RCP<Image> img = data( ResManager::getImage( "image/particle1" ) );
   _texVert = new Gfx::SamplerList();
   _texVert->addSampler( "colorTex0", img->texture(), state );

   // Geometry.
   _meshEdge = new MeshGeometry( MeshGeometry::LINES  );
   const int edgeAttribs[] = {
      MeshGeometry::POSITION,
      MeshGeometry::COLOR,
      0
   };
   _meshEdge->setAttributes( edgeAttribs );

   _meshVert = new MeshGeometry( MeshGeometry::POINTS );
   const int vertAttribs[] = {
      MeshGeometry::POSITION,
      MeshGeometry::COLOR,
      MeshGeometry::GENERIC_1, // Size
      0
   };
   _meshVert->setAttributes( vertAttribs );

   update();
}

//------------------------------------------------------------------------------
//!
void
DFPolygonRenderable::update()
{
   Vector<float> egver; // vec3: pos, vec3: color
   Vector<float> vgver; // vec3: pos, vec3: color, flt: size
   Vector<uint32_t> eindices;
   Vector<uint32_t> vindices;

   const DFPolygon* poly = _editor->polygon();

   // Vertices.
   for( auto v = poly->begin(); v != poly->end(); ++v )
   {
      vindices.pushBack( uint(vindices.size()) );
      pushBack( vgver, (*v) );
      pushBack( vgver, _col_v_default );
      pushBack( vgver, _siz_v_default );

      pushBack( egver, (*v) );
      pushBack( egver, _col_e_default );
   }
   // Edges.
   const uint n = uint(poly->numVertices());
   for( uint i = 0, j = n-1; i < n; j=i++ )
   {
      eindices.pushBack( j );
      eindices.pushBack( i );
   }

   // Tweak for selection.
   auto& selection = _editor->selection();
   for( auto cur = selection.begin(); cur != selection.end(); ++cur )
   {
      float* ptr         = vgver.data() + cur->_idx*7;
      Vec3f::as( ptr+3 ) = (*cur) == selection.last() ? _col_v_selected_p : _col_v_selected_s;
      ptr[6]             = _siz_v_selected;
   }

   // Updating the meshes.
   MeshGeometry* emesh = edgeMesh();
   MeshGeometry* vmesh = vertexMesh();

   emesh->clearPatches();
   emesh->deallocate();
   if( !egver.empty() )
   {
      emesh->allocateIndices( uint(eindices.size()) );
      emesh->copyIndices( eindices.data() );
      emesh->allocateVertices( uint(egver.size())/6 );
      emesh->copyAttributes( egver.data(), 6, 6, 0 );
      emesh->addPatch( 0, uint(eindices.size()) );
   }
   emesh->updateProperties();
   emesh->invalidateRenderableGeometry();


   vmesh->clearPatches();
   vmesh->deallocate();
   if( !vgver.empty() )
   {
      vmesh->allocateIndices( uint(vindices.size()) );
      vmesh->copyIndices( vindices.data() );
      vmesh->allocateVertices( uint(vgver.size())/7 );
      vmesh->copyAttributes( vgver.data(), 7, 7, 0 );
      vmesh->addPatch( 0, uint(vindices.size()) );
   }
   vmesh->updateProperties();
   vmesh->invalidateRenderableGeometry();
}

//------------------------------------------------------------------------------
//!
void
DFPolygonRenderable::render( Gfx::Pass& pass, const Viewport& ) const
{
   // Front pass.
   pass.setAlphaState( _defBlending );
   pass.setDepthState( _defDepth );
   pass.setConstants( _constFront );
   pass.setProgram( _progEdge );
   pass.setSamplers(0);
   pass.setWorldMatrix( Mat4f::identity().ptr() );
   _meshEdge->render( pass );
   pass.setProgram( _progVert );
   pass.setSamplers( _texVert.ptr() );
   _meshVert->render( pass );
}

//------------------------------------------------------------------------------
//!
bool
DFPolygonRenderable::pick( const Viewport& vp, const Vec2f& pos, float r, DFPolygonEditor::Pick& p )
{
   uint32_t idx;
   if( vertexMesh()->pick( vp, pos, r, idx ) )
   {
      p._idx = idx;
      return true;
   }
   return false;
}

/*==============================================================================
   CLASS DFPolygonManipulator
==============================================================================*/

class DFPolygonManipulator:
   public Manipulator
{
public:

   /*----- methods -----*/

   DFPolygonManipulator( DFPolygonEditor* e, DFPolygonRenderable* r );

   inline DFPolygonEditor*  editor()      { return _editor; }
   virtual Renderable* renderable() const { return _renderable.ptr(); }

   // Events.
   virtual bool onKeyPress( const Event& ev );
   virtual bool onPointerPress( const Event& ev );
   virtual bool onPointerRelease( const Event& ev );
   virtual bool onPointerMove( const Event& ev );

protected:

   /*----- data members -----*/

   DFPolygonEditor*          _editor;
   RCP<DFPolygonRenderable>  _renderable;
   bool                      _clearSel;
   DFPolygonEditor::Pick     _pick;
   Planef                    _plane;
};

//------------------------------------------------------------------------------
//!
DFPolygonManipulator::DFPolygonManipulator( DFPolygonEditor* e, DFPolygonRenderable* r ):
   _editor( e ), _renderable( r )
{
}

//------------------------------------------------------------------------------
//!
bool
DFPolygonManipulator::onKeyPress( const Event& ev )
{
   switch( ev.value() )
   {
      case Key::BACKSPACE:
      case Key::DELETE: _editor->removeVertex(); return true;
      default:;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
bool
DFPolygonManipulator::onPointerPress( const Event& ev )
{
   _clearSel = false;
   // Vertex creation.
   if( Core::isKeyPressed( Key::SHIFT ) )
   {
      // Intersection ray and normal.
      float t = CGConstf::infinity();
      Rayf ray( camera()->position(), viewport()->direction( ev.position() ) );
      Planef plane( Vec3f( 0.0f, 0.0f, 1.0f ) );
      if( Intersector::trace( plane, ray, t ) )
      {
         Vec3f p = ray.point(t);
         _editor->createVertex( p );
      }
      return true;
   }

   // Picking.
   if( _renderable->pick( *viewport(), ev.position(), 10.0f, _pick ) )
   {
      // Update selection.
      if( Core::isKeyPressed(Key::CTRL) )
      {
         if( !_editor->toggleSelection( _pick ) ) _pick.invalidate();
      }
      else if( !_editor->selection().has( _pick ) ) _editor->setSelection( _pick );
      // Find intersection plane.
      Vec3f n( 0.0f, 0.0f, 1.0f );
      Vec3f p = editor()->polygon()->vertex( _pick._idx );
      _plane  = Planef( n, p );
   }
   else
      _clearSel = true;
   return !_clearSel;
}

//------------------------------------------------------------------------------
//!
bool
DFPolygonManipulator::onPointerRelease( const Event& ev )
{
   _pick.invalidate();
   if( _clearSel && ev.pointer().withinPress(5) )
   {
      _clearSel = false;
      _editor->clearSelection();
   }
   return false;
}

//------------------------------------------------------------------------------
//!
bool
DFPolygonManipulator::onPointerMove( const Event& ev )
{
   if( _pick.isValid() )
   {
      Rayf ray( camera()->position(), viewport()->direction( ev.position() ) );
      float t = CGConstf::infinity();
      if( Intersector::trace( _plane, ray, t ) )
      {
         Vec3f p = ray.point(t);
         _editor->moveVertex( p );
      }
      return true;
   }
   return false;
}

/*==============================================================================
   CLASS DFPolygonEditor
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFPolygonEditor::DFPolygonEditor( DFPolygonNode* n ): _node(n)
{
   _renderable = new DFPolygonRenderable( this );
}

//------------------------------------------------------------------------------
//!
RCP<Manipulator>
DFPolygonEditor::manipulator()
{
   return RCP<Manipulator>( new DFPolygonManipulator( this, _renderable.ptr() ) );
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrList>
DFPolygonEditor::attributes() const
{
   RCP<DFNodeAttrList> atts = new DFNodeAttrList();

   // Strokes UI.
   RCP<DFNodeAttrList> strokes = new DFNodeAttrList();
   atts->add( DFNodeAttr( "XYZ", 0, "Position" ) );

   return atts;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrStates>
DFPolygonEditor::attributesStates() const
{
   RCP<DFNodeAttrStates> states = new DFNodeAttrStates();
   const Pick& pick             = _selection.last();

   if( pick.isValid() )
      states->set( 0, polygon()->vertex( pick._idx ) );

   return states;
}

//------------------------------------------------------------------------------
//!
void DFPolygonEditor::updateAttributes( const DFNodeAttrStates& attribs )
{
   for( auto cur = attribs.begin(); cur != attribs.end(); ++cur )
   {
      if( cur->_id == 0 )
      {
         moveVertex( cur->_value.getVec3() );
      }
   }
}

//------------------------------------------------------------------------------
//!
void DFPolygonEditor::updateUI()
{
   _node->graph()->msg().modify( _node, attributesStates().ptr() );
}

//------------------------------------------------------------------------------
//!
void
DFPolygonEditor::updateSelection()
{
   updateUI();
   if( _renderable.isValid() ) _renderable->update();
}

//------------------------------------------------------------------------------
//!
void
DFPolygonEditor::updateAll()
{
   // Invalidate graph.
   _node->graph()->invalidate( _node );
   // Invalidate UI and renderable.
   updateUI();
   if( _renderable.isValid() ) _renderable->update();
}

//------------------------------------------------------------------------------
//!
void
DFPolygonEditor::moveVertex( const Vec3f& p )
{
   if( _selection.empty() ) return;

   // Update geometry.
   DFPolygon* poly = polygon();
   Vec3f delta     = p - poly->vertex( _selection.last()._idx );

   for( auto pick = _selection.begin(); pick != _selection.end(); ++pick )
   {
      poly->vertex( pick->_idx ) += delta;
   }

   updateAll();
}

//------------------------------------------------------------------------------
//!
void
DFPolygonEditor::createVertex( const Vec3f& p )
{
   Pick nPick;
   if( _selection.empty() )
   {
      polygon()->addVertex( p );
      nPick._idx = int(polygon()->numVertices())-1;
   }
   else
   {
      nPick._idx = _selection.last()._idx+1;
      polygon()->insertVertex( nPick._idx, p );
   }

   // Handle selection.
   _selection.set( nPick );

   updateAll();
}

//------------------------------------------------------------------------------
//!
void
DFPolygonEditor::removeVertex()
{
   // Deleting vertices.
   DFPolygon* poly = polygon();
   for( auto pick = _selection.begin(); pick != _selection.end(); ++pick )
   {
      poly->removeVertex( pick->_idx );
      // For now only remove one element since other selection id could be invalid.
      break;
   }
   _selection.clear();

   updateAll();
}

/*==============================================================================
   CLASS DFPolygonNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFPolygonNode::DFPolygonNode()
{
   _output.delegate( makeDelegate( this, &DFPolygonNode::process ) );
   _polygon = new DFPolygon();
}

//------------------------------------------------------------------------------
//!
const ConstString&
DFPolygonNode::name() const
{
   return _polygonName;
}

//------------------------------------------------------------------------------
//!
uint
DFPolygonNode::numInputs() const
{
   return 0;
}

//------------------------------------------------------------------------------
//!
DFOutput*
DFPolygonNode::output()
{
   return &_output;
}

//------------------------------------------------------------------------------
//!
DFInput*
DFPolygonNode::input( uint )
{
   return nullptr;
}

//------------------------------------------------------------------------------
//!
RCP<DFPolygon>
DFPolygonNode::process()
{
   return _polygon;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeEditor>
DFPolygonNode::edit()
{
   if( _editor.isNull() )  _editor = new DFPolygonEditor( this );
   return _editor;
}

//------------------------------------------------------------------------------
//!
bool
DFPolygonNode::dumpCustom( TextStream& os, StreamIndent& indent ) const
{
   os << indent << "{" << nl;
   ++indent;
   for( auto cur = _polygon->begin(); cur != _polygon->end(); ++cur )
   {
      os << indent << VMFmt( *cur ) << "," << nl;
   }
   --indent;
   os << indent << "}" << nl;
   return os.ok();
}

NAMESPACE_END
