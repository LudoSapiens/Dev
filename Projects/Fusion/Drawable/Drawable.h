/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_DRAWABLE_H
#define FUSION_DRAWABLE_H

#include <Fusion/StdDefs.h>
#include <Fusion/VM/VM.h>

#include <Gfx/Pass/RenderNode.h>

#include <Base/Util/RCObject.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Drawable
==============================================================================*/

//!   

class Drawable: 
   public RCObject,
   public VMProxy
{

public:
   
   /*----- methods -----*/

   FUSION_DLL_API virtual void draw( const RCP<Gfx::RenderNode>& ) const = 0;

   // VM.
   FUSION_DLL_API bool performGet( VMState* );
   FUSION_DLL_API bool performSet( VMState* );
   
protected:

   /*----- methods -----*/

   Drawable();
};

NAMESPACE_END

#endif
