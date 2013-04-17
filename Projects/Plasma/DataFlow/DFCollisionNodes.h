/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_DFCOLLISION_NODES_H
#define PLASMA_DFCOLLISION_NODES_H

#include <Plasma/StdDefs.h>
#include <Plasma/DataFlow/DFGeomNodes.h>

NAMESPACE_BEGIN

/*==============================================================================
   STATIC INITIALIZATION
==============================================================================*/

void initializeCollisionNodes();
void terminateCollisionNodes();

/*==============================================================================
   CLASS DFAutoCollisionNode
==============================================================================*/

class DFAutoCollisionNode:
   public DFNode
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API DFAutoCollisionNode();

   // Attributes.
   PLASMA_DLL_API virtual const ConstString&  name() const;

   // Editor.
   PLASMA_DLL_API RCP<DFNodeEditor> edit();

   // Input/output.
   PLASMA_DLL_API virtual uint numInputs() const;

   PLASMA_DLL_API virtual DFOutput* output();
   PLASMA_DLL_API virtual DFInput* input( uint );

   // Dump.
   PLASMA_DLL_API virtual bool dumpCustom( TextStream& os, StreamIndent& indent ) const;

   int collisionType() const   { return _type; }
   void collisionType( int t ) { _type = t; }

protected:

   /*----- methods -----*/

   RCP<DFGeometry>  process();

   /*----- data members -----*/

   RCP<DFNodeEditor>  _editor;
   DFGeomInput        _input;
   DFGeomOutput       _output;
   int                _type;
};

NAMESPACE_END

#endif
