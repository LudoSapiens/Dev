/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Fusion/VM/VMSubject.h>

#include <Base/ADT/StringMap.h>
#include <Base/Dbg/DebugStream.h>

USING_NAMESPACE


/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

DBG_STREAM( os_vm, "VMSubject" );

//------------------------------------------------------------------------------
//!
enum
{
   ATTRIB_NOTIFY,
   ATTRIB_SUBJECT,
   NUM_ATTRIBS
};

//------------------------------------------------------------------------------
//!
StringMap _attributes(
   "notify",   ATTRIB_NOTIFY,
   "subject",  ATTRIB_SUBJECT,
   ""
);

//------------------------------------------------------------------------------
//!
int
notifyVM( VMState* vm )
{
   DBG_BLOCK( os_vm, "notifyVM" );

   VMSubject* vms = (VMSubject*)VM::thisPtr(vm);
   CHECK( vms != NULL );

   vms->subject().notify();

   return 0;
}

UNNAMESPACE_END


/*==============================================================================
  CLASS VMSubject
==============================================================================*/


//------------------------------------------------------------------------------
//!
VMSubject::VMSubject()
{}

//------------------------------------------------------------------------------
//!
VMSubject::~VMSubject()
{}

//------------------------------------------------------------------------------
//!
void
VMSubject::init( VMState* vm )
{
   DBG_BLOCK( os_vm, "VMSubject::init" );
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
}

//------------------------------------------------------------------------------
//!
bool
VMSubject::performGet( VMState* vm )
{
   DBG_BLOCK( os_vm, "VMSubject::performGet" );
   const char* str = VM::toCString( vm, -1 );
   switch( _attributes[str] )
   {
      case ATTRIB_NOTIFY:
      {
         VM::push( vm, this, notifyVM );
      }  return true;
      case ATTRIB_SUBJECT:
      {
         VM::push( vm, &_subject );
      }  return true;
      default:
      {
      }  break;
   }

   return false;
}

//------------------------------------------------------------------------------
//!
bool
VMSubject::performSet( VMState* vm )
{
   DBG_BLOCK( os_vm, "VMSubject::performSet" );
   const char* str = VM::toCString( vm, -2 );
   switch( _attributes[str] )
   {
      case ATTRIB_NOTIFY:
      case ATTRIB_SUBJECT:
         //Read-only
         return true;
      default:
         break;
   }

   return false;
}
