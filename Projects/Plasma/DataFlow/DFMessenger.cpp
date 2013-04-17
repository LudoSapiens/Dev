/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/DataFlow/DFMessenger.h>
#include <Plasma/DataFlow/DFGraph.h>
#include <Plasma/DataFlow/DFNodeAttr.h>
#include <Plasma/Resource/ResourceVM.h>

#include <CGMath/Variant.h>

#include <Base/Dbg/Defs.h>

#include <cstddef>

USING_NAMESPACE

/*==============================================================================
  UNNAMED NAMESPACE
==============================================================================*/
UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
void execute( const VMRef& ref, DFGraph* graph )
{
   if( ref.isValid() )
   {
      VMState* vm = ref.vm();
      VM::push( vm, ref );
      VM::pushProxy( vm, graph );
      VM::ecall( vm, 1, 0 );
   }
}

//------------------------------------------------------------------------------
//!
void execute( const VMRef& ref, DFNode* node )
{
   if( ref.isValid() )
   {
      VMState* vm = ref.vm();
      VM::push( vm, ref );
      VM::push( vm, node );
      VM::ecall( vm, 1, 0 );
   }
}

//------------------------------------------------------------------------------
//!
void execute( const VMRef& ref, DFNode* node, const DFNodeAttrList* a, const DFNodeAttrStates* t )
{
   if( ref.isValid() )
   {
      VMState* vm = ref.vm();
      VM::push( vm, ref );
      VM::push( vm, node );
      if( a || t )
      {
         VM::newTable( vm );
         if( a )
         {
            VM::push( vm, "attr" );
            ResourceVM::push( vm, *a );
            VM::set( vm, -3 );
         }
         if( t )
         {
            for( auto cur = t->begin(); cur != t->end(); ++cur )
            {
               VM::push( vm, cur->_id );
               VM::push( vm, cur->_value );
               VM::set( vm, -3 );
            }
         }
      }
      else
      {
         VM::push( vm );
      }
      VM::ecall( vm, 2, 0 );
   }
}

UNNAMESPACE_END

/*==============================================================================
  CLASS DFMessenger
==============================================================================*/

//------------------------------------------------------------------------------
//!
DFMessenger::DFMessenger()
{
}

//------------------------------------------------------------------------------
//!
DFMessenger::~DFMessenger()
{
}

#if __llvm__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif

//------------------------------------------------------------------------------
//!
DFGraph&
DFMessenger::graph()
{
   return *(DFGraph*)((char*)this - offsetof(DFGraph, _messenger));
}

//------------------------------------------------------------------------------
//!
const DFGraph&
DFMessenger::graph() const
{
   return *(const DFGraph*)((char*)this - offsetof(DFGraph, _messenger));
}

#if __llvm__
#pragma GCC diagnostic pop
#endif

//------------------------------------------------------------------------------
//!
void
DFMessenger::update()
{
   _graphUpdate.exec( &graph(), execute );
}

//------------------------------------------------------------------------------
//!
void
DFMessenger::update( DFNode* node )
{
   const auto it = _nodeUpdate.find( node );
   if( it != _nodeUpdate.end() )  (*it).second.exec( node, execute );
}

void
DFMessenger::modify( DFNode* node, const DFNodeAttrList* a, const DFNodeAttrStates* t )
{
   const auto it = _nodeModify.find( node );
   if( it != _nodeModify.end() )  (*it).second.exec( node, a, t, execute );
}

//------------------------------------------------------------------------------
//!
void
DFMessenger::remove( DFNode* node )
{
   const auto it = _nodeRemove.find( node );
   if( it != _nodeRemove.end() )  (*it).second.exec( node, execute );
}
