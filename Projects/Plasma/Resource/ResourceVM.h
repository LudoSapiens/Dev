/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_RESOURCEVM_H
#define PLASMA_RESOURCEVM_H

#include <Plasma/StdDefs.h>

#include <Fusion/VM/VM.h>

NAMESPACE_BEGIN

class DFNodeAttr;
class DFNodeAttrList;

/*==============================================================================
  VM proxy for resources.
==============================================================================*/

namespace ResourceVM
{

   void initialize();

   void push( VMState* vm, const DFNodeAttr& attr );
   void push( VMState* vm, const DFNodeAttrList& attrs );

};

NAMESPACE_END

#endif

