/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Particle/ParticleGenerator.h>

#include <Base/ADT/StringMap.h>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN


//------------------------------------------------------------------------------
//!
enum
{
   ATTRIB_DONE,
   ATTRIB_ENERGY,
   ATTRIB_USE_ENERGY,
   NUM_ATTRIBS
};

//------------------------------------------------------------------------------
//!
StringMap _attributes(
   "done",       ATTRIB_DONE,
   "energy",     ATTRIB_ENERGY,
   "useEnergy",  ATTRIB_USE_ENERGY,
   ""
);

//------------------------------------------------------------------------------
//!
int useEnergyVM( VMState* vm )
{
   ParticleGenerator* g = (ParticleGenerator*)VM::thisPtr(vm);
   CHECK( g != NULL );
   g->useEnergy( (float)VM::toNumber( vm, 1 ) );
   return 0;
}

UNNAMESPACE_END

//------------------------------------------------------------------------------
//!
void
ParticleGenerator::init( VMState* vm )
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
ParticleGenerator::performGet( VMState* vm )
{
   switch( _attributes[VM::toCString(vm, -1)] )
   {
      case ATTRIB_DONE:
         VM::push( vm, done() );
         return true;
      case ATTRIB_ENERGY:
         VM::push( vm, energy() );
         return true;
      case ATTRIB_USE_ENERGY:
         VM::push( vm, this, useEnergyVM );
         return true;
      default:
         break;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
bool
ParticleGenerator::performSet( VMState* vm )
{
   switch( _attributes[VM::toCString(vm, -2)] )
   {
      case ATTRIB_DONE:
         // Read-only.
         return true;
      case ATTRIB_ENERGY:
         energy( (float)VM::toNumber( vm, -1 ) );
         return true;
      case ATTRIB_USE_ENERGY:
         // Read-only.
         return true;
      default:
         break;
   }
   return false;
}
