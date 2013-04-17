/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Renderable/PlaneProjector.h>
#include <Plasma/World/Material.h>
#include <Plasma/World/Camera.h>
#include <Plasma/Intersector.h>

#include <Fusion/Core/Core.h>
#include <Fusion/Resource/ResManager.h>

#include <CGMath/Plane.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN


//------------------------------------------------------------------------------
//!
RCP<Gfx::DepthState> _zfail( new Gfx::DepthState() );
RCP<Gfx::DepthState> _zpass( new Gfx::DepthState() );

UNNAMESPACE_END

NAMESPACE_BEGIN

/*==============================================================================
   CLASS PlaneProjector
==============================================================================*/

PlaneProjector::PlaneProjector() :
   _cameraSpace( false ),
   _ref( Reff::identity() )
{
}

//------------------------------------------------------------------------------
//!
PlaneProjector::~PlaneProjector()
{
}

//------------------------------------------------------------------------------
//!
void
PlaneProjector::render( const RCP<Gfx::Pass>& pass ) const
{
   if( _camera.isNull() )
   {
      return;
   }

   // Create plane geometry the first time.
   if( _geom.isNull() )
   {
      const int elem_size = (3+2);  //XYZ, ST
      Vector<float>  vertices( 4*elem_size );
      Vector<ushort> indices( 6 ); // 2 Triangles == 1 Quad

      float st = 0.5f;
      float so = 1.0f;

      int bi = 0;
      vertices[bi+0] = -so;
      vertices[bi+1] = -so;
      vertices[bi+2] = 0.0f;
      vertices[bi+3] = 0.0f;
      vertices[bi+4] = 0.0f;
      bi += elem_size;
      vertices[bi+0] =  so;
      vertices[bi+1] = -so;
      vertices[bi+2] = 0.0f;
      vertices[bi+3] = st;
      vertices[bi+4] = 0.0f;
      bi += elem_size;
      vertices[bi+0] = so;
      vertices[bi+1] = so;
      vertices[bi+2] = 0.0f;
      vertices[bi+3] = st;
      vertices[bi+4] = st;
      bi += elem_size;
      vertices[bi+0] = -so;
      vertices[bi+1] = so;
      vertices[bi+2] = 0.0f;
      vertices[bi+3] = 0.0f;
      vertices[bi+4] = st;

      indices[0] = 0;
      indices[1] = 1;
      indices[2] = 2;
      indices[3] = 0;
      indices[4] = 2;
      indices[5] = 3;

      RCP<Gfx::IndexBuffer> indexBuffer = Core::gfx()->createBuffer(
         Gfx::INDEX_FMT_16, Gfx::BUFFER_FLAGS_NONE, indices.dataSize(), indices.data()
      );
      RCP<Gfx::VertexBuffer> vertexBuffer = Core::gfx()->createBuffer(
         Gfx::BUFFER_FLAGS_NONE, vertices.dataSize(), vertices.data()
      );
      vertexBuffer->addAttribute( Gfx::ATTRIB_TYPE_POSITION, Gfx::ATTRIB_FMT_32F_32F_32F, 0 );
      vertexBuffer->addAttribute( Gfx::ATTRIB_TYPE_TEXCOORD0, Gfx::ATTRIB_FMT_32F_32F, 12 );

      _geom = Core::gfx()->createGeometry( Gfx::PRIM_TRIANGLES );
      _geom->indexBuffer( indexBuffer );
      _geom->addBuffer( vertexBuffer );


      _visMaterial = new Material();
      _visMaterial->program( ResManager::getProgram( "shader/program/colorTex" ) );
      _visMaterial->setTexture( "colorTex", ResManager::getTexture( "image/checker2" ) );
      _visMaterial->setConstant( "color", Vec4f( 0.3f ).ptr() );

      _hiddenMaterial = new Material();
      _hiddenMaterial->program( ResManager::getProgram( "shader/program/colorTex" ) );
      _hiddenMaterial->setTexture( "colorTex", ResManager::getTexture( "image/checker2" ) );
      _hiddenMaterial->setConstant( "color", Vec4f( 0.05f ).ptr() );

      _zfail->depthTestFunc( Gfx::COMPARE_FUNC_GREATER );
      _zfail->depthWriting( false );
   }

   // Compute transformation matrix.
   Vec3f camZ = _camera->referential().orientation().getAxisZ();

   Reff ref = _ref;
   if( _cameraSpace )
   {
      ref.orientation( _camera->referential().orientation() );
   }

   Vec3f x, y, z;
   ref.orientation().getAxes( x, y, z );

   float dotx = CGM::abs( x.dot( camZ ) );
   float doty = CGM::abs( y.dot( camZ ) );
   float dotz = CGM::abs( z.dot( camZ ) );

   if( dotx > doty )
   {
      if( dotx > dotz )
      {
         ref.rotate( Vec3f( 0.0f, 1.0f, 0.0f ), CGConstf::pi_2() );
      }
   }
   else
   {
      if( doty > dotz )
      {
         ref.rotate( Vec3f( 1.0f, 0.0f, 0.0f ), CGConstf::pi_2() );
      }
   }

   _transform = ref.localToGlobal();

   // Draw geometry.
   _hiddenMaterial->apply( *pass );
   pass->setDepthState( _zfail );
   pass->setWorldMatrixPtr( _transform.ptr() );
   pass->execGeometry( _geom );
   pass->setDepthState( _zpass );
   _visMaterial->apply( *pass );
   pass->execGeometry( _geom );
}

//------------------------------------------------------------------------------
//!
Vec3f
PlaneProjector::project( const Vec2i& pos ) const
{
   // Compute Intersection plane.
   Vec3f normal;
   Vec3f camZ = _camera->referential().orientation().getAxisZ();

   Reff ref = _ref;
   if( _cameraSpace )
   {
      ref.orientation( _camera->referential().orientation() );
   }

   Vec3f x, y, z;
   ref.orientation().getAxes( x, y, z );

   float dotx = CGM::abs( x.dot( camZ ) );
   float doty = CGM::abs( y.dot( camZ ) );
   float dotz = CGM::abs( z.dot( camZ ) );

   if( dotx > doty )
   {
      if( dotx > dotz )
      {
         normal = x;
      }
      else
      {
         normal = z;
      }
   }
   else
   {
      if( doty > dotz )
      {
         normal = y;
      }
      else
      {
         normal = z;
      }
   }
   Planef plane( normal, _pick );

   // Intersect pointed ray with plane.
   Rayf ray( _camera->position(), _camera->direction( pos ) );
   float d = CGConstf::infinity();
   if( Intersector::trace( plane, ray, d ) )
   {
      return ray.point(d);
   }
   else
   {
      return _ref.position();
   }
}

NAMESPACE_END
