/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_COMBOBOX_H
#define FUSION_COMBOBOX_H

#include <Fusion/StdDefs.h>
#include <Fusion/Widget/Widget.h>

#include <Base/ADT/String.h>

NAMESPACE_BEGIN

class Menu;

/*==============================================================================
  CLASS ComboBox
==============================================================================*/

//!
//!
//! Attributes list for WM:
//!
//!   read only:
//!     item: Current selected widget.
//!
//!   read/write:
//!     itemId:  Id of the current selected widget.
//!     onItemChanged:  Cb call when the selected item widget change.

class ComboBox:
   public Widget
{

public:

   /*----- static methods -----*/

   static void initialize();

   /*----- methods -----*/

   FUSION_DLL_API ComboBox();

   inline const String& itemId() const { return _itemId; }
   FUSION_DLL_API void itemId( const String& id );
   FUSION_DLL_API void item( Widget* widget );

   FUSION_DLL_API bool opened() const;

   //! Callbacks
   FUSION_DLL_API void addOnItemChanged( const Delegate1<const String&>& );
   FUSION_DLL_API void removeOnItemChanged( const Delegate1<const String&>& );

   // VM.
   const char* meta() const;
   FUSION_DLL_API void init( VMState* vm );
   FUSION_DLL_API bool performGet( VMState* );
   FUSION_DLL_API bool performSet( VMState* );

protected:

   /*----- methods -----*/

   virtual ~ComboBox();

   //! Event handlers
   virtual void onClick( const Event& );
   virtual void onItemChanged( const String& );
   virtual bool isAttribute( const char* ) const;

private:

   /*----- methods -----*/

   void itemSelected( Widget* item );
   void popupped( Widget* );

   /*----- data members -----*/

   RCP<Widget>                  _item;
   RCP<Menu>                    _menu;
   bool                         _menuAbove;
   Vec2f                        _offset;
   String                       _itemId;
   Delegate1List<const String&> _onItemChanged;
   VMRef                        _onItemChangedRef;

};

NAMESPACE_END

#endif
