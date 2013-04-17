/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/DataFlow/ProceduralDataFlow.h>

#include <Fusion/VM/VMRegistry.h>

/*==============================================================================
  UNNAME NAMESPACE
==============================================================================*/

UNNAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
int connectVM( VMState* )
{
   return 0;
}

//------------------------------------------------------------------------------
//!
int outputVM( VMState* vm )
{
   DFGraph* graph = (DFGraph*)VM::userData( vm );
   DFNode* node   = (DFNode*)VM::toPtr( vm, 1 );

   if( node == nullptr )
   {
      StdErr << "ERROR - Invalid output node specified." << nl;
      return 0;
   }

   //StdErr << "Output name: " << node->name() << " #input: " << node->numInputs() << nl;

   graph->output( node );

   return 0;
}

//------------------------------------------------------------------------------
//!
int nodeVM( VMState* vm )
{
   DFGraph*   graph       = (DFGraph*)VM::userData( vm );
   ConstString type       = VM::toConstString( vm, VM::upvalue(1) );
   const DFNodeSpec* spec = DFNodeSpec::get( type );

   if( spec == nullptr ) return 0;

   RCP<DFNode> node = spec->create( vm, 1 );

   if( node.isNull() )  return 0;

   Vec2i pos(0,0);
   int   width = node->width();
   VM::get( vm, 1, "position", pos );
   VM::get( vm, 1, "width", width );

   graph->addNode( node.ptr(), pos, width );

   VM::push( vm, node.ptr() );
   return 1;
}

//------------------------------------------------------------------------------
//!
int readVM( VMState* vm )
{
   VM::push( vm, nodeVM, 1 );
   return 1;
}

//------------------------------------------------------------------------------
//!
const VM::Reg funcs[] = {
   // Graph.
   { "connect",      connectVM      },
   { "output",       outputVM       },
   { 0,0 }
};

//------------------------------------------------------------------------------
//!
void initVM( VMState* vm, uint /*mask*/ )
{
   VM::registerFunctions( vm, "_G", funcs );
}

UNNAMESPACE_END


NAMESPACE_BEGIN

/*==============================================================================
   CLASS ProceduralDataFlow
==============================================================================*/

//------------------------------------------------------------------------------
//!
void
ProceduralDataFlow::initialize()
{
   VMRegistry::add( initVM, VM_CAT_DF );
}

//------------------------------------------------------------------------------
//!
ProceduralDataFlow::ProceduralDataFlow(
   Resource<DFGraph>* res,
   const String&      path
):
   _res( res ),
   _path( path )
{
}

//------------------------------------------------------------------------------
//!
void
ProceduralDataFlow::execute()
{
   _graph = new DFGraph();

   {
      DFGraph::UpdateLock lock( _graph.ptr() );

      // Prepare to build the data flow graph.
      VMState* vm = VM::open( VM_CAT_DF | VM_CAT_MATH, readVM, VM::writesDisabledVM );

      VM::userData( vm, _graph.ptr() );
      VM::doFile( vm, _path, 0 );

      VM::close( vm );
   }

   // Set the resulting graph.
   _res->data( _graph.ptr() );
}

NAMESPACE_END

