/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Physics/PlasmaAttractors.h>

#include <Base/ADT/StringMap.h>
#include <Base/Dbg/DebugStream.h>

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_att, "Attractor" );


//------------------------------------------------------------------------------
//!
enum 
{
   ATTRIB_ACCELERATION,
   ATTRIB_ATTRACTIONMASK,
   ATTRIB_THRESHOLD
};

//------------------------------------------------------------------------------
//!
StringMap _attributes(
   "acceleration",   ATTRIB_ACCELERATION,
   "attractionMask", ATTRIB_ATTRACTIONMASK,
   "threshold",      ATTRIB_THRESHOLD,
   ""
);

UNNAMESPACE_END



NAMESPACE_BEGIN

namespace Attractors
{

//------------------------------------------------------------------------------
//! Initialize all of the attractors.
void initialize( VMState* vm )
{
   DBG_BLOCK( os_att, "Initializing all attractors" );
   PlasmaDirectionalAttractorVM::initialize( vm, "attractor" );
   PlasmaGravitationalAttractorVM::initialize( vm, "attractor" );
}

} // Attractors


/*==============================================================================
  CLASS PlasmaDirectionalAttractor
==============================================================================*/

//------------------------------------------------------------------------------
//!
PlasmaDirectionalAttractor::PlasmaDirectionalAttractor
( void )
{
   
}

//------------------------------------------------------------------------------
//!
PlasmaDirectionalAttractor::PlasmaDirectionalAttractor
( const Vec3f& acceleration ):
   DirectionalAttractor( acceleration )
{
   
}

//------------------------------------------------------------------------------
//!
PlasmaDirectionalAttractor::~PlasmaDirectionalAttractor
( void )
{
   
}

//------------------------------------------------------------------------------
//!
void
PlasmaDirectionalAttractor::init( VMState* vm )
{
   DBG_BLOCK( os_att, "PlasmaDirectionalAttractor::init" );
   if( VM::isTable(vm, -1) )
   {
      // Start iterating at index 0 (nil, pushed below).
      VM::push(vm);
      while( VM::next(vm, -2) )
      {
         // Let the performSet() routine handle the various assignments.
         performSet(vm);

         // Pop the value, but keep the key.
         VM::pop(vm, 1);
      }
   }
   else
   {
      DBG_MSG( os_att, "Top of stack isn't a table" );
   }
}

//------------------------------------------------------------------------------
//!
bool
PlasmaDirectionalAttractor::performGet( VMState* vm )
{
   DBG_BLOCK( os_att, "PlasmaDirectionalAttractor::performGet" );

   switch( _attributes[VM::toCString(vm,-1)] )
   {
      case ATTRIB_ACCELERATION:
         VM::push( vm, acceleration() );
         return true;
      case ATTRIB_ATTRACTIONMASK:
         VM::push( vm, attractionMask() );
         return true;
      default:
         printf("PlasmaDirectionalAttractor::performGet - '%s' unknown\n", VM::toCString(vm,-1) );
   } //switch..case
   return false;
}

//------------------------------------------------------------------------------
//!
bool
PlasmaDirectionalAttractor::performSet( VMState* vm )
{
   DBG_BLOCK( os_att, "PlasmaDirectionalAttractor::performSet" );

   switch( _attributes[VM::toCString(vm,-2)] )
   {
      case ATTRIB_ACCELERATION:
         acceleration( VM::toVec3f(vm, -1) );
         return true;
      case ATTRIB_ATTRACTIONMASK:
         attractionMask( VM::toUInt(vm,-1) );
         return true;
      default:
         printf("PlasmaDirectionalAttractor::performSet - '%s' unknown\n", VM::toCString(vm,-2) );
   }  //switch..case
   return false;
}


/*==============================================================================
  CLASS PlasmaGravitationalAttractor
==============================================================================*/

//------------------------------------------------------------------------------
//!
PlasmaGravitationalAttractor::PlasmaGravitationalAttractor
( void )
{
   
}

//------------------------------------------------------------------------------
//!
PlasmaGravitationalAttractor::PlasmaGravitationalAttractor
( const float threshold ):
   GravitationalAttractor( threshold )
{
   
}

//------------------------------------------------------------------------------
//!
PlasmaGravitationalAttractor::~PlasmaGravitationalAttractor
( void )
{
   
}

//------------------------------------------------------------------------------
//!
void
PlasmaGravitationalAttractor::init( VMState* vm )
{
   DBG_BLOCK( os_att, "PlasmaGravitationalAttractor::init" );
   if( VM::isTable(vm, -1) )
   {
      // Start iterating at index 0 (nil, pushed below).
      VM::push(vm);
      while( VM::next(vm, -2) )
      {
         // Let the performSet() routine handle the various assignments.
         performSet(vm);

         // Pop the value, but keep the key.
         VM::pop(vm, 1);
      }
   }
   else
   {
      DBG_MSG( os_att, "Top of stack isn't a table" );
   }
}

//------------------------------------------------------------------------------
//!
bool
PlasmaGravitationalAttractor::performGet( VMState* vm )
{
   DBG_BLOCK( os_att, "PlasmaGravitationalAttractor::performGet" );
   String key;
   if( !VM::isNumber(vm, -1) )
   {
      key = VM::toString(vm, -1);
   }
   else
   {
      key = String(VM::toUInt(vm, -1));
   }

   switch( _attributes[key.cstr()] )
   {
      case ATTRIB_THRESHOLD:
      {
         VM::push(vm, threshold());
         DBG_MSG( os_att, "Setting threshold to " << threshold() );
      } return true;
      default:
      {
         printf("PlasmaGravitationalAttractor::performGet - '%s' unknown\n", key.cstr());
      }
   } //switch..case
   return false;
}

//------------------------------------------------------------------------------
//!
bool
PlasmaGravitationalAttractor::performSet( VMState* vm )
{
   DBG_BLOCK( os_att, "PlasmaGravitationalAttractor::performSet" );
   String key;
   if( !VM::isNumber(vm, -2) )
   {
      key = VM::toString(vm, -2);
   }
   else
   {
      key = String(VM::toUInt(vm, -2));
   }

   switch( _attributes[key.cstr()] )
   {
      case ATTRIB_THRESHOLD:
      {
         threshold( (float)VM::toNumber(vm, -1) );
         DBG_MSG( os_att, "Returning threshold of " << threshold() );
      } return true;
      default:
      {
         printf("PlasmaGravitationalAttractor::performSet - '%s' unknown\n", key.cstr());
      }
   }  //switch..case
   return false;
}


NAMESPACE_END
