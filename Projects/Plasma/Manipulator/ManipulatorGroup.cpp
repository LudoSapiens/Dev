/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Manipulator/ManipulatorGroup.h>
#include <Plasma/Renderable/RenderableGroup.h>

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN


UNNAMESPACE_END


/*==============================================================================
  CLASS ManipulatorGroup
==============================================================================*/

NAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
ManipulatorGroup::ManipulatorGroup()
{
   _renderable = new RenderableGroup();
}

//------------------------------------------------------------------------------
//!
ManipulatorGroup::~ManipulatorGroup()
{}

//------------------------------------------------------------------------------
//!
void
ManipulatorGroup::render( const RCP<Gfx::RenderNode>& rn )
{
   for( auto m = _manips.begin(); m != _manips.end(); ++m )
   {
      (*m)->render( rn );
   }
}

//------------------------------------------------------------------------------
//!
Renderable*
ManipulatorGroup::renderable() const
{
   return _renderable.ptr();
}

//------------------------------------------------------------------------------
//!
void
ManipulatorGroup::onCameraChange()
{
   for( auto m = _manips.begin(); m != _manips.end(); ++m )
   {
      (*m)->onCameraChange();
   }
}

//------------------------------------------------------------------------------
//!
void
ManipulatorGroup::onViewportChange()
{
   for( auto m = _manips.begin(); m != _manips.end(); ++m )
   {
      (*m)->onViewportChange();
   }
}

//------------------------------------------------------------------------------
//!
bool
ManipulatorGroup::onPointerPress( const Event& ev )
{
   for( auto m = _manips.begin(); m != _manips.end(); ++m )
   {
      if( (*m)->onPointerPress( ev ) ) return true;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
bool
ManipulatorGroup::onPointerRelease( const Event& ev )
{
   for( auto m = _manips.begin(); m != _manips.end(); ++m )
   {
      if( (*m)->onPointerRelease( ev ) ) return true;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
bool
ManipulatorGroup::onPointerMove( const Event& ev )
{
   for( auto m = _manips.begin(); m != _manips.end(); ++m )
   {
      if( (*m)->onPointerMove( ev ) ) return true;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
bool
ManipulatorGroup::onPointerCancel( const Event& ev )
{
   for( auto m = _manips.begin(); m != _manips.end(); ++m )
   {
      if( (*m)->onPointerCancel( ev ) ) return true;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
bool
ManipulatorGroup::onPointerScroll( const Event& ev )
{
   for( auto m = _manips.begin(); m != _manips.end(); ++m )
   {
      if( (*m)->onPointerScroll( ev ) ) return true;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
bool
ManipulatorGroup::onKeyPress( const Event& ev )
{
   for( auto m = _manips.begin(); m != _manips.end(); ++m )
   {
      if( (*m)->onKeyPress( ev ) ) return true;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
bool
ManipulatorGroup::onKeyRelease( const Event& ev )
{
   for( auto m = _manips.begin(); m != _manips.end(); ++m )
   {
      if( (*m)->onKeyRelease( ev ) ) return true;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
bool
ManipulatorGroup::onChar( const Event& ev )
{
   for( auto m = _manips.begin(); m != _manips.end(); ++m )
   {
      if( (*m)->onChar( ev ) ) return true;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
bool
ManipulatorGroup::onAccelerate( const Event& ev )
{
   for( auto m = _manips.begin(); m != _manips.end(); ++m )
   {
      if( (*m)->onAccelerate( ev ) ) return true;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
void
ManipulatorGroup::camera( Camera* c )
{
   Manipulator::camera( c );
   for( auto m = _manips.begin(); m != _manips.end(); ++m )
   {
      (*m)->camera( c );
   }
}

//------------------------------------------------------------------------------
//!
void
ManipulatorGroup::viewport( Viewport* v )
{
   Manipulator::viewport( v );
   for( auto m = _manips.begin(); m != _manips.end(); ++m )
   {
      (*m)->viewport( v );
   }
}

//------------------------------------------------------------------------------
//!
void
ManipulatorGroup::widget( Widget* w )
{
   Manipulator::widget( w );
   for( auto m = _manips.begin(); m != _manips.end(); ++m )
   {
      (*m)->widget( w );
   }
}

//------------------------------------------------------------------------------
//!
void
ManipulatorGroup::remove( Manipulator* m )
{
   m->viewport( nullptr );
   m->widget( nullptr );
   _manips.remove( m );

   Renderable* r = m->renderable();
   if( r )  _renderable->remove( r );
}

//------------------------------------------------------------------------------
//!
void
ManipulatorGroup::clear()
{
   for( auto m = _manips.begin(); m != _manips.end(); ++m )
   {
      (*m)->viewport( nullptr );
      (*m)->widget( nullptr );
   }

   _manips.clear();
   _renderable->clear();
}

//------------------------------------------------------------------------------
//!
void
ManipulatorGroup::addFront( Manipulator* m )
{
   m->widget( Manipulator::widget() );
   m->viewport( Manipulator::viewport() );
   _manips.insert( _manips.begin(), m );

   Renderable* r = m->renderable();
   if( r )  _renderable->addFront( r );
}

//------------------------------------------------------------------------------
//!
void
ManipulatorGroup::addBack( Manipulator* m )
{
   m->widget( Manipulator::widget() );
   m->viewport( Manipulator::viewport() );
   _manips.pushBack( m );

   Renderable* r = m->renderable();
   if( r )  _renderable->addBack( r );
}

//------------------------------------------------------------------------------
//!
void
ManipulatorGroup::add( Manipulator* m, int pos )
{
   m->widget( Manipulator::widget() );
   m->viewport( Manipulator::viewport() );
   _manips.insert( _manips.begin()+pos, m );

   Renderable* r = m->renderable();
   if( r )  _renderable->add( r, pos );
}

NAMESPACE_END
