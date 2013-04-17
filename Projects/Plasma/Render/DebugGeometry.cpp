/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Render/DebugGeometry.h>

#include <Plasma/Resource/ResManager.h>

#include <Fusion/Core/Core.h>

#include <Gfx/Mgr/Manager.h>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

RCP<Gfx::Program>  _points_prog;
RCP<Gfx::Program>  _lines_prog;
RCP<Gfx::Program>  _triangles_prog;

UNNAMESPACE_END

//-----------------------------------------------------------------------------
//!
void
DebugGeometry::initialize()
{
}

//-----------------------------------------------------------------------------
//!
void
DebugGeometry::terminate()
{
   _points_prog    = NULL;
   _lines_prog     = NULL;
   _triangles_prog = NULL;
}

//-----------------------------------------------------------------------------
//!
DebugGeometry::DebugGeometry()
{
}

//-----------------------------------------------------------------------------
//!
DebugGeometry::~DebugGeometry()
{
}

//-----------------------------------------------------------------------------
//!
void
DebugGeometry::clear()
{
   _points.clear();
   _points_rgeom = NULL;
   _lines.clear();
   _lines_rgeom = NULL;
   _triangles.clear();
   _triangles_rgeom = NULL;
}

//-----------------------------------------------------------------------------
//!
void
DebugGeometry::computeRenderableGeometry()
{
   if( !_points.empty() )
   {
      if( _points_prog.isNull() )  _points_prog = data( ResManager::getProgram( "shader/program/vertexColor" ) );

      //if( _points_rgeom == NULL )
      {
         _points_rgeom = Core::gfx()->createGeometry( Gfx::PRIM_POINTS );

         RCP<Gfx::VertexBuffer> vBuffer = Core::gfx()->createBuffer( Gfx::BUFFER_FLAGS_NONE, _points.dataSize(), _points.data() );
         vBuffer->addAttribute( Gfx::ATTRIB_TYPE_POSITION, Gfx::ATTRIB_FMT_32F_32F_32F    , vBuffer->strideInBytes() );
         vBuffer->addAttribute( Gfx::ATTRIB_TYPE_COLOR   , Gfx::ATTRIB_FMT_32F_32F_32F_32F, vBuffer->strideInBytes() );
         _points_rgeom->addBuffer( vBuffer );
      }
   }

   if( !_lines.empty() )
   {
      if( _lines_prog.isNull() )  _lines_prog = data( ResManager::getProgram( "shader/program/vertexColor" ) );

      //if( _lines_rgeom == NULL )
      {
         _lines_rgeom = Core::gfx()->createGeometry( Gfx::PRIM_LINES );

         RCP<Gfx::VertexBuffer> vBuffer = Core::gfx()->createBuffer( Gfx::BUFFER_FLAGS_NONE, _lines.dataSize(), _lines.data() );
         vBuffer->addAttribute( Gfx::ATTRIB_TYPE_POSITION, Gfx::ATTRIB_FMT_32F_32F_32F    , vBuffer->strideInBytes() );
         vBuffer->addAttribute( Gfx::ATTRIB_TYPE_COLOR   , Gfx::ATTRIB_FMT_32F_32F_32F_32F, vBuffer->strideInBytes() );
         _lines_rgeom->addBuffer( vBuffer );
      }
   }

   if( !_triangles.empty() )
   {
      if( _triangles_prog.isNull() )  _triangles_prog = data( ResManager::getProgram( "shader/program/vertexColor" ) );

      //if( _triangles_rgeom == NULL )
      {
         _triangles_rgeom = Core::gfx()->createGeometry( Gfx::PRIM_TRIANGLES );

         RCP<Gfx::VertexBuffer> vBuffer = Core::gfx()->createBuffer( Gfx::BUFFER_FLAGS_NONE, _triangles.dataSize(), _triangles.data() );
         vBuffer->addAttribute( Gfx::ATTRIB_TYPE_POSITION, Gfx::ATTRIB_FMT_32F_32F_32F    , vBuffer->strideInBytes() );
         vBuffer->addAttribute( Gfx::ATTRIB_TYPE_COLOR   , Gfx::ATTRIB_FMT_32F_32F_32F_32F, vBuffer->strideInBytes() );
         _triangles_rgeom->addBuffer( vBuffer );
      }
   }
}

//-----------------------------------------------------------------------------
//!
void
DebugGeometry::render( Gfx::Pass& pass )
{
   if( !_points.empty() )
   {
      pass.setProgram( _points_prog );
      pass.execRangeGeometry( _points_rgeom, NULL );
   }
   if( !_lines.empty() )
   {
      pass.setProgram( _lines_prog );
      pass.execRangeGeometry( _lines_rgeom, NULL );
   }
   if( !_triangles.empty() )
   {
      pass.setProgram( _triangles_prog );
      pass.execRangeGeometry( _triangles_rgeom, NULL );
   }
}
