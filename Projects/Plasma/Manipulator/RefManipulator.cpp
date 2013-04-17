/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Manipulator/RefManipulator.h>
#include <Plasma/Geometry/MeshGeometry.h>

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

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
RCP<Gfx::DepthState> _failDepth   = new Gfx::DepthState();
RCP<Gfx::DepthState> _defDepth    = new Gfx::DepthState();
RCP<Gfx::AlphaState> _defBlending = new Gfx::AlphaState();


Vec3f _col_axis_default   = Vec3f( 0.2f, 0.2f, 1.0f );
Vec3f _col_axis_selected  = Vec3f( 4.0f, 0.0f, 0.0f );

//------------------------------------------------------------------------------
//! TODO: move code elsewhere... same code is in file DFStrokesTool.cpp
float distancePtPt( const Vec2f& pt0, const Vec2f& pt1 )
{
   return sqrLength( pt0-pt1 );
}

//------------------------------------------------------------------------------
//!
float distancePtSegment( const Vec2f& pt, const Vec2f& v0, const Vec2f& v1 )
{
   Vec2f e10  = v1-v0;
   Vec2f ptv0 = pt-v0;
   float e    = dot( ptv0, e10 );
   if( e <= 0.0 ) return distancePtPt( pt, v0 );
   float f    = dot( e10, e10 );
   if( e >= f ) return distancePtPt( pt, v1 );
   float t   = e / f;
   float d   = dot( ptv0, ptv0 ) - e*t;
   return d;
}

UNNAMESPACE_END


NAMESPACE_BEGIN

/*==============================================================================
   CLASS RefRenderable
==============================================================================*/

//------------------------------------------------------------------------------
//!
RefRenderable::RefRenderable():
   _mode( NONE ), _worldSpace( true ), _ref( Reff::identity() )
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

   _prog = data( ResManager::getProgram( "shader/program/editor/l_standard" ) );

   // Geometry.
   _mesh = new MeshGeometry( MeshGeometry::LINES  );
   const int edgeAttribs[] = {
      MeshGeometry::POSITION,
      MeshGeometry::COLOR,
      0
   };
   _mesh->setAttributes( edgeAttribs );

   update();
}

//------------------------------------------------------------------------------
//!
void
RefRenderable::update( const Reff& r, bool cb )
{
   _ref = r;
   if( cb ) _callbacks.exec();
}

//------------------------------------------------------------------------------
//!
void
RefRenderable::update()
{
   // Filling the geometry.
   Vector<float> ver;
   Vector<uint32_t> indices;

   for( int i = 0; i < 3; ++i )
   {
      Vec3f pos(0.0f);
      pos(i) = 1.0f;
      Vec3f col = isAxisSelected(i) ? _col_axis_selected : _col_axis_default;
      pushBack( ver, -pos );
      pushBack( ver, col );
      pushBack( ver, pos );
      pushBack( ver, col );
      indices.pushBack( i*2+0 );
      indices.pushBack( i*2+1 );
   }

   // Updating the mesh.
   _mesh->clearPatches();
   _mesh->deallocate();

   _mesh->allocateIndices( uint(indices.size()) );
   _mesh->copyIndices( indices.data() );
   _mesh->allocateVertices( uint(ver.size())/6 );
   _mesh->copyAttributes( ver.data(), 6, 6, 0 );
   _mesh->addPatch( 0, uint(indices.size()) );

   _mesh->updateProperties();
   _mesh->invalidateRenderableGeometry();
}

//------------------------------------------------------------------------------
//!
void
RefRenderable::render( Gfx::Pass& pass, const Viewport& vp ) const
{
   // Behind pass.
   pass.setDepthState( _failDepth );
   pass.setAlphaState( _defBlending );
   pass.setConstants( _constBack );
   pass.setWorldMatrix( renderMatrix( vp ).ptr() );
   pass.setProgram( _prog );
   pass.setSamplers(0);
   _mesh->render( pass );

   // Front pass.
   pass.setDepthState( _defDepth );
   pass.setConstants( _constFront );
   _mesh->render( pass );
}


//------------------------------------------------------------------------------
//!
bool
RefRenderable::isAxisSelected( int i )
{
   return _mode == i+1;
}

//------------------------------------------------------------------------------
//!
float
RefRenderable::scalingFactor( const Viewport& vp ) const
{
   // Compute a scaling factor so that the renderable stays to a fixed size,
   // no matter its distance.
   float l = length( vp.camera()->position() - _ref.position() );
   return l*0.125f;
}

//------------------------------------------------------------------------------
//!
Mat4f
RefRenderable::renderMatrix( const Viewport& vp ) const
{
   Mat4f mtx = _worldSpace ? Mat4f::translation( _ref.position() ) : _ref.toMatrix();
   mtx      *= Mat4f::scaling( scalingFactor(vp) );

   return mtx;
}

//------------------------------------------------------------------------------
//!
Vec3f
RefRenderable::axis( int i )
{
   if( _worldSpace )
   {
      Vec3f a(0.0f);
      a(i)=1.0f;
      return a;
   }
   switch( i )
   {
      case 0: return _ref.orientation().getAxisX();
      case 1: return _ref.orientation().getAxisY();
      case 2: return _ref.orientation().getAxisZ();
      default : return _ref.orientation().getAxisX();
   }
}

//------------------------------------------------------------------------------
//!
Vec3f
RefRenderable::axis()
{
   return _mode == 0 ? Vec3f(1.0f,0.0f,0.0f) : axis( _mode - 1);
}

/*==============================================================================
   CLASS RefManipulator
==============================================================================*/

//------------------------------------------------------------------------------
//!
bool
RefManipulator::onPointerPress( const Event& ev )
{
   Vec2f pos = ev.position();

   // Compute distance from each axis.
   Viewport& vp  = *viewport();
   Mat4f m       = vp.cameraMatrix() * _renderable->renderMatrix( vp );
   int nearAxis  = -1;
   float minDist = 10.0f*10.0f;

   for( int a = 0; a < 3; ++a )
   {
      // Compute Axis.
      Vec3f p(0.0f);
      p(a) = 1.0f;
      // Compute axis extremeties.
      Vec3f p0   = m | (-p);
      Vec3f p1   = m | p;
      float dist = distancePtSegment( pos, p0(0,1), p1(0,1) );

      if( dist < minDist )
      {
         nearAxis = a;
         minDist  = dist;
         _axisDir = normalize( p1(0,1)-p0(0,1) );
      }
   }

   _renderable->mode( (RefRenderable::Mode)(nearAxis+1) );
   _renderable->update();
   _axis = _renderable->axis();

   // Doing nothing for now.
   _mode = -1;

   return false;
}

//------------------------------------------------------------------------------
//!
bool
RefManipulator::onPointerRelease( const Event& )
{
   _renderable->mode( RefRenderable::NONE );
   _renderable->update();

   return false;
}

//------------------------------------------------------------------------------
//!
bool
RefManipulator::onPointerMove( const Event& ev )
{
   // TODO: How to switch between world/object referential?
   if( _renderable->mode() == RefRenderable::NONE ) return false;

   Pointer& pointer = ev.pointer();

   // Figure out in which mode to interact (translate or rotation) the first
   // time the pointer breach the threshold.
   if( !pointer.withinPress( 10 ) && _mode == -1 )
   {
      Vec2f dir = normalize( pointer.position() - pointer.pressPosition() );
      if( CGM::abs( dot( _axisDir, dir ) ) > 0.7f )
         _mode = 0;
      else
         _mode = 1;
   }
   // Translation.
   if( _mode == 0 )
   {
      // TODO: better scaling factor.
      Viewport& vp = *viewport();
      float t      = dot( _axisDir, pointer.deltaPosition() );
      Reff ref = _renderable->referential();
      ref.translate( _axis*t*_renderable->scalingFactor(vp)*0.01f );
      _renderable->update( ref, true );
   }
   // Rotation.
   if( _mode == 1 )
   {
      // TODO: better rotation direction computation.
      Vec2f axisDir( _axisDir.y, -_axisDir.x );
      float t  = dot( axisDir, pointer.deltaPosition() );
      Reff ref = _renderable->referential();
      ref.rotate( _axis, t*0.04f );
      ref.orientation( ref.orientation().normalize() );
      _renderable->update( ref, true );
   }

   return true;
}

NAMESPACE_END
