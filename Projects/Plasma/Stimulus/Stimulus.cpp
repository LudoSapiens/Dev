/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Stimulus/Stimulus.h>
#include <Plasma/World/Brain.h>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

//-----------------------------------------------------------------------------
//!
Map< ConstString, Stimulus::CtorFunc >  _ctorFuncs;

UNNAMESPACE_END


/*==============================================================================
  CLASS Stimulus
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
Stimulus::initialize()
{
}

//------------------------------------------------------------------------------
//!
void
Stimulus::terminate()
{
   _ctorFuncs.clear();
}

//-----------------------------------------------------------------------------
//!
bool
Stimulus::registerStimulus( const ConstString& type, Stimulus::CtorFunc ctorFunc )
{
   if( !_ctorFuncs.has(type) )
   {
      _ctorFuncs[type] = ctorFunc;
      return true;
   }
   else
   {
      CHECK( false );
      return false;
   }
}

//-----------------------------------------------------------------------------
//!
bool
Stimulus::unregisterStimulus( const ConstString& type )
{
   if( _ctorFuncs.has(type) )
   {
      _ctorFuncs.erase( type );
      return true;
   }
   else
   {
      CHECK( false );
      return false;
   }
}

//-----------------------------------------------------------------------------
//!
Stimulus*
Stimulus::create( const ConstString& type )
{
   const CtorFunc& func = _ctorFuncs[type];
   return func ? func() : nullptr;
}

//------------------------------------------------------------------------------
//!
Stimulus::Stimulus()
{
}

//------------------------------------------------------------------------------
//!
Stimulus::~Stimulus()
{
}

//------------------------------------------------------------------------------
//!
void
Stimulus::push( VMState* vm )
{
   // Note: If you change this, please make sure the SetOrder is still OK.
   VM::newTable( vm );

   VM::push( vm, "type" );
   VM::push( vm, type() );
   VM::set( vm, -3 );
}

//------------------------------------------------------------------------------
//!
bool
Stimulus::to( VMState* /*vm*/, int /*idx*/ )
{
   return true;
}
