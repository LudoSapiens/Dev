/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_MENUITEM_H
#define FUSION_MENUITEM_H

#include <Fusion/StdDefs.h>
#include <Fusion/Widget/Widget.h>

#include <Base/ADT/String.h>

NAMESPACE_BEGIN

class Menu;

/*==============================================================================
  CLASS MenuItem
==============================================================================*/

//!
//! Attributes list for WM:
//!
//!   read/write:
//!     menu: Menu widget used if the item is selected.

class MenuItem:
   public Widget
{

public:

   /*----- static methods -----*/

   static void initialize();

   /*----- methods -----*/

   FUSION_DLL_API MenuItem();

   FUSION_DLL_API virtual Widget* findWidget( const String& id );

   // VM.
   const char* meta() const;
   FUSION_DLL_API void init( VMState* vm );
   FUSION_DLL_API bool performGet( VMState* );
   FUSION_DLL_API bool performSet( VMState* );

protected:

   /*----- methods -----*/

   virtual ~MenuItem();

   //! Event handlers
   virtual void onClick( const Event& );
   virtual void onPointerEnter( const Event& );

   virtual bool isAttribute( const char* ) const;

private:

   /*----- methods -----*/

   void itemSelected( Widget* item );

   /*----- data members -----*/

   RCP<Menu> _menu;
};

NAMESPACE_END

#endif
