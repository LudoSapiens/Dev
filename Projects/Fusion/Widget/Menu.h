/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_MENU_H
#define FUSION_MENU_H

#include <Fusion/StdDefs.h>
#include <Fusion/Widget/Box.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS Menu
==============================================================================*/

//!
//! Attributes list for WM:
//!
//!   read/write:
//!     onItemSelect: Cb called when on children item widget has been selected.

class Menu:
   public Box
{

public:

   /*----- types and enumerations ----*/

   enum Message {
      ITEM_SELECTED = 1
   };

   /*----- static methods -----*/

   static void initialize();

   /*----- methods -----*/

   FUSION_DLL_API Menu();

   //! Callbacks
   FUSION_DLL_API void addOnItemSelect( const WidgetDelegate& );
   FUSION_DLL_API void removeOnItemSelect( const WidgetDelegate& );

   FUSION_DLL_API virtual void sendParentMessage( Widget* widget, int message );

   // VM.
   const char* meta() const;
   FUSION_DLL_API void init( VMState* vm );
   FUSION_DLL_API bool performGet( VMState* );
   FUSION_DLL_API bool performSet( VMState* );

protected:

   /*----- methods -----*/

   virtual ~Menu();

   virtual void onItemSelect( Widget* );
   virtual bool isAttribute( const char* ) const;

   /*----- data members -----*/

   WidgetDelegateList _onItemSelect;
   VMRef              _onItemSelectRef;
};

NAMESPACE_END

#endif

