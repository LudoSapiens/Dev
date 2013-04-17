/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Particle/ParticleAnimator.h>

#include <Base/ADT/StringMap.h>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN


UNNAMESPACE_END

//------------------------------------------------------------------------------
//!
void
ParticleAnimator::init( VMState* vm )
{
   if( VM::isTable(vm, -1) )
   {
      VM::push(vm); // Start iterating at index 0 (nil).
      while( VM::next(vm, -2) )
      {
         performSet(vm);
         VM::pop(vm, 1); // Pop the value, keep the key.
      }
   }
}

//------------------------------------------------------------------------------
//!
bool
ParticleAnimator::performGet( VMState* /*vm*/ )
{
   return false;
}

//------------------------------------------------------------------------------
//!
bool
ParticleAnimator::performSet( VMState* /*vm*/ )
{
   return false;
}
