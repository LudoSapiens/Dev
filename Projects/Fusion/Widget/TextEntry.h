/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef FUSION_TEXTENTRY_H
#define FUSION_TEXTENTRY_H

#include <Fusion/StdDefs.h>
#include <Fusion/Widget/Widget.h>
#include <Fusion/Drawable/Text.h>

NAMESPACE_BEGIN

/*==============================================================================
  CLASS TextEntry
==============================================================================*/

//!
//! Attributes list for WM:
//!
//!   read only:
//!     textCursor:  Position of the text cursor in character.
//!     inEdit:  True if the textEntry is been modified.
//!
//!   read/write:
//!     text:  Text value.

class TextEntry
   : public Widget
{

public:

   /*----- static methods -----*/

   static void initialize();

   /*----- methods -----*/

   FUSION_DLL_API TextEntry();

   FUSION_DLL_API void text( const String&, bool update = true );

   uint cursor() const           { return _cursor; }
   void cursor( const uint pos ) { _cursor = pos; }

   uint multiEdit() const        { return _multiEdit; }
   void multiEdit( bool v )      { _multiEdit = v; }

   FUSION_DLL_API bool edit();

   //! Callback
   FUSION_DLL_API void addOnModify( const Delegate1<String>& );
   FUSION_DLL_API void removeOnModify( const Delegate1<String>& );
   FUSION_DLL_API void addOnStartEdit( const Delegate0<>& );
   FUSION_DLL_API void removeOnStartEdit( const Delegate0<>& );
   FUSION_DLL_API void addOnEdit( const Delegate0<>& );
   FUSION_DLL_API void removeOnEdit( const Delegate0<>& );
   FUSION_DLL_API void addOnEndEdit( const Delegate0<>& );
   FUSION_DLL_API void removeOnEndEdit( const Delegate0<>& );

   //! Event handlers
   virtual void onPointerPress( const Event& );
   virtual void onKeyPress( const Event& );
   virtual void onChar( const Event& );
   virtual void onFocusLose( const Event& );

   // VM.
   const char* meta() const;
   FUSION_DLL_API void init( VMState* vm );
   FUSION_DLL_API bool performGet( VMState* );
   FUSION_DLL_API bool performSet( VMState* );

protected:

   /*----- methods -----*/

   FUSION_DLL_API virtual ~TextEntry();

   virtual bool isAttribute( const char* ) const;

private:

   /*----- data members -----*/

   bool                  _multiEdit;
   bool                  _inEdit;
   uint                  _cursor;
   String                _text;
   String                _prevText;
   RCP<Text>             _textDrawable;
   Delegate1List<String> _onModify;
   VMRef                 _onModifyRef;
   Delegate0List<>       _onStartEdit;
   VMRef                 _onStartEditRef;
   Delegate0List<>       _onEdit;
   VMRef                 _onEditRef;
   Delegate0List<>       _onEndEdit;
   VMRef                 _onEndEditRef;
};

NAMESPACE_END

#endif

