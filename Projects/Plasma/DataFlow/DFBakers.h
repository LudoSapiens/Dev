/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_DATA_FLOW_BAKERS_H
#define PLASMA_DATA_FLOW_BAKERS_H

#include <Plasma/StdDefs.h>
#include <Plasma/DataFlow/DFWorldNodes.h>
#include <Plasma/World/Probe.h>

NAMESPACE_BEGIN

/*==============================================================================
   STATIC INITIALIZATION
==============================================================================*/

void initializeBakers();
void terminateBakers();


/*==============================================================================
   CLASS DFProbeBaker
==============================================================================*/

class DFProbeBaker:
   public DFNode
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API DFProbeBaker();

   // Attributes.
   PLASMA_DLL_API virtual const ConstString&  name() const;

   // Input/output.
   PLASMA_DLL_API virtual uint numInputs() const;
   PLASMA_DLL_API virtual DFInput*  input( uint );
   PLASMA_DLL_API virtual DFOutput* output();

   // Editor.
   PLASMA_DLL_API RCP<DFNodeEditor> edit();

   inline void  markForBaking() { _mustBake = true; }

protected:

   /*----- methods -----*/

   RCP<DFWorld>  process();

   /*----- data members -----*/

   DFWorldInput       _input;
   DFWorldOutput      _output;
   RCP<DFNodeEditor>  _editor;
   RCP<DFWorld>       _world;
   bool               _mustBake;
};

NAMESPACE_END

#endif //PLASMA_DATA_FLOW_BAKERS_H
