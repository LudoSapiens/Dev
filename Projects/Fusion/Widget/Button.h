/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_BUTTON_H
#define FUSION_BUTTON_H

#include <Fusion/StdDefs.h>
#include <Fusion/Widget/Widget.h>

#include <Base/ADT/String.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Button
==============================================================================*/

//!

class Button
   : public Widget
{

public:

   /*----- static methods -----*/

   static void initialize();

   /*----- methods -----*/
   
   FUSION_DLL_API Button();

   // VM.
   const char* meta() const;
   FUSION_DLL_API void init( VMState* vm );
   FUSION_DLL_API bool performGet( VMState* );
   FUSION_DLL_API bool performSet( VMState* );

protected:
  
   /*----- methods -----*/   

   virtual ~Button();

   //! Event handlers
   virtual void onClick( const Event& );

   virtual bool isAttribute( const char* ) const;
   
private:
   
   /*----- data members -----*/

   bool _toggled;
};

NAMESPACE_END

#endif
