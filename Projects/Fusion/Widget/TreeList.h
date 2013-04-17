/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_TREELIST_H
#define FUSION_TREELIST_H

#include <Fusion/StdDefs.h>
#include <Fusion/Widget/Box.h>
#include <Fusion/Core/Event.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS TreeList
==============================================================================*/

//!
//! TreeList is a special WidgetContainer that should contain only TreeList as
//! subwidgets The look of a TreeList can be controlled by the _node member.
//!

class TreeList
   : public Box
{

public:

   /*----- types and enumerations ----*/

   // FIXME: Need a global message number manager
   enum Message {
      CHILD_SELECTED = 13213,
      CHILD_UNSELECT = 24434,
      FLAG_AS_CHILD  = 5112,
      FLAG_AS_ROOT   = 5113,
      CHECK_CHILD    = 6132,
      HAS_CHILD      = 6134,
      UNHIDE         = 84301,
      HIDE           = 84302
   };

   /*----- static methods -----*/

   static void initialize();

   /*----- methods -----*/

   FUSION_DLL_API TreeList();

   FUSION_DLL_API void text( const String& );

   FUSION_DLL_API void childListModified();

   inline bool isLast() const;
   inline bool hasChild() const;
   inline bool isRoot() const;

   //! Callbacks
   FUSION_DLL_API void addOnItemSelect( const Delegate1<const RCP<Widget>&>& );
   FUSION_DLL_API void removeOnItemSelect( const Delegate1<const RCP<Widget>&>& );

   FUSION_DLL_API void addOnModify( const Delegate1<const RCP<Widget>&>& );
   FUSION_DLL_API void removeOnModify( const Delegate1<const RCP<Widget>&>& );

   FUSION_DLL_API virtual void sendParentMessage( Widget* widget, int message );
   FUSION_DLL_API virtual void sendChildMessage( Widget* widget, int message );

   // VM.
   const char* meta() const;
   FUSION_DLL_API void init( VMState* vm );
   FUSION_DLL_API bool performGet( VMState* );
   FUSION_DLL_API bool performSet( VMState* );

protected:


   /*----- methods -----*/

   virtual ~TreeList();

   void selected( bool selected );
   void opened( bool opened );

   virtual bool isAttribute( const char* ) const;

   void modified();

private:

   /*----- data members -----*/

   Delegate1List<const RCP<Widget>&> _onItemSelect;
   VMRef                             _onItemSelectRef;

   Delegate1List<const RCP<Widget>&> _onModify;
   VMRef                             _onModifyRef;

   String                            _text;

   bool                              _opened;
   bool                              _selected;
   bool                              _isRoot;
   bool                              _hasChild;

};



//------------------------------------------------------------------------------
//!
inline bool
TreeList::isLast() const
{
   return _isRoot || (parent()->widgets().back() == this);
}

//------------------------------------------------------------------------------
//!
inline bool
TreeList::hasChild() const
{
   return _hasChild;
}

//------------------------------------------------------------------------------
//!
inline bool
TreeList::isRoot() const
{
   return _isRoot;
}

NAMESPACE_END

#endif

