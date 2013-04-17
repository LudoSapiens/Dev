/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/DataFlow/DFStrokesNodes.h>
#include <Plasma/DataFlow/DFStrokes.h>
#include <Plasma/Manipulator/Manipulator.h>
#include <Plasma/Renderable/Renderable.h>
#include <Plasma/Geometry/MeshGeometry.h>

#if _MSC_VER
// 'this' used in member initializer list.
#pragma warning( disable: 4355 )
#endif

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

ConstString _strokesToGeomName;
ConstString _strokesDebugName;

//------------------------------------------------------------------------------
//!
RCP<DFNode> strokesToGeomVM( VMState*, int )
{
   RCP<DFStrokesToGeometryNode> node = new DFStrokesToGeometryNode();
   return node;
}

//------------------------------------------------------------------------------
//!
RCP<DFNode> strokesDebugVM( VMState*, int )
{
   RCP<DFStrokesDebugNode> node = new DFStrokesDebugNode();
   return node;
}

//------------------------------------------------------------------------------
//!
RCP<Gfx::DepthState> _failDepth   = new Gfx::DepthState();
RCP<Gfx::DepthState> _defDepth    = new Gfx::DepthState();
RCP<Gfx::AlphaState> _defBlending = new Gfx::AlphaState();

// Colors:
// MODE_STROKES
Vec3f  _col_stk_v_default    = Vec3f( 2.0f, 2.0f, 2.0f );
Vec3f  _col_stk_v_selected_p = Vec3f( 4.0f, 0.0f, 0.0f );
Vec3f  _col_stk_v_selected_s = Vec3f( 0.0f, 1.0f, 0.0f );
Vec3f  _col_stk_e_default    = Vec3f( 0.0f, 0.0f, 0.0f );
Vec3f  _col_stk_e_selected   = _col_stk_v_selected_p;
// MODE_RINGS
Vec3f  _col_rng_v_default    = _col_stk_v_default;
Vec3f  _col_rng_v_selected_p = _col_stk_v_selected_p;
Vec3f  _col_rng_v_selected_s = _col_stk_v_selected_s;
Vec3f  _col_rng_e_default    = _col_stk_e_default;
Vec3f  _col_rng_e_selected_p = _col_stk_e_selected;
Vec3f  _col_rng_e_selected_s = _col_stk_v_selected_s;
// MODE_EDGES
Vec3f  _col_edg_v_default    = _col_stk_v_default;
Vec3f  _col_edg_v_selected_p = _col_stk_v_selected_p;
Vec3f  _col_edg_v_selected_s = _col_stk_v_selected_s;
Vec3f  _col_edg_e_sharp      = Vec3f( 1.5f, 0.0f, 0.0f );
Vec3f  _col_edg_e_smooth     = Vec3f( 0.0f, 0.0f, 1.5f );

// Sizes:
// MODE_STROKES
float  _siz_stk_v_default  =  8.0f;
// MODE_RINGS
float  _siz_rng_v_default  = _siz_stk_v_default;
// MODE_EDGES
float  _siz_edg_v_default  = _siz_stk_v_default;

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

//------------------------------------------------------------------------------
//!
void updateRingsManip(
   MeshGeometry*  emesh,
   MeshGeometry*  vmesh,
   DFStrokes*     strokesGeom
)
{
   Vector<float> egver; // vec3: pos, vec3: color
   Vector<float> vgver; // vec3: pos, vec3: color, flt: size
   Vector<uint32_t> eindices;
   Vector<uint32_t> vindices;

   // Each strokes
   Vec3f pos[4];
   uint32_t vID = 0;
   auto& strokes = strokesGeom->strokes();
   for( auto s = strokes.begin(); s != strokes.end(); ++s )
   {
      if( s->_vertices.empty() ) continue;

      // Each vertex/segment.
      for( size_t vi = 0; vi < s->_vertices.size(); ++vi )
      {
         DFStrokes::Vertex& v = s->_vertices[vi];
         Mat4f m              = strokesGeom->referential( *s, int(vi) ).toMatrix();

         for( int i = 0; i < 4; ++i ) pos[i] = m * v._corners[i];

         // Center vertex.
         vindices.pushBack( uint(vindices.size()) );
         pushBack( vgver, v._ref.position() );
         pushBack( vgver, _col_rng_v_default );
         pushBack( vgver, _siz_rng_v_default );

         // Ring edges.
         for( int i = 0; i < 4; ++i )
         {
            const Vec3f& p0 = pos[i];
            const Vec3f& p1 = pos[(i+1)%4];

            // Vertices.
            //vindices.pushBack( vindices.size() );
            //pushBack( vgver, p0 );
            //pushBack( vgver, _col_edg_v_default );
            //pushBack( vgver, _siz_edg_v_default );

            // Edges.
            pushBack( egver, p0 );
            pushBack( egver, _col_rng_e_default );
            pushBack( egver, p1 );
            pushBack( egver, _col_rng_e_default );

            eindices.pushBack( vID );
            eindices.pushBack( vID+1 );
            vID += 2;
         }
      }
   }

   // Updating the mesh
   emesh->allocateIndices( uint(eindices.size()) );
   emesh->copyIndices( eindices.data() );
   emesh->allocateVertices( uint(egver.size())/6 );
   emesh->copyAttributes( egver.data(), 6, 6, 0 );
   emesh->clearPatches();
   emesh->addPatch( 0, uint(eindices.size()) );
   emesh->updateProperties();
   emesh->invalidateRenderableGeometry();

   vmesh->allocateIndices( uint(vindices.size()) );
   vmesh->copyIndices( vindices.data() );
   vmesh->allocateVertices( uint(vgver.size())/7 );
   vmesh->copyAttributes( vgver.data(), 7, 7, 0 );
   vmesh->clearPatches();
   vmesh->addPatch( 0, uint(vindices.size()) );
   vmesh->updateProperties();
   vmesh->invalidateRenderableGeometry();
}

//------------------------------------------------------------------------------
//!
void updateEdgesManip(
   MeshGeometry*  emesh,
   MeshGeometry*  vmesh,
   DFStrokes*     strokesGeom
)
{
   Vector<float> egver; // vec3: pos, vec3: color
   Vector<float> vgver; // vec3: pos, vec3: color, flt: size
   Vector<uint32_t> eindices;
   Vector<uint32_t> vindices;

   // Each strokes
   Vec3f p0s[4];
   Vec3f p1s[4];
   uint32_t vID  = 0;
   auto& strokes = strokesGeom->strokes();
   for( auto s = strokes.begin(); s != strokes.end(); ++s )
   {
      if( s->_vertices.empty() ) continue;

      // Each vertex/segment.
      for( size_t vi = 0; vi < s->_vertices.size(); ++vi )
      {
         DFStrokes::Vertex& v = s->_vertices[vi];
         Mat4f m              = strokesGeom->referential( *s, int(vi) ).toMatrix();

         for( int i = 0; i < 4; ++i ) p1s[i] = m * v._corners[i];

         // Segment.
         if( vi > 0 )
         {
            DFStrokes::Segment& seg = s->_segments[vi-1];
            for( int i = 0; i < 4; ++i )
            {
               Vec3f c  = ((seg._creases>>i)&1) == 1 ? _col_edg_e_sharp : _col_edg_e_smooth;

               pushBack( egver, p0s[i] );
               pushBack( egver, c );
               pushBack( egver, p1s[i] );
               pushBack( egver, c );

               eindices.pushBack( vID );
               eindices.pushBack( vID+1 );
               vID += 2;
            }
         }
         // Vertex.
         Vec3f p0 = p1s[0];
         for( int i = 0; i < 4; ++i )
         {
            // Vertices.
            vindices.pushBack( uint(vindices.size()) );
            pushBack( vgver, p0 );
            pushBack( vgver, _col_edg_v_default );
            pushBack( vgver, _siz_edg_v_default );

            // Edges.
            Vec3f c  = ((v._creases>>i)&1) == 1 ? _col_edg_e_sharp : _col_edg_e_smooth;
            Vec3f p1 = p1s[(i+1)%4];
            pushBack( egver, p0 );
            pushBack( egver, c );
            pushBack( egver, p1 );
            pushBack( egver, c );

            eindices.pushBack( vID );
            eindices.pushBack( vID+1 );
            vID += 2;
            p0   = p1;
         }
         for( int i = 0; i < 4; ++i ) p0s[i] = p1s[i];
      }
   }

   // Updating the mesh
   emesh->allocateIndices( uint(eindices.size()) );
   emesh->copyIndices( eindices.data() );
   emesh->allocateVertices( uint(egver.size())/6 );
   emesh->copyAttributes( egver.data(), 6, 6, 0 );
   emesh->clearPatches();
   emesh->addPatch( 0, uint(eindices.size()) );
   emesh->updateProperties();
   emesh->invalidateRenderableGeometry();

   vmesh->allocateIndices( uint(vindices.size()) );
   vmesh->copyIndices( vindices.data() );
   vmesh->allocateVertices( uint(vgver.size())/7 );
   vmesh->copyAttributes( vgver.data(), 7, 7, 0 );
   vmesh->clearPatches();
   vmesh->addPatch( 0, uint(vindices.size()) );
   vmesh->updateProperties();
   vmesh->invalidateRenderableGeometry();
}


UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   STATIC INITIALIZATION
==============================================================================*/

//-----------------------------------------------------------------------------
//!
void initializeStrokesNodes()
{
   _strokesToGeomName = "strokesToGeometry";
   _strokesDebugName  = "strokesDebug";

   DFNodeSpec::registerNode(
      DFSocket::GEOMETRY,
      _strokesToGeomName, strokesToGeomVM,
      "Convert", "Converts a strokes into a geometry.",
      nullptr
   );

   DFNodeSpec::registerNode(
      DFSocket::STROKES,
      _strokesDebugName, strokesDebugVM,
      "Debug", "Show more info on the strokes.",
      nullptr
   );
}

//-----------------------------------------------------------------------------
//!
void terminateStrokesNodes()
{
   _strokesToGeomName = ConstString();
   _strokesDebugName  = ConstString();
}


/*==============================================================================
   CLASS DFStrokesOutput
==============================================================================*/

//------------------------------------------------------------------------------
//!
RCP<DFStrokes>
DFStrokesOutput::getStrokes()
{
   return _delegate();
}

//------------------------------------------------------------------------------
//!
DFSocket::Type
DFStrokesOutput::type() const
{
   return STROKES;
}

/*==============================================================================
   CLASS DFStrokesInput
==============================================================================*/

//------------------------------------------------------------------------------
//!
RCP<DFStrokes>
DFStrokesInput::getStrokes()
{
   if( !_output ) return nullptr;
   return _output->getStrokes();
}

//------------------------------------------------------------------------------
//!
DFSocket::Type
DFStrokesInput::type() const
{
   return STROKES;
}

//------------------------------------------------------------------------------
//!
bool
DFStrokesInput::isConnected() const
{
   return _output != nullptr;
}

//------------------------------------------------------------------------------
//!
void
DFStrokesInput::connect( DFOutput* output )
{
   CHECK( type() == output->type() );
   if( _output ) disconnectFrom( _output );
   _output = (DFStrokesOutput*)output;
}

//------------------------------------------------------------------------------
//!
void
DFStrokesInput::disconnect( DFOutput* output )
{
   if( _output == output ) _output = nullptr;
}

//------------------------------------------------------------------------------
//!
void
DFStrokesInput::disconnect()
{
   if( !_output ) return;
   disconnectFrom( _output );
   _output = nullptr;
}

/*==============================================================================
   CLASS DFStrokesMultiInput
==============================================================================*/

//------------------------------------------------------------------------------
//!
RCP<DFStrokes>
DFStrokesMultiInput::getStrokes( uint i )
{
   if( i >= _outputs.size() ) return nullptr;
   return _outputs[i]->getStrokes();
}

//------------------------------------------------------------------------------
//!
DFSocket::Type
DFStrokesMultiInput::type() const
{
   return STROKES;
}

//------------------------------------------------------------------------------
//!
bool
DFStrokesMultiInput::isConnected() const
{
   return !_outputs.empty();
}

//------------------------------------------------------------------------------
//!
void
DFStrokesMultiInput::connect( DFOutput* output )
{
   CHECK( type() == output->type() );
   _outputs.pushBack( (DFStrokesOutput*)output );
}

//------------------------------------------------------------------------------
//!
void
DFStrokesMultiInput::disconnect( DFOutput* output )
{
   _outputs.removeSwap( (DFStrokesOutput*)output );
}

//------------------------------------------------------------------------------
//!
void
DFStrokesMultiInput::disconnect()
{
   for( auto it = _outputs.begin(); it != _outputs.end(); ++it )
   {
      disconnectFrom( *it );
   }
   _outputs.clear();
}

/*==============================================================================
   CLASS DFStrokesToGeometryNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFStrokesToGeometryNode::DFStrokesToGeometryNode():
   _input(this)
{
   _output.delegate( makeDelegate( this, &DFStrokesToGeometryNode::process ) );
}

//------------------------------------------------------------------------------
//!
const ConstString&
DFStrokesToGeometryNode::name() const
{
   return _strokesToGeomName;
}

//------------------------------------------------------------------------------
//!
uint
DFStrokesToGeometryNode::numInputs() const
{
   return 1;
}

//------------------------------------------------------------------------------
//!
DFOutput*
DFStrokesToGeometryNode::output()
{
   return &_output;
}

//------------------------------------------------------------------------------
//!
DFInput*
DFStrokesToGeometryNode::input( uint id )
{
   return (id == 0) ? &_input : nullptr;
}

//------------------------------------------------------------------------------
//!
RCP<DFGeometry>
DFStrokesToGeometryNode::process()
{
   RCP<DFStrokes> strokes = _input.getStrokes();
   if( strokes.isNull() )  return nullptr;
   return strokes->geometry();
}

/*==============================================================================
   CLASS DFStrokesDebugRenderable
==============================================================================*/

class DFStrokesDebugRenderable:
   public Renderable
{
public:

   /*----- methods -----*/

   DFStrokesDebugRenderable();
   virtual void render( Gfx::Pass&, const Viewport& ) const;

   void update( DFStrokes* );

protected:

   /*----- methods -----*/

   void init();

   /*----- data members -----*/

   int                    _mode;
   RCP<MeshGeometry>      _meshEdge;
   RCP<MeshGeometry>      _meshVert;
   RCP<Gfx::Program>      _progEdge;
   RCP<Gfx::Program>      _progVert;
   RCP<Gfx::SamplerList>  _texVert;
   RCP<Gfx::ConstantList> _constFront;
   RCP<Gfx::ConstantList> _constBack;
};

//------------------------------------------------------------------------------
//!
DFStrokesDebugRenderable::DFStrokesDebugRenderable():
   _mode(0)
{
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
}

//------------------------------------------------------------------------------
//!
void
DFStrokesDebugRenderable::init()
{
   if( _progEdge.isValid() ) return;

   _failDepth->depthTestFunc( Gfx::COMPARE_FUNC_GREATER );
   _failDepth->depthWriting( false );

   // Material.
   Vec4f color(1.0f);
   RCP<Gfx::ConstantBuffer> constant = Core::gfx()->createConstants( 4*sizeof(float) );
   constant->addConstant( "color", Gfx::CONST_FLOAT4, 0, color.ptr() );
   _constFront = Gfx::ConstantList::create( constant );

   color = Vec4f(0.15f);
   constant = Core::gfx()->createConstants( 4*sizeof(float) );
   constant->addConstant( "color", Gfx::CONST_FLOAT4, 0, color.ptr() );
   _constBack = Gfx::ConstantList::create( constant );

   _progEdge = data( ResManager::getProgram( "shader/program/editor/l_standard" ) );
   _progVert = data( ResManager::getProgram( "shader/program/editor/p_standard" ) );

   Gfx::TextureState state;
   RCP<Image> img = data( ResManager::getImage( "image/particle1" ) );
   _texVert = new Gfx::SamplerList();
   _texVert->addSampler( "colorTex0", img->texture(), state );
}

//------------------------------------------------------------------------------
//!
void
DFStrokesDebugRenderable::update( DFStrokes* strokes )
{
   if( _mode == 0 )
      updateEdgesManip( _meshEdge.ptr(), _meshVert.ptr(), strokes );
   else
      updateRingsManip( _meshEdge.ptr(), _meshVert.ptr(), strokes );
}

//------------------------------------------------------------------------------
//!
void
DFStrokesDebugRenderable::render( Gfx::Pass& pass, const Viewport& ) const
{
   // Lazy init.
   const_cast<DFStrokesDebugRenderable*>(this)->init();

   // Behind pass.
   pass.setDepthState( _failDepth );
   pass.setAlphaState( _defBlending );
   pass.setConstants( _constBack );
   pass.setWorldMatrix( Mat4f::identity().ptr() );
   pass.setProgram( _progEdge );
   pass.setSamplers(0);
   _meshEdge->render( pass );
   pass.setProgram( _progVert );
   pass.setSamplers( _texVert.ptr() );
   _meshVert->render( pass );

   // Front pass.
   pass.setDepthState( _defDepth );
   pass.setConstants( _constFront );
   pass.setProgram( _progEdge );
   pass.setSamplers(0);
   _meshEdge->render( pass );
   pass.setProgram( _progVert );
   pass.setSamplers( _texVert.ptr() );
   _meshVert->render( pass );
}

/*==============================================================================
   CLASS DFStrokesDebugManipulator
==============================================================================*/

class DFStrokesDebugManipulator:
   public Manipulator
{
public:

   /*----- methods -----*/

   DFStrokesDebugManipulator( DFStrokesDebugRenderable* r ): _renderable( r ) {}

   virtual Renderable* renderable() const { return _renderable.ptr(); }

protected:

   /*----- data members -----*/

   RCP<DFStrokesDebugRenderable>  _renderable;
};

/*==============================================================================
   CLASS DFStrokesDebugEditor
==============================================================================*/

class DFStrokesDebugEditor:
   public DFNodeEditor
{
public:

   /*----- methods -----*/

   DFStrokesDebugEditor( DFStrokesDebugNode* n ): _node(n)
   {
      _renderable = new DFStrokesDebugRenderable();
   }

   PLASMA_DLL_API virtual RCP<Manipulator> manipulator();

   DFStrokesDebugRenderable* renderable() { return _renderable.ptr(); }

   //PLASMA_DLL_API virtual RCP<DFNodeAttrList> attributes() const;
   //PLASMA_DLL_API virtual RCP<DFNodeAttrStates> attributesStates() const;
   //PLASMA_DLL_API virtual void updateAttributes( const DFNodeAttrStates& );

protected:

   /*----- data members -----*/

   DFStrokesDebugNode*            _node;
   RCP<DFStrokesDebugRenderable>  _renderable;
};

//------------------------------------------------------------------------------
//!
RCP<Manipulator>
DFStrokesDebugEditor::manipulator()
{
   return RCP<Manipulator>( new DFStrokesDebugManipulator( _renderable.ptr() ) );
}

/*==============================================================================
   CLASS DFStrokesDebugNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFStrokesDebugNode::DFStrokesDebugNode():
   _input(this)
{
   _output.delegate( makeDelegate( this, &DFStrokesDebugNode::process ) );
   _editor = new DFStrokesDebugEditor( this );
}

//------------------------------------------------------------------------------
//!
const ConstString&
DFStrokesDebugNode::name() const
{
   return _strokesDebugName;
}

//------------------------------------------------------------------------------
//!
uint
DFStrokesDebugNode::numInputs() const
{
   return 1;
}

//------------------------------------------------------------------------------
//!
DFOutput*
DFStrokesDebugNode::output()
{
   return &_output;
}

//------------------------------------------------------------------------------
//!
DFInput*
DFStrokesDebugNode::input( uint id )
{
   return (id == 0) ? &_input : nullptr;
}

//------------------------------------------------------------------------------
//!
RCP<DFStrokes>
DFStrokesDebugNode::process()
{
   RCP<DFStrokes> strokes = _input.getStrokes();
   // Update renderable.
   DFStrokesDebugEditor* e = (DFStrokesDebugEditor*)_editor.ptr();
   if( e )
   {
      DFStrokesDebugRenderable* r = e->renderable();
      if( r ) r->update( strokes.ptr() );
   }
   return strokes;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeEditor>
DFStrokesDebugNode::edit()
{
   return _editor;
}


NAMESPACE_END
