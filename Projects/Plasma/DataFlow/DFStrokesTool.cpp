/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/DataFlow/DFStrokesTool.h>
#include <Plasma/DataFlow/DFNodeAttr.h>
#include <Plasma/DataFlow/DFGraph.h>
#include <Plasma/DataFlow/DFStrokes.h>
#include <Plasma/Geometry/MeshGeometry.h>
#include <Plasma/Intersector.h>
#include <Plasma/Manipulator/ManipulatorGroup.h>
#include <Plasma/Manipulator/RefManipulator.h>
#include <Plasma/Renderable/RenderableGroup.h>

#include <Fusion/Core/Key.h>
#include <Fusion/VM/VMFmt.h>

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

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
float  _siz_stk_v_selected = 13.0f;
// MODE_RINGS
float  _siz_rng_v_default  = _siz_stk_v_default;
float  _siz_rng_v_selected = _siz_stk_v_selected;
// MODE_EDGES
float  _siz_edg_v_default  = _siz_stk_v_default;
//float  _siz_edg_v_selected = _siz_stk_v_selected;

//------------------------------------------------------------------------------
//!
Vec3f alignedZ( const Quatf& q )
{
   Vec3f z = q.getAxisAligned().getAxisZ();
   Vec3f n(0.0f);
   n( z.maxComponent() ) = 1.0f;
   return n;
}

//------------------------------------------------------------------------------
//!
bool pick( DFStrokes* strokesGeom, Viewport& vp, const Vec2f& pos, DFStrokesEditor::Pick& p )
{
   p.setInvalid();
   Rayf ray( vp.camera()->position(), vp.direction( pos ) );

   auto& strokes = strokesGeom->strokes();
   Intersector::Hit hit;

   const uint32_t si_n = uint32_t(strokes.size());
   for( uint32_t si = 0; si < si_n; ++si )
   {
      DFStrokes::Stroke& s  = strokes[si];
      Reff r0               = strokesGeom->referential( s, 0 );
      Mat4f m0              = r0.toMatrix();
      DFStrokes::Vertex& v0 = s._vertices[0];

      Vec3f p0s[4];
      for( int c = 0; c < 4; ++c ) p0s[c] = m0*v0._corners[c];

      // Picking of first vertex face.
      if( Intersector::trace( p0s[2], p0s[1], p0s[0], ray, hit ) ) p.setStrokeRingFace( si, 0 );
      if( Intersector::trace( p0s[2], p0s[0], p0s[3], ray, hit ) ) p.setStrokeRingFace( si, 0 );

      // Picking of lateral faces.
      const uint32_t i_n = uint32_t(s._segments.size());
      for( uint32_t i = 0; i < i_n; ++i )
      {
         DFStrokes::Vertex& v1 = s._vertices[i+1];
         Reff r1               = strokesGeom->referential( s, int(i)+1 );
         Mat4f m1              = r1.toMatrix();

         Vec3f p1s[4];
         for( int c = 0; c < 4; ++c ) p1s[c] = m1*v1._corners[c];

         // Test the four side of the segment/block.
         for( uint16_t c = 0; c < 4; ++c )
         {
            int c2 = (c+1)%4;
            if( Intersector::trace( p0s[c], p0s[c2], p1s[c], ray, hit ) )  p.setStrokeFace( si, i, c );
            if( Intersector::trace( p0s[c2], p1s[c2], p1s[c], ray, hit ) ) p.setStrokeFace( si, i, c );
         }

         for( int c = 0; c < 4; ++c ) p0s[c] = p1s[c];
      }

      // Picking of last vertex face.
      uint32_t li = uint32_t(s._segments.size());
      if( li > 0 )
      {
         if( Intersector::trace( p0s[0], p0s[1], p0s[2], ray, hit ) ) p.setStrokeRingFace( si, li );
         if( Intersector::trace( p0s[0], p0s[2], p0s[3], ray, hit ) ) p.setStrokeRingFace( si, li );
      }
   }
   return p.isValid();
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

//------------------------------------------------------------------------------
//!
void updateStrokesManip(
   MeshGeometry*                      emesh,
   MeshGeometry*                      vmesh,
   DFStrokes*                         strokesGeom,
   const DFStrokesEditor::Selection&  selection
)
{
   Vector<float> egver; // vec3: pos, vec3: color
   Vector<float> vgver; // vec3: pos, vec3: color, flt: size
   Vector<uint32_t> eindices;
   Vector<uint32_t> vindices;

   // Each strokes
   uint32_t vID  = 0;
   auto& strokes = strokesGeom->strokes();
   for( auto s = strokes.begin(); s != strokes.end(); ++s )
   {
      if( s->_vertices.empty() ) continue;

      // First vertex.
      vindices.pushBack( uint(vindices.size()) );
      pushBack( vgver, s->_vertices[0]._ref.position() );
      pushBack( vgver, _col_stk_v_default );
      pushBack( vgver, _siz_stk_v_default );

      // Each segment.
      for( size_t i = 0; i < s->_segments.size(); ++i )
      {

         DFStrokes::Vertex& v0 = s->_vertices[i];
         DFStrokes::Vertex& v1 = s->_vertices[i+1];
         // Interior.
         Vec3f p0 = v0._ref.position();
         Vec3f p1 = v1._ref.position();

         pushBack( egver, p0 );
         pushBack( egver, _col_stk_e_default );
         pushBack( egver, p1 );
         pushBack( egver, _col_stk_e_default );

         eindices.pushBack( vID );
         eindices.pushBack( vID+1 );
         vID += 2;

         // Vertex.
         vindices.pushBack( uint(vindices.size()) );
         pushBack( vgver, p1 );
         pushBack( vgver, _col_stk_v_default );
         pushBack( vgver, _siz_stk_v_default );
      }
   }

   // Tweak for selection.
   for( auto cur = selection.begin(); cur != selection.end(); ++cur )
   {
      const DFStrokesEditor::Pick& p = *cur;
      switch( p.type() )
      {
         case DFStrokesEditor::PICK_STROKE_VERTEX:
         {
            float* ptr         = vgver.data() + p.geomIdx()*7;
            Vec3f::as( ptr+3 ) = p == selection.last() ? _col_stk_v_selected_p : _col_stk_v_selected_s;
            ptr[6]             = _siz_stk_v_selected;
         }  break;
         case DFStrokesEditor::PICK_STROKE_SEGMENT_EDGE:
         {
            float* ptr       = egver.data() + p.geomIdx()*12 + 3;
            Vec3f::as( ptr ) = _col_stk_e_selected;
            ptr             += 6;
            Vec3f::as( ptr ) = _col_stk_e_selected;
         }  break;
         default: ;
      }
   }

   // Updating the meshes.
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
void updateRingsManip(
   MeshGeometry*                      emesh,
   MeshGeometry*                      vmesh,
   DFStrokes*                         strokesGeom,
   const DFStrokesEditor::Selection&  selection
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
      const uint vi_n = uint(s->_vertices.size());
      for( uint vi = 0; vi < vi_n; ++vi )
      {
         DFStrokes::Vertex& v = s->_vertices[vi];
         Mat4f m              = strokesGeom->referential( *s, vi ).toMatrix();

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

   // Tweak for selection.
   for( auto cur = selection.begin(); cur != selection.end(); ++cur )
   {
      const DFStrokesEditor::Pick& p = *cur;
      switch( p.type() )
      {
         case DFStrokesEditor::PICK_STROKE_VERTEX:
         {
            Vec3f ecol = _col_rng_e_selected_s;
            Vec3f vcol = _col_rng_v_selected_s;
            if( p == selection.last() )
            {
               ecol = _col_rng_e_selected_p;
               vcol = _col_rng_v_selected_p;
            }

            float* ptr         = vgver.data() + p.geomIdx()*7;
            Vec3f::as( ptr+3 ) = vcol;
            ptr[6]             = _siz_rng_v_selected;
            ptr                = egver.data() + p.geomIdx()*6*2*4;
            for( int i = 0; i < 4; ++i, ptr += 6*2 )
            {
               Vec3f::as( ptr+3 ) = ecol;
               Vec3f::as( ptr+9 ) = ecol;
            }
         }  break;
         default: ;
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
   MeshGeometry*                      emesh,
   MeshGeometry*                      vmesh,
   DFStrokes*                         strokesGeom,
   const DFStrokesEditor::Selection&  selection
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
      const uint vi_n = uint(s->_vertices.size());
      for( uint vi = 0; vi < vi_n; ++vi )
      {
         DFStrokes::Vertex& v = s->_vertices[vi];
         Mat4f m              = strokesGeom->referential( *s, vi ).toMatrix();

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

   // Tweak for selection.
   for( auto cur = selection.begin(); cur != selection.end(); ++cur )
   {
      const DFStrokesEditor::Pick& p = *cur;
      switch( p.type() )
      {
         case DFStrokesEditor::PICK_STROKE_RING_VERTEX:
         {
            float* ptr         = vgver.data() + p.geomIdx()*7;
            Vec3f::as( ptr+3 ) = p == selection.last() ? _col_stk_v_selected_p : _col_stk_v_selected_s;
            ptr[6]             = _siz_stk_v_selected;
         }  break;
         case DFStrokesEditor::PICK_STROKE_RING_EDGE:
         {
            //float* ptr = egver.data() + p.geomIdx()*12 + 3;
            //Vec3f::as( ptr ) = _col_stk_e_selected;
            //ptr += 6;
            //Vec3f::as( ptr ) = _col_stk_e_selected;
         }  break;
         default: ;
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
RCP<Gfx::DepthState> _failDepth   = new Gfx::DepthState();
RCP<Gfx::DepthState> _defDepth    = new Gfx::DepthState();
RCP<Gfx::AlphaState> _defBlending = new Gfx::AlphaState();


// VM

//------------------------------------------------------------------------------
//!
enum
{
   ID_RINGS,
   ID_EDGES,
   ID_STROKES,
   ID_STROKES_POS,
   ID_CENTER_POS,
   ID_EDGE_POS,
   ID_CORNER0,
   ID_CORNER1,
   ID_CORNER2,
   ID_CORNER3,
   ID_FLAGS,
};


ConstString  _strokesName;

//------------------------------------------------------------------------------
//!
RCP<DFNode> strokesVM( VMState* vm, int idx )
{
   RCP<DFStrokesNode> node = new DFStrokesNode();

   DFStrokes* strokes = node->strokes();

   // Read one stroke.
   for( int s = 1; VM::geti( vm, -1, s ); ++s )
   {
      DFStrokes::Stroke& st = strokes->stroke( strokes->addStroke() );

      // Vertices.
      if( VM::get( vm, -1, "vertices" ) )
      {
         // Read one vertex.
         for( int i = 1; VM::geti( vm, -1, i ); ++i )
         {
            DFStrokes::Vertex& v = strokes->vertex( st, strokes->addVertex( st ) );
            VM::get( vm, -1, "ref", v._ref );
            VM::get( vm, -1, "s", v._subdivisions );
            VM::get( vm, -1, "c", v._creases );
            // Read all 4 corners.
            for( int c = 1; c <= 4; ++c )
            {
               if( VM::geti( vm, -1, c ) )
               {
                  v._corners[c-1] = VM::toVec3f( vm, -1 );
                  VM::pop(vm);
               }
            }
            VM::pop(vm);
         }
         VM::pop(vm);
      }

      // Segments.
      if( VM::get( vm, -1, "segments" ) )
      {
         // Reading segments.
         for( int i = 1; VM::geti( vm, -1, i ); ++i )
         {
            DFStrokes::Segment& seg = strokes->segment( st, strokes->addSegment( st ) );
            // Number of partition.
            if( VM::geti( vm, -1, 1 ) )
            {
               seg._partition = VM::toUInt( vm, -1 );
               VM::pop(vm);
            }
            VM::get( vm, -1, "s", seg._subdivisions );
            VM::get( vm, -1, "c", seg._creases );
            VM::pop(vm);
         }
         VM::pop(vm);
      }

      VM::pop(vm);
   }

   // Links.
   if( VM::get( vm, idx, "links" ) )
   {
      for( int i = 1; VM::geti( vm, -1, i ); ++i )
      {
         DFStrokes::Link& l = strokes->link( strokes->addLink() );
         // Vertex ID.
         if( VM::geti( vm, -1, 1 ) )
         {
            l._v = VM::toUInt( vm, -1 );
            VM::pop(vm);
         }
         // stroke ID.
         if( VM::geti( vm, -1, 1 ) )
         {
            l._stroke = VM::toUInt( vm, -1 );
            VM::pop(vm);
         }
         // Segment ID.
         if( VM::geti( vm, -1, 1 ) )
         {
            l._segment = VM::toUInt( vm, -1 );
            VM::pop(vm);
         }
         VM::pop(vm);
      }
      VM::pop(vm);
   }

   return node;
}

UNNAMESPACE_END


NAMESPACE_BEGIN

//-----------------------------------------------------------------------------
//!
void initializeStrokes()
{
   _strokesName = "strokes";

   DFNodeSpec::registerNode(
      DFSocket::STROKES,
      _strokesName, strokesVM,
      "Strokes", "A strokes based geometry.",
      "image/ui/dfnode/strokes"
   );
}

//-----------------------------------------------------------------------------
//!
void terminateStrokes()
{
   _strokesName = ConstString();
}


/*==============================================================================
   Private Class Definitions
==============================================================================*/

/*==============================================================================
   CLASS DFStrokesRenderable
==============================================================================*/

class DFStrokesRenderable:
   public Renderable
{
public:

   /*----- methods -----*/

   DFStrokesRenderable( DFStrokesEditor* e );
   virtual void render( Gfx::Pass&, const Viewport& ) const;

   void update();

   MeshGeometry*   edgeMesh() { return _meshEdge.ptr(); }
   MeshGeometry* vertexMesh() { return _meshVert.ptr(); }

   bool pick( const Viewport&, const Vec2f& pos, float r, DFStrokesEditor::Pick& );

   void computeGeomIdx( DFStrokesEditor::Pick& );

protected:

   friend class DFStrokesEditor::Pick;

   /*----- methods -----*/

   void convertVertexPick( uint32_t idx, DFStrokesEditor::Pick& dst );
   void convertEdgePick( uint32_t idx, DFStrokesEditor::Pick& dst );

   /*----- data members -----*/

   DFStrokesEditor*       _editor;
   RCP<MeshGeometry>      _meshEdge;
   RCP<MeshGeometry>      _meshVert;
   RCP<Gfx::Program>      _progEdge;
   RCP<Gfx::Program>      _progVert;
   RCP<Gfx::SamplerList>  _texVert;
   RCP<Gfx::ConstantList> _constFront;
   RCP<Gfx::ConstantList> _constBack;
}; //class DFStrokesRenderable

//------------------------------------------------------------------------------
//!
DFStrokesRenderable::DFStrokesRenderable( DFStrokesEditor* e ):
   _editor( e )
{
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
DFStrokesRenderable::update()
{
   switch( _editor->mode() )
   {
      case DFStrokesEditor::MODE_STROKES: updateStrokesManip( edgeMesh(), vertexMesh(), _editor->strokes(), _editor->selection() );
         break;
      case DFStrokesEditor::MODE_RINGS:   updateRingsManip( edgeMesh(), vertexMesh(), _editor->strokes(), _editor->selection() );
         break;
      case DFStrokesEditor::MODE_EDGES:   updateEdgesManip( edgeMesh(), vertexMesh(), _editor->strokes(), _editor->selection() );
         break;
      default:;
   }
}

//------------------------------------------------------------------------------
//!
void
DFStrokesRenderable::render( Gfx::Pass& pass, const Viewport& ) const
{
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

//------------------------------------------------------------------------------
//!
bool
DFStrokesRenderable::pick( const Viewport& vp, const Vec2f& pos, float r, DFStrokesEditor::Pick& hit )
{
   uint32_t idx;
   // Vertices
   if( vertexMesh()->pick( vp, pos, r, idx ) )
   {
      convertVertexPick( idx, hit );
      return true;
   }
   // Edges
   if( _editor->mode() == DFStrokesEditor::MODE_EDGES && edgeMesh()->pick( vp, pos, r, idx ) )
   {
      convertEdgePick( idx, hit );
      return true;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
void
DFStrokesRenderable::computeGeomIdx( DFStrokesEditor::Pick& pick )
{
   if( pick.type() == DFStrokesEditor::PICK_STROKE_VERTEX )
   {
      const DFStrokes& strokes = *_editor->strokes();
      uint32_t idx = 0;
      for( uint32_t i = 0; i < pick.strokeIdx(); ++i )
      {
         idx += uint32_t(strokes.stroke(i)._vertices.size());
      }
      idx += pick.primaryIdx();
      pick.geomIdx( idx );
   }
}

//------------------------------------------------------------------------------
//!
void
DFStrokesRenderable::convertVertexPick( uint32_t idx, DFStrokesEditor::Pick& dst )
{
   dst.geomIdx( idx );

   uint16_t sec;
   if( _editor->mode() == DFStrokesEditor::MODE_EDGES )
   {
      sec = idx%4;  // Compute the ring vertex index (0-3) already.
      idx = idx/4;  // Each ring has 4 vertices per stroke vertex.
   }

   auto& strokes = _editor->strokes()->strokes();

   for( uint32_t si = 0; si < strokes.size(); ++si )
   {
      if( idx < strokes[si]._vertices.size() )
      {
         switch( _editor->mode() )
         {
            case DFStrokesEditor::MODE_STROKES:
               dst.setStrokeVertex( si, idx );
               break;
            case DFStrokesEditor::MODE_RINGS:
               dst.setStrokeVertex( si, idx );
               break;
            case DFStrokesEditor::MODE_EDGES:
               dst.setStrokeRingVertex( si, idx, sec );
               break;
            default:
               CHECK( false );
               break;
         }
         return;
      }
      idx -= uint32_t(strokes[si]._vertices.size());
   }

   CHECK( false );
}

//------------------------------------------------------------------------------
//!
void
DFStrokesRenderable::convertEdgePick( uint32_t idx, DFStrokesEditor::Pick& hit )
{
   hit.geomIdx( idx );

   auto& strokes = _editor->strokes()->strokes();

   for( uint32_t si = 0; si < strokes.size(); ++si )
   {
      auto& vertices = strokes[si]._vertices;
      auto& segments = strokes[si]._segments;
      uint32_t n = uint32_t(vertices.size()*4 + segments.size()*4);
      if( idx < n )
      {
         if( (idx&4) == 0 )
         {
            // Even is a ring.
            hit.setStrokeRingEdge( si, idx/8, idx%4 );
         }
         else
         {
            // Odd is a segment.
            hit.setStrokeSegmentEdge( si, idx/8, idx%4 );
         }
         return;
      }
      idx -= n;
   }

   CHECK( false );
}

/*==============================================================================
   CLASS DFStrokesManipulator
==============================================================================*/

class DFStrokesManipulator:
   public Manipulator
{
public:

   /*----- methods -----*/

   DFStrokesManipulator( DFStrokesEditor* e, DFStrokesRenderable* r, ManipulatorGroup* g );

   inline DFStrokesEditor*  editor() { return _editor; }

   virtual Renderable* renderable() const;

   // Events.
   virtual bool onKeyPress( const Event& ev );
   virtual bool onPointerPress( const Event& ev );
   virtual bool onPointerRelease( const Event& ev );
   virtual bool onPointerMove( const Event& ev );

protected:

   /*----- methods -----*/

   bool handleManipulatorPick( const Vec2f& );

   void refManipModified();
   void updateRefManip();

   /*----- data members -----*/

   DFStrokesEditor*          _editor;
   RCP<DFStrokesRenderable>  _renderable;
   RCP<RefManipulator>       _refManip;
   ManipulatorGroup*         _parent;
   bool                      _clearSel;
   DFStrokesEditor::Pick     _hit;
   Planef                    _plane;
}; //class DFStrokesManipulator

//------------------------------------------------------------------------------
//!
DFStrokesManipulator::DFStrokesManipulator( DFStrokesEditor* e, DFStrokesRenderable* r, ManipulatorGroup* g ):
   _editor( e ), _renderable( r ), _parent( g )
{
   _hit.setInvalid();
   _refManip = new RefManipulator( new RefRenderable() );
   _refManip->refRenderable()->worldSpace( false );
   _refManip->refRenderable()->addOnModify( makeDelegate( this, &DFStrokesManipulator::refManipModified ) );
}

//------------------------------------------------------------------------------
//!
Renderable*
DFStrokesManipulator::renderable() const
{
   return _renderable.ptr();
}

//------------------------------------------------------------------------------
//!
bool
DFStrokesManipulator::handleManipulatorPick( const Vec2f& pos )
{
   if( _renderable->pick( *viewport(), pos, 10.0f, _hit ) )
   {
      switch( _hit.type() )
      {
         // Vertices.
         case DFStrokesEditor::PICK_STROKE_VERTEX:
         case DFStrokesEditor::PICK_STROKE_RING_VERTEX:
         {
            // Update selection.
            if( Core::isKeyPressed(Key::CTRL) )
            {
               if( !_editor->toggleSelection( _hit ) ) _hit.setInvalid();
            }
            else if( !_editor->selection().has( _hit ) ) _editor->setSelection( _hit );
            // Find intersection plane.
            Vec3f n = alignedZ( camera()->orientation() );
            Vec3f p = _renderable->vertexMesh()->position( _hit.geomIdx() );
            _plane  = Planef( n, p );
            updateRefManip();
            return true;
         }  break;
         // Edges.
         case DFStrokesEditor::PICK_STROKE_RING_EDGE:
         case DFStrokesEditor::PICK_STROKE_SEGMENT_EDGE:
         {
            _editor->changeCrease( _hit );
            return true;
         }  break;
         default:
            break;
      }
   }
   if( Core::isKeyPressed(Key::ALT) && pick( _editor->strokes(), *viewport(), pos, _hit ) )
   {
      StdErr << "We have picked a face: " << _hit.strokeIdx() << " " << _hit.primaryIdx() << " " << _hit.secondaryIdx() << ".\n";
      _editor->changeFace( _hit );
   }
   return false;
}

//------------------------------------------------------------------------------
//!
bool
DFStrokesManipulator::onKeyPress( const Event& ev )
{
   switch( ev.value() )
   {
      case Key::_1: _editor->mode( DFStrokesEditor::MODE_STROKES ); updateRefManip(); return true;
      case Key::_2: _editor->mode( DFStrokesEditor::MODE_RINGS );   updateRefManip(); return true;
      case Key::_3: _editor->mode( DFStrokesEditor::MODE_EDGES );   updateRefManip(); return true;
      case Key::BACKSPACE:
      case Key::DELETE: _editor->removeVertex();                    return true;
      default:;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
bool
DFStrokesManipulator::onPointerPress( const Event& ev )
{
   _clearSel = false;
   if( Core::isKeyPressed( Key::SHIFT ) )
   {
      auto& sel = _editor->selection();

      // Intersection ray and normal.
      Rayf ray( camera()->position(), viewport()->direction( ev.position() ) );
      float t = CGConstf::infinity();
      Vec3f n = alignedZ( camera()->orientation() );

      // Stroke creation.
      if( sel.empty() )
      {
         // Intersection plane.
         Planef plane( n );
         // New point.
         if( Intersector::trace( plane, ray, t ) )
         {
            Vec3f p = ray.point(t);
            _editor->createStroke( p );
         }
      }
      // Vertex/segment creation.
      else
      {
         DFStrokesEditor::Pick pick = *sel.begin();
         if( pick.type() != DFStrokesEditor::PICK_STROKE_VERTEX ) return true;

         // Intersection plane.
         Vec3f p = _renderable->vertexMesh()->position( pick.geomIdx() );
         Planef plane( n, p );
         // New point.
         if( Intersector::trace( plane, ray, t ) )
         {
            Vec3f p = ray.point(t);
            _editor->createVertex( p );
         }
      }
      return true;
   }
   // Manipulator picking.
   bool handled = handleManipulatorPick( ev.position() );
   _clearSel    = !handled;
   return handled;
}

//------------------------------------------------------------------------------
//!
bool
DFStrokesManipulator::onPointerRelease( const Event& ev )
{
   if( _clearSel && ev.pointer().withinPress(5) )
   {
      _clearSel = false;
      _editor->clearSelection();
   }

   updateRefManip();

   _hit.setInvalid();

   return false;
}

//------------------------------------------------------------------------------
//!
bool
DFStrokesManipulator::onPointerMove( const Event& ev )
{
   switch( _hit.type() )
   {
      case DFStrokesEditor::PICK_STROKE_VERTEX:
      case DFStrokesEditor::PICK_STROKE_RING_VERTEX:
      {
         if( _editor->mode() == DFStrokesEditor::MODE_RINGS && Core::isKeyPressed(Key::ALT) )
         {
            const Pointer& p = ev.pointer();
            Vec2f pressDelta = p.position() - p.pressPosition();
            uint        axis = pressDelta.maxComponent();
            float f = (32.0f + p.deltaPosition()(axis))/32.0f;
            _editor->scaleRing( f );
         }
         else
         {
            Rayf ray( camera()->position(), viewport()->direction( ev.position() ) );
            float t = CGConstf::infinity();
            if( Intersector::trace( _plane, ray, t ) )
            {
               Vec3f p = ray.point(t);
               _editor->moveVertex( p );
               updateRefManip();
            }
         }
         return true;
      }  break;
      default:
         break;
   }

   return false;
}

//------------------------------------------------------------------------------
//!
void
DFStrokesManipulator::refManipModified()
{
   const Reff& newRef = _refManip->refRenderable()->referential();
   switch( _editor->selection().last().type() )
   {
      case DFStrokesEditor::PICK_STROKE_VERTEX:
         _editor->moveRing( newRef );
         break;
      case DFStrokesEditor::PICK_STROKE_RING_VERTEX:
         _editor->moveVertex( newRef.position() );
         break;
      default:;
   }
}

//------------------------------------------------------------------------------
//!
void
DFStrokesManipulator::updateRefManip()
{
   auto& sel = _editor->selection();
   if( sel.numEntries() == 1 )
   {
      bool showRef = false;

      const DFStrokesEditor::Pick& p = sel.last();
      switch( p.type() )
      {
         case DFStrokesEditor::PICK_STROKE_VERTEX:
         {
            const DFStrokes& strk = *(_editor->strokes());
            const DFStrokes::Stroke& sc = strk.stroke( p.strokeIdx() );
            const DFStrokes::Vertex& vc = strk.vertex( sc, p.primaryIdx() );
            _refManip->refRenderable()->update( vc._ref );
            showRef = true;
         }  break;
         case DFStrokesEditor::PICK_STROKE_RING_VERTEX:
         {
            const DFStrokes& strk = *(_editor->strokes());
            const DFStrokes::Stroke& sc = strk.stroke( p.strokeIdx() );
            const DFStrokes::Vertex& vc = strk.vertex( sc, p.primaryIdx() );
            Reff ref = vc._ref;
            ref.translateLocal( vc._corners[p.secondaryIdx()] );
            _refManip->refRenderable()->update( ref );
            showRef = true;
         }  break;
         default:;
      }

      if( showRef && _refManip->viewport() == nullptr )
      {
         _parent->addBack( _refManip.ptr() );
      }
   }
   else
   {
      if( _refManip->viewport() != nullptr )
      {
         _parent->remove( _refManip.ptr() );
      }
   }
}

/*==============================================================================
   CLASS DFStrokesNode
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFStrokesNode::DFStrokesNode()
{
   _output.delegate( makeDelegate( this, &DFStrokesNode::process ) );
   _strokes = new DFStrokes();
}

//------------------------------------------------------------------------------
//!
const ConstString&
DFStrokesNode::name() const
{
   return _strokesName;
}

//------------------------------------------------------------------------------
//!
uint
DFStrokesNode::numInputs() const
{
   return 0;
}

//------------------------------------------------------------------------------
//!
DFOutput*
DFStrokesNode::output()
{
   return &_output;
}

//------------------------------------------------------------------------------
//!
DFInput*
DFStrokesNode::input( uint )
{
   return nullptr;
}

//------------------------------------------------------------------------------
//!
RCP<DFStrokes>
DFStrokesNode::process()
{
   return _strokes;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeEditor>
DFStrokesNode::edit()
{
   if( _editor.isNull() )  _editor = new DFStrokesEditor( this );
   return _editor;
}

//-----------------------------------------------------------------------------
//!
bool
DFStrokesNode::dumpCustom( TextStream& os, StreamIndent& indent ) const
{
   // Strokes.
   auto& strokes = _strokes->strokes();
   for( auto s = strokes.begin(); s != strokes.end(); ++s )
   {
      // Begin strokes.
      os << indent << "{" << nl;
      ++indent;

      // Begin vertices.
      os << indent << "vertices={" << nl;
      ++indent;

      // Vertices.
      auto& vertices = _strokes->vertices( *s );
      for( auto v = vertices.begin(); v != vertices.end(); ++v )
      {
         os << indent << "{"
                   <<  " " << "ref=" << VMFmt(v->_ref)
                   << ", " << "s=" << v->_subdivisions
                   << ", " << "c=" << String().format("0x%04x", v->_creases)
                   << ", " << VMFmt(v->_corners[0])
                   << ", " << VMFmt(v->_corners[1])
                   << ", " << VMFmt(v->_corners[2])
                   << ", " << VMFmt(v->_corners[3])
                   << " },"
                   << nl;
      }

      // End vertices.
      --indent;
      os << indent << "}," << nl;

      // Begin segments.
      os << indent << "segments={" << nl;
      ++indent;

      // Segments.
      os << indent;
      for( auto seg = s->_segments.begin(); seg != s->_segments.end(); ++seg )
      {
         os << "{"
            << seg->_partition
            << ",c=" << String().format("0x%02x", seg->_creases)
            << ",s=" << seg->_subdivisions
            << "}, ";
      }
      os << nl;

      // End segments.
      --indent;
      os << indent << "}," << nl;

      // End strokes.
      --indent;
      os << indent << "}," << nl;
   }

   // Links.
   uint n = _strokes->numLinks();
   if( n > 0 )
   {
      os << indent << "links={" << nl;
      ++indent;
      for( uint i = 0; i < n; ++i )
      {
         const DFStrokes::Link& link = _strokes->link(i);
         os << indent << "{" << link._v << "," << link._stroke << "," << link._segment << "}," << nl;
      }
      --indent;
      os << indent << "}," << nl;
   }

   return os.ok();
}


/*==============================================================================
   CLASS DFStrokesEditor
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFStrokesEditor::DFStrokesEditor( DFStrokesNode* n ):
   _node( n ),
   _mode( MODE_STROKES )
{
}

//------------------------------------------------------------------------------
//!
DFStrokesEditor::~DFStrokesEditor()
{
}

//------------------------------------------------------------------------------
//!
RCP<Manipulator>
DFStrokesEditor::manipulator()
{
   if( _renderable.isNull() ) _renderable = new DFStrokesRenderable( this );
   RCP<ManipulatorGroup> g = new ManipulatorGroup();
   g->addBack( new DFStrokesManipulator( this, _renderable.ptr(), g.ptr() ) );
   return g;
   //return new DFStrokesManipulator( this, _renderable.ptr(), g.ptr() );
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrList>
DFStrokesEditor::attributes() const
{
   RCP<DFNodeAttrList> atts = new DFNodeAttrList();

   // Strokes UI.
   RCP<DFNodeAttrList> strokes = new DFNodeAttrList();
   strokes->add( DFNodeAttr( "XYZ", ID_STROKES_POS, "Position" ) );
   atts->add( DFNodeAttr( ID_STROKES, "", strokes.ptr() ) );

   // Ring UI.
   RCP<DFNodeAttrList> rings = new DFNodeAttrList();
   rings->add( DFNodeAttr( "XYZ", ID_CENTER_POS, "Position" ) );
   rings->add( DFNodeAttr( "XYZ", ID_CORNER0, "Corner0" ) );
   rings->add( DFNodeAttr( "XYZ", ID_CORNER1, "Corner1" ) );
   rings->add( DFNodeAttr( "XYZ", ID_CORNER2, "Corner2" ) );
   rings->add( DFNodeAttr( "XYZ", ID_CORNER3, "Corner3" ) );
   rings->add( DFNodeAttr( "INT", ID_FLAGS  , "Flags"   ) );
   atts->add( DFNodeAttr( ID_RINGS, "", rings.ptr() ) );

   // Edge UI.
   RCP<DFNodeAttrList> edges = new DFNodeAttrList();
   edges->add( DFNodeAttr( "XYZ", ID_EDGE_POS, "Position" ) );
   atts->add( DFNodeAttr( ID_EDGES, "", edges.ptr() ) );

   return atts;
}

//------------------------------------------------------------------------------
//!
RCP<DFNodeAttrStates>
DFStrokesEditor::attributesStates() const
{
   const DFStrokes* strokesGeom = strokes();
   RCP<DFNodeAttrStates> states = new DFNodeAttrStates();
   const Pick& pick             = _selection.last();

   // TODO: Visibility info could be compressed by sending a table with
   // only the visible groups.

   switch( _mode )
   {
      case DFStrokesEditor::MODE_STROKES:
         states->set( ID_STROKES, true );
         states->set( ID_RINGS, false );
         states->set( ID_EDGES, false );
         if( pick.type() == PICK_STROKE_VERTEX )
         {
            const DFStrokes::Stroke& sc = strokesGeom->stroke( pick.strokeIdx() );
            const DFStrokes::Vertex& vc = strokesGeom->vertex( sc, pick.primaryIdx() );
            states->set( ID_STROKES_POS, vc._ref.position() );
         }
         else
            // Invalid...
            states->set( ID_STROKES_POS, Vec3f(0.0f) );
         break;
      case DFStrokesEditor::MODE_RINGS:
         states->set( ID_STROKES, false );
         states->set( ID_RINGS, true );
         states->set( ID_EDGES, false );
         if( pick.type() == PICK_STROKE_VERTEX )
         {
            const DFStrokes::Stroke& sc = strokesGeom->stroke( pick.strokeIdx() );
            const DFStrokes::Vertex& vc = strokesGeom->vertex( sc, pick.primaryIdx() );
            states->set( ID_CENTER_POS, vc._ref.position() );
            states->set( ID_CORNER0, vc._corners[0] );
            states->set( ID_CORNER1, vc._corners[1] );
            states->set( ID_CORNER2, vc._corners[2] );
            states->set( ID_CORNER3, vc._corners[3] );
            states->set( ID_FLAGS, float(vc._flags) );
         }
         else
         {
            // Invalid...
            states->set( ID_CENTER_POS, Vec3f(0.0f) );
            states->set( ID_CORNER0, Vec3f(0.0f) );
            states->set( ID_CORNER1, Vec3f(0.0f) );
            states->set( ID_CORNER2, Vec3f(0.0f) );
            states->set( ID_CORNER3, Vec3f(0.0f) );
            states->set( ID_FLAGS, 0.0f );
         }
         break;
      case DFStrokesEditor::MODE_EDGES:
         states->set( ID_STROKES, false );
         states->set( ID_RINGS, false );
         states->set( ID_EDGES, true );
         if( pick.type() == PICK_STROKE_RING_VERTEX )
         {
            const DFStrokes::Stroke& sc = strokesGeom->stroke( pick.strokeIdx() );
            const DFStrokes::Vertex& vc = strokesGeom->vertex( sc, pick.primaryIdx() );
            Reff ref                    = strokesGeom->referential( sc, pick.primaryIdx() );
            states->set( ID_EDGE_POS, ref.toMatrix() * vc._corners[pick.secondaryIdx()] );
         }
         else
            // Invalid...
            states->set( ID_EDGE_POS, Vec3f(0.0f) );
         break;
      default:
         break;
   }
   return states;
}

//------------------------------------------------------------------------------
//!
void DFStrokesEditor::updateAttributes( const DFNodeAttrStates& attribs )
{
   for( auto cur = attribs.begin(); cur != attribs.end(); ++cur )
   {
      const Pick& pick = _selection.last();
      PickType selType = pick.type();

      switch( cur->_id )
      {
         case ID_STROKES_POS:
         {
            if( selType == PICK_STROKE_VERTEX )
               moveVertex( (*cur)._value.getVec3() );
         }  break;
         case ID_CENTER_POS:
         {
            if( selType == PICK_STROKE_VERTEX )
               moveVertex( (*cur)._value.getVec3() );
         }  break;
         case ID_EDGE_POS:
         {
            if( selType == PICK_STROKE_RING_VERTEX )
               moveVertex( (*cur)._value.getVec3() );
         }  break;
         case ID_CORNER0:
         case ID_CORNER1:
         case ID_CORNER2:
         case ID_CORNER3:
         {
            Pick p;
            p.setStrokeRingVertex( pick.strokeIdx(), pick.primaryIdx(), cur->_id-ID_CORNER0 );
            if( selType == PICK_STROKE_VERTEX )
               moveCorner( p, (*cur)._value.getVec3() );
         }  break;
         case ID_FLAGS:
         {
            if( selType == PICK_STROKE_VERTEX )
            {
               DFStrokes* strokesGeom = strokes();
               DFStrokes::Stroke& sc = strokesGeom->stroke( pick.strokeIdx() );
               DFStrokes::Vertex& vc = strokesGeom->vertex( sc, pick.primaryIdx() );
               vc._flags = uint8_t( (*cur)._value.getFloat() );
               strokesGeom->invalidate();
               _node->graph()->invalidate( _node );
               updateUI();
               if( _renderable.isValid() ) _renderable->update();            }
         }
         default:;
      }
   }
}

//------------------------------------------------------------------------------
//!
void DFStrokesEditor::updateUI()
{
   _node->graph()->msg().modify( _node, attributesStates().ptr() );
}

//------------------------------------------------------------------------------
//!
void DFStrokesEditor::updateSelection()
{
   updateUI();
   if( _renderable.isValid() ) _renderable->update();
}

//------------------------------------------------------------------------------
//!
void DFStrokesEditor::updateAll()
{
   // Invalidate geometry.
   strokes()->invalidate();
   // Invalidate graph.
   _node->graph()->invalidate( _node );

   // Update the UI.
   updateUI();

   // Update manipulator geometry.
   if( _renderable.isValid() ) _renderable->update();
}

//------------------------------------------------------------------------------
//!
void DFStrokesEditor::mode( Mode m )
{
   if( m == _mode ) return;
   _mode = m;
   _selection.clear();
   if( _renderable.isValid() ) _renderable->update();
   updateUI();
}

//------------------------------------------------------------------------------
//!
void
DFStrokesEditor::changeFace( const Pick& hit )
{
   // Update geometry.
   DFStrokes* strokesGeom = strokes();
   DFStrokes::Stroke& s   = strokesGeom->stroke( hit.strokeIdx() );

   if( hit.type() == PICK_STROKE_FACE )
   {
      DFStrokes::Segment& seg = s._segments[hit.primaryIdx()];
      uint32_t shft           = hit.secondaryIdx()*4;
      uint32_t mask           = 0xf<<shft;
      uint32_t inc            = 0x1<<shft;

      if( (seg._subdivisions&mask) == mask )
         seg._subdivisions &= ~mask;
      else
         seg._subdivisions += inc;
   }
   else
   {
      DFStrokes::Vertex& v = s._vertices[hit.primaryIdx()];
      ++v._subdivisions;
      if( v._subdivisions > 0xf ) v._subdivisions = 0;
   }

   // Invalidate geometry.
   strokesGeom->invalidate();
   // Invalidate graph.
   _node->graph()->invalidate( _node );

   // Update manipulator geometry.
   if( _renderable.isValid() ) _renderable->update();
}

//------------------------------------------------------------------------------
//!
void
DFStrokesEditor::changeCrease( const Pick& hit )
{
   // Update geometry.
   DFStrokes* strokesGeom = strokes();
   DFStrokes::Stroke& s   = strokesGeom->stroke( hit.strokeIdx() );

   // Do we have a vertex edges?
   if( hit.type() == DFStrokesEditor::PICK_STROKE_RING_EDGE )
   {
      DFStrokes::Vertex& v = strokesGeom->vertex( s, hit.primaryIdx() );
      v._creases ^= 1 << hit.secondaryIdx();
   }
   // Do we have a face edge?
   else
   {
      DFStrokes::Segment& seg = strokesGeom->segment( s, hit.primaryIdx() );
      seg._creases ^= 1 << hit.secondaryIdx();
   }

   // Invalidate geometry.
   strokesGeom->invalidate();
   // Invalidate graph.
   _node->graph()->invalidate( _node );

   // Update manipulator geometry.
   if( _renderable.isValid() ) _renderable->update();
}

//------------------------------------------------------------------------------
//!
void
DFStrokesEditor::moveVertex( const Vec3f& p )
{
   if( _selection.empty() ) return;

   // Update geometry.
   DFStrokes* strokesGeom = strokes();
   Vec3f delta            = p - _renderable->vertexMesh()->position( _selection.last().geomIdx() );

   if( _selection.last().type() == PICK_STROKE_VERTEX )
   {
      for( auto pick = _selection.begin(); pick != _selection.end(); ++pick )
      {
         if( pick->type() != PICK_STROKE_VERTEX ) continue;
         DFStrokes::Stroke& sc = strokesGeom->stroke( pick->strokeIdx() );
         DFStrokes::Vertex& vc = strokesGeom->vertex( sc, pick->primaryIdx() );
         vc._ref.translate( delta );
      }
   }
   else
   {
      for( auto pick = _selection.begin(); pick != _selection.end(); ++pick )
      {
         if( pick->type() != PICK_STROKE_RING_VERTEX ) continue;
         DFStrokes::Stroke& sc = strokesGeom->stroke( pick->strokeIdx() );
         DFStrokes::Vertex& vc = strokesGeom->vertex( sc, pick->primaryIdx() );
         Reff ref              = strokesGeom->referential( sc, pick->primaryIdx() );
         vc._corners[pick->secondaryIdx()] += ref.orientation().getInversed()*delta;
      }
   }

   updateAll();
}

//------------------------------------------------------------------------------
//!
void
DFStrokesEditor::moveCorner( const Pick& hit, const Vec3f& p )
{
   // Update geometry.
   DFStrokes* strokesGeom = strokes();

   DFStrokes::Stroke& sc = strokesGeom->stroke( hit.strokeIdx() );
   DFStrokes::Vertex& vc = strokesGeom->vertex( sc, hit.primaryIdx() );

   vc._corners[hit.secondaryIdx()] = p;

   updateAll();
}

//------------------------------------------------------------------------------
//!
void
DFStrokesEditor::createVertex( const Vec3f& p )
{
   if( _selection.empty() )
   {
      createStroke( p );
      return;
   }

   // Update geometry.
   Pick pick               = _selection.last();
   DFStrokes* strokesGeom  = strokes();
   DFStrokes::Stroke& s    = strokesGeom->stroke( pick.strokeIdx() );
   uint vi                 = 0;

   // Add to the end of the stroke.
   if( pick.primaryIdx() == s._vertices.size()-1 )
   {
      vi                      = strokesGeom->addVertex( s );
      DFStrokes::Vertex& v    = strokesGeom->vertex( s, vi );
      uint ei                 = strokesGeom->addSegment( s );
      DFStrokes::Segment& seg = strokesGeom->segment( s, ei );
      if( vi > 0 )
      {
         // Copy previous end vertex.
         v = strokesGeom->vertex( s, vi-1 );
      }
      v._ref.position(p);
      if( ei > 0 )
      {
         // Copy previous end segment.
         seg = strokesGeom->segment( s, ei-1 );
      }
   }
   // Add to the start of the stroke.
   else if( pick.primaryIdx() == 0 )
   {
      vi                      = strokesGeom->insertVertex( s, 0 );
      DFStrokes::Vertex& v    = strokesGeom->vertex( s, 0 );
      DFStrokes::Segment& seg = strokesGeom->segment( s, strokesGeom->insertSegment( s, 0 ) );
      if( strokesGeom->vertices(s).size() > 1 )
      {
         v = strokesGeom->vertex( s, 1 );
      }
      v._ref.position(p);
      if( strokesGeom->segments(s).size() > 0 )
      {
         // Copy previous end segment.
         seg = strokesGeom->segment( s, 1 );
      }
   }
   else
   {
      // For now...
      return;
   }

   // Handle selection.
   Pick nPick;
   nPick.setStrokeVertex( pick.strokeIdx(), vi );
   _renderable->computeGeomIdx( nPick );
   _selection.set( nPick );

   updateAll();
}

//------------------------------------------------------------------------------
//!
void
DFStrokesEditor::moveRing( const Reff& r )
{
   // Update geometry.
   DFStrokes* strokesGeom = strokes();

   auto& sel = selection();
   if( sel.last().type() == PICK_STROKE_VERTEX )
   {
      for( auto pick = sel.begin(); pick != sel.end(); ++pick )
      {
         if( pick->type() != PICK_STROKE_VERTEX ) continue;
         DFStrokes::Stroke& sc = strokesGeom->stroke( pick->strokeIdx() );
         DFStrokes::Vertex& vc = strokesGeom->vertex( sc, pick->primaryIdx() );
         vc._ref = r;
      }
   }

   updateAll();
}

//------------------------------------------------------------------------------
//!
void
DFStrokesEditor::createStroke( const Vec3f& p )
{
   // Update geometry.
   DFStrokes* strokesGeom  = strokes();
   DFStrokes::Stroke& s    = strokesGeom->stroke( strokesGeom->addStroke() );
   DFStrokes::Vertex& v    = strokesGeom->vertex( s, strokesGeom->addVertex( s ) );
   v._ref.position(p);

   // Handle selection.
   _selection.clear();
   Pick nPick;
   nPick.setStrokeVertex( strokesGeom->numStrokes()-1, 0 );
   _renderable->computeGeomIdx( nPick );
   _selection.add( nPick );

   updateAll();
}

//------------------------------------------------------------------------------
//!
void
DFStrokesEditor::scaleRing( float sf )
{
   if( _selection.empty() ) return;

   // Update geometry.
   DFStrokes* strokesGeom = strokes();

   if( _selection.last().type() == PICK_STROKE_VERTEX )
   {
      for( auto pick = _selection.begin(); pick != _selection.end(); ++pick )
      {
         if( pick->type() != PICK_STROKE_VERTEX ) continue;
         DFStrokes::Stroke& sc = strokesGeom->stroke( pick->strokeIdx() );
         DFStrokes::Vertex& vc = strokesGeom->vertex( sc, pick->primaryIdx() );
         for( uint i = 0; i < 4; ++i )
         {
            vc._corners[i] *= sf; // Relative to _ref.position() already.
         }
      }
   }

   updateAll();
}

//------------------------------------------------------------------------------
//!
void
DFStrokesEditor::removeVertex()
{
   // Deleting vertices/segments.
   DFStrokes* strokesGeom = strokes();
   for( auto pick = _selection.begin(); pick != _selection.end(); ++pick )
   {
      if( pick->type() != PICK_STROKE_VERTEX ) continue;
      strokesGeom->removeVertex( strokesGeom->stroke( pick->strokeIdx() ), pick->primaryIdx() );
      // For now only remove one element since other selection id could be invalid.
      break;
   }
   _selection.clear();

   updateAll();
}


/*==============================================================================
   CLASS DFStrokesEditor::Pick
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
DFStrokesEditor::Pick::print( TextStream& os ) const
{
   switch( _type )
   {
      case PICK_INVALID:
         os << "<invalid>";
         break;
      case PICK_STROKE_VERTEX:
         os << "Stroke(" << _idxStk << ")Vertex(" << _idxPri << ")";
         break;
      case PICK_STROKE_RING_VERTEX:
         os << "Stroke(" << _idxStk << ")Ring(" << _idxPri << ")Vertex(" << _idxSec << ")";
         break;
      case PICK_STROKE_RING_EDGE:
         os << "Stroke(" << _idxStk << ")Ring(" << _idxPri << ")Edge(" << _idxSec << ")";
         break;
      case PICK_STROKE_SEGMENT_EDGE:
         os << "Stroke(" << _idxStk << ")Segment(" << _idxPri << ")Edge(" << _idxSec << ")";
         break;
      default:
         os << "<unknown>" << nl;
         break;
   }
}

NAMESPACE_END
