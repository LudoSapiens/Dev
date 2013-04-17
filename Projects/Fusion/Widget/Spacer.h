/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_SPACER_H
#define FUSION_SPACER_H

#include <Fusion/StdDefs.h>
#include <Fusion/Widget/Widget.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Spacer
==============================================================================*/

//!

class Spacer
   : public Widget
{

public:
   
   /*----- static methods -----*/

   static void initialize();

   /*----- methods -----*/

   Spacer();

   virtual void render( const RCP<Gfx::RenderNode>& );
   
   // VM.
   const char* meta() const;
   FUSION_DLL_API void init( VMState* vm );

protected:

   /*----- methods -----*/
   
   virtual ~Spacer();

};

NAMESPACE_END

#endif

