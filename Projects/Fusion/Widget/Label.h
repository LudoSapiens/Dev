/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_LABEL_H
#define FUSION_LABEL_H

#include <Fusion/StdDefs.h>
#include <Fusion/Widget/Widget.h>

#include <Base/ADT/String.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Label
==============================================================================*/

//!
//! Attributes list for WM:
//!
//!   read/write:
//!     text: Label text.

class Label 
   : public Widget
{

public:

   /*----- static methods -----*/

   static void initialize();

   /*----- methods -----*/

   FUSION_DLL_API Label();
   
   FUSION_DLL_API void text( const String& );
   
   // VM.
   const char* meta() const;
   FUSION_DLL_API void init( VMState* vm );
   FUSION_DLL_API bool performGet( VMState* );
   FUSION_DLL_API bool performSet( VMState* );

protected: 

   /*----- methods -----*/
   
   virtual ~Label();
   virtual bool isAttribute( const char* ) const;
   
private:
   
   /*----- data members -----*/

   String _text;
};

NAMESPACE_END

#endif
