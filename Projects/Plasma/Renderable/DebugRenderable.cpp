/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Renderable/DebugRenderable.h>
#include <Plasma/Resource/ResManager.h>
#include <Plasma/World/World.h>

#include <Fusion/Core/Core.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS DebugRenderable
==============================================================================*/

RCP<Gfx::DepthState> _noDepth;

//------------------------------------------------------------------------------
//!
DebugRenderable::DebugRenderable()
{
   _prog = data( ResManager::getProgram( "shader/program/vertexColor" ) );
   _geom = Core::gfx()->createGeometry( Gfx::PRIM_LINES );

   _vbuffer = Core::gfx()->createBuffer( Gfx::BUFFER_FLAGS_STREAMABLE, 0, 0 );
   _vbuffer->addAttribute( Gfx::ATTRIB_TYPE_POSITION,  Gfx::ATTRIB_FMT_32F_32F_32F, 0 );
   _vbuffer->addAttribute( Gfx::ATTRIB_TYPE_COLOR,     Gfx::ATTRIB_FMT_32F_32F_32F, 12 );
   _geom->addBuffer( _vbuffer );

   _ibuffer = Core::gfx()->createBuffer( Gfx::INDEX_FMT_16, Gfx::BUFFER_FLAGS_STREAMABLE, 0, 0 );
   _geom->indexBuffer( _ibuffer );

   _noDepth = new Gfx::DepthState();
   _noDepth->depthTesting( false );
   _noDepth->depthWriting( false );
}

//------------------------------------------------------------------------------
//!
DebugRenderable::~DebugRenderable()
{}

//------------------------------------------------------------------------------
//!
void
DebugRenderable::render( const RCP<Gfx::Pass>& pass ) const
{
   // Update geometry.
   ((DebugRenderable*)this)->addDebugGeometry();

   // Fill buffers.
   Core::gfx()->setData( _ibuffer, _idx.dataSize(), _idx.data() );
   Core::gfx()->setData( _vbuffer, _vertices.dataSize(), _vertices.data() );

   // Render.
   pass->setDepthState( _noDepth );
   pass->setProgram( _prog );
   pass->setWorldMatrixPtr( Gfx::Pass::identityMatrix() );
   pass->execGeometry( _geom );
}

//------------------------------------------------------------------------------
//!
void
DebugRenderable::addDebugGeometry()
{
   MotionWorld* mworld = world()->motionWorld();

   // Empty buffers.
   _idx.clear();
   _vertices.clear();
   uint id = 0;

   // Velocity vectors.
   Vec3f vcolor( 1.0f, 0.0f, 0.0f );
   for( uint i = 0; i < mworld->rigidBodies().size(); ++i )
   {
      RigidBody* body = mworld->rigidBodies()[i].ptr();

      const Vec3f& pos  = body->centerPosition();
      const Vec3f& lvel = body->linearVelocity();

      _vertices.pushBack( pos );
      _vertices.pushBack( vcolor );

      _vertices.pushBack( pos+lvel );
      _vertices.pushBack( vcolor );

      _idx.pushBack(id++);
      _idx.pushBack(id++);
   }
#if MOTION_BULLET
#  if defined(__GNUC__)
//#  warning FIXME
#  endif
#else
   // Contacts.
   Vec3f ncolor( 1.0f, 1.0f, 1.0f );
   Vec3f ccolor( 0.0f, 0.0f, 1.0f );
   for( uint i = 0; i < mworld->collisionConstraints().size(); ++i )
   {
      CollisionInfo::Contact*  contact = mworld->collisionConstraints()[i]._contact;

      const Vec3f& posA = contact->worldPositionA();
      const Vec3f& posB = contact->worldPositionB();
      const Vec3f& n    = contact->worldNormal();

      _vertices.pushBack( posA );
      _vertices.pushBack( ccolor );
      _vertices.pushBack( posB );
      _vertices.pushBack( ccolor );

      _idx.pushBack(id++);
      _idx.pushBack(id++);

      _vertices.pushBack( posB );
      _vertices.pushBack( ncolor );
      _vertices.pushBack( posB+n*0.2f );
      _vertices.pushBack( ncolor );

      _idx.pushBack(id++);
      _idx.pushBack(id++);
   }
#endif
}

NAMESPACE_END
