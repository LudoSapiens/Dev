/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Manipulator/Manipulator.h>
#include <Plasma/Renderable/Renderable.h>

#include <Fusion/VM/VMObjectPool.h>

#include <Base/ADT/StringMap.h>


/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

const char* _manip_meta = "Manipulator";


//------------------------------------------------------------------------------
//!
StringMap _attributes(
   ""
);

UNNAMESPACE_END


/*==============================================================================
  CLASS Manipulator
==============================================================================*/

NAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
Manipulator::Manipulator():
   _widget(0),
   _viewport(0)
{}

//------------------------------------------------------------------------------
//!
Manipulator::~Manipulator()
{}

//------------------------------------------------------------------------------
//!
void
Manipulator::render( const RCP<Gfx::RenderNode>& )
{}

//------------------------------------------------------------------------------
//!
Renderable*
Manipulator::renderable() const
{
   return nullptr;
}

//------------------------------------------------------------------------------
//!
void
Manipulator::onCameraChange()
{
}

//------------------------------------------------------------------------------
//!
void
Manipulator::onViewportChange()
{
}

//------------------------------------------------------------------------------
//!
bool
Manipulator::onPointerPress( const Event& )
{
   return false;
}

//------------------------------------------------------------------------------
//!
bool
Manipulator::onPointerRelease( const Event& )
{
   return false;
}

//------------------------------------------------------------------------------
//!
bool
Manipulator::onPointerMove( const Event& )
{
   return false;
}

//------------------------------------------------------------------------------
//!
bool
Manipulator::onPointerCancel( const Event& )
{
   return false;
}

//------------------------------------------------------------------------------
//!
bool
Manipulator::onPointerScroll( const Event& )
{
   return false;
}

//------------------------------------------------------------------------------
//!
bool
Manipulator::onKeyPress( const Event& )
{
   return false;
}

//------------------------------------------------------------------------------
//!
bool
Manipulator::onKeyRelease( const Event& )
{
   return false;
}

//------------------------------------------------------------------------------
//!
bool
Manipulator::onChar( const Event& )
{
   return false;
}

//------------------------------------------------------------------------------
//!
bool
Manipulator::onAccelerate( const Event& )
{
   return false;
}

//------------------------------------------------------------------------------
//!
void
Manipulator::camera( Camera* cam )
{
   _viewport->camera( cam );
   onCameraChange();
}

//------------------------------------------------------------------------------
//!
void
Manipulator::viewport( Viewport* viewport )
{
   _viewport = viewport;
   onViewportChange();
   onCameraChange();
}

//------------------------------------------------------------------------------
//!
void
Manipulator::widget( Widget* w )
{
   _widget = w;
}

//------------------------------------------------------------------------------
//!
void
Manipulator::initialize()
{
   VMObjectPool::registerObject(
      "UI",
      _manip_meta,
      stdCreateVM<Manipulator>,
      stdGetVM<Manipulator>,
      stdSetVM<Manipulator>
   );
}

//------------------------------------------------------------------------------
//!
const char*
Manipulator::meta() const
{
   return _manip_meta;
}

//------------------------------------------------------------------------------
//!
void
Manipulator::init( VMState* vm )
{
   if( VM::isTable( vm, -1 ) )
   {
      VM::push( vm );  // Start iterating at index 0 (nil).
      while( VM::next( vm, -2 ) )
      {
         performSet( vm );  // Forward assignments to performSet().
         VM::pop( vm, 1 );  // Pop the value, but keep the key.
      }

      // Creation-only parameters (if any).
   }
}

//------------------------------------------------------------------------------
//!
bool
Manipulator::performGet( VMState* /*vm*/ )
{
   //const char* attr = VM::toCString( vm, 2 );
   //switch( _attributes[attr] )
   //{
   //   default: break;
   //}
   return false;
}

//------------------------------------------------------------------------------
//!
bool
Manipulator::performSet( VMState* /*vm*/ )
{
   //const char* attr = VM::toCString( vm, 2 );
   //switch( _attributes[attr] )
   //{
   //   default: break;
   //}
   return false;
}

NAMESPACE_END
